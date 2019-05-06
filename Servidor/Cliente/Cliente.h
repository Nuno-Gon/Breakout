#pragma once

//Includes
#include <windowsx.h>
#include <windows.h>
#include <stdio.h>
#include <fcntl.h>
#include <io.h>
#include <tchar.h>
#include "..\Dll\dll.h"


#define MAX_LOADSTRING 100

#define PIPE_NAME TEXT("\\\\.\\pipe\\connect")


typedef struct {
	TCHAR nome[100];
	int score;
}jogador;

typedef struct {
	jogador jogadores[MAX_REGISTO];
}scores;



