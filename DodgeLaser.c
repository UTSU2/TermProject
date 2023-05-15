#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <conio.h>
#include <Windows.h>

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
int Delay = 50; // 100 msec delay, 이 값을 줄이면 속도가 빨라진다.
//int keep_moving = 1; // 1:계속이동, 0:한칸씩이동.
//int time_out = 30; // 제한시간
int score[2] = { 0 };
int item[WIDTH][HEIGHT] = { 0 }; // 1이면 Gold 있다는 뜻
int iteminterval = 7; // GOLD 표시 간격
//int called[2];

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

void showscore()
{
	textcolor(GREEN2, GRAY2);
	gotoxy(60, 24);
	printf("Player1 : %d", score[0]);
	textcolor(BLACK, WHITE);
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
void draw_box2_fill(int x1, int y1, int x2, int y2, int color)
{
	int x, y;

	textcolor(WHITE, BLACK);
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
	//getch();
	for (y = y1 + 1; y < y2; y++) {
		for (x = x1 + 2; x < x2; x++) {
			textcolor(color, color);
			gotoxy(x, y);
			printf(" ");
		}
	}

}
void StartMenu() {
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
			break;
		}
	}
}

void main()
{
	unsigned char ch; // 특수키 0xe0 을 입력받으려면 unsigned char 로 선언해야 함
	int oldx, oldy, newx, newy;
	int keep_moving;

	newx = oldx = 10;
	newy = oldy = 10;

	char buf[100];
	removeCursor(); // 커서를 안보이게 한다

	system("mode con cols=80 lines=24");
	cls(BLACK, WHITE);
	draw_box2(0, 0, 78, 22);
	StartMenu();

	system("mode con cols=88 lines=24");
	cls(BLACK, WHITE);
	textcolor(WHITE, BLACK);
	draw_box2(0, 0, 78, 22);
	putstar(oldx, oldy, STAR);
	ch = 0;
	keep_moving = 0;
	while (1) {
		if (_kbhit() == 1) {
			ch = _getch();
			if (ch == ESC) {
				break;
			}
			if (ch == SPECIAL1 || ch == SPECIAL2) {
				ch = _getch();
				switch (ch) {
				case UP:
				case DOWN:
				case LEFT:
				case RIGHT:
					keep_moving = 1;
					break;
				default:
					keep_moving = 0;
				}
			}
		}
		if (keep_moving) {
			switch (ch) {
			case UP:
				if (oldy > 1)
					newy = oldy - 1;
				break;
			case DOWN:
				if (oldy < HEIGHT - 3)
					newy = oldy + 1;
				break;
			case LEFT:
				if (oldx > 2)
					newx = oldx - 2;
				break;
			case RIGHT:
				if (oldx < WIDTH - 4)
					newx = oldx + 2;
				break;
			}
			erasestar(oldx, oldy);
			putstar(newx, newy, STAR1);
			oldx = newx; //마지막 위치 기억
			oldy = newy;
			keep_moving = 0; //한 번에 한칸씩만 이동할 예정
		}
		Sleep(Delay);
	}

	textcolor(BLACK, WHITE);
	gotoxy(21, 26); //끝나는 화면 위치

}
