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
DWORD WINAPI controlaBola(void);
DWORD WINAPI controlaBrinde(LPVOID p);

//Registo
void createRegistry();
void writeRegistry();
void readRegistry();
void inicia();

// Outras Funções
void trataComando(COMANDO_SHARED comando);
void inicia_mapa();
int getIdPlayer(HANDLE aux);
Player getPlayer(int idUser);
void desconectaPlayer(int id);
void inserePlayerJogo(HANDLE novo);
BOOL checkDireita(int idUser);
BOOL checkEsquerda(int idUser);
void insereBarreiraJogo(int id);
void moveJogadorEsquerda(int idUser);
void moveJogadorDireita(int idUser);


//Variaveis Globais
INT acabar;
MensagemJogo msgJogo;
dataCr memoriaPartilhadaServidor;
COMANDO_SHARED comandoLido;
BOOL loginPlayer = FALSE;
HANDLE thread_read_msg_memory;
HANDLE thread_bola, thread_brinde;
HANDLE eventoMemoria, eventoComeco, eventoTerminaJogo;
Scores score;
INT idUserPlayer = 1;
INT idTijolo = 1;
INT idBrinde = 1;


int _tmain(int argc, LPTSTR argv[]) {

	//Project Properties > Character Set > Use Unicode Character Set
#ifdef UNICODE
	_setmode(_fileno(stdin), _O_WTEXT);
	_setmode(_fileno(stdout), _O_WTEXT);
#endif 
	_tprintf(TEXT("*********************Servidor Ligado!*****************************\n"));

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


	//Abrir Registro
	createRegistry();
	inicia(); //Inicia o Registry


	inicia_mapa();

	thread_read_msg_memory = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)readMensagemMemory, NULL, 0, NULL);
	acabar = 0;
	WaitForSingleObject(eventoComeco, INFINITE);
	thread_bola = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)controlaBola, NULL, 0, NULL);

	do {
		CopyMemory(&memoriaPartilhadaServidor.sharedJogo->jogo, &msgJogo, sizeof(MensagemJogo)); //por no dll
	} while (acabar == 0);

	acabar = 1;
	CloseHandle(thread_read_msg_memory);
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

	if (comando.tipo != CMD_LOGIN) {
		id = getIdPlayer(comando.idHandle);
		aux = getPlayer(id);
	}

	//Outros tipos de comando

	switch (comando.tipo) {
	case CMD_LOGIN:
		COMANDO_SHARED aux_shared;
		aux_shared = comando;
		aux_shared.login = true;
		_tprintf(TEXT("Novo Utilizador Logado!\n"));
		//Insere no JOGO
		inserePlayerJogo(comando.idHandle);
		loginPlayer = TRUE;
		break;
	case CMD_MOVE_DIR:
		moveJogadorDireita(id);
		break;
	case CMD_MOVE_ESQ:
		moveJogadorEsquerda(id);
		break;
	case CMD_LOGOUT:
		desconectaPlayer(id);
		_tprintf(TEXT("Cliente [%d] desconectado!\n"), id);
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

	return true;
}


/**************************************************************************************************************************************/

/*********************************************************************** THREADS *******************************************************/
DWORD WINAPI readMensagemMemory(void) {
	//_tprintf(TEXT("Comecei Thread ler mensagem!\n"));
	while (1) {
		readMensagem(&memoriaPartilhadaServidor, &comandoLido); //função no DLL
		trataComando(comandoLido);
	}
	return 0;
}

