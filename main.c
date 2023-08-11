/*
    Trabalho 1 - Organização e recuperação de dados
    Professora: Valeria Delisandra Feltrim
    Alunos: 
        Leonardo Venâncio Correia (RA: 129266)
        Murilo Luis Calvo Neves (RA: 129037)
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#define TAM_MINIMO_SOBRA 10  // Tamanho mínimo que a sobra de um registro deve possuir para ser reaproveitada
#define NOME_ARQUIVO_DADOS "dados.dat"

typedef enum {False, True} booleano;  // Enum booleano para deixar o código mais limpo

// Funções auxiliares
void inserir_espaco_na_led(int offset, short tamanho, FILE* arquivo_de_dados);

// Funções de registros
void inserir_registro(char* novo_registro, FILE* arquivo_de_dados);  
void remover_registro(char* identificador, FILE* arquivo_de_dados);  
void buscar_registro(char* identificador, FILE* arquivo_de_dados);  // A implementar

// Funções de modos de operações
void impressao_da_led(FILE* arquivo_de_dados);  // A implementar
void fazer_operacoes(FILE* arquivo_de_dados, FILE* arquivo_de_operacoes); 

int main(int argc, char *argv[]) {
    /*
        Ponto de entrada principal do programa
    */

    FILE *arquivo_de_dados = fopen(NOME_ARQUIVO_DADOS, "rb+");

    if (arquivo_de_dados == NULL)
    {
        fprintf(stderr, "\nNao foi encontrado o arquivo de dados para leitura");
        exit(EXIT_FAILURE);
    }
    
    if (argc == 3 && strcmp(argv[1], "-e") == 0) {

        printf("Modo de execucao de operacoes ativado ... nome do arquivo = %s\n", argv[2]);
        FILE *arquivo_de_operacoes = fopen(argv[2], "rb");  // Como dito no enunciado, assume-se que esse arquivo sempre é aberto corretamente
        fazer_operacoes(arquivo_de_dados, arquivo_de_operacoes);
        fclose(arquivo_de_operacoes);

    } else if (argc == 2 && strcmp(argv[1], "-p") == 0) {

        printf("Modo de impressao da LED ativado ...\n");
        //imprime_led(arquivo_de_dados);

    } else {
        fprintf(stderr, "Argumentos incorretos!\n");
        fprintf(stderr, "Modo de uso:\n");
        fprintf(stderr, "$ %s -e nome_arquivo\n", argv[0]);
        fprintf(stderr, "$ %s -p\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    fclose(arquivo_de_dados);
    return 0;
}


void fazer_operacoes(FILE* arquivo_de_dados, FILE* arquivo_de_operacoes)
{
    /*
    Lê e interpreta cada linha de comando descrita no arquivo de operações.
    */
    char comando;
    char parametro[1024];

    while (comando = fgetc(arquivo_de_operacoes), comando != EOF)
    {
        fseek(arquivo_de_operacoes, 1, SEEK_CUR);
        fgets(parametro, 1024, arquivo_de_operacoes);
        int tamanho_parametro = strlen(parametro);
        
        if (parametro[tamanho_parametro - 1] == '\n')
        {
            parametro[tamanho_parametro - 1] = '\0';
        }

        switch (comando)
        {
            case 'r':
                printf("\n");
                remover_registro(parametro, arquivo_de_dados);
                break;  
            case 'i': 
                printf("\n");
                inserir_registro(parametro, arquivo_de_dados);
                break;
            case 'b':
                printf("\n"); 
                //buscar_registro(parametro, arquivo_de_dados);
                break;
            default:
                printf("\n");
                printf("\nA operacao '%c' nao e uma operacao valida", comando);
                break;
        }  
    }
}


void ler_nome_registro(char *registro, char *nome)
{
    // Ler identificador do registro
    // *nome = strtok(registro, "|");
    //printf("\nLendo nome do registro [%s]", registro);

    if (registro[0] == '*')
    {
        nome[0] = '\0';
    }

    int i = 0;
    while (registro[i] != '|' && registro[i] != '\0')
    {
        nome[i] = registro[i];
        i++;
    }
    nome[i] = '\0';
}


void le_dados_led(int offset, FILE* arquivo_de_dados, short *tamanho, int *proximo_ponteiro)
{
    fseek(arquivo_de_dados, offset, SEEK_SET);
    fread(tamanho, sizeof(short), 1, arquivo_de_dados);
    fseek(arquivo_de_dados, 1, SEEK_CUR);  // Pulando o '*'
    fread(proximo_ponteiro, sizeof(int), 1, arquivo_de_dados);
}


void inserir_espaco_na_led(int offset, short tamanho, FILE* arquivo_de_dados)
{
    /*
    Insere um espaço vazio na LED.
    Coloca o tamanho e o asterisco e conecta na LED.
    'Tamanho' deve incluir o espaco para o '*' e o tamanho no inicio
    */
    fseek(arquivo_de_dados, offset, SEEK_SET);
    short tamanho_para_registro = tamanho - sizeof(short);
    fwrite(&tamanho_para_registro, sizeof(short), 1, arquivo_de_dados);
    fwrite("*", sizeof(char), 1, arquivo_de_dados);

    // Conectar na LED
    short tamanho_antigo = -1;
    int aponta_antigo = -1;
    
    short tamanho_atual = -1;
    int aponta_atual = -1;

    short tamanho_proximo = -1;
    int aponta_proximo = -1;

    fseek(arquivo_de_dados, 0, SEEK_SET); // Vai para o inicio do arquivo
    fread(&aponta_proximo, sizeof(int), 1, arquivo_de_dados);  // Lê o primeiro ponteiro da LED

    if (aponta_proximo == -1)
    {
        fseek(arquivo_de_dados, 0, SEEK_SET);
        fwrite(&offset, sizeof(int), 1, arquivo_de_dados);
        fseek(arquivo_de_dados, offset + sizeof(short) + 1, SEEK_SET);
        int ptr = -1;
        fwrite(&ptr, sizeof(int), 1, arquivo_de_dados);
        return;
    }

    aponta_atual = aponta_proximo;
    le_dados_led(aponta_atual, arquivo_de_dados, &tamanho_proximo, &aponta_proximo);
    
    while (aponta_proximo != -1 && tamanho_proximo > tamanho_para_registro) 
    {
        tamanho_antigo = tamanho_atual;
        tamanho_atual = tamanho_proximo;

        aponta_antigo = aponta_atual;
        aponta_atual = aponta_proximo;

        le_dados_led(aponta_atual, arquivo_de_dados, &tamanho_proximo, &aponta_proximo);
    }

    if (tamanho_proximo < tamanho_para_registro)
    {
        if (aponta_antigo != -1)
        {
            fseek(arquivo_de_dados, aponta_antigo + sizeof(short) + 1, SEEK_SET);
        }
        else
        {
            fseek(arquivo_de_dados, 0, SEEK_SET);
        }
        
        fwrite(&offset, sizeof(int), 1, arquivo_de_dados);

        fseek(arquivo_de_dados, offset + sizeof(short) + 1, SEEK_SET);
        fwrite(&aponta_atual, sizeof(int), 1, arquivo_de_dados);
    }
    else
    {
        fseek(arquivo_de_dados, aponta_atual + sizeof(short) + 1, SEEK_SET);
        fwrite(&offset, sizeof(int), 1, arquivo_de_dados);

        fseek(arquivo_de_dados, offset + sizeof(short) + 1, SEEK_SET);
        fwrite(&aponta_proximo, sizeof(int), 1, arquivo_de_dados);
    }
}


void inserir_registro(char* novo_registro, FILE* arquivo_de_dados)
{
    /*
    Insere um novo registro no arquivo
    */
    char nome_registro[64];

    ler_nome_registro(novo_registro, nome_registro);

    short tamanho_novo_registro = strlen(novo_registro);

    printf("\nInsercao do registro de chave \"%s\" (%d bytes)", nome_registro, tamanho_novo_registro);

    fseek(arquivo_de_dados, 0, SEEK_SET);  // Coloca ponteiro de leitura no inicio

    int offset_atual_led;
    short tamanho_atual_led;

    fread(&offset_atual_led, sizeof(int), 1, arquivo_de_dados);  // Lê o offset do primeiro elemento da LED

    if (offset_atual_led == -1)
    {
        // Inserção quando ainda não foi feita nenhuma remoção
        printf("\nLocal: Fim do arquivo");
        fseek(arquivo_de_dados, 0, SEEK_END);
        fwrite(novo_registro, tamanho_novo_registro, 1, arquivo_de_dados);
    }
    else
    {
        fseek(arquivo_de_dados, offset_atual_led, SEEK_SET);  // Vai até a posição de inserção
        fread(&tamanho_atual_led, sizeof(short), 1, arquivo_de_dados);

        if (tamanho_atual_led < tamanho_novo_registro)  // Sem espaço vazio que caiba o elemento a ser adicionado
        {
            printf("\nLocal: Fim do arquivo");
            fseek(arquivo_de_dados, 0, SEEK_END);
            fwrite(novo_registro, tamanho_novo_registro, 1, arquivo_de_dados);
        }
        else
        {
            fseek(arquivo_de_dados, 1, SEEK_CUR); // Pulando o '*'
            int proximo_ponteiro_led;

            fread(&proximo_ponteiro_led, sizeof(int), 1, arquivo_de_dados); // Lendo o proximo ponteiro da led
            fseek(arquivo_de_dados, offset_atual_led, SEEK_SET); // Voltando para o íncio do local onde o registro deverá ser escrito

            printf("\nLocal de insercao: offset = %d bytes", offset_atual_led);
            
            booleano sobrou_espaco_suficiente = False;

            // Escrevendo o novo registro

            short sobra = tamanho_atual_led - tamanho_novo_registro;

            if (sobra > TAM_MINIMO_SOBRA)
            {
                sobrou_espaco_suficiente = True;
                // Só escreve um novo tamanho caso haja suficiente para ser reutilizado
                fwrite(&tamanho_novo_registro, sizeof(short), 1, arquivo_de_dados);
            }
            else
            {
                // Caso contrário, só passa adiante
                fseek(arquivo_de_dados, sizeof(short), SEEK_CUR);
            }

            fwrite(novo_registro, tamanho_novo_registro, 1, arquivo_de_dados);

            fseek(arquivo_de_dados, 0, SEEK_SET); // Voltando para o início do arquivo
            fwrite(&proximo_ponteiro_led, sizeof(int), 1, arquivo_de_dados); // Conectando a ponta inicial da LED com a próxima

            fseek(arquivo_de_dados, offset_atual_led + tamanho_novo_registro + sizeof(short), SEEK_SET); // Voltando para onde o espaço vazio está

            printf("\nTamanho do espaco reutilizado: %d ", tamanho_atual_led);
            if (sobra > TAM_MINIMO_SOBRA)
            {
                printf("(sobra de %d bytes)", sobra - sizeof(short));
                // Caso ainda haja espaço utilizável
                inserir_espaco_na_led(offset_atual_led + tamanho_novo_registro + sizeof(short),
                tamanho_atual_led - tamanho_novo_registro, arquivo_de_dados);
            }
        }
    }
}


void remover_registro(char* identificador, FILE* arquivo_de_dados)
{
    /*
    Remove um registro do arquivo
    */
    
    printf("\nRemocao do registro de chave \"%s\"", identificador);

    char identificador_atual[64];
    char buffer[256];

    fseek(arquivo_de_dados, sizeof(int), SEEK_SET);  // Garantindo que o ponteiro de entrada esteja no início do primeiro registro

    short tamanho_registro;

    int res = 1;

    int posicao_do_ponteiro_de_leitura = sizeof(int);

    do
    {   
        fread(&tamanho_registro, sizeof(short), 1, arquivo_de_dados);

        fread(buffer, sizeof(char), tamanho_registro, arquivo_de_dados);

        ler_nome_registro(buffer, identificador_atual);

        int res = strcmp(identificador_atual, identificador);

        if (strcmp(identificador_atual, identificador) == 0)
        {
            printf("\nRegistro removido! Tamanho: %d bytes", tamanho_registro);
            printf("\nLocal: offset = %d bytes", posicao_do_ponteiro_de_leitura);
            inserir_espaco_na_led(posicao_do_ponteiro_de_leitura, tamanho_registro + sizeof(short), arquivo_de_dados);
            return;
        }

        posicao_do_ponteiro_de_leitura += sizeof(short) + tamanho_registro;

    } while (0 == feof(arquivo_de_dados));

    printf("\nErro: o registro nao foi encontrado");
}