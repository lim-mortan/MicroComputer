//////////////////////////////////////////////////////////////////////////////
//   ������ : ���������� �����
// 	 �������� : ��ǥ ���� �Է��ϸ� ���������͸� ��ǥ ��(B~6��)���� �̵��ϰ��ϴ� 
//           �������α׷� �ۼ�
//            
//   ����� �ϵ����(���) : GPIO, Switch, LED, GLCD, Buzzer
//   ������ : 2022. 6.15
//   ������ Ŭ���� : ����Ϲ�
//          �й� : 2017132030
//          �̸� : ������
//////////////////////////////////////////////////////////////////////////////

#include "stm32f4xx.h"
#include "GLCD.h"
#include "FRAM.h"

#define SW0_PUSH        0xFE00  //PH8
#define SW1_PUSH        0xFD00  //PH9
#define SW2_PUSH        0xFB00  //PH10
#define SW3_PUSH        0xF700  //PH11
#define SW4_PUSH        0xEF00  //PH12
#define SW5_PUSH        0xDF00  //PH13
#define SW6_PUSH        0xBF00  //PH14
#define SW7_PUSH        0x7F00  //PH15

#define RGB_VIOLET GET_RGB(238,230,250) //�����

void _GPIO_Init(void);
void _EXTI_Init(void);

void DisplayInitScreen(void);
uint16_t KEY_Scan(void);
void BEEP(void);

void UpMoveELDisplay(void);
void DnMoveELDisplay(void);
void Framff (void);

void ELMovePaint(uint16_t display_x,uint16_t display_y,uint16_t display_w,uint16_t display_h);
void DelayMS(unsigned short wMS);
void DelayUS(unsigned short wUS);


uint16_t arrive_count;  //6�� ���� Ƚ��
uint16_t Mode = 0; //���۸��, �����ø��
uint16_t goal_f = 1; //��ǥ �� 
uint16_t now_f = 1; //���� ��(�ʱ� 1��) 

int main(void)
{
    Fram_Init();            // FRAM �ʱ�ȭ H/W �ʱ�ȭ
    Fram_Status_Config();   // FRAM �ʱ�ȭ S/W �ʱ�ȭ 

    LCD_Init();	// LCD ��� �ʱ�ȭ
    DelayMS(10);
    _GPIO_Init();	// GPIO �ʱ�ȭ
    _EXTI_Init();	// EXTI �ʱ�ȭ

    DisplayInitScreen();	// LCD �ʱ�ȭ��
    GPIOG->ODR &= ~0x00FF;	// �ʱⰪ: LED0~7 Off

    arrive_count = Fram_Read(616); //FRAM 616������ 6������Ƚ�� ����
    uint16_t now_key = 0;
    while(1)
    {
        if (Mode != 0) continue; //�� ���� ���
        now_key = KEY_Scan();//Ű��ĵ�� �޾ƿ�
        if (now_key == 0xFE00)
        {
            GPIOG->ODR &= ~0x00FF; // LED0~7 OFF
            GPIOG->ODR |= 0X0001; 	//  LED0 ON                        
            LCD_SetTextColor(RGB_BLACK);
           
            LCD_DisplayText(3, 3, "B");
            //BEEP();
            goal_f = 0; //B�� ����
            EXTI->IMR |= 0x8000;  //EXTI15 Interrupt Mask���� - ���۸�� ���ͷ�Ʈ �߻�
        }
        else if (now_key == 0xFD00)
            {
                GPIOG->ODR &= ~0x00FF;
                GPIOG->ODR |= 0X0002;   //  LED1 ON
                goal_f = 1;
            }
            else if (now_key == 0xFB00)
            {
                GPIOG->ODR &= ~0x00FF;
                GPIOG->ODR |= 0X0004;
                goal_f = 2;
            }
            else if (now_key == 0xF700)
            {
                GPIOG->ODR &= ~0x00FF;
                GPIOG->ODR |= 0X0008;
                goal_f = 3;
            }
            else if (now_key == 0xEF00)
            {
                GPIOG->ODR &= ~0x00FF;
                GPIOG->ODR |= 0X0010;
                goal_f = 4;
            }
            else if (now_key == 0xDF00)
            {
                GPIOG->ODR &= ~0x00FF;
                GPIOG->ODR |= 0X0020;
                goal_f = 5;
            }
            else if (now_key == 0xBF00)
            {
                GPIOG->ODR &= ~0x00FF;
                GPIOG->ODR |= 0X0040;
                goal_f = 6;
            }
            
        
       

        LCD_SetTextColor(RGB_BLACK);
    LCD_DisplayChar(3, 3, 0x30 + goal_f);
    //BEEP();
    EXTI->IMR |= 0x8000;  //���۸�� ���ͷ�Ʈ �߻�
    }
}

