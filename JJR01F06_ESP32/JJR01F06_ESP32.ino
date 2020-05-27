//Include libraries necessary for the project
#include <SimpleFOC.h>     //Library to control the BLDC Motors: https://github.com/askuric/Arduino-FOC
                           //Needs specific modifications to run on the ESP32
#include "driver/mcpwm.h"
#include "soc/mcpwm_reg.h"
#include "soc/mcpwm_struct.h"
#include <WiFi.h>
#include <ESP32WebServer.h>
#include <WebSocketsServer.h>
#include <math.h>

// MCPWM Pins
#define GPIO_PWM0A_OUT 25   //Set GPIO 25 as PWM0A
#define GPIO_PWM0B_OUT 16   //Set GPIO 16 as PWM0B
#define GPIO_PWM1A_OUT 26   //Set GPIO 26 as PWM1A
#define GPIO_PWM1B_OUT 17   //Set GPIO 17 as PWM1B
#define GPIO_PWM2A_OUT 27   //Set GPIO 27 as PWM2A
#define GPIO_PWM2B_OUT 18   //Set GPIO 18 as PWM2B

// Define encoder(s) interrupt pins
// Encoder for Motor A
#define arduinoInt1 38
#define arduinoInt2 39
// Encoder for Motor B
#define arduinoInt3 36
#define arduinoInt4 37

// Write wifi network and password
const char* ssid_wifi = "Write here your local network name";
const char* password_wifi = "Write here your password"; 
// Static variables that provide information from the webpage
static char *dataJoy;
static int16_t coordenadas[7];
static int16_t i;

int posX_joy = 200;
int posY_joy = 200;
byte save, play, stap, reset, demoM, demoC; 

