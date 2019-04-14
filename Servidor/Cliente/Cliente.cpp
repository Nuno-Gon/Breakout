//Includes
#include <windows.h>
#include <stdio.h>
#include "..\Dll\dll.h"
#define PIPE_NAME TEXT("\\\\.\\pipe\\conect")


//Prototipos para Cliente

HANDLE hpipe;
void CreatePipeCliente();
BOOL login = false;



//FUNÇÕES
//Função Principal!
/* 
	docs.microsoft.com/en-us/windows/desktop/learnwin32/winmain--the-application-entry-point
	Every Windows program includes an entry-point function that is named either WinMain or wWinMain. Here is the signature for wWinMain.
	int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow);
*/
int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPWSTR    lpCmdLine,
	_In_ int       nCmdShow) {


	//Ciclo para começar a lidar com informações, connectar serv, lidar com a informação
	while (1) {

		

	}


}

// **** PIPES ****
//Criar Pipe
void CreatePipeCliente() {
}

//Guardar TOP10 no Registo
void escreveRegestry() {

}

void leRegestry() {

}

void inserirRanking() {

}

void inicia() {

}

