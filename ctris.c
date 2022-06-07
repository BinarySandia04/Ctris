#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <string.h>

#define HEIGHT 20
#define WIDTH 10

typedef struct {
	int *data;
} Piece;

Piece *peces;

char buf[1];

int currentPiece = 0;
int currentRotation = 0;
int pieces = 0;

int npieces = -1;

unsigned int crows, ccols;
void definePieces(FILE* gameData);
void printLine();
void cleanScreen();
void startGame(FILE* gameData);
void dropPiece(int* taula);
void exitGame();
void getHitbox(int* readData, int* writeData);
void choosePiece();
void resetCursor();
void prepareFile(FILE* gameData);
void refreshConsoleSize();
void printCentered(char * text);
void swap(int *a, int *b);
void permutateBag(int quina, int start, int end, int *bago);
void printPiece(int code);
void resetTaulell();
int pieceFits(int *taula, int xc, int xy, int rotation, int piece);

int isInCurrentPiece(int x, int y, int cursorx, int cursory);

int cursorx, cursory;

int maxSoftTime;
int defaultSoftTime;

int taula[HEIGHT][WIDTH];

int main(int argc, char const *argv[])
{
	FILE* gameData;

	int frame = 0;
	int punts = 0;
	maxSoftTime = 300;
	defaultSoftTime = 60;
	int currentSoftTime = 0;

	gameData = fopen("ctris.data", "r");
	if(gameData == NULL){
		printf("No s'ha pogut obrir ctris.data\n");
		exit(1);
	}
	
	prepareFile(gameData);

	startGame(gameData);

	fclose(gameData);

	refreshConsoleSize();

	resetTaulell();

	resetCursor();
	
	choosePiece();

	do {
		fcntl(0, F_SETFL, fcntl(0, F_GETFL) | O_NONBLOCK);
		char input = buf[0];
		

		int writeData[4];
		getHitbox(peces[currentPiece].data + currentRotation * 16, writeData);
		int left = writeData[0], right = writeData[1], up = writeData[2], down = writeData[3];

		if(input == 'a'){
			if(pieceFits(taula, cursorx - 1, cursory, currentRotation, currentPiece)) cursorx--;
		}
		if(input == 'd'){
			if(pieceFits(taula, cursorx + 1, cursory, currentRotation, currentPiece)) cursorx++;
		}
		
		if(pieceFits(taula, cursorx, cursory + 1, currentRotation, currentPiece)){
			currentSoftTime = 0;
		} else {
			currentSoftTime++;
			if(currentSoftTime > defaultSoftTime){
				dropPiece(taula);
				resetCursor();
				choosePiece();
			}

		}

		if(frame % 100 == 0 || (input == 's')){
			if(pieceFits(taula, cursorx, cursory + 1, currentRotation, currentPiece)){
				cursory++;
			}
			punts ++;
		}

		if(input == 'k'){
			currentRotation++;
			currentRotation %= 4;
		}
		if(input == 'j'){
			currentRotation--;
			if(currentRotation == -1) currentRotation = 3;
		}
		if(input == 'r'){
			dropPiece(taula);
			resetCursor();
			choosePiece();

			resetTaulell();
		}

		if(input == ' '){
			/* Hard drop */
			dropPiece(taula);
			
			resetCursor();
			choosePiece();
	
		}

		for(int y = 0; y < HEIGHT; y++){
			int borrar = 1;
			for(int x = 0; x < WIDTH; x++){
				if(!taula[y][x]) borrar = 0;
			}
			if(borrar){
				for(int linia = y; linia > 0; linia--){
					for(int j = 0; j < WIDTH; j++){
						taula[linia][j] = taula[linia - 1][j];
					}
				}
			}

			if(y < 2){
				for(int x = 0; x < WIDTH; x++){
					if(taula[y][x]){
						resetTaulell();
					}
				}
			}
		}

		/* Mostrar el taulell */

		int y;
		
		
		printLine();
		
		char *text = "CTRIS";
		
		printCentered(text);
		char puntsText[15];
		sprintf(puntsText, "PUNTS: %06d", punts);
		printCentered(puntsText);

		printLine();

		for(int z = 4; z < crows / 2 - HEIGHT / 2; z++) printf("\n");

		for(int u = 0; u < ccols / 2 - WIDTH; u++) printf(" "); 
		for(y = 0; y < WIDTH * 2 + 2; y++) printf("-");
		for(y = 0; y < WIDTH * 2 + 2 + ccols / 2 - WIDTH; y++) printf("\b");
		printf("\n");
		
		for(y = 0; y < HEIGHT; y++){
			int x;

			for(int u = 0; u < ccols / 2 - WIDTH; u++) printf(" "); 

			printf("|");
			for(x = 0; x < WIDTH; x++){
				int npr = 1;
				if(isInCurrentPiece(x,y,cursorx,cursory)){
					printPiece(currentPiece+1);
					npr = 0;
				}
				if(taula[y][x]){
					printPiece(taula[y][x]);
					npr = 0;
				}
				if(npr) printf("  ");
			}
			printf("|");
			printf("\n");
			for(x = 0; x < WIDTH * 2 + 2 + ccols/2 - WIDTH; x++) printf("\b");
		}
		for(int u = 0; u < ccols / 2 - WIDTH; u++) printf(" "); 
		for(y = 0; y < WIDTH * 2 + 2; y++) printf("-");
		printf("\n");
		
		for(y = 0; y < WIDTH * 2 + 2 + ccols / 2; y++) printf("\b");
		for(y = 0; y < 4; y++) printf("\n");
		cleanScreen();

		frame++;
	} while(buf[0] != '.');

	exitGame();
}

