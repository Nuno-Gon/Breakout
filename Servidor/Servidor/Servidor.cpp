#include <tchar.h>
#include <Windows.h>
#include <io.h>
#include <iostream>
#include <stdio.h>
#include <fcntl.h>
#include <time.h>
#include <thread>

#include "..\Dll\dll.h"

using namespace std;

//Listagem de Funções
//Threads
DWORD WINAPI readMensagemMemory(void);
DWORD WINAPI writeMensagemMemory(void);
DWORD WINAPI controlaBola(LPVOID p);
DWORD WINAPI controlaBrinde(LPVOID p);
DWORD WINAPI moveTijolos(LPVOID p);

//Registo
void createRegistry();
void writeRegistry();
void readRegistry();
void inicia();

// Outras Funções
void trataComando(COMANDO_SHARED comando);
void inicia_mapa();
void inicia_configuracao_jogo();
void inicia_configuracao_nivel(int aux);
int getIdPlayer(HANDLE aux);
Player getPlayer(int idUser);
void desconectaPlayer(int id);
void inserePlayerJogo(HANDLE novo, COMANDO_SHARED aux);
BOOL checkDireita(int idUser);
BOOL checkEsquerda(int idUser);
void insereBarreiraJogo(int id);
void moveJogadorEsquerda(int idUser);
void moveJogadorDireita(int idUser);
void meteTop(int id);


//Variaveis Globais
INT acabar;
MensagemJogo msgJogo;
dataCr memoriaPartilhadaServidor;
COMANDO_SHARED comandoLido;
BOOL loginPlayer = FALSE;
HANDLE thread_read_msg_memory, thread_write_msg_memory;
HANDLE thread_bola, thread_brinde, thread_move_tijolos;
HANDLE eventoMemoria, eventoComeco, eventoTerminaJogo;
Scores score;
INT idUserPlayer = 1;
INT idTijolo = 1;
INT idBrinde = 1;
bool jogo;


