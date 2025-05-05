#include "memory.h"
#include "process.h"
#include "main.h"
#include "wallet.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
/**
 * Grupo 002
 * Autor Rafael Tomé, 60237
 * Autor Jaime Sousa, 58171 
 * 
 */

/* Função que lê do stdin com o scanf apropriado para cada tipo de dados
 * e valida os argumentos da aplicação, incluindo o saldo inicial,
 * o número de carteiras, o número de servidores, o tamanho dos buffers
 * e o número máximo de transações. Guarda essa informação na estrutura info_container.
 */
void main_args(int argc, char *argv[], struct info_container *info)
{
    if (argc != 6)
    {
        printf("[Main] Uso: ./SOchain init_balance n_wallets n_servers buff_size max_txs\n"
               "[Main] Exemplo: ./SOchain 100.0 2 1 5 5\n\n");
        exit(1);
    }
    info->init_balance = atof(argv[1]);
    info->n_wallets = atoi(argv[2]);
    info->n_servers = atoi(argv[3]);
    info->buffers_size = atoi(argv[4]);
    info->max_txs = atoi(argv[5]);
    printf("[Main] Parâmetros corretos!\n\n");
}

/* Função que reserva a memória dinâmica necessária, por exemplo,
 * para os arrays *_pids de info_container. Para tal, pode ser usada
 * a função allocate_dynamic_memory do memory.h.
 */
void create_dynamic_memory_structs(struct info_container *info, struct buffers *buffs)
{
    info->wallets_pids = allocate_dynamic_memory(info->n_wallets * sizeof(int));
    info->servers_pids = allocate_dynamic_memory(info->n_servers * sizeof(int));
    
    buffs->buff_main_wallets = allocate_dynamic_memory(sizeof(struct ra_buffer));
    buffs->buff_servers_main = allocate_dynamic_memory(sizeof(struct ra_buffer));
    buffs->buff_wallets_servers = allocate_dynamic_memory(sizeof(struct circ_buffer));    

}

/* Função que reserva a memória partilhada necessária para a execução
 * do SOchain, incluindo buffers e arrays partilhados. É necessário
 * reservar memória partilhada para todos os buffers da estrutura
 * buffers, incluindo tanto os buffers em si como os respetivos
 * pointers, assim como para os arrays *_stats, balances e a variável
 * terminate do info_container. Pode ser usada a função
 * create_shared_memory do memory.h.
 */
void create_shared_memory_structs(struct info_container *info, struct buffers *buffs)
{   
    buffs->buff_main_wallets->ptrs = create_shared_memory(ID_SHM_MAIN_WALLETS_PTR, sizeof(int));
    buffs->buff_main_wallets->buffer = create_shared_memory(ID_SHM_MAIN_WALLETS_BUFFER, sizeof(struct transaction) * info->buffers_size);

    buffs->buff_wallets_servers->ptrs = create_shared_memory(ID_SHM_WALLETS_SERVERS_PTR, sizeof(struct pointers));
    buffs->buff_wallets_servers->buffer = create_shared_memory(ID_SHM_WALLETS_SERVERS_BUFFER, sizeof(struct transaction) * info->buffers_size);
    buffs->buff_wallets_servers->ptrs->in = 0;
    buffs->buff_wallets_servers->ptrs->out = 0;
    
    buffs->buff_servers_main->ptrs = create_shared_memory(ID_SHM_SERVERS_MAIN_PTR, sizeof(int));
    buffs->buff_servers_main->buffer = create_shared_memory(ID_SHM_SERVERS_MAIN_BUFFER, sizeof(struct transaction) * info->buffers_size);


    info->balances = create_shared_memory(ID_SHM_BALANCES, sizeof(int) * info->n_wallets);
    for (int i=0; i < info->n_wallets; i++) {
        info->balances[i] = info->init_balance;
    }
    info->wallets_stats = create_shared_memory(ID_SHM_WALLETS_STATS, sizeof(int) * info->buffers_size);
    info->servers_stats = create_shared_memory(ID_SHM_SERVERS_STATS, sizeof(int) * info->buffers_size);
    info->terminate = create_shared_memory(ID_SHM_TERMINATE, sizeof(int));
    if (!info->terminate) {
        fprintf(stderr, "Failed to create terminate\n");
        exit(EXIT_FAILURE);
    }
    *info->terminate=0;



}

