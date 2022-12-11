///////////////////////////////////
// snake game                    //
// A Woody Chang specific version//
///////////////////////////////////

#include <stdio.h>

// randomize
#include <stdlib.h> // srand
#include <time.h>   // time();

#include <ctype.h>  // tolower(); to avoid pressing caps lock
#include <conio.h>  // getch(); one press sends a char without ENTER
#include <unistd.h> // sleep();
#include <string.h> // strcat(); connect two strings

/////////////   DEFINE THE CANVAS SIZE HERE   ///////////////////////////////
#define canvasY 21 // includes 19 valid Y, 2 borders
#define canvasX 34 // includes 31 valid X, 2 borders, 1 '\n'
#define canvasArea (canvasY * canvasX)
#define drawMeArray (canvasArea * 11) // make sure the number is big enough
/////////////////////////////////////////////////////////////////////////////

char canvas[canvasY][canvasX];
int  HEAD_POSITION[2];            		// [0] = y, [1] = x
int  BODY_POSITION[canvasArea][2] = {}; // this is to record snake (y, x) positions for tail cutting
									//to set all the elements to 0
char BORDER   = '#';
char HEAD     = '@';
char HEAD_DIE = 'Q';
char BODY     = 'o';
char BODY_DIE = '.';
char FRUIT    = 'X';
char REBORN   = '*';
char HELMET   = 'H';
int  DSCORE   = '$';
char HEAD_DIRECTION = 'w';
int  uroption     = 0;
int  high_score   = 0;
int  nowhighscore = 0;
int  score        = 0;
int  count        = 0;
int  caseg        = 0;
int  caseh        = 0;
int  timer        = 100;
int  dtimer       = 100;
int  doublescore  = 0;
int  snakelen     = 1;
int  REBORN_RATE  = 25;
int  HELMET_RATE  = 7;
int  DSCORE_RATE  = 20;
FILE *logFile;

void draw()// set a new array named drawMe that can copy the privious array 
			// and add some control code
{
	// the array canvas[][] includes wrap code '\n' and is well-formated,
	// so to output canvas at a time is actually performing excellent
	// now add color to the elements
	int y, x;
	char k[2];
	char drawMe[drawMeArray] = "";
	for (y = 0; y < canvasY; y++) for (x = 0; x < canvasX; x++)
	{
			switch (canvas[y][x])
			{
				case '@': // HEAD
					strcat(drawMe, "\e[0;32m@\e[m"); break; // green 
				case 'o': // BODY
					strcat(drawMe, "\e[0;32mo\e[m"); break; // green 
				case 'H': // HELMET
					strcat(drawMe, "\e[0;31mH\e[m"); break; // red
				case 'G': // WORMHOLE MODE //it's a hidden trick
					strcat(drawMe, "\e[0;36mG\e[m"); break; // blue 
				case 'Q': // DEAD HEAD
					// color become different with various situation
					// caseg is a variable to differentiate HEAD from '@' or 'G'
					// if caseg is even then change it into color of '@' 
					// otherwise change into color of 'G'
					if(caseh == 1) strcat(drawMe, "\e[0;31mQ\e[m");
					else if(caseg % 2 == 0) strcat(drawMe, "\e[0;32mQ\e[m");
					else strcat(drawMe, "\e[0;36mQ\e[m");
					break;
				case '.': // DEAD BODY
					strcat(drawMe, "\e[0;32m.\e[m"); break;
				case 'X': // FRUIT
					if(doublescore == 1) strcat(drawMe, "\e[0;33mX\e[m");
					else strcat(drawMe, "\e[0;35mX\e[m");
					break;
				case '*': // REBORN
					strcat(drawMe, "\e[0;33m*\e[m"); break;
				case '$':
					strcat(drawMe, "\e[0;33m$\e[m"); break;
				default:
					k[0] = canvas[y][x];
					k[1] = 0;// ascii of '\0' is 0 
					strcat(drawMe, k);
					break;
			}
	}
	// hide the cursor before printing out
	printf("\e[?25l");  
	printf("\e[%d;%dH", (0), (0));
	printf("\n");
	printf("      ");
	printf("\e[44m S N A K E  G A M E \e[m\n");
	// print canvas at once
	printf("%s\n", drawMe);
					// reserve three spaces for score
					// just to make the output tidier
	high_score = (high_score > score) ? high_score : score;
	printf("Score: %3d    High Score: %d\n", score, high_score);
	printf("\e[%d;%dH", (24), (0));
	if (nowhighscore < high_score) printf("\e[0;31m\t NEW RECORD  ! ! ! \e[m");
	printf("\e[%d;%dH", (26), (0));
	return;
}

