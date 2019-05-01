// O bloco ifdef a seguir é a forma padrão de criar macros que tornam a exportação
// de uma DLL mais simples. Todos os arquivos nessa DLL são compilados com DLL2_EXPORTS
// símbolo definido na linha de comando. Esse símbolo não deve ser definido em nenhum projeto
// que usa esta DLL. Desse modo, qualquer projeto cujos arquivos de origem incluem este arquivo veem
// Funções DLL2_API como importadas de uma DLL, enquanto esta DLL vê símbolos
// definidos com esta macro conforme são exportados.
#ifdef DLL_EXPORTS
#define DLL_API __declspec(dllexport)
#else
#define DLL_API __declspec(dllimport)
#endif

// Esta classe é exportada da DLL
class DLL_API CDll {
public:
	CDll(void);
	// TODO: adicione seus métodos aqui.
};

extern DLL_API int nDll;

DLL_API int fnDll(void);



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


//Memoria Partilhada 
TCHAR nomeMemoriaComandos[] = TEXT("Nome da Memoria Partilhada Comandos");
TCHAR nomeSemaforoPodeEscrever[] = TEXT("Semaforo Pode Escrever");
TCHAR nomeSemaforoPodeLer[] = TEXT("Semaforo Pode Ler");



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
	Dados* shared;
	HANDLE hMapFileMSG;
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
	DLL_IMP_API void createSharedMemory(dataCr* d);
	DLL_IMP_API void openSharedMemory(dataCr* d);
	DLL_IMP_API void readMensagem(dataCr* d, COMANDO_SHARED* s);
	DLL_IMP_API void writeMensagem(dataCr* d, COMANDO_SHARED* s);


}