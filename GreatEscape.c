/////////////////////////////////////////////////////////////
// 과제명: 대탈출 (EXTI)
// 과제개요: ROOM 마다 비밀번호(1자리 숫자)를 눌러 탈출
// 사용한 하드웨어(기능): GPIO, EXTI,GLCD ...
// 제출일: 2022. 06. 02
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

#define RGB_PINK GET_RGB(255, 0, 255)              //RGB 핑크색

void _GPIO_Init(void);

uint16_t KEY_Scan(void);

void BEEP(void);
//BEEP 500MS 지속 되는 소리

void DisplayInitScreen(void);
void DelayMS(unsigned short wMS);
void DelayUS(unsigned short wUS);
void _EXTI_Init(void);


uint8_t X = 0, Y = 0;

uint8_t start_flag = 0, end_flag = 0;               //시작 flag와 끝flag 이용하여  오류해결


int main(void)
{
    _GPIO_Init();       // GPIO (LED, SW, Buzzer) 초기화
    LCD_Init();      // LCD 모듈 초기화
    DelayMS(100);
    _EXTI_Init();   // EXTI 초기화
    GPIOG->ODR = 0xFF00;

    DisplayInitScreen();
    while (1)
    {
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

void DisplayInitScreen(void)    //초기
{
    LCD_Clear(RGB_WHITE);      // 하안색 화면 클리어
    LCD_SetFont(&Gulim8);      // 폰트 : 8
    LCD_SetBackColor(RGB_WHITE);   // 글자배경색 : Yellow
    LCD_SetTextColor(RGB_BLUE);   // 글자색 : Black
    LCD_DisplayText(0, 0, "2017132030 LHJ");   // Title=
    LCD_SetTextColor(RGB_BLACK);       //검정색 
    LCD_DisplayText(3, 0, "R 0 1 3 4 5 6 7");   // Title=
    LCD_SetPenColor(RGB_BLACK); //팬색:BLACK 

    for (int X = 0; X < 7; X++)
    {
        LCD_DrawRectangle(10 + X * 15, 70, 6, 6);
        LCD_SetBrushColor(RGB_WHITE);  //MOUSE COLOR :pink      
        LCD_DrawFillRect(11 + X * 15, 71, 5, 5);// MOUSE 초기 위치     //점위치 정할것     
    }

    LCD_DisplayChar(5, 16, 'S');       //S:    시작
    //LCD_DisplayChar(5, 16, 'W');       //W:    입력중 //EXTI  발생시
    //LCD_DisplayChar(5, 16, 'C');       //C:    탈출 성공   //마지막꺼 누르면 EXTI 작동되고 C로 변환
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
void _EXTI_Init(void)
{
    RCC->AHB1ENR |= 0x00000080;   // RCC_AHB1ENR GPIOH Enable
    RCC->APB2ENR |= 0x00004000;   // Enable System Configuration Controller Clock

    GPIOH->MODER &= ~0xFFFF0000;   // GPIOH PIN8~PIN15 Input mode (reset state)

    SYSCFG->EXTICR[2] |= 0x7077;
    SYSCFG->EXTICR[3] |= 0x7777;   // EXTI8,9에 대한 소스 입력은 GPIOH로 설정
    // EXTI8 <- PH8, EXTI9 <- PH9 
    // EXTICR3(EXTICR[2])를 이용 
    // reset value: 0x0000   

    EXTI->FTSR |= 0xFB00;      // EXTI8,9,11,12,13,14,15: Falling Trigger Enable  
    EXTI->IMR |= 0x0100;      // EXTI8인터럽트 mask (Interrupt Enable) 설정

    NVIC->ISER[0] |= (1 << 23);   // Enable 'Global Interrupt EXTI8,9'
    NVIC->ISER[1] |= (1 << 8);
    //NVIC 강의
    //16진수로 쓰게 되면   0x00800000;
    // Vector table Position 참조
}
void EXTI9_5_IRQHandler(void)// EXTI8-9까지 Interrupt Pending(발생) 여부?
{
    if (EXTI->PR & 0x0100) //0x0100 == 0x0100해도되고 0x0100 != 0x0000         // EXTI8 Interrupt Pending(발생) 여부?
    {
        EXTI->PR |= 0x0100;   //펜딩키트를 지워야하는데 1로 지운다   // Pending bit Clear (clear를 안하면 인터럽트 수행후 다시 인터럽트 발생)
        GPIOG->ODR |= 0x0001;
        X = 0;
        LCD_DisplayChar(5, 16, 'W');       //W:    입력중 //EXTI  발생시
        LCD_SetBrushColor(RGB_PINK);  // COLOR :pink       만들것     만들것 만들어 
        LCD_DrawFillRect(11 + X * 15, 71, 5, 5);//  초기 위치     //점위치 정할것     //그
        EXTI->IMR &= ~0x0100;
        EXTI->IMR |= 0x0200;
        BEEP();
    }
    //EXTI Example(2) : SW1가 Low에서 High가 될 때(Rising edge Trigger mode) EXTI9 인터럽트 발생, LED1 toggle
    else if (EXTI->PR & 0x0200)
    {
        EXTI->PR |= 0x0200;      // Pending bit Clear (clear를 안하면 인터럽트 수행후 다시 인터럽트 발생)
        LCD_DisplayChar(5, 16, 'W');       //W:    입력중 //EXTI  발생시
        GPIOG->ODR |= 0x0002;
        X = 1;
        LCD_SetBrushColor(RGB_PINK);  // COLOR :pink       만들것     만들것 만들어 
        LCD_DrawFillRect(11 + X * 15, 71, 5, 5);//  초기 위치     //점위치 정할것     //그림그 위치 잡을것
        EXTI->IMR &= ~0x0200;
        EXTI->IMR |= 0x0800;
        BEEP();
    }
}
void EXTI15_10_IRQHandler(void) {
    if (EXTI->PR & 0x0800)      // EXTI11-15까지 Interrupt Pending(발생) 여부?
    {
        EXTI->PR |= 0x0800;      // Pending bit Clear (clear를 안하면 인터럽트 수행후 다시 인터럽트 발생)
        LCD_DisplayChar(5, 16, 'W');       //W:    입력중 //EXTI  발생시
        GPIOG->ODR |= 0x0008;
        X = 2;
        LCD_SetBrushColor(RGB_PINK);  // COLOR :pink       
        LCD_DrawFillRect(11 + X * 15, 71, 5, 5);//  초기 위치     //점위치 정할것     
        EXTI->IMR &= ~0x0800;
        EXTI->IMR |= 0x1000;
        BEEP();
    }
    else if (EXTI->PR & 0x1000)
    {
        // Pending bit Clear (clear를 안하면 인터럽트 수행후 다시 인터럽트 발생)
        EXTI->PR |= 0x1000;
        LCD_DisplayChar(5, 16, 'W');       //W:    입력중 //EXTI  발생시
        GPIOG->ODR |= 0x0010;
        X = 3;
        LCD_SetBrushColor(RGB_PINK);  // COLOR :pink       
        LCD_DrawFillRect(11 + X * 15, 71, 5, 5);//  초기 위치     //점위치 정할것     
        EXTI->IMR &= ~0x1000;
        EXTI->IMR |= 0x2000;
        BEEP();
    }
    else if (EXTI->PR & 0x2000)
    {
        // Pending bit Clear (clear를 안하면 인터럽트 수행후 다시 인터럽트 발생)
        EXTI->PR |= 0x2000;
        LCD_DisplayChar(5, 16, 'W');       //W:    입력중 //EXTI  발생시
        GPIOG->ODR |= 0x0020;
        X = 4;
        LCD_SetBrushColor(RGB_PINK);  // COLOR :pink       
        LCD_DrawFillRect(11 + X * 15, 71, 5, 5);//  초기 위치     //점위치      
        EXTI->IMR &= ~0x2000;
        EXTI->IMR |= 0x4000;
        BEEP();
    }
    else if (EXTI->PR & 0x4000)
    {
        EXTI->PR |= 0x4000;      // Pending bit Clear (clear를 안하면 인터럽트 수행후 다시 인터럽트 발생)
        LCD_DisplayChar(5, 16, 'W');       //W:    입력중 //EXTI  발생시
        GPIOG->ODR |= 0x0040;
        X = 5;
        LCD_SetBrushColor(RGB_PINK);  // COLOR :pink      
        LCD_DrawFillRect(11 + X * 15, 71, 5, 5);//  초기 위치     
        EXTI->IMR &= 0x4000;
        EXTI->IMR |= 0x8000;
        BEEP();
    }
    else if (EXTI->PR & 0x8000)
    {
        EXTI->PR |= 0x8000;      // Pending bit Clear (clear를 안하면 인터럽트 수행후 다시 인터럽트 발생)
        LCD_DisplayChar(5, 16, 'C');       //C:    탈출성공 
        GPIOG->ODR |= 0x0080;
        X = 6;
        LCD_SetBrushColor(RGB_PINK);  //MOUSE COLOR :pink       만들것     만들것 만들어 
        LCD_DrawFillRect(11 + X * 15, 71, 5, 5);// MOUSE 초기 위치     //점위치 정할것     //그림그 위치 잡을것
        EXTI->IMR &= 0x8000;
        EXTI->IMR |= 0x0100;
        BEEP();
        DelayMS(1000);
        BEEP();
        DelayMS(500);
        BEEP();
        DelayMS(500);
        BEEP();
        DelayMS(3000);
        BEEP();
        GPIOG->ODR = 0xFF00;	// LED 초기값: LED0~7 Off
        DisplayInitScreen();
        EXTI->IMR &= ~0xFA00;        //EXTI9,11,12,13,14,15  인터럽트 disable설정
        EXTI->IMR |= 0x0100;      // EXTI8인터럽트 mask (Interrupt Enable) 설정

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
    for (i = 0; i < wMS; i++)
        DelayUS(1000);    // 1000us => 1ms
}

void DelayUS(unsigned short wUS)
{
    volatile int Dly = (int)wUS * 17;
    for (; Dly; Dly--);
}
