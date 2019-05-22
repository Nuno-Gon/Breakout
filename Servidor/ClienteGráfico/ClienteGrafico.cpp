//Includes
#include <windowsx.h>
#include <windows.h>
#include <stdio.h>
#include <fcntl.h>
#include <io.h>
#include <tchar.h>
#include "..\Dll\dll.h"

//Prototipos para Cliente
void createPipeCliente();
void escrevePipe(COMANDO_SHARED comando, HANDLE ioReady, OVERLAPPED ov, DWORD tam);

//THREADS
DWORD WINAPI leMensagemJogo(void);

//VARIAVEIS GLOBAIS
HANDLE hpipe;
BOOL login = false;
int pontosPlayer = 0;
TCHAR nomePlayer[100] = TEXT(" ");
MensagemJogo msgJogo;
HWND hWnd;

//thread
HANDLE thread_mensagem_jogo;
HANDLE thread_mensagem;

//FUNÇÕES
//Função Principal!
/*
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


	//while (1) {
		
	//}


	_gettchar();
	_tprintf(TEXT("\Terminei!\n"));
	//Mandar mensagem ao gateway e servidor a dizer que o cliente foi desconnectado
	system("pause");
}
*/
// Global Variables:
HINSTANCE hInst; //current instance
WCHAR szTitle[BUFFER_SIZE]; // The title bar text
WCHAR szWindowClass[BUFFER_SIZE]; // the main window class name

// Forward declarations of functions included in this code module:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK ListBoxExampleProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPWSTR    lpCmdLine,
	_In_ int       nCmdShow) 
{

	//PIPE
	createPipeCliente();
	DWORD mode = PIPE_READMODE_MESSAGE;
	SetNamedPipeHandleState(hpipe, &mode, NULL, NULL);

	//Verificar login

	thread_mensagem_jogo = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)leMensagemJogo, NULL, 0, NULL);


	MSG msg;

	while (1);

	return (int)msg.wParam;
	
}


//

//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEXW wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;

	//Criar cor
   //wcex.hIcon = LoadIcon(NULL, IDI_ERROR);  // icone na barra de tarefas
	//wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ICONGAME));
	wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(CreateSolidBrush(RGB(150, 70, 200)));
	// wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
	//wcex.lpszMenuName = MAKEINTRESOURCEW(IDC_CLIENTEGRAFICO);
	wcex.lpszClassName = szWindowClass;
	//wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_ICONGAME));

	return RegisterClassExW(&wcex);
}


//   FUNCTION: InitInstance(HINSTANCE, int)

//

//   PURPOSE: Saves instance handle and creates main window

//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
	hInst = hInstance; // Store instance handle in our global variable
	hWnd = CreateWindowW(szWindowClass, TEXT("Trabalho SO2"), WS_OVERLAPPEDWINDOW,
		0, 0, 530, 600, nullptr, nullptr, hInstance, nullptr);

	if (!hWnd){
		return FALSE;
	}

	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);

	return TRUE;

}

//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE:  Processes messages for the main window.
//
//  WM_COMMAND  - process the application menu
//  WM_PAINT    - Paint the main window
//  WM_DESTROY  - post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {

	return 0;
}


INT_PTR CALLBACK ListBoxExampleProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam) {
	return false;
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