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
#define POTION "§"
#define SLOW "◎"
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
int Delay = 2; // 100 msec delay, 이 값을 줄이면 속도가 빨라진다.
int keep_moving = 0; // 1:계속이동, 0:한칸씩이동.
int player_pos[WIDTH][HEIGHT] = { 0 };
int xLaser[WIDTH][HEIGHT] = { 0 };
int yLaser[WIDTH][HEIGHT] = { 0 };
int xLaser_count = 0;
int yLaser_count = 0;
int score[20] = { 0 };
int user_score = 0;
int Heart = 3; //목숨 개수
int Pcolor_frame; //목숨이 깎였을 때 색깔 표시
int potion[WIDTH][HEIGHT] = { 0 };
int slow[WIDTH][HEIGHT] = { 0 }; // 1이면 item 있다는 뜻
int slow_count = 0;
int called[2];
int frame_count = 0;
int Laser_frame_sync = 10;
int Laser_create_frame_sync = 50; //생성시 프레임
int p_frame_sync = 10;
int p_frame_sync_start = 0;
int item_frame_sync = 300; // item 표시 간격 : 프레임단위
int special_pattern = 0;
int special_frame_sync = 1500;

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
//미리 선언
void slow_item();
void potion_item();
// box 그리기 함수, ch 문자열로 (x1,y1) ~ (x2,y2) box를 그린다.
// 한글 문자를 그리는 용도로 사용 "□" 로 벽을 그리는 경우
void draw_box(int x1, int y1, int x2, int y2)
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

void potion_set() {
	int x, y;
	do {
		x = rand() % (WIDTH - 5) + 2;
		y = rand() % (HEIGHT - 4) + 1;
	} while ((potion[x][y] == 1 || slow[x][y] == 1) || x % 2 != 0);
	gotoxy(x, y);
	printf(POTION);
	potion[x][y] = 1;
}

void slow_set() {
	int x, y;
	do {
		x = rand() % (WIDTH - 5) + 2;
		y = rand() % (HEIGHT - 4) + 1;
	} while ((potion[x][y] == 1 || slow[x][y] == 1) || x % 2 != 0);
	gotoxy(x, y);
	printf(SLOW);
	slow[x][y] = 1;
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
		if (potion[newx][newy] == 1) {
			pm_Heart(1); //포션을 먹으면 체력 1칸 회복
		}
		else if (slow[newx][newy] == 1) {
			slow_count += 1;
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
				if (y == 24)
					xLaser_count--;
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
				if (x == 82)
					yLaser_count--;
				break;
			}
		}
	}
}

void clean_map() {
	int x,y;
	for (y = 1; y < HEIGHT - 2; y++) {
		for (x = 2; x < WIDTH - 5; x += 2) {
			gotoxy(x, y);
			printf(BLANK);
		}
	}
	for (x = 0; x < WIDTH; x++) { //레이저 위치, 아이템 모두 초기화(특수 패턴 생성 위해)
		for (y = 0; y < HEIGHT; y++) {
			potion[x][y] = 0;
			slow[x][y] = 0;
			xLaser[x][y] = 0;
			yLaser[x][y] = 0;
			xLaser_count = 0;
			yLaser_count = 0;
		}
	}

}

void special_pattern_x() {
	int x;
	for (x = 2; x < WIDTH - 3; x += 4) {
		textcolor(YELLOW1, YELLOW1);
		gotoxy(x, 1);
		printf(LASER);
		xLaser[x][1] = 1;
		xLaser_count++;
		textcolor(WHITE, BLACK);
	}
}