void printLine(){
	int y;
	for(y = 0; y < ccols; y++) printf("-");
	for(y = 0; y < ccols; y++) printf("\b");
	printf("\n");	
}

void printCentered(char * text){
	printf("%*s", ccols / 2 + strlen(text) / 2, text);
	for(int y = 0; y < ccols/2+strlen(text); y++) printf("\b");
	printf("\n");
}

void cleanScreen(){
		/* Netejar el taulell */

		buf[0] = 0;
		usleep(10000);

		printf("\e[1;1H\e[2J");
		read(0,buf,1);

		refreshConsoleSize();
}

void definePieces(FILE* gameData){
	
	int i, j, r;
	fscanf(gameData, "%d", &npieces);
	printf("%d\n", npieces);

	peces = (Piece*) malloc(npieces * sizeof(Piece));

	for(i = 0; i < npieces; i++){
		peces[i].data = (int *) malloc(64 * sizeof(int));
		for(j = 0; j < 64; j++){
			fscanf(gameData, "%d", &r);
			peces[i].data[j] = r;
			printf("%d ", peces[i].data[j]);
		}
		printf("\n");	
	}
}



void startGame(FILE* gameData){

	definePieces(gameData);
	srand((unsigned) time(NULL));

	system ("/bin/stty raw");
}

void exitGame(){
	int i = 0;

	for(i = 0; i < npieces; i++){
		free(peces[i].data);
	}
		
	free(peces);
	system ("/bin/stty cooked");
}

void getHitbox(int* readData, int* writeData){
	int left = -1;
	int right = -1;
	int up = -1;
	int down = -1;

	for(int y = 0; y < 4; y++){
		for(int x = 0; x < 4; x++){
			if(readData[y * 4 + x] == 1){
				if(left == -1) left = x;
				if(right == -1) right = x;
				if(up == -1) up = y;
				if(down == -1) down = y;

				if(down > y) down = y;
				if(up < y) up = y;
				if(left > x) left = x;
				if(right < x) right = x;
			}
		}
	}

	writeData[0]=left;
	writeData[1]=right;
	writeData[2]=up;
	writeData[3]=down;

}


int bag[7] = {-1,-1,-1,-1,-1,-1,-1};
int bagCounter = 0;
int chosenBag;

void choosePiece(){
	int nb = 1;
	for(int i = 0; i < 7; i++){
		if(bag[i] == -1) continue;
		nb = 0;
		currentPiece = bag[i];
		currentRotation = 0;

		bag[i] = -1;
		break;
	}
	if(nb){

		int bagID = rand() % 5040; // 7!
		
		int nb[7];
		for(int i = 0; i < 7; i++){
			nb[i] = i;
		}

		chosenBag = 0;
		permutateBag(bagID, 0, 6, nb);
	
		choosePiece();
	}
	
}

