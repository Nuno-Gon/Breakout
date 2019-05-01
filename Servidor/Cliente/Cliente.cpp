#include "Cliente.h"

//Prototipos para Cliente
void createPipeCliente();
void escrevePipe(COMANDO_SHARED comando, HANDLE ioReady, OVERLAPPED ov, DWORD tam);


//VARIAVEIS GLOBAIS
HANDLE hpipe;
BOOL login = false;
scores ranking;
int pontosPlayer = 0;
TCHAR nomePlayer[100] = TEXT(" ");



//FUN��ES
//Fun��o Principal!
int _tmain(int argc, LPTSTR argv[]) {

	//Project Properties > Character Set > Use Unicode Character Set
#ifdef UNICODE
	_setmode(_fileno(stdin), _O_WTEXT);
	_setmode(_fileno(stdout), _O_WTEXT);
#endif

	//PIPE
	createPipeCliente();
	DWORD mode = PIPE_READMODE_MESSAGE;
	SetNamedPipeHandleState(hpipe, &mode, NULL, NULL);

	//Login
	TCHAR buf[256];

	COMANDO_SHARED comando;
	HANDLE ioReady;
	OVERLAPPED ov;
	DWORD tam = 0;

	ioReady = CreateEvent(NULL, TRUE, FALSE, NULL);

	do {
		_tprintf(TEXT("Nome utilizador: "));
		_fgetts(buf, 256, stdin);
		comando.idUser = 0;
		comando.tipo = CMD_LOGIN;
		comando.idHandle = hpipe;
		ZeroMemory(&ov, sizeof(ov));
		ResetEvent(ioReady);
		ov.hEvent = ioReady;


		escrevePipe(comando, ioReady, ov, tam);
		login = true;
	} while (login == false);
	
	_tprintf(TEXT("FIZ LOGIN!\n"));
	

	
	while (1) {

	}

	_tprintf(TEXT("\Terminei!\n"));
	//_gettchar();
}


// **** PIPES ****
//Criar Pipe
void createPipeCliente() {
	_tprintf(TEXT("[LEITOR] Esperar pelo pipe '%s' (WaitNamedPipe)\n"), PIPE_NAME);
	/*
	BOOL WaitNamedPipeA(
		LPCSTR lpNamedPipeName,
		DWORD  nTimeOut
	);
	*/
	if (!WaitNamedPipe(PIPE_NAME, NMPWAIT_WAIT_FOREVER)) {
		_tprintf(TEXT("[ERRO] Ligar ao pipe '%s'! (WaitNamedPipe)\n"), PIPE_NAME);
		//exit(-1);
	}

	_tprintf(TEXT("[LEITOR] Liga��o ao pipe do escritor... (CreateFile)\n"));
	/*
	HANDLE CreateFileA(
		LPCSTR                lpFileName,
		DWORD                 dwDesiredAccess,
		DWORD                 dwShareMode,
		LPSECURITY_ATTRIBUTES lpSecurityAttributes,
		DWORD                 dwCreationDisposition,
		DWORD                 dwFlagsAndAttributes,
		HANDLE                hTemplateFile
	);
	*/
	hpipe = CreateFile(PIPE_NAME, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0 | FILE_FLAG_OVERLAPPED, NULL);
	if (hpipe == NULL) {
		_tprintf(TEXT("[ERRO] Ligar ao pipe '%s'! (CreateFile)\n"), PIPE_NAME);
		//exit(-1);
	}

	_tprintf(TEXT("[LEITOR] Liguei-me...\n"));

}

//Escreve Pipe
void escrevePipe(COMANDO_SHARED comando, HANDLE ioReady, OVERLAPPED ov, DWORD tam) {

	if (!WriteFile(hpipe, &comando, sizeof(COMANDO_SHARED), &tam, &ov)) {
		_tprintf(TEXT("[ERRO] Escrever no pipe! (WriteFile)\n"));
		//exit(-1);
	}

	WaitForSingleObject(ioReady, INFINITE);
	/*
	BOOL GetOverlappedResult(
		HANDLE       hFile,
		LPOVERLAPPED lpOverlapped,
		LPDWORD      lpNumberOfBytesTransferred,
		BOOL         bWait
	);
	*/
	GetOverlappedResult(hpipe, &ov, &tam, FALSE);
	_tprintf(TEXT("[ESCRITOR] Enviei %d bytes ao pipe ...\n"), tam);
}



//Guardar TOP10 no Registo
void escreveRegistry() {

}

void leRegistry() {

}

void inserirRanking() {

}

void inicia() {

}

