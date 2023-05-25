#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <conio.h>
#include <Windows.h>
#include <time.h>

// 색상 정의
#define BLACK	0
#define BLUE1	1
#define GREEN1	2
#define CYAN1	3
#define RED1	4
#define MAGENTA1 5
#define YELLOW1	6
#define GRAY1	7
#define GRAY2	8
#define BLUE2	9
#define GREEN2	10
#define CYAN2	11
#define RED2	12
#define MAGENTA2 13
#define YELLOW2	14
#define WHITE	15

//조작 및 캐릭터 관련
#define STAR '*'
#define LASER "○" //*로 대체할 예정
#define STAR1 "★" // player1 표시
#define BLANK "  " // ' ' 로하면 흔적이 지워진다 

#define ESC 0x1b //  ESC 누르면 종료

#define SPECIAL1 0xe0 // 특수키는 0xe0 + key 값으로 구성된다.
#define SPECIAL2 0x00 // keypad 경우 0x00 + key 로 구성된다.

#define UP  0x48 // Up key는 0xe0 + 0x48 두개의 값이 들어온다.
#define DOWN 0x50
#define LEFT 0x4b
#define RIGHT 0x4d

#define WIDTH 80
#define HEIGHT 24

//전역변수
int Delay = 10; // 100 msec delay, 이 값을 줄이면 속도가 빨라진다.
int keep_moving = 0; // 1:계속이동, 0:한칸씩이동.
int player_pos[WIDTH][HEIGHT] = { 0 };
int xLaser[WIDTH][HEIGHT] = { 0 };
int yLaser[WIDTH][HEIGHT] = { 0 };
int xLaser_count = 0;
int yLaser_count = 0;
int laserinterval = 1;
int score[20] = { 0 };
int user_score = 0;
int Heart = 3; //목숨 개수
int Pcolor_frame; //목숨이 깎였을 때 색깔 표시
int item[WIDTH][HEIGHT] = { 0 }; // 1이면 item 있다는 뜻
int iteminterval = 30; // item 표시 간격
int called[2];
int frame_count = 0;
int Laser_frame_sync = 100;
int p_frame_sync = 10;
int p_frame_sync_start = 0;

void gotoxy(int x, int y) //내가 원하는 위치로 커서 이동
{
	COORD pos; // Windows.h 에 정의
	pos.X = x;
	pos.Y = y;
	SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), pos);
}

void putstar(int x, int y, char ch)
{
	gotoxy(x, y);
	printf(STAR1);
}
void erasestar(int x, int y)
{
	gotoxy(x, y);
	printf(BLANK);
}

void textcolor(int fg_color, int bg_color)
{
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), fg_color | bg_color << 4);
}

void removeCursor(void) { // 커서를 안보이게 한다
	CONSOLE_CURSOR_INFO curInfo;
	GetConsoleCursorInfo(GetStdHandle(STD_OUTPUT_HANDLE), &curInfo);
	curInfo.bVisible = 0;
	SetConsoleCursorInfo(GetStdHandle(STD_OUTPUT_HANDLE), &curInfo);
}
void showCursor(void) { // 커서를 보이게 한다
	CONSOLE_CURSOR_INFO curInfo;
	GetConsoleCursorInfo(GetStdHandle(STD_OUTPUT_HANDLE), &curInfo);
	curInfo.bVisible = 1;
	SetConsoleCursorInfo(GetStdHandle(STD_OUTPUT_HANDLE), &curInfo);
}
void cls(int bg_color, int text_color) // 화면 지우기
{
	char cmd[100];
	system("cls");
	sprintf(cmd, "COLOR %x%x", bg_color, text_color);
	system(cmd);
}

// box 그리기 함수, ch 문자열로 (x1,y1) ~ (x2,y2) box를 그린다.
// 한글 문자를 그리는 용도로 사용 "□" 로 벽을 그리는 경우
void draw_box2(int x1, int y1, int x2, int y2)
{
	int x, y;
	//int len = strlen(ch);
	for (x = x1; x <= x2; x += 2) { // 한글은 2칸씩 차지한다.
		gotoxy(x, y1);
		printf("%s", "─"); // ㅂ 누르고 한자키 누르고 선택
		gotoxy(x, y2);
		printf("%s", "─");
	}
	for (y = y1; y <= y2; y++) {
		gotoxy(x1, y);
		printf("%s", "│");
		gotoxy(x2, y);
		printf("%s", "│");
	}
	gotoxy(x1, y1); printf("┌");
	gotoxy(x1, y2); printf("└");
	gotoxy(x2, y1); printf("┐");
	gotoxy(x2, y2); printf("┘");
}