/* GLCD �ʱ�ȭ�� ���� �Լ� */
void DisplayInitScreen(void)
{
    LCD_Clear(RGB_YELLOW);		// ȭ�� Ŭ���� Yellow
    LCD_SetFont(&Gulim8);		// ��Ʈ : ���� 8

    LCD_SetTextColor(RGB_BLACK);
    LCD_SetBackColor(RGB_YELLOW);
    LCD_DisplayText(1, 0, "2017132030 LHJ");
    LCD_DisplayChar(2, 0, 'S');
    LCD_DisplayText(2, 4, "6No:");
    LCD_DisplayText(3, 0, "FL:1 B 1 2 3 4 5 6");

    LCD_SetTextColor(RGB_VIOLET);
    LCD_DisplayChar(2, 2, 'O');
    arrive_count = Fram_Read(616); //FRAM 616������ 6������Ƚ�� ����
    LCD_DisplayChar(2, 8, 0x30 + arrive_count); //6�� ���� Ƚ��


    LCD_SetTextColor(RGB_BLACK);	// ���ڻ� : Black
    LCD_SetBackColor(RGB_YELLOW);	// ���ڹ��� : YELLOW

    LCD_SetBrushColor(RGB_RED); //���� ���������� ��ġ ǥ��
    LCD_DrawFillRect(55, 55, 8, 8);
    LCD_SetPenColor(RGB_GREEN);
    LCD_DrawRectangle(55, 55, 8, 8);
    LCD_DrawRectangle(0, 5, 157, 65);
}


/* GPIO (GPIOG(LED), GPIOH(Switch), GPIOF(Buzzer)) �ʱ� ����	*/
void _GPIO_Init(void)
{
    // LED (GPIO G) ���� : Output mode
    RCC->AHB1ENR |= 0x00000040;	// RCC_AHB1ENR : GPIOG(bit#6) Enable							
    GPIOG->MODER |= 0x00005555;	// GPIOG 0~7 : Output mode (0b01)						
    GPIOG->OTYPER &= ~0x00FF;	// GPIOG 0~7 : Push-pull  (GP8~15:reset state)	
    GPIOG->OSPEEDR |= 0x00005555;	// GPIOG 0~7 : Output speed 25MHZ Medium speed 

    // SW (GPIO H) ���� : Input mode 
    RCC->AHB1ENR |= 0x00000080;	// RCC_AHB1ENR : GPIOH(bit#7) Enable							
    GPIOH->MODER &= ~0xFFFF0000;	// GPIOH 8~15 : Input mode (reset state)				
    GPIOH->PUPDR &= ~0xFFFF0000;	// GPIOH 8~15 : Floating input (No Pull-up, pull-down) :reset state

    // Buzzer (GPIO F) ���� : Output mode
    RCC->AHB1ENR |= 0x00000020;	// RCC_AHB1ENR : GPIOF(bit#5) Enable							
    GPIOF->MODER |= 0x00040000;	// GPIOF 9 : Output mode (0b01)						
    GPIOF->OTYPER &= ~0x0200;	// GPIOF 9 : Push-pull  	
    GPIOF->OSPEEDR |= 0x00040000;	// GPIOF 9 : Output speed 25MHZ Medium speed 
}



/* EXTI �ʱ� ����  */
void _EXTI_Init(void)
{
    RCC->AHB1ENR |= 0x00000080;	// RCC_AHB1ENR GPIOH Enable  
    RCC->APB2ENR |= 0x00004000;	// Enable System Configuration Controller Clock  

    GPIOH->MODER &= ~0xFFFF0000;	// GPIOH PIN8~PIN15 Input mode (reset state)				 

    SYSCFG->EXTICR[3] |= 0x7000; //EXTI 15�� ���� �ҽ� �Է��� GPIOH�� ����

    EXTI->FTSR |= 0x8000;		// EXTI15 Falling Trigger Enable 

    EXTI->IMR |= 0x8000; //EXTI15 ���ͷ�Ʈ mask����

    NVIC->ISER[1] |= (1 << 8); //Enable Interrupt EXTI15
}

/* EXTI15~10 ���ͷ�Ʈ �ڵ鷯(ISR: Interrupt Service Routine) */
void EXTI15_10_IRQHandler(void)
{
    if (EXTI->PR & 0x8000)  //EXTI15 : ���۸�� ���ͷ�Ʈ
    {
        EXTI->PR |= 0x8000;  //EXTI15 Pending bit clear
        GPIOG->ODR |= 0X0080; // LED 7 ON
        Mode = 1; //���� ���۸��

        if (now_f < goal_f) //���������� ��� ��
        {
           UpMoveELDisplay();
            for (int i = now_f; i <= goal_f; i++) {  //���������� ��½� Bar �׸���
              ELMovePaint(40 + 16 * now_f, 55, 8 + (i - now_f) * 16, 8);  
            }
            for (int i = 0; i < 3; ++i)
            {
                DelayMS(500);
                BEEP();
            }

            LCD_SetTextColor(RGB_VIOLET);
            LCD_DisplayChar(2, 2, 'O');
            LCD_SetTextColor(RGB_BLACK);
            LCD_DisplayChar(2, 0, 'S');

            

            Framff();

            }
        }
         if (now_f > goal_f) //���������� �ϰ� ��
        {
         
            DnMoveELDisplay();

            for (int i = now_f; i > goal_f; i--) //���������� �ϰ� Bar �׸���
            {
               ELMovePaint(55,40 + 16 * now_f,8 , 8 + (i - now_f) * 16);
            }
            for (int i = 0; i < 3; ++i)
            {
                DelayMS(500);
                BEEP();
            }

            LCD_SetTextColor(RGB_BLACK);
            LCD_DisplayChar(2, 0, 'S');
            now_f = goal_f; //������ �ʱ�ȭ
        }
        if (now_f == goal_f)//: ���� �� �Է½�
        {
            EXTI->PR |= 0x8000; //EXTI15 Pending bit clear
            LCD_SetTextColor(RGB_VIOLET);
            LCD_DisplayChar(2, 2, 'O');
            LCD_SetTextColor(RGB_BLACK);
            LCD_DisplayChar(2, 0, 'S');
            GPIOG->ODR &= ~0x0080;
            BEEP();
        }
        GPIOG->ODR &= ~0X0080; //LED7 OFF
        EXTI->IMR &= ~0x8000; //EXTI15 ���ͷ�Ʈ mask Ǯ��
        Mode = 0; // �����ø��
 }

