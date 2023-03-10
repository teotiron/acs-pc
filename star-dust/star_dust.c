// Tiron Teodor 314CA
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*Folosesc sscanf pentru a converti un șir de 2 caractere într-o
valoare de tip signed char*/
signed char hex_to_dec(char *x)
{
	signed char a;
	if (sscanf(x, "%hhx", &a) == 1)
		return a;
	return 0;
}

void print_map(signed char **map, int n, int *lines)
{
	for (int i = 0; i < n; i++) {
		for (int j = 0; j < lines[i]; j++) {
			for (int k = 3; k >= 0; k--) {
				if (map[i][j * 4 + k] >= 0 && map[i][j * 4 + k] < 16)
					printf("0");
				printf("%hhX", map[i][j * 4 + k]);
			}
			printf(" ");
		}
		printf("\n");
	}
}

double shield_power(signed char **map, int n, int *lines)
{
	double value = 0;
	int nr = 0;
	for (int i = 0; i < 4 * lines[0]; i++) {
		value = value + (double)map[0][i];
		nr++;
	}
	for (int i = 0; i < 4 * lines[n - 1]; i++) {
		value = value + (double)map[n - 1][i];
		nr++;
	}
	for (int i = 1; i < n - 1; i++) {
		value = value + (double)map[i][0] + (double)map[i][4 * lines[i] - 1];
		nr = nr + 2;
	}
	return value / nr;
}

int date_size(char x)
{
	if (x == 'C')
		return 1;
	if (x == 'S')
		return 2;
	if (x == 'I')
		return 4;
	return 0;
}

void modify_map(signed char **map, int *lines)
{
	char x;
	int size, li, index;
	scanf("%s", &x);
	size = date_size(x);
	scanf("%d%d", &li, &index);
	char *word, *bit;
	word = (char *)calloc(9, sizeof(char));
	bit = (char *)calloc(3, sizeof(char));
	scanf("%s", word);
	//Verific dacă am spațiu suficient pe linie
	if (size * index > lines[li] * 4) {
		int oldsize = lines[li];
		//Calculez dimensiunea necesară pentru a efectua operația
		while (size * index > lines[li] * 4)
			lines[li]++;
		map[li] = (signed char *)realloc(map[li], 4 * lines[li]);
		//Setez valoarea 0 pe zona nou alocată
		for (int i = 4 * oldsize; i < 4 * lines[li]; i++)
			map[li][i] = 0;
	}
	int iteration = 0;
	//Desfac șirul citit în valori de tip signed char
	for (int i = (index - 1) * size; i < index * size; i++) {
		bit[0] = word[6 - 2 * iteration];
		bit[1] = word[7 - 2 * iteration];
		bit[2] = '\0';
		map[li][i] = hex_to_dec(bit);
		iteration++;
	}
	free(word);
	free(bit);
}

void delete_map(signed char **map)
{
	char x;
	int size, line, index;
	scanf("%s", &x);
	size = date_size(x);
	scanf("%d%d", &line, &index);
	for (int i = (index - 1) * size; i < index * size; i++)
		map[line][i] = 0;
}

void swap_map(signed char **map)
{
	char x;
	int size, line, index;
	signed char aux;
	scanf("%s", &x);
	size = date_size(x);
	scanf("%d%d", &line, &index);
	int iteration = 0;
	for (int i = index * size; i < index * size + size / 2; i++) {
		aux = map[line][i];
		map[line][i] = map[line][(index + 1) * size - 1 - iteration];
		map[line][(index + 1) * size - 1 - iteration] = aux;
		iteration++;
	}
}

/*Folosesc o funcție recursivă pentru a trece prin toate valorile de 0
adiacente valorii date*/
int count_zero(signed char **map, int n, int *lines, int pos_i, int pos_j)
{
	int value = 1;
	//Valoarea elementului parcurs devine -1, pentru a nu fi parcursă din nou
	map[pos_i][pos_j] = -1;
	if (pos_i - 1 >= 0 && pos_j < 4 * lines[pos_i - 1])
		if (map[pos_i - 1][pos_j] == 0)
			value = value + count_zero(map, n, lines, pos_i - 1, pos_j);
	if (pos_i + 1 < n && pos_j < 4 * lines[pos_i + 1])
		if (map[pos_i + 1][pos_j] == 0)
			value = value + count_zero(map, n, lines, pos_i + 1, pos_j);
	if (pos_j - 1 >= 0)
		if (map[pos_i][pos_j - 1] == 0)
			value = value + count_zero(map, n, lines, pos_i, pos_j - 1);
	if (pos_j + 1 < 4 * lines[pos_i])
		if (map[pos_i][pos_j + 1] == 0)
			value = value + count_zero(map, n, lines, pos_i, pos_j + 1);
	return value;
}

int main(void)
{
	int n;
	int *lines;
	signed char **map;
	scanf("%d", &n);
	char *x, *bit;
	x = (char *)calloc(9, sizeof(char));
	bit = (char *)calloc(3, sizeof(char));
	lines = (int *)calloc(n, sizeof(int));
	map = (signed char **)malloc(n * sizeof(signed char *));
	for (int i = 0; i < n; i++) {
		scanf("%d", &lines[i]);
		map[i] = (signed char *)calloc(4 * lines[i], sizeof(signed char));
		for (int j = 0; j < lines[i]; j++) {
			scanf("%s", x);
			//Desfac șirul citit în 4 valori de tip signed char
			for (int k = 0; k < 4; k++) {
				bit[0] = x[6 - 2 * k];
				bit[1] = x[7 - 2 * k];
				bit[2] = '\0';
				map[i][j * 4 + k] = hex_to_dec(bit);
			}
		}
	}
	double pow = shield_power(map, n, lines);
	int op;
	scanf("%d", &op);
	char type;
	for (int i = 0; i < op; i++) {
		scanf(" %c", &type);
		if (type == 'M')
			modify_map(map, lines);
		if (type == 'D')
			delete_map(map);
		if (type == 'S')
			swap_map(map);
	}

	printf("task 1\n");
	printf("%.8lf\n", pow);

	printf("task 2\n");
	print_map(map, n, lines);

	printf("task 3\n");
	int max_value = 0, min_i = 0, min_j = 0;
	for (int i = 0; i < n; i++)
		for (int j = 0; j < 4 * lines[i]; j++)
			if (map[i][j] == 0) {
				int value = count_zero(map, n, lines, i, j);
				if (value > max_value) {
					max_value = value;
					min_i = i;
					min_j = j;
				}
			}
	printf("%d %d %d\n", min_i, min_j, max_value);

	free(x);
	free(bit);
	free(lines);
	for (int i = 0; i < n; i++)
		free(map[i]);
	free(map);
}
