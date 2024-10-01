/////////////////////////////////////////////////////////////
// 과제명: Maze Runner
// 과제개요: 시작위치(0,0)에 있는 ‘Mouse’(파란 사각형)를 조이스틱을 이용하여 목표위치(4,4)에 이동시키는 게임
// 사용한 하드웨어(기능): GPIO, Joy-stick, ...
// 제출일: 2022. 5. 26
// 제출자 클래스: 목요일반
// 학번: 2017132030
// 이름: 임현종
///////////////////////////////////////////////////////////////
#include "stm32f4xx.h"
#include "GLCD.h"

#define SW0_PUSH        0xFE00  //PH8
#define SW1_PUSH        0xFD00  //PH9
#define SW2_PUSH        0xFB00  //PH10
#define SW3_PUSH        0xF700  //PH11
#define SW4_PUSH        0xEF00  //PH12
#define SW5_PUSH        0xDF00  //PH13
#define SW6_PUSH        0xBF00  //PH14
#define SW7_PUSH        0x7F00  //PH15

#define NAVI_PUSH      0x03C0  //PI5 0000 0011 1100 0000 
#define NAVI_UP         0x03A0  //PI6 0000 0011 1010 0000 
#define NAVI_DOWN   0x0360  //PI7 0000 0011 0110 0000 
#define NAVI_RIGHT   0x02E0  //PI8 0000 0010 1110 0000 
#define NAVI_LEFT      0x01E0  //PI9 0000 0001 1110 0000 

void _GPIO_Init(void);
uint16_t JOY_Scan(void);
uint16_t KEY_Scan(void);

void BEEP(void);
void DisplayInitScreen(void);
void DelayMS(unsigned short wMS);
void DelayUS(unsigned short wUS);


void Draw_Maze(void);                               //4*4판 만들기
void CntUP(uint16_t, uint16_t, uint16_t, uint16_t); //횟수 셀 함수
void Status(uint16_t, uint16_t);                    //현재 위치 표시
void erasemouse(uint16_t, uint16_t);                //마우스 포인터 지우기
void mousemove(uint16_t, uint16_t);                 //마우스 움직이기


uint8_t X = 0, Y = 0;
uint8_t Ucnt = 0, Dcnt = 0, Lcnt = 0, Rcnt = 0;     //up,down,left,right 카운트
uint8_t start_flag = 0, end_flag = 0;               //시작 flag와 끝flag 이용하여  오류해결


