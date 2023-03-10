// Tiron Teodor 314CA
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <ctype.h>
#define MAXCMDSIZE 100
#define PTRSIZE sizeof(pixel_t *)
#define PSIZE sizeof(pixel_t)

typedef struct {
	int magic;
	int size_x;
	int size_y;
	int maxval;
} metadata_t;

typedef struct {
	unsigned char red;
	unsigned char green;
	unsigned char blue;
} pixel_t;

typedef struct {
	int x1;
	int y1;
	int x2;
	int y2;
} area_t;

//Verific dacă c este un caracter de tip whitespace
int is_whitespace(char c)
{
	if (c == '\t' || c == '\n' || c == '\v' || c == '\f' || c == '\r')
		return 1;
	if (c == ' ')
		return 1;
	return 0;
}

//Verific dacă șirul dat reprezintă un număr
int valid_number(char *text)
{
	for (int i = 0; i < (int)strlen(text); i++)
		if (isdigit(text[i]) == 0 && text[i] != '-')
			return 0;
	return 1;
}

//Obțin numărul de rotații spre dreapta
int rotation_number(int deg)
{
	if (deg == 90 || deg == -270)
		return 1;
	if (deg == 180 || deg == -180)
		return 2;
	if (deg == 270 || deg == -90)
		return 3;
	if (deg == 0 || deg == 360 || deg == -360)
		return 0;
	return -1;
}

//Verific dacă toată zona a fost selectată
int all_is_selected(metadata_t data, area_t area)
{
	if (area.x1 != 0 || area.y1 != 0)
		return 0;
	if (area.x2 != data.size_x || area.y2 != data.size_y)
		return 0;
	return 1;
}

void clear_metadata(metadata_t *x)
{
	x->magic = 0;
	x->size_x = 0;
	x->size_y = 0;
	x->maxval = 0;
}

void closeimage(metadata_t data, pixel_t **img)
{
	for (int i = 0; i < data.size_y; i++)
		free(img[i]);
	if (data.magic != 0)
		free(img);
}

/*Pentru citirea valorilor întregi din fișierele binare, citesc caracter
cu caracter până întâlnesc un whitespace, apoi formez numărul*/
int get_value(FILE *in)
{
	int val, digit;
	char c;
	c = getc(in);
	while (is_whitespace(c) == 1 || c == '#')
		if (c == '#') {
			while (c != EOF && c != '\n' && c != 'r')
				c = getc(in);
		} else {
			c = getc(in);
		}
	val = c - '0';
	while (1) {
		c = getc(in);
		if (is_whitespace(c) == 1 || c == '#') {
			ungetc(c, in);
			return val;
		}
		if (c == EOF)
			return val;
		digit = c - '0';
		val = val * 10 + digit;
	}
}

//Încărcarea unei imagini grayscale ASCII în memorie
void load_p2(FILE *in, metadata_t data, pixel_t **img)
{
	int val;
	for (int i = 0; i < data.size_y; i++)
		for (int j = 0; j < data.size_x; j++) {
			val = get_value(in);
			img[i][j].red = val;
			img[i][j].green = val;
			img[i][j].blue = val;
		}
}

//Încărcarea unei imagini color ASCII în memorie
void load_p3(FILE *in, metadata_t data, pixel_t **img)
{
	for (int i = 0; i < data.size_y; i++)
		for (int j = 0; j < data.size_x; j++) {
			img[i][j].red = get_value(in);
			img[i][j].green = get_value(in);
			img[i][j].blue = get_value(in);
		}
}

//Încărcarea unei imagini grayscale binar în memorie
void load_p5(FILE *in, metadata_t data, pixel_t **img)
{
	int val = getc(in);
	for (int i = 0; i < data.size_y; i++)
		for (int j = 0; j < data.size_x; j++) {
			val = getc(in);
			img[i][j].red = val;
			img[i][j].green = val;
			img[i][j].blue = val;
		}
}

//Încărcarea unei imagini color binar în memorie
void load_p6(FILE *in, metadata_t data, pixel_t **img)
{
	getc(in);
	for (int i = 0; i < data.size_y; i++)
		for (int j = 0; j < data.size_x; j++) {
			img[i][j].red = getc(in);
			img[i][j].green = getc(in);
			img[i][j].blue = getc(in);
		}
}