int _tmain(int argc, LPTSTR argv[]) {

	//Project Properties > Character Set > Use Unicode Character Set
#ifdef UNICODE
	_setmode(_fileno(stdin), _O_WTEXT);
	_setmode(_fileno(stdout), _O_WTEXT);
#endif 
	_tprintf(TEXT("*********************Servidor Ligado!*****************************\n"));
	acabar = 0;
	jogo = false;
	//Mutex
	mutex_player = CreateMutex(NULL, FALSE, NULL);

	//por verificacoes se correu mal ou não
	eventoMemoria = CreateEvent(NULL, FALSE, FALSE, nomeEventoArrancaMemoria);
	eventoComeco = CreateEvent(NULL, FALSE, FALSE, nomeEventoComecoJogo);
	eventoTerminaJogo = CreateEvent(NULL, FALSE, FALSE, nomeEventoTerminaJogo);

	if (eventoComeco == NULL || eventoMemoria == NULL) {
		_tprintf(TEXT("ERRO!"));
		return 0;
	}

	//fazer Protecao
	createSharedMemoryJogo(&memoriaPartilhadaServidor);

	WaitForSingleObject(eventoMemoria, INFINITE);

	//Abrir memoria partilhada
	if (!openSharedMemory(&memoriaPartilhadaServidor)) {
		_tprintf(TEXT("Erro a abrir memoria Partilhada!\n"));
		system("pause");
		exit(0);
	}

	int nivel = 0;
	//Abrir Registro
	createRegistry();
	readRegistry();
	//inicia(); //Inicia o Registry
	inicia_mapa();

	thread_read_msg_memory = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)readMensagemMemory, NULL, 0, NULL);
	thread_write_msg_memory = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)writeMensagemMemory, NULL, 0, NULL);

	TCHAR str[BUFFER_SIZE];
	TCHAR ctr[BUFFER_SIZE];
	TCHAR ttr[BUFFER_SIZE];
	TCHAR vttr[BUFFER_SIZE];

	_tprintf(TEXT("\n"));
	do {
		_tprintf(TEXT("Root: "));
		fflush(stdin);
		_fgetts(str, BUFFER_SIZE, stdin);

		str[_tcslen(str) - 1] = '\0';

		for (int i = 0; i < _tcslen(str); i++) {
			str[i] = _totupper(str[i]);
		}

		if (_tcsicmp(TEXT("JOGO"), str) == 0) {



			if (jogo == false) {

				int count = 0;
				WaitForSingleObject(eventoComeco, INFINITE);
				_tprintf(TEXT("Jogo Iniciado!\n"));
				for (int x = 0; x < MAX_NUM_PLAYERS; x++) {
					if (msgJogo.players[x].idHandle != INVALID_HANDLE_VALUE) {
						if (msgJogo.players[x].login == true) {
							insereBarreiraJogo(msgJogo.players[x].id);
							count++;
						}

					}
				}

				for (int i = 0; i < MAX_NUM_PLAYERS; i++) {
					if (msgJogo.players[i].idHandle != INVALID_HANDLE_VALUE)
						if (msgJogo.players[i].barreira.ativa)
							msgJogo.players[i].barreira.dimensao -= 5 * count;

				}

				INT_PTR aux = 0;
				//Meter se um jogador estiver a entrar quando o jogo está iniciado não deixar controlar uma barreira
				Sleep(200);
				thread_bola = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)controlaBola, reinterpret_cast<LPVOID>(aux), 0, NULL);


				jogo = true;
			}
			else {
				_tprintf(TEXT("JA SE ENCONTRA UM JOGO A DECORRER!\n"));
			}
		}
		else if (_tcsicmp(TEXT("COMANDOS"), str) == 0) {
			_tprintf(TEXT("************** LISTAGEM DE COMANDOS **************\n"));
			_tprintf(TEXT("'JOGO' --> Cria Novo Jogo\n"));
			_tprintf(TEXT("'NIVEL' --> Inserir o nivel que pretende jogar\n"));
			_tprintf(TEXT("'CONFIGURAR' --> Configuracao ja existente\n"));
			_tprintf(TEXT("'CODE' --> Configurar aspectos do jogo!\n"));
			_tprintf(TEXT("'MOVER' --> Tijolos a mover\n"));
			_tprintf(TEXT("'TOP' --> Mostra TOP 10\n"));
			_tprintf(TEXT("'SAIR' --> DESLIGA SERVIDOR\n"));
		}
		else if (_tcsicmp(TEXT("NIVEL"), str) == 0) {

			bool teste = true;
			for (int i = 0; i < MAX_NUM_BOLAS; i++) {
				if (msgJogo.bolas[i].ativa != 0) {
					teste = false;
				}

			}

			if (teste) {
				jogo = false;
			}


			if (jogo == true) {
				_tprintf(TEXT("UM JOGO JA SE ENCONTRA A DECORRER!\n"));
			}
			else {
				nivel++;
				if (nivel > 3) {
					nivel = 1;
				}
				_tprintf(TEXT("NIVEL %d Configurado!\n"), nivel);
				inicia_configuracao_nivel(nivel);
			}

		}
		else if (_tcsicmp(TEXT("CONFIGURAR"), str) == 0) {
			bool teste = true;
			for (int i = 0; i < MAX_NUM_BOLAS; i++) {
				if (msgJogo.bolas[i].ativa != 0) {
					teste = false;
				}

			}

			if (teste) {
				jogo = false;
			}


			if (jogo == true) {
				_tprintf(TEXT("UM JOGO JA SE ENCONTRA A DECORRER!\n"));
			}
			else {
				_tprintf(TEXT("CONFIGURACAO FEITA!\n"));
				inicia_configuracao_jogo();
			}

		}
		else if (_tcsicmp(TEXT("CODE"), str) == 0) {
			bool teste = true;
			for (int i = 0; i < MAX_NUM_BOLAS; i++) {
				if (msgJogo.bolas[i].ativa != 0) {
					teste = false;
				}

			}

			if (teste) {
				jogo = false;
			}


			if (jogo == true) {
				_tprintf(TEXT("UM JOGO JA SE ENCONTRA A DECORRER!\n"));
			}
			else {
				_tprintf(TEXT("Bem Vindo ao menu configuracao!\n"));

				do {

					_tprintf(TEXT("Conf: "));
					fflush(stdin);
					_fgetts(ctr, BUFFER_SIZE, stdin);

					ctr[_tcslen(ctr) - 1] = '\0';

					for (int i = 0; i < _tcslen(ctr); i++) {
						ctr[i] = _totupper(ctr[i]);
					}


					if (_tcsicmp(TEXT("COMANDOS"), ctr) == 0) {
						_tprintf(TEXT("************** LISTAGEM DE COMANDOS **************\n"));
						_tprintf(TEXT("'BOLAS' --> Menu Configuracao Bolas\n"));
						_tprintf(TEXT("'BARREIRAS' --> Menu Configuracao Barreiras\n"));
						_tprintf(TEXT("'TIJOLOS' --> Menu Configuracao Tijolos\n"));
						_tprintf(TEXT("'BRINDES' --> Menu Configuracao Brindes!\n"));
						_tprintf(TEXT("'SAIR' --> Sair do modo configuracao\n"));
					}
					else if (_tcsicmp(TEXT("BOLAS"), ctr) == 0) {
						do {
							_tprintf(TEXT("Bolas: "));
							fflush(stdin);
							_fgetts(ttr, BUFFER_SIZE, stdin);

							ttr[_tcslen(ttr) - 1] = '\0';

							for (int i = 0; i < _tcslen(ttr); i++) {
								ttr[i] = _totupper(ttr[i]);
							}

							if (_tcsicmp(TEXT("COMANDOS"), ttr) == 0) {
								_tprintf(TEXT("************** LISTAGEM DE COMANDOS **************\n"));
								_tprintf(TEXT("'VELOCIDADE' --> Mudar a Velocidade das Bolas\n"));
								_tprintf(TEXT("'RAIO' --> Mudar o Raio das Bolas\n"));
								_tprintf(TEXT("'SAIR' --> Sair para o menu anterior\n"));
							}
							else if (_tcsicmp(TEXT("VELOCIDADE"), ttr) == 0) {
								_tprintf(TEXT("Velocidade: "));
								fflush(stdin);
								_fgetts(vttr, BUFFER_SIZE, stdin);

								vttr[_tcslen(vttr) - 1] = '\0';

								int aux_d = _ttoi(vttr);

								if (aux_d > 30 || aux_d <= 0) {
									_tprintf(TEXT("[ERRO] Insira um valor aceitavel! (nao negativo!)\n"));
								}
								else {
									for (int i = 0; i < MAX_NUM_BOLAS; i++) {
										msgJogo.bolas[i].velocidade = aux_d;
										msgJogo.bolas[i].velocidade_inicial = aux_d;
									}
									_tprintf(TEXT("Velocidade alterada para %d\n"), msgJogo.bolas[0].velocidade);
								}



							}
							else if (_tcsicmp(TEXT("RAIO"), ttr) == 0) {
								_tprintf(TEXT("Raio: "));
								fflush(stdin);
								_fgetts(vttr, BUFFER_SIZE, stdin);

								vttr[_tcslen(vttr) - 1] = '\0';

								int aux_d = _ttoi(vttr);

								if (aux_d > 30 || aux_d <= 0) {
									_tprintf(TEXT("[ERRO] Insira um valor aceitavel! (nao negativo!)\n"));
								}
								else {
									for (int i = 0; i < MAX_NUM_BOLAS; i++) {
										msgJogo.bolas[i].raio = aux_d;
									}
									_tprintf(TEXT("Valor do raio alterado para %d\n"), msgJogo.bolas[0].raio);
								}


							}
							else {
								_tprintf(TEXT("Comando nao reconhecido! (Digite: 'COMANDOS' para ajuda)\n"));
							}
						} while (_tcsicmp(TEXT("SAIR"), ttr));
					}
					else if (_tcsicmp(TEXT("BARREIRAS"), ctr) == 0) {
						do {
							_tprintf(TEXT("Barreiras: "));
							fflush(stdin);
							_fgetts(ttr, BUFFER_SIZE, stdin);

							ttr[_tcslen(ttr) - 1] = '\0';

							for (int i = 0; i < _tcslen(ttr); i++) {
								ttr[i] = _totupper(ttr[i]);
							}

							if (_tcsicmp(TEXT("COMANDOS"), ttr) == 0) {
								_tprintf(TEXT("************** LISTAGEM DE COMANDOS **************\n"));
								_tprintf(TEXT("'VELOCIDADE' --> Mudar a Velocidade das Barreiras\n"));
								_tprintf(TEXT("'DIMENSAO' --> Mudar a Dimensao das Barreiras\n"));
								_tprintf(TEXT("'SAIR' --> Sair para o menu anterior\n"));
							}
							else if (_tcsicmp(TEXT("VELOCIDADE"), ttr) == 0) {
								_tprintf(TEXT("Velocidade: "));
								fflush(stdin);
								_fgetts(vttr, BUFFER_SIZE, stdin);

								vttr[_tcslen(vttr) - 1] = '\0';

								int aux_d = _ttoi(vttr);

								if (aux_d > 100 || aux_d <= 0) {
									_tprintf(TEXT("[ERRO] Insira um valor aceitavel! (nao negativo!)\n"));
								}
								else {
									for (int i = 0; i < MAX_NUM_PLAYERS; i++) {
										msgJogo.players[i].barreira.velocidade = aux_d;
									}

									_tprintf(TEXT("Velocidade dos jogadores alterada para: %d\n"), msgJogo.players[0].barreira.velocidade);
								}



							}
							else if (_tcsicmp(TEXT("DIMENSAO"), ttr) == 0) {
								_tprintf(TEXT("Dimensao: "));
								fflush(stdin);
								_fgetts(vttr, BUFFER_SIZE, stdin);

								vttr[_tcslen(vttr) - 1] = '\0';

								int aux_d = _ttoi(vttr);

								if (aux_d > 200 || aux_d <= 0) {
									_tprintf(TEXT("[ERRO] Insira um valor aceitavel! (nao negativo!)\n"));
								}
								else {
									for (int i = 0; i < MAX_NUM_PLAYERS; i++) {
										msgJogo.players[i].barreira.dimensao = aux_d;
									}
									_tprintf(TEXT("Dimensao da Barreira Alterada para: %d\n"), msgJogo.players[0].barreira.dimensao);
								}


							}
							else {
								_tprintf(TEXT("Comando nao reconhecido! (Digite: 'COMANDOS' para ajuda)\n"));
							}
						} while (_tcsicmp(TEXT("SAIR"), ttr));


					}
					else if (_tcsicmp(TEXT("TIJOLOS"), ctr) == 0) {
						do {
							_tprintf(TEXT("Tijolos: "));
							fflush(stdin);
							_fgetts(ttr, BUFFER_SIZE, stdin);

							ttr[_tcslen(ttr) - 1] = '\0';

							for (int i = 0; i < _tcslen(ttr); i++) {
								ttr[i] = _totupper(ttr[i]);
							}

							if (_tcsicmp(TEXT("COMANDOS"), ttr) == 0) {
								_tprintf(TEXT("************** LISTAGEM DE COMANDOS **************\n"));
								_tprintf(TEXT("'NORMAL' -->  Definir probabilidade tijolos NORMAL\n"));
								_tprintf(TEXT("'MAGICO' -->  Definir probabilidade tijolos MAGICO\n"));
								_tprintf(TEXT("'RESISTENTE' -->Definir probabilidade tijolos RESISTENTE\n"));
								_tprintf(TEXT("'SAIR' --> Sair para o menu anterior\n"));
							}
							else if (_tcsicmp(TEXT("NORMAL"), ttr) == 0) {
								_tprintf(TEXT("Normal: "));
								fflush(stdin);
								_fgetts(vttr, BUFFER_SIZE, stdin);

								vttr[_tcslen(vttr) - 1] = '\0';

								int aux_d = _ttoi(vttr);

								if (aux_d > 100 || aux_d < 0) {
									_tprintf(TEXT("[ERRO] Insira um valor aceitavel! (nao negativo!)\n"));
								}
								else {
									int aux_ppp;
									for (int i = 0; i < MAX_NUM_TIJOLOS; i++) {
										srand(NULL);
										aux_ppp = rand() % 100;
									
										if (aux_ppp <= aux_d) {
											msgJogo.tijolos[i].vida = 1;
											msgJogo.tijolos[i].tipo = normal;
										}
										
									}
									_tprintf(TEXT("Tijolos Normais criados!\n"));
								}



							}
							else if (_tcsicmp(TEXT("MAGICO"), ttr) == 0) {
								_tprintf(TEXT("Magico: "));
								fflush(stdin);
								_fgetts(vttr, BUFFER_SIZE, stdin);

								vttr[_tcslen(vttr) - 1] = '\0';

								int aux_d = _ttoi(vttr);

								if (aux_d > 100 || aux_d < 0) {
									_tprintf(TEXT("[ERRO] Insira um valor aceitavel! (nao negativo!)\n"));
								}
								else {
									int aux_ppp;
									for (int i = 0; i < MAX_NUM_TIJOLOS; i++) {
										srand(NULL);
										aux_ppp = rand() % 100;

										if (aux_ppp <= aux_d) {
											msgJogo.tijolos[i].vida = 1;
											msgJogo.tijolos[i].tipo = magico;
										}

									}
									_tprintf(TEXT("Tijolos Magicos criados!\n"));
								}


							}
							else if (_tcsicmp(TEXT("RESISTENTE"), ttr) == 0) {
								_tprintf(TEXT("Resistente: "));
								fflush(stdin);
								_fgetts(vttr, BUFFER_SIZE, stdin);

								vttr[_tcslen(vttr) - 1] = '\0';

								int aux_d = _ttoi(vttr);

								if (aux_d > 100 || aux_d < 0) {
									_tprintf(TEXT("[ERRO] Insira um valor aceitavel! (nao negativo!)\n"));
								}
								else {
									int aux_ppp;
									for (int i = 0; i < MAX_NUM_TIJOLOS; i++) {
										srand(NULL);
										aux_ppp = rand() % 100;

										if (aux_ppp <= aux_d) {
											msgJogo.tijolos[i].vida =  2 + rand() % 3;
											msgJogo.tijolos[i].tipo = resistente;
										}

									}
									_tprintf(TEXT("Tijolos Resistentes criados!\n"));
								}


							}
							else {
								_tprintf(TEXT("Comando nao reconhecido! (Digite: 'COMANDOS' para ajuda)\n"));
							}
						} while (_tcsicmp(TEXT("SAIR"), ttr));
					}
					else if (_tcsicmp(TEXT("BRINDES"), ctr) == 0) {
						do {
							_tprintf(TEXT("Brindes: "));
							fflush(stdin);
							_fgetts(ttr, BUFFER_SIZE, stdin);

							ttr[_tcslen(ttr) - 1] = '\0';

							for (int i = 0; i < _tcslen(ttr); i++) {
								ttr[i] = _totupper(ttr[i]);
							}

							if (_tcsicmp(TEXT("COMANDOS"), ttr) == 0) {
								_tprintf(TEXT("************** LISTAGEM DE COMANDOS **************\n"));
								_tprintf(TEXT("'VELOCIDADE' --> Mudar a Velocidade dos Brindes\n"));
								_tprintf(TEXT("'DURACAO' --> Mudar a direcao dos Brindes\n"));
								_tprintf(TEXT("'SPU' --> Probabilidade do brinde Speed-up\n"));
								_tprintf(TEXT("'SLD' --> Probabilidade do brinde Slow-down\n"));
								_tprintf(TEXT("'VE' --> Probabilidade do brinde Vida Extra\n"));
								_tprintf(TEXT("'TP' --> Probabilidade do brinde Triple\n"));
								_tprintf(TEXT("'BR' --> Probabilidade do brinde Barreira\n"));
								_tprintf(TEXT("'SAIR' --> Sair para o menu anterior\n"));
							}
							else if (_tcsicmp(TEXT("VELOCIDADE"), ttr) == 0) {
								_tprintf(TEXT("Velocidade: "));
								fflush(stdin);
								_fgetts(vttr, BUFFER_SIZE, stdin);

								vttr[_tcslen(vttr) - 1] = '\0';

								int aux_d = _ttoi(vttr);

								if (aux_d > 30 || aux_d <= 0) {
									_tprintf(TEXT("[ERRO] Insira um valor aceitavel! (nao negativo!)\n"));
								}
								else {
									for (int i = 0; i < MAX_NUM_BRINDES; i++) {
										msgJogo.brindes[i].velocidade = aux_d;
									}
									_tprintf(TEXT("Velocidade alterada para %d\n"), msgJogo.brindes[0].velocidade);
								}



							}
							else if (_tcsicmp(TEXT("DURACAO"), ttr) == 0) {
								_tprintf(TEXT("Duracao: "));
								fflush(stdin);
								_fgetts(vttr, BUFFER_SIZE, stdin);

								vttr[_tcslen(vttr) - 1] = '\0';

								int aux_d = _ttoi(vttr);

								if (aux_d > 200 || aux_d <= 0) {
									_tprintf(TEXT("[ERRO] Insira um valor aceitavel! (nao negativo!)\n"));
								}
								else {
									for (int i = 0; i < MAX_NUM_BRINDES; i++) {
										msgJogo.brindes[i].duracao = aux_d;
									}
									_tprintf(TEXT("Valor da duracao alterado para %d segundos\n"), msgJogo.brindes[0].duracao);
								}


							}
							else if (_tcsicmp(TEXT("SPU"), ttr) == 0) {
								_tprintf(TEXT("Speed-up: "));
								fflush(stdin);
								_fgetts(vttr, BUFFER_SIZE, stdin);

								vttr[_tcslen(vttr) - 1] = '\0';

								int aux_d = _ttoi(vttr);

								if (aux_d > 100 || aux_d < 0) {
									_tprintf(TEXT("[ERRO] Insira um valor aceitavel! (nao negativo!)\n"));
								}
								else {
									int aux_ppp;
									for (int i = 0; i < MAX_NUM_BRINDES; i++) {
										srand(NULL);
										aux_ppp = rand() % 100;

										if (aux_ppp <= aux_d) {
											msgJogo.brindes[i].tipo = speed_up;
										}

									}
									_tprintf(TEXT("Brindes speed-up  criados!\n"));
								}


							}
							else if (_tcsicmp(TEXT("SLD"), ttr) == 0) {
								_tprintf(TEXT("Slow-down: "));
								fflush(stdin);
								_fgetts(vttr, BUFFER_SIZE, stdin);

								vttr[_tcslen(vttr) - 1] = '\0';

								int aux_d = _ttoi(vttr);

								if (aux_d > 100 || aux_d < 0) {
									_tprintf(TEXT("[ERRO] Insira um valor aceitavel! (nao negativo!)\n"));
								}
								else {
									int aux_ppp;
									for (int i = 0; i < MAX_NUM_BRINDES; i++) {
										srand(NULL);
										aux_ppp = rand() % 100;

										if (aux_ppp <= aux_d) {
											msgJogo.brindes[i].tipo = slow_down;
										}

									}
									_tprintf(TEXT("Brindes slow-down criados!\n"));
								}


							}
							else if (_tcsicmp(TEXT("VE"), ttr) == 0) {
							_tprintf(TEXT("Vida-extra: "));
							fflush(stdin);
							_fgetts(vttr, BUFFER_SIZE, stdin);

							vttr[_tcslen(vttr) - 1] = '\0';

							int aux_d = _ttoi(vttr);

							if (aux_d > 100 || aux_d < 0) {
								_tprintf(TEXT("[ERRO] Insira um valor aceitavel! (nao negativo!)\n"));
							}
							else {
								int aux_ppp;
								for (int i = 0; i < MAX_NUM_BRINDES; i++) {
									srand(NULL);
									aux_ppp = rand() % 100;

									if (aux_ppp <= aux_d) {
										msgJogo.brindes[i].tipo = vida_extra;
									}

								}
								_tprintf(TEXT("Brindes speed-up  criados!\n"));
							}


							}
							else if (_tcsicmp(TEXT("TP"), ttr) == 0) {
							_tprintf(TEXT("Triple: "));
							fflush(stdin);
							_fgetts(vttr, BUFFER_SIZE, stdin);

							vttr[_tcslen(vttr) - 1] = '\0';

							int aux_d = _ttoi(vttr);

							if (aux_d > 100 || aux_d < 0) {
								_tprintf(TEXT("[ERRO] Insira um valor aceitavel! (nao negativo!)\n"));
							}
							else {
								int aux_ppp;
								for (int i = 0; i < MAX_NUM_BRINDES; i++) {
									srand(NULL);
									aux_ppp = rand() % 100;

									if (aux_ppp <= aux_d) {
										msgJogo.brindes[i].tipo = triple;
									}

								}
								_tprintf(TEXT("Brindes triple criados!\n"));
							}


							}
							else if (_tcsicmp(TEXT("BR"), ttr) == 0) {
							_tprintf(TEXT("Barreira: "));
							fflush(stdin);
							_fgetts(vttr, BUFFER_SIZE, stdin);

							vttr[_tcslen(vttr) - 1] = '\0';

							int aux_d = _ttoi(vttr);

							if (aux_d > 100 || aux_d < 0) {
								_tprintf(TEXT("[ERRO] Insira um valor aceitavel! (nao negativo!)\n"));
							}
							else {
								int aux_ppp;
								for (int i = 0; i < MAX_NUM_BRINDES; i++) {
									srand(NULL);
									aux_ppp = rand() % 100;

									if (aux_ppp <= aux_d) {
										msgJogo.brindes[i].tipo = barreira;
									}

								}
								_tprintf(TEXT("Brindes barreira criados!\n"));
							}


							}
							else {
								_tprintf(TEXT("Comando nao reconhecido! (Digite: 'COMANDOS' para ajuda)\n"));
							}
						} while (_tcsicmp(TEXT("SAIR"), ttr));
					}
					else {
						_tprintf(TEXT("Comando nao reconhecido! (Digite: 'COMANDOS' para ajuda)\n"));
					}


				} while (_tcsicmp(TEXT("SAIR"), ctr));




				/*******************************************/
			}
		}
		else if (_tcsicmp(TEXT("TOP"), str) == 0) {
			_tprintf(TEXT("********* TOP 10 **********\n"));
			for (int i = 0; i < 10; i++) {
				_tprintf(TEXT("NOME: %s \t SCORE: %d\n"), score.jogadores[i].nome, score.jogadores[i].pontos);
			}

		}
		else if (_tcsicmp(TEXT("MOVER"), str) == 0) {
			if (jogo == true) {
				//Lancar thread para mexer tijolos
				for (int i = (MAX_NUM_TIJOLOS / MAX_NUM_TIJOLOS_LINHA - 1) * MAX_NUM_TIJOLOS_LINHA; i < MAX_NUM_TIJOLOS; i++) {
					INT_PTR aux = i;
					thread_move_tijolos = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)moveTijolos, reinterpret_cast<LPVOID>(aux), 0, NULL);
				}


			}
			else {
				_tprintf(TEXT("O jogo precisa de estar a correr!\n"));
			}

		}
		else {
			_tprintf(TEXT("Comando nao reconhecido! (Digite: 'COMANDOS' para ajuda)\n"));
		}


	} while (_tcsicmp(TEXT("SAIR"), str));

	acabar = 1;

	CloseHandle(thread_read_msg_memory);
	CloseHandle(thread_write_msg_memory);
	CloseHandle(thread_bola);
	CloseHandle(eventoComeco);
	CloseHandle(eventoMemoria);

	_tprintf(TEXT("*******************Servidor Desligado!****************************\n"));
	UnmapViewOfFile(memoriaPartilhadaServidor.sharedJogo);
	system("pause");
	return 0;
}

