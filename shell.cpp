/**
 * Codigo por: Maurício Benigno
 * Nome: MiniShell Linux
 * Para: Sistemas Operacionais II
 * Turma: CMP1075 - A01
 */
// -- [ BIBLIOTECAS C/LINUX ] -- 
// (Para uso das chamadas de sistema e tipos primitivos necessários)
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
// -- [ BIBLIOTECAS C++ ] -- 
//(Para uso do cout, string e stream para substituir o Split)
#include <iostream>
#include <sstream>
#include <cstring>
using namespace std;
// -- [ VARIÁVEIS GLOBAIS] --
int argumentos; // Usada para descobrir quantos argumentos o usuário utilizou
int argumentos1; // Usada para descobrir quantos argumentos o usuário utilizou com Pipe/ES
int argumentos2; // Usada para descobrir quantos argumentos o usuário utilizou com Pipe/ES
bool temPipeES;
bool executaBackground; // Usado para controle se processos irão executar em background ou não
// -- [ DEFINES GLOBAIS] --
#define MAX_ARGUMENTOS 10 // Usado para definir a quantidade maxima de argumentos que podem ser usados
// -- [ FUNÇÕES ] --
void terminar(int sinal); // Menu de escolha para o sinal de CTRL+C
void interpretadorComandos(string comandos[MAX_ARGUMENTOS]); //Interpreta os comandos digitados pelo usuário
void tratamentoPipeIO(string comandosP1[MAX_ARGUMENTOS],string comandosP2[MAX_ARGUMENTOS], int OP); // Trata os comandos identificados com pipe
char* removeEspacos(char *comandos); // Remove os espaços de palavras do tipo char.
// -- [ MAIN ] --
int main()
{
	string comando[MAX_ARGUMENTOS];
	string linha;
	int pid; // ID do processo	
	int status;
	(void) signal(SIGINT, terminar);  /* ITEM 4 - Captura sinal de CTRL+C e chama a função terminar*/ 
	
	while(1)
	{
		cout<<get_current_dir_name()<<"[MShell(pid:"<<getpid()<<")]$";
		getline(cin,linha);	
		/* Verifica se há algum Pipe, redirecionamento de ES ou pedido de execucao em Background*/
		temPipeES=false;
		executaBackground = false;
		for(int i=0; i<linha.size(); i++)
		{
			if(linha[i]=='|'||linha[i]=='>'||linha[i]=='<')
			{
				temPipeES=true;
				break;
			}
			if((i==linha.size()-1)&&linha[i]=='&')
			{
				cout<<" Back identificado."<<endl;
				executaBackground = true;
				linha[i]=' ';
			}
		}
		/* Separando os comandos na linha e colocando em vetor*/
		int i = 0;
		stringstream ssin(linha);
		argumentos=0;
    	while (ssin.good() && i < MAX_ARGUMENTOS) // Realiza a função de Split
		{
			string aux;
        	ssin >> aux; 
        	if(aux!="")
			{
				comando[argumentos]=aux;
				argumentos++;
			}	
        	++i;
    	}
		/* Passa o vetor de argumentos para o interpretador de comandos */	
		interpretadorComandos(comando);		
	}
	
	cout<<endl<<"Shell finalizado!"<<endl;
	return 0;
}
// -- [ INTERPRETADOR DE COMANDOS ] --
void interpretadorComandos(string comandos[MAX_ARGUMENTOS])
{
	if(temPipeES==true) /* ITEM 5 - Pipe e redirecionamento de entrada e saida*/ 
	{
		string comandos1[MAX_ARGUMENTOS];
		string comandos2[MAX_ARGUMENTOS];
		int tipo = 0;
		int k = 0;
		argumentos1=0;
		argumentos2=0;
		/* Laço de repetição que identifica se existe algum Pipe, entrada ou saida*/
		for(int i=0;i<argumentos; i++){
			k++;
			argumentos1++;
			comandos1[i] = comandos[i];
			if(comandos1[i]==">"){ // Identifica OUTPUT e atribui a ele ID 1
				comandos1[i] = "";
				argumentos1--;
				tipo=1;
				break;
			}
			else if(comandos1[i]=="<"){ // Identifica INPUT e atribui a ele ID 2
				comandos1[i] = "";
				argumentos1--;
				tipo=2;
				break;
			}
			else if(comandos1[i]=="|"){ // Identifica PIPE e atribui a ele ID 3
				comandos1[i] = "";
				argumentos1--;
				tipo=3;
				break;
			}	
		}
		for(int i=0; i< argumentos; i++){
			argumentos2++;
			comandos2[i] = comandos[i+k];
		}	
		//Chamando a função que faz contrle de IO e Pipe
		tratamentoPipeIO(comandos1,comandos2,tipo);
	}
	else if(comandos[0]=="exit"||comandos[0]=="sair") /* ITEM 1 - Terminar shell com comando exit*/ 
	{
		cin.ignore();
		cout<<"finalizando Shell"<<endl;
		exit (0);
	}
	else if(comandos[0]=="cd") /* ITEM 2 - Mudança de diretorio*/ 
	{
		const char * c = comandos[1].c_str();
		chdir(c);
	}
	else if(comandos[0]=="exec") /* ITEM 3 - Comando exec + comandos e termina*/ 
	{
		pid_t pid, wpid;
		int status;
		pid = fork(); //cria filho com essa chamada
		if (pid == 0) // Sempre que um processo filho é criado, seu pid fica com valor zerado
		{
			char *argv[argumentos-1];
			for(int i=1; i< argumentos; i++){ // convertendo cada argumento string para char*, porém pulando o comando exec
				argv[i-1] = new char[comandos[i].length() + 1]; 
				strcpy(argv[i-1], comandos[i].c_str());
			}
			argv[argumentos-1] = NULL;
			if (execvp(argv[0], argv) == -1) // Executa o comando e os argumentos com essa chamada
			{
				cout<<"O comando difitado é invalido!"<<endl;
				exit (0); // termina o processo filho caso o comando digitado seja invalido
			}
			exit (0); // Após executar o comando, ele termina o processo filho com essa chamada de sistema
		}
		else 
		{
			waitpid(-1,&status,0); // aguarda o termino do filho com essa chamada
			exit (0); // termina o processo pai com essa chama de sistema
		}
	}
	else if(comandos[0]=="shell") // Só um comando de teste que inicia um novo Shell sem finalizar o shell antigo
	{
		int pid = fork();	
		if(pid < 0)
			cout<<" Deu erro no fork"<<endl;
		if(pid!=0)
		{
			//waitpid(-1,&status,0);	
		}		
		else
		{
			char *argv[1];
			argv[0] = new char[comandos[0].length() + 1]; 
			strcpy(argv[0], comandos[0].c_str());
			execve("./shell",argv,0);
			pid = wait(0);
		}
	}
	else /* TODO COMANDO QUE NÃO FOI MAPEADO, CAI NESTA CONDIÇÃO E EXECUTA*/
	{
		pid_t pid, wpid;
		int status;
		pid = fork(); //cria filho com essa chamada
		if (pid == 0) // Sempre que um processo filho é criado, seu pid fica com valor zerado
		{
			char *argv[argumentos];
			for(int i=0; i< argumentos; i++){ // convertendo cada argumento string para char*, porém pulando o comando exec
				argv[i] = new char[comandos[i].length() + 1]; 
				strcpy(argv[i], comandos[i].c_str());
			}
			
			argv[argumentos] = NULL;
			if (execvp(argv[0], argv) == -1)  // Executa o comando e os argumentos com essa chamada
			{
				cout<<"Comando invalido!"<<endl;
				exit (0); // termina o processo filho caso o comando digitado seja invalido
			}
			exit (0);// termina o processo filho após a execução
		}
		else 
		{
			if(!executaBackground)
				waitpid(-1,&status,0); // aguarda o termino do filho com essa chamada
		}
	}	
}