void load_image(FILE *in, metadata_t *data, pixel_t ***img, char *filename)
{
	char c;
	int val;
	c = getc(in);
	if (c != 'P') {
		printf("Failed to load %s\n", filename);
		clear_metadata(data);
		return;
	}
	c = getc(in);
	if (c != '2' && c != '3' && c != '5' && c != '6') {
		printf("Failed to load %s\n", filename);
		clear_metadata(data);
		return;
	}
	val = c - '0';
	data->magic = val;
	data->size_x = get_value(in);
	data->size_y = get_value(in);
	data->maxval = get_value(in);
	printf("Loaded %s\n", filename);
	if (data->magic != 0) {
		*img = (pixel_t **)malloc(data->size_y * PTRSIZE);
		for (int i = 0; i < data->size_y; i++)
			(*img)[i] = (pixel_t *)malloc(data->size_x * PSIZE);
	}
	if (data->magic == 2)
		load_p2(in, *data, *img);
	if (data->magic == 3)
		load_p3(in, *data, *img);
	if (data->magic == 5)
		load_p5(in, *data, *img);
	if (data->magic == 6)
		load_p6(in, *data, *img);
}

void save_binary(metadata_t data, pixel_t **img, char *filename)
{
	FILE *out = fopen(filename, "wb");
	if (data.magic == 2 || data.magic == 5)
		fprintf(out, "P5\n");
	if (data.magic == 3 || data.magic == 6)
		fprintf(out, "P6\n");
	fprintf(out, "%d %d\n%d\n", data.size_x, data.size_y, data.maxval);
	if (data.magic == 2 || data.magic == 5) {
		for (int i = 0; i < data.size_y; i++)
			for (int j = 0; j < data.size_x; j++)
				putc(img[i][j].red, out);
	}
	if (data.magic == 3 || data.magic == 6) {
		for (int i = 0; i < data.size_y; i++)
			for (int j = 0; j < data.size_x; j++) {
				putc(img[i][j].red, out);
				putc(img[i][j].green, out);
				putc(img[i][j].blue, out);
			}
	}
	fclose(out);
	printf("Saved %s\n", filename);
}

void save_ascii(metadata_t data, pixel_t **img, char *filename)
{
	FILE *out = fopen(filename, "w");
	if (data.magic == 2 || data.magic == 5)
		fprintf(out, "P2\n");
	if (data.magic == 3 || data.magic == 6)
		fprintf(out, "P3\n");
	fprintf(out, "%d %d\n%d\n", data.size_x, data.size_y, data.maxval);
	if (data.magic == 2 || data.magic == 5) {
		for (int i = 0; i < data.size_y; i++) {
			for (int j = 0; j < data.size_x; j++)
				fprintf(out, "%d ", img[i][j].red);
			fprintf(out, "\n");
		}
	}
	if (data.magic == 3 || data.magic == 6) {
		for (int i = 0; i < data.size_y; i++) {
			for (int j = 0; j < data.size_x; j++) {
				fprintf(out, "%d ", img[i][j].red);
				fprintf(out, "%d ", img[i][j].green);
				fprintf(out, "%d ", img[i][j].blue);
			}
			fprintf(out, "\n");
		}
	}
	fclose(out);
	printf("Saved %s\n", filename);
}

area_t sel_all(metadata_t data)
{
	area_t newarea;
	newarea.x1 = 0;
	newarea.y1 = 0;
	newarea.x2 = data.size_x;
	newarea.y2 = data.size_y;
	return newarea;
}

area_t sel_area(metadata_t data, area_t area, int x1, int y1, int x2, int y2)
{
	area_t newarea;
	if (x1 < x2) {
		newarea.x1 = x1;
		newarea.x2 = x2;
	} else {
		newarea.x1 = x2;
		newarea.x2 = x1;
	}
	if (y1 < y2) {
		newarea.y1 = y1;
		newarea.y2 = y2;
	} else {
		newarea.y1 = y2;
		newarea.y2 = y1;
	}
	if (newarea.x2 > data.size_x || newarea.x1 < 0) {
		printf("Invalid set of coordinates\n");
		return area;
	}
	if (newarea.y2 > data.size_y || newarea.y1 < 0) {
		printf("Invalid set of coordinates\n");
		return area;
	}
	if (newarea.x1 == newarea.x2) {
		printf("Invalid set of coordinates\n");
		return area;
	}
	if (newarea.y1 == newarea.y2) {
		printf("Invalid set of coordinates\n");
		return area;
	}
	printf("Selected ");
	printf("%d %d %d %d\n", newarea.x1, newarea.y1, newarea.x2, newarea.y2);
	return newarea;
}