//Web page developed in HTML and JS codified in HEX
static const char INDEX_HTML[] PROGMEM ={0x1f,0x8b,0x08,0x08,0x07,0xf1,0xae,0x5e,0x00,0xff,0x69,0x6e,0x64,0x65,0x78,0x2e,0x68,0x74,0x6d,0x6c,0x2e,0x67,0x7a,0x00,0xed,0x5a,0xeb,0x73,0xdb,0x36,0x12,0xff,0x6c,0xfd,0x15,0x88,0x32,0x57,0x49,0xb1,0x9e,0x4e,0x73,0xed,0x28,0xa6,0x73,0x8d,0xec,0xb4,0xe9,0x39,0xae,0x27,0x4e,0x9a,0x68,0x3a,0x9d,0x0e,0x25,0x42,0x16,0x1b,0x8a,0xd0,0x91,0x94,0x1c,0xe7,0xf1,0xbf,0xf7,0xb7,0x78,0x11,0x24,0x25,0x59,0xc9,0xf4,0xc3,0xcd,0xdc,0xe9,0x49,0x2e,0xf6,0x85,0x05,0x76,0xb1,0x58,0xb0,0x76,0x7c,0xef,0xf4,0x97,0xd1,0xab,0xf1,0xe5,0x19,0xfb,0xe9,0xd5,0x8b,0xf3,0x93,0xda,0xf1,0x3c,0x5b,0x44,0x27,0xb5,0x83,0xe3,0x39,0xf7,0x03,0xfc,0x1f,0x1c,0x2f,0x78,0xe6,0xb3,0xd8,0x5f,0x70,0xaf,0xbe,0x0e,0xf9,0xcd,0x52,0x24,0x59,0x9d,0x4d,0x45,0x9c,0xf1,0x38,0xf3,0xea,0x37,0x61,0x90,0xcd,0xbd,0x80,0xaf,0xc3,0x29,0xef,0xc8,0x9b,0x36,0x0b,0xe3,0x30,0x0b,0xfd,0xa8,0x93,0x4e,0xfd,0x88,0x7b,0xfd,0xee,0x77,0x75,0xc9,0x28,0x0b,0xb3,0x88,0x9f,0xfc,0xfc,0xc3,0xc5,0xeb,0x2b,0xf6,0x73,0x72,0xdc,0x53,0xf7,0x56,0xc4,0x74,0xee,0x27,0x29,0x07,0xcb,0x55,0x36,0xeb,0x7c,0xaf,0x48,0x1c,0xd9,0x01,0x4f,0xa7,0x49,0xb8,0xcc,0x42,0x11,0x3b,0xe2,0x25,0xb7,0x0a,0xae,0xbf,0xca,0xe6,0x22,0x71,0xd0,0xce,0xb9,0x88,0xfd,0x24,0x10,0xec,0x87,0xc9,0x2a,0x0e,0xc2,0x94,0xad,0xd2,0x30,0xbe,0x66,0x2f,0xc5,0x84,0x27,0x99,0x60,0xa7,0x8d,0x1f,0x16,0xe1,0x54,0x30,0x25,0x41,0xb1,0x4b,0xb3,0x5b,0xd2,0xee,0x41,0xed,0x63,0xed,0x60,0x22,0xde,0x77,0xd2,0xf0,0x03,0x48,0x86,0x6c,0x22,0x92,0x80,0x27,0x1d,0x80,0x1e,0xd7,0x3e,0xd7,0x26,0x22,0xb8,0x25,0x8c,0x85,0x9f,0x5c,0x87,0xf1,0x90,0xf5,0x97,0x00,0x1f,0x2c,0xfd,0x20,0x90,0xc8,0xea,0x76,0x06,0x35,0x3a,0x33,0x7f,0x11,0x46,0xb7,0x43,0xb6,0x10,0xb1,0x48,0x97,0xfe,0x94,0xa3,0x61,0x2a,0x22,0x91,0x0c,0xd9,0xfd,0x99,0x7c,0x11,0xbf,0x6e,0x22,0x6e,0x88,0x1f,0x74,0x5c,0x46,0x3e,0xd0,0xc3,0x38,0x0a,0x63,0xde,0x99,0x45,0x9c,0x38,0x4d,0x23,0xee,0x83,0x60,0x22,0xb2,0xb9,0xc4,0x06,0x83,0xd5,0x22,0x3e,0xf7,0x33,0x9e,0xf8,0x11,0xe8,0x18,0x9b,0x45,0xc2,0xcf,0x86,0x2c,0xe2,0xb3,0xec,0x31,0x6e,0xe5,0x88,0x0c,0xd9,0xe0,0xd1,0x3f,0xe8,0x6e,0x11,0xc6,0x6a,0x8c,0x86,0xec,0x61,0x5f,0xea,0x66,0x79,0x8c,0x78,0xb6,0x9b,0xc5,0x77,0xfd,0xad,0x2c,0xee,0xff,0x29,0x6e,0x8f,0x4e,0xc3,0x35,0x29,0xae,0xda,0xbe,0x55,0x4d,0x07,0x73,0x1e,0x5e,0xcf,0x33,0x7b,0xab,0xad,0xf4,0x08,0x77,0x9a,0x2c,0xcd,0xc2,0xe9,0x3b,0x65,0x62,0x32,0x2b,0x34,0x5d,0xbe,0x67,0xa9,0x88,0xc2,0x80,0xdd,0x7f,0xf6,0xac,0x8f,0x97,0x11,0x20,0x31,0x8f,0xb6,0xa0,0xce,0xa6,0xc6,0x80,0x18,0xb9,0x9e,0x1e,0x3a,0x5c,0xe9,0x49,0x7c,0x4c,0xc3,0xc4,0x24,0xd8,0xab,0x4f,0xfc,0xe9,0xbb,0xeb,0x44,0x60,0x1e,0x74,0x94,0xfd,0xef,0x0f,0x1e,0xd2,0xfb,0x31,0xc6,0x1d,0xa8,0x53,0x4c,0x18,0x9e,0x48,0x0f,0x18,0x9c,0xfc,0x2c,0x6e,0xaf,0x48,0x2e,0xcb,0x67,0x2d,0xa0,0x68,0x0b,0xc2,0x35,0x0b,0x03,0xaf,0xae,0x7b,0x5e,0x3f,0x39,0xee,0x01,0x84,0x96,0x4b,0x91,0x62,0xf2,0x8b,0x98,0xbd,0x1d,0x1e,0x87,0xf1,0x72,0x95,0x59,0x34,0x6a,0xf9,0x80,0x16,0xfe,0xb6,0xce,0xb2,0xdb,0x25,0x34,0xc9,0xf8,0x7b,0xb8,0x52,0x0f,0xb4,0x13,0x12,0x68,0x49,0xc7,0x5b,0x49,0xc7,0x1b,0x49,0xf3,0x1f,0x68,0xa6,0x28,0x15,0xd6,0x64,0x95,0x65,0xe4,0x2e,0x6b,0x3f,0x5a,0xe1,0xf6,0xca,0x5f,0x73,0x66,0xa4,0xd4,0x99,0x88,0xa7,0x11,0xfa,0xe6,0xd5,0x53,0xc0,0x01,0x6e,0xb6,0x60,0x82,0x2f,0x62,0x78,0x89,0x29,0xea,0xf0,0xa1,0x19,0x2b,0x99,0x7c,0xc3,0xe3,0x74,0x89,0xf1,0xde,0xa9,0x4c,0x26,0x96,0xae,0x0e,0xb8,0xdd,0x9b,0xf6,0x25,0x47,0xa0,0x70,0x88,0x13,0xba,0xff,0x63,0xf9,0x35,0x5d,0x18,0x85,0x09,0x9c,0x8a,0x9d,0xf2,0x85,0x70,0x18,0x8e,0xc2,0x00,0x00,0xad,0xcf,0xe2,0x4e,0x7d,0x46,0x22,0x8c,0xd3,0x0a,0x0f,0x61,0x79,0xd0,0x54,0x34,0xf3,0x0a,0xd7,0x2a,0xcc,0x38,0x23,0xd9,0xfb,0xd3,0x5f,0xfb,0x36,0xf8,0xac,0xfd,0x84,0xd1,0x90,0x3c,0x95,0x42,0x1e,0xcb,0x7b,0x32,0xad,0x7b,0x4f,0xe6,0x72,0xef,0xa5,0x05,0x5c,0x80,0xea,0x41,0x01,0x22,0xf5,0x31,0x90,0x5a,0xef,0xae,0xd7,0x9b,0xb3,0xa7,0x57,0xbf,0x8c,0xfe,0x7d,0xf6,0xea,0xea,0x4e,0xd4,0xe2,0x4b,0x4a,0x43,0xe8,0x8d,0xf9,0x54,0x4e,0x67,0x8f,0xc5,0xfc,0x86,0xbd,0xe1,0x93,0x2b,0x31,0x7d,0xc7,0xb3,0x66,0xe3,0x26,0x1d,0xf6,0x7a,0x8d,0xc3,0x48,0x4c,0x7d,0x42,0xe8,0xce,0x45,0x9a,0x51,0xe0,0x3e,0x6c,0x0c,0xbf,0xef,0xf7,0x07,0xbd,0x46,0x9b,0xfd,0xd6,0x40,0xb8,0x5e,0x85,0xb1,0x68,0xfc,0xde,0x7a,0x5c,0xcb,0x99,0x75,0x11,0x3d,0x97,0x9c,0x78,0xce,0x56,0xb1,0x62,0xdf,0x6c,0xc9,0xa8,0x05,0x1c,0x04,0x03,0xde,0x8d,0xc4,0x75,0xb3,0x31,0x52,0x04,0x7f,0x68,0x20,0x63,0x1d,0xc6,0x1a,0xec,0x50,0x2a,0x72,0x8a,0x68,0xd9,0x6c,0xb5,0x1e,0x33,0x86,0x70,0x51,0x64,0xbd,0xe0,0x69,0xea,0x5f,0xf3,0x02,0x77,0xbe,0xc6,0xc0,0xb5,0x58,0x55,0xc6,0x15,0x4f,0xd6,0x3c,0x61,0xcd,0x84,0x4f,0xc3,0x35,0x4f,0x5b,0x43,0x06,0xbd,0x25,0x76,0x37,0xf0,0x33,0x9f,0x04,0xec,0x78,0x55,0x64,0xf3,0x24,0x11,0x49,0x51,0x32,0x41,0x36,0x49,0xb6,0xa6,0x64,0x67,0x84,0x72,0xef,0xde,0x3d,0x92,0x2c,0xb1,0x29,0x04,0x7e,0xe1,0x70,0x7d,0xc1,0xc0,0x5a,0xdd,0x6c,0xc4,0x40,0x38,0xce,0xa7,0xaa,0x37,0x20,0xf1,0x16,0x49,0x85,0x03,0x60,0xe4,0x93,0xb7,0x84,0xa1,0x9c,0x9e,0x78,0xd8,0xe9,0x5c,0xc2,0x70,0x3c,0x1b,0x68,0xce,0x2c,0x2f,0xe1,0x19,0x87,0x05,0x92,0x3b,0xf3,0xcb,0x58,0xda,0x25,0x09,0xcb,0xf1,0x06,0x85,0x45,0xb3,0xd6,0xc6,0x7b,0x8f,0x35,0x0d,0x55,0x93,0xb2,0x08,0x1f,0x8b,0x70,0xd2,0x66,0x4b,0x3f,0xc1,0x3c,0x85,0x1f,0xa7,0x2d,0x46,0x1d,0xb3,0xb7,0x20,0x70,0x6e,0x3e,0x7d,0x62,0x1f,0x3f,0x23,0x66,0x10,0x4b,0x99,0xe5,0x10,0x3f,0x2c,0x38,0x7c,0x06,0x36,0x01,0xf3,0x3c,0x17,0xbb,0xab,0x30,0x9e,0xb0,0x86,0x59,0xe5,0x1a,0x6c,0x58,0x69,0x6f,0xb5,0xb1,0xb4,0xc9,0xc5,0x75,0x27,0x2f,0x85,0xf1,0x84,0xf5,0x8b,0x2c,0x24,0x58,0xb2,0x50,0x2b,0xf2,0x4e,0x1e,0x1a,0xa5,0xc2,0x44,0xc1,0x25,0x97,0x90,0x62,0x59,0xec,0x47,0xcf,0xc2,0x28,0x1a,0xd1,0x22,0xba,0x93,0x61,0x15,0x1b,0x9d,0xbd,0xef,0xfb,0xfd,0xef,0xfa,0x47,0xa5,0xbe,0x56,0x50,0x0b,0xe2,0xce,0xc1,0xfd,0xcd,0x9d,0x36,0xa8,0x62,0x3f,0x61,0x47,0x9b,0xc5,0x58,0x94,0x82,0x98,0xab,0x2c,0x11,0xef,0xf8,0xfe,0xfd,0x72,0xf1,0xa9,0x67,0x0f,0x1f,0xf6,0x07,0xfd,0x7f,0x6e,0xe9,0x99,0x83,0x2c,0x85,0x22,0xfc,0x7f,0x41,0xdf,0xaa,0xd8,0x95,0xbe,0x55,0x50,0x0a,0x62,0xf6,0xed,0xdb,0x26,0x7c,0xea,0x1b,0x02,0x34,0xef,0x7f,0x5b,0xea,0xdb,0x06,0x64,0x29,0x14,0x79,0xb8,0x78,0xc9,0xb3,0x55,0x12,0xbf,0x12,0x23,0xb9,0xfe,0xed,0x94,0xb9,0x01,0xfd,0x09,0xcb,0x92,0x15,0x2f,0x4a,0xab,0xa2,0x21,0xec,0x1d,0xd4,0x0e,0x7a,0x3d,0x36,0x4a,0x38,0xa2,0x3b,0x1b,0xf9,0x31,0xd6,0x53,0xc6,0x23,0xbe,0x40,0x3b,0xf3,0xe3,0x80,0x21,0x23,0x67,0x21,0x32,0xaa,0x98,0x65,0x73,0x20,0x18,0x97,0x66,0x62,0xf2,0x27,0x42,0xb0,0x72,0x55,0x5c,0xe7,0x0d,0x1e,0x0b,0xc4,0x74,0x45,0xf4,0xdd,0x6b,0x9e,0x9d,0x29,0x56,0x4f,0x6f,0x9f,0x07,0x79,0x3c,0x20,0xb1,0x72,0xad,0x53,0xe2,0x1c,0x8a,0xa9,0xd4,0x43,0x13,0x35,0x1b,0x0a,0xa1,0x41,0xf8,0xea,0xb2,0x8b,0xa4,0xd5,0x53,0xa1,0x01,0xb0,0x70,0xd6,0xd4,0x9e,0xed,0xb1,0x7e,0x4b,0x25,0xdc,0x68,0x76,0xd5,0xe9,0x22,0xa9,0x00,0x2b,0x39,0x9e,0x8a,0xc2,0x38,0xb2,0x24,0xb1,0x5e,0xbd,0x81,0xe6,0x27,0xd9,0x96,0x8b,0x36,0xec,0xe5,0x7f,0x0e,0xb6,0x2c,0xd4,0x05,0x1a,0x0a,0xbc,0xfc,0x25,0x96,0xdd,0x60,0x34,0x0f,0x23,0x18,0x40,0x52,0xd8,0xde,0xd3,0x26,0xeb,0x7d,0xe6,0x69,0x3e,0x30,0x16,0x51,0x01,0xd2,0x6c,0x1c,0x05,0xb2,0xcf,0x0a,0x6f,0x89,0xe8,0x9d,0xd2,0xa0,0xb3,0xfe,0x63,0x86,0xb1,0x7a,0x2a,0x44,0x84,0x75,0x79,0xe0,0x8d,0x79,0x8a,0xff,0xbe,0x77,0x21,0xb0,0xd0,0x31,0xe4,0x54,0xe0,0x89,0xac,0x6c,0xb5,0x98,0xf1,0x84,0xc7,0x53,0x0a,0x9e,0x47,0xec,0x01,0x7b,0xe1,0x67,0xf3,0xee,0xe5,0x73,0xda,0x84,0x28,0x24,0xe3,0x51,0x2f,0xfd,0x20,0x5c,0x91,0xf9,0xb5,0x62,0xaa,0x87,0x9d,0x66,0xe1,0xb6,0x77,0xd4,0x3a,0x1c,0xf4,0x5b,0xad,0xde,0x91,0x56,0x7b,0xe1,0xbf,0x7f,0x21,0xd6,0xdc,0x84,0xfb,0x12,0xb3,0x43,0xf6,0x48,0xe3,0x99,0xb9,0x6d,0xa5,0x54,0x30,0x1f,0x62,0x8f,0xa2,0x2c,0x21,0x27,0xe3,0x5b,0xe0,0x14,0x4c,0xdd,0x63,0x46,0xa6,0x42,0x18,0xe7,0x08,0xda,0xe8,0x39,0x46,0x10,0x22,0x9d,0xa0,0x55,0xe7,0x27,0x91,0x84,0x1f,0xc8,0xfa,0xf0,0xe3,0x45,0x98,0x61,0xb1,0xad,0xb2,0x1d,0x18,0xc1,0xdb,0xa8,0x2e,0xf8,0x35,0xcd,0xca,0x1d,0x3c,0x1f,0xb0,0x0e,0x16,0xbf,0x22,0x93,0x5f,0xb1,0x27,0x0e,0xb1,0x75,0xaf,0x0a,0xb6,0xea,0x56,0x25,0x17,0x88,0x4a,0x72,0x2b,0x0c,0xb5,0x54,0x4c,0x82,0xd7,0x34,0x23,0xb0,0x03,0xa7,0x24,0x82,0x4d,0x57,0x09,0x06,0x3c,0x63,0x58,0xee,0xd5,0x46,0x48,0xcc,0x90,0x22,0xd0,0x06,0x51,0x8d,0x18,0x86,0x2b,0x78,0xeb,0x69,0x2b,0x9b,0x61,0x24,0xe0,0x58,0x03,0xc7,0x00,0xea,0x40,0x30,0xe7,0x18,0xd6,0x70,0x26,0xbd,0x5d,0x55,0x27,0x58,0xba,0x5a,0x52,0xf1,0x42,0x82,0x32,0xb1,0x9a,0xce,0x19,0x62,0x5a,0x2c,0xe0,0xfc,0x70,0xa7,0x3a,0xec,0x42,0xb0,0x34,0xf3,0xa9,0xbe,0x81,0x38,0x61,0x9d,0xd9,0x5c,0x68,0x77,0x6e,0xd5,0x0e,0xb0,0xf4,0x1b,0xb7,0x41,0x5c,0x39,0xa3,0x9c,0xef,0x3c,0x4c,0x51,0x6a,0xe0,0x49,0xb3,0x91,0xb3,0x41,0x56,0x26,0x10,0xa3,0x70,0x7b,0x45,0xb7,0x6d,0x36,0xf3,0xa3,0x94,0x93,0x47,0xdc,0x41,0x4d,0x9d,0xca,0x89,0x69,0x9a,0xee,0x4f,0x0b,0x2f,0xcd,0x49,0xcf,0xe2,0xc0,0xa1,0xc4,0xee,0x98,0xe3,0xf2,0x0e,0xfd,0x17,0x62,0x85,0x41,0x11,0x37,0xb1,0x64,0xf3,0x82,0xee,0x4e,0x71,0xb7,0x97,0x06,0x92,0xd6,0x6a,0x2f,0x69,0xf7,0xd6,0x5e,0xd2,0xae,0x96,0x39,0xe5,0xeb,0x65,0x51,0x77,0x0c,0xeb,0x69,0xe2,0xdf,0xc8,0xf1,0x33,0x61,0x3b,0x00,0xe0,0x4c,0x7b,0x28,0xb6,0x5a,0x0a,0xf0,0x5c,0x7b,0x68,0x53,0x4f,0x95,0xb6,0x71,0x3c,0x42,0xe8,0x3d,0xf8,0xaa,0x57,0xed,0x00,0xb3,0xf6,0x32,0x09,0xd7,0xb4,0xbe,0x60,0x21,0x9a,0x8b,0x20,0x25,0xd8,0xd7,0xbc,0x7a,0x52,0x0b,0xc9,0xf1,0x5f,0x54,0xd6,0xca,0xbb,0x65,0x82,0x8d,0x0c,0x80,0x48,0x0a,0x69,0x24,0x18,0x96,0x96,0x84,0x9b,0x58,0x68,0x7c,0x83,0xa8,0xc1,0xc7,0xa6,0xb6,0x45,0x43,0xe8,0x21,0x56,0x71,0xb8,0x3b,0xe1,0xa8,0xba,0x5c,0x22,0x7c,0x4a,0x0b,0x59,0xb0,0x9f,0x4c,0x2b,0x16,0xc2,0x3e,0xa2,0x10,0xef,0xda,0xac,0x8f,0x36,0x37,0x1a,0x17,0xc6,0x52,0x73,0xa2,0x12,0x95,0xc9,0x5d,0x2a,0xe9,0x87,0x8b,0x98,0xca,0x04,0xe1,0x8a,0x0a,0x31,0x0e,0xaa,0x93,0x36,0x54,0x91,0xa5,0xce,0x34,0xfa,0x9b,0x2d,0x66,0xa2,0xb1,0x0a,0x14,0x66,0x7d,0xaf,0x44,0x12,0x02,0xc2,0x98,0x09,0x9b,0x53,0xa0,0x91,0x41,0x03,0xe9,0xc0,0x26,0x23,0xda,0xc9,0xb3,0xdb,0x88,0x08,0x19,0x2a,0x1e,0xa1,0x2e,0xe0,0x1a,0xac,0x65,0xc2,0x94,0xbb,0xbe,0x68,0x02,0x4d,0x71,0x58,0xa2,0x38,0x71,0xc3,0xba,0xa5,0x2f,0xae,0x65,0x2e,0xb7,0x82,0xfc,0xf1,0x66,0xf9,0xe3,0xed,0xf2,0xc7,0xdb,0xe4,0xeb,0xfc,0xde,0x86,0x54,0x17,0xba,0x41,0x03,0x77,0x1a,0x29,0x9d,0xdb,0x9a,0x94,0x2a,0xc0,0xfb,0x4f,0x22,0xb8,0xb5,0x4a,0x97,0x58,0x02,0x6c,0x8c,0xe4,0x35,0xfd,0x63,0xf4,0xd0,0x46,0x11,0xfe,0x3a,0xa1,0x9c,0xc1,0x48,0x53,0xa8,0xc4,0xd7,0x8f,0x7e,0xd4,0x88,0x1b,0x66,0xf1,0x23,0x73,0xed,0x02,0x8f,0xfa,0x7d,0x23,0xf1,0x5c,0x2e,0x64,0xb2,0xfe,0x07,0x00,0x44,0x50,0x40,0x92,0x33,0x90,0xea,0x52,0x4d,0xe8,0x5b,0xdd,0x89,0x68,0xd2,0x53,0x3f,0x79,0xb7,0x9d,0x72,0x90,0x53,0xba,0xd9,0xb0,0x6b,0xb0,0x19,0x38,0x1a,0x17,0x00,0x7d,0xb9,0xa9,0xe8,0xa4,0xae,0x6b,0x55,0x76,0x2d,0xdb,0x5d,0x6b,0x83,0x0e,0xdb,0x5d,0xab,0xec,0x5c,0x32,0x30,0xa7,0x6c,0x86,0xe5,0x71,0xe1,0xc7,0x54,0x10,0x91,0x2b,0x4b,0xd9,0x5b,0xdc,0xb5,0xcd,0x96,0x49,0xa4,0xcf,0xe8,0x54,0x8f,0xf6,0xd5,0xc4,0xbf,0x4c,0x41,0xf3,0x48,0x13,0x28,0x7c,0x58,0xf5,0x32,0x91,0x00,0xe9,0xa8,0x13,0x14,0xc1,0xc9,0x57,0x67,0x89,0x58,0x60,0x19,0xa6,0x4a,0x7d,0x08,0x7d,0xb0,0x6b,0xf0,0x57,0x11,0xa1,0x10,0xa4,0x89,0x7a,0x99,0x88,0xa2,0x36,0xfb,0x20,0xc4,0x02,0x7c,0x0e,0x54,0xe9,0x05,0xa2,0xe9,0xff,0x54,0xe1,0x5a,0x5f,0x35,0x1a,0x79,0x03,0xf6,0xcd,0x37,0xba,0x4a,0x03,0xbd,0x91,0xac,0x4a,0x8d,0x78,0xfa,0x5b,0xff,0x77,0x0d,0xa0,0x94,0x5a,0x67,0xb8,0xa0,0x25,0xf5,0x0e,0xb4,0x4b,0x6e,0x21,0x5b,0xc2,0x42,0x94,0x8e,0x68,0xbc,0xf1,0x2e,0x3c,0x99,0xa1,0x50,0x7f,0x5f,0x28,0xc3,0x8a,0xd9,0x0c,0x25,0x8d,0x5c,0x46,0xc7,0xb8,0x9d,0x6a,0x38,0x97,0x35,0x78,0xcb,0xb9,0xd4,0xfa,0x4a,0x50,0x61,0x52,0x4d,0x4a,0xec,0x75,0xe0,0x41,0xaa,0x99,0x60,0xd6,0x5f,0xe8,0xc4,0xe0,0x25,0xd6,0x4a,0x9a,0xd3,0xe4,0x86,0x4e,0x58,0xb1,0x77,0xda,0xf3,0x0d,0xb3,0x97,0x9c,0x02,0xa1,0x5d,0x63,0x0f,0xaa,0xab,0xac,0x02,0xe5,0xa1,0x92,0x40,0x18,0xe8,0xcf,0x18,0xff,0xf2,0x60,0x23,0x05,0xd9,0x3c,0x39,0x28,0x85,0x24,0x69,0xcf,0x67,0x58,0xde,0xfe,0xb3,0x42,0xb2,0x18,0xa8,0x72,0x4f,0x1e,0xb5,0x51,0x16,0x4a,0x38,0xe5,0xfa,0xa1,0x3f,0x89,0x90,0xb7,0xd0,0x48,0x6e,0xd8,0xfd,0x95,0x86,0x29,0x4f,0x10,0xed,0x88,0x38,0xe9,0x21,0x1d,0x11,0x6c,0x30,0xd8,0xd7,0xda,0x6b,0x83,0xb9,0xaa,0xd6,0xaa,0x1a,0xab,0xd7,0xd3,0xac,0x56,0xf1,0x24,0x84,0x85,0x9c,0xbc,0x6f,0xe3,0x7a,0x57,0x75,0x49,0x99,0x2e,0x55,0x5d,0xd2,0xe6,0x6b,0xfb,0x7a,0xa4,0x4d,0xd2,0x8a,0x04,0x05,0x9f,0xd9,0xec,0x08,0x5b,0xe6,0xfd,0xff,0xc0,0x34,0xaf,0x1a,0xf0,0xf5,0xf2,0xff,0x93,0xfc,0xee,0x49,0x9e,0x6f,0x0f,0xf2,0x49,0xfe,0xf5,0xc9,0xf8,0x6a,0x82,0x83,0x96,0xbf,0x3d,0x17,0x7f,0x85,0x05,0x48,0x6d,0xb0,0xb1,0xfb,0x34,0x96,0xa3,0xd6,0x44,0x8e,0x07,0xbb,0x58,0x2d,0x70,0x68,0x4c,0x8d,0xcb,0xf0,0x3d,0x8f,0x34,0xae,0x76,0x44,0x2c,0x4d,0x69,0xf7,0x47,0xae,0xea,0x34,0xc5,0x43,0x0a,0x3d,0x31,0x34,0x17,0xd7,0xe2,0x64,0x0a,0x55,0xc0,0xaa,0x28,0xa2,0x37,0xdc,0xfb,0x69,0xa2,0x90,0x4b,0x9a,0xa8,0xea,0xcf,0x1e,0xaa,0xd8,0xb2,0xcf,0x36,0x5d,0xde,0x16,0xf6,0xe5,0x3a,0xcb,0x4e,0x11,0x90,0x12,0x1e,0xe1,0x1c,0x07,0x69,0x35,0xf6,0xf1,0x12,0xac,0xca,0x61,0xd9,0xdc,0xa7,0x0c,0x4b,0x16,0x90,0x52,0xaa,0xbd,0x51,0x19,0x0e,0x28,0x72,0x29,0x0f,0xb1,0x93,0x4e,0xc1,0x6a,0x63,0x8f,0x24,0x25,0x26,0x0c,0x6a,0x06,0x94,0x15,0x1a,0xf6,0xa5,0xad,0x8f,0xe9,0x21,0x0a,0x0a,0x54,0x72,0x71,0xcf,0x83,0xdc,0xee,0x29,0x7f,0xd9,0xd1,0xaf,0xf1,0x7f,0x6f,0xbf,0xa8,0x52,0x54,0xe9,0x97,0xdd,0x79,0xdc,0xf3,0xfc,0x38,0x53,0x97,0x9f,0x3e,0xa9,0x18,0x90,0x83,0xc6,0x74,0xc8,0xa1,0x12,0x68,0x3a,0x8d,0xf2,0x18,0x32,0x41,0x99,0x36,0x29,0x82,0x16,0x3b,0xac,0xb3,0xfa,0xa1,0x86,0xea,0x8d,0x41,0x4b,0xc2,0x34,0x28,0x3f,0xd6,0x29,0x80,0xf3,0xb3,0x9c,0x22,0xb6,0x3d,0xc0,0x29,0x80,0x9d,0x03,0x9b,0x02,0xdc,0x3d,0xa3,0x29,0x36,0x38,0xc7,0x32,0x2a,0x02,0x3b,0x87,0x64,0x29,0x6a,0x1b,0x4d,0x75,0xb6,0xa6,0x5b,0xec,0x99,0x58,0x0e,0x75,0x8e,0xa3,0x64,0xf0,0x75,0x4f,0x9f,0x14,0xc0,0x39,0x6c,0x92,0x00,0x56,0x38,0x57,0x52,0x38,0x85,0x43,0x24,0x0d,0x72,0x4f,0x8c,0x24,0x88,0xa2,0x2c,0x19,0xd8,0x0e,0x83,0x67,0x27,0x5b,0x01,0x8e,0xcd,0x98,0xfc,0x23,0xb8,0x3b,0x2b,0x09,0xb0,0x69,0x56,0x5e,0x88,0x64,0xe1,0x47,0xe1,0x87,0x0f,0x58,0x2c,0xe4,0x09,0x33,0x4d,0xcc,0xb7,0x92,0xc6,0x29,0x89,0x39,0xf3,0x8b,0xc2,0xee,0xb5,0x49,0x98,0x3b,0x83,0x7e,0x9f,0xe6,0x23,0xca,0x9d,0xfd,0xd2,0x94,0xda,0xe5,0x27,0x4d,0xa0,0x3f,0x30,0x5b,0x54,0xd4,0x63,0xf5,0x02,0xd3,0xea,0x15,0x36,0x7e,0xad,0x6e,0x26,0x9e,0x21,0xda,0x04,0x6a,0xef,0x80,0x73,0xe4,0x7d,0x74,0x1f,0xff,0x0d,0xba,0x6f,0xf4,0x05,0xa3,0xbb,0xab,0xfc,0xd8,0x2a,0x3f,0x2e,0x2b,0xff,0xa0,0x33,0xa8,0x74,0xa0,0x62,0x7d,0xc8,0x52,0x55,0x43,0x53,0xbe,0x2c,0x85,0x05,0x84,0x00,0x1f,0xfd,0x90,0xde,0x54,0xf0,0x68,0x0a,0x0d,0x14,0x25,0x12,0x3c,0xfb,0x83,0x6d,0xec,0x12,0x1b,0x17,0x84,0x84,0x9b,0x39,0xb6,0xbc,0xb4,0x67,0x41,0xac,0x48,0x51,0x71,0xa4,0x0d,0x2b,0x0f,0x0a,0x26,0xd0,0xae,0x29,0xa3,0xbc,0x4b,0xcc,0x2e,0xda,0xec,0xe2,0xac,0xcd,0xf0,0xb9,0xa2,0x2f,0x3e,0x6f,0xda,0x0c,0x9f,0x8b,0x37,0x32,0xe8,0x8c,0x88,0x79,0x2c,0x8f,0x35,0x52,0x3a,0x6d,0x9d,0x52,0x51,0x43,0x17,0x40,0xa4,0x01,0x4a,0x26,0x3c,0x0d,0xdd,0x03,0x66,0x63,0x43,0xfd,0xf8,0x00,0xed,0xac,0x3c,0x56,0xaf,0x9b,0xc9,0x6b,0x6b,0xc5,0x80,0x96,0x27,0x85,0xc1,0xc1,0xf9,0xb7,0x2c,0xec,0x1a,0x14,0xc7,0xf4,0xaa,0x12,0x4b,0xc1,0xca,0x20,0x9d,0x78,0xdb,0x8b,0xc5,0xd8,0x93,0x19,0xb4,0xe3,0x2d,0x68,0x08,0x87,0x36,0x07,0xca,0xd5,0x1d,0x49,0x7d,0xc9,0x11,0x1d,0x49,0x78,0x3a,0x67,0x8b,0xa0,0x0d,0x1c,0x2e,0x36,0x71,0x38,0xf9,0x12,0x1d,0xae,0x2c,0x07,0xc5,0xc4,0x5a,0x2e,0xd7,0xa3,0x5a,0x97,0xb7,0x7c,0x40,0xa0,0x58,0x79,0x78,0x64,0xa4,0x4e,0xe0,0x83,0x8f,0x58,0xa0,0x0b,0x12,0xde,0x48,0x09,0x52,0x84,0xae,0xe0,0x6a,0x62,0x83,0x73,0x58,0x44,0xd2,0xbd,0xb1,0x42,0xf3,0xee,0x54,0x8f,0x00,0xbe,0x48,0x91,0xb3,0x7d,0x14,0x71,0x90,0xb4,0x51,0xf4,0x3c,0x57,0x38,0xca,0xed,0x3e,0xc3,0xfd,0xf0,0xa4,0x82,0x39,0xb1,0xb3,0x07,0xef,0x2a,0xc1,0xa4,0xfa,0x85,0x5a,0x74,0x4f,0x9f,0xff,0x2a,0x0f,0xc6,0xe9,0x29,0xab,0x86,0x3c,0xa3,0x97,0x0f,0x44,0xd1,0x69,0x20,0x14,0xfa,0xc8,0xea,0xf2,0x10,0xad,0x3e,0x64,0xf4,0xa0,0x94,0x7a,0x46,0xac,0xde,0x66,0xf4,0x04,0x60,0x29,0x81,0x06,0x8a,0xac,0x39,0x31,0x08,0xd7,0x47,0xfd,0x47,0xfa,0xd1,0x14,0x23,0xbc,0x69,0x05,0xb5,0x73,0x29,0xa4,0xa7,0x11,0xfb,0x5c,0x3e,0x03,0xa4,0x13,0x8e,0x6d,0xc7,0x83,0xe5,0x87,0xbd,0xc0,0xc0,0xd2,0x6b,0x72,0x8a,0x65,0xfb,0x91,0x8f,0x89,0xbc,0x86,0xd5,0x49,0x66,0xd6,0x08,0xa8,0xf9,0x63,0x09,0xad,0x8f,0x25,0x9d,0xba,0xea,0x69,0x24,0xea,0x98,0xc9,0x8b,0x10,0xe4,0xd8,0x67,0x54,0xc3,0xa8,0xee,0xb5,0x93,0x89,0x51,0xac,0xca,0x63,0xec,0xf2,0x50,0x8f,0xd7,0xc9,0x07,0x95,0xe4,0x43,0x4d,0xf4,0x5c,0x1d,0x9e,0x19,0xed,0xa9,0x87,0x46,0xff,0x02,0x60,0x22,0x06,0xaf,0x4d,0x2a,0x00,0x00}; 

