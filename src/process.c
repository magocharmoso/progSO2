#include "memory.h"
#include "main.h"
#include "process.h"
#include "server.h"
#include "wallet.h"
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
/**
 * Grupo 002
 * Autor Rafael Tomé, 60237
 * Autor Jaime Sousa, 58171 
 * 
 */

/* Função que inicia um novo processo Wallet através da função fork do SO. O novo
 * processo filho irá executar a função execute_wallet respetiva, fazendo exit do retorno.
 * O processo pai devolve nesta função o pid do processo criado.
 */
int launch_wallet(int wallet_id, struct info_container* info, struct buffers* buffs){
    int pid = fork();
    if (pid > 0){
        return pid; 
    }
    else if(pid==0){
        int wallet_response = execute_wallet(wallet_id, info, buffs);
        exit(wallet_response);
    }
    else{
        return -1;
    }
    
};

/* Função que inicia um novo processo Server através da função fork do SO. O novo
 * processo filho irá executar a função execute_server, fazendo exit do retorno.
 * O processo pai devolve nesta função o pid do processo criado.
 */
int launch_server(int server_id, struct info_container* info, struct buffers* buffs){
    int pid = fork();
    if (pid > 0){
        return pid;
    }
    else if(pid==0){
        int server_response = execute_server(server_id, info, buffs); 
        exit(server_response);
    }
    else{
        return -1;
    }
};

/* Função que espera que um processo com PID process_id termine através da função waitpid. 
 * Devolve o retorno do processo, se este tiver terminado normalmente.
 */
int wait_process(int process_id){
    int status;


    if (waitpid(process_id, &status, 0) == -1) {
        return -1;
    }

    if (WIFEXITED(status)) {
        return WEXITSTATUS(status);
    } else {
        return -1;
    }
};
