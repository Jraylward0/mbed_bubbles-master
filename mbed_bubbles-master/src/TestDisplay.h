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

#ifndef TESTDISPLAY_H
#define TESTDISPLAY_H

#include "AR1021I2C.h"
#include "Graphics.h"
#include "LcdController.h"
#include "EaLcdBoardGPIO.h"

/**
 * Test the display connected with a FPC cable to the LPC4088 Experiment Base Board
 * as well as the AR1021 touch sensor on the board.
 */
class TestDisplay {
public:
	enum WhichDisplay {
		TFT_5,    // 5" display
		TFT_4_3,  // 4.3" display
	};

    /**
     * Create an interface to the display
     */
    TestDisplay(WhichDisplay which);
    ~TestDisplay();

    /**
     * Test the display
     *
     * @return true if the test was successful; otherwise false
     */
	bool runTest();

private:

    void calibrate_drawMarker(Graphics &g, uint16_t x, uint16_t y, bool erase);
    bool calibrate_display();

	char* _initStr;
    LcdController::Config* _lcdCfg;
    EaLcdBoardGPIO _lcdBoard;
    AR1021I2C _touch;

    uint32_t _framebuffer;
};

#endif

