#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <conio.h>
#include <Windows.h>
#include <time.h>

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
#define LASER '#' //*�� ��ü�� ����
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
int Delay = 100; // 100 msec delay, �� ���� ���̸� �ӵ��� ��������.
int keep_moving = 1; // 1:����̵�, 0:��ĭ���̵�.
//int time_out = 30; // ���ѽð�
int xLaser[WIDTH][HEIGHT] = { 0 };
int yLaser[WIDTH][HEIGHT] = { 0 };
int xLaser_count = 0;
int yLaser_count = 0;
int laserinterval = 4;
int score[100] = { 0 };
int user_score[1] = { 0 };
int item[WIDTH][HEIGHT] = { 0 }; // 1�̸� Gold �ִٴ� ��
int iteminterval = 30; // GOLD ǥ�� ����
int called[2];
int frame_count = 0;
int Laser_frame_sync = 10000;
int p_frame_sync = 10;
int p_frame_sync_start = 0;

//������ ����ü
struct sLaser {
	int set;
	int x;
	int y;
};

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

void player1(unsigned char ch)
{
	static int oldx = 10, oldy = 10, newx = 10, newy = 10;
	int move_flag = 0;
	static unsigned char last_ch = 0;

	if (called[0] == 0) { // ó�� �Ǵ� Restart
		oldx = 10, oldy = 10, newx = 10, newy = 10;
		putstar(oldx, oldy, STAR1);
		called[0] = 1;
		last_ch = 0;
		ch = 0;
	}
	// ���� �������� key �� ������ �����Ѵ�.
	if (last_ch == ch && frame_count % p_frame_sync != 0)
		return;
	if (keep_moving && ch == 0)
		ch = last_ch;
	last_ch = ch;

	switch (ch) {
	case UP:
		if (oldy > 1) // 0 �� Status Line
			newy = oldy - 1;
		else { // ���� �ε�ġ�� ������ �ݴ�� �̵�
			newy = oldy + 1;
			last_ch = DOWN;
		}
		move_flag = 1;
		break;
	case DOWN:
		if (oldy < HEIGHT - 3)
			newy = oldy + 1;
		else {
			newy = oldy - 1;
			last_ch = UP;
		}
		move_flag = 1;
		break;
	case LEFT:
		if (oldx > 2)
			newx = oldx - 2;
		else {
			newx = oldx + 2;
			last_ch = RIGHT;
		}
		move_flag = 1;
		break;
	case RIGHT:
		if (oldx < WIDTH - 4)
			newx = oldx + 2;
		else {
			newx = oldx - 2;
			last_ch = LEFT;
		}
		move_flag = 1;
		break;
	}
	if (move_flag) {
		erasestar(oldx, oldy); // ������ ��ġ�� * �� �����
		putstar(newx, newy, STAR1); // ���ο� ��ġ���� * �� ǥ���Ѵ�.
		oldx = newx; // ������ ��ġ�� ����Ѵ�.
		oldy = newy;
		move_flag = 1;
	}
}

void StartMenu() {
	//�ʱ�ȭ��
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
	//�ʱ⼳��
	int x, y;
	srand(time(NULL));
	user_score[0] = 0;
	for (x = 0; x < WIDTH; x++) {
		for (y = 0; y < HEIGHT; y++) {
			item[x][y] == 0;
		}
	}
	keep_moving = 1;
	xLaser_count = 0;
	yLaser_count = 0;
	iteminterval = 30;
	score[0] = 0;
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
		printf("- �����Ϸ��� �ƹ� Ű�� �Է��Ͻʽÿ�... -");
		Sleep(300);
		if (_kbhit()) {
			system("mode con cols=88 lines=25");
			cls(BLACK, WHITE);
			textcolor(WHITE, BLACK);
			draw_box2(0, 0, 78, 22);
			break;
		}
	}
}

void senseLaser(int Lx, int Ly, int set) {
	int i;
	if (set > 7) {
		for (i = 1; i < HEIGHT - 2; i++) {
			gotoxy(Lx, i);
			printf("*");
			gotoxy(Lx + 1, i);
			printf("*");
		}
	}
	else {
		for (i = 2; i < WIDTH - 2; i++) {
			gotoxy(i, Ly);
			printf("*");
			gotoxy(i, Ly + 1);
			printf("*");
		}
	}
}
void shootLaser(int Lx, int Ly, int set) {
	int i;
	if (set < 7) {
		for (i = 1; i < HEIGHT - 2; i++) {
			gotoxy(Lx, i);
			printf("#");
			gotoxy(Lx + 1, i);
			printf("#");
		}
		Sleep(50);
		for (i = 1; i < HEIGHT - 2; i++) {
			gotoxy(Lx, i);
			printf(" ");
			gotoxy(Lx + 1, i);
			printf(" ");
		}
	}
	else {
		for (i = 2; i < WIDTH - 2; i++) {
			gotoxy(i, Ly);
			printf("#");
			gotoxy(i, Ly + 1);
			printf("#");
		}
		Sleep(50);
		for (i = 2; i < WIDTH - 2; i++) {
			gotoxy(i, Ly);
			printf(" ");
			gotoxy(i, Ly + 1);
			printf(" ");
		}
	}
	//Lx = rand() % 76 + 2; //������ 2��
	//Ly = rand() % 20 + 1; //������ 2�� -> �ִ밪 1 ����
}

