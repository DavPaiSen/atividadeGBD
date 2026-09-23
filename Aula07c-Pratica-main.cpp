/*
 * Autores:
    Júlia Yasmin Silva Guimarães 12421BCC013
    Davi Paiva Sendin 12421BCC004
    Leonardo Pereira da Silva 12421BCC026
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define ATIVO 'a'
#define REMOVIDO 'r'

class MeuArquivo {
public:
    struct cabecalho { int quantidade; int disponivel; } cabecalho;
    struct registro { int tamanho; char status; char palavra[50]; } registro;
    typedef struct {int tamanho; char status; int proximo;} blocoDesalocado;

    // construtor: abre arquivo. Essa aplicacao deveria ler o arquivo se existente ou criar um novo.
    // Entretando recriaremos o arquivo a cada execucao ("wb+").
    void atualizaCabecalho(){
        fseek(fd,0, SEEK_SET);
        fwrite(&cabecalho, sizeof(struct cabecalho),1, fd);
    }
    MeuArquivo() {
        fd = fopen("dados.dat","wb+");
        cabecalho.disponivel=0;
        cabecalho.quantidade=0;
        atualizaCabecalho();
    }

    // Destrutor: fecha arquivo
    ~MeuArquivo() {
        fclose(fd);
    }

    // Insere uma nova palavra, consulta se há espaco disponível ou se deve inserir no final
    void inserePalavra(char *palavra) {
        // 1) Copia a palavra sem '\n' / '\r' (fgets manteria a quebra de linha)
        char buffer[51];
        memset(buffer, 0, sizeof(buffer));
        int len = 0;
        while (len < 50 && palavra[len] != '\0' && palavra[len] != '\n' && palavra[len] != '\r') {
            buffer[len] = palavra[len];
            len++;
        }
        if (len == 0) return; // linha vazia

        // 2) Define o tamanho do registro
        // Quando removido, o registro vira um blocoDesalocado (tamanho+status+proximo)
        const int TAM_FIXO = sizeof(int) + 1;                    // campos tamanho + status
        const int TAM_MIN = sizeof(blocoDesalocado) - TAM_FIXO;  // menor "tamanho" permitido
        int tamanho = (len < TAM_MIN) ? TAM_MIN : len;
        int tamanhoRegistro = TAM_FIXO + tamanho;                // registro inteiro

        // 3) Procura na lista de removidos o primeiro bloco onde o registro cabe.
        //    offset 0 = cabecalho --> 0 = fim da lista.
        int offset = -1;
        int atual = cabecalho.disponivel;
        int offsetAnterior = 0;
        blocoDesalocado bloco = {0, 0, 0}, blocoAnterior = {0, 0, 0};

        while (atual != 0) {
            fseek(fd, atual, SEEK_SET);
            if (fread(&bloco, sizeof(blocoDesalocado), 1, fd) != 1) {
                break; // lista corrompida: insere no final
            }

            // bloco.tamanho guarda o tamanho do bloco inteiro (ver removePalavra)
            if (bloco.tamanho >= tamanhoRegistro) {
                // retira o bloco da lista de disponiveis
                if (offsetAnterior == 0) {
                    cabecalho.disponivel = bloco.proximo;
                } else {
                    blocoAnterior.proximo = bloco.proximo;
                    fseek(fd, offsetAnterior, SEEK_SET);
                    fwrite(&blocoAnterior, sizeof(blocoDesalocado), 1, fd);
                }
                offset = atual;
                // Mantem o tamanho do bloco: o que sobrar eh fragmentacao interna
                // preenchido com '\0' --> Arquivo continua percorrivel registro a registro
                tamanho = bloco.tamanho - TAM_FIXO;
                break;
            }

            blocoAnterior = bloco;
            offsetAnterior = atual;
            atual = bloco.proximo;
        }

        // 4) Nenhum bloco cabe ou lista vazia: insere no final do arquivo
        if (offset == -1) {
            fseek(fd, 0, SEEK_END);
            offset = ftell(fd);
        }

        // 5) Grava o registro: tamanho (binario), status (1 byte) e palavra (sem '\n')
        char status = ATIVO;
        fseek(fd, offset, SEEK_SET);
        fwrite(&tamanho, sizeof(int), 1, fd);
        fwrite(&status, 1, 1, fd);
        fwrite(buffer, 1, tamanho, fd);

        // 6) Atualiza o cabecalho
        cabecalho.quantidade++;
        atualizaCabecalho();
    }

    // Marca registro como removido, atualiza lista de disponíveis, incluindo o cabecalho
    void removePalavra(int offset) {
        fseek(fd, offset, SEEK_SET);

        int tamanho;
        if (fread(&tamanho, sizeof(int), 1, fd) != 1) { //nao conseguiu ler o bloco
            return;
        }

        blocoDesalocado blocoDesalocado;
        blocoDesalocado.tamanho = tamanho + sizeof(int) + 1;//tamanho eh o tamanho do registro inteiro
        blocoDesalocado.proximo = cabecalho.disponivel;
        blocoDesalocado.status = REMOVIDO;
        cabecalho.disponivel = offset;
        cabecalho.quantidade--;

        fseek(fd, offset, SEEK_SET);
        fwrite(&blocoDesalocado, sizeof(blocoDesalocado), 1, fd);

        atualizaCabecalho();
    }

    // BuscaPalavra: retorno é o offset para o registro
    // Nao deve considerar registro removido
    int buscaPalavra(char *palavra) {
        fseek(fd, 0, SEEK_SET);
        if (fread(&cabecalho, sizeof(struct cabecalho), 1, fd) != 1) {
            return -1; //nao conseguiu achar  o cabecalho
        }

        char buffer[51];
        int tamanho;
        char status;

        while (true) {
            int posicao = ftell(fd);
            if ((fread(&tamanho, sizeof(int), 1, fd) != 1) || (fread(&status, 1, 1, fd) != 1)) {
                break; //nao conseguiu ler o tamanho ou o status do registro
            }

            if (status == ATIVO) {
                fread(buffer, 1, tamanho, fd);
                buffer[tamanho] = '\0';

                if (!strcmp(buffer, palavra)) { //palavras iguais
                    return posicao;
                }
            } else {
                fseek(fd, posicao + tamanho, SEEK_SET); //pula o bloco
            }
            
        }

        // retornar -1 caso nao encontrar
        return -1;
    }

private:
    // descritor do arquivo é privado, apenas métodos da classe podem acessá-lo
    FILE *fd;
};

int main(int argc, char** argv) {
    // abrindo arquivo dicionario.txt
    FILE *f = fopen("dicionario.txt","rt");

    // se não abriu
    if (f == NULL) {
        printf("Erro ao abrir arquivo.\n\n");
        return 0;
    }

    char *palavra = new char[50];

    // criando arquivo de dados
    MeuArquivo *arquivo = new MeuArquivo();
    while (!feof(f)) {
        fgets(palavra,50,f);
        arquivo->inserePalavra(palavra);
    }

    // fechar arquivo dicionario.txt
    fclose(f);

    printf("Arquivo criado.\n\n");

    char opcao;
    do {
        printf("\n\n1-Insere\n2-Remove\n3-Busca\n4-Sair\nOpcao:");
        opcao = getchar();
        if (opcao == '1') {
            printf("Palavra: ");
            scanf("%s",palavra);
            arquivo->inserePalavra(palavra);
        }
        else if (opcao == '2') {
            printf("Palavra: ");
            scanf("%s",palavra);
            int offset = arquivo->buscaPalavra(palavra);
            if (offset >= 0) {
                arquivo->removePalavra(offset);
                printf("Removido.\n\n");
            }
        }
        else if (opcao == '3') {
            printf("Palavra: ");
            scanf("%s",palavra);
            int offset = arquivo->buscaPalavra(palavra);
            if (offset >= 0)
                printf("Encontrou %s na posição %d\n\n",palavra,offset);
            else
                printf("Não encontrou %s\n\n",palavra);
        }
        if (opcao != '4') opcao = getchar();
    } while (opcao != '4');

    printf("\n\nIsso eh tudo, pessoal!\n\n");

    return (EXIT_SUCCESS);
}
