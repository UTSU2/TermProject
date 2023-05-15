#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <conio.h>
#include <Windows.h>

// ���� ����
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

//���� �� ĳ���� ����
#define STAR '*'
#define STAR1 "��" // player1 ǥ��
#define BLANK "  " // ' ' ���ϸ� ������ �������� 

#define ESC 0x1b //  ESC ������ ����

#define SPECIAL1 0xe0 // Ư��Ű�� 0xe0 + key ������ �����ȴ�.
#define SPECIAL2 0x00 // keypad ��� 0x00 + key �� �����ȴ�.

#define UP  0x48 // Up key�� 0xe0 + 0x48 �ΰ��� ���� ���´�.
#define DOWN 0x50
#define LEFT 0x4b
#define RIGHT 0x4d

#define WIDTH 80
#define HEIGHT 24

//��������
int Delay = 50; // 100 msec delay, �� ���� ���̸� �ӵ��� ��������.
//int keep_moving = 1; // 1:����̵�, 0:��ĭ���̵�.
//int time_out = 30; // ���ѽð�
int score[2] = { 0 };
int item[WIDTH][HEIGHT] = { 0 }; // 1�̸� Gold �ִٴ� ��
int iteminterval = 7; // GOLD ǥ�� ����
//int called[2];

void gotoxy(int x, int y) //���� ���ϴ� ��ġ�� Ŀ�� �̵�
{
	COORD pos; // Windows.h �� ����
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

void removeCursor(void) { // Ŀ���� �Ⱥ��̰� �Ѵ�
	CONSOLE_CURSOR_INFO curInfo;
	GetConsoleCursorInfo(GetStdHandle(STD_OUTPUT_HANDLE), &curInfo);
	curInfo.bVisible = 0;
	SetConsoleCursorInfo(GetStdHandle(STD_OUTPUT_HANDLE), &curInfo);
}
void showCursor(void) { // Ŀ���� ���̰� �Ѵ�
	CONSOLE_CURSOR_INFO curInfo;
	GetConsoleCursorInfo(GetStdHandle(STD_OUTPUT_HANDLE), &curInfo);
	curInfo.bVisible = 1;
	SetConsoleCursorInfo(GetStdHandle(STD_OUTPUT_HANDLE), &curInfo);
}
void cls(int bg_color, int text_color) // ȭ�� �����
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

// box �׸��� �Լ�, ch ���ڿ��� (x1,y1) ~ (x2,y2) box�� �׸���.
// �ѱ� ���ڸ� �׸��� �뵵�� ��� "��" �� ���� �׸��� ���
void draw_box2(int x1, int y1, int x2, int y2)
{
	int x, y;
	//int len = strlen(ch);
	for (x = x1; x <= x2; x += 2) { // �ѱ��� 2ĭ�� �����Ѵ�.
		gotoxy(x, y1);
		printf("%s", "��"); // �� ������ ����Ű ������ ����
		gotoxy(x, y2);
		printf("%s", "��");
	}
	for (y = y1; y <= y2; y++) {
		gotoxy(x1, y);
		printf("%s", "��");
		gotoxy(x2, y);
		printf("%s", "��");
	}
	gotoxy(x1, y1); printf("��");
	gotoxy(x1, y2); printf("��");
	gotoxy(x2, y1); printf("��");
	gotoxy(x2, y2); printf("��");
}
void draw_box2_fill(int x1, int y1, int x2, int y2, int color)
{
	int x, y;

	textcolor(WHITE, BLACK);
	for (x = x1; x <= x2; x += 2) { // �ѱ��� 2ĭ�� �����Ѵ�.
		gotoxy(x, y1);
		printf("%s", "��"); // �� ������ ����Ű ������ ����
		gotoxy(x, y2);
		printf("%s", "��");
	}
	for (y = y1; y <= y2; y++) {
		gotoxy(x1, y);
		printf("%s", "��");
		gotoxy(x2, y);
		printf("%s", "��");
	}
	gotoxy(x1, y1); printf("��");
	gotoxy(x1, y2); printf("��");
	gotoxy(x2, y1); printf("��");
	gotoxy(x2, y2); printf("��");
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
	printf("��       ����� ����� ����� �����\n");
	gotoxy(2, 2);
	printf("��       ��    �� ��       ��       ��    ��\n");
	gotoxy(2, 3);
	printf("��       ����� ����� ����� ����  \n");
	gotoxy(2, 4);
	printf("��       ��    ��       �� ��       ��    ��\n");
	gotoxy(2, 5);
	printf("����� ��    �� ����� ����� ��    ��\n");
	gotoxy(34, 17);
	printf("����     ���   ����   ����� �����\n");
	gotoxy(34, 18);
	printf("��    �� ��    �� ��    �� ��       ��      \n");
	gotoxy(34, 19);
	printf("��    �� ��    �� ��    �� ��  ��� �����\n");
	gotoxy(34, 20);
	printf("��    �� ��    �� ��    �� ��    �� ��      \n");
	gotoxy(34, 21);
	printf("����     ���   ����   ����� �����\n");

	textcolor(BLUE1, BLACK);
	gotoxy(65, 2);
	printf("��");
	gotoxy(63, 3);
	printf("��##��");
	gotoxy(61, 4);
	printf("������");
	gotoxy(63, 5);
	printf("��  ��");

	while (1) {
		int c1, c2;
		do {
			c1 = rand() % 16;
			c2 = rand() % 16;
		} while (c1 == c2);
		textcolor(c1, BLACK);
		gotoxy(21, 11);
		printf("- �����Ϸ��� �ƹ� Ű�� �Է��Ͻʽÿ�... -");
		Sleep(300);
		if (_kbhit()) {
			break;
		}
	}
}

void main()
{
	unsigned char ch; // Ư��Ű 0xe0 �� �Է¹������� unsigned char �� �����ؾ� ��
	int oldx, oldy, newx, newy;
	int keep_moving;

	newx = oldx = 10;
	newy = oldy = 10;

	char buf[100];
	removeCursor(); // Ŀ���� �Ⱥ��̰� �Ѵ�

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
			oldx = newx; //������ ��ġ ���
			oldy = newy;
			keep_moving = 0; //�� ���� ��ĭ���� �̵��� ����
		}
		Sleep(Delay);
	}

	textcolor(BLACK, WHITE);
	gotoxy(21, 26); //������ ȭ�� ��ġ

}
