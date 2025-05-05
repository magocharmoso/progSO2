#include "server.h"
#include "memory.h"
#include "main.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdlib.h>
/**
 * Grupo 002
 * Autor Rafael Tomé, 60237
 * Autor Jaime Sousa, 58171 
 * 
 */

int execute_server(int server_id, struct info_container* info, struct buffers* buffs) {
    int processed_transactions = 0;
    do {
        struct transaction tx;
        server_receive_transaction(&tx, info, buffs);
        if (tx.id == -1) {  
            usleep(5000);
        }
        else {
            server_process_transaction(&tx, server_id, info);
            if (tx.server_signature != -1) {
                printf("[Server %d] ledger <- [tx.id %d, src_id %d, dest_id %d, amount %.2f]\n", server_id, tx.id, tx.src_id, tx.dest_id, tx.amount);
                server_send_transaction(&tx, info, buffs);
            }else {
                printf("[Server %d] A transação %d falhou por alguma razão!\n", server_id, tx.id);
            }
                processed_transactions++; 
            }
        } while (*(info->terminate) == 0);
    
    return processed_transactions;
}

/* Função que lê uma transação do buffer de memória partilhada entre as carteiras e os servidores. Caso não
 * exista transações a serem lidas do buffer, retorna uma transação com o tx.id -1. Antes de tentar ler a 
 * transação do buffer, deve verificar se info->terminate tem valor 1. Em caso afirmativo, retorna imediatamente.
 */

void server_receive_transaction(struct transaction* tx, struct info_container* info, struct buffers* buffs) {
    if (*(info->terminate) == 1) {
        return;
    }

    read_wallets_servers_buffer(buffs->buff_wallets_servers, info->buffers_size, tx);
}

void server_process_transaction(struct transaction* tx, int server_id, struct info_container* info) {
    if (tx->amount <= 0 || tx->src_id < 0 || tx->dest_id < 0 ||
        tx->src_id >= info->n_wallets || tx->dest_id >= info->n_wallets) {
        tx->server_signature=-1;
        return;
    }

    if (info->balances[tx->src_id] < tx->amount) {
        tx->server_signature=-1;
        return;
    }

    if (tx->wallet_signature != tx->src_id) {
        tx->server_signature=-1;
        return;
    }
    info->balances[tx->src_id] -= tx->amount;
    info->balances[tx->dest_id] += tx->amount;
    info->servers_stats[server_id]++;
    tx->server_signature = server_id;
    printf("[Server %d] Li a transação %d do buffer e esta foi processada corretamente!\n", server_id, tx->id);
    return;
}
    

void server_send_transaction(struct transaction* tx, struct info_container* info, struct buffers* buffs) {
    if (tx->server_signature == -1) {
        return;
    }

    write_servers_main_buffer(buffs->buff_servers_main, info->buffers_size, tx);
}
