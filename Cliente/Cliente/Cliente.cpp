#include <tchar.h>
#include <Windows.h>
#include <io.h>
#include <iostream>
#include <stdio.h>
#include <fcntl.h>
#include <time.h>
#include <conio.h>

using namespace std;

int _tmain(int argc, LPTSTR argv[]) {
	HANDLE hPipe;
	char pid[100];
	/*
	// Sec attr for named pipe -> https://msdn.microsoft.com/en-us/library/windows/desktop/aa379560(v=vs.85).aspx
	SECURITY_ATTRIBUTES sa;
	ZeroMemory(&sa, sizeof(sa)); // -> https://msdn.microsoft.com/en-us/library/windows/desktop/aa366920(v=vs.85).aspx
	sa.nLength = sizeof(sa);
	sa.lpSecurityDescriptor = NULL;
	sa.bInheritHandle = FALSE;

	// named pipe name 
	string pipe_name = "\\\\.\\pipe\\";
	sprintf_s(pid, "%u", GetCurrentProcessId()); // -> %u unsigned int
	pipe_name.append(pid);
	printf("%s", pipe_name.c_str());

	hPipe = CreateNamedPipeA(pipe_name.c_str(), PIPE_ACCESS_INBOUND, PIPE_WAIT | PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE, 1, sizeof(pCli), sizeof(buf), 1000, NULL); // last param &sa if using SECURITY_ATTRIBUTES
	if (hPipe == INVALID_HANDLE_VALUE) {
		_tprintf(TEXT("[ERRO] Criar Named Pipe! (CreateNamedPipe)"));
		exit(-1);
	}*/
	//Project Properties > Character Set > Use Unicode Character Set
#ifdef UNICODE
	_setmode(_fileno(stdin), _O_WTEXT);
	_setmode(_fileno(stdout), _O_WTEXT);
#endif 

	/*HMODULE dll = LoadLibrary(L"");
	if (NULL != dll) {

	}
	else {
		cout << "";
	}*/

	

	_tprintf(TEXT("Olá Mundo!"));
	_gettchar();
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
	case WM_KEYDOWN:
	{
		switch (wParam)
		{
		case VK_LEFT:
		case VK_NUMPAD4:
			// move it left
			break;
		case VK_UP:
		case VK_NUMPAD8:
			// move it up
			break;
		case VK_RIGHT:
		case VK_NUMPAD6:
			// move it right
			break;
		case VK_DOWN:
		case VK_NUMPAD2:
			// move it down
			break;
		}

	}
	default:
		return DefWindowProc(hwnd, msg, wParam, lParam);
	}
	return 0;
}