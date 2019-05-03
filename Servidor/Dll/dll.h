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

#define CMD_MOVE_CIMA 1
#define CMD_MOVE_BAIXO 2
#define CMD_MOVE_ESQ 3 
#define CMD_MOVE_DIR 4
#define CMD_LOGIN 5
#define CMD_LOGOUT 6


//Mensagens (SINCRONIZAÇÃO)
HANDLE podeEscrever;
HANDLE podeLer;
HANDLE hMutexLer;
HANDLE hMutexEscrever;

//Objetos JOGO (SINCRONIZAÇÃO)
HANDLE mutex_bola;

//Memoria Partilhada 
TCHAR nomeMemoriaComandos[] = TEXT("Nome da Memoria Partilhada Comandos");
TCHAR nomeMemoriaJogo[] = TEXT("Nome da Mem�ria Partilhada Jogo");

TCHAR nomeSemaforoPodeEscrever[] = TEXT("Semaforo Pode Escrever");
TCHAR nomeSemaforoPodeLer[] = TEXT("Semaforo Pode Ler");

//EVENTOS
TCHAR nomeEventoComecoJogo[] = TEXT("EventoComeco");
TCHAR nomeEventoArrancaMemoria[] = TEXT("EventoMemoria");
TCHAR nomeEventoTerminaJogo[] = TEXT("EventoTermina");

//Estruturas
//COMANDO PARTILHADO (COMANDO_SHARED)
typedef struct {
	int id;
	INT pontos;
	bool login; //Verificar se foi login
}Player;

typedef struct {
	int idUser;
	int tipo;
	bool login;
	HANDLE idHandle;
} COMANDO_SHARED;

typedef struct {
	COORD coord;
}Bola;


typedef struct {
	Player players[MAX_NUM_PLAYERS];
	Bola bola; // Ver se só existe uma bola
}MensagemJogo; //Jogo

typedef struct {
	MensagemJogo jogo;
}DadosJogo;


//DADOS
typedef struct {
	COMANDO_SHARED PtrMemoria[BUFFER];
	int posRead, posWrite;
} Dados;

//DATACR
typedef struct {
	int posE, posL;
	Dados* shared;
	HANDLE hMapFileMSG;
	HANDLE hSemafroPodeEscrever;
	HANDLE hSemafroPodeLer;
	
	DadosJogo *sharedJogo;
	HANDLE hMapFileJogo;

}dataCr;






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
	DLL_IMP_API bool createSharedMemory(dataCr* d);
	DLL_IMP_API bool openSharedMemory(dataCr* d);
	DLL_IMP_API void readMensagem(dataCr* d, COMANDO_SHARED* s);
	DLL_IMP_API void writeMensagem(dataCr* d, COMANDO_SHARED* s);

	DLL_IMP_API void createSharedMemoryJogo(dataCr* d);
	DLL_IMP_API bool openSharedMemoryJogo(dataCr* d);
	DLL_IMP_API void readMensagemJogo(dataCr* d, MensagemJogo* s);
	DLL_IMP_API void writeMensagemJogo(dataCr* d, MensagemJogo* s);

}