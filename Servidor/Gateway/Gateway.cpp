#include <Windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <thread>
#include <fcntl.h>
#include <tchar.h>

#include "..\Dll\dll.h"

//Constantes
#define PIPE_NAME TEXT("\\\\.\\pipe\\connect")

//Variaveis
HANDLE cliente[MAX_NUM_PLAYERS];
HANDLE hPipe;
HANDLE thread_cliente;
BOOL login = FALSE;
int termina = 0;
dataCr memoriaPartilhadaGateway; //Semaphoros

static int id_user = 1;

/*Eventos*/
HANDLE id_evento_comeco;


//Protótipos funções
DWORD WINAPI recebe_comando_cliente(LPVOID param);
DWORD WINAPI aceita_cliente(LPVOID param);
BOOL existePlayer();
void inicializaVectorClientes();
BOOL verifica_e_coloca_handle_pipe(HANDLE pipe);
void eliminaHandlePlayer(HANDLE aux);


//Funções
int _tmain(void) {

#ifdef UNICODE
	_setmode(_fileno(stdin), _O_WTEXT);
	_setmode(_fileno(stdout), _O_WTEXT);
#endif
	_tprintf(TEXT("Gateway Começou!\n"));

	//Inicializar clientes
	inicializaVectorClientes();
	thread_cliente = CreateThread(NULL, 0, aceita_cliente, NULL, 0, NULL);
	do {
		_tprintf(TEXT("."));
	} while (existePlayer() || login == FALSE);

	//Processo de Fecho do Gateway
	_tprintf(TEXT("\nGateway Terminou!\n"));
	termina = 1;
	//CloseHandle(thread_cliente);
	for (int i = 0; i < MAX_NUM_PLAYERS; i++) {
		DisconnectNamedPipe(cliente[i]);
		CloseHandle(cliente[i]);
	}

	hPipe = CreateFile(PIPE_NAME, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	DisconnectNamedPipe(hPipe);
	CloseHandle(hPipe);
}

DWORD WINAPI recebe_comando_cliente(LPVOID param) {
	HANDLE x = (HANDLE*)param;
	DWORD n;

	HANDLE ioReady;
	OVERLAPPED ov;
	COMANDO_SHARED aux;
	ioReady = CreateEvent(NULL, TRUE, FALSE, NULL);

	do {
		ZeroMemory(&ov, sizeof(ov));
		ResetEvent(ioReady);
		ov.hEvent = ioReady;

		ReadFile(x, &aux, sizeof(COMANDO_SHARED), &n, &ov);

		WaitForSingleObject(ioReady, INFINITE);
		GetOverlappedResult(hPipe, &ov, &n, FALSE);
		if (!n) {
			_tprintf(TEXT("[Escritor leu] %d bytes... (ReadFile)\n"), n);
			break;
		}

		_tprintf(TEXT("[ESCRITOR leu] Recebi %d bytes tipo %d: (ReadFile)\n"), n, aux.tipo);

		if (aux.tipo == CMD_LOGOUT) {
			_tprintf(TEXT("[ESCRITOR leu] Vou eliminar um jogador!\n"));
			eliminaHandlePlayer(x);
		}


		writeMensagem(&memoriaPartilhadaGateway, &aux);

	} while (aux.tipo != CMD_LOGOUT);

	return 0;
}

DWORD WINAPI aceita_cliente(LPVOID param) {

	while (termina == 0) {
		Sleep(1);
		_tprintf(TEXT("[ESCRITOR] Criar uma copia do pipe '%s' ... (CreateNamedPipe)\n"), PIPE_NAME);
		hPipe = CreateNamedPipe(PIPE_NAME, PIPE_ACCESS_DUPLEX | FILE_FLAG_OVERLAPPED, PIPE_WAIT | PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE, 5, sizeof(COMANDO_SHARED) , sizeof(COMANDO_SHARED), 1000, NULL);

		if (hPipe == INVALID_HANDLE_VALUE) {
			_tprintf(TEXT("[ERRO] Criar Named Pipe! (CreateNamedPipe)"));
			exit(-1);
		}
		_tprintf(TEXT("[ESCRITOR] Esperar liga��o de um leitor... (ConnectNamedPipe)\n"));

		if (!ConnectNamedPipe(hPipe, NULL)) {
			CloseHandle(hPipe);
			_tprintf(TEXT("[ERRO] Liga��o ao leitor! (ConnectNamedPipe\n"));
			exit(-1);
		}
		else {

			if (!verifica_e_coloca_handle_pipe(hPipe)) {
				DisconnectNamedPipe(hPipe);
			}
		}

		SetEvent(id_evento_comeco);
		login = TRUE;
		_tprintf(TEXT("[ESCRITOR]  liga��o a um leitor com sucesso... (ConnectNamedPipe)\n"));
		//criar uma thread para cada cliente para ler msg vindas deles.
		if ((CreateThread(NULL, 0, recebe_comando_cliente, (LPVOID)hPipe, 0, NULL)) == 0) {
			_tprintf(TEXT("erro ao criar thread"));
			exit(-1);
		}
	
	}
	return 0;
	
}


BOOL existePlayer() {
	for (int i = 0; i < MAX_NUM_PLAYERS; i++) {
		if (cliente[i] != INVALID_HANDLE_VALUE) {
			return TRUE;
		}
	}

	return false;
}


void inicializaVectorClientes() {
	for (int i = 0; i < MAX_NUM_PLAYERS; i++) {
		cliente[i] = INVALID_HANDLE_VALUE;
	}
}


BOOL verifica_e_coloca_handle_pipe(HANDLE pipe) {
	for (int i = 0; i < MAX_NUM_PLAYERS; i++) {
		if (cliente[i] == INVALID_HANDLE_VALUE) {
			cliente[i] = pipe;
			return true;
		}
	}
	return false;
}

void eliminaHandlePlayer(HANDLE aux) {
	for (int i = 0; i < MAX_NUM_PLAYERS; i++) {
		if (cliente[i] == aux) {
			_tprintf(TEXT("[Escritor leu] ELeminado\n"));
			cliente[i] = INVALID_HANDLE_VALUE;
		}
	}
}