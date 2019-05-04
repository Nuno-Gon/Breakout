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

// Outras Funções
void trataComando(COMANDO_SHARED comando);

//Variaveis Globais
INT acabar;
MensagemJogo msgJogo;
dataCr memoriaPartilhadaServidor;
COMANDO_SHARED comandoLido;
BOOL loginPlayer = FALSE;
HANDLE thread_read_msg_memory;
HANDLE thread_bola;
HANDLE eventoMemoria, eventoComeco;

int _tmain(int argc, LPTSTR argv[]) {

//Project Properties > Character Set > Use Unicode Character Set
#ifdef UNICODE
	_setmode(_fileno(stdin), _O_WTEXT);
	_setmode(_fileno(stdout), _O_WTEXT);
#endif 
	srand(time_t(NULL));
	_tprintf(TEXT("\Servidor Ligado!\n"));
	
	//por verificacoes se correu mal ou não
	eventoComeco = CreateEvent(NULL, FALSE, FALSE, nomeEventoComecoJogo);
	eventoMemoria = CreateEvent(NULL, FALSE, FALSE, nomeEventoArrancaMemoria);

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
		id = (comando.idUser);

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