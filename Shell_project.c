/**
UNIX Shell Project

Sistemas Operativos
Grados I. Informatica, Computadores & Software
Dept. Arquitectura de Computadores - UMA

Some code adapted from "Fundamentos de Sistemas Operativos", Silberschatz et al.

To compile and run the program:
   $ gcc Shell_project.c job_control.c -o Shell
   $ ./Shell          
	(then type ^D to exit program)

**/

/**
 * Author: Jesús Alcázar Pérez
 * Curso: 2ºA Informática
*/

#include "job_control.h"   	// remember to compile with module job_control.c 
#include <string.h>			//Para strcmp()
#include <stdio.h>			//Para la I/O estándar
#include <signal.h>			//Permite la función signal()
#include <stdlib.h>

#define MAX_LINE 256 /* 256 chars per line, per command, should be enough. */

//Inicializo la lista para poder usarla con el manejador
job * job_list;

// -----------------------------------------------------------------------
//                            HANDLER       
// -----------------------------------------------------------------------
//Creo el manejador para SIGCHLD y lo asigno
void handler(int sig){
	int status, info;
	enum status status_res;
	pid_t pid;
	
	for(int i=list_size(job_list); i>=1; i--){
		job * current_job = get_item_bypos(job_list, i);
		pid = waitpid(current_job->pgid, &status, WUNTRACED | WNOHANG | WCONTINUED);
		if(pid==current_job->pgid){
			status_res = analyze_status(status, &info);
				
			switch (status_res){
			case SUSPENDED:
				current_job->state = STOPPED;
				printf("Background job running... pid: %d, command: %s\n", current_job->pgid, current_job->command);
				break;
			case SIGNALED:
				printf("Background process %s (%d) %s \n", current_job->command, current_job->pgid, status_strings[status_res]);
				delete_job(job_list, current_job);
				break;
			case EXITED:
				printf("Background process %s (%d) %s \n", current_job->command, current_job->pgid, status_strings[status_res]);
				delete_job(job_list, current_job);
				break;
			case CONTINUED:
				current_job->state = BACKGROUND;
				printf("Background process %s (%d) %s \n", current_job->command, current_job->pgid, status_strings[status_res]);
				break;
			}
			//fflush(stdout);
		}
	}
}


// -----------------------------------------------------------------------
void parse_redirections(char **args, char **file_in, char **file_out){
	*file_in = NULL;
	*file_out = NULL;
	char **args_start = args;
	while (*args) {
		int is_in = !strcmp(*args, "<");
		int is_out = !strcmp(*args, ">");
		if (is_in || is_out) {
			args++;
			if (*args){
				if (is_in) *file_in = *args;
				if (is_out) *file_out = *args;
				char **aux = args + 1;
				while (*aux) {
					*(aux-2) = *aux;
					aux++;
				}
				*(aux-2) = NULL;
				args--;
			} else {
				/* Syntax error */
				fprintf(stderr, "syntax error in redirection\n");
				args_start[0] = NULL; // Do nothing
			}	
		} else {
			args++;
		}
	}
}
// -----------------------------------------------------------------------

// -----------------------------------------------------------------------
//                            MAIN          
// -----------------------------------------------------------------------

