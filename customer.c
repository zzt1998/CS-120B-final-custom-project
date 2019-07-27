
#define F_CPU 8000000UL		/* Define CPU Frequency e.g. here its 8MHz */
#include <avr/io.h>		/* Include AVR std. library file */
#include <util/delay.h>		/* Include inbuilt defined Delay header file */
#include <avr/interrupt.h>
//****************************************时间
volatile unsigned char TimerFlag = 0;
unsigned long _avr_timer_M = 1; // Start count from here, down to 0. Default 1 ms.
unsigned long _avr_timer_cntcurr = 0; // Current internal count of 1ms ticks

  
void TimerOn() {
	TCCR1B = 0x0B;
	OCR1A = 125;
	TIMSK1 = 0x02;
	TCNT1=0;
	_avr_timer_cntcurr = _avr_timer_M;
	SREG |= 0x80;
}
void TimerOff() {
	TCCR1B = 0x00; // bit3bit1bit0=000: timer off
}
void TimerISR() {
	TimerFlag = 1;
}
ISR(TIMER1_COMPA_vect) {
	_avr_timer_cntcurr--;
	if (_avr_timer_cntcurr == 0) {
		TimerISR();
		_avr_timer_cntcurr = _avr_timer_M;
	}
}
void TimerSet(unsigned long M) {
	_avr_timer_M = M;
	_avr_timer_cntcurr = _avr_timer_M;
}
//******************************************display control
#define LCD_Data_Dir DDRC	/* Define LCD data port direction */
#define LCD_Command_Dir DDRD	/* Define LCD command port direction register */
#define LCD_Data_Port PORTC	/* Define LCD data port */
#define LCD_Command_Port PORTD	/* Define LCD data port */
#define RS PD6			/* Define Register Select signal pin */
#define RW PD5			/* Define Read/Write signal pin */
#define EN PD7			/* Define Enable signal pin */


void LCD_Command(unsigned char cmnd)
{
	LCD_Data_Port= cmnd;
	LCD_Command_Port &= ~(1<<RS);	/* RS=0 command reg. */
	LCD_Command_Port &= ~(1<<RW);	/* RW=0 Write operation */
	LCD_Command_Port |= (1<<EN);	/* Enable pulse */
	_delay_us(1);
	LCD_Command_Port &= ~(1<<EN);
	_delay_ms(2);
}

void LCD_Char (unsigned char char_data)  /* LCD data write function */
{
	LCD_Data_Port= char_data;
	LCD_Command_Port |= (1<<RS);	/* RS=1 Data reg. */
	LCD_Command_Port &= ~(1<<RW);	/* RW=0 write operation */
	LCD_Command_Port |= (1<<EN);	/* Enable Pulse */
	_delay_us(1);
	LCD_Command_Port &= ~(1<<EN);
	_delay_ms(2);			/* Data write delay */
}

void LCD_Init (void)			/* LCD Initialize function */
{
	LCD_Command_Dir = 0xFF;		/* Make LCD command port direction as o/p */
	LCD_Data_Dir = 0xFF;		/* Make LCD data port direction as o/p */
	_delay_ms(20);			/* LCD Power ON delay always >15ms */
	
	LCD_Command (0x38);		/* Initialization of 16X2 LCD in 8bit mode */
	LCD_Command (0x0C);		/* Display ON Cursor OFF */
	LCD_Command (0x06);		/* Auto Increment cursor */
	LCD_Command (0x01);		/* clear display */
	_delay_ms(2);			/* Clear display command delay> 1.63 ms */
	LCD_Command (0x80);		/* Cursor at home position */
}


void LCD_String (char *str)		/* Send string to LCD function */
{
	int i;
	for(i=0;str[i]!=0;i++)		/* Send each char of string till the NULL */
	{
		LCD_Char (str[i]);
	}
}

void LCD_String_xy (char row, char pos, char *str)  /* Send string to LCD with xy position */
{
	if (row == 0 && pos<16)
	LCD_Command((pos & 0x0F)|0x80);	/* Command of first row and required position<16 */
	else if (row == 1 && pos<16)
	LCD_Command((pos & 0x0F)|0xC0);	/* Command of first row and required position<16 */
	LCD_String(str);		/* Call LCD string function */
}

void LCD_Clear()
{
	LCD_Command (0x01);		/* clear display */
	LCD_Command (0x80);		/* cursor at home position */
}