ESP32WebServer server (80); 
WebSocketsServer webSocket = WebSocketsServer(8001);

//  BLDCMotor( int phA, int phB, int phC, int pp, int en)
//  - phA, phB, phC - motor A,B,C phase pwm pins
//  - pp            - pole pair number
//  - enable pin    - (optional input)
BLDCMotor motor = BLDCMotor(25, 26, 27, 7);
BLDCMotor motorB = BLDCMotor(16, 17, 18, 7);
//  Encoder(int encA, int encB , int cpr, int index)
//  - encA, encB    - encoder A and B pins
//  - ppr           - impulses per rotation  (cpr=ppr*4)
//  - index pin     - (optional input) 
Encoder encoder = Encoder(arduinoInt1, arduinoInt2, 2048);
Encoder encoderB = Encoder(arduinoInt3, arduinoInt4, 2048);

// Interrupt rutine intialisation
// channel A and B callbacks
void doA1(){encoder.handleA();}
void doB1(){encoder.handleB();}

void doA2(){encoderB.handleA();}
void doB2(){encoderB.handleB();}

//Time managing variables
float runTime, prevT = 0, timeDif, stateT;
int sp = 0, limit, timeInterval = 12000;
byte machineState = 0;

//System Gains and parameters
float ki = 1.0;  //0.3  -- 0.45  -- 0.33 -- 0.6
float ti = 5.0;  //5.0  -- 5.0  -- 9.0 -- 15
float kp = 8.0;   //15                  -- 3.5
float tf = 0.000; //      --0.01 -- 0.0125 -- 0.01
float ramp = 100000;     //               --300
float sourceVoltage = 5.125;
float limitV = 4.85;