int main(void)
{
	char inputBuffer[MAX_LINE]; /* buffer to hold the command entered */
	int background;             /* equals 1 if a command is followed by '&' */
	char *args[MAX_LINE/2];     /* command line (of 256) has max of 128 arguments */
	// probably useful variables:
	int pid_fork, pid_wait; /* pid for created and waited process */
	int status;             /* status returned by wait */
	enum status status_res; /* status processed by analyze_status() */
	int info;				/* info processed by analyze_status() */

	//Ignoramos las señales del terminal para poder proceder con el programa
		ignore_terminal_signals();
	//Creo la lista de tareas en bg o suspendidas
		job_list = new_list("TareasShell_BG_ST");
	//Asignamos el manejador a la señal SIGCHLD
		signal(SIGCHLD, handler);
		//signal(SIGTTIN, handler);

	
	while (1)   /* Program terminates normally inside get_command() after ^D is typed*/
	{   		
		printf("COMMAND->");
		fflush(stdout);

		get_command(inputBuffer, MAX_LINE, args, &background);  /* get next command */
		//Comando para la implementacion de la redireccion de la entrada y salida
		char *file_in, *file_out;
		parse_redirections(args, &file_in, &file_out);


		if(args[0]==NULL) continue;   // if empty command

		/* the steps are:
			 (1) fork a child process using fork()
			 (2) the child process will invoke execvp()
			 (3) if background == 0, the parent will wait, otherwise continue 
			 (4) Shell shows a status message for processed command 
			 (5) loop returns to get_commnad() function
		*/
		else{
	//--------------------  COMANDOS INTERNOS  --------------------------------
		//cd
		if (strcmp(args[0], "cd") == 0){
			/* Caso para la redireccion como Bash
			char* dir = args[1];
			if(dir==NULL) { dir = "/root/PracticaShell"; }  //getenv("HOME"); Para uso general
			int res = chdir(dir);
			if(res != 0) { printf("Unable to reach %s directory \n", args[1]); }*/
			
			char* dir = args[1];
			int res;
			if(dir!=NULL){ 
				res = chdir(dir); 
				if(res != 0) { printf("Unable to reach %s directory \n", args[1]); }
			}
			
		}
		//jobs	
		else if (strcmp(args[0], "jobs") == 0){
			//print_job_list(job_list);
			(list_size(job_list)==0) ? printf("No programs in the list \n") : print_job_list(job_list);
		}
		//fg	
		else if(strcmp(args[0], "fg") == 0){
			int n = 1;
			if (args[1]!= NULL) { n = atoi(args[1]); }
			//Localizo trabajo de la lista
			job * current_job = get_item_bypos(job_list, n);
			if(current_job!=NULL){
				//printf("Valores de current_job: pid %d, command: %s, %s\n",
				//							current_job->pgid, current_job->command, state_strings[current_job->state]);
			//duplico el trabajo para poder usar los valores tras borrarlo
				job * auxJob = new_job(current_job->pgid, current_job->command, current_job->state);
				//printf("Valores de auxJob: pid %d, command: %s, %s, next: %p\n",
				//							auxJob->pgid, auxJob->command, state_strings[auxJob->state], auxJob->next);
			//Cedo el terminal al trabajo
				set_terminal(auxJob->pgid);
			//Actualiza lista
				block_SIGCHLD();
				delete_job(job_list, current_job);
				unblock_SIGCHLD();
			//Mando senal de continuar
				killpg(auxJob->pgid, SIGCONT);
			//Recoger proceso fin/suspendido
				pid_t pidAux = waitpid(auxJob->pgid, &status, WUNTRACED);				
			//Recupero Terminal
				set_terminal(getpid());
			//Actualiza lista
				status_res = analyze_status(status, &info);
				if(status_res==SUSPENDED){ 
					block_SIGCHLD();
					auxJob->state = STOPPED;
					add_job(job_list, auxJob); 
					unblock_SIGCHLD();
				}
			//Informo
				printf("Foreground pid: %d, command: %s, %s, info: %d \n",
									auxJob->pgid, auxJob->command, status_strings[status_res], info);
			} else { printf("ERROR: Command %s not found \n", args[1]); }
		}
		//bg
		else if(strcmp(args[0], "bg") == 0){
			//Localizo tarea n en la lista
			int n = 1;
			if (args[1]!= NULL) n = atoi(args[1]);
			job * current_job = get_item_bypos(job_list, n);
			if(current_job!=NULL){
				//Actualiza la lista
				current_job->state = BACKGROUND;
				//Mando señal de continuar
				killpg((current_job->pgid), SIGCONT);
				//Informo
				printf("Background pid: %d, command: %s\n", current_job->pgid, current_job->command);
			} else 
				printf("ERROR: Command %s not found\n", args[1]);
			
			
		} else {
	//--------------------  COMANDOS EXTERNOS  --------------------------------
			pid_fork = fork();				//Crea un nuevo hijo
			
			//Proceso Hijo
			if(pid_fork==0) {
				new_process_group(getpid()); 	//Crea un nuevo grupo para el hijo 
				if(file_in){ 
					freopen(file_in, "r", stdin);
					if((freopen(file_in, "r", stdin))== NULL){
						fprintf(stderr, "Error: abriendo %s \n", args[3]);
						exit(-1);
					}
				}
				if(file_out){ 
					freopen(file_out, "w", stdout); 
					if((freopen(file_out, "w", stdout)) == NULL){
						fprintf(stderr, "Error: abriendo %s \n", args[3]);
						exit(-1);
					}
				}
				if(!background){ set_terminal(getpid()); }
				restore_terminal_signals();
				execvp(args[0], args);
				printf("ERROR, command not found: %s \n", args[0]);
				exit(status);
			} else {
			
			//Proceso Padre
				if(background){
					block_SIGCHLD();
					add_job(job_list, new_job(pid_fork, args[0], background));	
					unblock_SIGCHLD();
					printf("Background job running... pid: %d, command: %s \n", pid_fork, args[0]);
				} else {
					pid_wait = waitpid(pid_fork, &status, WUNTRACED);
					status_res = analyze_status(status, &info);
					set_terminal(getpid());
					if(status_res==SUSPENDED){
						block_SIGCHLD();
						add_job(job_list, new_job(pid_fork, args[0], STOPPED));
						unblock_SIGCHLD();
					}
					printf("Foreground pid: %d, command: %s, %s, info: %d \n",
										pid_wait, args[0], status_strings[status_res], info);
				}
			}
		}
		}
	} // end while
}

