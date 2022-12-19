#include <stdint.h>
#include "SysTick.h"
#include "msp432p401r.h"
#include "myheader.h"
#include <stdio.h>



//Engineer: Anthony Ruiz
//PART 2: addition of a walk functionality, this adds an extra walk/dont walk LED output and a walk button input
// this builds upon part 1:

// FSM Traffic Light: north/west intersection revolving around two traffic lights go north/west
// this FSM cycles through inputs depending on button press logic and a timer.


               //variables under state structure
struct State {
       uint32_t Out_veh;
       uint32_t Out_ped;
       //uint32_t statechecker;

       uint32_t Time;
       const struct State* Next[8]; //contains staes
        };

typedef const struct State S_Type_struc; //state structure

#define goN           &FSM[0] //state 0 in FSM
#define waitN         &FSM[1] //state 1 in FSM
#define goW           &FSM[2] //state 2 in FSM
#define waitW         &FSM[3] //state 3 in FSM
#define walkN_press   &FSM[4] //state 3 in FSM
#define walkN_release &FSM[5] //state 3 in FSM
#define walkW_press   &FSM[6] //state 3 in FSM
#define walkW_release &FSM[7] //state 3 in FSM
#define go_walk       &FSM[8] //state 3 in FSM
#define walk_flash1   &FSM[9] //state 3 in FSM
#define walk_flash2   &FSM[10]//state 3 in FSM
#define walk_flash3   &FSM[11]//state 3 in FSM
#define walk_flash4   &FSM[12]//state 3 in FSM
#define walk_flash5   &FSM[13]//state 3 in FSM
#define walk_flash6   &FSM[14]//state 3 in FSM
#define dont_walk     &FSM[15]//state 3 in FSM



#define SENSOR     *((volatile uint8_t *)(0x40004C40)) //------>sensor address port 5 input address
#define PED_SENSOR *((volatile uint8_t *)(0x40004C00)) //------->port1 input address
#define LIGHT      *((volatile uint8_t *)(0x40004C23)) //------->light address port 4 output address
#define WLK_LED    *((volatile uint8_t *)(0x40004C03))//------>walk light address port 2 output address

            //structure
            S_Type_struc FSM[16] = {
                    //out_veh, out_ped,  time,{[0], [1],  [2],   [3],... }
                                    {0x21, 0x01, 300, {goN, goN, waitN, waitN, walkN_press, walkN_press, walkN_press, walkN_press}}, // goS
                                    {0x11, 0x01, 100, {goW, goW, goW, goW, goW, goW, goW, goW}}, // waitS

                                    {0x0A, 0x01, 300, {goW, waitW, goW, waitW, walkW_press, walkW_press, walkW_press, walkW_press}}, // goW
                                    {0x0C, 0x01, 100, {goN, goN, goN, goN, goN, goN, goN, goN}}, // waitW

                                    {0x21, 0x01, 200, {goN, goN, goN, goN, walkN_release, walkN_release, walkN_release, walkN_release}}, // walkPressS
                                    {0x11, 0x01, 100, {go_walk, go_walk, go_walk, go_walk, go_walk, go_walk, go_walk, go_walk}}, // walkReleaseS
                                    {0x0A, 0x01, 200, {goW, goW, goW, goW, walkW_release, walkW_release, walkW_release, walkW_release}}, // walkPressW
                                    {0x0C, 0x01, 100, {go_walk, go_walk, go_walk, go_walk, go_walk, go_walk, go_walk, go_walk}}, // walkReleaseW

                                    // PEDESTRIAN states
                                    {0x09, 0x02, 200, {walk_flash1, walk_flash1, walk_flash1, walk_flash1, walk_flash1, walk_flash1, walk_flash1, walk_flash1}}, // walkGo
                                    {0x09, 0x01, 50,  {walk_flash2, walk_flash2, walk_flash2, walk_flash2, walk_flash2, walk_flash2, walk_flash2, walk_flash2}}, // walkFlash1
                                    {0x09, 0x00, 50,  {walk_flash3, walk_flash3, walk_flash3, walk_flash3, walk_flash3, walk_flash3, walk_flash3, walk_flash3}}, // walkFlash2
                                    {0x09, 0x01, 50,  {walk_flash4, walk_flash4, walk_flash4, walk_flash4, walk_flash4, walk_flash4, walk_flash4, walk_flash4}}, // walkFlash3
                                    {0x09, 0x00, 50,  {walk_flash5, walk_flash5, walk_flash5, walk_flash5, walk_flash5, walk_flash5, walk_flash5, walk_flash5}}, // walkFlash4
                                    {0x09, 0x01, 50,  {walk_flash6, walk_flash6, walk_flash6, walk_flash6, walk_flash6, walk_flash6, walk_flash6, walk_flash6}}, // walkFlash5
                                    {0x09, 0x00, 50,  {dont_walk, dont_walk, dont_walk, dont_walk, dont_walk, dont_walk, dont_walk, dont_walk}}, // walkFlash6
                                    {0x09, 0x01, 100, {goN, goN, goW, goN, goN, goN, goW, goN, }} // walkEnd



                          };


void main(void){

    S_Type_struc *Pt;
    uint32_t Input;

init_LED();        //initialize LED               p1
init_SW();         //initialize SWITCHES/buttons  p2
init_WLK_LED();    //initialize walk LED p4
SysTick_Init();    //initialize SysTick


Pt = goN;       //default where the pointer points to
                //south green, west red

            //7654 3210
            //0011 1111
        while(1)
        {
            LIGHT = (Pt-> Out_veh)&0x3F; //outputs to LEDs
            WLK_LED = (Pt -> Out_ped)&0x03;//output walk LED
            SysTick_Wait10ms(Pt -> Time); //waits 10ms
            Input = SENSOR| (((PED_SENSOR&0x02)^0x02) << 1) ;//reads next input value
            Pt = Pt -> Next [Input&0x07]; //INPUT



        }//end of FSM while loop
}//end of main




///////////                                 //////////////
/////////// Initialization of the LED's     //////////////


void init_LED(void){
    P4SEL0 &= ~0X3F;//~0X3F;//using port 4 as an output for LED's
    P4SEL1 &= ~0X3F;
    P4DIR |= 0X3F;
    //ports P4.0-P4.5   ------   3    F -->0011 1111
    //                  -------  F    A -->1111 1010 -->7654 3210
}

///////////                                    //////////////
/////////// Initialization of the buttons     //////////////

void init_SW(void){
//initialization of two ports for button inputs
//using port 5
    P5SEL0 &= ~0X03;//port 5.0 and P5.1 are connections for button inputs
    P5SEL1 &= ~0X03; //0000 0011
    P5DIR &= ~0X03;

    P1SEL0 &= ~0x02;    //pedestrian p1.1 input
    P1SEL1 &= ~0x02;
    P1DIR &= ~0x02;
    P1REN |= 0x02;
    P1OUT |= 0x02;
}

void init_WLK_LED(void){
//initialization of the port for walk LED output
//using port 2
    P2SEL0 &= ~0X07;                                               //pins   7654 3210
    P2SEL1 &= ~0X07;                                               //binary 0000 0000
    P2DIR  |=  0X07; //port 2.0 (red)and 2.1 (green)are LED outputs            //hex    0000 0011
    P2DS   |=  0x07;

}

/*(NOTE: when initializing ports, DIR=1  means output, DIR = 0 means input)*/
