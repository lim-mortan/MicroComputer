/////////////////////////////////////////////////////////////
// ������: TP1. ������ ����
// ��������: ���ڸ� 2���� ���ڸ� ADD(+), SUB(-), OR(#), AND(&)�����ϴ� ���⸦ ����
// ����� �ϵ����(���): GPIO, EXTI, GLCD , KEY_Scan, BEEP, IRQHandler, stm32f4xx, Delay, FRAM, JOY_Scan
// ������: 2022. 6. 10
// ������ Ŭ����: ����Ϲ�
// �й�: 2017132030
// �̸�: ������
///////////////////////////////////////////////////////////////
#include "stm32f4xx.h"
#include "GLCD.h"
#include "FRAM.h"

void _GPIO_Init(void);
void _EXTI_Init(void);
void BEEP(void);
void Operand(void);    // ���۷��� �Լ�
void MYADD(void);      // ���� �Լ�
void MYSUB(void);      // ���� �Լ�
void MYAND(void);      // and�Լ�
void MYOR(void);       // OR�Լ�
void MYWARNING(void);  //���� �Լ�
void DelayMS(unsigned short wMS);
void DelayUS(unsigned short wUS);
void DisplayInitScreen(void);

#define SW0_PUSH        0xFE00  //PH8
#define SW1_PUSH        0xFD00  //PH9
#define SW2_PUSH        0xFB00  //PH10
#define SW3_PUSH        0xF700  //PH11
#define NAVI_PUSH	0x03C0  //PI5 0000 0011 1100 0000 
#define NAVI_UP		0x03A0  //PI6 0000 0011 1010 0000 
#define NAVI_DOWN     0x0360  //PI7 0000 0011 0110 0000 
#define NAVI_RIGHT	0x02E0  //PI8 0000 0010 1110 0000 
#define NAVI_LEFT	0x01E0  //PI9 0000 0001 1110 0000 

uint16_t KEY_Scan(void);
uint16_t JOY_Scan(void);
uint8_t	SW0_Flag = 0, SW1_Flag, SW2_Flag, SW3_Flag, SW4_Flag, SW5_Flag, SW6_Flag, SW7_Flag;
uint8_t op_D, op_C, op_B, op_A, op_1p = 0, op_2p = 0, U_key, L_key, D_key, R_key;   // ��Ʈ �ڸ��� ������ FRAM �Լ�


int R = 0, L = 0, U = 0, D = 0; // cnt(R,L,U,D)
int F1_0, F1_1, F1_2, F1_3;
int R2_0, R2_1, R2_2, R2_3;
int Se1, Se2;
int PL, p, point[5], SUM, MI, AND_p, OR_p;
int C_a = 0, C_b = 0, C_c = 0, C_d = 0;  //������ �Լ��� ���̴� ���� �� �迭, ����üũ

void First_Init();

int main(void)
{
    First_Init();

    L_key = Fram_Read(525);   // FRAM Ư���ּ�(525)�κ���1 byte (return) �о���� �Լ�
    R_key = Fram_Read(526);   // FRAM Ư���ּ�(526)�κ���1 byte (return) �о���� �Լ�
    U_key = Fram_Read(527);   // FRAM Ư���ּ�(527)�κ���1 byte (return) �о���� �Լ�
    D_key = Fram_Read(528);   // FRAM Ư���ּ�(528)�κ���1 byte (return) �о���� �Լ�

    while (1)
    {
        Operand();
        if (C_a && !C_b && (C_c || C_d))    // !not�����ڷ� �ڵ���� //ù��° ���� �������ϸ�
        {
            R2_3 = 0, R2_2 = 0, R2_1 = 0, R2_0 = 0, C_c = 0, C_d = 0;
            LCD_DrawChar(13 + 35, 27, ' ');
            LCD_DrawChar(20 + 35, 27, ' ');
            LCD_DrawChar(27 + 35, 27, ' ');
            LCD_DrawChar(34 + 35, 27, ' ');
            MYWARNING();
            SW4_Flag = 0, op_2p = 1;
        }
        else if (C_a && C_b && !C_c && C_d) // ����2�� �������ϸ�
        {
            C_d = 0;
            MYWARNING();
        }
        else if ((C_a > 1 || C_b > 1 || C_c > 1) && !C_d) // ����3�� �������ϸ�
        {
            C_a = 1, C_b = 1, C_c = 1;
            MYWARNING();
        }
        else if ((C_a > 1 || C_b > 1 || C_c > 1) && C_d) // ����4�� �������ϸ�
        {
            C_a = 1, C_b = 1, C_c = 1;
            MYWARNING();
        }
    } // while(1)
} // main()

