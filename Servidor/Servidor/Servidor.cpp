#include <tchar.h>
#include <Windows.h>
#include <io.h>
#include <iostream>
#include <stdio.h>
#include <fcntl.h>
#include <time.h>
#include "Jogo.h"

using namespace std;

int _tmain(int argc, LPTSTR argv[]) {

//Project Properties > Character Set > Use Unicode Character Set
#ifdef UNICODE
	_setmode(_fileno(stdin), _O_WTEXT);
	_setmode(_fileno(stdout), _O_WTEXT);
#endif

	JOGO jogo;

	_tprintf(TEXT("Olá Mundo!"));
	_gettchar();
}