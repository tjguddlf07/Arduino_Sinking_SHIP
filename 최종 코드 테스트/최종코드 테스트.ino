/*Digital : 2,3 자이로 센서 (SCL,SDA)
  Analog  : 0,1,2 : 물감지 센서
  Digital : 내부 구조음 MP3 모듈
  XRST 22번
  XCS 23번 
  DREQ 24번
  XDCS 25번
  Digital : 외부 구조음 MP3 모듈
  XRST 28번
  XCS 29번
  DREQ 30번
  XDCS 31번
  Digital : 4,6 : 내부 스피커
  Digital : 5,7: 외부 스피커
  Digital : 8,9,10,11,12,13 : LED
  Analog  : 4,5 : LCD*/


#include <Wire.h>                   //I2C 통신 라이브라리
#include "I2Cdev.h"                 //I2c 통신(자이로 센서) 제어 라이브러리
#include "MPU9250.h"                //자이로 센서 제어 라이브러리
#include <LiquidCrystal_I2C.h>      //LiquidCrystal 디스플레이(LCD) 제어 라이브러리
LiquidCrystal_I2C lcd(0x3F,16,2);    //LCD객체 생성 (I2C 주소는 LCD에 맞게 수정)


MPU9250 accelGyroMag;             //자이로 센서 객체
int16_t AcX,AcY,AcZ;    
int16_t GyX,GyY,GyZ;
int16_t MgX,MgY,MgZ;
//9축 자이로 센서 계산 변수 값

double rollGyro,pitchGyro,yawZ;
double rollAcc,pitchAcc,yawXY;
double pitch=0, roll=0, yaw=0;
//자이로 센서 제어 계산식 초기화

float DT = 0.01;                     //계산식 형 변환  

int water_pin1=A0;
int water_pin2=A1;
int water_pin3=A2;
//물감지 센서 3개 선언 & 변수와 핀 연결  

int PLAYE1 = 4; //내부음
int PLAYE2 = 5; //외부음

int FT1 = 6;    //내부음 스피커
int FT2 = 7;    //외부음 스피커




int led[6]={8,9,10,11,12,13};       //디지털 핀 8,9,10,11,12,13을 led에 배열로 지정



int command; // 시리얼 모니터의 명령을 받아들이는 변수

void setup() {
  Wire.begin();                     //I2C통신 초기화 및 주소값 지정
  accelGyroMag.initialize();        //자이로 센서 초기화

  Serial.begin(9600);               //시리얼 모니터 설정

  pinMode(PLAYE1,OUTPUT);                       // playe를 출력으로
  pinMode(PLAYE2,OUTPUT);
  pinMode(FT1,OUTPUT);
  pinMode(FT2,OUTPUT);
  Serial.println("*********command*********");  // MP3 모듈 제어 명령
  Serial.println("1. record 10sec");            // 1. 녹음 10초
  Serial.println("2. play ");                   // 2. 재생

  
  for(int i=0;i<6;i++){             //LED 제어문
    pinMode(led[i],OUTPUT);
  }
  lcd.init();                       //I2C LCD 초기화
  lcd.backlight();                  //I2C LCD의 백라이트
}