void special_pattern_y() {
	int y;
	for (y = 2; y < HEIGHT - 2; y += 2) {
		textcolor(YELLOW1, YELLOW1);
		gotoxy(2, y);
		printf(LASER);
		yLaser[2][y] = 1;
		yLaser_count++;
		textcolor(WHITE, BLACK);
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
	int x, y, i = 2;
	srand(time(NULL));
	user_score = 0;
	for (x = 0; x < WIDTH; x++) {
		for (y = 0; y < HEIGHT; y++) {
			potion[x][y] = 0;
			slow[x][y] = 0;
			xLaser[x][y] = 0;
			yLaser[x][y] = 0;
			player_pos[x][y] = 0;
		}
	}
	player_pos[10][10] = 1;
	keep_moving = 1;
	Heart = 3;
	slow_count = 0;
	xLaser_count = 0;
	yLaser_count = 0;
	called[0] = called[1] = 0;
	frame_count = 0;
	p_frame_sync = 10;
	p_frame_sync_start = 0;
	Laser_frame_sync = 10;
	Laser_create_frame_sync = 50;
	item_frame_sync = 300;
	Pcolor_frame = 0;
	special_pattern = 0;
	special_frame_sync = 1500;

	while (1) {
		int c1, c2;
		textcolor(BLUE1, BLACK);
		if (i == 2) {
			gotoxy(65, i);
			printf("  ");
			gotoxy(63, i + 1);
			printf("      ");
			gotoxy(61, i + 2);
			printf("          ");
			gotoxy(63, i + 3);
			printf("      ");
			i++;
		}
		else {
			gotoxy(65, i);
			printf("  ");
			gotoxy(63, i + 1);
			printf("      ");
			gotoxy(61, i + 2);
			printf("          ");
			gotoxy(63, i + 3);
			printf("      ");
			i--;
		}
		gotoxy(65, i);
		printf("■");
		gotoxy(63, i + 1);
		printf("■##■");
		gotoxy(61, i + 2);
		printf("■■■■■");
		gotoxy(63, i + 3);
		printf("■  ■");
		do {
			c1 = rand() % 16;
			c2 = rand() % 16;
		} while (c1 == c2);
		textcolor(c1, BLACK);
		gotoxy(21, 11);
		printf("- 시작하려면 아무 키나 입력하십시오... -");

		Sleep(300);
		if (kbhit()) {
			system("mode con cols=100 lines=25");
			cls(BLACK, WHITE);
			textcolor(WHITE, BLACK);
			draw_box(0, 0, 78, 22);
			show_Heart();
			show_score();
			break;
		}
	}
}

void main()
{
	unsigned char ch; // 특수키 0xe0 을 입력받으려면 unsigned char 로 선언해야 함

	int run_time, start_time;
	int laser_time = 0, lasercount = 0;
	int laser_stack_count = 0, laser_stack = 0;
	int random_item = 0, random_pattern = 0;

	clock_t start = 0, now = 0, oldscore = 0, miliscore = 0;

START:
	laser_time = 0;
	lasercount = 0;
	laser_stack = 0;
	laser_stack_count = 0;
	removeCursor(); // 커서를 안보이게 한다
	system("mode con cols=80 lines=24");
	cls(BLACK, WHITE);
	draw_box(0, 0, 78, 22);
	StartMenu(); //시작화면

	srand(time(NULL));
	textcolor(WHITE, BLACK);
	putstar(10, 10, STAR1);
	gotoxy(30, 23);
	printf("난이도 : 1");
	ch = 0;
	keep_moving = 0;
	//시간측정
	start_time = time(NULL);
	//점수측정
	start = clock;
	while (1) {
		run_time = time(NULL) - start_time;
		//레이저 속도 결정(난이도 결정)
		if (slow_count > 0 && laser_stack > 0) { //난이도 한칸 낮춰 줌
			Laser_create_frame_sync += 10;
			Laser_frame_sync += 2;
			laser_stack--;
			slow_count--;
		}
		else if (slow_count > 0 && laser_stack == 0) {
			slow_count--;
		}
		else if (slow_count == 0 && run_time % 10 == 0 && Laser_frame_sync > 3 && run_time / 10 != laser_stack_count) { //cre= 50, fra= 10 시작
			Laser_create_frame_sync -= 10;
			Laser_frame_sync -= 2;
			laser_stack++;
			laser_stack_count++;
		}
		gotoxy(30, 23); //난이도 표시
		if (laser_stack + 1 == 5)
			printf("난이도 : master");
		else {
			printf("난이도 :       ");
			gotoxy(30, 23);
			printf("난이도 : %d", laser_stack + 1);
		}
		
		if (frame_count > 0 && frame_count % special_frame_sync == 0 && special_pattern == 0) { //맵 초기화 후 특수 패턴 시작
			special_pattern = 1;
			clean_map();
			random_pattern = rand() % 2;
			if (random_pattern == 0) {
				special_pattern_x();
			}
			else {
				special_pattern_y();
			}
		}

		if (frame_count % Laser_create_frame_sync == 0 && special_pattern == 0) { //레이저 방출(특수 패턴 없을 때만 생성)
			Laser_start();
		}
		if (frame_count % item_frame_sync == 0 && special_pattern == 0) { //아이템 생성(특수 패턴 없을 때만 생성)
			random_item = rand() % 2 + 1;
			if (random_item == 1)
				slow_set();
			else
				potion_set();
		}
		
		oldscore = miliscore; //점수
		now = clock();
		miliscore = (now - start) % CLOCKS_PER_SEC;
		if (miliscore > oldscore) {
			user_score++;
		}
		show_user_score(user_score);
		if (Heart <= 0) //게임 오버 조건
			break;

		if (kbhit() == 1) { //캐릭터 동작
			char c1;
			c1 = getch();
			if (c1 == '\0')
				continue;
			else
				ch = c1;
			if (ch == ESC) {
				exit(0);
			}
			if (ch == SPECIAL1) {
				ch = getch();
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

		if (special_pattern == 1 && xLaser_count == 0 && yLaser_count == 0) { //패턴 끝났으면 원상 복귀
			special_pattern = 0;
		}
		
		for (int x = 0; x < WIDTH; x++) { //아이템이 레이저에 닿으면 없어짐
			for (int y = 0; y < HEIGHT; y++) {
				if ((xLaser[x][y] == 1 && slow[x][y] == 1) || (yLaser[x][y] == 1 && slow[x][y] == 1)) {
					slow[x][y] = 0;
					gotoxy(x, y);
				}
				if ((xLaser[x][y] == 1 && potion[x][y] == 1) || (yLaser[x][y] == 1 && potion[x][y] == 1)) {
					potion[x][y] = 0;
					gotoxy(x, y);
				}
			}
		}
		for (int x = 0; x < WIDTH; x++) { //레이저한테 맞았을 경우
			for (int y = 0; y < HEIGHT; y++) {
				if (Pcolor_frame == 0 && ((xLaser[x][y] == 1 && player_pos[x][y] == 1) || (yLaser[x][y] == 1 && player_pos[x][y] == 1))) {
					pm_Heart(0);
					Pcolor_frame += 70;
					textcolor(BLUE1, BLACK);
					putstar(x, y, STAR1);
					textcolor(WHITE, BLACK);
					break;
				}
			}
		}
		for (int x = 0; x < WIDTH; x++) { //플레이어 화면에서 사라지지 않게 하기 위함
			for (int y = 0; y < HEIGHT; y++) {
				if (player_pos[x][y] == 1 && Pcolor_frame == 0) {
					textcolor(WHITE, BLACK);
					putstar(x, y, STAR1);
					break;
				}
				else if (player_pos[x][y] == 1 && Pcolor_frame != 0) {
					textcolor(BLUE1, BLACK);
					putstar(x, y, STAR1);
					textcolor(WHITE, BLACK);
					break;
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
	printf(" 당신의 점수는 : %10d ", user_score);
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