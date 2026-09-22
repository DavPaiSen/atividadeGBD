/*
 * Autores: seunome seunromatricula
    Júlia Yasmin Silva Guimarães 12421BCC013
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

class MeuArquivo {
public:
    struct cabecalho { int quantidade; int disponivel; } cabecalho;
    struct registro { int tamanho; char status; char palavra[50]; } registro;

    // construtor: abre arquivo. Essa aplicacao deveria ler o arquivo se existente ou criar um novo.
    // Entretando recriaremos o arquivo a cada execucao ("wb+").
    void atualizaCabecalho(){
        fseek(fd,0, SEEK_SET);
        fwrite(&cabecalho, size(struct cabecalho),1, fd);
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
        // implementar aqui
    }

    // Marca registro como removido, atualiza lista de disponíveis, incluindo o cabecalho
    void removePalavra(int offset) {
        // implementar aqui
    }

    // BuscaPalavra: retorno é o offset para o registro
    // Nao deve considerar registro removido
    int buscaPalavra(char *palavra) {
        // implementar aqui

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