//Outras Funções
void trataComando(COMANDO_SHARED comando) {
	int id = 0;
	Player aux;
	//_tprintf(TEXT("COMANDO: %d\n"), comando.idHandle);


	if (comando.tipo != CMD_LOGIN) {
		id = getIdPlayer(comando.idHandle);
		aux = getPlayer(id);
		//	_tprintf(TEXT("id: %d\n"), id);
			//_tprintf(TEXT("handdle: %d\n"), aux.idHandle);
	}

	//Outros tipos de comando

	switch (comando.tipo) {
	case CMD_LOGIN:
		for (int i = 0; i < MAX_NUM_PLAYERS; i++) {
			if (comando.idHandle == msgJogo.players[i].idHandle) {
				_tprintf(TEXT("Utilizador ja se encontra Logado!\n"));
				return;
			}
		}

		_tprintf(TEXT("Novo Utilizador Logado!\n"));
		inserePlayerJogo(comando.idHandle, comando);
		loginPlayer = TRUE;
		break;
	case CMD_MOVE_DIR:
		moveJogadorDireita(id);
		break;
	case CMD_MOVE_ESQ:
		moveJogadorEsquerda(id);
		break;
	case CMD_LOGOUT:
		meteTop(id);
		desconectaPlayer(id);
		_tprintf(TEXT("Cliente [%d] desconectado!\n"), id);
		break;
	case CMD_REGISTRY:
		readRegistry();
		msgJogo.ranking = score;
		break;
	}

	return;
}


/******************************************************************** MOVIMENTO JOGADOR *****************************************************/

void moveJogadorDireita(int idUser) {
	//Verificar se pode mover para a direita, e não colide com outros utilizadores (EIXO DOS X)
	WaitForSingleObject(mutex_player, INFINITE);
	for (int i = 0; i < MAX_NUM_PLAYERS; i++) {
		if (msgJogo.players[i].id == idUser && checkDireita(idUser)) {
			msgJogo.players[i].barreira.coord.X += msgJogo.players[i].barreira.velocidade; //Outro movimento variavel
			ReleaseMutex(mutex_player);
			return;
		}
	}
	ReleaseMutex(mutex_player);

}

