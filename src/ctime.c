#include "ctime.h"
#include "transaction.h"  // Inclui o header que define a estrutura transaction
#include <stdio.h>
#include <time.h>

/* Função que muda o tempo quando a main processa a transação
 * O registo de tempos é feito com a função clock_gettime da biblioteca standard time.h
 */
void write_main_time(struct transaction* tx) {
    struct timespec ts;

    // Obtém o tempo atual usando clock_gettime
    if (clock_gettime(CLOCK_REALTIME, &ts) == -1) {
        perror("Erro ao obter o tempo com clock_gettime");
        return;
    }

    // Atualiza o tempo da transação na estrutura tx (usando segundos)
    tx->change_time.main_time = ts.tv_sec;

    // Se necessário, adicione um print ou log para verificar o valor
    //printf("Tempo registrado pela main: %ld\n", ts.tv_sec);
}

/* Função que muda o tempo quando a wallet processa a transação
 * O registo de tempos é feito com a função clock_gettime da biblioteca standard time.h
 */
void write_wallet_time(struct transaction* tx) {
    struct timespec ts;

    // Obtém o tempo atual usando clock_gettime
    if (clock_gettime(CLOCK_REALTIME, &ts) == -1) {
        perror("Erro ao obter o tempo com clock_gettime");
        return;
    }

    // Atualiza o tempo da transação na estrutura tx (usando segundos)
    tx->change_time.wallet_time = ts.tv_sec;

    // Se necessário, adicione um print ou log para verificar o valor
    //printf("Tempo registrado pela wallet: %ld\n", ts.tv_sec);
}

/* Função que muda o tempo quando o server processa a transação
 * O registo de tempos é feito com a função clock_gettime da biblioteca standard time.h
 */
void write_server_time(struct transaction* tx) {
    struct timespec ts;

    // Obtém o tempo atual usando clock_gettime
    if (clock_gettime(CLOCK_REALTIME, &ts) == -1) {
        perror("Erro ao obter o tempo com clock_gettime");
        return;
    }

    // Atualiza o tempo da transação na estrutura tx (usando segundos)
    tx->change_time.server_time = ts.tv_sec;

    // Se necessário, adicione um print ou log para verificar o valor
    //printf("Tempo registrado pelo server: %ld\n", ts.tv_sec);
}
