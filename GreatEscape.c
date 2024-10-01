/////////////////////////////////////////////////////////////
// ������: ��Ż�� (EXTI)
// ��������: ROOM ���� ��й�ȣ(1�ڸ� ����)�� ���� Ż��
// ����� �ϵ����(���): GPIO, EXTI,GLCD ...
// ������: 2022. 06. 02
// ������ Ŭ����: ����Ϲ�
// �й�: 2017132030
// �̸�: ������
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

#define RGB_PINK GET_RGB(255, 0, 255)              //RGB ��ũ��

void _GPIO_Init(void);

uint16_t KEY_Scan(void);

void BEEP(void);
//BEEP 500MS ���� �Ǵ� �Ҹ�

void DisplayInitScreen(void);
void DelayMS(unsigned short wMS);
void DelayUS(unsigned short wUS);
void _EXTI_Init(void);


uint8_t X = 0, Y = 0;

uint8_t start_flag = 0, end_flag = 0;               //���� flag�� ��flag �̿��Ͽ�  �����ذ�


int main(void)
{
    _GPIO_Init();       // GPIO (LED, SW, Buzzer) �ʱ�ȭ
    LCD_Init();      // LCD ��� �ʱ�ȭ
    DelayMS(100);
    _EXTI_Init();   // EXTI �ʱ�ȭ
    GPIOG->ODR = 0xFF00;

    DisplayInitScreen();
    while (1)
    {
    }
}


void _GPIO_Init(void)
{
    // LED (GPIO G) ����
    RCC->AHB1ENR |= 0x00000040;   // RCC_AHB1ENR : GPIOG(bit#6) Enable                     
    GPIOG->MODER |= 0x00005555;   // GPIOG 0~7 : Output mode (0b01)                  
    GPIOG->OTYPER &= ~0x00FF;   // GPIOG 0~7 : Push-pull  (GP8~15:reset state)   
    GPIOG->OSPEEDR |= 0x00005555;   // GPIOG 0~7 : Output speed 25MHZ Medium speed 

    // SW (GPIOT H) ���� 
    RCC->AHB1ENR |= 0x00000080;   // RCC_AHB1ENR : GPIOH(bit#7) Enable                     
    GPIOH->MODER &= ~0xFFFF0000;   // GPIOH 8~15 : Input mode (reset state)            
    GPIOH->PUPDR &= ~0xFFFF0000;   // GPIOH 8~15 : Floating input (No Pull-up, pull-down) :reset state

    // Buzzer (GPIO F) ���� 
    RCC->AHB1ENR |= 0x00000020; // RCC_AHB1ENR : GPIOF(bit#5) Enable                     
    GPIOF->MODER |= 0x00040000;   // GPIOF 9 : Output mode (0b01)                  
    GPIOF->OTYPER &= ~0x0200;   // GPIOF 9 : Push-pull     
    GPIOF->OSPEEDR |= 0x00040000;   // GPIOF 9 : Output speed 25MHZ Medium speed 


    //Joy Stick SW(PORT I) ���� :  INPUT MODE  :
    RCC->AHB1ENR |= 0x00000100;   // RCC_AHB1ENR GPIOI Enable
    GPIOI->MODER &= ~0x000FFC00;   // GPIOI 5~9 : Input mode (reset state)
    GPIOI->PUPDR &= ~0x000FFC00;   // GPIOI 5~9 : Floating input (No Pull-up, pull-down) (reset state)

}

