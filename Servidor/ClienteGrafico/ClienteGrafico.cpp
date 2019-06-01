// ClienteGrafico.cpp : Define o ponto de entrada para o aplicativo.
//
//Includes
#include "framework.h"
#include "ClienteGrafico.h"
#include <windowsx.h>
#include <windows.h>
#include <stdio.h>
#include <fcntl.h>
#include <io.h>
#include <tchar.h>
#include "..\Dll\dll.h"

#define MAX_LOADSTRING 100


//Prototipos para Cliente
void createPipeCliente();
void escrevePipe(COMANDO_SHARED comando, HANDLE ioReady, OVERLAPPED ov, DWORD tam);
BOOL verifica_ON();

//THREADS
DWORD WINAPI leMensagemJogo(void);

//VARIAVEIS GLOBAIS
HANDLE hpipe;
BOOL login = false;
int pontosPlayer = 0;
TCHAR nomePlayer[100] = TEXT(" ");
MensagemJogo msgJogo;

//thread
HANDLE thread_mensagem_jogo;
HANDLE thread_mensagem;


// Variáveis Globais:
HINSTANCE hInst;                                // instância atual
WCHAR szTitle[MAX_LOADSTRING];                  // O texto da barra de título
WCHAR szWindowClass[MAX_LOADSTRING];            // o nome da classe da janela principal


// Declarações de encaminhamento de funções incluídas nesse módulo de código:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);
	/*
    // TODO: Coloque o código aqui.
	//PIPE
	createPipeCliente();
	DWORD mode = PIPE_READMODE_MESSAGE;
	SetNamedPipeHandleState(hpipe, &mode, NULL, NULL);
	*/


	/*
	thread_mensagem_jogo = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)leMensagemJogo, NULL, 0, NULL);
	*/
    // Inicializar cadeias de caracteres globais
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_CLIENTEGRAFICO, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // Realize a inicialização do aplicativo:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_CLIENTEGRAFICO));

    MSG msg;

    // Loop de mensagem principal:
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    return (int) msg.wParam;
}

//
//  FUNÇÃO: MyRegisterClass()
//
//  FINALIDADE: Registra a classe de janela.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_CLIENTEGRAFICO));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(CreateSolidBrush(RGB(0, 0, 255)));
	// wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_CLIENTEGRAFICO);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

//
//   FUNÇÃO: InitInstance(HINSTANCE, int)
//
//   FINALIDADE: Salva o identificador de instância e cria a janela principal
//
//   COMENTÁRIOS:
//
//        Nesta função, o identificador de instâncias é salvo em uma variável global e
//        crie e exiba a janela do programa principal.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   hInst = hInstance; // Armazenar o identificador de instância em nossa variável global

   HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);


   /*
   HWND hWnd = CreateWindowW(szWindowClass, TEXT("Arkanoid"), WS_OVERLAPPEDWINDOW, // segundo parametro szTitle
	   CW_USEDEFAULT, 0,1000, 900, nullptr, nullptr, hInstance, nullptr);
	   */
   if (!hWnd)
   {
      return FALSE;
   }

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}

//
//  FUNÇÃO: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  FINALIDADE: Processa as mensagens para a janela principal.
//
//  WM_COMMAND  - processar o menu do aplicativo
//  WM_PAINT    - Pintar a janela principal
//  WM_DESTROY  - postar uma mensagem de saída e retornar
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	//Variaveis
	COMANDO_SHARED comando;
	HANDLE ioReady;
	OVERLAPPED ov;
	DWORD tam = 0;

	PAINTSTRUCT ps;
	ioReady = CreateEvent(NULL, TRUE, FALSE, NULL);
    switch (message)
    {
	case WM_CREATE: //Quando é chamado o createWindow


		break;
    case WM_COMMAND:
        {
			
		if (!verifica_ON())
			login = false;

		ZeroMemory(&ov, sizeof(ov));
		ResetEvent(ioReady);
		ov.hEvent = ioReady;

            int wmId = LOWORD(wParam);
            // Analise as seleções do menu:
            switch (wmId)
            {
			case IDM_LOGIN:
				if (login == false) {
					DialogBox(hInst, MAKEINTRESOURCE(IDD_LOGIN), hWnd, About);
					comando.idUser = 0;
					comando.tipo = CMD_LOGIN;
					comando.idHandle = hpipe;
					escrevePipe(comando, ioReady, ov, tam);
					login = true;
				}
				else {
					DialogBox(hInst, MAKEINTRESOURCE(IDD_LOGINFAIL), hWnd, About);
				}

				break;
            case IDM_ABOUT:
                DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
                break;

            case IDM_EXIT:
				//mandar mensagem ao servidor para inserir no ranking e dizer que desconectou !
				comando.idUser = 0;
				comando.tipo = CMD_LOGOUT;
				comando.idHandle = hpipe;
				login = false;


                DestroyWindow(hWnd);
                break;
            default:
                return DefWindowProc(hWnd, message, wParam, lParam);
            }
        }



        break;
    case WM_PAINT:
        { //pintar
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);
            // TODO: Adicione qualquer código de desenho que use hdc aqui...
            EndPaint(hWnd, &ps);
        }
        break;

	case WM_KEYDOWN:
		//Lidar com as teclas primidas
		break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
	case WM_CLOSE:

		break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

// Manipulador de mensagem para a caixa 'sobre'.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
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


BOOL verifica_ON() { //Meter no lado do servidor a funcionar
	for (int i = 0; i < MAX_NUM_PLAYERS; i++) {
		if (msgJogo.players[i].idHandle == hpipe)
			return true;
	}
	return false;
}