void Operand(void)  //�� ���۷��忡 �� ���� ��� �Լ�
{
    switch (KEY_Scan())
    {
    case SW0_PUSH: 	//SW0
        if (!SW0_Flag)
        {
            GPIOG->BSRRL = 0x0001; 	//  LED0 (PG0) Toggle
            op_D = 1;
            SW0_Flag = 1;
        }
        else
        {
            GPIOG->BSRRH = 0x0001; 	//  LED0 (PG0) Toggle
            op_D = 0;
            SW0_Flag = 0;
        }
        break;
    case SW1_PUSH: 	//SW1
        if (!SW1_Flag)
        {
            GPIOG->BSRRL = 0x0002;   //  LED1 (PG1) Toggle
            op_C = 1;
            SW1_Flag = 1;
        }
        else
        {
            GPIOG->BSRRH = 0x0002;   //  LED1 (PG1) Toggle
            op_C = 0;
            SW1_Flag = 0;
        }
        break;
    case SW2_PUSH: 	//SW2
        if (!SW2_Flag)
        {
            GPIOG->BSRRL = 0x0004;   //  LED2 (PG1) Toggle
            op_B = 1;
            SW2_Flag = 1;
        }
        else
        {
            GPIOG->BSRRH = 0x0004;   //  LED2 (PG1) Toggle
            op_B = 0;
            SW2_Flag = 0;
        }
        break;
    case SW3_PUSH: 	//SW3
        if (!SW3_Flag)
        {
            GPIOG->BSRRL = 0x0008;   //  LED3 (PG1) Toggle
            op_A = 1;
            SW3_Flag = 1;
        }
        else
        {
            GPIOG->BSRRH = 0x0008;   //  LED3 (PG1) Toggle
            op_A = 0;
            SW3_Flag = 0;
        }
        break;
    } // switch(KEY_Scan())
}   //void Operand(void)

void MYADD(void) //���� �Լ�
{
    Se1 = F1_3 * 8 + F1_2 * 4 + F1_1 * 2 + F1_0; //ù��° ���۷��� �������� ��������
    Se2 = R2_3 * 8 + R2_2 * 4 + R2_1 * 2 + R2_0; //�ι�° ���۷��� �������� ��������
    SUM = Se1 + Se2;

    for (int p = 0; p <= 4; p++)
    {
        point[p] = SUM % 2;    // 2�� �������� �� �������� ����
        SUM = SUM / 2;             // ���� ����
    }
    if (point[4]) // �ټ�°�ڸ����� �������� ������
    {
        LCD_DrawChar(82, 27, 0x30 + point[4]);
        LCD_DrawChar(89, 27, 0x30 + point[3]);
        LCD_DrawChar(96, 27, 0x30 + point[2]);
        LCD_DrawChar(103, 27, 0x30 + point[1]);
        LCD_DrawChar(110, 27, 0x30 + point[0]);

    }
    else //�׹�°�ڸ� �������̶��
    {
        LCD_DrawChar(82, 27, ' ');
        LCD_DrawChar(89, 27, 0x30 + point[3]);
        LCD_DrawChar(96, 27, 0x30 + point[2]);
        LCD_DrawChar(103, 27, 0x30 + point[1]);
        LCD_DrawChar(110, 27, 0x30 + point[0]);
    }
}   //void Plus(void)

