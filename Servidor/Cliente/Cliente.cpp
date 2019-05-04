#include "Cliente.h"

//Prototipos para Cliente
void createPipeCliente();
void escrevePipe(COMANDO_SHARED comando, HANDLE ioReady, OVERLAPPED ov, DWORD tam);
void lePipe();

//THREADS
DWORD WINAPI leMensagemJogo(void);


//VARIAVEIS GLOBAIS
HANDLE hpipe;
BOOL login = false;
scores ranking;
int pontosPlayer = 0;
TCHAR nomePlayer[100] = TEXT(" ");
MensagemJogo msgJogo;

//thread
HANDLE thread_mensagem_jogo;
HANDLE thread_mensagem;

//Regestry
HKEY hKey;
LPCTSTR sk = TEXT("SOFTWARE\\Breakout");
LPCTSTR value = TEXT("Scores");
void inicia();
void leRegistry();
void escreveRegistry();

//FUNÇÕES
//Função Principal!
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

	thread_mensagem_jogo = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)leMensagemJogo, NULL, 0, NULL);

	//TESTE REGISTRY
	inicia();

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
		exit(-1);
	}

	_tprintf(TEXT("[LEITOR] Ligação ao pipe do escritor... (CreateFile)\n"));
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
		exit(-1);
	}

	_tprintf(TEXT("[LEITOR] Liguei-me...\n"));

}

//Escreve Pipe
void escrevePipe(COMANDO_SHARED comando, HANDLE ioReady, OVERLAPPED ov, DWORD tam) {

	if (!WriteFile(hpipe, &comando, sizeof(COMANDO_SHARED), &tam, &ov)) {
		_tprintf(TEXT("[ERRO] Escrever no pipe! (WriteFile)\n"));
		exit(-1);
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
	LONG openRes = RegOpenKeyEx(HKEY_CURRENT_USER, sk, 0, KEY_ALL_ACCESS, &hKey);
	if (openRes == ERROR_SUCCESS) {
		_tprintf(TEXT("Success opening key."));
	}
	else {
		_tprintf(TEXT("Error opening key."));	}
	LONG setRes = RegSetValueEx(hKey, value, 0, REG_SZ, (LPBYTE)& ranking, sizeof(scores));
	if (setRes == ERROR_SUCCESS) {		_tprintf(TEXT("Success writing to Registry."));
	}
	else {
		_tprintf(TEXT("Error writing to Registry."));
	}
	LONG closeOut = RegCloseKey(hKey);
	if (closeOut == ERROR_SUCCESS) {
		_tprintf(TEXT("Success closing key."));
	}
	else {
		_tprintf(TEXT("Error closing key."));
	}
}

void leRegistry() {
	LONG openRes = RegOpenKeyEx(HKEY_CURRENT_USER, sk, 0, KEY_ALL_ACCESS, &hKey);
	if (openRes == ERROR_SUCCESS) {
		_tprintf(TEXT("Success opening key."));
	}
	else {
		_tprintf(TEXT("Error opening key."));
	}
	DWORD tam = sizeof(scores);
	RegGetValue(HKEY_CURRENT_USER, sk, value, RRF_RT_ANY, NULL, (PVOID)& ranking, &tam);
}

void inserirRanking() {
	//Inserir valor no rank
}

void inicia() {
	//Inicializar para mostrar
	swprintf_s(ranking.jogadores[0].nome, TEXT("Hugo"));
	ranking.jogadores[0].score = 10;

	for (int i = 1; i < 10; i++) {
		//_tcscpy_s(ranking.jogadores[i].nome, sizeof(100), "Nuno e  HUGO");
		swprintf_s(ranking.jogadores[i].nome, TEXT("Nuno"));
		ranking.jogadores[i].score = 5;
	}
	escreveRegistry();

	swprintf_s(ranking.jogadores[1].nome, TEXT("MANUEL"));
	ranking.jogadores[1].score = 6;

	leRegistry();
	_tprintf(TEXT("REGISTRY!\n TOP SCORE\n"));
	for (int i = 0; i < 10; i++) {

		_tprintf(TEXT("NOME: %s \t SCORE: %d\n"), ranking.jogadores[i].nome ,ranking.jogadores[i].score);

	}
		
	_tprintf(TEXT("\n"));
}


DWORD WINAPI leMensagemJogo(void) {
	HANDLE IoReady;
	OVERLAPPED ov;
	DWORD tam;
	IoReady = CreateEvent(NULL, TRUE, FALSE, NULL);
	int aux = 0;
	while (1) {
		if (login == TRUE) {
			ZeroMemory(&ov, sizeof(ov));
			ResetEvent(IoReady);
			ov.hEvent = IoReady;

			ReadFile(hpipe, &msgJogo, sizeof(MensagemJogo), &tam, &ov);

			_tprintf(TEXT("Posicao da Bola: (%d , %d)\n"), msgJogo.bola.coord.X, msgJogo.bola.coord.Y);
			Sleep(500);
			WaitForSingleObject(IoReady, INFINITE);
			GetOverlappedResult(hpipe, &ov, &tam, FALSE);
		}
	}
	return 0;
}

