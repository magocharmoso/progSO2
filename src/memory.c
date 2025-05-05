#include "memory.h"
#include "wallet.h"
#include "server.h"
#include <stdlib.h>
#include <stdio.h>
#include <sys/mman.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
/**
 * Grupo 002
 * Autor Rafael Tomé, 60237
 * Autor Jaime Sousa, 58171 
 * 
 */

/* Função que reserva uma zona de memória dinâmica com o tamanho indicado
 * por size, preenche essa zona de memória com o valor 0, e retorna um 
 * apontador para a mesma.
 */
void* allocate_dynamic_memory(int size) {
    void* ptr = malloc(size);
    if (ptr == NULL) {
        exit(EXIT_FAILURE);
    }
    return ptr;
}


/* Função que reserva uma zona de memória partilhada com tamanho indicado
 * por size e nome name, preenche essa zona de memória com o valor 0, e 
 * retorna um apontador para a mesma. Pode concatenar o id do utilizador 
 * resultante da função getuid() a name, para tornar o nome único para 
 * a zona de memória.
 */
void* create_shared_memory(char* name, int size) {
    shm_unlink(name);
    int shm_fd = shm_open(name, O_CREAT | O_RDWR| O_EXCL, 0666);
    if (shm_fd == -1) {
        perror("Erro em shm_open");
        return NULL;
    }
    if (ftruncate(shm_fd, size) == -1) {
        perror("Erro em ftruncate");    
    }
    void* ptr = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    close(shm_fd);
    memset(ptr, 0, size);
    return ptr;
}

/* Liberta uma zona de memória dinâmica previamente alocada.
 */
void deallocate_dynamic_memory(void* ptr) {
    free(ptr);
    ptr=NULL;
}

/* Remove uma zona de memória partilhada previamente criada. 
 */
void destroy_shared_memory(char* name, void* ptr, int size) {
    munmap(ptr, size);
    shm_unlink(name);
}

/* Escreve uma transação no buffer de memória partilhada entre a Main e as carteiras.
 * A transação deve ser escrita numa posição livre do buffer, 
 * tendo em conta o tipo de buffer e as regras de escrita em buffers desse tipo.
 * Se não houver nenhuma posição livre, não escreve nada e a transação é perdida.
 */
void write_main_wallets_buffer(struct ra_buffer* buffer, int buffer_size, struct transaction* tx) {
    if (*buffer->ptrs < buffer_size) {
        buffer->buffer[*buffer->ptrs] = *tx;
        (*buffer->ptrs)++;
    }
}

/* Função que escreve uma transação no buffer de memória partilhada entre as carteiras e os servidores.
 * A transação deve ser escrita numa posição livre do buffer, tendo em conta o tipo de buffer 
 * e as regras de escrita em buffers desse tipo. Se não houver nenhuma posição livre, não escreve nada.
 */
void write_wallets_servers_buffer(struct circ_buffer* buffer, int buffer_size, struct transaction* tx) { 
    if ((buffer->ptrs->in + 1) % buffer_size != buffer->ptrs->out) {
        buffer->buffer[buffer->ptrs->in] = *tx;
        buffer->ptrs->in = (buffer->ptrs->in + 1) % buffer_size;
    }
}

/* Função que escreve uma transação no buffer de memória partilhada entre os servidores e a Main, a qual 
 * servirá de comprovativo da execução da transação. A transação deve ser escrita numa posição livre do 
 * buffer, tendo em conta o tipo de buffer e as regras de escrita em buffers desse tipo. 
 * Se não houver nenhuma posição livre, não escreve nada.
 */
void write_servers_main_buffer(struct ra_buffer* buffer, int buffer_size, struct transaction* tx) {
    if (*buffer->ptrs < buffer_size) {
        buffer->buffer[*buffer->ptrs] = *tx;
        (*buffer->ptrs)++;
    }
}

/* Função que lê uma transação do buffer entre a Main e as carteiras, se houver alguma disponível para ler 
 * e que seja direcionada a própria carteira que está a tentar ler. A leitura deve ser feita tendo em conta 
 * o tipo de buffer e as regras de leitura em buffers desse tipo. Se não houver nenhuma transação disponível, 
 * afeta tx->id com o valor -1.
 */
void read_main_wallets_buffer(struct ra_buffer* buffer, int wallet_id, int buffer_size, struct transaction* tx) {
    if (*buffer->ptrs < buffer_size) {
        for (int i = 0; i < *buffer->ptrs; i++) {
            if (buffer->buffer[i].src_id == wallet_id) {
                *tx = buffer->buffer[i];
                for (int i = 1; i < *buffer->ptrs; i++) {
                    buffer->buffer[i - 1] = buffer->buffer[i];
                }    
                (*buffer->ptrs)--;
                return;
            }
        }
    }
    tx->id = -1;
}

/* Função que lê uma transação do buffer entre as carteiras e os servidores, se houver alguma disponível para ler.
 * A leitura deve ser feita tendo em conta o tipo de buffer e as regras de leitura em buffers desse tipo. Qualquer
 * servidor pode ler qualquer transação deste buffer. Se não houver nenhuma transação disponível, 
 * afeta tx->id com o valor -1.
 */
void read_wallets_servers_buffer(struct circ_buffer* buffer, int buffer_size, struct transaction* tx) {
    if (buffer->ptrs->in == buffer->ptrs->out) {
        tx->id = -1;
        return;
    }
    *tx = buffer->buffer[buffer->ptrs->out];
    buffer->ptrs->out = (buffer->ptrs->out + 1) % buffer_size;
    }

/* Função que lê uma transação do buffer entre os servidores e a Main, se houver alguma disponível para ler 
 * e que tenha o tx->id igual a tx_id. A leitura deve ser feita tendo em conta o tipo de buffer e as regras 
 * de leitura em buffers desse tipo. Se não houver nenhuma transação disponível, afeta tx->id com o valor -1.
 */
void read_servers_main_buffer(struct ra_buffer* buffer, int tx_id, int buffer_size, struct transaction* tx) {
    for (int i=0; i < *buffer->ptrs; i++) {
        if (buffer->buffer[i].id == tx_id && *buffer->ptrs < buffer_size) {
            *tx = buffer->buffer[i];
            for (int i = 1; i < *buffer->ptrs; i++) {
                buffer->buffer[i - 1] = buffer->buffer[i];
            }    
            (*buffer->ptrs)--;
            return;
        }
    }
    tx->id=-1;
}
