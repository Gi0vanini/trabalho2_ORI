/* ==========================================================================
 * Universidade Federal de São Carlos - Campus Sorocaba
 * Disciplina: Organização e Recuperação da Informação
 * Prof. Tiago A. de Almeida
 *
 * Trabalho 02 - Árvore B
 *
 * RA: 743541
 * Aluno: Giovanni Marçon Rossi
 * ========================================================================== */

/* Bibliotecas */
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

/* Tamanho dos campos dos registros */
#define TAM_PRIMARY_KEY 11
#define TAM_NOME 31
#define TAM_GENERO 2
#define TAM_NASCIMENTO 11
#define TAM_CELULAR 16
#define TAM_VEICULO 31
#define TAM_PLACA 9
#define TAM_DATA 9
#define TAM_HORA 6
#define TAM_TRAJETO 121
#define TAM_VALOR 7
#define TAM_VAGAS 2
#define TAM_STRING_INDICE (30 + 6 + 4 + 1)

#define TAM_REGISTRO 256
#define MAX_REGISTROS 1000
#define MAX_ORDEM 150
#define TAM_ARQUIVO (MAX_REGISTROS * TAM_REGISTRO + 1)

/* Saídas do usuário */
#define OPCAO_INVALIDA "Opcao invalida!\n"
#define MEMORIA_INSUFICIENTE "Memoria insuficiente!\n"
#define REGISTRO_N_ENCONTRADO "Registro(s) nao encontrado!\n"
#define CAMPO_INVALIDO "Campo invalido! Informe novamente.\n"
#define ERRO_PK_REPETIDA "ERRO: Ja existe um registro com a chave primaria: %s.\n"
#define ARQUIVO_VAZIO "Arquivo vazio!"
#define INICIO_BUSCA "********************************BUSCAR********************************\n"
#define INICIO_LISTAGEM "********************************LISTAR********************************\n"
#define INICIO_ALTERACAO "********************************ALTERAR*******************************\n"
#define INICIO_ARQUIVO "********************************ARQUIVO*******************************\n"
#define INICIO_INDICE_PRIMARIO "***************************INDICE PRIMÁRIO****************************\n"
#define INICIO_INDICE_SECUNDARIO "***************************INDICE SECUNDÁRIO**************************\n"
#define SUCESSO "OPERACAO REALIZADA COM SUCESSO!\n"
#define FALHA "FALHA AO REALIZAR OPERACAO!\n"
#define NOS_PERCORRIDOS_IP "Busca por %s. Nos percorridos:\n"
#define NOS_PERCORRIDOS_IS "Busca por %s.\nNos percorridos:\n"

/* Registro da Carona */
typedef struct
{
	char pk[TAM_PRIMARY_KEY];
	char nome[TAM_NOME];
	char genero[TAM_GENERO];
	char nascimento[TAM_NASCIMENTO]; /* DD/MM/AAAA */
	char celular[TAM_CELULAR];
	char veiculo[TAM_VEICULO];
	char placa[TAM_PLACA];
	char trajeto[TAM_TRAJETO];
	char data[TAM_DATA];   /* DD/MM/AA, ex: 24/09/19 */
	char hora[TAM_HORA];   /* HH:MM, ex: 07:30 */
	char valor[TAM_VALOR]; /* 999.99, ex: 004.95 */
	char vagas[TAM_VAGAS];
} Carona;

/*Estrutura da chave de um nó do Índice Primário*/
typedef struct Chaveip
{
	char pk[TAM_PRIMARY_KEY];
	int rrn;
} Chave_ip;

/*Estrutura da chave de um  do Índice Secundário*/
typedef struct Chaveis
{
	char string[TAM_STRING_INDICE];
	char pk[TAM_PRIMARY_KEY];
} Chave_is;

/* Estrutura das Árvores-B */
typedef struct nodeip
{
	int num_chaves;  /* numero de chaves armazenadas*/
	Chave_ip *chave; /* vetor das chaves e rrns [m-1]*/
	int *desc;		 /* ponteiros para os descendentes, *desc[m]*/
	char folha;		 /* flag folha da arvore*/
} node_Btree_ip;

typedef struct nodeis
{
	int num_chaves;  /* numero de chaves armazenadas*/
	Chave_is *chave; /* vetor das chaves e rrns [m-1]*/
	int *desc;		 /* ponteiros para os descendentes, *desc[m]*/
	char folha;		 /* flag folha da arvore*/
} node_Btree_is;

typedef struct
{
	int raiz;
} Indice;

/* Variáveis globais */
char ARQUIVO[TAM_ARQUIVO];
char ARQUIVO_IP[2000 * sizeof(Chave_ip)];
char ARQUIVO_IS[2000 * sizeof(Chave_is)];
int ordem_ip;
int ordem_is;
int nregistros;
int nregistrosip; /*Número de nós presentes no ARQUIVO_IP*/
int nregistrosis; /*Número de nós presentes no ARQUIVO_IS*/
int tamanho_registro_ip;
int tamanho_registro_is;
/* ==========================================================================
 * ========================= PROTÓTIPOS DAS FUNÇÕES =========================
 * ========================================================================== */