void Framff (void)
{
  now_f = goal_f;  //������ �ʱ�ȭ
  if (now_f == 6) { //6�� ���޽�
  arrive_count++;
  Fram_Write(616, arrive_count); //FRAM616������ 6������Ƚ�� ����
  if (arrive_count > 9) { //6������Ƚ�� 9->0
  arrive_count = 0;
  Fram_Write(616, arrive_count); //FRAM616������ 6������Ƚ�� ����
 
  }
  
LCD_DisplayChar(2, 8, 0x30 + arrive_count);
}
}
uint8_t key_flag = 0; // global var
uint16_t KEY_Scan(void)	// input key SW0 - SW7 
{ 
	uint16_t key;
	key = GPIOH->IDR & 0xFF00;	// any key pressed ?
	if(key == 0xFF00)		// if no key, check key off
	{  	if(key_flag == 0)
        		return key;
      		else
		{	DelayMS(10);
        		key_flag = 0;
        		return key;
        	}
    	}
  	else				// if key input, check continuous key
	{	if(key_flag != 0)	// if continuous key, treat as no key input
        		return 0xFF00;
      		else			// if new key,delay for debounce
		{	key_flag = 1;
			DelayMS(10);
 			return key;
        	}
	}
}

void UpMoveELDisplay(void)
{
    LCD_SetTextColor(RGB_BLACK);
    LCD_DisplayChar(2, 0, 'M');  //���۸�� ǥ��
    LCD_SetTextColor(RGB_RED);
    LCD_DisplayChar(2, 2, '>');  //��� ǥ��
    LCD_SetBrushColor(RGB_YELLOW);
    LCD_DrawFillRect(0, 52, 150, 15); // ���������� ��� �����
    LCD_SetBrushColor(RGB_RED);
    LCD_SetPenColor(RGB_GRAY);
    LCD_DrawFillRect((40 + 16 * now_f), 55, 8, 8); //���� ���������� �׸���
    LCD_DrawRectangle((40 + 16 * now_f), 55, 8, 8);
    BEEP();
    DelayMS(500);
    BEEP();
    LCD_SetBrushColor(RGB_RED);
    LCD_SetPenColor(RGB_GRAY);
}

void DnMoveELDisplay(void)
{
   LCD_SetTextColor(RGB_BLUE);
    LCD_DisplayChar(2, 2, '<'); //���������� �ϰ� ǥ��
    LCD_SetTextColor(RGB_BLACK);
    LCD_DisplayChar(2, 0, 'M'); //���۸�� ǥ��
    LCD_SetBrushColor(RGB_YELLOW);
    LCD_DrawFillRect(0, 52, 150, 15); // ���������� �̵���� �����
    LCD_SetBrushColor(RGB_BLUE);
    LCD_SetPenColor(RGB_GRAY);
    LCD_DrawFillRect((40 + 16 * now_f), 55, 8, 8);   //���� ���������� �׸���
    LCD_DrawRectangle((40 + 16 * now_f), 55, 8, 8);  

    BEEP();
    DelayMS(500);
    BEEP();
    LCD_SetBrushColor(RGB_BLUE);
    LCD_SetPenColor(RGB_GRAY);
}


void ELMovePaint(uint16_t display_x,uint16_t display_y,uint16_t display_w,uint16_t display_h)
{
   DelayMS(500);
   LCD_DrawFillRect(display_x,display_y, display_w, display_h);
   LCD_DrawRectangle(display_x,display_y, display_w, display_h);
}


/* Buzzer: Beep for 30 ms */
void BEEP(void)
{
    GPIOF->ODR |= 0x0200;	// PF9 'H' Buzzer on
    DelayMS(30);		// Delay 30 ms
    GPIOF->ODR &= ~0x0200;	// PF9 'L' Buzzer off
}

void DelayMS(unsigned short wMS)
{
    register unsigned short i;
    for (i = 0; i < wMS; i++)
        DelayUS(1000);	// 1000us => 1ms
}

void DelayUS(unsigned short wUS)
{
    volatile int Dly = (int)wUS * 17;
    for (; Dly; Dly--);
}
