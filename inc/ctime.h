#ifndef CTIME_H
#define CTIME_H

#include <time.h>  // Para poder usar o tipo time_t

// Definição da estrutura de dados para armazenar três timestamps
struct timestamps {
    time_t main_time;    // Marca de tempo para o processamento pela main
    time_t wallet_time;      // Marca de tempo para o processamento pela wallets
    time_t server_time; // Marca de tempo para o processamento pelo server
};


/* Função que muda o tempo quando a main processa a transação-
* O registo de tempos é feito com a funcao clock_gettime da biblioteca standart time.h
*/
void write_main_time(struct transaction* tx);

/* Função que muda o tempo quando a wallet processa a transação
* O registo de tempos é feito com a função clock_gettime da biblioteca standard time.h
*/
void write_wallet_time(struct transaction* tx);

/* Função que muda o tempo quando o server processa a transação
* O registo de tempos é feito com a função clock_gettime da biblioteca standard time.h
*/
void write_server_time(struct transaction* tx);

#endif // CTIME_H