void terminar(int sig) /* ITEM 4 - Tratamento da captura de sinal, com S/N para cancelar ou finalizar o Shell*/ 
{  
    for(;;)
    {
        cout<<"\nDeseja mesmo encerrar esse Shell? S para confirmar\n"<<endl;
        char aux=getchar();
        if(aux=='S' || aux=='s' )
        {
            exit(0); // Essa chamada termina o shell, caso a chamada seja positiva
        }
        if(aux=='N' || aux=='n' )
        {
            break; // Esse comando interrompe o laço e permite a continuidade de execução do shell
        }
    }
}
// -- [ TRATAMENTO DOS PROCESSOS PIPE E ENTRADA/SAIDA ] --
void tratamentoPipeIO(string comandosP1[MAX_ARGUMENTOS],string comandosP2[MAX_ARGUMENTOS], int OP)
{
	switch(OP)
	{
		case 1:
			temPipeES==false;
			int vetor_pipe_[2];
	        if(pipe(vetor_pipe_) == -1) {
	          perror("Pipe failed");
	          exit(1);
	        }
	        if(fork() == 0) // Primeiro filho que executa o primeiro comando da lista e grava na memoria
	        {	            
	            char *argv[argumentos1];
				for(int i=0; i< argumentos1; i++){ // convertendo cada argumento string para char*, porém pulando o comando exec
					argv[i] = new char[comandosP1[i].length() + 1]; 
					strcpy(argv[i], comandosP1[i].c_str());
				}
				argv[argumentos1] = NULL;
				
				close(STDOUT_FILENO); // Desliga a saída de dados na tela
	            dup(vetor_pipe_[1]);   // Direciona a saida que ia pra tela, para a memoria do Pipe
	            close(vetor_pipe_[0]); // Fecha o pipe de leitura
	            close(vetor_pipe_[1]); // Fecha o pipe de escrita
				
				if (execvp(argv[0], argv) == -1)  // Executa o comando e os argumentos com essa chamada
				{
					cout<<"Comando invalido XX!"<<endl;
					exit (0); // termina o processo filho caso o comando digitado seja invalido
				}
	        }
			wait(0); // O processo pai espera o termino do primeiro filho, antes de executar o segundo
			if(fork() == 0) //Segundo filho que executa comandos lendo dados a partir da memoria
	        {
	            int contador =0;
	            char *argv[argumentos2];
				for(int i=0; i< argumentos2; i++){ // convertendo cada argumento string para char*, porém pulando o comando exec
					argv[i] = new char[comandosP2[i].length() + 1]; 
					strcpy(argv[i], comandosP2[i].c_str());
					cout<<" -> "<<argv[i]<<" contador: "<<contador<<endl;
					if(comandosP2[i]=="")
					{ }
					else
						contador++;				
				}

			 	char *argv2[contador];
				for(int i=0; i<contador; i++)
					argv2[i]=argv[i];
				argv2[contador]=NULL;	
	            
	            close(STDIN_FILENO); // Desliga a entrada de dados pelo teclado
	            dup(vetor_pipe_[0]);  // Direciona a entrada do teclado pela memoria do Pipe
	            close(vetor_pipe_[1]);// Fecha o pipe de escrita
	            close(vetor_pipe_[0]);// Fecha o pipe de leitura
	            

	            /*cout<<"DUP: "<<vetor_pipe_[0]<<endl;
	            
	            if (execvp(argv2[0], argv2) == -1)  // Executa o comando e os argumentos com essa chamada
				{
					cout<<"Comando invalido!"<<endl;
					exit (0); // termina o processo filho caso o comando digitado seja invalido
				}*/
	        }
	
	        close(vetor_pipe_[0]);
	        close(vetor_pipe_[1]);
	        wait(0);
	        wait(0);
			break;
		case 2:
			
			break;
		case 3:
			temPipeES==false;
			int vetor_pipe[2];
	        if(pipe(vetor_pipe) == -1) {
	          perror("Pipe failed");
	          exit(1);
	        }
	        if(fork() == 0) // Primeiro filho que executa o primeiro comando da lista e grava na memoria
	        {	            
	            char *argv[argumentos1];
				for(int i=0; i< argumentos1; i++){ // convertendo cada argumento string para char*, porém pulando o comando exec
					argv[i] = new char[comandosP1[i].length() + 1]; 
					strcpy(argv[i], comandosP1[i].c_str());
				}
				argv[argumentos1] = NULL;
				
				close(STDOUT_FILENO); // Desliga a saída de dados na tela
	            dup(vetor_pipe[1]);   // Direciona a saida que ia pra tela, para a memoria do Pipe
	            close(vetor_pipe[0]); // Fecha o pipe de leitura
	            close(vetor_pipe[1]); // Fecha o pipe de escrita
				
				if (execvp(argv[0], argv) == -1)  // Executa o comando e os argumentos com essa chamada
				{
					cout<<"Comando invalido!"<<endl;
					exit (0); // termina o processo filho caso o comando digitado seja invalido
				}
	        }
			wait(0); // O processo pai espera o termino do primeiro filho, antes de executar o segundo
			if(fork() == 0) //Segundo filho que executa comandos lendo dados a partir da memoria
	        {
	            int contador =0;
	            char *argv[argumentos2];
				for(int i=0; i< argumentos2; i++){ // convertendo cada argumento string para char*, porém pulando o comando exec
					argv[i] = new char[comandosP2[i].length() + 1]; 
					strcpy(argv[i], comandosP2[i].c_str());
					if(comandosP2[i]=="")
					{ }
					else
						contador++;				
				}

			 	char *argv2[contador];
				for(int i=0; i<contador; i++)
					argv2[i]=argv[i];
				argv2[contador]=NULL;	
	            
	            close(STDIN_FILENO); // Desliga a entrada de dados pelo teclado
	            dup(vetor_pipe[0]);  // Direciona a entrada do teclado pela memoria do Pipe
	            close(vetor_pipe[1]);// Fecha o pipe de escrita
	            close(vetor_pipe[0]);// Fecha o pipe de leitura
	            
	            if (execvp(argv2[0], argv2) == -1)  // Executa o comando e os argumentos com essa chamada
				{
					cout<<"Comando invalido!"<<endl;
					exit (0); // termina o processo filho caso o comando digitado seja invalido
				}
	        }
	
	        close(vetor_pipe[0]);
	        close(vetor_pipe[1]);
	        wait(0);
	        wait(0);
			break;
	}
	// limpar vetores e contadores
	argumentos = 0;
	argumentos1 = 0;
	argumentos2 = 0;
	temPipeES = false;
	
}