void Laser_start() {
	int x, y, set;
	set = rand() % 10;
	if (set < 7) {
		x = rand() % (WIDTH - 5) + 2;
		textcolor(WHITE, WHITE);
		gotoxy(x, 1);
		printf("*");
		xLaser[x][1] = 1;
		xLaser_count++;
		textcolor(WHITE, BLACK);
	}
	else {
		y = rand() % (HEIGHT - 4) + 1;
		textcolor(WHITE, WHITE);
		gotoxy(2, y);
		printf("*");
		yLaser[2][y] = 1;
		yLaser_count++;
		textcolor(WHITE, BLACK);
	}
}

void xLaser_shoot() {
	int x, y;

	if (xLaser_count == 0)
		return;
	for (x = 0; x < WIDTH; x++) {
		for (y = 0; y <= HEIGHT; y++) {
			if (xLaser[x][y] && xLaser[x][y + 1] != 1) {
				if (y > 3 && y < HEIGHT - 3) {
					gotoxy(x, y - 3);
					printf(BLANK);
					xLaser[x][y - 3] = 0;
					gotoxy(x, y + 1);
					textcolor(WHITE, WHITE);
					printf("*");
					textcolor(WHITE, BLACK);
					xLaser[x][y + 1] = 1;
					break;
				}

				else if (y <= 3) {
					gotoxy(x, y + 1);
					textcolor(WHITE, WHITE);
					printf("*");
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
		for (x = 0; x < WIDTH; x++) {
			if (yLaser[x][y] && yLaser[x + 1][y] != 1) {
				if (x > 5 && x < WIDTH - 5) {
					gotoxy(x - 4, y);
					printf(BLANK);
					yLaser[x - 4][y] = 0;
					gotoxy(x + 1, y);
					textcolor(WHITE, WHITE);
					printf("*");
					textcolor(WHITE, BLACK);
					yLaser[x + 1][y] = 1;
					break;
				}
				else if (x <= 5) {
					gotoxy(x + 1, y);
					textcolor(WHITE, WHITE);
					printf("*");
					textcolor(WHITE, BLACK);
					yLaser[x + 1][y] = 1;
					break;
				}
			}
			else if (x >= 4 && yLaser[x - 4][y] == 1) {
				gotoxy(x - 4, y);
				printf(BLANK);
				yLaser[x - 4][y] = 0;
				break;
			}
		}
	}
}

void show_time(int remain_time)
{
	gotoxy(31, 24);
	textcolor(WHITE, YELLOW1);
	printf("�ð� : %02d", remain_time);
	textcolor(WHITE, BLACK);
}

void main()
{
	unsigned char ch; // Ư��Ű 0xe0 �� �Է¹������� unsigned char �� �����ؾ� ��
	int oldx, oldy, newx, newy;

	int run_time, start_time, remain_time, last_remain_time;
	int laser_time = 0;
	int Lx, Ly, sh, lasercount = 0;

	newx = oldx = 10;
	newy = oldy = 10;

	//START:
	laser_time = 0;
	lasercount = 0;
	removeCursor(); // Ŀ���� �Ⱥ��̰� �Ѵ�
	system("mode con cols=80 lines=24");
	cls(BLACK, WHITE);
	draw_box2(0, 0, 78, 22);
	StartMenu(); //����ȭ��

	putstar(oldx, oldy, STAR);
	ch = 0;
	keep_moving = 1;
	//srand(time(NULL));
	start_time = time(NULL);
	remain_time = 0;
	show_time(remain_time);
	while (1) {
		run_time = time(NULL) - start_time;
		if (run_time > laser_time && (run_time % laserinterval == 0)) {
			Laser_start();
			laser_time = run_time;
		}

		show_time(run_time);
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
					if (frame_count % p_frame_sync == 0)
						player1(0);
				}
			}
		}
		else {
			if (frame_count % p_frame_sync == 0)
				player1(0);
		}
		if (frame_count % Laser_frame_sync == 0) {
			xLaser_shoot();
			yLaser_shoot();
		}
		Sleep(Delay);
		frame_count++;
	}

	textcolor(BLACK, WHITE);
	gotoxy(21, 26); //������ ȭ�� ��ġ

}