/*În funcție de parametrul comenzii SELECT, efectuează selecția întregii
imagini sau doar a unei zone determinată de 4 numere naturale*/
void sel(metadata_t data, area_t *area, char *token)
{
	if (strcmp(token, "ALL") == 0) {
		*area = sel_all(data);
		printf("Selected ALL\n");
	} else {
		int x1, y1, x2, y2;
		x1 = atoi(token);
		token = strtok(NULL, " ");
		if (!token || valid_number(token) == 0) {
			printf("Invalid command\n");
			return;
		}
		y1 = atoi(token);
		token = strtok(NULL, " ");
		if (!token || valid_number(token) == 0) {
			printf("Invalid command\n");
			return;
		}
		x2 = atoi(token);
		token = strtok(NULL, " ");
		if (!token || valid_number(token) == 0) {
			printf("Invalid command\n");
			return;
		}
		y2 = atoi(token);
		*area = sel_area(data, *area, x1, y1, x2, y2);
	}
}

//Aplică filtrul alb-negru asupra selecției specificate
void grayscale(pixel_t **img, area_t area)
{
	for (int i = area.y1; i < area.y2; i++)
		for (int j = area.x1; j < area.x2; j++) {
			double nv = img[i][j].red + img[i][j].green + img[i][j].blue;
			nv = nv / 3.00;
			img[i][j].red = round(nv);
			img[i][j].green = round(nv);
			img[i][j].blue = round(nv);
		}
	printf("Grayscale filter applied\n");
}

//Aplică filtrul sepia asupra selecției specificate
void sepia(pixel_t **img, area_t area)
{
	for (int i = area.y1; i < area.y2; i++)
		for (int j = area.x1; j < area.x2; j++) {
			double nred = 0.393 * img[i][j].red;
			nred += 0.769 * img[i][j].green + 0.189 * img[i][j].blue;
			double ngreen = 0.349 * img[i][j].red;
			ngreen += 0.686 * img[i][j].green + 0.168 * img[i][j].blue;
			double nblue = 0.272 * img[i][j].red;
			nblue += 0.534 * img[i][j].green + 0.131 * img[i][j].blue;
			if (nred > 255)
				img[i][j].red = 255;
			else
				img[i][j].red = round(nred);
			if (ngreen > 255)
				img[i][j].green = 255;
			else
				img[i][j].green = round(ngreen);
			if (nblue > 255)
				img[i][j].blue = 255;
			else
				img[i][j].blue = round(nblue);
		}
	printf("Sepia filter applied\n");
}

//Copiază valorile din selecția imaginii img în imaginea newimg
void copy_sel(pixel_t **img, pixel_t **newimg, area_t area)
{
	for (int i = area.y1; i < area.y2; i++)
		for (int j = area.x1; j < area.x2; j++) {
			newimg[i - area.y1][j - area.x1].red = img[i][j].red;
			newimg[i - area.y1][j - area.x1].green = img[i][j].green;
			newimg[i - area.y1][j - area.x1].blue = img[i][j].blue;
		}
}

//Rotește întreaga imagine cu 90 de grade la dreapta
void rotate_all(metadata_t data, pixel_t **img, pixel_t **newimg)
{
	for (int i = 0; i < data.size_y; i++)
		for (int j = 0; j < data.size_x; j++) {
			newimg[j][data.size_y - 1 - i].red = img[i][j].red;
			newimg[j][data.size_y - 1 - i].green = img[i][j].green;
			newimg[j][data.size_y - 1 - i].blue = img[i][j].blue;
		}
}

//Rotește o selecție pătratică cu 90 de grade la dreapta
void rotate_sel(pixel_t **img, area_t area)
{
	int n = area.x2 - area.x1;
	pixel_t **newimg;
	newimg = (pixel_t **)malloc(n * PTRSIZE);
	for (int i = 0; i < n; i++)
		newimg[i] = (pixel_t *)malloc(n * PSIZE);
	for (int i = area.y1; i < area.y2; i++)
		for (int j = area.x1; j < area.x2; j++) {
			newimg[j - area.x1][n - i + area.y1 - 1].red = img[i][j].red;
			newimg[j - area.x1][n - i + area.y1 - 1].green = img[i][j].green;
			newimg[j - area.x1][n - i + area.y1 - 1].blue = img[i][j].blue;
		}
	for (int i = area.y1; i < area.y2; i++)
		for (int j = area.x1; j < area.x2; j++) {
			img[i][j].red = newimg[i - area.y1][j - area.x1].red;
			img[i][j].green = newimg[i - area.y1][j - area.x1].green;
			img[i][j].blue = newimg[i - area.y1][j - area.x1].blue;
		}
	for (int i = 0; i < n; i++)
		free(newimg[i]);
	free(newimg);
}

