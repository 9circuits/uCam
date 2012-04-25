/*
    uCam.cpp
    
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

#include "UCAM529.h"
#include <string.h>

byte   _SYNC_COMMAND[6] = {0xAA,0x0D,0x00,0x00,0x00,0x00};
byte   _ACK_COMMAND[6] =  {0xAA,0x0E,0x0D,0x00,0x00,0x00};
byte   _INITIAL[6] =      {0xAA,0x01,0x00,0x07,0x00,0x01}; //Current image: Smallest
byte   _PACK_SIZE[6] =    {0xAA,0x06,0x08,0x00,0x02,0x00};
byte   _SNAPSHOT[6] =     {0xAA,0x05,0x00,0x00,0x00,0x00};
byte   _GET_PICTURE[6]=   {0xAA,0x04,0x01,0x00,0x00,0x00};
byte   _PACKET_ACK[6] =   {0xAA,0x0E,0x00,0x00,0x00,0x00};

UCAM529::UCAM529(HardwareSerial *camPort, GM862 *dataDevice, byte onOffPin, byte quality) {
    this->camPort = camPort;
    this->dataDevice = dataDevice;
    this->onOffPin = onOffPin;
    _INITIAL[5] = quality;
}

void UCAM529::init() {
    if (attempt_sync())
    {
        if (wait_for_sync())
        {
            cam_sync = true;
        }
    }
}

void UCAM529::takePicture() {
    if (send_initial())
    {
        if (set_package_size())
        {
            if (do_snapshot())
            {
                if (get_picture())
                {
                    get_data();
                }
            }
        }
    }
}

int UCAM529::send_initial() {
    int ack_success = 0;
    delay(500);
    byte ack[6] = {0xAA, 0x0E, 0x01, 0x00, 0x00, 0x00};
    Serial.println("Intitial is being sent");
    for (int i = 0; i< 6; i++) {

        Serial2.print(_INITIAL[i], BYTE);

    }

    if (wait_for_ack(ack)) {
        Serial.print("\nINITIAL has been acked...\n");
        ack_success = 1;
        return ack_success;
    }
}

int UCAM529::set_package_size() {
    int ack_success = 0;

    byte ack[6] = {0xAA, 0x0E, 0x06, 0x00, 0x00, 0x00};

    for (int i = 0; i< 6; i++) {
        Serial2.print(_PACK_SIZE[i], BYTE);
        delay(300);
    }

    while (!Serial2.available());

    if (wait_for_ack(ack)) {
        Serial.print("\nSET PACKAGE SIZE has been acked...\n");
        ack_success = 1;
        return ack_success;
    }
}

int UCAM529::do_snapshot() {
    int ack_success = 0;

    byte ack[6] = {0xAA, 0x0E, 0x05, 0x00, 0x00, 0x00};

    for (int i = 0; i< 6; i++) {
        Serial2.print(_SNAPSHOT[i], BYTE);
    }

    while (!Serial2.available());

    if (wait_for_ack(ack)) {
        Serial.print("\nSNAPSHOT has been acked...\n");
        ack_success = 1;
        return ack_success;
    }
}

int UCAM529::get_picture() {
    int ack_success = 0;

    byte ack[6] = {0xAA, 0x0E, 0x04, 0x00, 0x00, 0x00};

    for (int i = 0; i< 6; i++) {
        Serial2.print(_GET_PICTURE[i], BYTE);
    }

    while (!Serial2.available());

    if (wait_for_ack(ack)) {
        Serial.print("\nGET PICTURE has been acked...\n");
        ack_success = 1;
        return ack_success;
    }
}

int UCAM529::get_data() {
    byte my_ack[6] = {0xAA, 0x0E, 0x00, 0x00, 0x00, 0x00};

    byte data[6];

    int i = 0;

    byte b;

    while (!Serial2.available());

    while(Serial2.available())
    {
        b = Serial2.read();
        if (b == 0xAA)
        {
            data[i++] = b;
            break;
        }
    }

    while (Serial2.available()) {
        b = Serial2.read();

        data[i++] = b;

        Serial.println(b, HEX);

    }

    long image_size = 0;

    for(int i = 5; i >= 3; i--)
    {
        image_size = (image_size << 8) | data[i];
    }

    Serial.print("Size of the image = ");
    Serial.println(image_size);

    int blocks = ceil((float(image_size)/506.0));

    Serial.println("Number of packets = ");
    Serial.println(blocks);

    byte k = 0x00;
    int z = 0;

    while(Serial2.available())
    {
        b = Serial2.read();
        if (b > 0)
            Serial.print("b");
    }

    for (int j = 0; j < blocks; j++)
    {
        z = 0;

        my_ack[4] = k++;

        for (int i = 0; i< 6; i++) {
            Serial2.print(my_ack[i], BYTE);
        }


        while (!Serial2.available());

        while (z < 4)
        {
            b = Serial2.read();
            if(b != -1)
            {
                z++;
            }
        }

        int size_to_read = 506;

        if (j == blocks - 1)
        {
            size_to_read = image_size - j * 506;
        }


        z = 0;

        Serial.print("Packet number = ");
        Serial.println(j + 1);

        while (z < size_to_read)
        {
            if (Serial2.available())
            {
                b = Serial2.read();


                if (b == 0x00)
                {
                    //Serial3.print("ONI");
                    // modem.send("ONI");
                    dataDevice->send("ONI");
                }
                else
                {
                    //Serial3.print(b);
                    // modem.sendByte(b);
                    dataDevice->sendByte(b);
                }

                // delay(1);
                dataDevice->flowControl();

                z++;

              //Serial.print(z);
            }
        }


        Serial.print("Bytes read = ");
        Serial.println(z);

        z = 0;

        while (z < 2)
        {
            b = Serial2.read();
            if(b != -1)
            {
                z++;
            }
        }

    }

    dataDevice->send("BYE");

}


int UCAM529::send_ack() {

    Serial.println("Sending ACK for sync");
    for (int i = 0; i< 6; i++) {
        Serial2.print(_ACK_COMMAND[i], BYTE);
    }

    Serial.print("Now we can take images!\n");

    delay(2000);
}

int UCAM529::wait_for_sync() {
    byte cam_reply;

    int sync_success = 0;

    int last_reply = 0;

    Serial.print("\nWaiting for SYNC...\n");

    while (!Serial2.available());

    Serial.print("\nReceiving data. Testing to see if it is SYNC...\n");

    while (Serial2.available()) {

        cam_reply = Serial2.read();

       // Serial.print(cam_reply, BYTE);

        if (cam_reply == 0xAA && last_reply == 0) {
            last_reply++;
        }

        if (cam_reply == 0x0D && last_reply == 1) {
            last_reply++;
        }

        if (cam_reply == 0x00 && last_reply == 2) {
            last_reply++;
        }

        if (cam_reply == 0x00 && last_reply == 3) {
            last_reply++;
        }

        if (cam_reply == 0x00 && last_reply == 4) {
            last_reply++;
        }

        if (cam_reply == 0x00 && last_reply == 5) {
            last_reply++;
            Serial.print("\nCamera has SYNCED...\n");
            sync_success = 1;

            break;
        }
    }

    return sync_success;
}

int UCAM529::attempt_sync()
{
    int attempts = 0;

    byte cam_reply;

    int ack_success = 0;

    int last_reply = 0;

    byte ack[6] = {0xAA, 0x0E, 0x0D, 0x00, 0x00, 0x00};

    while(attempts < 60 && ack_success == 0) {
        for (int i = 0; i< 6; i++) {
            Serial2.print(_SYNC_COMMAND[i], BYTE);
        }

        attempts = attempts++;
        if (Serial2.available())
        {
            if (wait_for_ack(ack)) {
                Serial.print("\nCamera has Acked...\n");
                ack_success = 1;
                return ack_success;
            }
        }

    }

    return ack_success;
}


int UCAM529::wait_for_ack(byte command[6]) {
    byte cam_reply;

    int wait_success = 0;

    int last_reply = 0;

    while (!Serial2.available());

    Serial.println("Hopefully receiving an ack");

    while (Serial2.available()) {
        cam_reply = Serial2.read();
        Serial.println(cam_reply, HEX);
        delay(100);
        if (cam_reply == command[last_reply] && last_reply == 0) {
            last_reply++;
        }

        if (cam_reply == command[last_reply] && last_reply == 1) {
            last_reply++;
        }

        if (cam_reply == command[last_reply] && last_reply == 2) {
            last_reply++;
        }

        if (last_reply == 3) {
            last_reply++;
        }

        if (cam_reply == command[last_reply] && last_reply == 4) {
            last_reply++;
        }

        if (cam_reply == command[last_reply] && last_reply == 5) {
            last_reply++;
            wait_success = 1;
            break;
        }
    }

    return wait_success;
}