void fruitPosition(int a)
{
	int i = 0;
	// to find a space to place a new fruit
	while (canvas[0][i] != ' ') i = (rand() % canvasArea);
	switch (a)
	{
		case 0: //create a fruit
			canvas[0][i] = FRUIT; break;
		case 1: // create a reborn fruit
			canvas[0][i] = REBORN; break;
		case 2: // create a helmet
			canvas[0][i] = HELMET; break;
		case 3:
			canvas[0][i] = DSCORE; break;
	}
	return;
}

void init()
{
	int x, y;
	// set all the elements by space, from top to bottom, left to right
	for (y = 0; y < canvasY; y++) for (x = 0; x < canvasX; x++) canvas[y][x] = ' ';
	// draw horizontal border lines
	for (x = 0; x < canvasX; x++) canvas[0][x] = canvas[canvasY - 1][x] = BORDER;
	// draw vertical border lines
	// also add '\n' in order to output canvas content at once later
	for (y = 0; y < canvasY; y++)
	{
		canvas[y][0] = canvas[y][canvasX - 2] = BORDER;
		canvas[y][canvasX - 1] = '\n'; 
	}
	// position head in the center
	HEAD_POSITION[0] = (canvasY - 1) / 2;// 0 --> y
	HEAD_POSITION[1] = (canvasX - 2) / 2;// 1 --> x
	canvas[HEAD_POSITION[0]][HEAD_POSITION[1]] = HEAD;
	// set BODY_POSITION the same as HEAD_POSITION
	BODY_POSITION[0][0] = HEAD_POSITION[0];
	BODY_POSITION[0][1] = HEAD_POSITION[1];
	// position fruit somewhere
	fruitPosition(0);
	fruitPosition(0);
	// read high score
	logFile = fopen("HiScore.txt", "a"); fclose(logFile);// append FILE "HiScore.txt"
	logFile = fopen("HiScore.txt", "r");// read
	// get high_score value from logFile
	fscanf(logFile,"%d", &high_score);
	fclose(logFile);
	nowhighscore = high_score;
	draw();
}

void die()
{
	caseh = (HEAD == 'H')? 1:0;
	// change the snake layout and play dead
	int x, y;
	for (y = 0; y < canvasY; y++)
		for (x = 0; x < canvasX; x++)
		{
			if (canvas[y][x] == HEAD) canvas[y][x] = HEAD_DIE;
			if (canvas[y][x] == BODY) canvas[y][x] = BODY_DIE;
		}
	draw();
	// write high score
	logFile = fopen("HiScore.txt", "w");
	fprintf(logFile, "%d", high_score);
	fclose(logFile);
	return;
}

void cutTail()
{
	// remove the eariest pixel, restore as a space
	canvas[BODY_POSITION[0][0]][BODY_POSITION[0][1]] = ' ';

	// move one pixel ahead to colibrate the corresponding position
	int i;
	for (i = 1; i < canvasArea; i++)
	{
		BODY_POSITION[i - 1][0] = BODY_POSITION[i][0];
		BODY_POSITION[i - 1][1] = BODY_POSITION[i][1];
	}
	return;
}