/* Recebe do usuário uma string simulando o arquivo completo e retorna o número
  * de registros. */
int carregar_arquivo();

/* Recupera do arquivo o registro com o rrn informado e retorna os dados na
 * struct Carona */
Carona recuperar_registro(int rrn);

/* Exibe a Carona */
int exibir_registro(int rrn);

/*Gera Chave da struct Carona*/
void gerarChave(Carona *novo);


/* CRIAÇÃO DE ÍNDICES */

/* Cria o iprimary */
void criar_iprimary(Indice *iprimary);

/* Cria o índice de Caronas */
void criar_iride(Indice *iride);


/* FUNÇÕES DE APOIO */

/* Passa os arquivos da struct Carona para a string buffer com todos os marcadores */
void construir_buffer(char *buffer, Carona *novo);

/* Insere no campo no registro do arq */
void inserir_campo_arq(char *arq, char *campo, int *reg_preenchido);

/* Escreve um nó da árvore no arquivo de índice. */
/* O terceiro parametro serve para informar qual indice se trata */
void write_btree(void *salvar, int rrn, int ip);

/* Lê um nó do arquivo de índice e retorna na estrutura*/
void *read_btree(int rrn, int ip);

/* Aloca dinamicamente os vetores de chaves e descendentes */
void *criar_no(int ip);

/* Libera o no node da memória principal */
void libera_no(void *node, int ip);


/* INSERÇÃO */

/* Lê da entrada padão e insere na struct Carona 'novo' */
void criar_carona(Carona *novo);

/* Insere um novo registro na Árvore B */
void insere_chave_ip(Indice *iprimary, Chave_ip chave);

/* Função auxiliar para ser chamada recursivamente, inserir as novas chaves nas
 * folhas e tratar overflow no retorno da recursão. */
int insere_aux_ip(int noderrn, Chave_ip *chave);
int insere_aux_is(int noderrn, Chave_is *chave);

/* Função auxiliar que ordena as chaves em esq + a chave a ser inserida e divide
 * entre os nós esq e dir. Retorna o novo nó à direita, a chave promovida e seu
 * descendente direito, que pode ser nulo, caso a nó seja folha. */
int divide_no_ip(int rrnesq, Chave_ip *chave, int desc_dir_rrn);
int divide_no_is(int rrnesq, Chave_is *chave, int desc_dir_rrn);


/* BUSCAS */

/* Atualiza os dois índices com o novo registro inserido */
void inserir_registro_indices(Indice *iprimary, Indice *iride, Carona j);

/* Percorre a arvore e retorna o RRN da chave informada.  Retorna -1, caso não
 * encontrada. */
/* exibir_registro serve para se os registros no cainho devem ou não ser exibidos */
int buscar_chave_ip(int noderrn, char *pk, int exibir_caminho);

/* Percorre a arvore e retorna a pk da string destino/data-hora informada. Retorna -1, caso não
 * encontrada. */
char *buscar_chave_is(const int noderrn, const char *titulo, const int exibir_caminho);


/* LISTAGEM */

/* Realiza percurso pré-ordem imprimindo as chaves primárias dos registros e o
 * nível de cada nó (raiz = nível 1) */
int imprimir_arvore_ip(int noderrn, int nivel);

/* Realiza percurso em-ordem imprimindo todas as caronas na ordem lexicografica do destino e data/hora e o
 * nível de cada nó (raiz = nível 1) */
int imprimir_arvore_is(int noderrn, int nivel);


/********************FUNÇÕES DO MENU*********************/
void cadastrar(Indice *iprimary, Indice *iride);

int alterar(Indice iprimary);

void buscar(Indice iprimary, Indice iride);

void listar(Indice iprimary, Indice iride);

/*******************************************************/