void MYSUB(void)    // �����Լ�
{
    Se1 = F1_3 * 8 + F1_2 * 4 + F1_1 * 2 + F1_0; //ù��° ���۷��� �������� ��������
    Se2 = R2_3 * 8 + R2_2 * 4 + R2_1 * 2 + R2_0; //�ι�° ���۷��� �������� ��������
    SUM = Se1 - Se2;

    if (SUM > 0) {
        for (int i = 0; i <= 4; i++)
        {
            point[p] = SUM % 2;
            SUM = SUM / 2;
        }
        if (point[4]) //�ټ�°�ڸ����� �������� ������
        {
            LCD_DrawChar(82, 27, 0x30 + point[4]);
            LCD_DrawChar(89, 27, 0x30 + point[3]);
            LCD_DrawChar(96, 27, 0x30 + point[2]);
            LCD_DrawChar(103, 27, 0x30 + point[1]);
            LCD_DrawChar(110, 27, 0x30 + point[0]);
        }
        else // �׹�°�ڸ� �������̶��
        {
            LCD_DrawChar(82, 27, ' ');
            LCD_DrawChar(89, 27, 0x30 + point[3]);
            LCD_DrawChar(96, 27, 0x30 + point[2]);
            LCD_DrawChar(103, 27, 0x30 + point[1]);
            LCD_DrawChar(110, 27, 0x30 + point[0]);
        }

    }
    else {
        for (int i = 0; i <= 4; i++)
        {
            point[p] = -SUM % 2;
            SUM = SUM / 2;
        }
        LCD_DrawChar(82, 27, ' ');
        LCD_DrawChar(89, 27, 0x30 + point[3]);
        LCD_DrawChar(96, 27, 0x30 + point[2]);
        LCD_DrawChar(103, 27, 0x30 + point[1]);
        LCD_DrawChar(110, 27, 0x30 + point[0]);
    }

}   //void Minus(void)

void MYAND(void)  //������ AND�Լ�
{
    if (F1_0 && R2_0)
        point[0] = F1_0;
    else point[0] = 0;

    if (F1_1 && R2_1)
        point[1] = F1_1;
    else point[1] = 0;

    if (F1_2 && R2_2)
        point[2] = F1_2;
    else point[2] = 0;

    if (F1_3 && R2_3)
        point[3] = F1_3;
    else point[3] = 0;

    LCD_DrawChar(82, 27, ' ');
    LCD_DrawChar(89, 27, 0x30 + point[3]);
    LCD_DrawChar(96, 27, 0x30 + point[2]);
    LCD_DrawChar(103, 27, 0x30 + point[1]);
    LCD_DrawChar(110, 27, 0x30 + point[0]);
}   //void AND(void)

void MYOR(void)   // ������ OR�Լ�
{
    if (F1_0 || R2_0)
        point[0] = 1;
    else  point[0] = 0;

    if (F1_1 || R2_1)
        point[1] = 1;
    else point[1] = 0;

    if (F1_2 || R2_2)
        point[2] = 1;
    else point[2] = 0;

    if (F1_3 || R2_3)
        point[3] = 1;
    else point[3] = 0;

    LCD_DrawChar(82, 27, ' ');
    LCD_DrawChar(89, 27, 0x30 + point[3]);
    LCD_DrawChar(96, 27, 0x30 + point[2]);
    LCD_DrawChar(103, 27, 0x30 + point[1]);
    LCD_DrawChar(110, 27, 0x30 + point[0]);
}   //void OR(void)

