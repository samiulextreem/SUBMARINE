//calling all the include files

#include <Arduino.h>
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "ESP32Servo.h"
#include "BluetoothSerial.h"






#define VEL_SENSOR 35  //pin to check velocity of the sub
#define USER_INPUT 36  // pin to read user analog input


BluetoothSerial SerialBT;

int FIN_Pin_1 = 12;     //servo motor to control fin 1
int FIN_PIN_2 = 13;     //servo motor to control fin 2
int PROP_Pin =  14;     // PWM control of PROPELLER


Servo servo_fin_1;         // servo PWM initialization for servo motor 1
Servo servo_fin_2;        // servo PWM initialization for servo motor 1
ESP32PWM motor_prop_F;   // PWM initialization for power control of propeller motor 


int freq = 50;
int freq_prop = 50;


//Declaring several function to control each part of the submarine independently from each other with their each loop


void SERVO_FIN_1( void *pvParameters );   // Function to control servo fin
void SERVO_FIN_2( void *pvParameters );   // Function to control servo fin
void PROPELLER_M( void *pvParameters );   // Function to control propeller speed

void BT_HANDLER( void *pvParameters );    //bluetooth function initiation
void USER_SERIAL_INPUT(void *pvParameters);       //example user serial input

void VELOCITY_MEASUREMENT( void *pvParameters );  // function to measure the velocity of the submarine

void USER_INPUT_1( void *pvParameters);           //example user input





// declaring message passing queue to pass information from one function to another
static QueueHandle_t servo_1_command_Q = NULL;
static QueueHandle_t servo_2_command_Q = NULL;
static QueueHandle_t propeller_m_Q = NULL;
static QueueHandle_t sensors_Q = NULL;


void setup() {
  // put your setup code here, to run once:
 
	Serial.begin(115200);
	analogReadResolution(12);                     //// Set ADC resolution to 12 bits (0-4095)
	SerialBT.begin("ESP32");




	servo_fin_1.attach(FIN_Pin_1,500,2400);
	servo_fin_2.attach(FIN_PIN_2,500,2400);
	motor_prop_F.attachPin(PROP_Pin, freq_prop, 10);// 1KHz 8 bit PWM declaration





	servo_1_command_Q = xQueueCreate(5,sizeof(int)); // starting queue for servo 1
	servo_2_command_Q = xQueueCreate(5,sizeof(int)); // starting queue for servo 2
	propeller_m_Q = xQueueCreate(5,sizeof(int));     // starting queue for motor
	sensors_Q = xQueueCreate(5,sizeof(int));         // starting queue for sensors
	
	//initializing all the functions to run independently from each other using real time operating system API
	xTaskCreate(BT_HANDLER,"BT operation", 2024 , NULL , 2  , NULL);
	xTaskCreate(SERVO_FIN_1,"servo to control fin 1", 2024 , NULL, 2  , NULL );
	xTaskCreate(SERVO_FIN_2,"servo to control fin 2", 2024 , NULL, 2  , NULL );
	//xTaskCreate(USER_SERIAL_INPUT,"accepts user input", 2024 , NULL , 2  , NULL);
	xTaskCreate(PROPELLER_M,"propeller power", 2024 , NULL , 2  , NULL);
	

	//disabling the following function unless actual sensors and potentiometer is attached
	// xTaskCreate(USER_INPUT_1,"user input function", 2024 , NULL , 2  , NULL);
	// xTaskCreate(VELOCITY_MEASUREMENT,"velocity measurement function", 2024 , NULL , 2  , NULL);
}



void loop() {
	//idle loop
	delay(20);
	
	
}

// Function to control the position of the servo motor angle by moving the PWM signal linearly from current value to target value
void SERVO_FIN_1( void *pvParameters ){
	int servo_angle = 0;               //variable to receive and hold the value of user desired angle
	// int target_angle ;           // converting the input data in a scale of 0 to 1
	int current_angle;           //current PWM value in a scale of 0 to 1
  	while (1)
  	{
		if (xQueueReceive(servo_1_command_Q,&servo_angle,10) == true){             //checking if any new data is available in the queue and read it
			// target_angle = (float)(servo_angle*1.00)/100.00 ;                      //scale it in a value of 0 to 1
			if (current_angle < servo_angle){                                      //checking if the new value is bigger or smaller and which direction to move  
				// linearly move the PWM value from current value to desired value
				for (int s_angle = current_angle; s_angle <= servo_angle; s_angle  = s_angle + 1) {
		  			servo_fin_1.write(s_angle);
					delay(50);
				}
		  		
	  		}
			if (current_angle > servo_angle){                                         //checking if the new value is bigger or smaller and which direction to move
				// linearly move the PWM value from current value to desired value
				for (int s_angle = current_angle; s_angle >= servo_angle; s_angle  = s_angle - 1) {
		  			servo_fin_1.write(s_angle);
		  			delay(50);
				}
	  		}
			delay(20);
			current_angle = servo_angle;         //after the operation, set target value as current value

		}else{
			servo_fin_1.write(servo_angle); // if no new data receiver, then making sure that PWM signal is according to current angle
			delay(20);
		}
		delay(20);
		
 	}

}


