#include <Phpoc.h>
#include <Servo.h>

int angle_init[]	= {90, 101, 165, 153, 90, 120}; // when motor stands straight. In web, the angle when motor stands straight is {0, 90, 130, 180, 0, 0};
int angle_offset[]	= {0, 11, -15, -27, 0, 137}; // offset between real servo motor and angle on web
int cur_angles[]	= {90, 101, 165, 153, 90, 120}; // current angles of six motors (degree) 
int dest_angles[]	= {0, 0, 0, 0, 0, 0}; // destined angles
int angle_max[]		= {180, 180, 160, 120, 180, 137};
int angle_min[]		= { 0, 0, 0, 20, 0, 75};
int direction[]		= {1, 1, 1, 1, 1 ,-1};
int angleSteps[]	= {3, 2, 2, 2, 4 ,4}; // moving steps of each motor (degree)

Servo servo1;
Servo servo2;
Servo servo3;
Servo servo4;
Servo servo5;
Servo servo6;

Servo servo[6] = {servo1, servo2, servo3, servo4, servo5, servo6};

PhpocServer server(80);
PhpocClient client;

int stepNum = 0;

void setup(){
	Serial.begin(9600);

	Phpoc.begin(PF_LOG_SPI | PF_LOG_NET);
	server.beginWebSocket("remote_arm");

	servo1.attach(2);  // attaches the servo on pin 2 to the servo object
	servo2.attach(3);  // attaches the servo on pin 3 to the servo object
	servo3.attach(4);  // attaches the servo on pin 4 to the servo object
	servo4.attach(5);  // attaches the servo on pin 5 to the servo object
	servo5.attach(6);  // attaches the servo on pin 6 to the servo object
	servo6.attach(7);  // attaches the servo on pin 7 to the servo object

	for(int i = 0; i < 6; i++)
		servo[i].write(angle_init[i]);
}

void loop() {
	PhpocClient client = server.available();

	if(client) {
		String angleStr = client.readLine();

		if(angleStr) {
			Serial.println(angleStr);
			int commaPos1 = -1;
			int commaPos2;

			for(int i = 0; i < 5; i++) {
				commaPos2 = angleStr.indexOf(',', commaPos1 + 1);
				int angle = angleStr.substring(commaPos1 + 1, commaPos2).toInt();
				dest_angles[i] = angle * direction[i] + angle_offset[i];
				commaPos1 = commaPos2;
			}

			int angle5 = angleStr.substring(commaPos1 + 1).toInt();
			dest_angles[5] = angle5 * direction[5] + angle_offset[5];

			stepNum = 0;

			// move motors in many small steps to make motor move smooth, avoiding move motor suddently. The below is step calculation
			for(int i = 0; i < 6; i++) {
				int dif = abs(cur_angles[i] - dest_angles[i]);
				int step = dif / angleSteps[i];

				if(stepNum < step)
					stepNum = step;
			}
		}
	}

	// move motors step by step
	if(stepNum > 0) {
		for(int i = 0; i < 6; i++) {
			int angleStepMove = (dest_angles[i] - cur_angles[i]) / stepNum;
			cur_angles[i] += angleStepMove;

			if(cur_angles[i] > angle_max[i])
				cur_angles[i] = angle_max[i];
			else if(cur_angles[i] < angle_min[i])
				cur_angles[i] = angle_min[i];

			servo[i].write(cur_angles[i]);
		}

		stepNum--;
		delay(20);
	}
}