void DisplayInitScreen(void)    //�ʱ�
{
    LCD_Clear(RGB_WHITE);      // �ϾȻ� ȭ�� Ŭ����
    LCD_SetFont(&Gulim8);      // ��Ʈ : 8
    LCD_SetBackColor(RGB_WHITE);   // ���ڹ��� : Yellow
    LCD_SetTextColor(RGB_BLUE);   // ���ڻ� : Black
    LCD_DisplayText(0, 0, "2017132030 LHJ");   // Title=
    LCD_SetTextColor(RGB_BLACK);       //������ 
    LCD_DisplayText(3, 0, "R 0 1 3 4 5 6 7");   // Title=
    LCD_SetPenColor(RGB_BLACK); //�һ�:BLACK 

    for (int X = 0; X < 7; X++)
    {
        LCD_DrawRectangle(10 + X * 15, 70, 6, 6);
        LCD_SetBrushColor(RGB_WHITE);  //MOUSE COLOR :pink      
        LCD_DrawFillRect(11 + X * 15, 71, 5, 5);// MOUSE �ʱ� ��ġ     //����ġ ���Ұ�     
    }

    LCD_DisplayChar(5, 16, 'S');       //S:    ����
    //LCD_DisplayChar(5, 16, 'W');       //W:    �Է��� //EXTI  �߻���
    //LCD_DisplayChar(5, 16, 'C');       //C:    Ż�� ����   //�������� ������ EXTI �۵��ǰ� C�� ��ȯ
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
    SYSCFG->EXTICR[3] |= 0x7777;   // EXTI8,9�� ���� �ҽ� �Է��� GPIOH�� ����
    // EXTI8 <- PH8, EXTI9 <- PH9 
    // EXTICR3(EXTICR[2])�� �̿� 
    // reset value: 0x0000   

    EXTI->FTSR |= 0xFB00;      // EXTI8,9,11,12,13,14,15: Falling Trigger Enable  
    EXTI->IMR |= 0x0100;      // EXTI8���ͷ�Ʈ mask (Interrupt Enable) ����

    NVIC->ISER[0] |= (1 << 23);   // Enable 'Global Interrupt EXTI8,9'
    NVIC->ISER[1] |= (1 << 8);
    //NVIC ����
    //16������ ���� �Ǹ�   0x00800000;
    // Vector table Position ����
}
void EXTI9_5_IRQHandler(void)// EXTI8-9���� Interrupt Pending(�߻�) ����?
{
    if (EXTI->PR & 0x0100) //0x0100 == 0x0100�ص��ǰ� 0x0100 != 0x0000         // EXTI8 Interrupt Pending(�߻�) ����?
    {
        EXTI->PR |= 0x0100;   //���ŰƮ�� �������ϴµ� 1�� �����   // Pending bit Clear (clear�� ���ϸ� ���ͷ�Ʈ ������ �ٽ� ���ͷ�Ʈ �߻�)
        GPIOG->ODR |= 0x0001;
        X = 0;
        LCD_DisplayChar(5, 16, 'W');       //W:    �Է��� //EXTI  �߻���
        LCD_SetBrushColor(RGB_PINK);  // COLOR :pink       �����     ����� ����� 
        LCD_DrawFillRect(11 + X * 15, 71, 5, 5);//  �ʱ� ��ġ     //����ġ ���Ұ�     //��
        EXTI->IMR &= ~0x0100;
        EXTI->IMR |= 0x0200;
        BEEP();
    }
    //EXTI Example(2) : SW1�� Low���� High�� �� ��(Rising edge Trigger mode) EXTI9 ���ͷ�Ʈ �߻�, LED1 toggle
    else if (EXTI->PR & 0x0200)
    {
        EXTI->PR |= 0x0200;      // Pending bit Clear (clear�� ���ϸ� ���ͷ�Ʈ ������ �ٽ� ���ͷ�Ʈ �߻�)
        LCD_DisplayChar(5, 16, 'W');       //W:    �Է��� //EXTI  �߻���
        GPIOG->ODR |= 0x0002;
        X = 1;
        LCD_SetBrushColor(RGB_PINK);  // COLOR :pink       �����     ����� ����� 
        LCD_DrawFillRect(11 + X * 15, 71, 5, 5);//  �ʱ� ��ġ     //����ġ ���Ұ�     //�׸��� ��ġ ������
        EXTI->IMR &= ~0x0200;
        EXTI->IMR |= 0x0800;
        BEEP();
    }
}
void EXTI15_10_IRQHandler(void) {
    if (EXTI->PR & 0x0800)      // EXTI11-15���� Interrupt Pending(�߻�) ����?
    {
        EXTI->PR |= 0x0800;      // Pending bit Clear (clear�� ���ϸ� ���ͷ�Ʈ ������ �ٽ� ���ͷ�Ʈ �߻�)
        LCD_DisplayChar(5, 16, 'W');       //W:    �Է��� //EXTI  �߻���
        GPIOG->ODR |= 0x0008;
        X = 2;
        LCD_SetBrushColor(RGB_PINK);  // COLOR :pink       
        LCD_DrawFillRect(11 + X * 15, 71, 5, 5);//  �ʱ� ��ġ     //����ġ ���Ұ�     
        EXTI->IMR &= ~0x0800;
        EXTI->IMR |= 0x1000;
        BEEP();
    }
    else if (EXTI->PR & 0x1000)
    {
        // Pending bit Clear (clear�� ���ϸ� ���ͷ�Ʈ ������ �ٽ� ���ͷ�Ʈ �߻�)
        EXTI->PR |= 0x1000;
        LCD_DisplayChar(5, 16, 'W');       //W:    �Է��� //EXTI  �߻���
        GPIOG->ODR |= 0x0010;
        X = 3;
        LCD_SetBrushColor(RGB_PINK);  // COLOR :pink       
        LCD_DrawFillRect(11 + X * 15, 71, 5, 5);//  �ʱ� ��ġ     //����ġ ���Ұ�     
        EXTI->IMR &= ~0x1000;
        EXTI->IMR |= 0x2000;
        BEEP();
    }
    else if (EXTI->PR & 0x2000)
    {
        // Pending bit Clear (clear�� ���ϸ� ���ͷ�Ʈ ������ �ٽ� ���ͷ�Ʈ �߻�)
        EXTI->PR |= 0x2000;
        LCD_DisplayChar(5, 16, 'W');       //W:    �Է��� //EXTI  �߻���
        GPIOG->ODR |= 0x0020;
        X = 4;
        LCD_SetBrushColor(RGB_PINK);  // COLOR :pink       
        LCD_DrawFillRect(11 + X * 15, 71, 5, 5);//  �ʱ� ��ġ     //����ġ      
        EXTI->IMR &= ~0x2000;
        EXTI->IMR |= 0x4000;
        BEEP();
    }
    else if (EXTI->PR & 0x4000)
    {
        EXTI->PR |= 0x4000;      // Pending bit Clear (clear�� ���ϸ� ���ͷ�Ʈ ������ �ٽ� ���ͷ�Ʈ �߻�)
        LCD_DisplayChar(5, 16, 'W');       //W:    �Է��� //EXTI  �߻���
        GPIOG->ODR |= 0x0040;
        X = 5;
        LCD_SetBrushColor(RGB_PINK);  // COLOR :pink      
        LCD_DrawFillRect(11 + X * 15, 71, 5, 5);//  �ʱ� ��ġ     
        EXTI->IMR &= 0x4000;
        EXTI->IMR |= 0x8000;
        BEEP();
    }
    else if (EXTI->PR & 0x8000)
    {
        EXTI->PR |= 0x8000;      // Pending bit Clear (clear�� ���ϸ� ���ͷ�Ʈ ������ �ٽ� ���ͷ�Ʈ �߻�)
        LCD_DisplayChar(5, 16, 'C');       //C:    Ż�⼺�� 
        GPIOG->ODR |= 0x0080;
        X = 6;
        LCD_SetBrushColor(RGB_PINK);  //MOUSE COLOR :pink       �����     ����� ����� 
        LCD_DrawFillRect(11 + X * 15, 71, 5, 5);// MOUSE �ʱ� ��ġ     //����ġ ���Ұ�     //�׸��� ��ġ ������
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
        GPIOG->ODR = 0xFF00;	// LED �ʱⰪ: LED0~7 Off
        DisplayInitScreen();
        EXTI->IMR &= ~0xFA00;        //EXTI9,11,12,13,14,15  ���ͷ�Ʈ disable����
        EXTI->IMR |= 0x0100;      // EXTI8���ͷ�Ʈ mask (Interrupt Enable) ����

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