int main()
{
	char *p; /* # */
			 /* Arquivo */
	int carregarArquivo = 0;
	nregistros = 0, nregistrosip = 0, nregistrosis = 0;
	scanf("%d\n", &carregarArquivo); /* 1 (sim) | 0 (nao) */
	if (carregarArquivo)
		nregistros = carregar_arquivo();

	scanf("%d %d%*c", &ordem_ip, &ordem_is);

	tamanho_registro_ip = ordem_ip * 3 + 4 + (-1 + ordem_ip) * 14;
	tamanho_registro_is = ordem_is * 3 + 4 + (-1 + ordem_is) * (TAM_STRING_INDICE + 10);

	/* Índice primário */
	Indice iprimary;
	criar_iprimary(&iprimary);

	/* Índice secundário de nomes dos Caronas */
	Indice iride;
	criar_iride(&iride);

	/* Execução do programa */
	int opcao = 0;
	while (1)
	{
		scanf("%d%*c", &opcao);
		switch (opcao)
		{
		case 1: /* Cadastrar uma nova Carona */
			cadastrar(&iprimary, &iride);
			break;
		case 2: /* Alterar a qtd de vagas de uma Carona */
			printf(INICIO_ALTERACAO);
			if (alterar(iprimary))
				printf(SUCESSO);
			else
				printf(FALHA);
			break;
		case 3: /* Buscar uma Carona */
			printf(INICIO_BUSCA);
			buscar(iprimary, iride);
			break;
		case 4: /* Listar todos as Caronas */
			printf(INICIO_LISTAGEM);
			listar(iprimary, iride);
			break;
		case 5: /* Imprimir o arquivo de dados */
			printf(INICIO_ARQUIVO);
			printf("%s\n", (*ARQUIVO != '\0') ? ARQUIVO : ARQUIVO_VAZIO);
			break;
		case 6: /* Imprime o Arquivo de Índice Primário*/
			printf(INICIO_INDICE_PRIMARIO);
			if (!*ARQUIVO_IP)
				puts(ARQUIVO_VAZIO);
			else
				for (p = ARQUIVO_IP; *p != '\0'; p += tamanho_registro_ip)
				{
					fwrite(p, 1, tamanho_registro_ip, stdout);
					puts("");
				}
			break;
		case 7: /* Imprime o Arquivo de Índice Secundário*/
			printf(INICIO_INDICE_SECUNDARIO);
			if (!*ARQUIVO_IS)
				puts(ARQUIVO_VAZIO);
			else
				for (p = ARQUIVO_IS; *p != '\0'; p += tamanho_registro_is)
				{
					fwrite(p, 1, tamanho_registro_is, stdout);
					puts("");
				}
			//printf("%s\n", ARQUIVO_IS);
			break;
		case 8: /*Libera toda memória alocada dinâmicamente (se ainda houver) e encerra*/
			return 0;
		default: /* exibe mensagem de erro */
			printf(OPCAO_INVALIDA);
			break;
		}
	}
	return -1;
}

/* ==========================================================================
 * ================================= FUNÇÕES ================================
 * ========================================================================== */

/* Recebe do usuário uma string simulando o arquivo completo e retorna o número
 * de registros. */
int carregar_arquivo(){
	scanf("%[^\n]\n", ARQUIVO);
	return strlen(ARQUIVO) / TAM_REGISTRO;
}

/* Exibe a Carona */
int exibir_registro(int rrn){
	if (rrn < 0)
		return 0;

	Carona j = recuperar_registro(rrn);
	char *traj, trajeto[TAM_TRAJETO];

	printf("%s\n", j.pk);
	printf("%s\n", j.nome);
	printf("%s\n", j.genero);
	printf("%s\n", j.nascimento);
	printf("%s\n", j.celular);
	printf("%s\n", j.veiculo);
	printf("%s\n", j.placa);
	printf("%s\n", j.data);
	printf("%s\n", j.hora);
	printf("%s\n", j.valor);
	printf("%s\n", j.vagas);

	strcpy(trajeto, j.trajeto);

	traj = strtok(trajeto, "|");

	while (traj != NULL)
	{
		printf("%s", traj);
		traj = strtok(NULL, "|");
		if (traj != NULL)
		{
			printf(", ");
		}
	}

	printf("\n");

	return 1;
}

/* Recupera do arquivo o registro com o rrn informado e retorna os dados na
 * struct Carona */
Carona recuperar_registro(int rrn){

	char temp[257], *p;
	strncpy(temp, ARQUIVO + ((rrn)*TAM_REGISTRO), TAM_REGISTRO);
	temp[256] = '\0';
	Carona j;

	p = strtok(temp, "@");
	strcpy(j.nome, p);
	p = strtok(NULL, "@");
	strcpy(j.genero, p);
	p = strtok(NULL, "@");
	strcpy(j.nascimento, p);
	p = strtok(NULL, "@");
	strcpy(j.celular, p);
	p = strtok(NULL, "@");
	strcpy(j.veiculo, p);
	p = strtok(NULL, "@");
	strcpy(j.placa, p);
	p = strtok(NULL, "@");
	strcpy(j.trajeto, p);
	p = strtok(NULL, "@");
	strcpy(j.data, p);
	p = strtok(NULL, "@");
	strcpy(j.hora, p);
	p = strtok(NULL, "@");
	strcpy(j.valor, p);
	p = strtok(NULL, "@");
	strcpy(j.vagas, p);

	gerarChave(&j);

	return j;
}

/* Gera uma chave primária para a struct Carona novo */
void gerarChave(Carona *novo){
    // Nome
    novo->pk[0] = novo->nome[0];
    // Placa
    novo->pk[1] = novo->placa[0];
    novo->pk[2] = novo->placa[1];
    novo->pk[3] = novo->placa[2];
    // Data
    novo->pk[4] = novo->data[0];
    novo->pk[5] = novo->data[1];
    novo->pk[6] = novo->data[3];
    novo->pk[7] = novo->data[4];
    // Hora
    novo->pk[8] = novo->hora[0];
    novo->pk[9] = novo->hora[1];
    // \0
    novo->pk[10] = '\0';

    return;
}

