#include "clog.h"
#include <stdlib.h>
#include <string.h>
#include <time.h>

static FILE *log_file = NULL;  // Só visível dentro de clog.c

int clog_init(void) {
    char log_filename[256] = {0};  // String para armazenar o nome do ficheiro de log
    FILE *settings = fopen("../settings.txt", "r");  // Abrir settings.txt para leitura

    if (!settings) {
        return -1;  // Não conseguiu abrir o ficheiro de configurações
    }

    char line[512];
    // Percorrer todas as linhas do ficheiro de configurações
    while (fgets(line, sizeof(line), settings)) {
        // Procurar a linha que contém 'log_filename = ...'
        if (sscanf(line, "log_filename = %255s", log_filename) == 1) {
            fclose(settings);  // Fecha o ficheiro após encontrar o nome
            if (log_filename[0] != '\0') {
                log_file = fopen(log_filename, "a");  // Abre o ficheiro de log em modo append
                if (log_file == NULL) {
                    return -2;  // Erro ao abrir o ficheiro de log
                }
                return 0;  // Inicialização bem-sucedida
            } else {
                fclose(settings);
                return -3;  // Não foi encontrado um nome válido para o ficheiro de log
            }
        }
    }

    fclose(settings);  // Fecha o ficheiro de configurações
    return -3;  // Não encontrou a linha 'log_filename' no ficheiro
}

void clog_write(const char *operation) {
    if (!log_file) return;

    struct timespec ts;
    struct tm *tm_info;
    char time_str[32];

    clock_gettime(CLOCK_REALTIME, &ts);
    tm_info = localtime(&ts.tv_sec);
    strftime(time_str, sizeof(time_str), "%Y%m%d %H:%M:%S", tm_info);

    fprintf(log_file, "%s.%03ld %s\n", time_str, ts.tv_nsec / 1000000, operation);
    fflush(log_file);  // Garantir que escreve no ficheiro
}

void clog_close(void) {
    if (log_file) {
        fclose(log_file);
        log_file = NULL;
    }
}
