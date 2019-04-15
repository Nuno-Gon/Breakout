#include <windows.h>
#include <stdio.h>
#include <stdlib.h>

#include "..\Dll\dll.h"

//Constantes
#define PIPE_NAME TEXT("\\\\.\\pipe\\connect")

//Variaveis
HANDLE cliente[MAX_NUM_PLAYERS];
HANDLE hPipe;
BOOL login = FALSE;
int termina = 0;

static int id_user = 1;


//Protótipos funções
DWORD WINAPI recebe_comando_cliente(LPVOID param);
DWORD WINAPI aceita_cliente(LPVOID param);
BOOL existePlayer();


void inicia_vetor_clientes();


//Funções
int _tmain(void) {

#ifdef UNICODE
	_setmode(_fileno(stdin), _O_WTEXT);
	_setmode(_fileno(stdout), _O_WTEXT);
#endif


}

DWORD WINAPI recebe_comando_cliente(LPVOID param) {

}

DWORD WINAPI aceita_cliente(LPVOID param) {

}

BOOL existePlayer() {

}

void inicia_vetor_clientes() {

}