/* ==========================================================================
 * ============================= CRIAR ÍNDICES ==============================
 * ========================================================================== */

/* Cria o iprimary*/
void criar_iprimary(Indice *iprimary){
	// Por enquanto é so isso :P
	// Quando for fazer teste com arquivos, ou ter que recuperar 1 a 1 do arquivo de dados e inserir eles aqui
	iprimary->raiz = -1;
	return;
}

/* Cria o índice de Caronas  */
void criar_iride(Indice *iride){
	// Mesma coisa da função acima
	iride->raiz = -1;
	return;
}

/* ==========================================================================
 * =========================== FUNÇÕES DE APOIO =============================
 * ========================================================================== */

/* Passa os arquivos da struct Carona para a string buffer com todos os marcadores */
/* A função espera que o buffer fornecido tenhe o tamanho de de TAM_REGISTROS + 1 */
/* A variável reg_preenchido serve como referência para a função inserir_campo_arq */
/* para saber quantos campos existem no registro atualmente, assim permitindo escrever */
/* na string arq, onde a função anterior havia parado. */
void construir_buffer(char *buffer, Carona *novo){
	int reg_preenchido = 0;

	inserir_campo_arq(buffer, novo->nome, &reg_preenchido);
	inserir_campo_arq(buffer, novo->genero, &reg_preenchido);
	inserir_campo_arq(buffer, novo->nascimento, &reg_preenchido);
	inserir_campo_arq(buffer, novo->celular, &reg_preenchido);
	inserir_campo_arq(buffer, novo->veiculo, &reg_preenchido);
	inserir_campo_arq(buffer, novo->placa, &reg_preenchido);
	inserir_campo_arq(buffer, novo->trajeto, &reg_preenchido);
	inserir_campo_arq(buffer, novo->data, &reg_preenchido);
	inserir_campo_arq(buffer, novo->hora, &reg_preenchido);
	inserir_campo_arq(buffer, novo->valor, &reg_preenchido);
	inserir_campo_arq(buffer, novo->vagas, &reg_preenchido);

	for(int i = reg_preenchido; i < TAM_REGISTRO; i++){
		buffer[i] = '#';
	}

	buffer[TAM_REGISTRO] = '\0';

	return;
}

/* Insere no campo no registro do arq */
void inserir_campo_arq(char *arq, char *campo, int *reg_preenchido){
	int temp;

	temp = sprintf((arq + *reg_preenchido), "%s", campo);	// Colocando o campo no registro
	*reg_preenchido += temp;								// Adicionando o número total de caracteres adicionados no registro preenchido
	arq[*reg_preenchido] = '@';								// Adicionando o caractere separador
	*reg_preenchido += 1;									// Adicionando o número total de caracteres devido a adição do caractere separador

	return;
}

/* Escreve um nó da árvore no arquivo de índice. */
/* O terceiro parametro serve para informar qual indice se trata */
/* Se ip é 1, aloca um nó do índice primário */
/* Se ip é deferente de 1, aloca um nó do índice secundário */
void write_btree(void *salvar, int rrn, int ip){
	if(ip){
		node_Btree_ip *save = (node_Btree_ip *)salvar;
		
		char *p_arq_ip;
		char buffer[tamanho_registro_ip + 1];

		char num_chaves[4];
		char rrn_registros[5];
		char rrn_desc[4];

		// Escreve o número de chaves no buffer
		sprintf(num_chaves, "%.3d", save->num_chaves);
		strncpy(buffer, num_chaves, 3);

		// Escreve no buffer as chaves e os rrn dos registros do nó
		for(int i = 0; i < (ordem_ip - 1); i++){
			if(i < save->num_chaves){
				// buffer + qtd_chaves + i*tam_conteudo
				strncpy((buffer + 3 + (14*i)), save->chave[i].pk, 10);
				
				sprintf(rrn_registros, "%.4d", save->chave[i].rrn);
				// buffer + qtd_chaves + i*tam_conteudo + TAM_PRIMARY_KEY
				strncpy((buffer + 3 + (14*i) + 10), rrn_registros, 4);	
			}else{
				strncpy((buffer + 3 + (14*i)), "##############", 14);
			}
		}
		
		// Escreve no buffer se o nó é folha ou não
		buffer[3 + ((ordem_ip - 1) * 14)] = save->folha;

		// Escreve no buffer os decendentes do nó
		for(int i = 0; i < ordem_ip; i++){
			if(i < (save->num_chaves+1)){
				if(save->desc[i] == -1){
					strncpy((buffer + 3 + ((ordem_ip-1) * 14) + 1 + i*3), "***", 3);
				}else{
					sprintf(rrn_desc, "%.3d", save->desc[i]);
					strncpy((buffer + 3 + ((ordem_ip-1) * 14) + 1 + i*3), rrn_desc, 3);
				}
			}else{
				strncpy((buffer + 3 + ((ordem_ip-1) * 14) + 1 + i*3), "***", 3);
			}
		}
		
		// Escreve no aquivo de daods do IP o buffer criado
		// OBS IMPORTANTE nregistrosip só  incrementado APÓS a inserção do registro no aquivo de dados
		if(nregistrosip == rrn){ 	// Coloca \0

			buffer[tamanho_registro_ip] = '\0';
			p_arq_ip = ARQUIVO_IP + (rrn*tamanho_registro_ip);
			strncpy(p_arq_ip, buffer, tamanho_registro_ip + 1);
		}else{						// Não coloca \0

			p_arq_ip = ARQUIVO_IP + (rrn*tamanho_registro_ip);
			strncpy(p_arq_ip, buffer, tamanho_registro_ip);
		}

		return;
	}else{
		return;
	}
}