void MYWARNING(void)  // ERROR�߻��� 
{
    LCD_SetBrushColor(RGB_RED); //�÷� RED
    LCD_DrawFillRect(140, 3, 10, 10); // �ʱ� ��ġ
    LCD_SetPenColor(RGB_GREEN); //  GREEN
    LCD_DrawRectangle(140, 3, 10, 10); //  error �簢�� �׸���  
    DelayMS(2000);
    LCD_SetBrushColor(RGB_GRAY); //�÷� GRAY
    LCD_DrawFillRect(140, 3, 10, 10); // �ʱ� ��ġ
    BEEP();
    DelayMS(500);
    BEEP();
}

/* GLCD �ʱ�ȭ�� ���� �Լ� */
void DisplayInitScreen(void)
{
    LCD_Clear(RGB_YELLOW);		// ȭ�� Ŭ����
    LCD_SetFont(&Gulim7);		// ��Ʈ : ���� 7
    LCD_SetBackColor(RGB_BLUE);	// ���ڹ��� : BLUE
    LCD_SetTextColor(RGB_WHITE);	// ���ڻ� : WHITE
    LCD_DrawText(4, 3, "LHJ_Calculator");  // �л� ���� �̴ϼ�
    LCD_SetBackColor(RGB_YELLOW);	//���ڹ��� : Yellow
    LCD_SetTextColor(RGB_BLACK);	// ���ڻ� : BLACK
    LCD_DrawText(105, 3, "Error");  // Error ǥ��
    LCD_SetFont(&Gulim8);		// ��Ʈ : ���� 8
    LCD_DrawChar(5, 26, '>'); // ������â >ǥ��
    LCD_SetFont(&Gulim7);		// ��Ʈ : ���� 7
    LCD_DrawText(3, 50, "+:0, -:0, #:0, &:0"); // ������ ��� ǥ��
    LCD_SetTextColor(RGB_RED); // ���ڹ��� : RED
    LCD_DrawChar(15, 50, '0');
    LCD_DrawChar(45, 50, '0');
    LCD_DrawChar(75, 50, '0');
    LCD_DrawChar(105, 50, '0');
    LCD_DrawChar(15, 50, 0x30 + L_key);
    LCD_DrawChar(45, 50, 0x30 + R_key);
    LCD_DrawChar(75, 50, 0x30 + U_key);
    LCD_DrawChar(105, 50, 0x30 + D_key);
    LCD_SetBrushColor(RGB_GRAY); //  �÷� GRAY
    LCD_DrawFillRect(140, 3, 10, 10); // �ʱ� ��ġ
    LCD_SetPenColor(RGB_GREEN); // ���: GREEN
    LCD_DrawRectangle(140, 3, 10, 10); //  error �簢�� �׸���  
    LCD_DrawRectangle(3, 2, 85, 11); // �̴ϼȻ簢�� �׸���  
    LCD_DrawRectangle(3, 25, 150, 15); // ����â�簢�� �׸���  
    LCD_DrawRectangle(0, 0, 159, 127); // ��üȭ��簢�� �׸���  
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
    RCC->AHB1ENR |= 0x00000080;  // (1<<7)	// RCC_AHB1ENR GPIOH Enable
    RCC->APB2ENR |= 0x00004000;  // (1<<14)	// Enable System Configuration Controller Clock

    GPIOH->MODER &= ~0xFF000000;	// GPIOH PIN12~PIN15 Input mode (reset state)				 

    EXTI->FTSR |= 0x0020;         // EXTI5: Falling Trigger  Enable
    EXTI->FTSR |= 0x0040;         // EXTI6: FallingTrigger  Enable
    EXTI->FTSR |= 0x0080;         // EXTI7: Falling Trigger  Enable
    EXTI->FTSR |= 0x0100;         // EXTI8: Falling Trigger  Enable
    EXTI->FTSR |= 0x0200;         // EXTI9: FallingTrigger  Enable
    EXTI->FTSR |= 0x1000;         // EXTI12: FallingTrigger  Enable
    EXTI->FTSR |= 0x2000;         // EXTI13: Falling Trigger  Enable
    EXTI->FTSR |= 0x4000;         // EXTI14: FallingTrigger  Enable
    EXTI->FTSR |= 0x8000;         // EXTI15: FallingTrigger  Enable

    EXTI->IMR |= 0x1000;		// EXTI12���ͷ�Ʈ mask (Interrupt Enable) ����
    EXTI->IMR |= 0x4000;		// EXTI14���ͷ�Ʈ mask (Interrupt Enable) ����
    EXTI->IMR |= 0x8000;		// EXTI15���ͷ�Ʈ mask (Interrupt Enable) ����

    // EXTI�ʱ�ȭ
    SYSCFG->EXTICR[1] |= 0x8880;   // pi 5, 6, 7
    SYSCFG->EXTICR[2] |= 0x0088;  // pi 8, 9   
    SYSCFG->EXTICR[3] |= 0x7777;  // ph12~15

    NVIC->ISER[0] |= (1 << 23);    //0x00800000   // Enable 'Global Interrupt EXTI5, 6, 7, 8, 9'
    NVIC->ISER[1] |= (1 << 8);    //0x00000100     //Enable 'Global Interrupt EXTI11, 12, 13, 14, 15' 
}

