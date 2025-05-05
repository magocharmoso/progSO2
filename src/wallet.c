#include <unistd.h>
#include <stdio.h>
#include "memory.h"
#include "main.h"
#include "wallet.h"
/**
 * Grupo 002
 * Autor Rafael Tomé, 60237
 * Autor Jaime Sousa, 58171 
 * 
 */
/* Função principal de uma carteira. Deve executar um ciclo infinito onde,
 * em cada iteração, lê uma transação da main apenas caso o src_id da transação seja
 * o seu próprio id. Se info->terminate ainda for 0, a carteira assina autorizando a transação, 
 * encaminhando-a para os servidores. Se a transação tiver um id igual a -1, é ignorada e espera-se 
 * alguns milisegundos antes de tentar ler uma nova transação do buffer. Se info->terminate for 1,
 * a execução termina e retorna o número de transações assinadas.
 */
int execute_wallet(int wallet_id, struct info_container* info, struct buffers* buffs){
    struct transaction tx;
    do {
        wallet_receive_transaction(&tx, wallet_id, info, buffs);
        if(tx.id == -1){
            usleep(500000);
        }
        else {
            wallet_process_transaction(&tx, wallet_id, info);
            wallet_send_transaction(&tx,info,buffs);
        }
    } while(*info->terminate == 0);
    return info->wallets_stats[wallet_id];

};

/* Função que lê uma transação do buffer de memória partilhada entre a main e as carteiras apenas 
 * caso o src_id da transação seja o seu próprio id. Antes de tentar ler a transação, deve verificar 
 * se info->terminate tem valor 1. Em caso afirmativo, retorna imediatamente da função.
 */
void wallet_receive_transaction(struct transaction* tx, int wallet_id, struct info_container* info, struct buffers* buffs){
    if (*info->terminate == 1) {
        return;
    }

    read_main_wallets_buffer(buffs->buff_main_wallets, wallet_id, info->buffers_size, tx);
};

/* Função que assina uma transação comprovando que a carteira de origem src_id da transação corresponde
 * ao wallet_id. Atualiza o campo wallet_signature da transação e incrementa o contador de transações 
 * assinadas pela carteira.
 */
void wallet_process_transaction(struct transaction* tx, int wallet_id, struct info_container* info){
    if(tx->src_id != wallet_id){
        return;
    }
    tx->wallet_signature = wallet_id; 
    info->wallets_stats[wallet_id]++;
    printf("[Wallet %d] Li a transação %d do buffer e a assinei!\n", wallet_id, tx->id);

    //Regista o tempo
    write_wallet_time(tx);
};

/* Função que escreve uma transação assinada no buffer de memória partilhada entre
 * as carteiras e os servidores. Se não houver espaço disponível no buffer, a transação
 * perde-se.
 */
void wallet_send_transaction(struct transaction* tx, struct info_container* info, struct buffers* buffs){
    write_wallets_servers_buffer(buffs->buff_wallets_servers, info->buffers_size, tx);
}