/* Lê um nó do arquivo de índice e retorna na estrutura */
/* Se ip é 1, aloca um nó do índice primário */
/* Se ip é deferente de 1, aloca um nó do índice secundário */
void *read_btree(int rrn, int ip){
	if(ip){
		char *p_arq_ip;
		char buffer[tamanho_registro_ip + 1];
		
		node_Btree_ip *novo;
		char num_chaves[4];
		char rrn_registro[5];
		char rrn_desc[4];

		// Alocando espaço para o novo nó
		novo = (node_Btree_ip *)malloc(sizeof(node_Btree_ip));

		// Preenchendo o buffer com o registro desejado
		p_arq_ip = ARQUIVO_IP + (rrn*tamanho_registro_ip);
		strncpy(buffer, p_arq_ip, tamanho_registro_ip);
		buffer[tamanho_registro_ip] = '\0';

		// Colocando o num_chaves em 'novo'
		num_chaves[0] = buffer[0];
		num_chaves[1] = buffer[1];
		num_chaves[2] = buffer[2];
		num_chaves[3] = '\0';

		novo->num_chaves = atoi(num_chaves);

		// Colocando as chaves e os rrn do registro em 'novo'
		novo->chave = (Chave_ip *)malloc(sizeof(Chave_ip) * (ordem_ip - 1));
		
		for(int i = 0; i < novo->num_chaves; i++){
			strncpy(novo->chave[i].pk, buffer + 3 + (i * 14), 10);
			novo->chave[i].pk[TAM_PRIMARY_KEY - 1] = '\0';

			// buffer[qtd_chaves + i*tam_conteudo + tam_chave + byte_do_rrn]
			rrn_registro[0] = buffer[3 + (i * 14) + 10 + 0];
			rrn_registro[1] = buffer[3 + (i * 14) + 10 + 1];
			rrn_registro[2] = buffer[3 + (i * 14) + 10 + 2];
			rrn_registro[3] = buffer[3 + (i * 14) + 10 + 3];
			rrn_registro[4] = '\0';

			novo->chave[i].rrn = atoi(rrn_registro);
		}

		// Colocando o valor do campo folha em 'novo'
		novo->folha = buffer[3 + ((ordem_ip - 1) * 14)];

		// Colocando os valores dos rrn das folhas decendentes
		novo->desc = (int *)malloc(sizeof(int) * ordem_ip);

		for(int i = 0; i < ordem_ip; i++){
			if(buffer[i*3 + (3 + ((ordem_ip-1) * 14) + 1)] != '*'){

				// buffer[qtd_chaves + tam_conteudo + indicador_folha + i*tam_rrn_desc + byte_do_rrn]		
				rrn_desc[0] = buffer[3 + ((ordem_ip-1) * 14) + 1 + i*3 + 0];
				rrn_desc[1] = buffer[3 + ((ordem_ip-1) * 14) + 1 + i*3 + 1];
				rrn_desc[2] = buffer[3 + ((ordem_ip-1) * 14) + 1 + i*3 + 2];
				rrn_desc[3] = '\0';

				novo->desc[i] = atoi(rrn_desc);
			}else{
				novo->desc[i] = -1;
			}
		}

		return novo;
	}else{
		return NULL;
	}
}

/* Aloca dinamicamente os vetores de chaves e descendentes */
/* Se ip é 1, aloca um nó do índice primário */
/* Se ip é deferente de 1, aloca um nó do índice secundário */
void *criar_no(int ip){
	if(ip){
		node_Btree_ip *novo;
		novo = (node_Btree_ip *)malloc(sizeof(node_Btree_ip));

		novo->num_chaves = 0;

		// Alocando memória para as chaves do nó
		novo->chave = (Chave_ip *)malloc(sizeof(Chave_ip) * (ordem_ip - 1));

		novo->folha = '?';

		// Alocando memória para os decendentes do nó
		novo->desc = (int *)malloc(sizeof(int) * ordem_ip);

		for(int i = 0; i < ordem_ip; i++){
			novo->desc[i] = -1;
		}

		return novo;
	}else{
		return NULL;
	}
}

