#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <windows.h> //x, y 좌표로 글자를 출력하게 해주는 gotoxy 함수를 만들 때 사용
#include <conio.h> //엔터(Enter) 없이 실시간으로 키보드 입력을 받기 위해서 사용, 게임 중간에 엔터 안누르고 바로 키 입력이 가능하도록

#define NUM 6 // 어항의 수
#define MAX_WATER 100 // 어항의 최대 물 높이
#define WATER_AMOUNT 3 // 물주기 시 어항에 추가되는 물 높이

// 어항 구조체 정의 (fishSize 멤버를 추가함)
typedef struct { // struct FishTank라는 변수 상자 안에는 water와 isAlive가 세트로 들어있음 //원하는 형태의 자료형
	int water;         // 물 높이
	int isAlive;       // 물고기 생존 여부
	int fishSize;      // 물고기 크기 : 레벨이 증가, 시간이 지나가며 커짐 (추가 사항)
} FishTank;
FishTank fishTanks[NUM];   // 어항 배열

int level;
long prevElapsedTime;       // 직전에 물준 시간
long totalElapsedTime;      // long으로 많은 자료형 : 총 경과 시간
long startTime = 0;         // 게임 시작 시간 등 저장
FishTank* cursor;           // 현재 물을 줄 어항을 지시하는 포인터
int position = 0;           // 선택된 어항 번호, 0,1,2,..NUM-1

// 함수 프로토타입 선언 : 사전 예고의 역할을 함
void initData();
void update();
void render();
void decreaseWater(FishTank* tanks, int size, long elapsedTime); // 구조체 포인터를 사용하는 함수로 수정
void growFish(FishTank* tanks, int size, long elapsedTime);     // 물고기 성장 처리 함수 추가
int checkFishAlive();
int checkWin();
void gotoxy(int x, int y); //x, y 좌표로 글자를 출력하게 해주는 함수
int nonBlockingGetch(); //게임이 안 멈추게 함

// 입력 처리 관련 함수
void processInput();
void moveCursorLeft();
void moveCursorRight();
void giveWater();

int main(void) {
	int gameOver = 0;
	initData();             // 게임 초기화 함수 호출
	startTime = clock();    // 프로그램 실행 이후 현재까지 흐른 시간 저장
	cursor = fishTanks;     // 물을 줄 어항을 표시하는 커서
	system("cls");          // 화면 초기화

	while (!gameOver) {
		position = cursor - fishTanks;  // 현재 커서가 가리키는 어항 번호 계산

		// 키 입력 처리
		processInput();

		// 게임 데이터 업데이트
		update();
		position = cursor - fishTanks;

		// 게임 화면 출력
		render();

		// 레벨업 체크, 난이도 조정
		if (totalElapsedTime / 20 > level - 1) {
			level++;        // 레벨업
			gotoxy(10, 3);
			printf("Congratulations! Level Up!          ");
		}

		// 게임 승패 체크
		if (checkWin()) {
			gameOver = 1;
		}
		if (checkFishAlive() == 0) {
			gotoxy(2, 3);
			printf("All fish have died. Game Over.            ");
			gameOver = 1;
		}
		prevElapsedTime = totalElapsedTime;
		Sleep(100);     // 화면 깜빡임과 CPU 사용량을 줄이기 위한 짧은 대기
	}

	getchar();          // 게임 화면 유지 (대기)
	system("cls"); // 게임 종료 후 화면 초기화
	return 0;
}

// 게임 초기화하는 함수
void initData() {
	level = 1;
	prevElapsedTime = 0;
	totalElapsedTime = 0;

	for (int i = 0; i < NUM; i++) {
		fishTanks[i].isAlive = 1;   // 물고기 생존 여부 초기화
		fishTanks[i].water = 100;   // 물 높이 초기화
		fishTanks[i].fishSize = 1;  // 물고기 초기 크기 설정
	}
}

// 입력 처리 함수
void processInput() {
	int input; //누른 키 저장상자
	input = nonBlockingGetch();     // 키입력이 없으면 -1을 리턴
	if (input == -1) {
		return;                     // 입력이 없으면 아무 일도 하지 않고 종료
	}
	switch (input) {
	case 'j':                       // 왼쪽 이동
		moveCursorLeft();
		break;
	case 'l':                       // 오른쪽 이동
		moveCursorRight();
		break;
	case 'k':                       // 물주기
		giveWater();
		break;
	default:
		gotoxy(2, 13);
		printf("Invalid Input!                 ");
		break;
	}
}

// 커서를 왼쪽으로 이동
void moveCursorLeft() {
	if (cursor > fishTanks) {
		cursor--;
	}
}

// 커서를 오른쪽으로 이동
void moveCursorRight() {
	if (cursor < fishTanks + NUM - 1) {
		cursor++;
	}
}

// 현재 커서가 가리키는 어항에 물주기
void giveWater() {
	if (cursor->isAlive) { // 멤버 선택 연산자 -> : 물고기가 살아있을 때만 물주기 가능
		cursor->water += WATER_AMOUNT; //커서가 가리키는 어항에 물주기
		if (cursor->water > MAX_WATER) { //커서가 가리키는 어항의 물이 최대치를 넘지 않도록 조정
			cursor->water = MAX_WATER; //커서가 가리키는 어항의 물이 최대치를 넘지 않도록 조정
		}
	}
}