void moveJogadorEsquerda(int idUser) {
	//Verificar se pode mover para a direita e não colide com outros utilizadores (EIXO DOS X)
	WaitForSingleObject(mutex_player, INFINITE);
	for (int i = 0; i < MAX_NUM_PLAYERS; i++) {
		if (msgJogo.players[i].id == idUser && checkEsquerda(idUser)) {
			msgJogo.players[i].barreira.coord.X -= msgJogo.players[i].barreira.velocidade; //Outro movimento variavel
			ReleaseMutex(mutex_player);
			return;
		}
	}
	ReleaseMutex(mutex_player);
}


/************************************************************* CHECKA SE PODE MOVER **************************************************/

BOOL checkDireita(int idUser) {
	//Fazer verificação nos eixo dos x, se alguma barreira está lá, Limites do mapa
	Player aux = getPlayer(idUser);

	if (aux.id == -1)
		return false;

	INT x = aux.barreira.coord.X + aux.barreira.dimensao;
	INT y = aux.barreira.coord.Y + ALT_BARREIRA;

	if (x >= LIMITE_DIREITO) {
		return false;
	}


	for (int i = 0; i < MAX_NUM_PLAYERS; i++) {
		if (msgJogo.players[i].idHandle != INVALID_HANDLE_VALUE) {
			if (msgJogo.players[i].barreira.ativa == 1) {
				if (aux.id != msgJogo.players[i].id) {
					if (x + aux.barreira.velocidade >= msgJogo.players[i].barreira.coord.X && x + aux.barreira.velocidade <= msgJogo.players[i].barreira.coord.X + msgJogo.players[i].barreira.dimensao && y - ALT_BARREIRA == msgJogo.players[i].barreira.coord.Y) {
						return false;
					}
				}
			}

		}


	}

	return true;
}

BOOL checkEsquerda(int idUser) {
	//Fazer verificação nos eixo dos x, se alguma barreira está lá, Limites do mapa
	Player aux = getPlayer(idUser);

	if (aux.id == -1)
		return false;

	INT x = aux.barreira.coord.X;
	INT y = aux.barreira.coord.Y + ALT_BARREIRA;

	if (x <= LIMITE_ESQUERDO) {
		return false;
	}


	for (int i = 0; i < MAX_NUM_PLAYERS; i++) {
		if (msgJogo.players[i].idHandle != INVALID_HANDLE_VALUE) {
			if (msgJogo.players[i].barreira.ativa) {
				if (aux.id != msgJogo.players[i].id) {

					if (x - aux.barreira.velocidade <= msgJogo.players[i].barreira.coord.X + msgJogo.players[i].barreira.dimensao && x - aux.barreira.velocidade >= msgJogo.players[i].barreira.coord.X && y - ALT_BARREIRA == msgJogo.players[i].barreira.coord.Y) {
						return false;
					}
				}
			}
		}

	}

	return true;
}


/**************************************************************************************************************************************/

/*********************************************************************** THREADS *******************************************************/
DWORD WINAPI readMensagemMemory(void) {
	//_tprintf(TEXT("Comecei Thread ler mensagem!\n"));
	while (1) {
		readMensagem(&memoriaPartilhadaServidor, &comandoLido); //função no DLL
	//	_tprintf(TEXT("Comando LIDO: %d! \n"), comandoLido.idHandle);
		trataComando(comandoLido);
	}
	return 0;
}

DWORD WINAPI writeMensagemMemory(void) {
	do {
		CopyMemory(&memoriaPartilhadaServidor.sharedJogo->jogo, &msgJogo, sizeof(MensagemJogo)); //por no dll
	} while (acabar == 0);

	return 0;
}


DWORD WINAPI moveTijolos(LPVOID p) {
	INT_PTR id = reinterpret_cast<INT_PTR>(p);
	Sleep(30);
	do {
		if (msgJogo.tijolos[id].vida > 0) {
			do {
				msgJogo.tijolos[id].coord.X += 1;
				Sleep(20);

			} while (msgJogo.tijolos[id].coord.X + msgJogo.tijolos[id].dimensao <= LIMITE_DIREITO - 15);


			for (int i = 0; i < ALT_TIJOLO + 10; i++) {
				msgJogo.tijolos[id].coord.Y += 1;
				Sleep(20);
			}

			do {
				msgJogo.tijolos[id].coord.X -= 1;
				Sleep(20);
			} while (msgJogo.tijolos[id].coord.X >= LIMITE_ESQUERDO + 15);

			for (int i = 0; i < ALT_TIJOLO + 10; i++) {
				msgJogo.tijolos[id].coord.Y -= 1;
				Sleep(20);
			}

		}
		else {
			return 0;
		}
	} while (jogo == true);

	return 0;
}