void loop() {
  //방위각(0도 ~ 360도) 측정 
  accelGyroMag.getMotion9(&AcX,&AcY,&AcZ,&GyX,&GyY,&GyZ,&MgX,&MgY,&MgZ);           
  
  //Serial.print(MgX); Serial.print(" ");
  //Serial.print(MgY); Serial.print(" ");
  //Serial.print(MgZ); Serial.println(" ");
  
  //가속도 계산
  pitchAcc = atan2(AcY,AcZ)*180/PI; // 라디안 to 각도값
  rollAcc = -atan2(AcX,AcZ)*180/PI;
  
  //자이로 계산
  pitchGyro = GyX/131.07; // 16비트 값 to 250'/sec
  rollGyro = GyY/131.07;
  
  //상보필터
  pitch = (0.98*(pitch+(pitchGyro*DT)))+(0.02*pitchAcc);
  roll = (0.98*(roll+(rollGyro*DT)))+(0.02*rollAcc);
  
  //yaw 계산
  yaw = atan2(MgY,MgX)*180/PI+180;
  
  //Serial.print("pitch: "); Serial.print(pitch)
  //Serial.print(" roll: "); Serial.print(roll);
  //Serial.print(" yaw: "); Serial.println(yaw);

  if(yaw <= 20){                    //선박의 기울기 값이 20도 이하인 경우 
    lcd.setCursor(0,0);             //LCD 출력위치 Top Left  
    lcd.print("SAFE");              //LCD에 SAFE 출력
    delay(5000);                    //LCD 5초간 지연

  }else {                           //선박의 기울기 값이 20도를 넘는 경우
    lcd.clear();                    //LCD 모든 내용 삭제
    lcd.setCursor(0,0);             //LCD 출력위치 Top Left
    lcd.print("DANGER");            //LCD에 DANGER 출력
    delay(5000);                    //LCD 5초간 지연
    
    for(int i=0; i<6;i++){          //LED(에어백) 제어문              
    digitalWrite(led[i],HIGH);      //LED(에어백) 점등
    delay(5000);                    //LED 5초간 지연     
   }
  }

  int val1=analogRead(A0);          //수분수위센서값을 'val1'에 저장
  int val2=analogRead(A1);          //수분수위센서값을 'val1'에 저장
  int val3= analogRead(A2);         //수분수위센서값을 'Val3'에 저장
  
  //Serial.println(val1);         
  //Serial.println(val2);
  //Serial.println(val3);
  //센서 값을 시리얼 모니터에 출력
  //delay(100);
  
  if(0 < val1 && val1 <= 100){     //전방수위센서 값이 0~100이하인 경우
    lcd.setCursor(0,1);             //LCD 출력위치 Bottom Left  
    lcd.print("SAFE");              //LCD에 SAFE 출력
    delay(5000);                    //LCD 5초간 지연
    
 }else if(100 < val1 && val1 <= 150){//전방수위센서 값이 100~150이하인 경우
    lcd.clear();                    //LCD 모든 내용 삭제
    lcd.setCursor(0,1);             //LCD 출력 위치 Bottom Left 
    lcd.print("DANGER");            //LCD에 DANGER 출력
    delay(5000);                    //LCD 5초간 지연
    
    for(int i=0; i<6;i++){          //LED(에어백) 제어문              
    digitalWrite(led[i],HIGH);      //LED(에어백) 점등
    delay(5000);                    //LED 5초간 지연     
  }
 }else {                            //전방수위센서 값이 150을 초과하는 경우
    lcd.clear();                    //LCD 모든 내용 삭제
    lcd.setCursor(0,1);             //LCD 출력 위치 Bottom Left 
    lcd.print("EMERGENCY");         //LCD에 EMERGENCY 출력
    delay(5000);                    //LCD 5초간 지연

  for(int i=0; i<6;i++){            //LED(에어백) 제어문              
    digitalWrite(led[i],HIGH);      //LED(에어백) 점등
    delay(5000);                    //LED 5초간 지연     
  }    
 }
  if(0 < val2 && val2 <= 100){      //중앙수위센서 값이 0~100이하인 경우
    lcd.setCursor(0,1);             //LCD 출력위치 Bottom Left   
    lcd.print("SAFE");              //LCD에 SAFE 출력
    delay(5000);                    //LCD 5초간 지연
    
 }else if(100 < val2 && val2 <= 150){//중앙수위센서 값이 100~150이하인 경우
    lcd.clear();                    //LCD 모든 내용 삭제
    lcd.setCursor(0,1);             //LCD 출력 위치 Bottom Left 
    lcd.print("DANGER");            //LCD에 DANGER 출력
    delay(5000);                    //LCD 5초간 지연
    
    for(int i=0; i<6;i++){          //LED(에어백) 제어문              
    digitalWrite(led[i],HIGH);      //LED(에어백) 점등
    delay(5000);                    //LED 5초간 지연     
  }
 }else {                            //중앙수위센서 값이 150을 초과하는 경우
    lcd.clear();                    //LCD 모든 내용 삭제
    lcd.setCursor(0,1);             //LCD 출력 위치 Bottom Left 
    lcd.print("EMERGENCY");         //LCD에 EMERGENCY 출력
    delay(5000);                    //LCD 5초간 지연

  for(int i=0; i<6;i++){            //LED(에어백) 제어문              
    digitalWrite(led[i],HIGH);      //LED(에어백) 점등
    delay(5000);                    //LED 5초간 지연     
  }    
 }
  if(0 < val3 && val3 <= 100){       //전방수위센서 값이 0~100이하인 경우
    lcd.setCursor(0,1);             //LCD 출력위치 Bottom Left   
    lcd.print("SAFE");              //LCD에 SAFE 출력
    delay(5000);                    //LCD 5초간 지연
    
 }else if(100 < val3 && val3 <= 150){//후방수위센서 값이 100~150이하인 경우
    lcd.clear();                    //LCD 모든 내용 삭제
    lcd.setCursor(0,1);             //LCD 출력 위치 Bottom Left 
    lcd.print("DANGER");            //LCD에 DANGER 출력
    delay(5000);                    //LCD 5초간 지연
    
    for(int i=0; i<6;i++){          //LED(에어백) 제어문              
    digitalWrite(led[i],HIGH);      //LED(에어백) 점등
    delay(5000);                    //LED 5초간 지연     
  }
 }else {                            //후방수위센서 값이 150을 초과하는 경우
    lcd.clear();                    //LCD 모든 내용 삭제
    lcd.setCursor(0,1);             //LCD 출력 위치 Bottom Left 
    lcd.print("EMERGENCY");         //LCD에 EMERGENCY 출력
    delay(5000);                    //LCD 5초간 지연

  for(int i=0; i<6;i++){            //LED(에어백) 제어문              
    digitalWrite(led[i],HIGH);      //LED(에어백) 점등
    delay(5000);                    //LED 5초간 지연     
  }    
 } 
if(Serial.available()) {     // 시리얼 통신이 연결되어 있을 시  
    command=Serial.read();
    
    if(command == 'E') {
      digitalWrite(PLAYE1,HIGH);
      digitalWrite(PLAYE2,HIGH);
      
      //Serial.println("P1");
      //Serial.println("P2");
      delay(10000);  
      
      digitalWrite(PLAYE1,LOW);
      digitalWrite(PLAYE2,LOW);
      
  }else if(command == 'F'){
      digitalWrite(FT1,HIGH);
      digitalWrite(FT2,HIGH);
      
      //Serial.println("P1");
      //Serial.println("P2");
      delay(10000);
      
      digitalWrite(FT1,LOW);
      digitalWrite(FT2,LOW);   
  }else{
    Serial.println("Wrong input value");
  }
}
}