/*Construiește o copie a zonei selectate și înlocuiește imaginea originală
cu copia realizată*/
void crop(metadata_t *data, pixel_t ***img, area_t *area)
{
	pixel_t **newimg;
	int new_x = area->x2 - area->x1;
	int new_y = area->y2 - area->y1;
	newimg = (pixel_t **)malloc(new_y * PTRSIZE);
	for (int i = 0; i < new_y; i++)
		newimg[i] = (pixel_t *)malloc(new_x * PSIZE);
	copy_sel(*img, newimg, *area);
	closeimage(*data, *img);
	data->size_x = new_x;
	data->size_y = new_y;
	*img = (pixel_t **)malloc(data->size_y * PTRSIZE);
	for (int i = 0; i < data->size_y; i++)
		(*img)[i] = (pixel_t *)malloc(data->size_x * PSIZE);
	*area = sel_all(*data);
	copy_sel(newimg, *img, *area);
	closeimage(*data, newimg);
	printf("Image cropped\n");
}

void rotate(metadata_t *data, pixel_t ***img, area_t *area, int deg)
{
	int nr = rotation_number(deg);
	if (nr != -1) {
		if ((area->x2 - area->x1) == (area->y2 - area->y1)) {
			for (int i = 0; i < nr; i++)
				rotate_sel(*img, *area);
			printf("Rotated %d\n", deg);
			return;
		}
		if (all_is_selected(*data, *area) == 1) {
			for (int i = 0; i < nr; i++) {
				pixel_t **newimg;
				newimg = (pixel_t **)malloc(area->x2 * PTRSIZE);
				for (int i = 0; i < area->x2; i++)
					newimg[i] = (pixel_t *)malloc(area->y2 * PSIZE);
				rotate_all(*data, *img, newimg);
				closeimage(*data, *img);
				int aux = data->size_x;
				data->size_x = data->size_y;
				data->size_y = aux;
				*img = (pixel_t **)malloc(area->x2 * PTRSIZE);
				for (int i = 0; i < area->x2; i++)
					(*img)[i] = (pixel_t *)malloc(area->y2 * PSIZE);
				*area = sel_all(*data);
				copy_sel(newimg, *img, *area);
				closeimage(*data, newimg);
			}
			printf("Rotated %d\n", deg);
			return;
		}
		printf("The selection must be square\n");
		return;
	}
	printf("Unsupported rotation angle\n");
}

int main(void)
{
	pixel_t **img;
	metadata_t data;
	area_t area;
	clear_metadata(&data);
	char *cmd = (char *)malloc(MAXCMDSIZE * sizeof(char));
	while (1) {
		fgets(cmd, MAXCMDSIZE, stdin);
		if (cmd[strlen(cmd) - 1] == '\n')
			cmd[strlen(cmd) - 1] = '\0';
		char *token;
		token = strtok(cmd, " ");
		if (strcmp(token, "EXIT") == 0) {
			if (data.magic == 0)
				printf("No image loaded\n");
			free(cmd);
			closeimage(data, img);
			return 0;
		}
		if (strcmp(token, "LOAD") == 0) {
			token = strtok(NULL, " ");
			closeimage(data, img);
			FILE *in = fopen(token, "rb");
			if (!in) {
				printf("Failed to load %s\n", token);
				clear_metadata(&data);
				continue;
			}
			load_image(in, &data, &img, token);
			area = sel_all(data);
			fclose(in);
			continue;
		}
		if (data.magic == 0) {
			printf("No image loaded\n");
			continue;
		}
		if (strcmp(token, "SELECT") == 0) {
			token = strtok(NULL, " ");
			sel(data, &area, token);
			continue;
		}
		if (strcmp(token, "ROTATE") == 0) {
			token = strtok(NULL, " ");
			int deg = atoi(token);
			rotate(&data, &img, &area, deg);
			continue;
		}
		if (strcmp(token, "CROP") == 0) {
			crop(&data, &img, &area);
			continue;
		}
		if (strcmp(token, "GRAYSCALE") == 0) {
			if (data.magic == 3 || data.magic == 6)
				grayscale(img, area);
			else
				printf("Grayscale filter not available\n");
			continue;
		}
		if (strcmp(token, "SEPIA") == 0) {
			if (data.magic == 3 || data.magic == 6)
				sepia(img, area);
			else
				printf("Sepia filter not available\n");
			continue;
		}
		if (strcmp(token, "SAVE") == 0) {
			token = strtok(NULL, " ");
			char *filename = (char *)malloc(MAXCMDSIZE * sizeof(char));
			strcpy(filename, token);
			token = strtok(NULL, " ");
			if (token && strcmp(token, "ascii") == 0)
				save_ascii(data, img, filename);
			else
				save_binary(data, img, filename);
			free(filename);
			continue;
		}
		printf("Invalid command\n");
	}
}