DWORD WINAPI controlaBola(LPVOID p) { 
	INT_PTR id = reinterpret_cast<INT_PTR>(p);
	bool semvidas = false;
	bool acabou = true;

	msgJogo.bolas[id].velocidade = msgJogo.bolas[id].velocidade_inicial;
	msgJogo.bolas[id].ativa = 1;
	do {
		//MOVIMENTO
		if (msgJogo.bolas[id].cima) { //se mover para cima

			if (msgJogo.bolas[id].direita) { //Se mover para a direita
				msgJogo.bolas[id].coord.X += msgJogo.bolas[id].velocidade;
				msgJogo.bolas[id].coord.Y -= msgJogo.bolas[id].velocidade;
			}
			else {
				msgJogo.bolas[id].coord.X -= msgJogo.bolas[id].velocidade;
				msgJogo.bolas[id].coord.Y -= msgJogo.bolas[id].velocidade;
			}


		}
		else { // Se mover para baixo
			if (msgJogo.bolas[id].direita) { //Se mover para a direita
				msgJogo.bolas[id].coord.X += msgJogo.bolas[id].velocidade;
				msgJogo.bolas[id].coord.Y += msgJogo.bolas[id].velocidade;
			}
			else {
				msgJogo.bolas[id].coord.X -= msgJogo.bolas[id].velocidade;
				msgJogo.bolas[id].coord.Y += msgJogo.bolas[id].velocidade;
			}

		}


		if (msgJogo.bolas[id].coord.Y <= LIMITE_SUPERIOR) { // se bater no LIMITE SUPERIOR INVERTER A DIREÇÃO DA BOLA
			msgJogo.bolas[id].cima = false;
		}
		else if (msgJogo.bolas[id].coord.X >= LIMITE_DIREITO) {
			msgJogo.bolas[id].direita = false;
		}
		else if (msgJogo.bolas[id].coord.X <= LIMITE_ESQUERDO) {
			msgJogo.bolas[id].direita = true;
		}


		//Verificar se bate na barreira
		for (int i = 0; i < MAX_NUM_PLAYERS; i++) {
			if (msgJogo.players[i].idHandle != INVALID_HANDLE_VALUE) //Ver se está tudo bem, porque estou a comprar só um raio!
				if (msgJogo.bolas[id].coord.Y + msgJogo.bolas[id].raio / 2 >= (msgJogo.players[i].barreira.coord.Y - ALT_BARREIRA) && msgJogo.bolas[id].coord.X + msgJogo.bolas[id].raio >= msgJogo.players[i].barreira.coord.X && msgJogo.bolas[id].coord.X <= msgJogo.players[i].barreira.coord.X + msgJogo.players[i].barreira.dimensao) {
					msgJogo.bolas[id].cima = true;
					msgJogo.bolas[id].jogador = msgJogo.players[i].id;
				}
		}


		//Verificar se embate nos tijolos e se bater retirar tijolo
		for (int i = 0; i < MAX_NUM_TIJOLOS; i++) {
			if (msgJogo.tijolos[i].vida > 0) {
				if (msgJogo.bolas[id].coord.Y - msgJogo.bolas[id].raio <= msgJogo.tijolos[i].coord.Y + ALT_TIJOLO && msgJogo.bolas[id].coord.Y + msgJogo.bolas[id].raio >= msgJogo.tijolos[i].coord.Y &&
					msgJogo.bolas[id].coord.X - msgJogo.bolas[id].raio <= msgJogo.tijolos[i].coord.X + LARG_TIJOLO && msgJogo.bolas[id].coord.X + msgJogo.bolas[id].raio >= msgJogo.tijolos[i].coord.X) {


					//Adicionar pontos ao jogador
					for (int x = 0; x < MAX_NUM_PLAYERS; x++) {
						if (msgJogo.players[x].idHandle != INVALID_HANDLE_VALUE) {
							if (msgJogo.players[x].id == msgJogo.bolas[id].jogador) {
								msgJogo.players[x].pontos += 1;
								x = MAX_NUM_PLAYERS + 1;
							}


						}

					}


					//verificar para que lado do tijolo embate
					if (msgJogo.bolas[id].cima) {
						if (!msgJogo.bolas[id].direita) { //para cima e para a esquerda

							if (msgJogo.tijolos[i].coord.X + LARG_TIJOLO - msgJogo.bolas[id].coord.X > msgJogo.tijolos[i].coord.Y + ALT_TIJOLO - msgJogo.bolas[id].coord.Y) {
								msgJogo.bolas[id].cima = false;
							}
							else {
								msgJogo.bolas[id].direita = true;
							}

						}
						else { //Cima direita

							if (msgJogo.bolas[id].coord.X - msgJogo.tijolos[i].coord.X > msgJogo.tijolos[i].coord.Y + ALT_TIJOLO - msgJogo.bolas[id].coord.Y) {
								msgJogo.bolas[id].cima = false;
							}
							else {
								msgJogo.bolas[id].direita = false;
							}


						}
					}
					else {

						if (!msgJogo.bolas[id].direita) { //para baixo e para a esquerda

							if (msgJogo.tijolos[i].coord.X + LARG_TIJOLO - msgJogo.bolas[id].coord.X > msgJogo.bolas[id].coord.Y - msgJogo.tijolos[i].coord.Y) {
								msgJogo.bolas[id].cima = true;
							}
							else {
								msgJogo.bolas[id].direita = true;
							}

						}
						else { //Para baixo e para a direita

							if (msgJogo.bolas[id].coord.X - msgJogo.tijolos[i].coord.X > msgJogo.bolas[id].coord.Y - msgJogo.tijolos[i].coord.Y) {
								msgJogo.bolas[id].cima = true;
							}
							else {
								msgJogo.bolas[id].direita = false;
							}


						}

					}

					msgJogo.tijolos[i].vida--;
					if (msgJogo.tijolos[i].vida <= 0) { //CAIR BRINDE AO FAZER ISTO !!!

						if (msgJogo.tijolos[i].tipo == magico) {

							for (int x = 0; x < MAX_NUM_BRINDES; x++) {
								if (msgJogo.brindes[x].ativo == 0) {

									//Se for preciso, configurar mais os brindes

									msgJogo.brindes[x].coord.X = msgJogo.tijolos[i].coord.X;
									msgJogo.brindes[x].coord.Y = msgJogo.tijolos[i].coord.Y;

									INT_PTR aux = x;

									thread_brinde = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)controlaBrinde, reinterpret_cast<LPVOID>(aux), 0, NULL);

									x = MAX_NUM_BRINDES + 1;
								}
							}

						}



						msgJogo.tijolos[i].coord.X = -30;
						msgJogo.tijolos[i].coord.Y = -30;



					}
				}


			}
		}


		//Verificar se bate contra o brinde
		for (int i = 0; i < MAX_NUM_BRINDES; i++) {
			if (msgJogo.brindes[i].ativo != 0) {
				if (msgJogo.bolas[id].coord.Y - msgJogo.bolas[id].raio <= msgJogo.brindes[i].coord.Y + ALT_BRINDE && msgJogo.bolas[id].coord.Y + msgJogo.bolas[id].raio >= msgJogo.brindes[i].coord.Y &&
					msgJogo.bolas[id].coord.X - msgJogo.bolas[id].raio <= msgJogo.brindes[i].coord.X + msgJogo.brindes[i].dimensao && msgJogo.bolas[id].coord.X + msgJogo.bolas[id].raio >= msgJogo.brindes[i].coord.X) {

					//verificar para que lado do tijolo embate
					if (msgJogo.bolas[id].cima) {
						if (!msgJogo.bolas[id].direita) { //para cima e para a esquerda

							if (msgJogo.brindes[i].coord.X + msgJogo.brindes[i].dimensao - msgJogo.bolas[id].coord.X > msgJogo.brindes[i].coord.Y + ALT_BRINDE - msgJogo.bolas[id].coord.Y) {
								msgJogo.bolas[id].cima = false;
							}
							else {
								msgJogo.bolas[id].direita = true;
							}

						}
						else { //Cima direita

							if (msgJogo.bolas[id].coord.X - msgJogo.brindes[i].coord.X > msgJogo.brindes[i].coord.Y + ALT_BRINDE - msgJogo.bolas[id].coord.Y) {
								msgJogo.bolas[id].cima = false;
							}
							else {
								msgJogo.bolas[id].direita = false;
							}


						}
					}
					else {

						if (!msgJogo.bolas[id].direita) { //para baixo e para a esquerda

							if (msgJogo.brindes[i].coord.X + msgJogo.brindes[i].dimensao - msgJogo.bolas[id].coord.X > msgJogo.bolas[id].coord.Y - msgJogo.brindes[i].coord.Y) {
								msgJogo.bolas[id].cima = true;
							}
							else {
								msgJogo.bolas[id].direita = true;
							}

						}
						else { //Para baixo e para a direita

							if (msgJogo.bolas[id].coord.X - msgJogo.brindes[i].coord.X > msgJogo.bolas[id].coord.Y - msgJogo.brindes[i].coord.Y) {
								msgJogo.bolas[id].cima = true;
							}
							else {
								msgJogo.bolas[id].direita = false;
							}


						}

					}

				}


			}
		}




		/*****************************************************************************************************************/


		//Se a bola sair dos limites
		if (msgJogo.bolas[id].coord.Y >= LIMITE_INFERIOR) { // SE A BOLA PASSAR O LIMITE INFERIOR
			msgJogo.bolas[id].ativa = 0;
			msgJogo.bolas[id].coord.Y = -30;
			msgJogo.bolas[id].coord.X = -30;

			for (int i = 0; i < MAX_NUM_PLAYERS; i++) {
				if (msgJogo.players[i].idHandle != INVALID_HANDLE_VALUE)
					if (msgJogo.players[i].barreira.ativa)
						msgJogo.players[i].vidas--;
			}
		}


		//Se um jogador ficou sem vidas, tirar a barra
		for (int i = 0; i < MAX_NUM_PLAYERS; i++) {
			if (msgJogo.players[i].idHandle != INVALID_HANDLE_VALUE) {
				if (msgJogo.players[i].vidas <= 0) {
					if (msgJogo.players[i].barreira.ativa != 0) {
						msgJogo.players[i].barreira.ativa = 0;
						msgJogo.players[i].barreira.coord.X = -30;
						msgJogo.players[i].barreira.coord.Y = -30;
						meteTop(msgJogo.players[i].id);
					}
				}
			}
		}


		acabou = true;
		//Se acabar os tijolos
		for (int i = 0; i < MAX_NUM_TIJOLOS; i++) {
			if (msgJogo.tijolos[i].vida > 0) {
				acabou = false;
			}
		}


		//Se acabar as vidas dos jogadores
		semvidas = true;
		for (int i = 0; i < MAX_NUM_PLAYERS; i++) {
			if (msgJogo.players[i].idHandle != INVALID_HANDLE_VALUE) {
				if (msgJogo.players[i].barreira.ativa)
					if (msgJogo.players[i].vidas > 0) {
						semvidas = false;
						break;
					}
			}
		}



		if (acabou || semvidas) { //Acabar com as bolas todas
			for (int i = 0; i < MAX_NUM_BOLAS; i++) {
				msgJogo.bolas[i].ativa = 0;
				msgJogo.bolas[i].coord.X = -30;
				msgJogo.bolas[i].coord.Y = -30;
				jogo = false;
			}
		}




		Sleep(5);

	} while (msgJogo.bolas[id].ativa);

	Sleep(200);
	bool check = false;
	//Verificar se tem bolas a correr
	for (int i = 0; i < MAX_NUM_BOLAS; i++) {
		if (msgJogo.bolas[i].ativa == 1) {
			check = true;
			break;
		}
	}

	if (!check && !acabou && !semvidas) {
		//lancar nova bola
		msgJogo.bolas[0].cima = true;
		msgJogo.bolas[0].direita = rand() % 2;
		msgJogo.bolas[0].coord.X = LIMITE_ESQUERDO + (rand() % LIMITE_DIREITO);
		msgJogo.bolas[0].coord.Y = LIMITE_INFERIOR - 20;
		INT_PTR aux = 0;
		thread_bola = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)controlaBola, reinterpret_cast<LPVOID>(aux), 0, NULL);
	}


	return 0;
}

DWORD WINAPI controlaBrinde(LPVOID p) {
	INT_PTR id = reinterpret_cast<INT_PTR>(p);
	bool acabou = false;
	int aux;
	int numero;
	msgJogo.brindes[id].ativo = 1;
	//_tprintf(TEXT("Funcao do brind evocada!\nBrinde do tipo: %d\n\n\n\n"), msgJogo.brindes[id].tipo);
	do {

		msgJogo.brindes[id].coord.Y += msgJogo.brindes[id].velocidade;
		//_tprintf(TEXT("COORDENADA Y: %d\n"), msgJogo.brindes[id].coord.Y);
		//_tprintf(TEXT("Funcao do brind evocada!\n"));
		Sleep(5);

		for (int i = 0; i < MAX_NUM_PLAYERS; i++) {
			if (msgJogo.players[i].idHandle != INVALID_HANDLE_VALUE) { //Verificar se os brindes terão espaço de lado
				if (msgJogo.brindes[id].coord.Y + ALT_BRINDE >= msgJogo.players[i].barreira.coord.Y &&
					msgJogo.brindes[id].coord.X + msgJogo.brindes[id].dimensao >= msgJogo.players[i].barreira.coord.X &&
					msgJogo.brindes[id].coord.X <= msgJogo.players[i].barreira.coord.X + msgJogo.players[i].barreira.dimensao) {

					//Verificar se hita em alguma barreira existente se sim dar lhe o power-up
		//enum Tipo_Brinde { speed_up, slow_down, vida_extra, triple, barreira }; //Adicionar outros brindes consoante a originalidade
					switch (msgJogo.brindes[id].tipo) {
					case speed_up:
						//_tprintf(TEXT("Speed-up!\n"));

						msgJogo.brindes[id].ativo = 0;
						msgJogo.brindes[id].coord.Y = -30;
						msgJogo.brindes[id].coord.X = -30;

						for (int j = 0; j < MAX_NUM_BOLAS; j++) {				

						if (msgJogo.bolas[j].velocidade * 1.20 > msgJogo.bolas[j].velocidade_inicial * 2) {
							//_tprintf(TEXT("DADA\n\n"));
						}
						else {
							msgJogo.bolas[j].velocidade = msgJogo.bolas[j].velocidade * 1.20;

								Sleep(msgJogo.brindes[id].duracao * 1000);

							msgJogo.bolas[j].velocidade -= msgJogo.bolas[j].velocidade_inicial * 0.2;

						}

						}
						i = MAX_NUM_PLAYERS + 1;
						acabou = true;
						break;
					case slow_down:
						//_tprintf(TEXT("Slowdown!\n")); //VELOCIDADE DA BLA
						//PReciso de meter a duração, lancar uma thread com temporizador
						msgJogo.brindes[id].ativo = 0;
						msgJogo.brindes[id].coord.Y = -30;
						msgJogo.brindes[id].coord.X = -30;


						for (int j = 0; j < MAX_NUM_BOLAS; j++) {
					
						if (msgJogo.bolas[j].velocidade - msgJogo.bolas[j].velocidade * 0.20 <= msgJogo.bolas[j].velocidade_inicial * 0.60) {
						
						}
						else {
							msgJogo.bolas[j].velocidade = msgJogo.bolas[j].velocidade - msgJogo.bolas[j].velocidade * 0.20;


								Sleep(msgJogo.brindes[id].duracao * 1000);


							msgJogo.bolas[j].velocidade += msgJogo.bolas[j].velocidade_inicial * 0.2;
						}
						}
						i = MAX_NUM_PLAYERS + 1;
						acabou = true;
						break;
					case vida_extra:
						//	_tprintf(TEXT("Vida Extra!\n"));
						msgJogo.players[i].vidas += 1;
						i = MAX_NUM_PLAYERS + 1;
						msgJogo.brindes[id].ativo = 0;
						msgJogo.brindes[id].coord.Y = -30;
						msgJogo.brindes[id].coord.X = -30;
						acabou = true;
						break;
					case triple:
						INT_PTR outra;
						numero = 2;
						for (int o = 0; o < MAX_NUM_BOLAS; o++) {
							if (msgJogo.bolas[o].ativa == 0 && numero > 0) {
								outra = o;
								thread_bola = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)controlaBola, reinterpret_cast<LPVOID>(outra), 0, NULL);
								numero--;
							}
						}

						i = MAX_NUM_PLAYERS + 1;
						msgJogo.brindes[id].ativo = 0;
						msgJogo.brindes[id].coord.Y = -30;
						msgJogo.brindes[id].coord.X = -30;

						acabou = true;


						//edada
					//	_tprintf(TEXT("BARREIRA\n"));
						break;
					case barreira:
						//	_tprintf(TEXT("BRINDE BARREIRA!\n"));
						switch (rand() % 2) {
						case 0:
							msgJogo.players[i].barreira.dimensao += rand() % 20;
							break;
						case 1:
							msgJogo.players[i].barreira.dimensao -= rand() % 20;
							break;
						}

						if (msgJogo.players[i].barreira.dimensao <= 40) {
							msgJogo.players[i].barreira.dimensao += rand() % 20;
						}

						if (msgJogo.players[i].barreira.dimensao >= 175) {
							msgJogo.players[i].barreira.dimensao -= rand() % 50;
						}

						i = MAX_NUM_PLAYERS + 1;
						msgJogo.brindes[id].ativo = 0;
						msgJogo.brindes[id].coord.Y = -30;
						msgJogo.brindes[id].coord.X = -30;
						acabou = true;
						break;
					}


				}
			}
		}



		//Verificar se chegou aqui!
		if (msgJogo.brindes[id].coord.Y > LIMITE_INFERIOR) {
			//	_tprintf(TEXT("TRDADA\n"));
			msgJogo.brindes[id].ativo = 0;
			msgJogo.brindes[id].coord.Y = -30;
			msgJogo.brindes[id].coord.X = -30;
			acabou = true;
		}


	} while (!acabou);
	return 0;
}

