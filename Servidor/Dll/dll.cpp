//DLL.cpp
#include <windows.h>
#include "dll.h"

//Para verificar ao carregar a dll que a aplicação irá ocupar mais memória
char ponteiro[40960];

//definição da varíavel global
int nDLL = 1234;

//Funções (por ordem de declaração)




void readMensagem(dataCr* d, COMANDO_SHARED * s) {
	
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

void writeMensagem(dataCr* d, COMANDO_SHARED * s) {
	WaitForSingleObject(d->hSemafroPodeEscrever, INFINITE);
	WaitForSingleObject(hMutexEscrever, INFINITE);

	d->posE = d->shared->posWrite;
	d->shared->posWrite = (d->shared->posWrite + 1) % BUFFER;

	CopyMemory(&d->shared->PtrMemoria[d->posE], s, sizeof(COMANDO_SHARED));

	ReleaseMutex(hMutexEscrever);
	//ReleaseSemaphore(d->hSemafroPodeLer);

}
