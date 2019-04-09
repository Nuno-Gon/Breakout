#include <tchar.h>
#include <Windows.h>
#include <io.h>
#include <stdio.h>
#include <fcntl.h>
#include <time.h>

int _tmain(int argc, LPTSTR argv[]) {

//Project Properties > Character Set > Use Unicode Character Set
#ifdef UNICODE
	_setmode(_fileno(stdin), _O_WTEXT);
	_setmode(_fileno(stdout), _O_WTEXT);
#endif

	_tprintf(TEXT("Olá Mundo!"));

	_gettchar(); //pause
}