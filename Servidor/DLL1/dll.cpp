// dll.cpp : Define as funções exportadas para a DLL.
//

#include "pch.h"
#include "framework.h"
#include "dll.h"


// Isto é um exemplo de uma variável exportada
DLL_API int nDll=0;

// Isto é um exemplo de uma função exportada.
DLL_API int fnDll(void)
{
    return 0;
}

// Este é o construtor de uma classe que foi exportada.
CDll::CDll()
{
    return;
}



//DLL.cpp
#include <windows.h>

//Para verificar ao carregar a dll que a aplicação irá ocupar mais memória
char ponteiro[40960];

//definição da varíavel global
int nDLL = 1234;

//Funções (por ordem de declaração)


void createSharedMemory(dataCr* d) {
	//Criar memoria partilhada
	d->hMapFileMSG = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, sizeof(Dados), nomeMemoriaComandos);
	if (d->hMapFileMSG == NULL) {
		_tprintf(TEXT("[Erro]Criação de objectos do Windows(%d)\n"), GetLastError());
		return;
	}


	d->shared = (Dados*)MapViewOfFile(d->hMapFileMSG, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(Dados));
	if (d->shared == NULL) {
		_tprintf(TEXT("[Erro]Mapeamento da memoria partilhada(%d)\n"), GetLastError());
		return;
	}

	d->hSemafroPodeEscrever = CreateSemaphore(NULL, 1000, 1000, nomeSemaforoPodeEscrever);
	if (d->hSemafroPodeEscrever == NULL) {
		_tprintf(TEXT("O Semaforo deu erro!\n"));
		return;
	}


	d->hSemafroPodeLer = CreateSemaphore(NULL, 0, BUFFER, nomeSemaforoPodeLer);
	if (d->hSemafroPodeLer == NULL) {
		_tprintf(TEXT("O Semaforo deu erro!\n"));
		return;
	}

	d->shared->posWrite = 0;
	d->shared->posRead = 0;


}

void openSharedMemory(dataCr* d) {
	//abrir memoria partilhada
	d->hMapFileMSG = OpenFileMapping(FILE_MAP_ALL_ACCESS, FALSE, nomeMemoriaComandos);
	if (d->hMapFileMSG == NULL) {
		_tprintf(TEXT("Could not open file mapping object (%d).\n"), GetLastError());
		return;
	}

	d->shared = (Dados*)MapViewOfFile(d->hMapFileMSG, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(Dados));
	if (d->shared == NULL) {
		_tprintf(TEXT("[Erro]Mapeamento da memoria partilhada(%d)\n"), GetLastError());
		CloseHandle(d->hMapFileMSG);
		return;
	}

	d->hSemafroPodeEscrever = OpenSemaphore(SYNCHRONIZE, TRUE, nomeSemaforoPodeEscrever);
	if (d->hSemafroPodeEscrever == NULL) {
		_tprintf(TEXT("ERRO no Semaforo!\n"));
		return;
	}

	d->hSemafroPodeLer = OpenSemaphore(SYNCHRONIZE, TRUE, nomeSemaforoPodeLer);
	if (d->hSemafroPodeLer == NULL) {
		_tprintf(TEXT("Erro no Semaforo!\n"));
		return;
	}


}

void readMensagem(dataCr* d, COMANDO_SHARED* s) {

	/*
	DWORD WaitForSingleObject(
		HANDLE hHandle,
		DWORD  dwMilliseconds
	);
	*/
	WaitForSingleObject(d->hSemafroPodeEscrever, INFINITE);
	d->posL = d->shared->posRead;
	d->shared->posRead = (d->shared->posRead + 1) % BUFFER;

	WaitForSingleObject(hMutexler, INFINITE);

	/*
	void CopyMemory(
		_In_       PVOID  Destination,
		_In_ const VOID   *Source,
		_In_       SIZE_T Length
);
	*/
	CopyMemory(s, &d->shared->PtrMemoria[d->posL], sizeof(COMANDO_SHARED));

	ReleaseMutex(hMutexler);

	/*
	BOOL ReleaseSemaphore(
		HANDLE hSemaphore,
		LONG   lReleaseCount,
		LPLONG lpPreviousCount
	);
	*/
	ReleaseSemaphore(d->hSemafroPodeEscrever, 1, NULL);


}

void writeMensagem(dataCr * d, COMANDO_SHARED * s) {
	WaitForSingleObject(d->hSemafroPodeEscrever, INFINITE);
	WaitForSingleObject(hMutexEscrever, INFINITE);

	d->posE = d->shared->posWrite;
	d->shared->posWrite = (d->shared->posWrite + 1) % BUFFER;

	CopyMemory(&d->shared->PtrMemoria[d->posE], s, sizeof(COMANDO_SHARED));

	ReleaseMutex(hMutexEscrever);
	ReleaseSemaphore(d->hSemafroPodeLer, 1, NULL);

}
