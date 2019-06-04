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

//Listagem de Fun��es
//Threads
DWORD WINAPI readMensagemMemory(void);
DWORD WINAPI controlaBola(void);

//Registo
void createRegistry();
void writeRegistry();
void readRegistry();
void inicia();

// Outras Fun��es
void trataComando(COMANDO_SHARED comando);
void inicia_mapa();
int getIdPlayer(HANDLE aux);
void iniciar_tijolos();
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
HANDLE thread_bola;
HANDLE eventoMemoria, eventoComeco;
Scores score;
INT idUserPlayer = 1;
INT idTijolo = 1;


//VARIAVEIS CONFIGURAVEIS JOGO
INT movimentoBarreira = 6;

int _tmain(int argc, LPTSTR argv[]) {

	//Project Properties > Character Set > Use Unicode Character Set
#ifdef UNICODE
	_setmode(_fileno(stdin), _O_WTEXT);
	_setmode(_fileno(stdout), _O_WTEXT);
#endif 
	_tprintf(TEXT("*********************Servidor Ligado!*****************************\n"));

	//Mutex
	mutex_player = CreateMutex(NULL, FALSE, NULL);


	//por verificacoes se correu mal ou n�o
	eventoComeco = CreateEvent(NULL, FALSE, FALSE, nomeEventoComecoJogo);
	eventoMemoria = CreateEvent(NULL, FALSE, FALSE, nomeEventoArrancaMemoria);

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

	WaitForSingleObject(eventoComeco, INFINITE);

	//Abrir Registro
	createRegistry();
	inicia(); //Inicia o Registry


	inicia_mapa();


	while (1) {
		acabar = 0;

		iniciar_tijolos();

		thread_read_msg_memory = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)readMensagemMemory, NULL, 0, NULL);
		thread_bola = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)controlaBola, NULL, 0, NULL);

		do {
			CopyMemory(&memoriaPartilhadaServidor.sharedJogo->jogo, &msgJogo, sizeof(MensagemJogo)); //por no dll
		} while (1);

		acabar = 1;
		CloseHandle(thread_read_msg_memory);
		CloseHandle(thread_bola);
		CloseHandle(eventoComeco);
		CloseHandle(eventoMemoria);
	}

	_tprintf(TEXT("*******************Servidor Desligado!****************************\n"));
	UnmapViewOfFile(memoriaPartilhadaServidor.sharedJogo);
	system("pause");
	return 0;
}

//THREADS
//L� mensagens da Mem�ria
DWORD WINAPI readMensagemMemory(void) {
	//_tprintf(TEXT("Comecei Thread ler mensagem!\n"));
	while (1) {
		readMensagem(&memoriaPartilhadaServidor, &comandoLido); //fun��o no DLL
		trataComando(comandoLido);
	}
	return 0;
}

//Outras Fun��es
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
	//Verificar se pode mover para a direita, e n�o colide com outros utilizadores (EIXO DOS X)
	WaitForSingleObject(mutex_player, INFINITE);
	for (int i = 0; i < MAX_NUM_PLAYERS; i++) {
		if (msgJogo.players[i].id == idUser && checkDireita(idUser)) {
			msgJogo.players[i].barreira.coord.X += movimentoBarreira; //Outro movimento variavel
			ReleaseMutex(mutex_player);
			return;
		}
	}
	ReleaseMutex(mutex_player);

}

void moveJogadorEsquerda(int idUser) {
	//Verificar se pode mover para a direita e n�o colide com outros utilizadores (EIXO DOS X)
	WaitForSingleObject(mutex_player, INFINITE);
	for (int i = 0; i < MAX_NUM_PLAYERS; i++) {
		if (msgJogo.players[i].id == idUser && checkEsquerda(idUser)) {
			msgJogo.players[i].barreira.coord.X -= movimentoBarreira; //Outro movimento variavel
			ReleaseMutex(mutex_player);
			return;
		}
	}
	ReleaseMutex(mutex_player);
}


/************************************************************* CHECKA SE PODE MOVER **************************************************/

