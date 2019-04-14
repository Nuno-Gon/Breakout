#pragma once
#include <windows.h>
#include <tchar.h>
#include <stdio.h>
#include <stdlib.h>
#include <thread>


#define TAM 256

#define MAX_NUM_PLAYERS 20


//Mensagens
HANDLE podeEscrever;
HANDLE podeLer;


//Estruturas


#ifdef DLL_EXPORTS
#define DLL_IMP_API __declspec(dllexport)
#else
#define DLL_IMP_API __declspec(dllimport)
#endif

extern "C"
{
	//Váriavel global da DLL
	extern DLL_IMP_API int nDLL;

	DLL_IMP_API void createSharedMemery(dataCr *d);
	DLL_IMP_API void openSharedMemery(dataCr *d);
	DLL_IMP_API void writeMensagem(dataCr* d, COMANDO_SAHRED *comando);
	DLL_IMP_API void readMensagen(dataCr* d, COMANDO_SAHRED * s);


}