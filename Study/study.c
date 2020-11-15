#include <stdio.h>
#include <stdlib.h>
#include <wiringPi.h>
#include <softTone.h>
// 각 FND와 연결된 라즈베리파이 핀(S0, S1, …, S5)
const int FndSelectPin[6] = {4, 17, 18, 27, 22, 23};
// FND의 LED와 연결된 라즈베리파이 핀(A, B, …, H)
const int FndPin[8] = {6, 12, 13, 16, 19, 20, 26, 21};
// FND에 출력되는 문자 (0~9) 배열
const int FndFont[10] = {0x3F, 0x06, 0x5B, 0x4F, 0x66, 0x6D, 0x7D, 0x07, 0x7F, 0x67};
// 초음파 센서의 Trigger, Echo 핀
const int TP = 2; //Output
const int EP = 3; //Input
// Buzzer의 음역, 핀
const int melody[8] = {131,147,165,175,196,220,247,262};
const int BP = 5;
// 앉아있는지 여부 체크하는 Flag
int is_sit = 0;

// 초기화 함수, WiringPi 라이브러리 초기화, Select 핀 및 LED 핀 초기화를 담당)
void Init() {
    int i;
    if (wiringPiSetupGpio() == -1) {
        printf("wiringPiSetupGpio() error \n");
        exit(-1);
    }
    for (i = 0; i < 6; i++) {
        pinMode(FndSelectPin[i], OUTPUT);    // Select 핀을 출력으로 설정
        digitalWrite(FndSelectPin[i], HIGH); // Select 핀 OFF
    }
    for (i = 0; i < 8; i++) {
        pinMode(FndPin[i], OUTPUT);   // LED 핀을 출력으로 설정
        digitalWrite(FndPin[i], LOW); // LED 핀을 OFF
    }
    pinMode(TP, OUTPUT);
    pinMode(EP, INPUT);
    softToneCreate(BP);
}

// 초음파 센서를 이용해 Distance를 받아오는 함수
double getDistance() {
    double fDistance = 0.0;
    int nStartTime, nEndTime;
    nStartTime = nEndTime = 0;

    digitalWrite(TP, LOW);

    delayMicroseconds(10);
    digitalWrite(TP, HIGH);

    delayMicroseconds(10);
    digitalWrite(TP, LOW);

    while (digitalRead(EP) == LOW);
    nStartTime = micros();

    while (digitalRead(EP) == HIGH);
    nEndTime = micros();

    fDistance = (nEndTime - nStartTime) / 29. / 2.;
    
    return fDistance;
}

// FND를 선택하는 함수, S0 ~ S5 중 파라미터(position)에 해당하는 FND 선택
void FndSelect(int position) {
    int i;
    for (i = 0; i < 6; i++) {
        if (i == position) {
            digitalWrite(FndSelectPin[i], LOW); // 선택된 FND의 Select 핀 ON
        }
        else {
            digitalWrite(FndSelectPin[i], HIGH); // 선택되지 않은 FND의 Select 핀 OFF
        }
    }
}

// FND를 출력하는 함수
void FndDisplay(int position, int num) {
    int i, j;
    int flag = 0;     // FndPin[ ]을 ON/OFF
    int shift = 0x01; // FndFont와 And 연산하여 출력할 LED의 상태 결정
    for (i = 0; i < 8; i++) {
        flag = (FndFont[num] & shift); // i = 0, FndFont[ 0 ] = 0x3F라 하면 (0b00111111 & 0b00000100 = 1) 이다.
        digitalWrite(FndPin[i], flag); // FndPin[ ]을 flag( 0또는 1 )로 ON/OFF
        shift <<= 1;                   // 왼쪽으로 한 비트 쉬프트한다. I = 0이라 하면, ( shift = 0b00000001 )에서 ( shift = 0b00000010)로 변한다.
    }

    FndSelect(position);
}

int main()
{
    double fDistance;
    int i, pos, time;
    int data[6] = {0, 0, 0, 0, 0, 0}; // 출력할 문자 데이터
    Init();

    while (1) {
        fDistance = getDistance();

        // 초음파 센서 감지, 버튼 딩동댕
        if (fDistance < 25.0 && is_sit == 0) {
            is_sit = 1;
            softToneWrite(BP, melody[0]);
            delay(250);
            softToneWrite(BP, melody[2]);
            delay(250);
            softToneWrite(BP, melody[4]);
            delay(250);
            softToneWrite(BP, 0);
        }

        // 초음파 센서 감지, 버튼 댕동딩
        else if (fDistance > 30.0 && is_sit == 1) {
            is_sit = 0;
            softToneWrite(BP, melody[4]);
            delay(250);
            softToneWrite(BP, melody[2]);
            delay(250);
            softToneWrite(BP, melody[0]);
            delay(250);
            softToneWrite(BP, 0);
        }

        // FND
        if (is_sit == 1) {
            for (i = 0; i < 5; i++) {
                if(i==1||i==3) {
                    if (data[i] == 6) {
                        data[i + 1]++;
                        data[i] = 0;
                    }
                }
                else if (data[i] == 10) {
                    data[i + 1]++;
                    data[i] = 0;
                }
            }
            time = millis();
            while (millis() - time < 994) {         // 밑에 Delay 되는 6만큼 보정
                for (pos = 0; pos < 6; pos++) {
                    FndDisplay(pos, data[pos]);
                    delay(1);
                }
            }
            data[0]++;
        }
        else {
            for (pos = 0; pos < 6; pos++) {
                FndDisplay(pos, data[pos]);
                delay(1);
            }
        }
    }
    return 0;
}