/* Liberta a memória dinâmica previamente reservada. Pode utilizar a
 * função deallocate_dynamic_memory do memory.h
 */
void destroy_dynamic_memory_structs(struct info_container *info, struct buffers *buffs)
{
    deallocate_dynamic_memory(info->servers_pids);
    deallocate_dynamic_memory(info->wallets_pids);
    deallocate_dynamic_memory(buffs->buff_main_wallets);
    deallocate_dynamic_memory(buffs->buff_servers_main);
    deallocate_dynamic_memory(buffs->buff_wallets_servers);
}

/* Liberta a memória partilhada previamente reservada. Pode utilizar a
 * função destroy_shared_memory do memory.h
 */
void destroy_shared_memory_structs(struct info_container *info, struct buffers *buffs)
{
    destroy_shared_memory(ID_SHM_MAIN_WALLETS_BUFFER, buffs->buff_main_wallets->buffer, info->buffers_size);
    destroy_shared_memory(ID_SHM_SERVERS_MAIN_BUFFER, buffs->buff_servers_main->buffer, info->buffers_size);
    destroy_shared_memory(ID_SHM_WALLETS_SERVERS_BUFFER, buffs->buff_wallets_servers->buffer, info->buffers_size);
    destroy_shared_memory(ID_SHM_MAIN_WALLETS_PTR, buffs->buff_main_wallets->ptrs, sizeof(int));
    destroy_shared_memory(ID_SHM_MAIN_WALLETS_BUFFER, buffs->buff_main_wallets->buffer, sizeof(struct transaction) * info->buffers_size);
    destroy_shared_memory(ID_SHM_WALLETS_SERVERS_PTR, buffs->buff_wallets_servers->ptrs, sizeof(struct pointers));
    destroy_shared_memory(ID_SHM_WALLETS_SERVERS_BUFFER, buffs->buff_wallets_servers->buffer, sizeof(struct transaction) * info->buffers_size);
    destroy_shared_memory(ID_SHM_SERVERS_MAIN_PTR, buffs->buff_servers_main->ptrs, sizeof(int));
    destroy_shared_memory(ID_SHM_SERVERS_MAIN_BUFFER, buffs->buff_servers_main->buffer, sizeof(struct transaction) * info->buffers_size);
    destroy_shared_memory(ID_SHM_BALANCES, info->balances, sizeof(int) * info->n_wallets);
    destroy_shared_memory(ID_SHM_WALLETS_STATS, info->wallets_stats, sizeof(int) * info->buffers_size);
    destroy_shared_memory(ID_SHM_SERVERS_STATS, info->servers_stats, sizeof(int) * info->buffers_size);
    destroy_shared_memory(ID_SHM_TERMINATE, info->terminate, sizeof(int));
}

/* Função que cria os processos das carteiras e servidores.
 * Os PIDs resultantes são armazenados nos arrays apropriados
 * da estrutura info_container.
 */
void create_processes(struct info_container *info, struct buffers *buffs)
{
    for (int wallet_id=0; wallet_id < info->n_wallets; wallet_id++) {
        info->wallets_pids[wallet_id] = launch_wallet(wallet_id, info, buffs);
    }
    for (int server_id=0; server_id < info->n_servers; server_id++) {
        info->servers_pids[server_id] = launch_server(server_id, info, buffs);
    }
}

/* Função responsável pela interação com o utilizador.
 * Permite o utilizador pedir para visualizar saldos, criar
 * transações, consultar recibos, ver estatísticas do sistema e
 * encerrar a execução.
 */
void user_interaction(struct info_container *info, struct buffers *buffs)
{
    char user_in[20];
    int tx_counter = 0;
    while (1)
    {
        printf("\nIntroduzir operação: ");
        int scanfStatus = scanf("%19s", user_in);
        if (scanfStatus == 1) {
            if (strncmp(user_in, "bal", 3) == 0)
            {
                print_balance(info);
            }
            else if (strncmp(user_in, "trx", 3) == 0)
            {
                create_transaction(&tx_counter, info, buffs);
                sleep(1);
            }
            else if (strncmp(user_in, "rcp", 3) == 0)
            {
                receive_receipt(info, buffs);
            }
            else if (strcmp(user_in, "stat") == 0)
            {
                print_stat(tx_counter, info);
            }
            else if (strcmp(user_in, "help") == 0)
            {
                help();
            }
            else if (strcmp(user_in, "end") == 0)
            {
                end_execution(info, buffs);
            }
            else
            {
                printf("[Main] Operação não reconhecida, insira 'help' para assistência.\n");
            }
        }
        else {
            printf("[Main] Operação não reconhecida, insira 'help' para assistência.\n");
            help();
        }
    }
}