//Five Bar Parrallel Robot constants
const int l0 = 63;
const int l1 = 65;
const int l2 = 130;
const int l3 = 130;
const int l4 = 65;

//Angles calculated from the inverse kinematic model
float angleA, angleB;
float radA, radB;

//Circle DEMO coordinates
float circleA[] = {-0.3941,-0.4402,-0.4856,-0.5301,-0.5737,-0.616,-0.6569,-0.6963,-0.7337,-0.769,-0.8017,-0.8316,-0.858,-0.8804,-0.8983,-0.911,-0.9177,-0.9177,-0.9105,-0.8953,-0.872,-0.8403,-0.8006,-0.7534,-0.6998,-0.6408,-0.5777,-0.5117,-0.4442,-0.3761,-0.3083,-0.2416,-0.1764,-0.1133,-0.0524,0.0061,0.0621,0.1156,0.1665,0.215,0.2612,0.305,0.3467,0.3863,0.4239,0.4596,0.4935,0.5256,0.5562,0.5851,0.6126,0.6385,0.663,0.6861,0.7078,0.7281,0.747,0.7644,0.7803,0.7947,0.8074,0.8185,0.8276,0.8347,0.8397,0.8424,0.8426,0.8401,0.8347,0.8262,0.8147,0.7999,0.7819,0.7607,0.7364,0.7091,0.6791,0.6465,0.6116,0.5746,0.5358,0.4953,0.4534,0.4102,0.366,0.3208,0.2748,0.2282,0.1811,0.1335,0.0856,0.0374,-0.011,-0.0594,-0.1079,-0.1562,-0.2045,-0.2524,-0.3001,-0.3474,-0.3941};
float circleB[] = {-1.1362,-1.1087,-1.0798,-1.0492,-1.0171,-0.9832,-0.9475,-0.9099,-0.8703,-0.8286,-0.7848,-0.7386,-0.6901,-0.6392,-0.5857,-0.5297,-0.4712,-0.4103,-0.3472,-0.282,-0.2153,-0.1475,-0.0794,-0.0119,0.0541,0.1172,0.1762,0.2298,0.277,0.3167,0.3484,0.3717,0.3869,0.3941,0.3941,0.3874,0.3747,0.3568,0.3344,0.308,0.2781,0.2454,0.2101,0.1727,0.1333,0.0924,0.0501,0.0066,-0.038,-0.0834,-0.1295,-0.1762,-0.2235,-0.2712,-0.3191,-0.3674,-0.4157,-0.4642,-0.5126,-0.561,-0.6092,-0.6571,-0.7047,-0.7518,-0.7984,-0.8444,-0.8896,-0.9338,-0.977,-1.0189,-1.0594,-1.0982,-1.1352,-1.1701,-1.2027,-1.2327,-1.26,-1.2843,-1.3055,-1.3235,-1.3383,-1.3498,-1.3583,-1.3637,-1.3662,-1.366,-1.3633,-1.3583,-1.3512,-1.3421,-1.331,-1.3183,-1.3039,-1.288,-1.2706,-1.2517,-1.2314,-1.2097,-1.1866,-1.1621,-1.1362};

