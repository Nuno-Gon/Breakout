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
//DWORD WINAPI readMensagemMemory(void);

// Outras Funções
//void trataComando(COMANDO_SHARED comando);

//Variaveis Globais
INT acabar;
dataCr memoriaPartilhadaServidor;
COMANDO_SHARED comandoLido;
BOOL loginPlayer = FALSE;
HANDLE thread_read_msg_memory;

int _tmain(int argc, LPTSTR argv[]) {

//Project Properties > Character Set > Use Unicode Character Set
#ifdef UNICODE
	_setmode(_fileno(stdin), _O_WTEXT);
	_setmode(_fileno(stdout), _O_WTEXT);
#endif 
	
	_tprintf(TEXT("\Servidor Ligado!\n"));
	//openSharedMemory(&memoriaPartilhadaServidor);

	
	//thread_read_msg_memory = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)readMensagemMemory, NULL, 0, NULL);
	while (1){
		acabar = 0;
		

		
		
	}
	
	//CloseHandle(thread_read_msg_memory);
	_tprintf(TEXT("\Terminei!\n"));
}

//THREADS
/*
//Lê mensagens da Memória
DWORD WINAPI readMensagemMemory(void) {
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


	if (comando.tipo != CMD_LOGIN) {
		id = (comando.idUser);

	}


	//Outros tipos de comando

	switch (comando.tipo) {
	case CMD_LOGIN: 
		_tprintf(TEXT("\n Cliente Conectado!\n"));
		loginPlayer = TRUE;
		break;
	case CMD_LOGOUT:
		_tprintf(TEXT("\n Cliente desconectado!\n"));
		break;
	}

	return;
}

*/