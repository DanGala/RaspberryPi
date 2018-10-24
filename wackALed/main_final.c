/*
 * main.c
 */
#include "wiringPi.h"
#include "fsm.h"
#include "tmr.h"
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <time.h>
#include <signal.h>
#include <sys/time.h>

#define GPIO_LED_SE 0
#define GPIO_LED_1 1
#define GPIO_LED_2 2
#define GPIO_LED_3 3
#define GPIO_LED_4 4
#define GPIO_BUTTON_SE 16
#define GPIO_BUTTON_1 19
#define GPIO_BUTTON_2 20
#define GPIO_BUTTON_3 21
#define GPIO_BUTTON_4 26

#define FLAG_BUTTON_SE 0x01
#define FLAG_BUTTON_1 0x02
#define FLAG_BUTTON_2 0x04
#define FLAG_BUTTON_3 0x08
#define FLAG_BUTTON_4 0x10
#define FLAG_LED_1 0x20
#define FLAG_LED_2 0x40
#define FLAG_LED_3 0x80
#define FLAG_LED_4 0x100
#define FLAG_TIMER 0x200
#define FLAG_LED_SE 0x400

#define NLEDS 4
#define CLK_MS 500
#define ROUNDS 10
#define BTN_FAIL_MAX 3
#define PENALTY 3000 //ms
#define TIMEOUT 5000 //ms
#define MILLION 1000000

enum fsm_state{
	WAIT_START,
	WAIT_PUSH,
	WAIT_END,
	EXCTN_WAIT_START,
};

int flags=0;
int mistakes=0;
int rounds=0;
float timing[ROUNDS];

void button_se_isr (void){ flags |= FLAG_BUTTON_SE; };
void button_1_isr (void){ flags |= FLAG_BUTTON_1; };
void button_2_isr (void){ flags |= FLAG_BUTTON_2; };
void button_3_isr (void){ flags |= FLAG_BUTTON_3; };
void button_4_isr (void){ flags |= FLAG_BUTTON_4; };
void timer_isr (union sigval value) { flags |= FLAG_TIMER; };

void cleanUp(){
	flags &= ~(FLAG_TIMER);
	digitalWrite(GPIO_LED_SE, 0);
	flags &= ~(FLAG_LED_SE | FLAG_BUTTON_SE);
	digitalWrite(GPIO_LED_1,0);
	flags &= ~(FLAG_LED_1 | FLAG_BUTTON_1);
	digitalWrite(GPIO_LED_2,0);
	flags &= ~(FLAG_LED_2 | FLAG_BUTTON_2);
	digitalWrite(GPIO_LED_3,0);
	flags &= ~(FLAG_LED_3 | FLAG_BUTTON_3);
	digitalWrite(GPIO_LED_4,0);
	flags &= ~(FLAG_LED_4 | FLAG_BUTTON_4);
}

void fillArray(){
	int i;
	int min = TIMEOUT*1000 + PENALTY*1000;
	for(i=0; i<ROUNDS; i++){
		if(timing[i]<min && timing[i]!=0) min=timing[i];
	}
	for(i=0; i<ROUNDS; i++){
		if(timing[i]==0) timing[i]=min;
	}
}

int pressed_se (fsm_t* this) {
	return (flags & FLAG_BUTTON_SE);
}

// Returns 1 if the button pressed matches the LED switched on
int pressed_ok (fsm_t* this) { 
	if(flags&FLAG_LED_1){
			if(flags&FLAG_BUTTON_1) return 1;
	}else if(flags&FLAG_LED_2){
			if(flags&FLAG_BUTTON_2) return 1;
	}else if(flags&FLAG_LED_3){
			if(flags&FLAG_BUTTON_3) return 1;
	}else if(flags&FLAG_LED_4){
			if(flags&FLAG_BUTTON_4) return 1;
	}
	return 0;
}

// Returns 1 if the button pressed doesn't match the LED switched on
int pressed_fail (fsm_t* this) { 
	if(flags&FLAG_TIMER) return 1;
	if(flags&FLAG_LED_1){
		if((flags&FLAG_BUTTON_2) || (flags&FLAG_BUTTON_3) || (flags&FLAG_BUTTON_4)) return 1;
	}else if(flags&FLAG_LED_2){
		if((flags&FLAG_BUTTON_1) || (flags&FLAG_BUTTON_3) || (flags&FLAG_BUTTON_4)) return 1;
	}else if(flags&FLAG_LED_3){
		if((flags&FLAG_BUTTON_1) || (flags&FLAG_BUTTON_2) || (flags&FLAG_BUTTON_4)) return 1;
	}else if(flags&FLAG_LED_4){
		if((flags&FLAG_BUTTON_1) || (flags&FLAG_BUTTON_2) || (flags&FLAG_BUTTON_3)) return 1;
	}
	return 0;
}

// Returns 1 if the number of rounds or the number of maximum mistakes are reached
int game_over (fsm_t* this) { 
	if(mistakes>BTN_FAIL_MAX) fillArray();
	if(timing[ROUNDS-1]!=0){
		return 1;
	}else return 0;
}

void delay_until(unsigned int next){
	unsigned int now = millis();
	if(next > now){
		delay(next-now);
	}
}

void switchOnRandom(int NumLEDS, fsm_t* this){
	int seed = rand();
	srand((unsigned int)(seed));
	int r = rand();
	if(r<RAND_MAX/4){
		flags |= FLAG_LED_1;
		digitalWrite(GPIO_LED_1, 1);
	}else if (r>RAND_MAX/4 && r<RAND_MAX/2){
		flags |= FLAG_LED_2;
		digitalWrite(GPIO_LED_2, 1);
	}else if(r>RAND_MAX/2 && r<(RAND_MAX/4)*3){
		flags |= FLAG_LED_3;
		digitalWrite(GPIO_LED_3, 1);
	}else if(r>(RAND_MAX/4)*3){
		flags |= FLAG_LED_4;
		digitalWrite(GPIO_LED_4, 1);
	}
  // Update round counter
	rounds++;
  // Reactivate the timer
	tmr_startms((tmr_t*)(this->user_data), TIMEOUT);
}

