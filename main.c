#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_ITENS 100
#define TAM_NOME 100

typedef struct {
	char nome[TAM_NOME];
	int quantidade;
} Item;

/* Lê uma linha do teclado, descarta o excesso e remove o \n.
   Retorna 0 se a entrada acabou (EOF), 1 caso contrário. */
static int ler_linha(const char *mensagem, char *destino, int tamanho) {
	if (mensagem) printf("%s", mensagem);
	if (!fgets(destino, tamanho, stdin)) return 0;
	if (!strchr(destino, '\n')) {
		int c;
		while ((c = getchar()) != '\n' && c != EOF);
	}
	destino[strcspn(destino, "\r\n")] = '\0';
	return 1;
}

/* Lê um número inteiro.
   Retorna 1 se válido, 0 se inválido, -1 se a entrada acabou. */
static int ler_inteiro(const char *mensagem, int *valor) {
	char buffer[64];
	char *fim;
	long numero;

	if (!ler_linha(mensagem, buffer, sizeof(buffer))) return -1;
	numero = strtol(buffer, &fim, 10);
	if (fim == buffer) return 0;
	while (*fim == ' ' || *fim == '\t') fim++;
	if (*fim != '\0') return 0;
	if (numero < -2147483647L || numero > 2147483647L) return 0;
	*valor = (int)numero;
	return 1;
}

/* Formato do inventory.txt: nome e quantidade em duas linhas seguidas por produto */
static int carregar_estoque(Item estoque[], int capacidade) {
	FILE *arquivo = fopen("inventory.txt", "r");
	char nome[TAM_NOME];
	char linha[64];
	int quantidade;
	int total = 0;

	if (!arquivo) return 0;

	while (total < capacidade && fgets(nome, sizeof(nome), arquivo)) {
		nome[strcspn(nome, "\r\n")] = '\0';
		if (!fgets(linha, sizeof(linha), arquivo)) break;
		if (nome[0] != '\0' && sscanf(linha, "%d", &quantidade) == 1 && quantidade > 0) {
			strcpy(estoque[total].nome, nome);
			estoque[total].quantidade = quantidade;
			total++;
		}
	}

	fclose(arquivo);
	return total;
}

static int salvar_estoque(const Item estoque[], int total) {
	FILE *arquivo = fopen("inventory.txt", "w");
	int sucesso = 1;

	if (!arquivo) return 0;
	for (int i = 0; i < total && sucesso; i++) {
		if (fprintf(arquivo, "%s\n%d\n", estoque[i].nome, estoque[i].quantidade) < 0)
			sucesso = 0;
	}
	if (fclose(arquivo) != 0) sucesso = 0;
	return sucesso;
}

static int buscar_item(const Item estoque[], int total, const char *nome) {
	for (int i = 0; i < total; i++) {
		if (strcmp(estoque[i].nome, nome) == 0) return i;
	}
	return -1;
}

static int perguntar_continuar(void) {
	char resposta[16];

	while (1) {
		if (!ler_linha("Deseja realizar outra operacao? (s/n): ", resposta, sizeof(resposta)))
			return 0;
		if (resposta[1] == '\0') {
			if (resposta[0] == 's' || resposta[0] == 'S') return 1;
			if (resposta[0] == 'n' || resposta[0] == 'N') return 0;
		}
		printf("Resposta invalida. Digite s ou n.\n");
	}
}

static void adicionar_item(Item estoque[], int *total) {
	char nome[TAM_NOME];
	int quantidade;
	int indice;

	if (*total == MAX_ITENS) {
		printf("Estoque cheio.\n\n");
		return;
	}
	if (!ler_linha(" Digite o nome do item: ", nome, sizeof(nome))) return;
	if (nome[0] == '\0') {
		printf("Nome invalido.\n\n");
		return;
	}
	if (ler_inteiro(" Digite a quantidade: ", &quantidade) != 1 || quantidade <= 0) {
		printf("Quantidade invalida.\n\n");
		return;
	}

	indice = buscar_item(estoque, *total, nome);
	if (indice >= 0) {
		estoque[indice].quantidade += quantidade;
	} else {
		strcpy(estoque[*total].nome, nome);
		estoque[*total].quantidade = quantidade;
		(*total)++;
	}
	if (!salvar_estoque(estoque, *total))
		printf("Aviso: nao foi possivel salvar inventory.txt.\n");
	printf("Item adicionado com sucesso!\n\n");
}

static void remover_item(Item estoque[], int *total) {
	char nome[TAM_NOME];
	int quantidade;
	int indice;

	if (!ler_linha(" Digite o nome do item: ", nome, sizeof(nome))) return;
	indice = buscar_item(estoque, *total, nome);
	if (indice < 0) {
		printf("Item nao encontrado.\n\n");
		return;
	}
	if (ler_inteiro("Digite a quantidade a ser removida: ", &quantidade) != 1 || quantidade <= 0) {
		printf("Quantidade invalida.\n\n");
		return;
	}

	if (quantidade > estoque[indice].quantidade) {
		printf("Estoque insuficiente. Quantidade disponivel: %d\n\n",
			estoque[indice].quantidade);
		return;
	}
	if (quantidade == estoque[indice].quantidade) {
		for (int i = indice; i < *total - 1; i++) estoque[i] = estoque[i + 1];
		(*total)--;
		if (!salvar_estoque(estoque, *total))
			printf("Aviso: nao foi possivel salvar inventory.txt.\n");
		printf("Item removido do estoque!\n\n");
	} else {
		estoque[indice].quantidade -= quantidade;
		if (!salvar_estoque(estoque, *total))
			printf("Aviso: nao foi possivel salvar inventory.txt.\n");
		printf("Quantidade atualizada com sucesso!\n\n");
	}
}

static void listar_estoque(const Item estoque[], int total) {
	if (total == 0) {
		printf("O estoque esta vazio.\n\n");
		return;
	}
	printf("\nItens em estoque:\n");
	for (int i = 0; i < total; i++)
		printf("%s - %d unidade(s)\n", estoque[i].nome, estoque[i].quantidade);
	printf("\n");
}

int main(void) {
	Item estoque[MAX_ITENS];
	int total = carregar_estoque(estoque, MAX_ITENS);
	int opcao;
	int resultado;

	while (1) {
		printf("================================\n");
		printf("   Controle de Estoque\n");
		printf("================================\n");
		printf("Selecione uma opcao:\n");
		printf("1. Adicionar Item\n");
		printf("2. Remover Item\n");
		printf("3. Listar Estoque\n");
		printf("4. Sair\n");

		resultado = ler_inteiro("Opcao: ", &opcao);
		if (resultado < 0) break;
		if (resultado == 0) {
			printf("Entrada invalida. Digite um numero de 1 a 4.\n\n");
			continue;
		}

		if (opcao == 4) break;

		if (opcao == 1) {
			adicionar_item(estoque, &total);
		} else if (opcao == 2) {
			remover_item(estoque, &total);
		} else if (opcao == 3) {
			listar_estoque(estoque, total);
		} else {
			printf("Opcao invalida! Tente novamente.\n\n");
			continue;
		}

		if (!perguntar_continuar()) break;
	}

	printf("\nObrigado por usar o Controle de Estoques! Ate a proxima.\n");

	return 0;
}