void cutAllTail()
{
	int j;
	for (j = 0; j < canvasArea; j++) if (canvas[0][j] == 'o') canvas[0][j] = ' ';
	// since tail is all been cut, set all element to 0
	memset(BODY_POSITION, 0, sizeof(BODY_POSITION));
	BODY_POSITION[0][0] = HEAD_POSITION[0];
	BODY_POSITION[0][1] = HEAD_POSITION[1];
	count = 0;
}

int driving()
{
	// speed control
	// k represents sleep time, and will become smaller with the time passing by
	int k;
	k = uroption * 1000 - count * 500;
	// set a maximum speed by limiting a minimum k value
	k = (k < 100000) ? 100000 : k; 
	usleep(k);
	// check if there is any new input
	// if yes, normalize the input
	// if no, follow last direction
	char input, nextstep;
	if (kbhit()) input = tolower(getch()); // make sure it can still work if the player presses caps lock
	else input = HEAD_DIRECTION;

	
/////////////////////////////////////////////////////////////
////   using arrows might cause delay in certain environment
////// arrow keys handling
////switch(input) {
////	case 72:    // key up
////		input = 'w'; break;
////	case 80:    // key down
////		input = 'z'; break;
////	case 75:    // key right
////		input = 'a'; break;
////	case 77:    // key left
////		input = 'd'; break;
////}
/////////////////////////////////////////////////////////////


	// prevent snake from heading back
	switch(input)
	{
		case 'w':
			HEAD_DIRECTION = (HEAD_DIRECTION == 'z') ? HEAD_DIRECTION : input; break;
		case 'z':
			HEAD_DIRECTION = (HEAD_DIRECTION == 'w') ? HEAD_DIRECTION : input; break;
		case 'a':
			HEAD_DIRECTION = (HEAD_DIRECTION == 'd') ? HEAD_DIRECTION : input; break;
		case 'd':
			HEAD_DIRECTION = (HEAD_DIRECTION == 'a') ? HEAD_DIRECTION : input; break;
		case 'g':
			HEAD = (HEAD == '@')? 'G' : '@';
			// make sure caseg is odd when HEAD is 'G'
			caseg ++;
			break;
		case 's':
			return 1;
	}
	// the head turns to the body
	canvas[HEAD_POSITION[0]][HEAD_POSITION[1]] = BODY;
	// new position of the head
	switch(HEAD_DIRECTION)
	{
		case 'w':
			HEAD_POSITION[0]--;// head go up
			break;
		case 'z':
			HEAD_POSITION[0]++;// head go down
			break;
		case 'a':
			HEAD_POSITION[1]--;// head go left
			break;
		case 'd':
			HEAD_POSITION[1]++;// head go right
			break;
	}
	// officically move forward, increase the counter
	count++;
	// store the next pixel in variable nextstep
	nextstep = canvas[HEAD_POSITION[0]][HEAD_POSITION[1]];
	// Wormhole mode, pass through the wall
	// analyze this special case first before positioning head
	if (nextstep == '#' && (HEAD == 'G' || HEAD == 'H')) {
		if (HEAD_POSITION[0] == 0)             HEAD_POSITION[0] = canvasY - 2;
		if (HEAD_POSITION[0] == (canvasY - 1)) HEAD_POSITION[0] = 1;
		if (HEAD_POSITION[1] == 0)             HEAD_POSITION[1] = canvasX - 3;
		if (HEAD_POSITION[1] == (canvasX - 2)) HEAD_POSITION[1] = 1;
		nextstep = canvas[HEAD_POSITION[0]][HEAD_POSITION[1]];	
}
	// position the head
	// replace first and discuss nextstep later
	canvas[HEAD_POSITION[0]][HEAD_POSITION[1]] = HEAD;
	// if the snake gets helmet, wormhole mode
	if(HEAD == HELMET){
		if(timer > 0 && timer <= 100) {
		timer--;
		printf("\e[%d;%dH", (26), (0));
		printf("\e[0;31mHelmet time remaining : %3d\e[m",timer);
		}
		else {
		printf("\e[%d;%dH", (26), (0));
		printf("                                 ");
		HEAD = (caseg % 2 == 0) ? '@': 'G';
		}	
	}
	
	if(doublescore == 1){
		if(dtimer > 0 && dtimer <= 100) {
		dtimer--;
		printf("\e[%d;%dH", (27), (0));
		printf("\e[0;33mDouble score ! ! remaining : %3d\e[m",dtimer);
		}
		else {
		printf("\e[%d;%dH", (27), (0));
		printf("                                 ");
		doublescore = 0;
		}	
	}
		
	// we subsitute a snake tail of its head every time
	// and cut tail when moving
	// while eats 'X' we just don't cut the tail and it seems growing up
	int i;
	// this for loop is used to find an element in BODY_POSITION that is 0
	for (i = 0; i < canvasArea; i++) if (BODY_POSITION[i][0] == 0) break;
	// give the value to BODY_POSITION for cutTail usage
	BODY_POSITION[i][0] = HEAD_POSITION[0];
	BODY_POSITION[i][1] = HEAD_POSITION[1];
				
	// what is in next pixel
	switch(nextstep)
	{
		case '*':
			// get *, the snake reborn
			cutAllTail();
			draw();
			break;
		case '$':
			doublescore = 1;
			dtimer = 100;
			cutTail();
			draw();
			break;
		case 'H':
			HEAD = 'H';
			// reset the timer every time snake obtains a helmet
			timer = 100;
			cutTail();
			draw();
			break;
		case 'X': // now eats a fruit, find a new place for fruit placement
			if(doublescore == 1) score += 20;
			else score += 10;
			fruitPosition(0);
			// there are some chances to get various items
			if (rand() % REBORN_RATE == 0) fruitPosition(1);
			else if (rand() % HELMET_RATE == 0) fruitPosition(2);
			else if	(rand() % DSCORE_RATE == 0) fruitPosition(3);
			draw();
			break;
		case ' ': // safe to continue
			if (count > 3) cutTail();
			draw();
			break;
		case 'o':
			if(BODY_POSITION[0][0] != HEAD_POSITION[0] || BODY_POSITION[0][1] != HEAD_POSITION[1]) cutTail();
			die();
			return 2;
		case '#':
			cutTail();
			die();
			return 2;
	}

	return 0;
}

