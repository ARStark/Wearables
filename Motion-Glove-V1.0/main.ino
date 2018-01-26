/* Needed for Accelerometer */
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_LSM303_U.h>
/* end Accelerometer */

/* Needed for NeoPixel on FLORA board*/
#include <Adafruit_NeoPixel.h> 
#define PIN 8
/* end NeoPixel */

/* Needed for Bluefruit LE module*/
#include <Arduino.h>
#include "Adafruit_BLE.h"
#include "Adafruit_BluefruitLE_UART.h"
#include "BluefruitConfig.h"

#define BLUEFRUIT_HWSERIAL_NAME     Serial1
#define FACTORYRESET_ENABLE         0
#define MINIMUM_FIRMWARE_VERSION    "0.6.6"
#define MODE_LED_BEHAVIOUR          "MODE"
/* end Bluefruit LE */ 

//constructor for for hardware serial --- FLORA requires hw serial
Adafruit_BluefruitLE_UART ble(BLUEFRUIT_HWSERIAL_NAME, BLUEFRUIT_UART_MODE_PIN);

/* constructor for NeoPixel*/
Adafruit_NeoPixel strip = Adafruit_NeoPixel(1, PIN, NEO_GRB + NEO_KHZ800);

/* Assign a unique ID to this sensor at the same time */
Adafruit_LSM303_Accel_Unified accel = Adafruit_LSM303_Accel_Unified(54321);


// A small helper
void error(const __FlashStringHelper*err) {
  Serial.println(err);
  while (1);
}


void setup() {
	// put your setup code here, to run once:
  
    // commented out #ifndef...#endif to allow for code to start up with out needing to be plugged into USB cable.
  
    // #ifndef ESP8266
    // while (!Serial);     // will pause Zero, Leonardo, etc until serial console opens
    // #endif

    Serial.begin(9600);
      
    /* Begin accelerometer setup*/
    Serial.println("Accelerometer Test"); Serial.println("");

    /* Initialise the sensor */
    if(!accel.begin())
    {
		/* There was a problem detecting the ADXL345 ... check your connections */
		Serial.println("Ooops, no LSM303 detected ... Check your wiring!");
		while(1);
	}
 	/* end accelerometer setup */
	
	/* On board NeoPixel setup */
	strip.begin();
	strip.setBrightness(50);
	strip.show(); // Initialize all pixels to 'off'
	/* end on board NeoPixel setup */

	/* begin Bluefruit LE setup */
    /* Initialise the module */
	Serial.print(F("Initialising the Bluefruit LE module: "));

	if ( !ble.begin(VERBOSE_MODE) )
	{
		error(F("Couldn't find Bluefruit, make sure it's in CoMmanD mode & check wiring?"));
	}
	Serial.println( F("OK!") );

	if ( FACTORYRESET_ENABLE )
	{
		/* Perform a factory reset to make sure everything is in a known state */
		Serial.println(F("Performing a factory reset: "));
		if ( ! ble.factoryReset() ){
			error(F("Couldn't factory reset"));
		}
	}

	/* Disable command echo from Bluefruit */
	ble.echo(false);

	Serial.println("Requesting Bluefruit info:");
	/* Print Bluefruit information */
	ble.info();

	Serial.println(F("Please use Adafruit Bluefruit LE app to connect in UART mode"));
	Serial.println(F("Then Enter characters to send to Bluefruit"));
	Serial.println();

	ble.verbose(false);  // debug info is a little annoying after this point!

	/* Wait for connection */
	while (! ble.isConnected()) {
  
		delay(500);
	}

	Serial.println(F("******************************"));

	// LED Activity command is only supported from 0.6.6
	if ( ble.isVersionAtLeast(MINIMUM_FIRMWARE_VERSION) )
	{
		// Change Mode LED Activity
		Serial.println(F("Change LED activity to " MODE_LED_BEHAVIOUR));
		ble.sendCommandCheckOK("AT+HWModeLED=" MODE_LED_BEHAVIOUR);
	}

	// Set module to DATA mode
	Serial.println( F("Switching to DATA mode!") );
	ble.setMode(BLUEFRUIT_MODE_DATA);

	Serial.println(F("******************************"));
	/* end accelerometer setup */
} //end setup()



void loop() {
		
	// put your main code here, to run repeatedly:
	/* Get a new sensor event */
	sensors_event_t event;
	accel.getEvent(&event);

	/* print X Y Z values to Arduino serial terminal. Used for initial debugging */
	Serial.print("X Raw: "); Serial.print(accel.raw.x); Serial.print("  ");
	Serial.print("Y Raw: "); Serial.print(accel.raw.y); Serial.print("  ");
	Serial.print("Z Raw: "); Serial.print(accel.raw.z); Serial.println("");

	/* Color thresholds. Still working to make capture of hand orientation feel as natural as possible 
	   Keeps previous color until accelerometer values meet criteria for a new color. Will be updated to send
	   direction controls to Pic32 Bluetooth Robot. */
	   
    if (accel.raw.x >= -600 && accel.raw.x <=-150 && \
	accel.raw.y <= 200 && accel.raw.y >=50){
    colorWipe(strip.Color(255, 0, 0), 500); // Red
    
	ble.print("Rest\n");
    //Serial.print("Stop\n"); //for computer debugging
    }

    else if (accel.raw.x > -150 && accel.raw.y < 50  ) {
    colorWipe(strip.Color(255, 238, 11), 500); // Yellow
    
	ble.print("Forward\n");
    //Serial.print("Forward\n"); //for computer debugging
    }

    else if (accel.raw.x < -600 and accel.raw.y <325 ) {
    colorWipe(strip.Color(0, 0, 255), 500); // Blue
    
	ble.print("Backward\n");
    //Serial.print("Backward\n"); //for computer debugging
    }

    else if (accel.raw.y < -400 && accel.raw.x < -50 ) {
    colorWipe(strip.Color(255, 0, 255), 500); // Magenta
    
	ble.print("Turning left\n");
    //Serial.print("Turning left\n"); //for computer debugging
    }
    else if (accel.raw.y > 575 && accel.raw.x < 300) {
    colorWipe(strip.Color(0, 255, 0), 500); // Green
    
	ble.print("Turning right\n");
    //Serial.print("Turning right\n");//for computer debugging
    }
	/* End color thresholds */
  
  delay(10);

}//end loop()

/* Function came from Adafruit 
	URL: https://learn.adafruit.com/getting-started-with-flora/blink-onboard-neopixel */
	
// Fill the dots one after the other with a color
void colorWipe(uint32_t c, uint8_t wait) {
  for(uint16_t i=0; i<strip.numPixels(); i++) {
      strip.setPixelColor(i, c);
      strip.show();
      delay(wait);
  }
}