int main(void)
{
	_GPIO_Init();       // GPIO (LED, SW, Buzzer) 초기화
	LCD_Init();      // LCD 모듈 초기화
	DelayMS(100);
	GPIOG->ODR = 0xFF00;
	GPIOG->ODR |= 0x0001;
	DisplayInitScreen();
	Draw_Maze();
	mousemove(X, Y);
	LCD_SetTextColor(RGB_RED);                        
	LCD_DisplayChar(6, 22, 'X');
	LCD_DisplayChar(6, 24, 'Y');

	while (1)
	{
		switch (KEY_Scan())
		{
		case SW0_PUSH:
			start_flag = 0;
			end_flag = 0;                                       //end_flag를 추가하여 시퀀스가 돌고 나서  모든 동작을 초기화시킴
			X = 0, Y = 0;
			Ucnt = Dcnt = Rcnt = Lcnt = 0;
			GPIOG->ODR &= 0xFF00;   // 초기값: LED0~7 Off
			GPIOG->BSRRL = 0x0001;
			DisplayInitScreen();   // LCD 초기화면
			LCD_SetTextColor(RGB_RED);
			LCD_DisplayChar(6, 22, 'X');
			LCD_DisplayChar(6, 24, 'Y');
			mousemove(X, Y);
			break;

		case SW7_PUSH:
			if (start_flag == 0 && end_flag == 0)               //05.26 end_flag가 없었기에 start_flag가 여길 한번 더 작동해서이슈 해결
			{
				BEEP();
				GPIOG->BSRRL = 0x0001;
				GPIOG->BSRRL = 0x0080;
				start_flag = 1;
				LCD_DisplayChar(6, 22, '0');
				LCD_DisplayChar(6, 24, '0');
			}
			break;
		}
		if (start_flag == 1)
		{
			GPIOG->ODR |= 0x0001;
			GPIOG->ODR |= 0x0080;
			switch (JOY_Scan())
			{
			case NAVI_LEFT:
				if (X == 0)
				{
					BEEP();
					DelayMS(500);
					BEEP();
					break;
				}
				else {
					erasemouse(X, Y);
					Draw_Maze();
					X--;
					Lcnt++;
					if (Lcnt >= 10)
					{
						Lcnt = 0;
					}
					GPIOG->ODR |= 0x0008;     // LED 3 ON
					GPIOG->ODR &= ~0x0016;    // LED 1,2,4 OFF 
					BEEP();
					mousemove(X, Y);
					CntUP(Ucnt, Dcnt, Lcnt, Rcnt);
					Status(X, Y);
				}
				break;

			case NAVI_RIGHT:
				if (X == 4)
				{
					BEEP();
					DelayMS(500);
					BEEP();
					break;
				}
				else {
					erasemouse(X, Y);
					Draw_Maze();
					X++;
					Rcnt++;
					GPIOG->ODR |= 0x0010;     // LED 4 ON
					GPIOG->ODR &= ~0x000E;    // LED 1,2,3 OFF 
					if (Rcnt >= 10)
					{
						Rcnt = 0;
					}
					BEEP();
					mousemove(X, Y);
					CntUP(Ucnt, Dcnt, Lcnt, Rcnt);
					Status(X, Y);

				}
				break;
			case NAVI_UP:
				if (Y == 0)
				{
					BEEP();
					DelayMS(500);
					BEEP();
					break;
				}

				else {
					erasemouse(X, Y);
					Draw_Maze();
					Y--;
					Ucnt++;
					GPIOG->ODR |= 0x0002;     // LED 1 ON
					GPIOG->ODR &= ~0x001C;    // LED 2,3,4
					if (Ucnt >= 10)
					{
						Ucnt = 0;
					}
					BEEP();
					mousemove(X, Y);
					CntUP(Ucnt, Dcnt, Lcnt, Rcnt);
					Status(X, Y);
				}
				break;

			case NAVI_DOWN:
				if (Y == 4)
				{
					BEEP();
					DelayMS(500);
					BEEP();
					break;
				}
				else
				{
					erasemouse(X, Y);
					Draw_Maze();
					Y++;
					Dcnt++;
					GPIOG->ODR |= 0x0004;     // LED 2 ON
					GPIOG->ODR &= ~0x001A;    // LED 1,3,4 OFF
					if (Dcnt >= 10)
					{
						Dcnt = 0;
					}
					BEEP();
					mousemove(X, Y);
					CntUP(Ucnt, Dcnt, Lcnt, Rcnt);
					Status(X, Y);
				}
				break;
			}
			if (X == 4 && Y == 4)                   //(4.4) 도착시 부저 3번 울림.
			{
				DelayMS(500);
				BEEP();
				DelayMS(500);
				BEEP();
				DelayMS(500);
				BEEP();
				DelayMS(500);
				start_flag = 0;
				end_flag = 1;
			}
		}
	}
}

void _GPIO_Init(void)
{
	// LED (GPIO G) 설정
	RCC->AHB1ENR |= 0x00000040;   // RCC_AHB1ENR : GPIOG(bit#6) Enable                     
	GPIOG->MODER |= 0x00005555;   // GPIOG 0~7 : Output mode (0b01)                  
	GPIOG->OTYPER &= ~0x00FF;   // GPIOG 0~7 : Push-pull  (GP8~15:reset state)   
	GPIOG->OSPEEDR |= 0x00005555;   // GPIOG 0~7 : Output speed 25MHZ Medium speed 

									// SW (GPIOT H) 설정 
	RCC->AHB1ENR |= 0x00000080;   // RCC_AHB1ENR : GPIOH(bit#7) Enable                     
	GPIOH->MODER &= ~0xFFFF0000;   // GPIOH 8~15 : Input mode (reset state)            
	GPIOH->PUPDR &= ~0xFFFF0000;   // GPIOH 8~15 : Floating input (No Pull-up, pull-down) :reset state

								   // Buzzer (GPIO F) 설정 
	RCC->AHB1ENR |= 0x00000020; // RCC_AHB1ENR : GPIOF(bit#5) Enable                     
	GPIOF->MODER |= 0x00040000;   // GPIOF 9 : Output mode (0b01)                  
	GPIOF->OTYPER &= ~0x0200;   // GPIOF 9 : Push-pull     
	GPIOF->OSPEEDR |= 0x00040000;   // GPIOF 9 : Output speed 25MHZ Medium speed 


									//Joy Stick SW(PORT I) 설정 :  INPUT MODE  :
	RCC->AHB1ENR |= 0x00000100;   // RCC_AHB1ENR GPIOI Enable
	GPIOI->MODER &= ~0x000FFC00;   // GPIOI 5~9 : Input mode (reset state)
	GPIOI->PUPDR &= ~0x000FFC00;   // GPIOI 5~9 : Floating input (No Pull-up, pull-down) (reset state)

}