DWORD WINAPI controlaBola(void) {
	bool acabou = false;
	while (1) {
		//MOVIMENTO
		if (msgJogo.bola.cima) { //se mover para cima

			if (msgJogo.bola.direita) { //Se mover para a direita
				msgJogo.bola.coord.X += msgJogo.bola.velocidade;
				msgJogo.bola.coord.Y -= msgJogo.bola.velocidade;
			}
			else {
				msgJogo.bola.coord.X -= msgJogo.bola.velocidade;
				msgJogo.bola.coord.Y -= msgJogo.bola.velocidade;
			}


		}
		else { // Se mover para baixo
			if (msgJogo.bola.direita) { //Se mover para a direita
				msgJogo.bola.coord.X += msgJogo.bola.velocidade;
				msgJogo.bola.coord.Y += msgJogo.bola.velocidade;
			}
			else {
				msgJogo.bola.coord.X -= msgJogo.bola.velocidade;
				msgJogo.bola.coord.Y += msgJogo.bola.velocidade;
			}

		}

		//Se a bola sair dos limites
		if (msgJogo.bola.coord.Y >= LIMITE_INFERIOR) { // SE A BOLA PASSAR O LIMITE INFERIOR
			msgJogo.bola.ativa = 0;
			msgJogo.bola.coord.Y = -30;
			msgJogo.bola.coord.X = -30;
			bool bola = false;

			for (int i = 0; i < MAX_NUM_PLAYERS; i++) { //Criar a nova bola, vou reutilizar a mesma thread
				if (msgJogo.players[i].idHandle != INVALID_HANDLE_VALUE) {


					msgJogo.players[i].vidas--;

					//JOGO ACABOU 
					if (msgJogo.players[i].vidas <= 0) {

						//PERDERAM OU SEJA! DESLIGAR E VER A PONTUAÇÂO
						//Desliga do jogo e pode metê-lo no top 10 (verificar se fica)
						return 0;

					}

					//LANCA A NOVA BOLA (QUE é a mesma XD)
					if (msgJogo.players[i].vidas > 0) {
						Sleep(200);
						msgJogo.bola.ativa = 1;
						msgJogo.bola.coord.X = LIMITE_ESQUERDO + (rand() % LIMITE_DIREITO);
						msgJogo.bola.coord.Y = LIMITE_INFERIOR - 50;
						msgJogo.bola.cima = true;
						bola = true;
						acabou = true;
						i = MAX_NUM_PLAYERS + 1;
					}
				}

			}

		}

		if (msgJogo.bola.coord.Y <= LIMITE_SUPERIOR) { // se bater no LIMITE SUPERIOR INVERTER A DIREÇÃO DA BOLA
			msgJogo.bola.cima = false;
		}
		else if (msgJogo.bola.coord.X >= LIMITE_DIREITO) {
			msgJogo.bola.direita = false;
		}
		else if (msgJogo.bola.coord.X <= LIMITE_ESQUERDO) {
			msgJogo.bola.direita = true;
		}


		//Verificar se bate na barreira
		for (int i = 0; i < MAX_NUM_PLAYERS; i++) {
			if (msgJogo.players[i].idHandle != INVALID_HANDLE_VALUE) //Ver se está tudo bem, porque estou a comprar só um raio!
				if (msgJogo.bola.coord.Y + msgJogo.bola.raio / 2 >= (msgJogo.players[i].barreira.coord.Y - ALT_BARREIRA) && msgJogo.bola.coord.X + msgJogo.bola.raio >= msgJogo.players[i].barreira.coord.X && msgJogo.bola.coord.X <= msgJogo.players[i].barreira.coord.X + msgJogo.players[i].barreira.dimensao) {
					msgJogo.bola.cima = true;
				}
		}


		//Verificar se embate nos tijolos e se bater retirar tijolo
		for (int i = 0; i < MAX_NUM_TIJOLOS; i++) {
			if (msgJogo.tijolos[i].vida > 0) {
				if (msgJogo.bola.coord.Y - msgJogo.bola.raio <= msgJogo.tijolos[i].coord.Y + ALT_TIJOLO && msgJogo.bola.coord.Y + msgJogo.bola.raio >= msgJogo.tijolos[i].coord.Y &&
					msgJogo.bola.coord.X - msgJogo.bola.raio <= msgJogo.tijolos[i].coord.X + LARG_TIJOLO && msgJogo.bola.coord.X + msgJogo.bola.raio >= msgJogo.tijolos[i].coord.X) {

					//verificar para que lado do tijolo embate
					if (msgJogo.bola.cima) {
						if (!msgJogo.bola.direita) { //para cima e para a esquerda

							if (msgJogo.tijolos[i].coord.X + LARG_TIJOLO - msgJogo.bola.coord.X > msgJogo.tijolos[i].coord.Y + ALT_TIJOLO - msgJogo.bola.coord.Y) {
								msgJogo.bola.cima = false;
							}
							else {
								msgJogo.bola.direita = true;
							}

						}
						else { //Cima direita

							if (msgJogo.bola.coord.X - msgJogo.tijolos[i].coord.X > msgJogo.tijolos[i].coord.Y + ALT_TIJOLO - msgJogo.bola.coord.Y) {
								msgJogo.bola.cima = false;
							}
							else {
								msgJogo.bola.direita = false;
							}


						}
					}
					else {

						if (!msgJogo.bola.direita) { //para baixo e para a esquerda

							if (msgJogo.tijolos[i].coord.X + LARG_TIJOLO - msgJogo.bola.coord.X > msgJogo.bola.coord.Y - msgJogo.tijolos[i].coord.Y) {
								msgJogo.bola.cima = true;
							}
							else {
								msgJogo.bola.direita = true;
							}

						}
						else { //Para baixo e para a direita

							if (msgJogo.bola.coord.X - msgJogo.tijolos[i].coord.X > msgJogo.bola.coord.Y - msgJogo.tijolos[i].coord.Y) {
								msgJogo.bola.cima = true;
							}
							else {
								msgJogo.bola.direita = false;
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
				if (msgJogo.bola.coord.Y - msgJogo.bola.raio <= msgJogo.brindes[i].coord.Y + ALT_BRINDE && msgJogo.bola.coord.Y + msgJogo.bola.raio >= msgJogo.brindes[i].coord.Y &&
					msgJogo.bola.coord.X - msgJogo.bola.raio <= msgJogo.brindes[i].coord.X + msgJogo.brindes[i].dimensao && msgJogo.bola.coord.X + msgJogo.bola.raio >= msgJogo.brindes[i].coord.X) {

					//verificar para que lado do tijolo embate
					if (msgJogo.bola.cima) {
						if (!msgJogo.bola.direita) { //para cima e para a esquerda

							if (msgJogo.brindes[i].coord.X + msgJogo.brindes[i].dimensao - msgJogo.bola.coord.X > msgJogo.brindes[i].coord.Y + ALT_BRINDE - msgJogo.bola.coord.Y) {
								msgJogo.bola.cima = false;
							}
							else {
								msgJogo.bola.direita = true;
							}

						}
						else { //Cima direita

							if (msgJogo.bola.coord.X - msgJogo.brindes[i].coord.X > msgJogo.brindes[i].coord.Y + ALT_BRINDE - msgJogo.bola.coord.Y) {
								msgJogo.bola.cima = false;
							}
							else {
								msgJogo.bola.direita = false;
							}


						}
					}
					else {

						if (!msgJogo.bola.direita) { //para baixo e para a esquerda

							if (msgJogo.brindes[i].coord.X + msgJogo.brindes[i].dimensao - msgJogo.bola.coord.X > msgJogo.bola.coord.Y - msgJogo.brindes[i].coord.Y) {
								msgJogo.bola.cima = true;
							}
							else {
								msgJogo.bola.direita = true;
							}

						}
						else { //Para baixo e para a direita

							if (msgJogo.bola.coord.X - msgJogo.brindes[i].coord.X > msgJogo.bola.coord.Y - msgJogo.brindes[i].coord.Y) {
								msgJogo.bola.cima = true;
							}
							else {
								msgJogo.bola.direita = false;
							}


						}

					}

				}


			}
		}

		//Verifica vitória ou termino de Jogo!

		for (int i = 0; i < MAX_NUM_TIJOLOS; i++) {
			if (msgJogo.tijolos[i].vida > 0) {
				acabou = false;
				i = MAX_NUM_TIJOLOS + 1;
			}
		}

		//Acabou o jogo
		if (acabou) {
			msgJogo.bola.ativa = 0;
			msgJogo.bola.coord.X = -30;
			msgJogo.bola.coord.Y = -30;


			break;
		}

		acabou = true;

		Sleep(5);
	}

	//Fazer verificação se ganha

	return 0;
}

DWORD WINAPI controlaBrinde(LPVOID p) {
	INT_PTR id = reinterpret_cast<INT_PTR>(p);
	bool acabou = false;
	int aux;
	msgJogo.brindes[id].ativo = 1;
	_tprintf(TEXT("Funcao do brind evocada!\nBrinde do tipo: %d\n\n\n\n"), msgJogo.brindes[id].tipo);
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
						aux = msgJogo.players[i].barreira.velocidade;

						if (msgJogo.players[i].barreira.velocidade + msgJogo.players[i].barreira.velocidade_inicial * 0.20 > msgJogo.players[i].barreira.velocidade_inicial * 2) {
							//_tprintf(TEXT("DADA\n\n"));
							msgJogo.brindes[id].ativo = 0;
							msgJogo.brindes[id].coord.Y = -30;
							msgJogo.brindes[id].coord.X = -30;

						}
						else {
							msgJogo.players[i].barreira.velocidade = msgJogo.players[i].barreira.velocidade + msgJogo.players[i].barreira.velocidade_inicial * 0.20;
							msgJogo.brindes[id].ativo = 0;
							msgJogo.brindes[id].coord.Y = -30;
							msgJogo.brindes[id].coord.X = -30;
							Sleep(msgJogo.brindes[id].duracao * 1000);
							msgJogo.players[i].barreira.velocidade = aux;

						}

					
						i = MAX_NUM_PLAYERS + 1;
						acabou = true;
						break;
					case slow_down:
						//_tprintf(TEXT("Slowdown!\n"));
						//PReciso de meter a duração, lancar uma thread com temporizador
						if (msgJogo.players[i].barreira.velocidade_inicial - msgJogo.players[i].barreira.velocidade < msgJogo.players[i].barreira.velocidade_inicial * 0.60) {
							msgJogo.brindes[id].ativo = 0;
							msgJogo.brindes[id].coord.Y = -30;
							msgJogo.brindes[id].coord.X = -30;
						}
						else {
							aux = msgJogo.players[i].barreira.velocidade;
							msgJogo.players[i].barreira.velocidade = msgJogo.players[i].barreira.velocidade - msgJogo.players[i].barreira.velocidade_inicial * 0.20;
							msgJogo.brindes[id].ativo = 0;
							msgJogo.brindes[id].coord.Y = -30;
							msgJogo.brindes[id].coord.X = -30;

							Sleep(msgJogo.brindes[id].duracao * 1000);
							msgJogo.players[i].barreira.velocidade = aux;
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
						//edada
					//	_tprintf(TEXT("BARREIRA\n"));
						break;
					case barreira:
					//	_tprintf(TEXT("BRINDE BARREIRA!\n"));
						msgJogo.players[i].barreira.dimensao += rand() % 10;
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
	msgJogo.bola.ativa = 1;
	msgJogo.bola.coord.X = LIMITE_ESQUERDO + (rand() % LIMITE_DIREITO);
	msgJogo.bola.coord.Y = LIMITE_INFERIOR - 20;
	msgJogo.bola.cima = true;
	msgJogo.bola.direita = true;
	msgJogo.bola.velocidade = 1;
	msgJogo.bola.raio = 5;

	//Barreira
	for (int i = 0; i < MAX_NUM_PLAYERS; i++) {
		msgJogo.players[i].barreira.id = -1;
		msgJogo.players[i].barreira.dimensao = 40; //ainda  verificar
		msgJogo.players[i].barreira.coord.X = -20;
		msgJogo.players[i].barreira.coord.Y = -20;
		msgJogo.players[i].barreira.velocidade = 15;
		msgJogo.players[i].barreira.velocidade_inicial = 15;


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
		msgJogo.brindes[i].dimensao = 2 * msgJogo.bola.raio;
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

void inserePlayerJogo(HANDLE novo) {
	for (int i = 0; i < MAX_NUM_PLAYERS; i++) {
		if (msgJogo.players[i].idHandle == INVALID_HANDLE_VALUE) {
			_tprintf(TEXT("Cliente Inicializado no Jogo!\n"));
			msgJogo.players[i].id = idUserPlayer++;
			msgJogo.players[i].idHandle = novo;
			msgJogo.players[i].vidas = MAX_NUM_VIDAS;
			insereBarreiraJogo(msgJogo.players[i].id);
			return;
		}
	}
}

//FAlta meter a barreira no sitio certo
void insereBarreiraJogo(int id) {
	int x;
	int y;

	for (int i = 0; i < MAX_NUM_PLAYERS; i++) {
		if (msgJogo.players[i].id == id) {
			msgJogo.players[i].barreira.coord.X = LIMITE_ESQUERDO + 10;
			msgJogo.players[i].barreira.coord.Y = LIMITE_INFERIOR;
			_tprintf(TEXT("O Jogador foi %d foi colocado na posicao: x = %d, y = %d\n"), id, msgJogo.players[i].barreira.coord.X, msgJogo.players[i].barreira.coord.Y);

			//Fazer função para verificar se ficou na posição certa
		}
	}
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
		_tprintf(TEXT("Success closing key."));
	}
	else {
		_tprintf(TEXT("Error closing key."));
	}
}

//Guardar TOP10 no Registo
void writeRegistry() {
	LONG openRes = RegOpenKeyEx(HKEY_CURRENT_USER, REGKEY, 0, KEY_ALL_ACCESS | KEY_WOW64_64KEY, &hKey);
	if (openRes == ERROR_SUCCESS) {
		_tprintf(TEXT("Success opening key."));
	}
	else {
		_tprintf(TEXT("Error opening key."));
	}

	LONG setRes = RegSetValueEx(hKey, value, 0, REG_SZ, (LPBYTE)& score, sizeof(Scores));
	if (setRes == ERROR_SUCCESS) {
		_tprintf(TEXT("Success writing to Registry."));
	}
	else {
		_tprintf(TEXT("Error writing to Registry."));
	}

	LONG closeOut = RegCloseKey(hKey);
	if (closeOut == ERROR_SUCCESS) {
		_tprintf(TEXT("Success closing key."));
	}
	else {
		_tprintf(TEXT("Error closing key."));
	}
}

//LE registo
void readRegistry() {
	LONG openRes = RegOpenKeyEx(HKEY_CURRENT_USER, (LPWSTR)REGKEY, 0, KEY_ALL_ACCESS, &hKey);
	if (openRes == ERROR_SUCCESS) {
		_tprintf(TEXT("Success opening key."));
	}
	else {
		_tprintf(TEXT("Error opening key."));
	}


	DWORD tam = sizeof(Scores);
	RegGetValue(HKEY_CURRENT_USER, REGKEY, value, RRF_RT_ANY, NULL, (PVOID)& score, &tam);
	LONG closeOut = RegCloseKey(hKey);
	if (closeOut == ERROR_SUCCESS) {
		_tprintf(TEXT("Success closing key."));
	}
	else {
		_tprintf(TEXT("Error closing key."));
	}
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

/***************************************************************************************************************************************/