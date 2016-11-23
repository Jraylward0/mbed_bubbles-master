/*
 *  Copyright 2013 Embedded Artists AB
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */

/******************************************************************************
 * Includes
 *****************************************************************************/

#include<mbed.h>
#include<sdram.h>
#include "TestDisplay.h"
#include "BubbleDemo.h"

/******************************************************************************
 * Defines and typedefs
 *****************************************************************************/

#define LCD_CONFIGURATION_43 \
        40,                         /* horizontalBackPorch */ \
        5,                          /* horizontalFrontPorch */ \
        2,                          /* hsync */ \
        480,                        /* width */ \
        8,                          /* verticalBackPorch */ \
        8,                          /* verticalFrontPorch */ \
        2,                          /* vsync */ \
        272,                        /* height */ \
        false,                      /* invertOutputEnable */ \
        false,                      /* invertPanelClock */ \
        true,                       /* invertHsync */ \
        true,                       /* invertVsync */ \
        1,                          /* acBias */ \
        LcdController::Bpp_16_565,  /* bpp */ \
        9000000,                    /* optimalClock */ \
        LcdController::Tft,         /* panelType */ \
        false                       /* dualPanel */

#define LCD_INIT_STRING_43  (char*)"v1,cd0,c50,cc0,c30,d100,c31,d100,cd1,d10,o,c51,cc100"

#define LCD_CONFIGURATION_50 \
        46,                         /* horizontalBackPorch */ \
        20,                          /* horizontalFrontPorch */ \
        2,                          /* hsync */ \
        800,                        /* width */ \
        23,                          /* verticalBackPorch */ \
        10,                          /* verticalFrontPorch */ \
        3,                          /* vsync */ \
        480,                        /* height */ \
        false,                      /* invertOutputEnable */ \
        false,                      /* invertPanelClock */ \
        true,                       /* invertHsync */ \
        true,                       /* invertVsync */ \
        1,                          /* acBias */ \
        LcdController::Bpp_16_565,  /* bpp */ \
        30000000,                   /* optimalClock */ \
        LcdController::Tft,         /* panelType */ \
        false                       /* dualPanel */

#define LCD_INIT_STRING_50  (char*)"v1,cc0,c31,d50,o,d200,c51,cc100"

#define MY_ABS(__a)  (((__a) < 0) ? -(__a) : (__a))

/******************************************************************************
 * Public Functions
 *****************************************************************************/

/*
   Prerequisites:
 
   - A display must be connected to the LPC4088 Experiment Base Board
     with the FPC connector

   - The touch controller uses the I2C bus so for this test to work 
     jumpers JP8 and JP9 on the LPC4088 Experiment Base Board must 
     both be in positions 1-2

*/

TestDisplay::TestDisplay(WhichDisplay which) : 
    _initStr(NULL),
    _lcdCfg(NULL),
    _lcdBoard(P0_27, P0_28),
    _touch(P0_27, P0_28, P2_25) {
    
    switch (which) {
        case TFT_5:
            _lcdCfg = new LcdController::Config(LCD_CONFIGURATION_50);
            _initStr = LCD_INIT_STRING_50;
            break;
        case TFT_4_3:
            _lcdCfg = new LcdController::Config(LCD_CONFIGURATION_43);
            _initStr = LCD_INIT_STRING_43;
            break;
        default:
            mbed_die();
    }
            
    if (sdram_init() == 1) {
        printf("Failed to initialize SDRAM\n");
        _framebuffer = 0;
    } else {
        _framebuffer = (uint32_t) malloc(_lcdCfg->width * _lcdCfg->height * 2 * 3); // 2 is for 16 bit color, 3 is the number of buffers
        if (_framebuffer != 0) {
            memset((uint8_t*)_framebuffer, 0, _lcdCfg->width * _lcdCfg->height * 2 * 3);
        }
    }
}

TestDisplay::~TestDisplay() {
    if (_framebuffer != 0) {
        free((void*)_framebuffer);
        _framebuffer = 0;
    }
}

bool TestDisplay::runTest() {
    do {
        if (_framebuffer == 0) {
            printf("Failed to allocate memory for framebuffer\n");
            break;
        }
        
        EaLcdBoard::Result result = _lcdBoard.open(_lcdCfg, _initStr);
        if (result != EaLcdBoard::Ok) {
            printf("Failed to open display, error %d\n", result);
            break;
        }

        result = _lcdBoard.setFrameBuffer(_framebuffer);
        if (result != EaLcdBoard::Ok) {
            printf("Failed to set framebuffer, error %d\n", result);
            break;
        }
        
        BubbleDemo bubbleDemo((uint8_t *)_framebuffer, _lcdCfg->width, _lcdCfg->height);
        while (1) {
            bubbleDemo.run(_lcdBoard, 750, 20);
        }
    } while(0);
    
    return false;
}

void TestDisplay::calibrate_drawMarker(Graphics &g, uint16_t x, uint16_t y, bool erase) {
    uint16_t color = (erase ? 0x0000 : 0xffff);
    g.put_line(x-15, y, x+15, y, color);
    g.put_line(x, y-15, x, y+15, color);
    g.put_circle(x, y, color, 10, false);
}

bool TestDisplay::calibrate_display() {
    bool morePoints = true;
    uint16_t x, y;
    int point = 0;
    Graphics g((uint16_t*)_framebuffer, _lcdCfg->width, _lcdCfg->height);
    
    do {
        if (!_touch.init(_lcdCfg->width, _lcdCfg->height)) {
            printf("Failed to initialize touch controller\n");
            break;
        }
        if (!_touch.calibrateStart()) {
            printf("Failed to start calibration\n");
            break;
        }  
        while (morePoints) {
            if (point++ > 0) {
                // erase old location
                calibrate_drawMarker(g, x, y, true);
            }
            if (!_touch.getNextCalibratePoint(&x, &y)) {
                printf("Failed to get calibration point\n");
                break;
            }
            calibrate_drawMarker(g, x, y, false);
            if (!_touch.waitForCalibratePoint(&morePoints, 0)) {
                printf("Failed to get user click\n");
                break;
            }
        }
        if (morePoints) {
            // aborted calibration due to error(s)
            break;
        }

        // erase old location
        calibrate_drawMarker(g, x, y, true);

        // allow user to draw for 5999 seconds
        Timer t;
        t.start();
        TouchPanel::touchCoordinate_t tc;
        while(t.read() < 6000) {
            if (_touch.read(tc)) {
                //printf("TC: x,y,z = {%5d, %5d, %5d}\n", tc.x, tc.y, tc.z);
                if (tc.z) {
                    g.put_dot(tc.x, tc.y, 0xffff);
                }
            }
        }
    } while(0);
    
    return !morePoints;
}