void DisplayInitScreen(void)
{
	LCD_Clear(RGB_WHITE);      // 하안색 화면 클리어
	LCD_SetFont(&Gulim7);      // 폰트 : 7
	LCD_SetBackColor(RGB_YELLOW);   // 글자배경색 : Yellow
	LCD_SetTextColor(RGB_BLUE);   // 글자색 : Blue
	LCD_DisplayText(0, 0, "Maze Runner");   // Title

	LCD_SetTextColor(RGB_BLACK);       //검정색 선 
	LCD_SetBackColor(RGB_WHITE);     //  빨간색 배경
	Draw_Maze();

	LCD_SetPenColor(RGB_BLACK); //팬색:BLACK 
	LCD_DisplayText(1, 1, "(0,0)");
	LCD_DisplayText(1, 13, "(4,0)");
	LCD_DisplayText(4, 14, "RESET (SW0)");
	LCD_DisplayText(5, 14, "START (SW7)");
	LCD_DisplayText(6, 14, "STATUS:(X,Y)");

	LCD_SetTextColor(RGB_BLACK);
	LCD_DisplayText(8, 1, "(0,4)");
	LCD_DisplayText(8, 13, "(4,4)");
	LCD_DisplayText(10, 2, "U:0,D:0,L:0,R:0");
	LCD_SetTextColor(RGB_RED);
	LCD_DisplayChar(10, 4, '0');
	LCD_DisplayChar(10, 8, '0');
	LCD_DisplayChar(10, 12, '0');
	LCD_DisplayChar(10, 16, '0');
}

void Draw_Maze(void)
{
	for (int i = 0; i<5; i++)
	{
		LCD_SetPenColor(RGB_BLACK);
		LCD_DrawHorLine(23, 23 + 15 * i, 61);
		LCD_DrawVerLine(23 + 15 * i, 23, 61);
	}
}

uint8_t joy_flag = 0;

uint16_t JOY_Scan(void)   // input joy stick NAVI_* 
{
	uint16_t key;
	key = GPIOI->IDR & 0x03E0;   // any key pressed ?
	if (key == 0x03E0)      // if no key, check key off
	{
		if (joy_flag == 0)
			return key;
		else
		{
			DelayMS(10);
			joy_flag = 0;
			return key;
		}
	}
	else            // if key input, check continuous key
	{
		if (joy_flag != 0)   // if continuous key, treat as no key input
			return 0X03E0;
		else         // if new key,delay for debounce
		{
			joy_flag = 1;
			DelayMS(10);
			return key;
		}
	}
}
uint8_t key_flag = 0;
uint16_t KEY_Scan(void)   // input key SW0 - SW7 
{
	uint16_t key;
	key = GPIOH->IDR & 0xFF00;   // any key pressed ?
	if (key == 0xFF00)      // if no key, check key off
	{
		if (key_flag == 0)
			return key;
		else
		{
			DelayMS(10);
			key_flag = 0;
			return key;
		}
	}
	else            // if key input, check continuous key
	{
		if (key_flag != 0)   // if continuous key, treat as no key input
			return 0xFF00;
		else         // if new key,delay for debounce
		{
			key_flag = 1;
			DelayMS(10);
			return key;
		}
	}
}

void BEEP(void)         /* beep for 30 ms */
{
	GPIOF->ODR |= 0x0200;   // PF9 'H' Buzzer on
	DelayMS(30);      // Delay 30 ms
	GPIOF->ODR &= ~0x0200;   // PF9 'L' Buzzer off
}

void DelayMS(unsigned short wMS)
{
	register unsigned short i;
	for (i = 0; i<wMS; i++)
		DelayUS(1000);    // 1000us => 1ms
}

void DelayUS(unsigned short wUS)
{
	volatile int Dly = (int)wUS * 17;
	for (; Dly; Dly--);
}

void CntUP(uint16_t u, uint16_t d, uint16_t l, uint16_t r)      //횟수 숫자만 올릴수 있게 카운트
{
	LCD_DisplayChar(10, 4, Ucnt + 0x30);
	LCD_DisplayChar(10, 8, Dcnt + 0x30);
	LCD_DisplayChar(10, 12, Lcnt + 0x30);
	LCD_DisplayChar(10, 16, Rcnt + 0x30);
}

void Status(uint16_t x, uint16_t y)                             //위치 상태 표시
{
	LCD_DisplayChar(6, 22, x + 0x30);
	LCD_DisplayChar(6, 24, y + 0x30);
}

void mousemove(uint16_t x, uint16_t y)                          //마우스 초기위치 
{
	LCD_SetBrushColor(RGB_BLUE);  //MOUSE COLOR :BLUE
	LCD_DrawFillRect(20 + X * 15, 20 + Y * 15, 6, 6);// MOUSE 초기 위치
}

void erasemouse(uint16_t x, uint16_t y)                         //마우스 기록 남기지않고 계속해서 지워줌
{
	LCD_SetBrushColor(RGB_WHITE);  //MOUSE COLOR :BLUE
	LCD_DrawFillRect(20 + X * 15, 20 + Y * 15, 6, 6);// MOUSE 초기 위치 
}