/**************************************************************************************************************************************/


/********************************************************************* CONFIGURAÇÃO DO JOGO ********************************************/

//Configuração inicial do MAPA
void inicia_mapa() {

	srand(time(NULL));
	//BOLA
	for (int i = 0; i < MAX_NUM_BOLAS; i++) {
		msgJogo.bolas[i].ativa = 0;
		msgJogo.bolas[i].coord.X = LIMITE_ESQUERDO + (rand() % LIMITE_DIREITO);
		msgJogo.bolas[i].coord.Y = LIMITE_INFERIOR - 20;
		msgJogo.bolas[i].cima = true;
		switch (rand() % 2) {
		case 0:
			msgJogo.bolas[i].direita = true;
			break;
		case 1:
			msgJogo.bolas[i].direita = false;
		}
		msgJogo.bolas[i].velocidade = 1;
		msgJogo.bolas[i].velocidade_inicial = 1;
		msgJogo.bolas[i].raio = 5;

	}

	//Barreira
	for (int i = 0; i < MAX_NUM_PLAYERS; i++) {
		msgJogo.players[i].barreira.ativa = 0;
		msgJogo.players[i].barreira.id = -1;
		msgJogo.players[i].barreira.dimensao = 120; //ainda  verificar
		msgJogo.players[i].barreira.coord.X = -20;
		msgJogo.players[i].barreira.coord.Y = -20;
		msgJogo.players[i].barreira.velocidade = 15;


		msgJogo.players[i].id = -1;
		msgJogo.players[i].pontos = 0;
		msgJogo.players[i].idHandle = INVALID_HANDLE_VALUE;
		msgJogo.players[i].vidas = 3;
	}

	//Tijolos  Tipo_Tijolo{normal, resistente, magico};
	for (int i = 0; i < MAX_NUM_TIJOLOS; i++) {
		msgJogo.tijolos[i].id = idTijolo++;
		msgJogo.tijolos[i].vida = 1;
		msgJogo.tijolos[i].coord.X = LIMITE_SUPERIOR + 35 + ((i % MAX_NUM_TIJOLOS_LINHA) * (LARG_TIJOLO + 10)); //Secalhar mudar porque o tijolo não é um quadrado, mas para começar ;)
		msgJogo.tijolos[i].coord.Y = LIMITE_ESQUERDO + 20 + ((i / MAX_NUM_TIJOLOS_LINHA) * (ALT_TIJOLO + 20));

		switch (rand() % 3)
		{
		case 0:
			msgJogo.tijolos[i].tipo = normal;
			break;
		case 1:
			msgJogo.tijolos[i].tipo = resistente;
			msgJogo.tijolos[i].vida = 2 + rand() % 3;
			break;
		case 2:
			msgJogo.tijolos[i].tipo = magico;
			break;
		default:
			msgJogo.tijolos[i].tipo = normal;
			break;
		}

		//_tprintf(TEXT("TIPO: %d\n"), msgJogo.tijolos[i].tipo);
		//_tprintf(TEXT("Tijolo Colocado %d na posicao: x = %d, y = %d\n"), msgJogo.tijolos[i].id, msgJogo.tijolos[i].coord.X, msgJogo.tijolos[i].coord.Y);
	}

	//brindes enum Tipo_Brinde { speed_up, slow_down, vida_extra, triple, barreira }; //Adicionar outros brindes consoante a originalidade
	for (int i = 0; i < MAX_NUM_BRINDES; i++) {
		msgJogo.brindes[i].id = 1;
		msgJogo.brindes[i].ativo = 0;
		msgJogo.brindes[i].dimensao = 2 * msgJogo.bolas[0].raio;
		msgJogo.brindes[i].duracao = 10; //EM SEGUNDOS

		switch (rand() % 4)
		{
		case 0:
			msgJogo.brindes[i].tipo = speed_up;
			break;
		case 1:
			msgJogo.brindes[i].tipo = slow_down;
			break;
		case 2:
			msgJogo.brindes[i].tipo = vida_extra;
			break;
		case 3:
			msgJogo.brindes[i].tipo = triple;
			break;
		default:
			msgJogo.brindes[i].tipo = speed_up;
			break;
		}
		//_tprintf(TEXT("Brinde criado! %d\n"), msgJogo.brindes[i].tipo);

		msgJogo.brindes[i].velocidade = 1;
	}
}

void inicia_configuracao_jogo() {
	srand(time(NULL));
	//BOLA
	for (int i = 0; i < MAX_NUM_BOLAS; i++) {
		msgJogo.bolas[i].ativa = 0;
		msgJogo.bolas[i].coord.X = LIMITE_ESQUERDO + (rand() % LIMITE_DIREITO);
		msgJogo.bolas[i].coord.Y = LIMITE_INFERIOR - 20;
		msgJogo.bolas[i].cima = true;
		switch (rand() % 2) {
		case 0:
			msgJogo.bolas[i].direita = true;
			break;
		case 1:
			msgJogo.bolas[i].direita = false;
		}
		msgJogo.bolas[i].velocidade = 1;
		msgJogo.bolas[i].raio = 5;
		msgJogo.bolas[i].velocidade_inicial = 1;

	}

	//Barreira
	for (int i = 0; i < MAX_NUM_PLAYERS; i++) {
		msgJogo.players[i].barreira.ativa = 0;
		msgJogo.players[i].barreira.id = -1;
		msgJogo.players[i].barreira.dimensao = 120; //ainda  verificar
		msgJogo.players[i].barreira.coord.X = -20;
		msgJogo.players[i].barreira.coord.Y = -20;
		msgJogo.players[i].barreira.velocidade = 15;


		msgJogo.players[i].pontos = 0;
		msgJogo.players[i].vidas = 3;
	}

	//Tijolos  Tipo_Tijolo{normal, resistente, magico};
	for (int i = 0; i < MAX_NUM_TIJOLOS; i++) {
		msgJogo.tijolos[i].id = idTijolo++;
		msgJogo.tijolos[i].vida = 1;
		msgJogo.tijolos[i].coord.X = LIMITE_SUPERIOR + 35 + ((i % MAX_NUM_TIJOLOS_LINHA) * (LARG_TIJOLO + 10)); //Secalhar mudar porque o tijolo não é um quadrado, mas para começar ;)
		msgJogo.tijolos[i].coord.Y = LIMITE_ESQUERDO + 20 + ((i / MAX_NUM_TIJOLOS_LINHA) * (ALT_TIJOLO + 20));

		switch (rand() % 3)
		{
		case 0:
			msgJogo.tijolos[i].tipo = normal;
			break;
		case 1:
			msgJogo.tijolos[i].tipo = resistente;
			msgJogo.tijolos[i].vida = 2 + rand() % 3;
			break;
		case 2:
			msgJogo.tijolos[i].tipo = magico;
			break;
		default:
			msgJogo.tijolos[i].tipo = normal;
			break;
		}

		//_tprintf(TEXT("TIPO: %d\n"), msgJogo.tijolos[i].tipo);
		//_tprintf(TEXT("Tijolo Colocado %d na posicao: x = %d, y = %d\n"), msgJogo.tijolos[i].id, msgJogo.tijolos[i].coord.X, msgJogo.tijolos[i].coord.Y);
	}

	//brindes enum Tipo_Brinde { speed_up, slow_down, vida_extra, triple, barreira }; //Adicionar outros brindes consoante a originalidade
	for (int i = 0; i < MAX_NUM_BRINDES; i++) {
		msgJogo.brindes[i].id = 1;
		msgJogo.brindes[i].ativo = 0;
		msgJogo.brindes[i].dimensao = 2 * msgJogo.bolas[0].raio;
		msgJogo.brindes[i].duracao = 10; //EM SEGUNDOS

		switch (rand() % 5)
		{
		case 0:
			msgJogo.brindes[i].tipo = speed_up;
			break;
		case 1:
			msgJogo.brindes[i].tipo = slow_down;
			break;
		case 2:
			msgJogo.brindes[i].tipo = vida_extra;
			break;
		case 3:
			msgJogo.brindes[i].tipo = triple;
			break;
		default:
			msgJogo.brindes[i].tipo = barreira;
			break;
		}
		//_tprintf(TEXT("Brinde criado! %d\n"), msgJogo.brindes[i].tipo);

		msgJogo.brindes[i].velocidade = 1;
	}
}