void cover()// just display the cover
{
	printf("\n");
	printf("      ");           
	printf("\e[44m S N A K E  G A M E \e[m\n"); // SNAKE GAME title with blue background
	printf("#################################\n");
	printf("#                               #\n");
	printf("#                               #\n");
	printf("#     Powered by Woody Chang    #\n");
	printf("#                               #\n");
	printf("#                               #\n");
	printf("#                               #\n");
	printf("#           UP    w             #\n");
	printf("#           DOWN  z             #\n");
	printf("#           LEFT  a             #\n");
	printf("#           RIGHT d             #\n");
	printf("#                               #\n");
	printf("#           PAUSE s             #\n");
	printf("#                               #\n");
	printf("#                               #\n");
	printf("#                               #\n");
	printf("#                               #\n");
	printf("#    PRESS ANY KEY TO START     #\n");
	printf("#                               #\n");
	printf("#                               #\n");
	printf("#################################\n");
	getch();
	return;
}

void option()
{
	// (,) means(y, x)
	// move the cursor to coordinate (3, 0)
	printf("\e[%d;%dH", (3), (0)); 
	printf("#################################\n");
	printf("#                               #\n");
	printf("#     CHOOSE THE DEGREE OF      #\n");
	printf("#          DIFFICULTY           #\n");
	printf("#                               #\n");
	printf("#                               #\n");
	printf("#                               #\n");
	printf("#          EASY   1             #\n");
	printf("#          NORMAL 2             #\n");
	printf("#          HARD   3             #\n");
	printf("#                               #\n");
	printf("#                               #\n");
	printf("#                               #\n");
	printf("#                               #\n");
	printf("#                               #\n");
	printf("#                               #\n");
	printf("#                               #\n");
	printf("#                               #\n");
	printf("#                               #\n");
	printf("#                               #\n");
	printf("#################################\n");
	int a = 0;
	char num;
	while (a == 0){
		num = getch();
		switch(num)
		{
			case '1': uroption = 300; break;
			case '2': uroption = 200; break;
			case '3': uroption = 100; break;
			default : a--;
		}
		a++;
	}
	return;
}