// Function to control the position of the servo motor angle by moving the PWM signal linearly from current value to target value
void SERVO_FIN_2( void *pvParameters ){
	int servo_angle;              //variable to receive and hold the value of user desired angle
	// int target_angle ;          // variable which is used to convert the received data in a scale of 0 to 1
	int current_angle;          //current PWM value in a scale of 0 to 1
  	while (1)
  	{
		if (xQueueReceive(servo_2_command_Q,&servo_angle,10) == true){    //checking if any new data is available in the queue and read it
			// target_angle = (float)(servo_angle*1.00)/100.00 ;              //scale it in a value of 0 to 1
			if (current_angle < servo_angle){                            //checking if the new value is bigger or smaller and which direction to move  
				// linearly move the PWM value from current value to desired value
				for (int s_angle = current_angle; s_angle <= servo_angle; s_angle  = s_angle + 1) {
		  			servo_fin_2.write(s_angle);
					delay(50);
				}
	  		}
			if (current_angle > servo_angle){                          //checking if the new value is bigger or smaller and which direction to move
				// linearly move the PWM value from current value to desired value
				for (int s_angle = current_angle; s_angle >= servo_angle; s_angle  = s_angle - 1) {
		  			servo_fin_2.write(s_angle);
		  			delay(50);
				}
	  		}
			delay(20);
			current_angle = servo_angle;                  //after the operation, set target value as current value
		}else{
			servo_fin_2.write(current_angle);       // if no new data receiver, then making sure that PWM signal is according to current angle
			delay(20);
		}
		delay(20);

 	}

}



// FGunction to control the power of motor using PWM signal
void PROPELLER_M(void *pvParameters){
	int prop_power;                        //place holder for receiving the command for propeller
	float target_power;                    // variable which is used to convert the received data in a scale of 0 to 1
	float current_power = 0;
	while (1)
	{
		if (xQueueReceive(propeller_m_Q,&prop_power,10) == true){    // checking if new command is available
			Serial.print("motor has received command to set velocity"); // serial monitor update
			target_power = (float)(prop_power*1.00)/100; //scaling it between 0 and 1
			Serial.println(target_power); 
			if (target_power != current_power){
		  		motor_prop_F.writeScaled(target_power);
	  		}
			delay(20);
			current_power = target_power;
		}else{
			motor_prop_F.writeScaled(target_power);
			delay(20);
		}
		delay(20);
	}
	
}

//Function to measure the velocity from the user input, A potentiometer and convert it to digital data
void USER_INPUT_1(void *pvParameters){
	int Value;
	while (1)                             //unlimited loop
	{
		Value = analogRead(USER_INPUT); // Read analog input
		float voltage_val = Value * (3.3 / 4095.0); // Convert ADC reading to voltage
		if(xQueueSend(sensors_Q,&voltage_val,10) == true){
			Serial.println("direction has been sent to motor");
		}
		delay(100); 
	}
	
}


//Function to measure the velocity from the sensors and convert it to digital data
void VELOCITY_MEASUREMENT(void *pvParameters){
	int Value;
	while (1)                              //unlimited loop
	{
		Value = analogRead(VEL_SENSOR); // Read analog input if sensor
		float voltage_val = Value * (3.3 / 4095.0); // Convert ADC reading to voltage
		if(xQueueSend(sensors_Q,&voltage_val,10) == true){               //sending data to sensors_Q queue
			Serial.println("direction has been sent to motor");
		}
		delay(100); 
	}
	
}


