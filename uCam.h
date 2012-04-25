/*
    uCam.h
    
    Some code for controlling uCam 529 by 
    4D Systems (http://www.4dsystems.com.au/)
    
    (c) Copyright 2012 9 Circuits. All Rights Reserved.
    
    This file is part of uCam Control Code.

    uCam Control Code is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    uCam Control Code is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with uCam Control Code.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef UCAM529_h
#define UCAM529_h

#include "GM862.h"
#include "WProgram.h"

class UCAM529 {

public:
    UCAM529(HardwareSerial *camPort, GM862 *dataDevice, byte onOffPin, byte quality);

    void init();
    void takePicture();

    HardwareSerial *camPort;
    GM862 *dataDevice;
    
private:
    int send_initial();
    int set_package_size();
    int do_snapshot();
    int get_picture();
    int get_data();
    int send_ack();
    int wait_for_sync();
    int attempt_sync();
    int wait_for_ack(byte command[6]);

    boolean cam_sync;
    char pin[5];
    byte state;
    byte onOffPin;
};


#endif
