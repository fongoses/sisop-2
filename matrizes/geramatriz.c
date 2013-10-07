/***********************************************************************
 * Universidade Federal do Rio Grande do Sul - Instituto de Informatica
 * Departamento de Informatica Aplicada
 * Sistemas Operacionais II - N - 2013/2
 * Professor: Alberto Egon Schaeffer Filho
 * Alunos:
 *     Luiz Gustavo Frozi de Castro e Souza
 *     Mario Cesar Gasparoni Junior
 ***********************************************************************
 * DESCRIÇÃO:
 * Programa auxiliar de geração de matrizes para casos de teste.
 * Gera um arquivo de saída de uma matri "A" de "m" linhas por "n"
 * colunas.
 * A matriz contém apenas números inteiros aleatórios de 0 a 100
 ***********************************************************************
 * USO:
 * $ geramatriz <n_linhas> <n_colunas> [-i] <arquivo_saida>
 *
 * <n_linhas> - Número de Linhas (m)
 * <n_colunas> - Número de Colunas (n)
 * -i - Opcional, especifica se a matriz a ser gerada deve ser a
 *      identidade
 * <arquivo_saida> - nome do arquivo de saída.
 ***********************************************************************
 * FORMATO DO ARQUIVO DE SAÍDA:
 * O arquivo de saída terá um cabeçalho com o número de linhas e o
 * número de colunas e, nas linhas subsequentes as linhas da matriz com
 * o valor das colunas separados por um espaço.
 *
 * LINHAS = #LINHAS
 * COLUNAS = #COLUNAS
 * A11 A12 A13 ... A1n
 * A21 A22 A23 ... A2n
 * A31 A32 A33 ... A3n
 * ... ... ... ... ...
 * Am1 Am2 Am3 ... Amn
 *
 * Ou em um exemplo:
 *
 * LINHAS = 3
 * COLUNAS = 3
 * 17 8 14
 * 4 23 9
 * 24 7 10
 *
 ************************************************************************
 * DATA DA ÚLTIMA ALTERAÇÃO:
 * 2013-10-06
 ************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define VALOR_MAXIMO 100

struct MATRIX {
    int linhas;
    int colunas;
    long *elemento;
};

int n_linhas = 0, n_colunas = 0;
struct MATRIX *A;
FILE *arquivo;

struct MATRIX *gerarIdentidade(int linhas, int colunas) {
}

struct MATRIX *gerarMatriz(int linhas, int colunas) {
}

void gravarArquivo(struct MATRIX M, FILE arq) {
}

int main(int argc, char *argv[]) {
    if(argc < 4) {
        printf("Uso: $ geramatriz <n_linhas> <n_colunas> [-i] <arquivo_saida>\n");
        return(1);
    }

    n_linhas = atoi(argv[1]);
    n_colunas = atoi(argv[2]);

    if(argc == 4) {
        if(strcmp(argv[3], "-i") != 0) {
            printf("Uso: $ geramatriz <n_linhas> <n_colunas> [-i] <arquivo_saida>\n");
            return(1);
        }

        arquivo = fopen(argv[4], "w");

        if(arquivo == NULL) {
            printf("Erro ao abrir o arquivo %s\n", argv[4]);
            return(2);
        }

        A = gerarIdentidade(n_linhas, n_colunas);
    } else {
        arquivo = fopen(argv[3], "w");

        if(arquivo == NULL) {
            printf("Erro ao abrir o arquivo %s\n", argv[3]);
            return(2);
        }

        A = gerarMatriz(n_linhas, n_colunas);
    }

    gravarArquivo(A, arquivo);

    close(arquivo);

    return(0);
}