void inicia_configuracao_nivel(int aux) {
	srand(time(NULL));
	if (aux == 1) {

		//BOLA
		for (int i = 0; i < MAX_NUM_BOLAS; i++) {
			msgJogo.bolas[i].ativa = 0;
			msgJogo.bolas[i].coord.X = LIMITE_ESQUERDO + (rand() % LIMITE_DIREITO);
			msgJogo.bolas[i].coord.Y = LIMITE_INFERIOR - 20;
			msgJogo.bolas[i].cima = true;
			switch (rand() % 2) {
			case 0:
				msgJogo.bolas[i].direita = true;
				break;
			case 1:
				msgJogo.bolas[i].direita = false;
			}
			msgJogo.bolas[i].velocidade = 1;
			msgJogo.bolas[i].velocidade_inicial = 1;
			msgJogo.bolas[i].raio = 5;

		}

		//Barreira
		for (int i = 0; i < MAX_NUM_PLAYERS; i++) {
			msgJogo.players[i].barreira.ativa = 0;
			msgJogo.players[i].barreira.id = -1;
			msgJogo.players[i].barreira.dimensao = 120; //ainda  verificar
			msgJogo.players[i].barreira.coord.X = -20;
			msgJogo.players[i].barreira.coord.Y = -20;
			msgJogo.players[i].barreira.velocidade = 15;


			msgJogo.players[i].pontos = 0;
			msgJogo.players[i].vidas = 3;
		}

		//Tijolos  Tipo_Tijolo{normal, resistente, magico};
		for (int i = 0; i < MAX_NUM_TIJOLOS; i++) {
			msgJogo.tijolos[i].id = idTijolo++;
			msgJogo.tijolos[i].vida = 1;
			msgJogo.tijolos[i].coord.X = LIMITE_SUPERIOR + 35 + ((i % MAX_NUM_TIJOLOS_LINHA) * (LARG_TIJOLO + 10)); //Secalhar mudar porque o tijolo não é um quadrado, mas para começar ;)
			msgJogo.tijolos[i].coord.Y = LIMITE_ESQUERDO + 20 + ((i / MAX_NUM_TIJOLOS_LINHA) * (ALT_TIJOLO + 20));

			switch (rand() % 3)
			{
			case 0:
				msgJogo.tijolos[i].tipo = normal;
				break;
			case 1:
				msgJogo.tijolos[i].tipo = resistente;
				msgJogo.tijolos[i].vida = 2 + rand() % 3;
				break;
			case 2:
				msgJogo.tijolos[i].tipo = magico;
				break;
			default:
				msgJogo.tijolos[i].tipo = normal;
				break;
			}

			//_tprintf(TEXT("TIPO: %d\n"), msgJogo.tijolos[i].tipo);
			//_tprintf(TEXT("Tijolo Colocado %d na posicao: x = %d, y = %d\n"), msgJogo.tijolos[i].id, msgJogo.tijolos[i].coord.X, msgJogo.tijolos[i].coord.Y);
		}

		//brindes enum Tipo_Brinde { speed_up, slow_down, vida_extra, triple, barreira }; //Adicionar outros brindes consoante a originalidade
		for (int i = 0; i < MAX_NUM_BRINDES; i++) {
			msgJogo.brindes[i].id = 1;
			msgJogo.brindes[i].ativo = 0;
			msgJogo.brindes[i].dimensao = 2 * msgJogo.bolas[0].raio;
			msgJogo.brindes[i].duracao = 10; //EM SEGUNDOS

			switch (rand() % 4)
			{
			case 0:
				msgJogo.brindes[i].tipo = speed_up;
				break;
			case 1:
				msgJogo.brindes[i].tipo = slow_down;
				break;
			case 2:
				msgJogo.brindes[i].tipo = vida_extra;
				break;
			case 3:
				msgJogo.brindes[i].tipo = triple;
				break;
			default:
				msgJogo.brindes[i].tipo = speed_up;
				break;
			}
			//_tprintf(TEXT("Brinde criado! %d\n"), msgJogo.brindes[i].tipo);

			msgJogo.brindes[i].velocidade = 1;
		}
	}
	else if (aux == 2) {
		//BOLA
		for (int i = 0; i < MAX_NUM_BOLAS; i++) {
			msgJogo.bolas[i].ativa = 0;
			msgJogo.bolas[i].coord.X = LIMITE_ESQUERDO + (rand() % LIMITE_DIREITO);
			msgJogo.bolas[i].coord.Y = LIMITE_INFERIOR - 20;
			msgJogo.bolas[i].cima = true;
			switch (rand() % 2) {
			case 0:
				msgJogo.bolas[i].direita = true;
				break;
			case 1:
				msgJogo.bolas[i].direita = false;
			}
			msgJogo.bolas[i].velocidade = 1;
			msgJogo.bolas[i].velocidade_inicial = 1;
			msgJogo.bolas[i].raio = 5;

		}

		//Barreira
		for (int i = 0; i < MAX_NUM_PLAYERS; i++) {
			msgJogo.players[i].barreira.ativa = 0;
			msgJogo.players[i].barreira.id = -1;
			msgJogo.players[i].barreira.dimensao = 120; //ainda  verificar
			msgJogo.players[i].barreira.coord.X = -20;
			msgJogo.players[i].barreira.coord.Y = -20;
			msgJogo.players[i].barreira.velocidade = 15;


			msgJogo.players[i].pontos = 0;
			msgJogo.players[i].vidas = 3;
		}

		//Tijolos  Tipo_Tijolo{normal, resistente, magico};
		for (int i = 0; i < MAX_NUM_TIJOLOS; i++) {
			msgJogo.tijolos[i].id = idTijolo++;
			msgJogo.tijolos[i].vida = 1;
			msgJogo.tijolos[i].coord.X = LIMITE_SUPERIOR + 35 + ((i % MAX_NUM_TIJOLOS_LINHA) * (LARG_TIJOLO + 10)); //Secalhar mudar porque o tijolo não é um quadrado, mas para começar ;)
			msgJogo.tijolos[i].coord.Y = LIMITE_ESQUERDO + 20 + ((i / MAX_NUM_TIJOLOS_LINHA) * (ALT_TIJOLO + 20));


			msgJogo.tijolos[i].tipo = magico;

		}

		//_tprintf(TEXT("TIPO: %d\n"), msgJogo.tijolos[i].tipo);
		//_tprintf(TEXT("Tijolo Colocado %d na posicao: x = %d, y = %d\n"), msgJogo.tijolos[i].id, msgJogo.tijolos[i].coord.X, msgJogo.tijolos[i].coord.Y);


	//brindes enum Tipo_Brinde { speed_up, slow_down, vida_extra, triple, barreira }; //Adicionar outros brindes consoante a originalidade
		for (int i = 0; i < MAX_NUM_BRINDES; i++) {
			msgJogo.brindes[i].id = 1;
			msgJogo.brindes[i].ativo = 0;
			msgJogo.brindes[i].dimensao = 2 * msgJogo.bolas[0].raio;
			msgJogo.brindes[i].duracao = 10; //EM SEGUNDOS

			switch (rand() % 4)
			{
			case 0:
				msgJogo.brindes[i].tipo = speed_up;
				break;
			case 1:
				msgJogo.brindes[i].tipo = slow_down;
				break;
			case 2:
				msgJogo.brindes[i].tipo = vida_extra;
				break;
			case 3:
				msgJogo.brindes[i].tipo = triple;
				break;
			default:
				msgJogo.brindes[i].tipo = speed_up;
				break;
			}
			//_tprintf(TEXT("Brinde criado! %d\n"), msgJogo.brindes[i].tipo);

			msgJogo.brindes[i].velocidade = 1;
		}
	}
	else if (aux == 3) {
		//BOLA
		for (int i = 0; i < MAX_NUM_BOLAS; i++) {
			msgJogo.bolas[i].ativa = 0;
			msgJogo.bolas[i].coord.X = LIMITE_ESQUERDO + (rand() % LIMITE_DIREITO);
			msgJogo.bolas[i].coord.Y = LIMITE_INFERIOR - 20;
			msgJogo.bolas[i].cima = true;
			switch (rand() % 2) {
			case 0:
				msgJogo.bolas[i].direita = true;
				break;
			case 1:
				msgJogo.bolas[i].direita = false;
			}
			msgJogo.bolas[i].velocidade = 1;
			msgJogo.bolas[i].velocidade_inicial = 1;
			msgJogo.bolas[i].raio = 5;

		}

		//Barreira
		for (int i = 0; i < MAX_NUM_PLAYERS; i++) {
			msgJogo.players[i].barreira.ativa = 0;
			msgJogo.players[i].barreira.id = -1;
			msgJogo.players[i].barreira.dimensao = 120; //ainda  verificar
			msgJogo.players[i].barreira.coord.X = -20;
			msgJogo.players[i].barreira.coord.Y = -20;
			msgJogo.players[i].barreira.velocidade = 15;

			msgJogo.players[i].pontos = 0;
			msgJogo.players[i].vidas = 3;
		}

		//Tijolos  Tipo_Tijolo{normal, resistente, magico};
		for (int i = 0; i < MAX_NUM_TIJOLOS; i++) {
			msgJogo.tijolos[i].id = idTijolo++;
			msgJogo.tijolos[i].vida = 1;
			msgJogo.tijolos[i].coord.X = LIMITE_SUPERIOR + 35 + ((i % MAX_NUM_TIJOLOS_LINHA) * (LARG_TIJOLO + 10)); //Secalhar mudar porque o tijolo não é um quadrado, mas para começar ;)
			msgJogo.tijolos[i].coord.Y = LIMITE_ESQUERDO + 20 + ((i / MAX_NUM_TIJOLOS_LINHA) * (ALT_TIJOLO + 20));

			switch (rand() % 2)
			{
			case 0:
				msgJogo.tijolos[i].tipo = magico;
				break;
			case 1:
				msgJogo.tijolos[i].tipo = resistente;
				msgJogo.tijolos[i].vida = 2 + rand() % 3;
				break;
			}

			//_tprintf(TEXT("TIPO: %d\n"), msgJogo.tijolos[i].tipo);
			//_tprintf(TEXT("Tijolo Colocado %d na posicao: x = %d, y = %d\n"), msgJogo.tijolos[i].id, msgJogo.tijolos[i].coord.X, msgJogo.tijolos[i].coord.Y);
		}

		//brindes enum Tipo_Brinde { speed_up, slow_down, vida_extra, triple, barreira }; //Adicionar outros brindes consoante a originalidade
		for (int i = 0; i < MAX_NUM_BRINDES; i++) {
			msgJogo.brindes[i].id = 1;
			msgJogo.brindes[i].ativo = 0;
			msgJogo.brindes[i].dimensao = 2 * msgJogo.bolas[0].raio;
			msgJogo.brindes[i].duracao = 10; //EM SEGUNDOS

			switch (rand() % 10)
			{
			case 0:
				msgJogo.brindes[i].tipo = speed_up;
				break;
			case 1:
				msgJogo.brindes[i].tipo = slow_down;
				break;
			case 2:
				msgJogo.brindes[i].tipo = vida_extra;
				break;
			case 3:
				msgJogo.brindes[i].tipo = triple;
				break;
			case 4:
				msgJogo.brindes[i].tipo = barreira;
			default:
				msgJogo.brindes[i].tipo = speed_up;
				break;
			}
			//_tprintf(TEXT("Brinde criado! %d\n"), msgJogo.brindes[i].tipo);

			msgJogo.brindes[i].velocidade = 1;
		}


	}

}