/* Função que imprime as estatísticas finais do SOchain, i{ncluindo
 * o número de transações assinadas por cada carteira e processadas
 * por cada servidor.
 */
void write_final_statistics(struct info_container *info)
{
    printf("[Main] A encerrar a execução do SOchain! As estatísticas da execução são:\n");
    for (int i = 0; i < info->n_wallets; i++)
    {
        printf("[Main] A carteira %d assinou %d transações e terminou com %.2f SOT!\n", i, info->wallets_stats[i], info->balances[i]);
    }
    for (int j = 0; j < info->n_servers; j++)
    {
        printf("[Main] O servidor %d assinou %d transações!\n", j, info->servers_stats[j]);
    }
}

/* Termina a execução do programa. Deve atualizar a flag terminate e,
 * em seguida, aguardar a terminação dos processos filhos, escrever as
 * estatísticas finais e retornar.
 */
void end_execution(struct info_container *info, struct buffers *buffs)
{
    *info->terminate = 1;
    write_final_statistics(info);
    wait_processes(info);
    destroy_shared_memory_structs(info, buffs);
    destroy_dynamic_memory_structs(info, buffs);
    exit(EXIT_SUCCESS);
}

/* Aguarda a terminação dos processos filhos previamente criados. Pode usar
 * a função wait_process do process.h
 */
void wait_processes(struct info_container *info)
{
    for (int i = 0; i < info->n_servers; i++)
    {
        wait_process(info->servers_pids[i]);
    }
    for (int j = 0; j < info->n_wallets; j++)
    {
        wait_process(info->wallets_pids[j]);
    }
}

/* Imprime o saldo atual de uma carteira identificada pelo id que ainda está
 * no stdin a espera de ser lido com o scanf dentro da função
 */
void print_balance(struct info_container *info) {
    int id;
    int scanfStatus = scanf("%d", &id);
    if (scanfStatus == 1 && id >= 0 && id < info->n_wallets)
        printf("O saldo da carteira %d é de %.2f SOT atualmente.", id, info->balances[id]);
    else
        printf("[Main] Não foi possível verificar o saldo da carteira %d\n", id);
}

/* Cria uma nova transação com os dados inseridos pelo utilizador na linha de
 * comandos (e que ainda estão no stdin a espera de serem lidos com o scanf
 * dentro da função), escreve-a no buffer de memória partilhada entre a main
 * e as carteiras e atualiza o contador de transações criadas tx_counter. Caso
 * a aplicação já tenha atingido o número máximo de transações permitidas
 * a função retorna apenas uma mensagem de erro e não cria a nova transação.
 */
void create_transaction(int *tx_counter, struct info_container *info, struct buffers *buffs)
{
    if (*tx_counter < info->max_txs)
    {   
        int src_id, dest_id;
        struct transaction tx;
        float amount;
        int scanfStatus = scanf("%d %d %f", &src_id, &dest_id, &amount);
        if (scanfStatus == 3) { 
            tx.amount = amount;
            tx.dest_id = dest_id;
            tx.id = *tx_counter;
            tx.server_signature = -1;
            tx.src_id = src_id;
            tx.wallet_signature = -1;
            printf("[Main] A transação %d foi criada para transferir %.2f SOT da carteira %d para a carteira %d!\n", *tx_counter, amount, src_id, dest_id);
            write_main_wallets_buffer(buffs->buff_main_wallets, info->buffers_size, &tx);
            (*tx_counter)++;
        }
        else {
            printf("[Main] Pedido mal formado");
        }
    }
    else
    {
        printf("[Main] Erro máximo de transações atingido");
    }
}