/* EXTI5~9 ���ͷ�Ʈ �ڵ鷯(ISR: Interrupt Service Routine) */
void EXTI9_5_IRQHandler(void)
{
    if (EXTI->PR & 0x0020)			// EXTI5 Interrupt Pending(�߻�) ����?
    {
        EXTI->PR |= 0x0020;		// Pending bit Clear (clear�� ���ϸ� ���ͷ�Ʈ ������ �ٽ� ���ͷ�Ʈ �߻�)
        C_b += 1;
        if (C_a == 1 && C_b == 1)
        {
            BEEP();
            op_1p = 1;
            SW4_Flag = 0, op_2p = 1;
        }
    }
    if (EXTI->PR & 0x0040)			// EXTI6 Interrupt Pending(�߻�) ����?
    {
        EXTI->PR |= 0x0040;		// Pending bit Clear (clear�� ���ϸ� ���ͷ�Ʈ ������ �ٽ� ���ͷ�Ʈ �߻�)
        if (op_1p == 0)
        {
            LCD_SetTextColor(RGB_BLACK);	// ���ڻ� : Black
            LCD_DrawChar(41, 27, '#');
            U = U_key;
            U += 1, PL = 0, MI = 0, AND_p = 0, OR_p = 1, U_key = U;
            LCD_SetTextColor(RGB_RED); // ���ڻ� : RED
            if (U == 0)
                LCD_DrawChar(75, 50, 0x30); //  ACII�ڵ� 0
            if (U > 0 && U < 10)
                LCD_DrawChar(75, 50, 0x30 + U); // UȽ�� ǥ��
            if (U >= 10)
            {
                U -= 10;
                LCD_DrawChar(75, 50, 0x30 + U);
            }
        }
        Fram_Write(527, U_key);  // FRAM(527)������ ���� 
    }
    if (EXTI->PR & 0x0080)			// EXTI7 Interrupt Pending(�߻�) ����?
    {
        EXTI->PR |= 0x0080;		// Pending bit Clear (clear�� ���ϸ� ���ͷ�Ʈ ������ �ٽ� ���ͷ�Ʈ �߻�)
        if (op_1p == 0)
        {
            LCD_SetTextColor(RGB_BLACK);	// ���ڻ� : Black
            LCD_DrawChar(41, 27, '&');
            D = D_key;
            D += 1, PL = 0, MI = 0, AND_p = 1, OR_p = 0, D_key = D;
            LCD_SetTextColor(RGB_RED); // ���ڹ��� : RED
            if (D == 0)
                LCD_DrawChar(105, 50, 0x30); // ASCII�ڵ� 0
            if (D > 0 && D < 10)
                LCD_DrawChar(105, 50, 0x30 + D); // DȽ�� ǥ��
            if (D >= 10)
            {
                D -= 10;
                LCD_DrawChar(105, 50, 0x30 + D);
            }
        }
        Fram_Write(528, D_key);  // FRAM(528)������ ����
    }
    if (EXTI->PR & 0x0100)			// EXTI8 Interrupt Pending(�߻�) ����?
    {
        EXTI->PR |= 0x0100;		// Pending bit Clear (clear�� ���ϸ� ���ͷ�Ʈ ������ �ٽ� ���ͷ�Ʈ �߻�)
        if (op_1p == 0)
        {
            LCD_SetTextColor(RGB_BLACK);	// ���ڻ� : Black
            LCD_DrawChar(41, 27, '-');
            R = R_key;
            R += 1, PL = 0, MI = 1, AND_p = 0, OR_p = 0, R_key = R;
            LCD_SetTextColor(RGB_RED); // ���ڻ� : RED
            if (R == 0)
                LCD_DrawChar(45, 50, 0x30); // ASCII�ڵ� 0
            if (R > 0 && R < 10)
                LCD_DrawChar(45, 50, 0x30 + R); // RȽ�� ǥ��
            if (R >= 10)
            {
                R -= 10;
                LCD_DrawChar(45, 50, 0x30 + R);
            }
        }
        Fram_Write(526, R_key);  // FRAM(526)������ ����
    }
    if (EXTI->PR & 0x0200)		// EXTI9 Interrupt Pending(�߻�) ����?
    {
        EXTI->PR |= 0x0200;		// Pending bit Clear (clear�� ���ϸ� ���ͷ�Ʈ ������ �ٽ� ���ͷ�Ʈ �߻�)
        if (op_1p == 0)
        {
            LCD_SetTextColor(RGB_BLACK);	// ���ڻ� : Black
            LCD_DrawChar(41, 27, '+');
            L = L_key;
            L += 1, PL = 1, MI = 0, AND_p = 0, OR_p = 0, L_key = L;
            LCD_SetTextColor(RGB_RED); // ���ڹ��� : RED
            if (L == 0)
                LCD_DrawChar(15, 50, 0x30); // ASCII�ڵ� 0
            if (L > 0 && L < 10)
                LCD_DrawChar(15, 50, 0x30 + L); // LȽ�� ǥ��
            if (L >= 10)
            {
                L -= 10;
                LCD_DrawChar(15, 50, 0x30 + L);
            }
        }
        Fram_Write(525, L_key);  // FRAM(525)������ ���� 
    }
}   //void EXTI9_5_IRQHandler(void)

