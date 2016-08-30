/*
 Name:		AccessSystem.ino
 Created:	2016/7/10 12:22:07
 Author:	Fing
*/

#include <SoftwareSerial.h>
#include <Servo.h>
#include <MFRC522.h>
#include <SPI.h>

/*自定义RC522的两个PIN口*/
#define SS_PIN 10
#define RST_PIN 9

#define BT_RX 3		//蓝牙模块端口
#define BT_TX 2
#define SERVO_PIN 6		//舵机端口
#define BUZZ_PIN 4
#define CARDS 2		//已验证的卡片数目

MFRC522	mfrc522(SS_PIN, RST_PIN);		//新建RC522对象
SoftwareSerial BT(BT_TX, BT_RX);		//创建蓝牙软串口对象，避免冲突

Servo myservo;		//创建舵机对象
bool isAuthed = false;		//验证是否通过
const byte AuthedID[CARDS][4] = { {0xFF, 0xFF, 0xFF, 0xFF}};  //可以保存多个卡片UID值
char val;		//用来存储蓝牙接收数据

void setup() {
	BT.begin(9600);		//蓝牙串口，波特率9600
	myservo.attach(SERVO_PIN);		//连接舵机
	SPI.begin();		//初始化SPI总线
	mfrc522.PCD_Init();		//初始化MFRC522卡
	pinMode(BUZZ_PIN, OUTPUT);		//初始化蜂鸣器
	digitalWrite(BUZZ_PIN, HIGH);
	myservo.write(45);
}

// the loop function runs over and over again until power down or reset
void loop()
{
	Authenticate();
	if (isAuthed) {
		OpenDoor();
		BeepChecked();
		delay(3000);
		isAuthed = false;
	}
	CloseDoor();
	isAuthed = false;
	BTCheck();
	delay(500);
}


//Servo开关门
void OpenDoor()
{
	myservo.write(140);
}

void CloseDoor()
{
	myservo.write(45);
}

//验证卡片
void Authenticate()
{
	//检测是否有新卡片
	if (!mfrc522.PICC_IsNewCardPresent() || !mfrc522.PICC_ReadCardSerial()) {
		delay(50);
		return;
	}

	//进行卡片验证
	for (byte num = 0; num < CARDS; num++)		//卡片循环  
	{
		byte i;
		for (i = 0; i < mfrc522.uid.size; i++)
		{
			if (mfrc522.uid.uidByte[i] != AuthedID[num][i]) break;
		}
		if (i == mfrc522.uid.size)
		{
			isAuthed = true;		//验证通过
			break;
		}
		else if (num == CARDS - 1) {
			BeepFailed();
		}
	}
}

//蓝牙扫描
void BTCheck() {
	if (BT.available()) {
		val = BT.read();
		if (val == '1') {
			OpenDoor();
			BeepChecked();
			delay(3000);
			isAuthed = false;
		}
		val = 0;
	}
}

void BeepChecked() {
	digitalWrite(BUZZ_PIN, LOW);
	delay(200);
	digitalWrite(BUZZ_PIN, HIGH);
}

void BeepFailed() {
	digitalWrite(BUZZ_PIN, LOW);
	delay(100);
	digitalWrite(BUZZ_PIN, HIGH);
	delay(100);
	digitalWrite(BUZZ_PIN, LOW);
	delay(100);
	digitalWrite(BUZZ_PIN, HIGH);
}