void inserePlayerJogo(HANDLE novo, COMANDO_SHARED aux) {
	for (int i = 0; i < MAX_NUM_PLAYERS; i++) {
		if (msgJogo.players[i].idHandle == INVALID_HANDLE_VALUE) {
			msgJogo.players[i].id = idUserPlayer++;
			msgJogo.players[i].idHandle = novo;
			wcscpy_s(msgJogo.players[i].nome, aux.nome);
			msgJogo.players[i].login = true;
			_tprintf(TEXT("Cliente %s com o id: %d \tInicializado no Jogo!\n"), msgJogo.players[i].nome, msgJogo.players[i].id);
			//_tprintf(TEXT("HANDLE : %d\n"), novo);
			msgJogo.players[i].vidas = MAX_NUM_VIDAS;
			return;
		}
	}
}

void insereBarreiraJogo(int id) {
	int x;
	int y;
	bool pode;
	int coordenada;

	for (int i = 0; i < MAX_NUM_PLAYERS; i++) {
		if (msgJogo.players[i].id == id) {
			msgJogo.players[i].barreira.ativa = true;
			msgJogo.players[i].barreira.coord.Y = LIMITE_INFERIOR;
		}
	}


	pode = true;

	do {
		coordenada = rand() % LIMITE_DIREITO;
		for (int x = 0; x < MAX_NUM_PLAYERS; x++) {
			if (msgJogo.players[x].idHandle != INVALID_HANDLE_VALUE) {
				if (msgJogo.players[x].barreira.coord.X + msgJogo.players[x].barreira.dimensao <= coordenada && msgJogo.players[x].barreira.coord.X >= coordenada + msgJogo.players[x].barreira.dimensao)
					pode = false;
			}
		}
	} while (!pode);


	msgJogo.players[id].barreira.coord.X = coordenada;

	_tprintf(TEXT("O Jogador foi %d foi colocado na posicao: x = %d, y = %d\n"), id, msgJogo.players[id].barreira.coord.X, msgJogo.players[id].barreira.coord.Y);
}


/**************************************************************************************************************************************/


/***************************************************************** FUNÇÔES SOBRE UTILIZADOR ***********************************************/

int getIdPlayer(HANDLE aux) {
	for (int i = 0; i < MAX_NUM_PLAYERS; i++) {
		if (aux == msgJogo.players[i].idHandle) {
		//	_tprintf(TEXT("Retornado id = %d\n"), msgJogo.players[i].id);
			return msgJogo.players[i].id;
		}
	}
	return -1;
}

Player getPlayer(int idUser) {
	Player aux;
	aux.id = -1;
	for (int i = 0; i < MAX_NUM_PLAYERS; i++) {
		if (msgJogo.players[i].id == idUser) {
		//	_tprintf(TEXT("Player com %d [Retorno]\n"), msgJogo.players[i].id);
			return msgJogo.players[i];
		}
	}
	//_tprintf(TEXT("Player com %d [Retorno]\n"), aux.id);
	return aux;
}


//Desligar um Jogador
void desconectaPlayer(int id) {
	for (int i = 0; i < MAX_NUM_PLAYERS; i++) {
		msgJogo.players[i].id = -1;
		msgJogo.players[i].idHandle = INVALID_HANDLE_VALUE;
	}
}

/*************************************************************************************************************************************/

/*********************************************************************** REGISTERY *****************************************************/
//Criar Registo
void createRegistry() {
	DWORD dwDisposition;
	RegCreateKeyEx(HKEY_CURRENT_USER, REGKEY, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hKey, &dwDisposition); //Key set to NULL but dwDisposition == REG_OPENED_EXISTING_KEY

	if (dwDisposition != REG_CREATED_NEW_KEY && dwDisposition != REG_OPENED_EXISTING_KEY)
		_tprintf(TEXT("Erro creating new key!\n"));

	LONG closeOut = RegCloseKey(hKey);
	if (closeOut == ERROR_SUCCESS) {
		_tprintf(TEXT("Success closing key.\n"));
	}
	else {
		_tprintf(TEXT("Error closing key.\n"));
	}
}

//Guardar TOP10 no Registo
void writeRegistry() {
	LONG openRes = RegOpenKeyEx(HKEY_CURRENT_USER, REGKEY, 0, KEY_ALL_ACCESS | KEY_WOW64_64KEY, &hKey);
	if (openRes == ERROR_SUCCESS) {
		_tprintf(TEXT("Success opening key.\n"));
	}
	else {
		_tprintf(TEXT("Error opening key.\n"));
	}

	//LONG setRes = RegSetValueEx(hKey, value, 0, REG_SZ, (LPBYTE)& score, sizeof(Scores));
	LONG setRes = RegSetValueEx(hKey, value, 0, REG_BINARY, (LPBYTE)& score, sizeof(Scores));
	if (setRes == ERROR_SUCCESS) {
		_tprintf(TEXT("Success writing to Registry.\n"));
	}
	else {
		_tprintf(TEXT("Error writing to Registry.\n"));
	}

	LONG closeOut = RegCloseKey(hKey);
	if (closeOut == ERROR_SUCCESS) {
		_tprintf(TEXT("Success closing key.\n"));
	}
	else {
		_tprintf(TEXT("Error closing key.\n"));
	}
}

//LE registo
void readRegistry() {
	LONG openRes = RegOpenKeyEx(HKEY_CURRENT_USER, (LPWSTR)REGKEY, 0, KEY_ALL_ACCESS, &hKey);
	if (openRes == ERROR_SUCCESS) {
		_tprintf(TEXT("Success opening key.\n"));
	}
	else {
		_tprintf(TEXT("Error opening key.\n"));
	}


	DWORD tam = sizeof(Scores);
	RegGetValue(HKEY_CURRENT_USER, REGKEY, value, RRF_RT_ANY, NULL, (PVOID)& score, &tam);
	LONG closeOut = RegCloseKey(hKey);
	if (closeOut == ERROR_SUCCESS) {
		_tprintf(TEXT("Success closing key.\n"));
	}
	else {
		_tprintf(TEXT("Error closing key.\n"));
	}
	msgJogo.ranking = score;
}

//Inicia para teste REGISTRY
void inicia() {
	//Inicializar para mostrar
	swprintf_s(score.jogadores[0].nome, TEXT("Hugo"));
	score.jogadores[0].pontos = 10;

	for (int i = 1; i < 10; i++) {
		//_tcscpy_s(ranking.jogadores[i].nome, sizeof(100), "Nuno e  HUGO");
		swprintf_s(score.jogadores[i].nome, TEXT("Nuno"));
		score.jogadores[i].pontos = 5;
	}
	writeRegistry();

	swprintf_s(score.jogadores[1].nome, TEXT("MANUEL"));
	score.jogadores[1].pontos = 6;

	readRegistry();
	_tprintf(TEXT("REGISTRY!\n TOP SCORE\n"));
	for (int i = 0; i < 10; i++) {
		_tprintf(TEXT("NOME: %s \t SCORE: %d\n"), score.jogadores[i].nome, score.jogadores[i].pontos);
	}
	_tprintf(TEXT("\n"));
}


void meteTop(int id) {
	bool insere = false;
	int posicao = 0;
	int p = 0;

	for (int i = 0; i < 10; i++) {
		if (msgJogo.players[id].pontos > score.jogadores[i].pontos) {
			posicao = i;
			insere = true;
			break;
		}
	}


	if (insere) {
		Scores aux;

		for (int i = 0; i < 10; i++) {
			if (i == posicao) {
				p += 1;
				aux.jogadores[i] = msgJogo.players[id];
			}
			else {
				aux.jogadores[i] = score.jogadores[p];
			}
			p++;
		}


		score = aux;
		msgJogo.ranking = aux;
	}

	writeRegistry();

}

/***************************************************************************************************************************************/