//Four-points DEMO
float coinsA[] = {-1.5, 1, 1.6, 0.5};
float coinsB[] = {-1.1, 2, 0, -1};

//Array with fixed size for saved positions
static float posA[50];
static float posB[50];

//Target variable, works if virtual joystick is disabled
float target = 0;
int savedIndex = 0;
// loop stats variables
unsigned long t = 0;
long timestamp = micros();

//Declare functions that are written below before void setup()
static void IRAM_ATTR setup_MCPWM();
void WebSocketEvent_vd(uint8_t num_u8s, WStype_t type_ws, uint8_t *payload_u8sp, size_t length);
void serialEvent();
void handleData();
void inverseKinemstics();
void printGains();

void setup() {
  Serial.begin(115200);
  setup_MCPWM();
  _delay(100);
  
  //  Quadrature::ENABLE - CPR = 4xPPR  - default
  encoder.quadrature = Quadrature::ENABLE;
  encoderB.quadrature = Quadrature::ENABLE;

  // Pullup::EXTERN - external pullup added - dafault
  // Pullup::INTERN - needs internal arduino pullup
  encoder.pullup = Pullup::INTERN;
  encoderB.pullup = Pullup::INTERN;

  // initialise encoder hardware
  encoder.init();
  encoder.enableInterrupts(doA1,doB1);
  encoderB.init();
  encoderB.enableInterrupts(doA2,doB2);
  
  // power supply voltage, default 12V
  motor.voltage_power_supply = sourceVoltage;
  motorB.voltage_power_supply = sourceVoltage;

  // sellect FOC loop to be used
  // ControlType::voltage
  // ControlType::velocity
  // ControlType::angle
  motor.controller = ControlType::angle;
  motorB.controller = ControlType::angle;
  
  // velocity PI controller parameters
  motor.PI_velocity.P = ki;                         // default K=1.0 Ti = 0.003
  motor.PI_velocity.I = ti;
  motor.PI_velocity.voltage_limit = limitV;         //defualt voltage_power_supply
  // jerk control using voltage voltage ramp
  motor.PI_velocity.voltage_ramp = ramp;            // default value is 300 volts per sec  ~ 0.3V per millisecond
  // velocity low pass filtering
  motor.LPF_velocity.Tf = tf;                       // default 10ms 

  // velocity PI controller parameters
  motorB.PI_velocity.P = ki;                        // default K=1.0 Ti = 0.003
  motorB.PI_velocity.I = ti;
  motorB.PI_velocity.voltage_limit = limitV;        //defualt voltage_power_supply/2
  // jerk control using voltage voltage ramp
  motorB.PI_velocity.voltage_ramp = ramp;           // default value is 300 volts per sec  ~ 0.3V per millisecond
  // velocity low pass filtering
  motorB.LPF_velocity.Tf = tf;                      // default 10ms
  
  // angle P controller 
  motor.P_angle.P = kp;                             // default K=70
  // maximal velocity of the poisiiton control
  motor.P_angle.velocity_limit = 200000;            // default 20

  // angle P controller 
  motorB.P_angle.P = kp;                            // default K=70
  //  maximal velocity of the poisiiton control
  motorB.P_angle.velocity_limit = 200000;           // default 20

  // use debugging with serial for motor init
  //motor.useDebugging(Serial);

  // link the motor to the sensor
  motor.linkSensor(&encoder);
  motorB.linkSensor(&encoderB);

  // intialise motor
  // align encoder and start FOC
  motor.init();
  motor.initFOC();
  Serial.println("Motor A ready.");
  _delay(200);
  
  motorB.init();
  motorB.initFOC();
  Serial.println("Motor B ready.");
  _delay(200);
  
  Serial.println("JANUS Ready.");
  Serial.println("Set the target angle using serial terminal:");
  _delay(500);

  //Wifi initialization
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid_wifi, password_wifi);
  Serial.print("Conectando a:\t");
  Serial.println(ssid_wifi); 
  
  // Waits for connection to be established
  while (WiFi.status() != WL_CONNECTED){
    delay(200);
    Serial.print('.');
  }

  //Print server IP to virtual COM
  Serial.println(WiFi.localIP());

  server.on("/", []() {                              
      server.sendHeader("content-encoding","gzip");                             
      server.send_P(200, "text/html", INDEX_HTML,sizeof(INDEX_HTML));     
  });
  server.begin();                                   
  Serial.println("WebServer initiated...");  

  webSocket.begin();
  /*Event of the WebSocket*/
  webSocket.onEvent(WebSocketEvent_vd);

  //Print starting system gains, can be changed though the virtual COM
  printGains();
}

