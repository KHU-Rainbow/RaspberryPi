# Rainbow - RPi

## Project Explanation
### 1. 초음파 센서
- 사용자 다리와 떨어져있는 거리 측정
- 책상 밑에 사용자의 무릎 높이에 설치
- 임계값 = 25cm 라고 설정 (RPi.c의 fDistance 값)
- 임계값을 통해 사용자가 앉은 상태 인식
### 2. FND 센서
- 누적된 공부 시간 시각화 기능 제공
- 사용자가 확인할 수 있도록 책상 위에 설치
- 자동으로 시간이 측정되는 '스탑워치' 기능 제공
- 측정 거리 < 임계값 → FND 시작
- 측정 거리 >= 임계값 → FND 멈춤
### 3. Buzzer 센서
- 시간의 누적 여부를 알려주는 청각적 알림 센서
- 측정 거리 < 임계값 → Buzzer 도미솔♬
- 측정 거리 >= 임계값 → Buzzer 솔미도♬

## Build Method
- **Basic Configuration**
```
sudo apt-get update
sudo apt-get upgrade
```
- **Install Git on RPi**
```
sudo apt-get install git
```
- **Install wiringPi on RPi**
```
+ wiringPi
- I2C, SPI, UART 등의 통신 제어 함수 제공
- GPIO 포트에 대한 설정 및 프로그래밍 인터페이스 제공

git clone https://github.com/WiringPi/WiringPi
mv WiringPi wiringPi
cd wiringPi
./build

```
- **Check wiringPi downloaded well**
```
gpio -v
```
- **Compile RPi.c and Execute**
```
make
./study
```
- **crontab을 통해 매일 0시 0분에 AWS RDS와 연동**
<img src="https://user-images.githubusercontent.com/20378368/99489838-f31f6700-29ab-11eb-8629-e8a91411307d.PNG" width="90%"></img>

# Rainbow - Packet

## Project Explanation
### 인터넷 사용 감지
- 사용자가 비행기 모드를 해제하고 네트워크를 사용하는 순간 알림.
- 핸드폰에서 주변 AP(Access Point)를 찾는 패킷인 Probe Request 패킷을 전송함.
- 네트워크 어댑터를 Monitor Mode로 전환하여 Probe Packet들을 캡쳐함.
- 캡쳐한 패킷들 중에서, 사용자의 핸드폰 MAC 주소와 일치하는 패킷을 골라냄.
- 해당 패킷의 RSSI (신호세기)값이 일정 수준 이상인 경우, 이를 DB에 전송함.

## Build Method
- **Basic Configuration for Network Adapter**
```
sudo ifconfig [어댑터명 ex)wlx9cefd5fbc93c] down
sudo ip link [어댑터명 ex)wlx9cefd5fbc93c] name wlan1

sudo iwconfig wlan1 mode monitor
sudo ifconfig wlan1 up

```
<img src="https://user-images.githubusercontent.com/58834907/97774821-8419d400-1b9e-11eb-9e95-7b389b2d0c2b.PNG" width="90%"></img>


- **Execute Program**
```
source ./dependencies.sh
sudo apt-get install libpcap0.8-dev
make
sudo ./packet <interface> <mac address>
sudo ./packet wlan1 50:50:A4:0E:16:90
```
<img src="https://user-images.githubusercontent.com/58834907/97774846-a0b60c00-1b9e-11eb-8aa7-966a3d615e45.PNG" width="90%"></img>


- **parser.cpp 내부의 curl을 이용해 감지될 때마다 AWS RDS와 연동**
```
CURL *curl;
CURLcode res;

std::string strTargetURL;
std::string strResourceJSON;
std::string s_today(today);

struct curl_slist *headerlist = nullptr;
headerlist = curl_slist_append(headerlist, "Content-Type: application/json");

strTargetURL = "https://r89kbtj8x9.execute-api.us-east-1.amazonaws.com/dev/rainbow-post-detect";
strResourceJSON = "{\"Packet_date\": \"" + s_today + "\", " + "\"Packet_time\": \"" + std::to_string(p_data) +"\"}";

curl_global_init(CURL_GLOBAL_ALL);
curl = curl_easy_init();

if (curl)
{
    curl_easy_setopt(curl, CURLOPT_URL, strTargetURL.c_str());
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headerlist);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, false);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, false);
    curl_easy_setopt(curl, CURLOPT_POST, 1L);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, strResourceJSON.c_str());
    
    res = curl_easy_perform(curl);
    
    curl_easy_cleanup(curl);
    curl_slist_free_all(headerlist);
}
```