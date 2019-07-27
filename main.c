#include <avr/io.h>
#include "scheduler.h"
#include <avr/eeprom.h>	
#include "keypad.h"
#include "customer.c"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

void ADC_init() {
	ADCSRA |= (1 << ADEN) | (1 << ADSC) | (1 << ADATE);
}
unsigned short adc_x;
 
unsigned char showingpo[5]={1,2,17,25,35};
unsigned char showingpo_temp[5]={1,2,17,25,35};
unsigned char trackingpo[5]={16,16,16,16,16};
unsigned char trackingpo_temp[5]={16,16,16,16,16};
unsigned char number0fblocks=0;
unsigned char showuptime=1;
unsigned char firstblock=1;
//************
unsigned char showingpo_do[5]={5,11,20,22,35};
unsigned char showingpo_temp_do[5]={5,11,20,22,35};
unsigned char trackingpo_do[5]={32,32,32,32,32};
unsigned char trackingpo_temp_do[5]={32,32,32,32,32};
unsigned char number0fblocks_do=0;
unsigned char firstblock_do=1;
unsigned char press;
//**************
unsigned char i;
unsigned char input_hold='D';
unsigned char up_dowm = 2;
unsigned char player_po = 17;
unsigned char score_time;
unsigned char control_flash =0 ;
unsigned char ten;
unsigned char one;
unsigned char higestten;
unsigned char higestone;
//**************music 
unsigned char music_i;
unsigned char music_change;
unsigned char song1[8]={'A','H','B','G','C','F','D','E'};
unsigned char song2[8]="CGEBHECB";
unsigned char song[8];
const double notes[26] = {329.63, 329.63, 329.63, 329.63, 329.63, 329.63, 329.63, 392.00, 261.63, 293.66, 329.63, 349.23, 349.23, 349.23, 349.23, 349.23, 329.63, 329.63, 329.63, 329.63, 329.63, 293.66, 293.66, 329.63, 293.66, 392.00 };
void music_transfer(){
	if(song[music_i] == 'A'){set_PWM(4440.0);}
	else if(song[music_i] == 'B'){set_PWM(2616.3);}
	else if(song[music_i] == 'C'){set_PWM(5232.5);}
	else if(song[music_i] == 'D'){set_PWM(2936.6);}
	else if(song[music_i] == 'E'){set_PWM(3296.3);}
	else if(song[music_i] == 'F'){set_PWM(3492.3);}
	else if(song[music_i] == 'G'){set_PWM(3920.0);}
	else if(song[music_i] == 'H'){set_PWM(5232.5);}
	
}
enum music_state{music_menu,music_play,music_winlose};
int music_tick(int state){
	switch(state){
		case music_menu:
			if(input_hold == 'A'){
				for(music_change=0;music_change<8;music_change++){song[music_change]=song1[music_change];}
				state = music_play;
				music_i = 0;
			}
			break;
		case music_play:
			if(input_hold == 'D'){
				state = music_menu;
				music_i = 0;
			}
			else if(input_hold == 's'){
				for(music_change=0;music_change<8;music_change++){song[music_change]=song2[music_change];}
				state = music_winlose;
				music_i = 0;
			}
			break;
		case music_winlose:
			if(input_hold == 'D'){
				state = music_menu;
				music_i = 0;
			}
			break;
	}
	switch(state){
		case music_menu:
			set_PWM(notes[music_i]*10);
			music_i++;
			if(music_i==26){music_i = 0;}
			
			break;
		case music_play:
			/*music_transfer();
			music_i++;
			if(music_i==8){music_i = 0;}*/
				set_PWM(0);
				if(adc_x >= 950 && input_hold == 'A'){set_PWM(329.63);}
				else if(adc_x <= 50 && input_hold == 'A'){set_PWM(5232.5);}
			break;
		case music_winlose:
			music_transfer();
			music_i++;
			if(music_i==8){music_i = 0;}
			break;
	}
	return state;
}	
//**************
unsigned char highestscore;
enum score_state{scoreinit};
int score_tick(int state){	
	eeprom_write_byte((uint8_t*)1,showuptime);
	score_time = eeprom_read_byte((uint8_t*)1);
	
	highestscore = eeprom_read_byte((uint8_t*)2);
	if(score_time > highestscore){
		eeprom_write_byte((uint8_t*)2,showuptime);
	}
	highestscore = eeprom_read_byte((uint8_t*)2);
	switch (state){
		
		case scoreinit:
		ten = score_time /10;
		one = score_time %10;
		higestten =  highestscore/10;
		higestone = highestscore %10;
	if((player_po == trackingpo[firstblock-1] || player_po == trackingpo_do[firstblock_do-1])&&control_flash == 0){
		input_hold = 's';
		LCD_Clear();
		LCD_Command(0x80);
		LCD_String("lose  highest:");
		LCD_Char(higestten+'0');
		LCD_Char(higestone+'0');
		LCD_Cursor(17);
		LCD_String("score: ");
		control_flash = 1;
		LCD_Char(ten+'0');
		LCD_Char(one+'0');
	}
	if(score_time>48 && control_flash == 0){
		input_hold = 's';
		LCD_Clear();
		LCD_Command(0x80);
		LCD_String("win  highest:");
		LCD_Char(higestten+'0');
		LCD_Char(higestone+'0');
		LCD_Cursor(17);
		LCD_String("score: ");
		control_flash = 1;
		LCD_Char(ten+'0');
		LCD_Char(one+'0');
	}
	state = scoreinit;
	break;}
	return state;
}
enum player_state{playerstart};
int player_tick(int state){
	switch(state){
		case playerstart:
			if(input_hold == 'A'){
				if(up_dowm == 2 && player_po>16){player_po = player_po - 16;up_dowm = 3;}
				if(up_dowm == 1 && player_po<17){player_po = player_po +16;up_dowm = 3;}
				
			LCD_Cursor(player_po);
			if(showuptime%2 == 1){LCD_Char(2);}
			else{LCD_Char(1);}}
			state = playerstart;
			break;
	}
	return state;
}
enum getinput_state{getinit,getpress};
int getinput_tick(int state){
	press = ~PINA & 0x06;
	switch(state){
		case getinit:
			if(press){
				state =getpress;
				if(press == 2 && input_hold != 's'){input_hold = 'A';}
				else if(press == 4){input_hold = 'D';}
				
			}
			if(adc_x >= 950){up_dowm = 2;}
			if(adc_x <= 50){up_dowm = 1;}
			else{state = getinit;}break;
		case getpress:
			if(press){state = getpress;}
			else{state = getinit;}
			break;
	}
	return state;
}
enum menuNreset{menu,start,lose_win};
int menu_tick(int state){
	switch(state){
		case menu:
			if(input_hold == 's'){state = lose_win;}
			else if(input_hold == 'A'){
				state = start;
			}
			else{
				state = menu;
			}
			
			break;
		case start:
			if(input_hold == 's'){state = lose_win;}
			else if(input_hold == 'D'){
				state = menu;
				LCD_Clear();
			}
			else{
				state = start;
			}break;
		case lose_win:
			if(input_hold == 's'){state = lose_win;}
			else if(input_hold == 'D'){
				state = menu;
				LCD_Clear();
			}
			else{
				state = start;
			}break;
			break;
	}
	unsigned char z=0; 
	switch(state){
		case menu:
			
//*************************************reset
			
			for(z=0;z<5;z++){showingpo[z]=showingpo_temp[z];}
			for(z=0;z<5;z++){trackingpo[z]=trackingpo_temp[z];}
//*********do
			for(z=0;z<5;z++){showingpo_do[z]=showingpo_temp_do[z];}
			for(z=0;z<5;z++){trackingpo_do[z]=trackingpo_temp_do[z];}
//******
			 number0fblocks=0;
			 number0fblocks_do=0;
			 showuptime=1;
			 firstblock=1;
			firstblock_do=1;
			control_flash = 0;
//************************************reset everything
			 LCD_Command(0x80);
			LCD_String("press A to begin ");
			
			break;
		case start:
			break;
		case lose_win:
			break;
	}
	return state;
}