void permutateBag(int quina, int start, int end, int *bago){
	if(chosenBag) return;

	if(start == end){
		if(quina == bagCounter){
			bagCounter = 0;
			chosenBag = 1;

			for(int i = 0; i < 7; i++){
				bag[i] = *(bago + i);
				printf("%d ", bag[i]);
			}

			return;
		}
		bagCounter++;
	}

	for(int i = start; i < end + 1; i++){
		swap(bago + start, bago + i);
		permutateBag(quina, start + 1, end, bago);
		swap(bago + i, bago + start);
	}
}

void swap(int *a, int *b){
	int c = *a;
	*a = *b;
	*b = c;
}

void dropPiece(int* taula){
	pieces++;
	// taula[19 * WIDTH + 4] = 1;
	// currentPiece
	
	int ybed = cursory;
	
	while(ybed < HEIGHT){
		int grounded = 0;
		for(int xd = 0; xd < 4; xd++){
			if(xd + cursorx > WIDTH || xd + cursorx < 0) continue; 
			for(int yd = 0; yd < 4; yd++){
				if(!peces[currentPiece].data[currentRotation * 16 + yd * 4 + xd]) continue;
				if(yd + 1 < 4){
					if(peces[currentPiece].data[currentRotation * 16 + (yd + 1) * 4 + xd]) continue;
				}

				if(taula[(ybed + yd + 1) * WIDTH + xd + cursorx]) grounded = 1;
				if(ybed + yd >= HEIGHT - 1){
					grounded = 1;
					break;
				}
			}
			if(grounded) break;
		}
		if(grounded) break;
		else ybed++;
	}

	
	for(int yd = 0; yd < 4; yd++){
		for(int xd = 0; xd < 4; xd++){
			if(peces[currentPiece].data[currentRotation * 16 + yd * 4 + xd]){
				taula[(ybed + yd) * WIDTH + xd + cursorx] = peces[currentPiece].data[currentRotation * 16 + yd * 4 + xd];
			}	
		}
	}

	// printf("%d\n", ybed);
	// exit(1);
}

int isInCurrentPiece(int x, int y, int cursorx, int cursory){

	if(x >= cursorx && x < cursorx + 4 && y >= cursory && y < cursory + 4){
		int xd = x - cursorx;
		int yd = y - cursory;
		if(peces[currentPiece].data[currentRotation * 16 + yd * 4 + xd]) return 1;
		else return 0;
	}
	return 0;
}

void refreshConsoleSize(){
	struct winsize size;
	ioctl(0, TIOCGWINSZ, (char *) &size);
	crows = size.ws_row;
	ccols = size.ws_col;
}

void resetCursor(){
	cursorx = WIDTH / 2 - 1;
	cursory = -1;
}

int pieceFits(int *taula, int xc, int yc, int rotation, int piece){
	int fits = 1;
	for(int xd = 0; xd < 4; xd++){
		for(int yd = 0; yd < 4; yd++){
			if(!peces[piece].data[rotation * 16 + yd * 4 + xd]) continue;
			if(xc + xd >= WIDTH) fits = 0;
			if(yc + yd >= HEIGHT) fits = 0;
			if(xc + xd < 0) fits = 0;
			if(taula[(yc + yd) * WIDTH + xc + xd]){
				fits = 0;
			}
			if(!fits) break;
		}
	}
	return fits;
}

void printPiece(int code){
	printf("\e[1m");

	switch(code){
		case 1:
			printf("\e[38;5;45m");
			break;
		case 2:
			printf("\e[38;5;226m");
			break;
		case 3:
			printf("\e[38;5;69m");
			break;
		case 4:
			printf("\e[38;5;208m");
			break;
		case 5:
			printf("\e[38;5;196m");
			break;
		case 6:
			printf("\e[38;5;141m");
			break;
		case 7:
			printf("\e[38;5;82m");
			break;
	}

	printf("■ ");

	printf("\e[0m");

}

void prepareFile(FILE* gameData){
	char c = '\0';
	while(c != '#'){
		fscanf(gameData, "%c", &c);
	}
	return;
}

void resetTaulell(){
	// Posar la taula 0s:
	for(int y = 0; y < HEIGHT; y++){
		for(int x = 0; x < WIDTH; x++){
			taula[y][x] = 0;
		}
	}


}
