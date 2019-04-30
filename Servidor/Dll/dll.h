#pragma once
#include <windows.h>
#include <tchar.h>
#include <stdio.h>
#include <stdlib.h>
#include <thread>


#define TAM 256
#define BUFFER_SIZE 100
#define BUFFER 10

#define MAX_NUM_PLAYERS 20

#define CMD_LOGIN 5
#define CMD_LOGOUT 6


//Mensagens
HANDLE podeEscrever;
HANDLE podeLer;
HANDLE hMutexler;
HANDLE hMutexEscrever;


//Estruturas

//COMANDO PARTILHADO (COMANDO_SHARED)
typedef struct {
	int idUser;
	int tipo;
	HANDLE idHandle;
} COMANDO_SHARED;


//DADOS
typedef struct {
	COMANDO_SHARED PtrMemoria[BUFFER];
	int posRead, posWrite;
} Dados;

//DATACR
typedef struct {
	int posE, posL;
	Dados * shared;
	HANDLE hSemafroPodeEscrever;
	HANDLE hSemafroPodeLer;
}dataCr;

typedef struct {
	int id;

	INT pontos;
}Player;


#ifdef DLL_EXPORTS
#define DLL_IMP_API __declspec(dllexport)
#else
#define DLL_IMP_API __declspec(dllimport)
#endif

extern "C"
{
	//Váriavel global da DLL
	extern DLL_IMP_API int nDLL;

	//Protótipos Funções
	DLL_IMP_API void readMensagem(dataCr* d, COMANDO_SHARED * s);
	DLL_IMP_API void writeMensagem(dataCr* d, COMANDO_SHARED * s);


}