enum screen_roll{init};
int display1(int state){
	switch(state){
		case init:
		if(input_hold == 'A'){
			if(showuptime == showingpo[number0fblocks]){
				number0fblocks++;
			}
			
			LCD_Clear();
			i=firstblock;
			for(i;i<=number0fblocks;i++){
				LCD_Cursor(trackingpo[i-1]);
				LCD_Char(0);
				trackingpo[i-1]=trackingpo[i-1]-1;
				if(trackingpo[i-1]==0){firstblock++;}
			}
		//**********
			if(showuptime == showingpo_do[number0fblocks_do]){
				number0fblocks_do++;
			}
			showuptime++;
			i=firstblock_do;
			for(i;i<=number0fblocks_do;i++){
				LCD_Cursor(trackingpo_do[i-1]);
				LCD_Char(0);
				trackingpo_do[i-1]=trackingpo_do[i-1]-1;
				if(trackingpo_do[i-1]==16){firstblock_do++;}
			}
		//**********
		}
			state = init;		
			break;
	}
	return state;
}

int main(void) {
	DDRA = 0x00; PORTA = 0xFF;
	DDRB = 0xFF; PORTB = 0x00;
	DDRC = 0xFF; PORTC = 0x00;
	DDRD = 0xFF; PORTD = 0x00;
	ADC_init();
	LCD_Init();
	PWM_init();
	PWM_on();
	LCD_Custom_Char(0,dowm);
	LCD_Custom_Char(1,up);
	LCD_Custom_Char(2,jump);
	if(eeprom_read_byte((uint8_t*)2)<1 || eeprom_read_byte((uint8_t*)2)>70){eeprom_write_byte((uint8_t*)2,1);}
	
	static task task1,task2,task3,task4,task5,task6;
	task *tasks[] = {&task1,&task2,&task3,&task4,&task5,&task6};
	const unsigned short numTasks = sizeof(tasks)/sizeof(task*);
		
	task1.state = init;//Task initial state
	task1.period = 70;//Task Period
	task1.elapsedTime = task1.period;//Task current elapsed time.
	task1.TickFct = &display1;//Function pointer for the tick

	task2.state = getinit;//Task initial state
	task2.period = 1;//Task Period
	task2.elapsedTime = task2.period;//Task current elapsed time.
	task2.TickFct = &getinput_tick;//Function pointer for the tick

	task3.state = menu;//Task initial state
	task3.period = 70;//Task Period
	task3.elapsedTime = task3.period;//Task current elapsed time.
	task3.TickFct = &menu_tick;//Function pointer for the tick

	task4.state = playerstart;//Task initial state
	task4.period = 70;//Task Period
	task4.elapsedTime = task4.period;//Task current elapsed time.
	task4.TickFct = &player_tick;//Function pointer for the tick	
	
	task5.state = scoreinit;//Task initial state
	task5.period = 20;//Task Period
	task5.elapsedTime = task5.period;//Task current elapsed time.
	task5.TickFct = &score_tick;//Function pointer for the tick

	task6.state = music_menu;//Task initial state
	task6.period = 30;//Task Period
	task6.elapsedTime = task6.period;//Task current elapsed time.
	task6.TickFct = &music_tick;//Function pointer for the tick	
	
	TimerSet(1);
	TimerOn();

	unsigned short i;//Scheduler for-loop iterator
	while (1) {
		unsigned char reset_eeprom;
		reset_eeprom = ~PINA & 0x08;
		if(reset_eeprom ){eeprom_write_byte((uint8_t*)2,1);}
		ADC_init();
		adc_x= ADC;
		for(i = 0; i < numTasks; i++){//Scheduler code
			if(tasks[i]->elapsedTime == tasks[i]->period){//Task is ready to tick
				tasks[i]->state= tasks[i]->TickFct(tasks[i]->state);//set next state
				tasks[i]->elapsedTime = 0;//Reset the elapsed time for next tick;
			}
			tasks[i]->elapsedTime += 1;
			
		}
		while(!TimerFlag);
		TimerFlag = 0;
	}
	return 1;
}