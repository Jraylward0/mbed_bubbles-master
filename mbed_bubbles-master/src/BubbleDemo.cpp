/******************************************************************************
 * Includes
 *****************************************************************************/

#include<mbed.h>
#include<LcdController.h>
#include<EaLcdBoard.h>
#include<wchar.h>
#include"BubbleDemo.h"


/******************************************************************************
 * Typedefs and defines
 *****************************************************************************/

#define PI 3.1415926535897932384626433832795

/* Red color mask, 565 mode */
#define REDMASK       0xF800
/* Red shift value, 565 mode */
#define REDSHIFT      11
/* Green color mask, 565 mode */
#define GREENMASK     0x07E0
/* Green shift value, 565 mode */
#define GREENSHIFT    5
/* Blue color mask, 565 mode */
#define BLUEMASK      0x001F
/* Blue shift value, 565 mode */
#define BLUESHIFT     0

/* Number of colors in 565 mode */
#define NUM_COLORS    65536
/* Number of red colors in 565 mode */
#define RED_COLORS    0x20
/* Number of green colors in 565 mode */
#define GREEN_COLORS  0x40
/* Number of blue colors in 565 mode */
#define BLUE_COLORS   0x20

/******************************************************************************
 * Local variables
 *****************************************************************************/


/******************************************************************************
 * External variables
 *****************************************************************************/

/******************************************************************************
 * Local functions
 *****************************************************************************/


void BubbleDemo::initialize() {
 float radian;
 const float phase1 = 2 * PI / 3;
 const float phase2 = 4 * PI / 3;

 for(i = 0; i < NumBalls; i++) {
  x[i] = this->windowX / 2;
  y[i] = this->windowY / 2;
  r[i] = i * 2 + 10;

  oldX[i] = x[i];
  oldY[i] = y[i];

  velX[i] = 1;
  velY[i] = 1;

  radian = i * 2 * PI / NumBalls;
  red[i] = cos(radian) * RED_COLORS / 2 + (RED_COLORS / 2 - 1);
  green[i] = cos(radian + phase2) * GREEN_COLORS / 2 + (GREEN_COLORS / 2 - 1);
  blue[i] = cos(radian + phase1) * BLUE_COLORS / 2 + (BLUE_COLORS / 2 - 1);
 }
}

void BubbleDemo::collision() {
 float disX = x[j] - x[i];
 float disY = y[j] - y[i];
 float d2 = disX * disX + disY * disY;

 if(d2 != 0) {
  float rij = r[i] + r[j];
  float rij2 = rij * rij;

  if(d2 < rij2) {
   float ii = (disX * velX[i] + disY * velY[i]) / d2;
   float ji = (disX * velY[i] - disY * velX[i]) / d2;
   float ij = (disX * velX[j] + disY * velY[j]) / d2;
   float jj = (disX * velY[j] - disY * velX[j]) / d2;
   float ratio = rij / sqrt(d2);

   velX[i] = ij * disX - ii * disY;
   velY[i] = ij * disY + ii * disX;
   velX[j] = ji * disX - jj * disY;
   velY[j] = ji * disY + jj * disX;

   disX *= (ratio - 1) / 2;
   disY *= (ratio - 1) / 2;

   x[j] += disX;
   y[j] += disY;
   x[i] -= disX;
   y[i] -= disY;
  }
 }
}

void BubbleDemo::borders() {
 if(x[i] >= this->windowX - r[i] - 1) {
  x[i] = this->windowX - r[i] - 1;
  velX[i] = -velX[i];
 } else if(x[i] <= r[i]) {
  x[i] = r[i];
  velX[i] = -velX[i];
 }

 if(y[i] >= this->windowY - r[i] - 1) {
  y[i] = this->windowY - r[i] - 1;
  velY[i] = -velY[i];
 } else if(y[i] <= r[i]) {
  y[i] = r[i];
  velY[i] = -velY[i];
 }
}

void BubbleDemo::draw() {
 graphics.put_circle( oldX[i], oldY[i], 0, r[i], 0 );
 graphics.put_circle( x[i], y[i], (red[i] << REDSHIFT) + (green[i] << GREENSHIFT) + (blue[i] << BLUESHIFT), r[i], 0);

 oldX[i] = x[i];
 oldY[i] = y[i];
}


/******************************************************************************
 * Public functions
 *****************************************************************************/
 
BubbleDemo::BubbleDemo(uint8_t *pFrameBuf, uint16_t dispWidth, uint16_t dispHeight) 
    : graphics((uint16_t *)pFrameBuf, dispWidth, dispHeight) {

    this->windowX = dispWidth;
    this->windowY = dispHeight;
    this->pFrmBuf  = (uint16_t *)pFrameBuf;
    this->pFrmBuf1 = (uint16_t *)pFrameBuf;
    this->pFrmBuf2 = (uint16_t *)((uint32_t)pFrameBuf + dispWidth*dispHeight*2);
    this->pFrmBuf3 = (uint16_t *)((uint32_t)pFrameBuf + dispWidth*dispHeight*4);

    initialize();
}

void BubbleDemo::run(EaLcdBoardGPIO& lcdBoard, uint32_t loops, uint32_t delayMs) {
  
  printf("BubbleDemo, %d loops, %dms delay\n", loops, delayMs);

    //update framebuffer
    graphics.setFrameBuffer(this->pFrmBuf);
    lcdBoard.setFrameBuffer((uint32_t)this->pFrmBuf);
    memset((void*)(pFrmBuf), 0, this->windowX * this->windowY * 2);

    for(int32_t n=0;n<loops;n++) {

        for(i = 0; i < NumBalls; i++) {
            x[i] += velX[i];
            y[i] += velY[i];
            
            for(j = i + 1; j < NumBalls; j++)
                collision();
                
            borders();
            
            if((int)x[i] != (int)oldX[i] || (int)y[i] != (int)oldY[i])
                draw();
        }

        wait_ms(delayMs);
    }
    memset((void*)(pFrmBuf), 0, this->windowX * this->windowY * 2);
}