void LCD_Custom_Char (unsigned char loc, unsigned char *msg)
{
	unsigned char i;
	if(loc<8)
	{
		LCD_Command (0x40 + (loc*8));	/* Command 0x40 and onwards forces the device to point CGRAM address */
		for(i=0;i<8;i++)	/* Write 8 byte for generation of 1 character */
		LCD_Char(msg[i]);
	}
}
void LCD_Cursor(unsigned char column) {
	if ( column < 17 ) { // 16x1 LCD: column < 9
		// 16x2 LCD: column < 17
		LCD_Command(0x80 + column - 1);
		} else {
		LCD_Command(0xB8 + column - 9);	// 16x1 LCD: column - 1
		// 16x2 LCD: column - 9
	}
}
//****************************************
	unsigned char level = 2;
	unsigned char dowm[8] = { 0x4,0xa,0x11,0x11,0x11,0x11,0xa,0x4}; 
	unsigned char up[8] = {0x4,0xa,0x4,0x15,0xe,0x4,0xa,0x11};
	unsigned char jump[8] = {0x4,0xa,0x4,0x4,0x1f,0x4,0x1b,0x0};
	unsigned char x=17;
	unsigned char lose=0;
	unsigned char direbut;
	unsigned char direbut_temp;
	unsigned char hold_up=1;
enum direction_state{direinit,direup,diredowm,direleft,direright}direstate;
void snake_dire(){
	
	switch(direstate){
	case direinit:
		LCD_Custom_Char(0,dowm);
		direstate = direright;
		break;
	case direright:
		if(direbut == 0x01){
			direstate = direup;
		}
		else if(direbut == 0x02){
			direstate = diredowm;
		}
		else{
			direstate = direright;
			x++;
		}
		break;
	case direleft:
	if(direbut == 0x01){
		direstate = direup;
	}
	else if(direbut == 0x02){
		direstate = diredowm;
	}
	else{
		direstate = direleft;
		x--;
	}
	break;
	case direup:
		if(direbut == 0x04){
			direstate = direright;
		}
		else if(direbut == 0x08){
			direstate = direleft;
		}
		else{
			direstate = direup;}
		break;		
	case diredowm:
	if(direbut == 0x04){
		direstate = direright;
	}
	else if(direbut == 0x08){
		direstate = direleft;
	}
	else{
	direstate = diredowm;}
	break;
	}
	
	
	switch(direstate){
		case direinit:
			break;
		case direright:
			LCD_Cursor(x);
			LCD_Char(0);
			break;
		case direleft:
			LCD_Cursor(x);
			LCD_Char(0);
			break;
		case direup:
			if(hold_up==1){				
				LCD_Custom_Char(0, up);
				hold_up =2;
			}
			else if(hold_up == 2){
				x = x-16;
				level = 1;
				LCD_Custom_Char(0, dowm);
				hold_up = 1;
			}
			LCD_Cursor(x);
			LCD_Char(0);
			break;
		case diredowm:
		if(hold_up==2){
			LCD_Custom_Char(0, dowm);
			hold_up=1;
		}
		else if(hold_up == 1){
			x = x+16;
			level = 2;
			LCD_Custom_Char(0, up);
			hold_up = 2;
		}
		LCD_Cursor(x);
		LCD_Char(0);
		break;
	}
}

void check_border(){
	if(x<0 || x>32){
		lose = 1;
	}
	else if(level == 1 && x>16){
		lose =1;
	}
	else if(level == 2 && x<17){
		lose = 1;
	}
	if(lose == 1){
		LCD_Clear();
		LCD_Command(0x80);
		LCD_String("you lose");
	}
}
unsigned char geneposition=0;
unsigned char score;
unsigned check_hit;
void gene_score(){
	unsigned char gene[4]={0x03,0x13,0x0A,0x19};
	
	if(geneposition<4){LCD_Cursor(gene[geneposition]);LCD_Char(1);}
	       
	            //把预定的点打出来
	if(gene[geneposition]==x && hold_up == 2){
		score ++;
		geneposition++;
	}							//判断分数	
}

//***************************
void set_PWM(double frequency) {
	static double current_frequency; // Keeps track of the currently set frequency
	// Will only update the registers when the frequency changes, otherwise allows
	// music to play uninterrupted.
	if (frequency != current_frequency) {
		if (!frequency) { TCCR3B &= 0x08; } //stops timer/counter
		else { TCCR3B |= 0x03; } // resumes/continues timer/counter
		// prevents OCR3A from overflowing, using prescaler 64
		// 0.954 is smallest frequency that will not result in overflow
		if (frequency < 0.954) { OCR3A = 0xFFFF; }
		// prevents OCR0A from underflowing, using prescaler 64					// 31250 is largest frequency that will not result in underflow
		else if (frequency > 31250) { OCR3A = 0x0000; }
		// set OCR3A based on desired frequency
		else { OCR3A = (short)(8000000 / (128 * frequency)) - 1; }
		TCNT3 = 0; // resets counter
		current_frequency = frequency; // Updates the current frequency
	}
}

void PWM_on() {
	TCCR3A = (1 << COM3A0 );
	// COM3A0: Toggle PB3 on compare match between counter and OCR0A
	TCCR3B = (1 << WGM32) | (1 << CS31) | (1 << CS30);
	// WGM02: When counter (TCNT0) matches OCR0A, reset counter
	// CS01 & CS30: Set a prescaler of 64
	set_PWM(0);
}


void PWM_init() {
	TCCR3A = 0x00;
	TCCR3B = 0x00;
}
//****************shift register
 
