// dllmain.cpp : Define o ponto de entrada para o aplicativo DLL.
#include "pch.h"
#include <windows.h>
#include "dll.h"

char ponteiro[40960];
//Definição de variavel global
int nDLL = 1234;

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}

void createSharedMemory(dataCr* d) {
	d->hMapFileMSG = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, sizeof(Dados), nomeMemoriaComandos);
	if (d->hMapFileMSG == NULL) {
		_tprintf(TEXT("[Erro]Cria��o de objectos do Windows(%d)\n"), GetLastError());
		return;
	}

	d->shared = (Dados*)MapViewOfFile(d->hMapFileMSG, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(Dados));
	if (d->shared == NULL) {
		_tprintf(TEXT("[Erro]Mapeamento da mem�ria partilhada(%d)\n"), GetLastError());
		return;
	}


	d->hSemafroPodeEscrever = CreateSemaphore(NULL, 1000, 1000, nomeSemaforoPodeEscrever);
	if (d->hSemafroPodeEscrever == NULL) {
		_tprintf(TEXT("O semafro correu mal\n"));
		return;
	}

	d->hSemafroPodeLer = CreateSemaphore(NULL, 0, BUFFER, nomeSemaforoPodeLer);
	if (d->hSemafroPodeLer == NULL) {
		_tprintf(TEXT("O semafro deu problemas\n"));
		return;
	}


	d->shared->posWrite = 0;
	d->shared->posRead = 0;
}


void openSharedMemory(dataCr* d) {
	d->hMapFileMSG = OpenFileMapping(FILE_MAP_ALL_ACCESS, FALSE, nomeMemoriaComandos);
	if (d->hMapFileMSG == NULL) {
		_tprintf(TEXT("Could not open file mapping object (%d).\n"), GetLastError());
		return;
	}

	d->shared = (Dados*)MapViewOfFile(d->hMapFileMSG, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(Dados));
	if (d->shared == NULL) {
		_tprintf(TEXT("[Erro]Mapeamento da mem�ria partilhada(%d)\n"), GetLastError());
		CloseHandle(d->hMapFileMSG);
		return;
	}


	d->hSemafroPodeEscrever = OpenSemaphore(SYNCHRONIZE, TRUE, nomeSemaforoPodeEscrever);
	if (d->hSemafroPodeEscrever == NULL) {
		_tprintf(TEXT("Erro Semaphoro!\n"));
		return;
	}

	d->hSemafroPodeLer = OpenSemaphore(SYNCHRONIZE, TRUE, nomeSemaforoPodeLer);
	if (d->hSemafroPodeLer == NULL) {
		_tprintf(TEXT("O Semaforo da erro!\n"));
		return;
	}
}

void readMensagem(dataCr* d, COMANDO_SHARED* s) {
	WaitForSingleObject(d->hSemafroPodeLer, INFINITE);
	d->posL = d->shared->posRead;
	d->shared->posRead = (d->shared->posRead + 1) % BUFFER;
	WaitForSingleObject(hMutexler, INFINITE);

	//_tprintf(TEXT("Li do buffer %d: '%s'\n"), d->posL, d->shared->PtrMemoria[d->posL]);  // Reader reads data

	CopyMemory(s, &d->shared->PtrMemoria[d->posL], sizeof(COMANDO_SHARED));

	ReleaseMutex(hMutexler);
	ReleaseSemaphore(d->hSemafroPodeEscrever, 1, NULL);
}

void writeMensagem(dataCr* d, COMANDO_SHARED* s) {
	WaitForSingleObject(d->hSemafroPodeEscrever, INFINITE);
	WaitForSingleObject(hMutexEscrever, INFINITE);

	d->posE = d->shared->posWrite;
	d->shared->posWrite = (d->shared->posWrite + 1) % BUFFER;

	CopyMemory(&d->shared->PtrMemoria[d->posE], s, sizeof(COMANDO_SHARED));


	ReleaseMutex(hMutexEscrever);
	ReleaseSemaphore(d->hSemafroPodeLer, 1, NULL);
}