void loop() {
  // Manage INPUTs from the virtual COM
  serialEvent();
  
  // Iterative state calculation calculating angle and setting FOC pahse voltage
  // The faster you run this funciton the better, in arduino loop it should have ~1kHz, the best would be to be in ~10kHz range
  motor.loopFOC();
  motorB.loopFOC();

  radA = motor.shaft_angle;
  radB = motorB.shaft_angle;
  
  //Function intended to be used with serial plotter to monitor motor variables, significantly slowing the execution down!!!!
  //motor_monitor();

  // STATE MACHINE
  if (machineState == 0){   //Default, Robot control through wifi controller
    inverseKinematics(posX_joy, posY_joy);
    motor.move(angleA);
    motorB.move(angleB);
  }
  else if ( machineState == 3){   //Saved positions DEMO
     motor.move(posA[sp]);
     motorB.move(posB[sp]);
  }
  else if ( machineState == 6){   //Circle DEMO
    motor.move(circleA[sp]+0.6);
    motorB.move(circleB[sp]+0.6);
  }
  else if ( machineState == 7){   //Coins DEMO
    motor.move(coinsA[sp]);
    motorB.move(coinsB[sp]);
  }

  //Time managment for dinamic movement DEMOs
  runTime = micros();
  timeDif = runTime - prevT;
  prevT = runTime;
  stateT += timeDif;

  if(sp == limit){
    sp = 0 ;
  }
  
  if(stateT >= timeInterval && (machineState == 6 || machineState == 7 || machineState == 3 )){
    sp += 1;
    stateT = 0;
    //Serial.println(sp);
  }
  
  server.handleClient();
  webSocket.loop();                                       
  handleData();
}