// 데이터를 갱신하는 함수
void update() {
	// 총 경과 시간, 초 단위로 표시
	totalElapsedTime = (clock() - startTime) / CLOCKS_PER_SEC;

	// 최근 경과 시간, 초 단위
	long diffTime = totalElapsedTime - prevElapsedTime;

	if (diffTime > 0) {
		// 구조체 포인터(fishTanks)를 전달하여 물 감소 및 물고기 성장 처리
		decreaseWater(fishTanks, NUM, diffTime);
		growFish(fishTanks, NUM, diffTime);
	}
}

// 화면 출력 함수
void render() {
	// 게임 이름 표시
	gotoxy(2, 2); //gotoxy로 위치 지정
	printf("@ Virtual Fish Tank Project @");

	// 게임 레벨 표시
	gotoxy(2, 3);
	printf("Level:%2d", level);

	// 총 경과 시간 표시
	gotoxy(2, 4);
	printf("Total Time: %ld sec", totalElapsedTime);

	// 어항 번호 출력
	gotoxy(2, 6);
	printf("[Tank No] ");
	for (int i = 0; i < NUM; i++) {
		printf(" %3d", i);
	}

	// 어항 물높이 출력
	gotoxy(2, 7);
	printf("[Water  ] ");
	for (int i = 0; i < NUM; i++) {
		if (fishTanks[i].isAlive) {
			printf(" %3d", fishTanks[i].water);
		}
		else {
			printf(" END"); // 물고기가 죽은 경우 표시
		}
	}

	// 물고기 크기 출력 (추가 사항 : 현재 물고기 상태)
	gotoxy(2, 8);
	printf("[FishSz ] ");
	for (int i = 0; i < NUM; i++) {
		if (fishTanks[i].isAlive) {
			printf(" %3d", fishTanks[i].fishSize);
		}
		else {
			printf("   X"); // 죽은 물고기는 X로 표시
		}
	}

	// 현재 커서의 위치 표시 (출력 라인 조정)
	gotoxy(2, 9);
	printf("           ");
	for (int i = 0; i < position; i++) {
		printf("    ");
	}
	printf("*");

	// 키 입력 방법 고지
	gotoxy(2, 11);
	printf("Left(j)  Water(k)  Right(l)");
}

// 구조체 포인터를 사용하여 어항의 물을 증발시키는 함수 (추가사항)
void decreaseWater(FishTank* tanks, int size, long elapsedTime) { //세 가지 매개변수 받고 시작
	for (int i = 0; i < size; i++) { //모든 어항 검사
		if ((tanks + i)->isAlive) {
			// 기본 감쇠량(level * 3)에 물고기 크기(fishSize)에 비례한 가중치를 추가하여 물 소비량 증가
			int waterConsumption = (level * 2 + (tanks + i)->fishSize) * (int)elapsedTime;
			(tanks + i)->water -= waterConsumption; //어항 속 물고기 커짐에 따라 물 소비량 증가

			if ((tanks + i)->water <= 0) {
				(tanks + i)->water = 0;
				(tanks + i)->isAlive = 0; // 물이 다 마르면 물고기가 죽음
			}
		}
	}
}

// 구조체 포인터를 사용하여 물고기를 성장시키는 함수 (추가사항)
void growFish(FishTank* tanks, int size, long elapsedTime) {
	for (int i = 0; i < size; i++) {
		// 물고기가 살아있고 어항에 물이 절반 이상(50) 있을 때 정상 성장
		if ((tanks + i)->isAlive && (tanks + i)->water > 50) {
			// 시간 경과와 현재 레벨의 영향을 받아 크기가 증가
			(tanks + i)->fishSize += level * (int)elapsedTime;
		}
	}
}

// 게임 승리 여부를 체크하는 함수
int checkWin() {
	if (level == 5) {
		gotoxy(2, 3);
		printf("Max Level reached! You Win! Game Over.      ");
		return 1;
	}
	return 0;
}

// 물고기가 모두 죽었는지를 확인하는 함수
int checkFishAlive() {
	for (int i = 0; i < NUM; i++) {
		if (fishTanks[i].isAlive > 0) {
			return 1;
		}
	}
	return 0;
}

// 콘솔창 출력 위치 지정
void gotoxy(int x, int y)
{
	COORD CursorPosition = { x, y }; //x, y 저장
	SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), CursorPosition); //get...: 검은색 콘솔 화면 창만 건들인다는 의미
} //set..: 윈도우가 제공하는 실제 기능 함수로, "지정한 콘솔 창의 커서 위치를 내가 원하는 좌표로 순간 이동시켜라

// Non-blocking getch() 구현
int nonBlockingGetch() {
	if (_kbhit()) { //키보드 입력 되었는 지 확인하는 함수, 입력이 있으면 1, 없으면 0 반환
		return _getch();  // 키가 입력되었으면 입력된 키를 반환
	}
	return -1;            // 키가 입력되지 않았다면 -1을 반환
}