BOOL checkDireita(int idUser) {
	//Fazer verifica��o nos eixo dos x, se alguma barreira est� l�, Limites do mapa
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
	//Fazer verifica��o nos eixo dos x, se alguma barreira est� l�, Limites do mapa
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

DWORD WINAPI controlaBola(void) {
	//termina quando o cliente insere uma tecla

	while (1) {

		//NAO ESQUECER DE VERIFICAR QUANDO bate na barreira para ir para cima

		//Verifica��es para a bola deve mexer agora


		if (msgJogo.bola.coord.Y > LIMITE_INFERIOR + 1) { // SE A BOLA PASSAR O LIMITE INFERIOR
	//Acabar esta bola
			msgJogo.bola.ativa = 0;
			msgJogo.bola.coord.Y = -30;
			msgJogo.bola.coord.X = -30;
			break;
		}


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




		if (msgJogo.bola.coord.Y <= LIMITE_SUPERIOR) { // se bater no LIMITE SUPERIOR INVERTER A DIRE��O DA BOLA
			msgJogo.bola.cima = false;
		}
		else if (msgJogo.bola.coord.Y >= LIMITE_INFERIOR) {//Meter um else onde verifica se imbate numa barreira, � se a possi��o dos x quando, est� no limit � igual a da barreira!
			msgJogo.bola.cima = true;
		}
		else if (msgJogo.bola.coord.X >= LIMITE_DIREITO){
			msgJogo.bola.direita = false;
		}
		else if (msgJogo.bola.coord.X <= LIMITE_ESQUERDO) {
			msgJogo.bola.direita = true;
		}
		Sleep(5);
	}

	return 0;
}


/**************************************************************************************************************************************/


/********************************************************************* CONFIGURA��O DO JOGO ********************************************/

//Configura��o inicial do MAPA
void inicia_mapa() {
	//Barreira
	for (int i = 0; i < MAX_NUM_PLAYERS; i++) {
		msgJogo.players[i].barreira.id = -1;
		msgJogo.players[i].barreira.dimensao = 40; //ainda  verificar
		msgJogo.players[i].barreira.coord.X = -1;
		msgJogo.players[i].barreira.coord.Y = -1;


		msgJogo.players[i].id = -1;
		msgJogo.players[i].pontos = 0;
		msgJogo.players[i].idHandle = INVALID_HANDLE_VALUE;
		msgJogo.players[i].vidas = 3;
	}

	//Tijolos
	for (int i = 0; i < MAX_NUM_TIJOLOS; i++) {
			msgJogo.tijolos[i].id = idTijolo++;
			msgJogo.tijolos[i].vida = 1;
			msgJogo.tijolos[i].coord.X = LIMITE_SUPERIOR + 35 + ((i % MAX_NUM_TIJOLOS_LINHA) * (LARG_TIJOLO + 10)); //Secalhar mudar porque o tijolo n�o � um quadrado, mas para come�ar ;)
			msgJogo.tijolos[i].coord.Y = LIMITE_ESQUERDO + 20 + ((i / MAX_NUM_TIJOLOS_LINHA) * (ALT_TIJOLO + 20));
			//_tprintf(TEXT("Tijolo Colocado %d na posicao: x = %d, y = %d\n"), msgJogo.tijolos[i].id, msgJogo.tijolos[i].coord.X, msgJogo.tijolos[i].coord.Y);
	}

	//BOLA
	msgJogo.bola.ativa = 1;
	msgJogo.bola.coord.X = LIMITE_DIREITO / 2;
	msgJogo.bola.coord.Y = LIMITE_INFERIOR - 20;
	msgJogo.bola.cima = true;
	msgJogo.bola.direita = true;
	msgJogo.bola.velocidade = 1;
}


//Iniciar tijolos
void iniciar_tijolos() {
	//Meter configura��es para iniciar jogo


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

			//Fazer fun��o para verificar se ficou na posi��o certa
		}
	}
}

/**************************************************************************************************************************************/


/***************************************************************** FUN��ES SOBRE UTILIZADOR ***********************************************/

int getIdPlayer(HANDLE aux) {
	for (int i = 0; i < MAX_NUM_PLAYERS; i++) {
		if (aux == msgJogo.players[i].idHandle) {
			_tprintf(TEXT("Retornado id = %d\n"), msgJogo.players[i].id);
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
			_tprintf(TEXT("Player com %d [Retorno]\n"), msgJogo.players[i].id);
			return msgJogo.players[i];
		}
	}
	_tprintf(TEXT("Player com %d [Retorno]\n"), aux.id);
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