void readysetgo()
{
	printf("\e[%d;%dH", (3), (0)); 
	printf("#################################\n");
	printf("#                               #\n");
	printf("#                               #\n");
	printf("#                               #\n");
	printf("#                               #\n");
	printf("#                               #\n");
	printf("#                               #\n");
	printf("#                               #\n");
	printf("#                               #\n");
	printf("#                               #\n");
	printf("#          READY. . .           #\n");
	printf("#                               #\n");
	printf("#                               #\n");
	printf("#                               #\n");
	printf("#                               #\n");
	printf("#                               #\n");
	printf("#                               #\n");
	printf("#                               #\n");
	printf("#                               #\n");
	printf("#                               #\n");
	printf("#################################\n");
	// stay for one second
	sleep(1);
	
	printf("\e[%d;%dH", (13), (0)); 
	printf("#           SET. . .            #\n");
	sleep(1);
	
	printf("\e[%d;%dH", (13), (0)); 
	printf("#           GO ! ! !            #\n");
	sleep(1);
	
}


int main(void)
{
	// change Windows command prompt to English codepage to ensure escape code enablement
	system("chcp 437");
	// hide the cursor to prevent screen from flashing
	printf("\e[?25l");
	// clean up the screen
	printf("\e[H\e[J");
	// randomize
	srand((unsigned) time(NULL));
	// display cover
	cover();
	// set options
	option();
	readysetgo();
	// environment initiation
	init();
	
	int a = 0;
	int i;
	char k = ' ';
	// repeat running, quit by specified return code
	while (a == 0)
	{
		a = driving();
		switch(a)
		{
			case 1: // user pause
				printf("\e[%d;%dH", (28), (0));
				printf("Pause\tPress ENTER to resume");
				// keep waiting until user press enter
				while (getch() != '\r');
				a = 0;
				printf("\e[%d;%dH", (26), (0));
				printf("                                 \n");
				printf("            SET. . .             \n");
				printf("                                 \n");
				sleep(1);
				printf("\e[%d;%dH", (27), (0));
				printf("            GO ! ! !             \n");
				sleep(1);
				printf("\e[%d;%dH", (27), (0));
				printf("                                 \n");
				break;
			case 2: // die
				for (i = 0; i < canvasArea; i++) (canvas[0][i] == '.')? snakelen++ : snakelen;
				printf("                                 \n");
				printf("                                 \n");
				printf("\e[%d;%dH", (26), (0));
				printf("Game Over! Length of snake : %d\n\nPlay again? (y/n)", snakelen);
				printf("\e[?25h"); // resume the cursor flashing
				
				while (k != 'n')
				{
					// to avoid pressing the caps lock
					k = tolower(getch());
					// play again
					if (k == 'y')
					{
						a = 0;
						// reset
						memset(BODY_POSITION, 0, sizeof(BODY_POSITION));
						HEAD_DIRECTION = 'w';
						HEAD = '@';
						score = 0;
						count = 0;
						snakelen = 1;
						caseg = 0;
						doublescore = 0;
						printf("\e[?25l");// hide the cursor
						printf("\e[%d;%dH", (24), (0));
						printf("                                 \n");
						printf("                                 \n");
						printf("                                 \n");
						printf("                                 \n");
						printf("                                 \n");
						option();
						readysetgo();
						init();
						k = 'n';
					}
				}
				k = ' ';
				break;
		}
	}
	return 0;
}


