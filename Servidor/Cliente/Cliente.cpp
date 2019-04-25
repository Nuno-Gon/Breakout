#include "Cliente.h"

//Prototipos para Cliente
void createPipeCliente();


//VARIAVEIS GLOBAIS
HANDLE hpipe;
BOOL login = false;
scores ranking;
int pontosPlayer = 0;



//FUNÇÕES
//Função Principal!
int _tmain(int argc, LPTSTR argv[]) {

	//Project Properties > Character Set > Use Unicode Character Set
#ifdef UNICODE
	_setmode(_fileno(stdin), _O_WTEXT);
	_setmode(_fileno(stdout), _O_WTEXT);
#endif 

	createPipeCliente();

	

	_tprintf(TEXT("\Terminei!\n"));
	_gettchar();
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