//Inverse kinematic model. Accepts x and y positions [mm] from the virtual joystick and returns motors' angles [rad].
void inverseKinematics(int posX, int posY){
  if (posX == 0 && posY == 0){
    posX = 200;
    posY = 200;
  }
  
  posX = (posX - 200)*1.45;
  posY = 300 - posY;
  if (posY <= 0) posY = 0;
  
  float dA = sqrt((posY*posY)+(((l0/2)+posX)*((l0/2)+posX)));
  float argA = ((dA*dA)+(l1*l1)-(l2*l2))/(2*dA*l1);
  if (argA >= 1) argA = 1;
  else if (argA <= 0) argA = 0;
  float phiA = acos(argA);
  //if (posY >=50 || posX <= 0){
    float thetaA = atan2(posY,((l0/2)+posX));
    angleA = -((thetaA + phiA - 14*PI/15))+0.1;
  //}
  //else{
    //angleA = -((phiA - 14*PI/15))+0.1;
  //}

  
  float dB = sqrt((posY*posY)+(((l0/2)-posX)*((l0/2)-posX)));
  float argB = ((dB*dB)+(l4*l4)-(l3*l3))/(2*dB*l4);
  if (argB >= 1) argB = 1;
  else if (argB <= 0) argB = 0;
  float phiB = acos(argB);
  
  //if (posY >=50 || posX > 0){
    float thetaB = atan2(posY,((l0/2)-posX));
    angleB = ((thetaB + phiB - 14*PI/15))+0.35;
  //}
  //else{
    //angleB = ((phiB - 14*PI/15))+0.35;
  //}
   
  //Serial.print("posX: ");
  //Serial.print(posX);
  //Serial.print(", posY: ");
  //Serial.print(posY);
  //Serial.println(thetaA);
  //Serial.println(dA);
  //Serial.println(argA);
  //Serial.println(phiA);
  //Serial.print(", angleA: ");
  //Serial.print(angleA);
  //Serial.print(", angleB: ");
  //Serial.println(angleB);
}

//Function that handles data from the webpage
void handleData(){
  if (coordenadas[0]!=posX_joy || coordenadas[1]!=posY_joy)
  {
    Serial.print("X:");
    Serial.print(coordenadas[0]);
    Serial.print("\t");
    Serial.print("Y:");
    Serial.println(coordenadas[1]);
  }
  if(coordenadas[2]==1 && save!=coordenadas[2]){
    Serial.println("Position saved");
    posA[savedIndex] = radA;
    posB[savedIndex] = radB;
    Serial.print("Motor A angle: ");
    Serial.println(posA[savedIndex]);
    Serial.print("Motor B angle: ");
    Serial.println(posB[savedIndex]);
    Serial.print("Saved Position Index: ");
    Serial.println(savedIndex);
    savedIndex += 1;
  }
  else if(coordenadas[3]==1 && play!=coordenadas[3] ){
    if(posA[0] == 0 && posB[0] == 0){
      Serial.println("No points saved.");
    }
    else{
      Serial.println("Play saved positions");
      timeInterval = 500000;
      limit = savedIndex;
      sp = 0;
      motor.P_angle.P = 25;
      motorB.P_angle.P = 25;
      machineState = 3;
    }
  }
  else if(coordenadas[4]==1 && stap!=coordenadas[4]){
    Serial.println("Stop");
    motor.P_angle.P = kp;
    motorB.P_angle.P = kp;
    machineState = 0;
  }
  else if(coordenadas[5]==1 && reset!=coordenadas[5]){
    Serial.println("Reset saved positions");
    savedIndex = 0;
    limit = 0;
    sp = 0;
    motor.P_angle.P = kp;
    motorB.P_angle.P = kp;
    machineState = 0;
    memset(posA, 0.1, sizeof(posA));
    memset(posB, 0.35, sizeof(posB));
  }
  else if(coordenadas[6]==1 && demoC!=coordenadas[6]){
    Serial.println("Circle DEMO");
    limit = 100;
    timeInterval = 15000;
    sp = 0;
    motor.P_angle.P = 25;
    motorB.P_angle.P = 25;
    
    machineState = 6;
  }
  else if(coordenadas[7]==1 && demoM!=coordenadas[7]){
    Serial.println("Coins DEMO");
    limit = 4;
    timeInterval = 500000;
    sp = 0;
    motor.P_angle.P = 25;
    motorB.P_angle.P = 25;
    machineState = 7;
  }
  
  posX_joy=coordenadas[0];
  posY_joy=coordenadas[1];
  save = coordenadas[2];
  play = coordenadas[3];
  stap = coordenadas[4];
  reset = coordenadas[5];
  demoC = coordenadas[6];
  demoM = coordenadas[7];
}


// Serial communication callback
void serialEvent() {
  // a string to hold incoming data
  static String inputString;
  while (Serial.available()) {
    // get the new byte:
    char inChar = (char)Serial.read();
    // add it to the inputString:
    inputString += inChar;
    // if the incoming character is a newline
    // end of input
    if (inChar == '\n') {
      if(inputString.charAt(0) == 'P'){
        motor.PI_velocity.P = inputString.substring(1).toFloat();
        motorB.PI_velocity.P = inputString.substring(1).toFloat();
        printGains();
      }else if(inputString.charAt(0) == 'I'){
        motor.PI_velocity.I = inputString.substring(1).toFloat();
        motorB.PI_velocity.I = inputString.substring(1).toFloat();
        printGains();
      }else if(inputString.charAt(0) == 'F'){
        motor.LPF_velocity.Tf = inputString.substring(1).toFloat();
        motorB.LPF_velocity.Tf = inputString.substring(1).toFloat();
        printGains();
      }else if(inputString.charAt(0) == 'K'){
        motor.P_angle.P = inputString.substring(1).toFloat();
        motorB.P_angle.P = inputString.substring(1).toFloat();
        printGains();
      }else if(inputString.charAt(0) == 'R'){
        motor.PI_velocity.voltage_ramp = inputString.substring(1).toFloat();
        motorB.PI_velocity.voltage_ramp = inputString.substring(1).toFloat();
        printGains();
      }else if(inputString.charAt(0) == 'L'){
        motor.PI_velocity.voltage_limit = inputString.substring(1).toFloat();
        motorB.PI_velocity.voltage_limit = inputString.substring(1).toFloat();
        printGains();
      }else if(inputString.charAt(0) == 'T'){
        Serial.print("Average loop time is (microseconds): ");
        Serial.println((micros() - timestamp)/t);
        t = 0;
        timestamp = micros();
      }else if(inputString.charAt(0) == 'C'){
        Serial.print("Contoller type: ");
        int cnt = inputString.substring(1).toFloat();
        if(cnt == 0){
          Serial.println("angle!");
          motor.controller = ControlType::angle;
        }else if(cnt == 1){
          Serial.println("velocity!");
          motor.controller = ControlType::velocity;
        }else if(cnt == 2){
          Serial.println("volatge!");
          motor.controller = ControlType::voltage;
        }
        Serial.println();
        t = 0;
        timestamp = micros();
      }else{
        target = inputString.toFloat();
        Serial.print("Target : ");
        Serial.println(target);
        inputString = "";
      }
      inputString = "";
    }
  }
}