void S1E1(fsm_t* this){
  // Switch off active LEDs and clear flags
	cleanUp();
  // Reset timings array & round's and mistakes' counters
	int i;
	for(i=0; i<ROUNDS; i++){
		timing[i]=0;
	}
	rounds = 0;
	mistakes = 0;
  // Call Random LED function
	switchOnRandom(NLEDS, this);
}

void S2E2(fsm_t* this){
  // Deactivate the timer and get the remaining time until timeout
	float t = tmr_halt((tmr_t*)(this->user_data));
  // Switch off active LEDs and clear flags
	cleanUp();
  // Store the users's button-pressing timing value in milliseconds 
	timing[rounds-1] = TIMEOUT*1000 - t;
  // If game is not over yet, start next round
	if(rounds<ROUNDS) switchOnRandom(NLEDS, this);
}

void S2E3(fsm_t* this){
  // Deactivate the timer and get the remaining time until timeout
	float t = tmr_halt((tmr_t*)(this->user_data));
  // Switch off active LEDs and clear flags
	cleanUp();
  // Increase the mistakes counter's value
	mistakes++;
  // Store timing + penalty
	timing[rounds-1] = TIMEOUT*1000 - t + PENALTY*1000;
  // If game is not over yet, start next round
	if((rounds<ROUNDS)&&(mistakes<=BTN_FAIL_MAX)) switchOnRandom(NLEDS, this);
}

void S2E4(fsm_t* this){
  // Deactivate the timer
	tmr_halt((tmr_t*)(this->user_data));
  // Switch of active LEDs and clear flags
	cleanUp();
  // Switch on the LED_SE
	digitalWrite(GPIO_LED_SE, 1);
}

/*void S2E5(fsm_t* this){

}*/

void S3E1(fsm_t* this){
	float tMean=0;
	float tMax=0;
	float tMin=timing[0];
  // Calculate mean time
	int i;
	for(i=0; i<ROUNDS; i++){
		tMean += timing[i];
	}
	tMean /= ROUNDS;
  // Calculate max time
	for(i=0; i<ROUNDS; i++){
		if(timing[i]>tMax) tMax=timing[i];
	}
  // Calculate min time
	for(i=0; i<ROUNDS; i++){
		if(timing[i]<tMin) tMin=timing[i];
	}
  // Display results
	printf("Mean time was: %7.6f s\n", tMean/MILLION);
	printf("Maximum time was: %7.6f s\n", tMax/MILLION);
	printf("Minimum time was: %7.6f s\n", tMin/MILLION);
	printf("Number of mistakes was: %d\n", mistakes);
  // Switch off active LEDs and clear flags
	cleanUp();
	digitalWrite(GPIO_LED_SE, 1);
}


// Transition matrix {OriginState, Guard, EndState, OutputFunction}
fsm_trans_t reflexes[] = {
		{WAIT_START, pressed_se, WAIT_PUSH, S1E1},
		{WAIT_PUSH, pressed_ok, WAIT_PUSH, S2E2},
		{WAIT_PUSH, pressed_fail, WAIT_PUSH, S2E3},
		{WAIT_PUSH, game_over, WAIT_END, S2E4},
		{WAIT_END, pressed_se, WAIT_START, S3E1},
		{-1, NULL, -1, NULL},
};

int main ()
{
	tmr_t* tmr = tmr_new(timer_isr);
	fsm_t* reflexes_fsm = fsm_new (WAIT_START, reflexes, tmr);

	//Configuration
	wiringPiSetupGpio();
	pinMode(GPIO_BUTTON_SE,INPUT);
	pinMode(GPIO_BUTTON_1,INPUT);
	pinMode(GPIO_BUTTON_2,INPUT);
	pinMode(GPIO_BUTTON_3,INPUT);
	pinMode(GPIO_BUTTON_4,INPUT);
	pinMode(GPIO_LED_SE,OUTPUT);
	pinMode(GPIO_LED_1,OUTPUT);
	pinMode(GPIO_LED_2,OUTPUT);
	pinMode(GPIO_LED_3,OUTPUT);
	pinMode(GPIO_LED_4,OUTPUT);
	digitalWrite(GPIO_LED_SE, 1);
	flags |= FLAG_LED_SE;
	digitalWrite(GPIO_LED_1, 0);
	digitalWrite(GPIO_LED_2, 0);
	digitalWrite(GPIO_LED_3, 0);
	digitalWrite(GPIO_LED_4, 0);
	wiringPiISR(GPIO_BUTTON_SE, INT_EDGE_FALLING, button_se_isr);
	wiringPiISR(GPIO_BUTTON_1, INT_EDGE_FALLING, button_1_isr);
	wiringPiISR(GPIO_BUTTON_2, INT_EDGE_FALLING, button_2_isr);
	wiringPiISR(GPIO_BUTTON_3, INT_EDGE_FALLING, button_3_isr);
	wiringPiISR(GPIO_BUTTON_4, INT_EDGE_FALLING, button_4_isr);

	unsigned int next = millis();
	while(1){
		fsm_fire(reflexes_fsm);
		next += CLK_MS;
		delay_until(next);
	}
	tmr_destroy((tmr_t*)(reflexes_fsm->user_data));
	fsm_destroy(reflexes_fsm);

	return 0;
}