/* EXTI10~15 ���ͷ�Ʈ �ڵ鷯(ISR: Interrupt Service Routine) */
void EXTI15_10_IRQHandler(void)
{
    if (EXTI->PR & 0x1000)		// EXTI12 Interrupt Pending(�߻�) ����?
    {
        EXTI->PR |= 0x1000;		// Pending bit Clear (clear�� ���ϸ� ���ͷ�Ʈ ������ �ٽ� ���ͷ�Ʈ �߻�)
        if (SW4_Flag == 0 && op_2p == 0)	// ���ͷ�Ʈ (EXTI12) �߻�
        {
            C_a += 1;
            if (C_a == 1)
            {
                BEEP();
                EXTI->IMR |= 0xF3E0;  // EXTI5,6,7,8,9,12, 13,14,15���ͷ�Ʈ mask (Interrupt Enable) ����
                F1_3 = op_D, F1_2 = op_C, F1_1 = op_B, F1_0 = op_A;
                LCD_SetTextColor(RGB_BLACK);	// ���ڻ� : BLACK
                LCD_DrawChar(13, 27, 0x30 + F1_3);  // operand1 ǥ��
                LCD_DrawChar(20, 27, 0x30 + F1_2);
                LCD_DrawChar(27, 27, 0x30 + F1_1);
                LCD_DrawChar(34, 27, 0x30 + F1_0);
                op_2p = 1;
                GPIOG->ODR &= ~0x00FF;	// �ʱⰪ: LED0~7 Off
                op_D = 0, op_C = 0, op_B = 0, op_A = 0; // operand �� �ʱ�ȭ
            }
        }
        else if (SW4_Flag == 0 && op_2p == 1)
        {
            C_c += 1;
            if (C_b == 1 && C_c == 1)
            {
                BEEP();
                EXTI->IMR |= 0xF3E0;  // EXTI5,6,7,8,9,12, 13,14,15���ͷ�Ʈ mask (Interrupt Enable) ����
                R2_3 = op_D, R2_2 = op_C, R2_1 = op_B, R2_0 = op_A;
                LCD_SetTextColor(RGB_BLACK);	// ���ڻ� : BLACK
                LCD_DrawChar(13 + 35, 27, 0x30 + R2_3);   // �ι�° ���۷��� ǥ��
                LCD_DrawChar(20 + 35, 27, 0x30 + R2_2);
                LCD_DrawChar(27 + 35, 27, 0x30 + R2_1);
                LCD_DrawChar(34 + 35, 27, 0x30 + R2_0);
                op_2p = 0;
                GPIOG->ODR &= ~0x00FF;	// �ʱⰪ: LED0~7 Off
                op_D = 0, op_C = 0, op_B = 0, op_A = 0; // ���۷��� ��� �ʱ�ȭ
            }
        }
        SW4_Flag = 0;
    }
    else if (EXTI->PR & 0x2000)		// EXTI13 Interrupt Pending(�߻�) ����?
    {
        EXTI->PR |= 0x2000;		// Pending bit Clear (clear�� ���ϸ� ���ͷ�Ʈ ������ �ٽ� ���ͷ�Ʈ �߻�)
        if (SW5_Flag == 0)	// ���ͷ�Ʈ (EXTI13) �߻�
        {
            C_d = 1;
            if (C_d == 1 && C_c == 1)
            {
                LCD_SetTextColor(RGB_BLACK);	// ���ڻ� : BLACK
                LCD_DrawChar(76, 27, '=');  //��� ����� ��=�� ǥ��
                DelayMS(500);
                BEEP();
                DelayMS(500);
                BEEP();
                SW5_Flag = 0;
                if (PL == 1)
                    MYADD();
                else if (MI == 1)
                    MYSUB();
                else if (AND_p == 1)
                    MYAND();
                else if (OR_p == 1)
                    MYOR();
            }
            SW5_Flag = 1;
        }
        SW5_Flag = 0;
    }
    else if (EXTI->PR & 0x4000)		// EXTI14 Interrupt Pending(�߻�) ����?
    {
        EXTI->PR |= 0x4000;		// Pending bit Clear (clear�� ���ϸ� ���ͷ�Ʈ ������ �ٽ� ���ͷ�Ʈ �߻�)
        if (SW6_Flag == 0)	// ���ͷ�Ʈ (EXTI14) �߻� FRAM�� ��� ��0������ clear
        {
            LCD_SetTextColor(RGB_RED); // ���ڻ� : RED
            LCD_DrawChar(15, 50, '0');
            LCD_DrawChar(45, 50, '0');
            LCD_DrawChar(75, 50, '0');
            LCD_DrawChar(105, 50, '0');
            U = 0, D = 0, L = 0, R = 0;
            L_key = U, R_key = R, U_key = U, D_key = D;
            Fram_Write(525, L_key);  // FRAM(525)������ ���� 
            Fram_Write(526, R_key);  // FRAM(526)������ ���� 
            Fram_Write(527, U_key);  // FRAM(527)������ ���� 
            Fram_Write(528, D_key);  // FRAM(528)������ ���� 
        }
    }
    else if (EXTI->PR & 0x8000)		// EXTI15 Interrupt Pending(�߻�) ����?
    {
        if (SW7_Flag == 0)	// ���ͷ�Ʈ (EXTI15) �߻�     ��Enter��(SW7)�Է�
        {
            EXTI->PR |= 0x8000;		// Pending bit Clear (clear�� ���ϸ� ���ͷ�Ʈ ������ �ٽ� ���ͷ�Ʈ �߻�)
            DisplayInitScreen();
            Operand();
            op_2p = 0, op_1p = 0, C_a = 0, C_b = 0, C_c = 0, C_d = 0;
            EXTI->IMR &= ~0xF3E0;  // EXTI5,6,7,8,9,12, 13,14,15���ͷ�Ʈ mask (Interrupt disable) ����
            EXTI->IMR |= 0x1000;		// EXTI12���ͷ�Ʈ mask (Interrupt Enable) ����
            EXTI->IMR |= 0x4000;		// EXTI14���ͷ�Ʈ mask (Interrupt Enable) ����
            EXTI->IMR |= 0x8000;		// EXTI15���ͷ�Ʈ mask (Interrupt Enable) ����
            SW7_Flag = 0;
            GPIOG->ODR &= ~0x00FF;
        }
    }
}   //void EXTI15_10_IRQHandler(void)