//Function that prints the value of the system control gains.
void printGains(){
  Serial.print("PI velocity P: ");
  Serial.print(motor.PI_velocity.P);
  Serial.print(",\t I: ");
  Serial.print(motor.PI_velocity.I);
  Serial.print(",\t Low passs filter Tf: ");
  Serial.print(motor.LPF_velocity.Tf,4);
  Serial.print(",\t Kp angle: ");
  Serial.println(motor.P_angle.P);
  Serial.print("Voltage ramp: ");
  Serial.print(motor.PI_velocity.voltage_ramp);
  Serial.print(",\t Voltage limit: ");
  Serial.println(motor.PI_velocity.voltage_limit);
}

// Utility function intended to be used with serial plotter to monitor motor variables, significantly slowing the execution down!!!!
void motor_monitor() {
  switch (motor.controller) {
    case ControlType::velocity:
      Serial.print(motor.voltage_q);
      Serial.print("\t");
      Serial.print(motor.shaft_velocity_sp);
      Serial.print("\t");
      Serial.println(motor.shaft_velocity);
      break;
    case ControlType::angle:
      Serial.print(motor.voltage_q);
      Serial.print("\t");
      Serial.print(motor.shaft_angle_sp);
      Serial.print("\t");
      Serial.println(motor.shaft_angle);
      break;
    case ControlType::voltage:
      Serial.print(motor.voltage_q);
      Serial.print("\t");
      Serial.print(motor.shaft_angle);
      Serial.print("\t");
      Serial.println(motor.shaft_velocity);
      break;
  }
}

// Function that initializes the GPIOs that are going to be used for the PWM signals that drive the L293
static void setup_MCPWM_pins(){
    Serial.println("Initializing MCPWM control PINs...");
    mcpwm_gpio_init(MCPWM_UNIT_0, MCPWM0A, GPIO_PWM0A_OUT);
    mcpwm_gpio_init(MCPWM_UNIT_0, MCPWM0B, GPIO_PWM0B_OUT);
    mcpwm_gpio_init(MCPWM_UNIT_0, MCPWM1A, GPIO_PWM1A_OUT);
    mcpwm_gpio_init(MCPWM_UNIT_0, MCPWM1B, GPIO_PWM1B_OUT);
    mcpwm_gpio_init(MCPWM_UNIT_0, MCPWM2A, GPIO_PWM2A_OUT);
    mcpwm_gpio_init(MCPWM_UNIT_0, MCPWM2B, GPIO_PWM2B_OUT);
}

#define TIMER_CLK_PRESCALE 1
#define MCPWM_CLK_PRESCALE 0
#define MCPWM_PERIOD_PRESCALE 4
#define MCPWM_PERIOD_PERIOD 2048

static void IRAM_ATTR setup_MCPWM(){
     setup_MCPWM_pins();

     mcpwm_config_t pwm_config;
     pwm_config.frequency = 4000000;  //frequency = 20000Hz
     pwm_config.cmpr_a = 0;      //duty cycle of PWMxA = 50.0%
     pwm_config.cmpr_b = 0;      //duty cycle of PWMxB = 50.0%
     pwm_config.counter_mode = MCPWM_UP_DOWN_COUNTER; // Up-down counter (triangle wave)
     pwm_config.duty_mode = MCPWM_DUTY_MODE_0; // Active HIGH
     mcpwm_init(MCPWM_UNIT_0, MCPWM_TIMER_0, &pwm_config);    //Configure PWM0A & PWM0B with above settings
     mcpwm_init(MCPWM_UNIT_0, MCPWM_TIMER_1, &pwm_config);    //Configure PWM0A & PWM0B with above settings
     mcpwm_init(MCPWM_UNIT_0, MCPWM_TIMER_2, &pwm_config);    //Configure PWM0A & PWM0B with above settings

     mcpwm_stop(MCPWM_UNIT_0, MCPWM_TIMER_0);
     mcpwm_stop(MCPWM_UNIT_0, MCPWM_TIMER_1);
     mcpwm_stop(MCPWM_UNIT_0, MCPWM_TIMER_2);
     MCPWM0.clk_cfg.prescale = MCPWM_CLK_PRESCALE;
  
     MCPWM0.timer[0].period.prescale = MCPWM_PERIOD_PRESCALE;
     MCPWM0.timer[1].period.prescale = MCPWM_PERIOD_PRESCALE;
     MCPWM0.timer[2].period.prescale = MCPWM_PERIOD_PRESCALE;    
     delay(1);
     MCPWM0.timer[0].period.period = MCPWM_PERIOD_PERIOD;
     MCPWM0.timer[1].period.period = MCPWM_PERIOD_PERIOD;
     MCPWM0.timer[2].period.period = MCPWM_PERIOD_PERIOD;
     delay(1);
     MCPWM0.timer[0].period.upmethod =0;
     MCPWM0.timer[1].period.upmethod =0;
     MCPWM0.timer[2].period.upmethod =0;
     delay(1); 
     
     mcpwm_start(MCPWM_UNIT_0, MCPWM_TIMER_0);
     mcpwm_start(MCPWM_UNIT_0, MCPWM_TIMER_1);
     mcpwm_start(MCPWM_UNIT_0, MCPWM_TIMER_2);
   
     mcpwm_sync_enable(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_SELECT_SYNC_INT0, 0);
     mcpwm_sync_enable(MCPWM_UNIT_0, MCPWM_TIMER_1, MCPWM_SELECT_SYNC_INT0, 0);
     mcpwm_sync_enable(MCPWM_UNIT_0, MCPWM_TIMER_2, MCPWM_SELECT_SYNC_INT0, 0);
     delayMicroseconds(1000);
     MCPWM0.timer[0].sync.out_sel = 1;
     delayMicroseconds(1000);
     MCPWM0.timer[0].sync.out_sel = 0;

}

//Web sockets event handle
void WebSocketEvent_vd(uint8_t num_u8s, WStype_t type_ws, uint8_t *payload_u8sp, size_t length) 
{
    switch(type_ws) 
    {
        /*When there is a new connection sends a string of "0"*/
        case WStype_CONNECTED: 
        {
        }
        break;
        /*When there is a new connection sends a string of "0"*/
        case WStype_DISCONNECTED:
        {
        }
        break;
        /*When there is new data available from the HTML*/
        case WStype_TEXT:
        {
            dataJoy = (char*)payload_u8sp;  
            char delimiter [] = " ";
            char* rest = strtok (dataJoy, delimiter);
            i=0; 
            while(rest != NULL) 
            {
              coordenadas[i]=atoi(rest);
              rest = strtok(NULL, delimiter);
              i++;
            }      
        }
        break;
        
        /*When there is an error in the WebSocket communication*/
        case WStype_ERROR:                                                                
        {
        }
        break;
    }
}