void BT_HANDLER( void *pvParameters ){
	String velocity = "v"; 
	String Direction_fin_1 = "x";
	String Direction_fin_2 = "y";

	while (1)
	{
		if (SerialBT.available()) {
        	String incoming_data = SerialBT.readString();   //read user input from serial terminal
			if(incoming_data.length() > 0){	
				if(incoming_data[incoming_data.length()- 1] == '\n'){   //when returned is pressed 
					SerialBT.println(incoming_data);                         //serial monitor update about the input data
					String characters = "";                                 // value declaration for command type, velocity or direction
   					String numbers = "";                                    // command value  declaration
					//iteration over the user input is storing it in string and number
					for (int i = 0; i < incoming_data.length(); i++) {   
      					char c = incoming_data.charAt(i);
						if (isAlpha(c)) {
							characters += c;
						} else if (isDigit(c)) {
							numbers += c;
						}
					}
					SerialBT.print("command initial : ");
					SerialBT.println(characters);
					if (characters == Direction_fin_1){
						SerialBT.println("Direction adjustment command");
						int direction_data =  numbers.toInt();
						if(numbers != NULL){
							SerialBT.println("sending command to servo fin 1 ");
							if(xQueueSend(servo_1_command_Q,&direction_data,10) == true){
								SerialBT.println("direction has been sent to FIN 1"); //sending command to servo motor fin 1 handling function
							}

						}
					}
					if (characters == Direction_fin_2){
						SerialBT.println("Direction adjustment command");
						int direction_data =  numbers.toInt();
						if(numbers != NULL){
							SerialBT.println("sending command to servo fin 2");
							if(xQueueSend(servo_2_command_Q,&direction_data,10) == true){
								SerialBT.println("direction has been sent to FIN 2"); //sending command to servo motor fin 2 handling function
							}
						}
					}


					if(characters == velocity){
						SerialBT.println("velocity command");
						int velocity_data = numbers.toInt();
						if(numbers != NULL){
							SerialBT.println("sending command to DC motor");
							if(xQueueSend(propeller_m_Q,&velocity_data,10) == true){
								SerialBT.println("velocity has been sent motor control");  //sending command to propeller motor power handling function
							}
						}
					}

				}
			}
    	}
    
    	delay(20);
	}
	
}








//Function to take user input from serial terminal and relay it to the appropiate functions handling appropiate operation
void USER_SERIAL_INPUT (void *pvParameters){
	 //if input starts with v then, it means velocity command with percentages , if it is d then direction command for servo fins
	 // as example v 34  for velocity 34 percent, d 23 as fin angle of 23 degree.
	String velocity = "v"; 
	String Direction_fin_1 = "x";
	String Direction_fin_2 = "y";

	while (1){

		if (Serial.available() > 0) {
			String incoming_data = Serial.readString();   //read user input from serial terminal
			if(incoming_data.length() > 0){	
				if(incoming_data[incoming_data.length()- 1] == '\n'){   //when returned is pressed 
					Serial.println(incoming_data);                         //serial monitor update about the input data
					String characters = "";                                 // value declaration for command type, velocity or direction
   					String numbers = "";                                    // command value  declaration
					//iteration over the user input is storing it in string and number
					for (int i = 0; i < incoming_data.length(); i++) {   
      					char c = incoming_data.charAt(i);
						if (isAlpha(c)) {
							characters += c;
						} else if (isDigit(c)) {
							numbers += c;
						}
					}
					Serial.print("command initial : ");
					Serial.println(characters);
					if (characters == Direction_fin_1){
						Serial.println("Direction adjustment command");
						int direction_data =  numbers.toInt();
						if(numbers != NULL){
							Serial.println("sending command to servo fin 1 ");
							if(xQueueSend(servo_1_command_Q,&direction_data,10) == true){
								Serial.println("direction has been sent to FIN 1"); //sending command to servo motor fin 1 handling function
							}

						}
					}
					if (characters == Direction_fin_2){
						Serial.println("Direction adjustment command");
						int direction_data =  numbers.toInt();
						if(numbers != NULL){
							Serial.println("sending command to servo fin 2");
							if(xQueueSend(servo_2_command_Q,&direction_data,10) == true){
								Serial.println("direction has been sent to FIN 2"); //sending command to servo motor fin 2 handling function
							}
						}
					}


					if(characters == velocity){
						Serial.println("velocity command");
						int velocity_data = numbers.toInt();
						if(numbers != NULL){
							Serial.println("sending command to DC motor");
							if(xQueueSend(propeller_m_Q,&velocity_data,10) == true){
								Serial.println("velocity has been sent motor control");  //sending command to propeller motor power handling function
							}
						}
					}

				}
			}

		}
		
  	}
}