void show_Heart() {
	gotoxy(2, 23);

	switch (Heart) {
	case 0:
		printf("        ");
		textcolor(WHITE, BLACK);
		gotoxy(2, 23);
		break;
	case 1:
		printf("        ");
		textcolor(WHITE, BLACK);
		gotoxy(2, 23);
		printf("♥");
		break;
	case 2:
		printf("        ");
		textcolor(WHITE, BLACK);
		gotoxy(2, 23);
		printf("♥ ♥");
		break;
	case 3:
		printf("        ");
		textcolor(WHITE, BLACK);
		gotoxy(2, 23);
		printf("♥ ♥ ♥");
		break;
	}
}

void pm_Heart(int val) {
	if (val == 1) {
		Heart++;
		show_Heart();
	}
	else {
		Heart--;
		show_Heart();
	}
}

void player1(unsigned char ch)
{
	static int oldx = 10, oldy = 10, newx = 10, newy = 10;
	int move_flag = 0;
	static unsigned char last_ch = 0;

	if (called[0] == 0) { // 처음 또는 Restart
		oldx = 10, oldy = 10, newx = 10, newy = 10;
		putstar(oldx, oldy, STAR1);
		called[0] = 1;
		last_ch = 0;
		ch = 0;
	}
	
	last_ch = ch;

	switch (ch) {// 0 은 Status Line
	case UP:
		if (oldy > 1) {
			newy = oldy - 1;
			player_pos[oldx][oldy] = 0;
			player_pos[newx][newy] = 1;
		}
		else {
			newy = oldy;
		}
		move_flag = 1;
		break;
	case DOWN:
		if (oldy < HEIGHT - 3) {
			newy = oldy + 1;
			player_pos[oldx][oldy] = 0;
			player_pos[newx][newy] = 1;
		}
		else {
			newy = oldy;
		}
		move_flag = 1;
		break;
	case LEFT:
		if (oldx > 2) {
			newx = oldx - 2;
			player_pos[oldx][oldy] = 0;
			player_pos[newx][newy] = 1;
		}
		else {
			newx = oldx;
		}
		move_flag = 1;
		break;
	case RIGHT:
		if (oldx < WIDTH - 4) {
			newx = oldx + 2;
			player_pos[oldx][oldy] = 0;
			player_pos[newx][newy] = 1;
		}
		else {
			newx = oldx;
		}
		move_flag = 1;
		break;
	}
	if (move_flag) {
		if (Pcolor_frame > 0) {
			textcolor(BLUE1, BLACK);
		}
		erasestar(oldx, oldy); // 마지막 위치의 * 를 지우고
		putstar(newx, newy, STAR1); // 새로운 위치에서 * 를 표시한다.
		oldx = newx; // 마지막 위치를 기억한다.
		oldy = newy;
		move_flag = 0;
	}
}

void Laser_start() {
	int x, y, set, i, tmp;
	set = rand() % 10;
	if (set < 7) {
		while (1) {
			tmp = 0;
			x = rand() % (WIDTH - 5) + 2;
			for (i = 0; i < HEIGHT; i++) {
				if (xLaser[x][i] || x % 2 != 0)
					tmp = 1;
			}
			if (tmp == 1) continue;
			break;
		}
		textcolor(YELLOW1, YELLOW1);
		gotoxy(x, 1);
		printf(LASER);
		xLaser[x][1] = 1;
		xLaser_count++;
		textcolor(WHITE, BLACK);
	}
	else {
		while (1) {
			tmp = 0;
			y = rand() % (HEIGHT - 4) + 1;
			for (i = 0; i < WIDTH; i++) {
				if (yLaser[i][y])
					tmp = 1;
			}
			if (tmp == 1) continue;
			break;
		}
		textcolor(YELLOW1, YELLOW1);
		gotoxy(2, y);
		printf(LASER);
		yLaser[2][y] = 1;
		yLaser_count++;
		textcolor(WHITE, BLACK);
	}
}