/* Libera o no node da memória principal */
/* Se ip é 1, aloca um nó do índice primário */
/* Se ip é deferente de 1, aloca um nó do índice secundário */
void libera_no(void *node, int ip){
	if(ip){
		node_Btree_ip *aux;

		aux = (node_Btree_ip *)node;

		free(aux->chave);
		free(aux->desc);
		free(aux);

		return;
	}else{
		return;
	}
}

/* ==========================================================================
 * ============================ FUNÇÕES DO MENU =============================
 * ========================================================================== */

void cadastrar(Indice *iprimary, Indice *iride){
	Carona novo;
	int rrn = 0;

	criar_carona(&novo);
	
	rrn = buscar_chave_ip(iprimary->raiz, novo.pk, 0);
	
	if(rrn == -1){
		inserir_registro_indices(iprimary, iride, novo);
	}else{
		printf(ERRO_PK_REPETIDA, novo.pk);
		return;
	}

	return;
}

int alterar(Indice iprimary){

}

void buscar(Indice iprimary, Indice iride){
	int opcao;
	
	char pk[TAM_PRIMARY_KEY];
	int rrn;

	scanf("%d%*c", &opcao);

	switch (opcao){
	case 1:
		scanf("%[^\n]%*c", pk);
		
		printf("Busca por %s. Nos percorridos:\n", pk);

		rrn = buscar_chave_ip(iprimary.raiz, pk, 1);
		printf("\n");
		
		if(rrn != -1)
			exibir_registro(rrn);
		else
			printf(REGISTRO_N_ENCONTRADO);
		break;
	
	case 2:

		break;
	}
}

void listar(Indice iprimary, Indice iride){
	int opcao;
	
	scanf("%d%*c", &opcao);

	switch(opcao){
		case 1:
			if(!imprimir_arvore_ip(iprimary.raiz, 1)){
				printf(REGISTRO_N_ENCONTRADO);
			}
			break;

		case 2:
			imprimir_arvore_is(iride.raiz, 1);
			break;
	}
}

/* ==========================================================================
 * =============================== INSERÇÕES ================================
 * ========================================================================== */

/* Lê da entrada padão e insere na struct Carona 'novo' */
void criar_carona(Carona *novo){
    scanf("%[^\n]%*c", novo->nome);
    scanf("%[^\n]%*c", novo->genero);
    scanf("%[^\n]%*c", novo->nascimento);
    scanf("%[^\n]%*c", novo->celular);
    scanf("%[^\n]%*c", novo->veiculo);
    scanf("%[^\n]%*c", novo->placa);
    scanf("%[^\n]%*c", novo->trajeto);
    scanf("%[^\n]%*c", novo->data);
    scanf("%[^\n]%*c", novo->hora);
    scanf("%[^\n]%*c", novo->valor);
    scanf("%[^\n]%*c", novo->vagas);

    gerarChave(novo);

    return;
}

/* Atualiza os dois índices com o novo registro inserido */
void inserir_registro_indices(Indice *iprimary, Indice *iride, Carona j){
	Chave_ip add_ip;

	add_ip.rrn = nregistros;
	strcpy(add_ip.pk, j.pk);

	insere_chave_ip(iprimary, add_ip);

	char buffer[TAM_REGISTRO + 1];
	char *pos_arq = ARQUIVO + (nregistros*TAM_REGISTRO);	//Coloca o ponteiro para onde o registro será inserido no arquivo de dados
	construir_buffer(buffer, &j);
	strncpy(pos_arq, buffer, (TAM_REGISTRO + 1));

	nregistros++;

	return;
}

/* Insere um novo registro na Árvore B */
void insere_chave_ip(Indice *iprimary, Chave_ip chave){
	// Caso a árvore esteja vazia
	if(iprimary->raiz == -1){
		node_Btree_ip *novo;

		novo = (node_Btree_ip *)criar_no(1);
		
		novo->folha = 'T';
		novo->num_chaves = 1;
		novo->chave[0].rrn = chave.rrn;
		strcpy(novo->chave[0].pk, chave.pk);

		iprimary->raiz = nregistrosip;

		write_btree(novo, nregistrosip, 1);

		libera_no(novo, 1);

		nregistrosip++;
		return;
	}else{
		int rrn_filho_dir;
		rrn_filho_dir = insere_aux_ip(iprimary->raiz, &chave);

		if(chave.rrn != -1){
			node_Btree_ip *novo;

			novo = (node_Btree_ip *)criar_no(1);
			
			novo->folha = 'F';
			novo->num_chaves = 1;
			novo->chave[0].rrn = chave.rrn;
			strcpy(novo->chave[0].pk, chave.pk);
			
			novo->desc[0] = iprimary->raiz;
			novo->desc[1] = rrn_filho_dir;

			iprimary->raiz = nregistrosip;

			write_btree(novo, nregistrosip, 1);

			libera_no(novo, 1);

			nregistrosip++;
		}

		return;
	}
}

