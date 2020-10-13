#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>

/*********************************************/
/* 와이파이와 관련된 정의/변수들                */
/*********************************************/
#ifndef STASSID
#define STASSID "I LOVE YOU~~~"
#define STAPSK  "qsJobs1003"
#endif
const char* ssid     = STASSID;
const char* password = STAPSK;
const char* host = "192.168.43.15";
const uint16_t port = 7770;
ESP8266WiFiMulti WiFiMulti;
WiFiClient client;

/*********************************************/
/* Device/Unit과 관련된 정의/변수들             */
/*********************************************/
//#define UNIT_COUNT 2
//bool units_state[UNIT_COUNT] = {false, false};
//const int unit_control_pins[UNIT_COUNT] = {D1, D2};
//const int unit_toggle_button_pins[UNIT_COUNT] = {D3, D4};
//const char* device_id = "device_id_1";
//const char* device_type = "switch";
#define UNIT_COUNT 1
bool units_state[UNIT_COUNT] = {false};
const int unit_control_pins[UNIT_COUNT] = {D1};
const int unit_toggle_button_pins[UNIT_COUNT] = {D3};
String device_id = "device_id_1";
String device_type = "switch";

/*********************************************/
/* 함수 원형 선언                             */
/*********************************************/
// write_func(String) : TCP 서버 요청 보냄
bool write_func(String data);
// get_token(String, index) : 토큰을 가져옴
String get_token(String data, int index);

/*********************************************/
/* Setup() : PinMode 설정 및 Wifi 네트워크 설정 */
/*********************************************/
void setup() {
  // 시리얼 통신 속도 설정
  Serial.begin(115200);
  
  // Unit과 관련된 설정 - Pin mode 등 -
  int i = 0;
  for (i = 0; i < UNIT_COUNT; i++) {
    pinMode(unit_control_pins[i], OUTPUT);
  }
  for (i = 0; i < UNIT_COUNT; i++) {
    pinMode(unit_toggle_button_pins[i], INPUT);
  }
  
  // 와이파이 설정 및 연결 시작
  WiFi.mode(WIFI_STA);
  WiFiMulti.addAP(ssid, password);

  Serial.println();
  Serial.println();
  Serial.print("Wait for WiFi... ");

  while (WiFiMulti.run() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  delay(500);

  // TCP 서버와의 connnection을 생성함
  Serial.print("connecting to ");
  Serial.print(host);
  Serial.print(':');
  Serial.println(port);
  while(!client.connect(host, port)) {
    Serial.println("connection failed");
    Serial.println("wait 5 sec...");
    delay(5000);
  }

  // TCP 서버에 현재 디바이스에 대한 정보를 등록함
  String register_data = "OK;6;4;2;0;0;" + device_id + ";" + device_type + ";\n";
  if (write_func(register_data)) {
    String read_data = client.readStringUntil('\n');
    Serial.print("received : ");
    Serial.println(read_data);
    Serial.println("Registerd new device");  
  } else {
    Serial.println("Failed to register new device");
  }
}


/*********************************************/
/* loop() : 동작을 실질적으로 수행하는 메인 루프 */
/*********************************************/
void loop() {
  String read_data = client.readStringUntil('\n');
  if (read_data.length() > 0) {
    Serial.print("received : ");
    Serial.print(read_data);
    String unit_index = get_token(read_data, 4);
    bool on_off = get_token(read_data, 5) == "1";
    if (unit_index == "0") {
      units_state[0] = on_off;
      Serial.print("[0] ON_OFF : ");
      Serial.println(on_off);
    } else if (unit_index == "1") {
      units_state[1] = on_off;
      Serial.print("[1] ON_OFF : ");
      Serial.println(on_off);
    }
    String req_ok_data = "OK;6;4;2;0;0;" + device_id + ";" + device_type + ";\n";
    write_func(req_ok_data);
    String req_data_for_raspberry = "OK;8;4;3;" + get_token(read_data, 4) + ";" + get_token(read_data, 5) + ";" + get_token(read_data, 6) + ";" + get_token(read_data, 7) + ";" + get_token(read_data, 8) + ";" + get_token(read_data, 9) + ";\n";
    write_func(req_data_for_raspberry);
    Serial.print(req_data_for_raspberry + " & " + req_ok_data + " : ");
    Serial.println("req ok");
  }
  for (int i = 0; i < UNIT_COUNT; i++) {
    digitalWrite(unit_control_pins[i], units_state[i] ? HIGH : LOW);
  }
}

/*********************************************/
/* write_func(String) : TCP 서버 요청 보냄    */
/* Return Type : bool -> 요청 성공 여부       */
/*********************************************/
bool write_func(String data) {
  client.print(data);
  String read_data = client.readStringUntil('\n');
  Serial.println(read_data);
  
  String req_state = get_token(read_data, 0);
  String client_type = get_token(read_data, 2);
  return req_state == "OK" && client_type == "0";
}

/*********************************************/
/* get_token(String, index) : 토큰을 가져옴   */
/* Return Type : String -> 패킷을 분리한 토큰  */
/*********************************************/
String get_token(String data, int index) {
  int i = 0;
  int semicolon_index = 0;
  String data_copy = data;
  String ret = "";
  
  for (i = 0; i < index + 1; i++) {
    semicolon_index = data_copy.indexOf(";");
    if (semicolon_index != -1) {
      ret = data_copy.substring(0, semicolon_index);
      data_copy = data_copy.substring(semicolon_index + 1);
    } else {
      return data_copy;
    }
  }
  return ret;
}