void xLaser_shoot() {
	int x, y;

	if (xLaser_count == 0)
		return;
	for (x = 0; x < WIDTH; x += 2) {
		for (y = 0; y <= HEIGHT; y++) {
			if (xLaser[x][y] && xLaser[x][y + 1] != 1) {
				if (y > 3 && y < HEIGHT - 3) {
					gotoxy(x, y - 3);
					printf(BLANK);
					xLaser[x][y - 3] = 0;
					gotoxy(x, y + 1);
					textcolor(YELLOW1, YELLOW1);
					printf(LASER);
					textcolor(WHITE, BLACK);
					xLaser[x][y + 1] = 1;
					break;
				}

				else if (y <= 3) {
					gotoxy(x, y + 1);
					textcolor(YELLOW1, YELLOW1);
					printf(LASER);
					textcolor(WHITE, BLACK);
					xLaser[x][y + 1] = 1;
					break;
				}
			}
			if (y >= 3 && xLaser[x][y - 3] == 1) {
				gotoxy(x, y - 3);
				printf(BLANK);
				xLaser[x][y - 3] = 0;
				break;
			}
		}
	}
}

void yLaser_shoot() {
	int x, y;

	if (yLaser_count == 0)
		return;
	for (y = 0; y < HEIGHT; y++) {
		for (x = 0; x < WIDTH + 4; x++) {
			if (yLaser[x][y] && yLaser[x + 2][y] != 1) {
				if (x > 7 && x < WIDTH - 5) {
					gotoxy(x - 6, y);
					printf(BLANK);
					yLaser[x - 6][y] = 0;
					gotoxy(x + 2, y);
					textcolor(YELLOW1, YELLOW1);
					printf(LASER);
					textcolor(WHITE, BLACK);
					yLaser[x + 2][y] = 1;
					break;
				}
				else if (x <= 7) {
					gotoxy(x + 2, y);
					textcolor(YELLOW1, YELLOW1);
					printf(LASER);
					textcolor(WHITE, BLACK);
					yLaser[x + 2][y] = 1;
					break;
				}
			}
			if (x >= 7 && yLaser[x - 6][y] == 1) {
				gotoxy(x - 6, y);
				printf(BLANK);
				yLaser[x - 6][y] = 0;
				break;
			}
		}
	}
}

void show_time(int remain_time)
{
	gotoxy(31, 24);
	textcolor(WHITE, YELLOW1);
	printf("시간 : %02d", remain_time);
	textcolor(WHITE, BLACK);
}

void show_score() {
	int i, y = 2;
	int length = 20;
	gotoxy(82, 1);
	printf("[점수판]");
	for (i = 0; i < length; i++) {
		gotoxy(82, y);
		printf("%2d : %010d", i + 1, score[i]);
		y++;
	}
}

void show_user_score(int user_sc) {
	gotoxy(62, 23);
	textcolor(WHITE, YELLOW1);
	printf("점수 : %010d", user_sc);
	textcolor(WHITE, BLACK);
}

void StartMenu() {
	//초기화면
	gotoxy(2, 1);
	textcolor(YELLOW2, BLACK);
	printf("■       ■■■■ ■■■■ ■■■■ ■■■■\n");
	gotoxy(2, 2);
	printf("■       ■    ■ ■       ■       ■    ■\n");
	gotoxy(2, 3);
	printf("■       ■■■■ ■■■■ ■■■■ ■■■  \n");
	gotoxy(2, 4);
	printf("■       ■    ■       ■ ■       ■    ■\n");
	gotoxy(2, 5);
	printf("■■■■ ■    ■ ■■■■ ■■■■ ■    ■\n");
	gotoxy(34, 17);
	printf("■■■     ■■   ■■■   ■■■■ ■■■■\n");
	gotoxy(34, 18);
	printf("■    ■ ■    ■ ■    ■ ■       ■      \n");
	gotoxy(34, 19);
	printf("■    ■ ■    ■ ■    ■ ■  ■■ ■■■■\n");
	gotoxy(34, 20);
	printf("■    ■ ■    ■ ■    ■ ■    ■ ■      \n");
	gotoxy(34, 21);
	printf("■■■     ■■   ■■■   ■■■■ ■■■■\n");

	textcolor(BLUE1, BLACK);
	gotoxy(65, 2);
	printf("■");
	gotoxy(63, 3);
	printf("■##■");
	gotoxy(61, 4);
	printf("■■■■■");
	gotoxy(63, 5);
	printf("■  ■");
	//초기설정
	int x, y;
	srand(time(NULL));
	user_score = 0;
	for (x = 0; x < WIDTH; x++) {
		for (y = 0; y < HEIGHT; y++) {
			item[x][y] = 0;
			xLaser[x][y] = 0;
			yLaser[x][y] = 0;
			player_pos[x][y] = 0;
		}
	}
	player_pos[10][10] = 1;
	keep_moving = 1;
	Heart = 3;
	xLaser_count = 0;
	yLaser_count = 0;
	iteminterval = 30;
	laserinterval = 1;
	called[0] = called[1] = 0;
	frame_count = 0;
	p_frame_sync = 10;
	p_frame_sync_start = 0;
	Laser_frame_sync = 5;

	while (1) {
		int c1, c2;
		do {
			c1 = rand() % 16;
			c2 = rand() % 16;
		} while (c1 == c2);
		textcolor(c1, BLACK);
		gotoxy(21, 11);
		printf("- 시작하려면 아무 키나 입력하십시오... -");
		Sleep(300);
		if (_kbhit()) {
			system("mode con cols=100 lines=25");
			cls(BLACK, WHITE);
			textcolor(WHITE, BLACK);
			draw_box2(0, 0, 78, 22);
			show_Heart();
			show_score();
			break;
		}
	}
}