/* Função auxiliar para ser chamada recursivamente, inserir as novas chaves nas
 * folhas e tratar overflow no retorno da recursão. */
int insere_aux_ip(int noderrn, Chave_ip *chave){
	node_Btree_ip *no;

	no = (node_Btree_ip *)read_btree(noderrn, 1);

	if(no->folha == 'T'){
		if(no->num_chaves < ordem_ip - 1){
			int i = no->num_chaves - 1;

			while(i >= 0 && strcmp(chave->pk, no->chave[i].pk) < 0){
				strcpy(no->chave[i+1].pk, no->chave[i].pk);
				no->chave[i+1].rrn = no->chave[i].rrn;
				i--;
			}

			strcpy(no->chave[i+1].pk, chave->pk);
			no->chave[i+1].rrn = chave->rrn;

			no->num_chaves++;

			write_btree(no, noderrn, 1);

			libera_no(no, 1);
			
			chave->rrn = -1;
			return -1;

		}else{
			libera_no(no, 1);
			return divide_no_ip(noderrn, chave, -1);
		}
	}else{
		int i = no->num_chaves - 1;
		int rrn_filho_dir;

		while(i >= 0 && strcmp(chave->pk, no->chave[i].pk) < 0)
			i--;
		
		i++;

		rrn_filho_dir = insere_aux_ip(no->desc[i], chave);

		if(chave->rrn != -1){
			if(no->num_chaves < ordem_ip - 1){
				i = no->num_chaves - 1;

				while(i >= 0 && strcmp(chave->pk, no->chave[i].pk) < 0){
					strcpy(no->chave[i+1].pk, no->chave[i].pk);
					no->chave[i+1].rrn = no->chave[i].rrn;
					no->desc[i+2] = no->desc[i+1];
					i--;
				}

				strcpy(no->chave[i+1].pk, chave->pk);
				no->chave[i+1].rrn = chave->rrn;
				no->desc[i+2] = rrn_filho_dir;

				no->num_chaves++;

				write_btree(no, noderrn, 1);

				libera_no(no, 1);

				chave->rrn = -1;
				return -1;
			}else{
				libera_no(no, 1);
				return divide_no_ip(noderrn, chave, rrn_filho_dir);
			}
		}else{
			libera_no(no, 1);
			return -1;
		}
	}
}

int insere_aux_is(int noderrn, Chave_is *chave){

}

/* Função auxiliar que ordena as chaves em esq + a chave a ser inserida e divide
 * entre os nós esq e dir. Retorna o novo nó à direita, a chave promovida e seu
 * descendente direito, que pode ser nulo, caso a nó seja folha. */
int divide_no_ip(int rrnesq, Chave_ip *chave, int desc_dir_rrn){
	node_Btree_ip *no;
	node_Btree_ip *novo;

	no = (node_Btree_ip *)read_btree(rrnesq, 1);
	novo = (node_Btree_ip *)criar_no(1);

	int i = no->num_chaves - 1;
	int chave_alocada = 0;

	novo->folha = no->folha;
	novo->num_chaves = (ordem_ip-1)/2;

	for(int j = (novo->num_chaves-1); j >= 0; j--){
		if(!chave_alocada && strcmp(chave->pk, no->chave[i].pk) > 0){
			strcpy(novo->chave[j].pk, chave->pk);
			novo->chave[j].rrn = chave->rrn;
			novo->desc[j+1] = desc_dir_rrn;
			chave_alocada = 1;
		}else{
			strcpy(novo->chave[j].pk, no->chave[i].pk);
			novo->chave[j].rrn = no->chave[i].rrn;
			novo->desc[j+1] = no->desc[i+1];
			i--;
		}
	}

	if(!chave_alocada){
		while(i >= 0 && strcmp(chave->pk, no->chave[i].pk) < 0){
			strcpy(no->chave[i+1].pk, no->chave[i].pk);
			no->chave[i+1].rrn = no->chave[i].rrn;
			no->desc[i+2] = no->desc[i+1];
			i--;
		}

		strcpy(no->chave[i+1].pk, chave->pk);
		no->chave[i+1].rrn = chave->rrn;
		no->desc[i+2] = desc_dir_rrn;
	}

	strcpy(chave->pk, no->chave[(ordem_ip/2)].pk);
	chave->rrn = no->chave[(ordem_ip/2)].rrn;
	novo->desc[0] = no->desc[(ordem_ip/2) + 1];
	no->num_chaves = ordem_ip/2;

	write_btree(no, rrnesq, 1);
	write_btree(novo, nregistrosip, 1);

	nregistrosip++;

	libera_no(no, 1);
	libera_no(novo, 1);

	return nregistrosip - 1;
}