/* Tenta ler o recibo da transação (identificada por id, o qual ainda está no
 * stdin a espera de ser lido dentro da função com o scanf) do buffer de memória
 * partilhada entre os servidores e a main, comprovando a conclusão da transação.
 */
void receive_receipt(struct info_container *info, struct buffers *buffs)
{
    int tx_id;
    int scanfStatus = scanf("%d", &tx_id);
    if (scanfStatus == 1)
    {
        struct transaction tx;
        read_servers_main_buffer(buffs->buff_servers_main, tx_id, info->buffers_size, &tx);
        if (tx.id != -1) {
            printf("[Main] O comprovativo da execução da transação %d foi obtido.\n", tx.id);
            printf("[Main] O comprovativo da transação id %d contém src_id %d, dest_id %d, amount %.2f e foi assinado pela carteira %d e servidor %d.\n", tx.id, tx.src_id, tx.dest_id, tx.amount, tx.wallet_signature, tx.server_signature);

        }
        else {
            printf("[Main] O comprovativo da execução da transação %d não está disponível.", tx.id);
        }
    }
    else
    {
        printf("[Main] Valor inválido");
    }
}

/* Imprime as estatísticas atuais do sistema, incluindo as configurações iniciais
 * do sistema, o valor das variáveis terminate e contador da transações criadas,
 * os pids dos processos e as restantes informações (e.g., número de transações
 * assinadas pela entidade e saldo atual no caso das carteiras).
 */
void print_stat(int tx_counter, struct info_container *info)
{
    printf("- Configuração inicial:\n"
           "\tPropriedade\tValor\n");
    printf("\tinit_balance\t%f\n", info->init_balance);
    printf("\tn_wallets\t%d\n", info->n_wallets);
    printf("\tn_servers\t%d\n", info->n_servers);
    printf("\tbuffers_size\t%d\n", info->buffers_size);
    printf("\tmax_txs\t\t%d\n", info->max_txs);
    printf("- Variáveis atuais:\n");
    printf("\tterminate\t%d\n", *info->terminate);
    printf("\ttx_count\t%d\n", tx_counter);
    printf("- Informações sobre as carteiras:\n");
    printf("\tCarteira\tPID\tSaldo\t\tTransações Assinadas\n");
    for (int i = 0; i < info->n_wallets; i++)
    {
        printf("\t%d\t\t%d\t%.2f SOT\t%d\n", i, info->wallets_pids[i], info->balances[i], info->wallets_stats[i]);
    }
    printf("- Informações sobre os servidores:\n");
    printf("\tServidor\tPID\tTransações Processadas\n");
    for (int j = 0; j < info->n_servers; j++)
    {
        printf("\t%d\t\t%d\t%d\n", j, info->servers_pids[j], info->servers_stats[j]);
    }
}

/* Exibe informações sobre os comandos disponíveis na aplicação.
 */
void help()
{
    printf("\nComandos disponíveis:\n"
           "1. bal id - Obtém o saldo atual da carteira cujo identificador é id.\n"
           "2. trx src_id dest_id amount - Cria uma transação enviando 'amount' SOT tokens de 'src_id' para 'dest_id'.\n"
           "3. rcp id - Obtém o comprovativo da transação identificada por 'id'.\n"
           "4. stat - Mostra o estado atual das variáveis do info_container.\n"
           "5. help - Exibe informações sobre as operações disponíveis.\n"
           "6. end - Termina a execução do sistema SOchain.\n");
}

/* Função principal do SOchain. Inicializa o sistema, chama as funções de alocação
 * de memória, a de criação de processos filhos, a de interação do utilizador
 * e aguarda o encerramento dos processos para chamar as funções para libertar
 * a memória alocada.
 */
int main(int argc, char *argv[])
{ 
    struct info_container *info = malloc(sizeof(struct info_container));
    struct buffers *buffs = malloc(sizeof(struct buffers));
    if (info == NULL || buffs == NULL) {
        printf("[MAIN] Erro no info_container ou buffers\n");
        exit(EXIT_FAILURE);
    }
    main_args(argc, argv, info);
    create_dynamic_memory_structs(info, buffs);
    create_shared_memory_structs(info, buffs);
    create_processes(info, buffs);
    while (*info->terminate == 0)
    {
        user_interaction(info, buffs);
    }
}