uint8_t joy_flag = 0;
uint16_t JOY_Scan(void)	// input joy stick NAVI_* 
{
    uint16_t key;
    key = GPIOI->IDR & 0x03E0;	// any key pressed ?
    if (key == 0x03E0)		// if no key, check key off
    {
        if (joy_flag == 0)
            return key;
        else
        {
            DelayMS(10);
            joy_flag = 0;
            return key;     //LEFT : 0x01E0 RIGHT : 0x02E0 DOWN : 0x0360 UP: 0x03A0 PUSH : 0x03C0
        }
    }
    else				// if key input, check continuous key
    {
        if (joy_flag != 0)	// if continuous key, treat as no key input
            return 0x03E0;
        else			// if new key,delay for debounce
        {
            joy_flag = 1;
            DelayMS(10);
            return key;
        }
    }
}   //uint16_t JOY_Scan(void

/* Switch�� �ԷµǾ����� ���ο� � switch�� �ԷµǾ������� ������ return�ϴ� �Լ�  */
uint8_t key_flag = 0;
uint16_t KEY_Scan(void)	// input key SW0 - SW7 
{
    uint16_t key;
    key = GPIOH->IDR & 0xFF00;	// any key pressed ?
    if (key == 0xFF00)		// if no key, check key off
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
    else				// if key input, check continuous key
    {
        if (key_flag != 0)	// if continuous key, treat as no key input
            return 0xFF00;
        else			// if new key,delay for debounce
        {
            key_flag = 1;
            DelayMS(10);
            return key;
        }
    }
}   //uint16_t KEY_Scan(void)

/* Buzzer: Beep for 30 ms */
void BEEP(void)
{
    GPIOF->ODR |= 0x0200;	// PF9 'H' Buzzer on
    DelayMS(30);		// Delay 30 ms
    GPIOF->ODR &= ~0x0200;	// PF9 'L' Buzzer off
}   //void BEEP(void)

void DelayMS(unsigned short wMS)
{
    register unsigned short i;
    for (i = 0; i < wMS; i++)
        DelayUS(1000);	// 1000us => 1ms
}   //void DelayMS(unsigned short wMS)

void DelayUS(unsigned short wUS)
{
    volatile int Dly = (int)wUS * 17;
    for (; Dly; Dly--);
}   //void DelayUS(unsigned short wUS)

void First_Init()
{
    LCD_Init();
    DelayMS(10);

    _GPIO_Init();
    _EXTI_Init();
    Fram_Init();
    Fram_Status_Config();

    DisplayInitScreen();
    GPIOG->ODR &= ~0x00FF;
}