int divide_no_is(int rrnesq, Chave_is *chave, int desc_dir_rrn){

}

/* ==========================================================================
 * ================================ BUSCAS ==================================
 * ========================================================================== */

/* Percorre a arvore e retorna o RRN da chave informada.  Retorna -1, caso não
 * encontrada. */
int buscar_chave_ip(int noderrn, char *pk, int exibir_caminho){
	node_Btree_ip *no;
	
	if(noderrn == -1)
		return -1;

	no = (node_Btree_ip *)read_btree(noderrn, 1);

	//imprime o nó antes de continuar a busca
	if(exibir_caminho){
		printf("%s", no->chave[0].pk);
		for(int i = 1; i < no->num_chaves; i++){
			printf(", %s", no->chave[i].pk);
		}
		printf("\n");
	}

	int i = 0;

	while(i < no->num_chaves && strcmp(pk, no->chave[i].pk) > 0)
		i++;

	if(i <= no->num_chaves && strcmp(pk, no->chave[i].pk) == 0)
		return no->chave[i].rrn;
	
	if(no->folha == 'T')
		return -1;
	else
		return buscar_chave_ip(no->desc[i], pk, exibir_caminho);
	
	
	// node_Btree_ip* no;
	// int rrn_encontrado = -1;

	// if(noderrn == -1)
	// 	return -1;

	// no = (node_Btree_ip *)read_btree(noderrn, 1);


	// //imprime o nó antes de continuar a busca
	// if(exibir_caminho){
	// 	printf("%s", no->chave[0].pk);
	// 	for(int i = 1; i < no->num_chaves; i++){
	// 		printf(", %s", no->chave[i].pk);
	// 	}
	// 	printf("\n");
	// }
	
	// if(no->folha == 'T'){
	// 	for(int i = 0; i < no->num_chaves; i++){
			
	// 		if(strcmp(no->chave[i].pk, pk) == 0){
	// 			rrn_encontrado = no->chave[i].rrn;
	// 			libera_no(no, 1);
	// 			return rrn_encontrado;
	// 		}			
	// 	}

	// 	return rrn_encontrado;

	// }else{
	// 	for(int i = 0; i < no->num_chaves; i++){
			
	// 		if(strcmp(no->chave[i].pk, pk) == 0){
	// 			rrn_encontrado = no->chave[i].rrn;
	// 			libera_no(no, 1);
	// 			printf("era p parar aqui\n");
	// 			return rrn_encontrado;
	// 		} 
			
	// 		if(i == 0 && strcmp(no->chave[i].pk, pk) > 0){
	// 			// Desce mais para a esquerda
	// 			noderrn = no->desc[0];
	// 			libera_no(no, 1);
	// 			return buscar_chave_ip(noderrn ,pk, exibir_caminho);
	// 		}

	// 		if(i == (no->num_chaves-1)){
	// 			// Desce no mais para a direita
	// 			noderrn = no->desc[no->num_chaves-1];
	// 			libera_no(no, 1);
	// 			return buscar_chave_ip(noderrn, pk, exibir_caminho);
	// 		}

	// 		if(strcmp(no->chave[i].pk, pk) < 0 && strcmp(no->chave[i+1].pk, pk) > 0){
	// 			// Entra no meio de i e i + 1
	// 			noderrn = no->desc[i+1];
	// 			libera_no(no, 1);
	// 			return buscar_chave_ip(noderrn, pk, exibir_caminho);
	// 		}			
	// 	}
	// }
}

/* Percorre a arvore e retorna a pk da string destino/data-hora informada. Retorna -1, caso não
 * encontrada. */
char *buscar_chave_is(const int noderrn, const char *titulo, const int exibir_caminho){

}

/* ==========================================================================
 * =============================== LISTAGEM =================================
 * ========================================================================== */

/* Realiza percurso pré-ordem imprimindo as chaves primárias dos registros e o
 * nível de cada nó (raiz = nível 1) */
// Retorna 0 caso a árvore esteja vazia
int imprimir_arvore_ip(int noderrn, int nivel){
	node_Btree_ip *no;

	// Checando se a árvore tem raiz
	if(noderrn == -1)
		return 0;

	no = (node_Btree_ip *)read_btree(noderrn, 1);

	printf("%d -", nivel);

	for(int i = 0; i < (no->num_chaves-1); i++)
		printf(" %s,", no->chave[i].pk);
	

	printf(" %s\n", no->chave[no->num_chaves - 1].pk);

	for(int i = 0; i < (no->num_chaves+1); i++){
		if(no->desc[i] != -1)
			imprimir_arvore_ip(no->desc[i], nivel+1);
	}

	libera_no(no, 1);

	return 1;
}

/* Realiza percurso em-ordem imprimindo todas as caronas na ordem lexicografica do destino e data/hora e o
 * nível de cada nó (raiz = nível 1) */
int imprimir_arvore_is(int noderrn, int nivel){

}
