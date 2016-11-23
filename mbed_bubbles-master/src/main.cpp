/******************************************************************************
 * Includes
 *****************************************************************************/
#include<mbed.h>

#include"TestDisplay.h"

/******************************************************************************
 * Typedefs and defines
 *****************************************************************************/


/******************************************************************************
 * Local variables
 *****************************************************************************/

DigitalOut myled(LED1);

/******************************************************************************
 * Local functions
 *****************************************************************************/

/*
 * Test program for the 4.3" and 5" displays. This test is supposed to run
 * on a LPC4088QSB board on an LPC4088 Experiment Base Board.
 */


int main() {
    printf("\n"
           "---\n"
           "Display Demo Program for 4.3 and 5 inch display on the LPC4088 Experiment Base Board\n"
           "Build Date: " __DATE__ " at " __TIME__ "\n"
           "\n");

    //TestDisplay display(TestDisplay::TFT_4_3);
    TestDisplay display(TestDisplay::TFT_5);
    display.runTest();
    
    while(1) {
        myled = 1;
        wait(0.2);
        myled = 0;
        wait(0.2);
    }
}