void main()
{
	unsigned char ch; // 특수키 0xe0 을 입력받으려면 unsigned char 로 선언해야 함
	int oldx, oldy, newx, newy;

	int run_time, start_time, remain_time, last_remain_time;
	int laser_time = 0;
	int Lx, Ly, sh, lasercount = 0;

	clock_t start = 0, now = 0, oldscore = 0, miliscore = 0;


	newx = oldx = 10;
	newy = oldy = 10;

START:
	laser_time = 0;
	lasercount = 0;
	removeCursor(); // 커서를 안보이게 한다
	system("mode con cols=80 lines=24");
	cls(BLACK, WHITE);
	draw_box2(0, 0, 78, 22);
	StartMenu(); //시작화면

	putstar(oldx, oldy, STAR);
	ch = 0;
	keep_moving = 0;
	//시간측정
	start_time = time(NULL);
	remain_time = 0;
	//점수측정
	start = clock;
	while (1) {
		run_time = time(NULL) - start_time;
		if (run_time > laser_time && (run_time % laserinterval == 0)) {
			Laser_start();
			laser_time = run_time;
		}
		oldscore = miliscore;
		now = clock();
		miliscore = (now - start) % CLOCKS_PER_SEC;
		if (miliscore > oldscore) {
			user_score++;
		}
		show_user_score(user_score);
		if (Heart <= 0)
			break;

		if (_kbhit() == 1) {
			char c1;
			c1 = _getch();
			if (c1 == '\0')
				continue;
			else
				ch = c1;
			if (ch == ESC) {
				exit(0);
			}
			if (ch == SPECIAL1) {
				ch = _getch();
				switch (ch) {
				case UP:
				case DOWN:
				case LEFT:
				case RIGHT:
					player1(ch);
					break;
				default:
					keep_moving = 1;
				}
			}
		}
		if (frame_count % Laser_frame_sync == 0) {
			xLaser_shoot();
			yLaser_shoot();
		}
		for (int x = 0; x < WIDTH; x++) {
			for (int y = 0; y < HEIGHT; y++) {
				if (Pcolor_frame == 0 && ((xLaser[x][y] == 1 && player_pos[x][y] == 1) || (yLaser[x][y] == 1 && player_pos[x][y] == 1))) {
					pm_Heart(0);
					Pcolor_frame += 50;
				}
			}
		}
		Sleep(Delay);
		frame_count++;
		if (Pcolor_frame > 0) {
			Pcolor_frame--;
		}
		
	} //게임 진행
	//게임 오버
	gotoxy(25, 7);
	textcolor(GREEN2, BLACK);
	printf(" 당신의 점수는 : %10d ",user_score);
	for (int i = 0; i < 20; i++) {
		if (user_score > score[i] && i == 19) {
			score[i] = user_score;
			break;
		}
		else if (user_score > score[i]) {
			score[i + 1] = score[i];
			score[i] = user_score;
			break;
		}
	}
	while (1) {
		int c1, c2;
		do { // 색을 변경하면서 Game Over 출력
			c1 = rand() % 16;
			c2 = rand() % 16;
		} while (c1 == c2);
		textcolor(c1, c2);
		gotoxy(29, 10);
		printf(" -★☆GAME OVER☆★- ");
		gotoxy(24, 13);
		textcolor(WHITE, BLACK);
		printf("Hit (R) to Restart (Q) to Quit");
		Sleep(300);
		if (kbhit()) {
			ch = getch();
			if (ch == 'r' || ch == 'q')
				break;
		}
	}
	if (ch == 'r')
		goto START;

	textcolor(BLACK, WHITE);
	gotoxy(21, 26); //끝나는 화면 위치

}