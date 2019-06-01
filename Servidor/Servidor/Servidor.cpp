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

//Registo
void createRegistry();
void writeRegistry();
void readRegistry();
void inicia();

// Outras Funções
void trataComando(COMANDO_SHARED comando);
void inicia_mapa();
int getIdPlayer(HANDLE aux);

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

int _tmain(int argc, LPTSTR argv[]) {

//Project Properties > Character Set > Use Unicode Character Set
#ifdef UNICODE
	_setmode(_fileno(stdin), _O_WTEXT);
	_setmode(_fileno(stdout), _O_WTEXT);
#endif 
	_tprintf(TEXT("\Servidor Ligado!\n"));
	
	//Mutex



	//por verificacoes se correu mal ou não
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
		_tprintf(TEXT("\Erro a abrir memoria Partilhada!\n"));
		system("pause");
		exit(0);
	}

		WaitForSingleObject(eventoComeco, INFINITE);
	
	//Abrir Registro
		createRegistry();
		inicia(); //Inicia o Registry


		inicia_mapa();


		while (1){
			acabar = 0;

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

	_tprintf(TEXT("\Servidor Desligado!\n"));
	UnmapViewOfFile(memoriaPartilhadaServidor.sharedJogo);
	system("pause");
	return 0;
}

//THREADS
//Lê mensagens da Memória
DWORD WINAPI readMensagemMemory(void) {
	_tprintf(TEXT("\Comecei Thread ler mensagem!\n"));
	while (1) {
	readMensagem(&memoriaPartilhadaServidor, &comandoLido); //função no DLL
	trataComando(comandoLido);
	}
	return 0;
}

//Outras Funções
void trataComando(COMANDO_SHARED comando) {
	int id = 0;
	Player aux;
	_tprintf(TEXT("\n Entrei Trata Comando!\n"));

	if (comando.tipo != CMD_LOGIN) {
		id = getIdPlayer(comando.idHandle);

	}

	//Outros tipos de comando

	switch (comando.tipo) {
	case CMD_LOGIN:
		COMANDO_SHARED aux_shared;
		aux_shared = comando;
		aux_shared.login = true;
		_tprintf(TEXT("\n Login Feito com Sucesso!\n"));
		loginPlayer = TRUE;
		break;
	case CMD_LOGOUT:
		_tprintf(TEXT("\n Cliente desconectado!\n"));
		break;
	}

	return;
}


DWORD WINAPI controlaBola(void) {
	//termina quando o cliente insere uma tecla

	while (1) {
		msgJogo.bola.coord.X = rand() % 10;
		msgJogo.bola.coord.Y = rand() % 10;
	}

}

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

//Inicia para teste
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
	_tprintf(TEXT("\nREGISTRY!\n TOP SCORE\n"));
	for (int i = 0; i < 10; i++) {
		_tprintf(TEXT("NOME: %s \t SCORE: %d\n"), score.jogadores[i].nome, score.jogadores[i].pontos);
	}
	_tprintf(TEXT("\n"));
}

int getIdPlayer(HANDLE aux) {
	for (int i = 0; i < MAX_NUM_PLAYERS; i++) {
		if (aux == msgJogo.players[i].idHandle) {
			_tprintf(TEXT("\nRetornado id = %d"), msgJogo.players[i].id);
			return msgJogo.players[i].id;
		}
	}
	return -1;
}


void inicia_mapa() {
	//Configuração inicial do MAPA
	//Barreira
	for (int i = 0; i < MAX_NUM_PLAYERS; i++) {
		msgJogo.players[i].barreira.id = -1;
		msgJogo.players[i].barreira.dimensao = 4; //ainda  verificar
		msgJogo.players[i].barreira.coord.X = -1;
		msgJogo.players[i].barreira.coord.Y = -1;


		msgJogo.players[i].id = -1;
		msgJogo.players[i].pontos = 0;
		msgJogo.players[i].idHandle = INVALID_HANDLE_VALUE;
		msgJogo.players[i].vidas = 3;
	}



}