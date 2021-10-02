/* Pinout do Circuito com Arduino

	* Pinos Digitais:
	0  -> ...............
	1  -> ...............
	2  -> Módulo Reprap LCD 12864 - RS
	3  -> Módulo Reprap LCD 12864 - RW
	4  -> Módulo Reprap LCD 12864 - Enable
	5  -> Rotary Encoder OutA
	6  -> Rotary Encoder OutB
	7  -> Rotary Encoder Button
	8  -> DHT22 Data
	9  -> Bomba Água
	10 -> Relé - Lâmpadas
	11 -> Sensor Nível 1
	12 -> Sensor Nível 2
	13 -> ...............
  44 -> Módulo Reprap LCD 12864 - Buzzer
  52 -> RX - ESP8266-01 (TX)
  53 -> TX - ESP8266-01 (RX)

	* Pinos Analógicos:
	A0 -> Leitura Sensor Umidade Solo
	A1 -> Leitura Sensor Luminosidade - LDR
	A2 -> ...............
	A3 -> ...............

  * Pinos de Comunicação - I2C
	20 -> I2C SDA - RTC
	21 -> I2C SCL - RTC

  * Memória EEPROM:
  Adresses:
  00: Tempo entre as medições (IoT)                         /Variável de Controle: interval
  01: Sensibilidade da Umidade do Solo                      /Variável de Controle: ctrlAutoSensibSM
  02: Sensibilidade do Sensor de Luz                        /Variável de Controle: ctrlAutoSensibLDR

  03: Controle Personalizado - Quais fatores                /Variável de Controle: ctrlPersonFatores

  04: Controle Personalizado - Domingo                      /Variável de Controle: selection_DoW [0]
  05: Controle Personalizado - Segunda                      /Variável de Controle: selection_DoW [1]
  06: Controle Personalizado - Terça                        /Variável de Controle: selection_DoW [2]
  07: Controle Personalizado - Quarta                       /Variável de Controle: selection_DoW [3]
  08: Controle Personalizado - Quinta                       /Variável de Controle: selection_DoW [4]
  09: Controle Personalizado - Sexta                        /Variável de Controle: selection_DoW [5]
  10: Controle Personalizado - Sábado                       /Variável de Controle: selection_DoW [6]

  11: Controle Personalizado - Manhã                        /Variável de Controle: selection_PeriodDia [0]
  12: Controle Personalizado - Tarde                        /Variável de Controle: selection_PeriodDia [1]
  13: Controle Personalizado - Noite                        /Variável de Controle: selection_PeriodDia [2]
  14: Controle Personalizado - Madrugada                    /Variável de Controle: selection_PeriodDia [3]

  15: Configuração Horário dos Períodos - Manhã HORAS       /Variável de Controle: manha_hour
  16: Configuração Horário dos Períodos - Manhã MINUTOS     /Variável de Controle: manha_minute
  17: Configuração Horário dos Períodos - Tarde HORAS       /Variável de Controle: tarde_hour
  18: Configuração Horário dos Períodos - Tarde MINUTOS     /Variável de Controle: tarde_minute
  19: Configuração Horário dos Períodos - Noite HORAS       /Variável de Controle: noite_hour
  20: Configuração Horário dos Períodos - Noite MINUTOS     /Variável de Controle: noite_minute
  21: Configuração Horário dos Períodos - Madrugada HORAS   /Variável de Controle: madrug_hour
  22: Configuração Horário dos Períodos - Madrugada MINUTOS /Variável de Controle: madrug_minute

  23: Tipo do Controle realizado pela Estuda - Controle Automático/Personalizado /Variável de Controle: ctrl_type

  25-49: Nome do SSID                                       /Variável de Controle: ssid
  50-74: Senha da rede                                      /Variável de Controle: password 
*/

/* Blibliotecas externas */
#include <Wire.h>
#include <DS3231.h>
#include <DHT.h>
#include "U8glib.h"
#include <RotaryEncoder.h>
#include "SoftwareSerial.h"
#include <EEPROM.h>

/* Funções de escrita e leitura da EEPROM */
void writeString (char add,String data);
String read_String (char add);

/* Variáveis dos Pinos Digitais de Entrada ou Leitura */
/* Criação do objeto do LCD */
U8GLIB_ST7920_128X64_1X u8g (4, 3, 2, 30); //Enable, RW, RS, RESET

/* Rotary Encoder - Configurações */
RotaryEncoder encoder (5, 6); //OutA, OutB
#define re_button 7
int re_buttonRead = 0;
int re_newPos     = 1;

/* DHT22 Sensor Temperatura e Umidade Ar - Configurações */
#define DHTPIN 8
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);
float h, t; // -> Humidity, Temperature

/* Sensores de Nível - Configurações */
const int snPins[] = {11, 12};
int snStates[]     = {0, 0};

/* IOT - ESP8266-01 */
SoftwareSerial esp(52, 53); // RX, TX

// Server Settings
String server = "krabifygamestudios-services.000webhostapp.com";
String uri = "/connEstufa.php";
String data;

// Wifi Settings (SSID)
String ssid = "";
String password = "";

// Connect to Wifi: Keyboard
int ssid_length = 0;
int password_length = 0;
int conWifiController = 0;
String ssid_Char = "";
String keyboard_header = "  Insira o nome da rede";
String Keyboard_string = "";
String conWifi_string = "";
String ssidDB_normal[] = 
{
  "1", "2", "3", "4", "5", "6", "7", "8", "9",
  "q", "w", "e", "r", "t", "y", "u", "i", "o", "p",
  "a", "s", "d", "f", "g", "h", "j", "k", "l",
  "z", "x", "c", "v", "b", "n", "m"
};
String ssidDB_shift[] = 
{
  "@", "#", "$", "_", "&", "-", "*", "!", "%",
  "Q", "W", "E", "R", "T", "Y", "U", "I", "O", "P",
  "A", "S", "D", "F", "G", "H", "J", "K", "L",
  "Z", "X", "C", "V", "B", "N", "M"
};

/* Pinos Analógicos */
/* Sensor Umidade Solo - Configurações */
#define soilMoistureSensorPin A0
int  sm_value;
bool sm_state;

/* Sensor Luminosidade - LDR */
#define ldrPin A1
int ldrValue;
int perLDR = 0;

/* Pinos Comunicação I2C */
/* Criação de objetos do RTC */
DS3231  rtc(SDA, SCL);

/* Pinos Digitais de Saída */
/* Bomba Água - Controle Digital */
#define bombaPin 9

/* Lâmpadas de Iluminação - Módulo Relé */
int minLux;
#define lampPin 10

/* Módulo Reprap LCD 12864 - Buzzer */
#define buzzer 44

/* Controles Internos */
/* Controle de Erros - Umidade Solo */
#define sm_offsetTime 30 // Tempo em segundos
int sm_timer;
int perSM = 0;
bool stopAll = false;

/* Controle da Interface do Usuário - LCD */
int display             = 0;
bool selectMenu         = false;
bool goBackToInfoScreen = false;
bool openControlMenu    = false;
bool openConfigMenu     = false;
bool openControlAuto    = false;
bool openProgFat        = false;
bool openHorPer         = false;
bool openIot            = false;
bool openSM             = false;
bool openSL             = false;
bool openDoW            = false;
bool openDoW2           = false;
bool openDoW3           = false;
bool openPeriodDia      = false;
bool openConWifi        = false;
bool openAddSSID_name   = false;
bool openKeyboard_Shift = false;
bool openAddPassword    = false;
bool openMedTemp        = false;

/* Controle da Interface do Usuário - Horários dos Períodos */
bool lockMenuRotaryEncoder = false;
int re_newPosHorPer = 0;
/*
 * 0 -> Manhã
 * 1 -> Tarde
 * 2 -> Noite
 * 3 -> Madrugada
 */
int HorPerID = 0;
//Manhã
int manha_hour = 6;
int manha_minute = 0;
//Tarde
int tarde_hour = 12;
int tarde_minute = 0;
//Noite
int noite_hour = 18;
int noite_minute = 0;
//Madrugada
int madrug_hour = 0;
int madrug_minute = 0;

/* Controle da Interface do Usuário - Tempo entre as medições */
unsigned long previousMillis = 0;
long interval = 300000; //800
int ledState = LOW;

/* Debug - ESP8266-01 no display */
String esp_message1 = "";

/* EEPROM - Variáveis de Controle */
  // Controle Automático - Sensibilidade Umidade do Solo
  // 0 -> Sensível
  // 1 -> Moderado
  // 2 -> Pouco sensível
  int ctrlAutoSensibSM = 0;

  // Controle Automático - Sensibilidade Iluminação
  // 0 -> Sensível
  // 1 -> Moderado
  // 2 -> Pouco sensível
  int ctrlAutoSensibLDR = 0;

  // Controle Personalizado - Quais Fatores:
  // 0 -> Irrigação
  // 1 -> Iluminação
  // 2 -> Irrigação e Iluminação
  int ctrlPersonFatores = 0;

  // Controle Personalizado - Dias da Semana:
  String Dow = "";
  bool selection_DoW[8];

  // Controle Personalizado - Períodos do Dia:
  String PeriodDia = "";
  bool selection_PeriodDia[5];

/* Controle da Interface do Usuário - Sinalização dos níveis de sensibilidade */
// 0 -> Controle Automático
// 1 -> Controle Personalizado
int ctrl_type = 0;
String text_SMSensibLevel  []  = {"", "", ""};
String text_LDRSensibLevel []  = {"", "", ""};

/* Controle da Interface do Usuário - Time RTC */
Time rtc_t;
int seconds;
int minutes;
int hours;

/* Controle Automático - Variável de controle do desligamento da lâmpada */
// Obs: se o usuário desligar a estufa, essa varíavel é resetada
String ca_day;
bool watering;

const uint8_t return_bitmap[] U8G_PROGMEM = {
  0x00,         // 00000000 
  0x08,         // 00001000 
  0x1c,         // 00011100
  0x3e,         // 00111110
  0x7f,         // 01111111
  0x1c,         // 00011100 
  0x7c,         // 01111100 
  0x7c          // 01111100 
};

const uint8_t graus_bitmap[] U8G_PROGMEM = {
  0x18,         // 00011000 
  0x24,         // 00100100 
  0x24,         // 00100100
  0x18,         // 00011000
  0x00,         // 00000000
  0x00,         // 00000000 
  0x00,         // 00000000 
  0x00          // 00000000  
};

const uint8_t CeresSoftware [] PROGMEM = {
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x07, 0xC0, 0x00, 0x00, 0x00, 0x00, 0x03, 0xFC, 0xC0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x1F, 0xE0, 0x00, 0x00, 0x00, 0x00, 0x03, 0xFC, 0xC0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x18, 0x70, 0x00, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x30, 0x30, 0xF0, 0xD8, 0x78, 0x3C, 0x03, 0x00, 0xCD, 0x9B, 0x9C, 0x63, 0x19, 0xF0, 0xD8, 0xF0,
0x30, 0x01, 0xF8, 0xFC, 0xFC, 0x7E, 0x03, 0xF8, 0xCF, 0xDF, 0xFE, 0x63, 0x1B, 0xF8, 0xFD, 0xF8,
0x30, 0x03, 0x0C, 0xC1, 0x86, 0x63, 0x03, 0xF8, 0xCC, 0x18, 0xC6, 0x27, 0x90, 0x38, 0xC3, 0x0C,
0x30, 0x23, 0xFC, 0xC1, 0xFE, 0x3C, 0x03, 0x00, 0xCC, 0x18, 0xC6, 0x37, 0xB1, 0xF8, 0xC3, 0xFC,
0x30, 0x33, 0x00, 0xC1, 0x80, 0x0F, 0x03, 0x00, 0xCC, 0x18, 0xC6, 0x34, 0xB3, 0x98, 0xC3, 0x00,
0x18, 0x73, 0x8C, 0xC1, 0xC6, 0x63, 0x03, 0x00, 0xCC, 0x18, 0xC6, 0x14, 0xA3, 0x18, 0xC3, 0x8C,
0x1F, 0xE1, 0xF8, 0xC0, 0xFC, 0x7F, 0x03, 0x00, 0xCC, 0x18, 0xC6, 0x1C, 0xE3, 0xF8, 0xC1, 0xF8,
0x07, 0xC0, 0xF0, 0xC0, 0x78, 0x3E, 0x03, 0x00, 0xCC, 0x18, 0xC6, 0x18, 0x61, 0xC8, 0xC0, 0xF0,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x00, 0xF0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0F, 0x01, 0xCC, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1F, 0x03, 0x8C, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x61, 0x0E, 0x03, 0x9C, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x73, 0x0E, 0x01, 0xF0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x73, 0x0E, 0x00, 0xF0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x77, 0x1C, 0x01, 0xF8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x76, 0x1C, 0x03, 0xB8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x74, 0x1C, 0x07, 0x38, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x7D, 0xB8, 0x37, 0x38, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3B, 0xB8, 0x77, 0x70, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x33, 0x30, 0x63, 0xC0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x80, 0x07, 0x70, 0x0C, 0x11, 0x00, 0x68, 0x00, 0x64, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x80, 0x02, 0x40, 0x04, 0x02, 0x00, 0x99, 0x00, 0x20, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0xE6, 0xC2, 0x8F, 0xE7, 0x37, 0xD8, 0x83, 0xB4, 0xEC, 0xC7, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x92, 0x43, 0x84, 0xA4, 0x92, 0x48, 0x71, 0x15, 0x25, 0x24, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x92, 0x82, 0x44, 0x64, 0x92, 0x50, 0x89, 0x15, 0x25, 0x23, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x92, 0x82, 0x24, 0xA4, 0x92, 0x50, 0xC9, 0x15, 0x25, 0x25, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0xE1, 0x07, 0x7E, 0xF7, 0x3F, 0x20, 0xB1, 0x8E, 0xFE, 0xC7, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x05, 0x00, 0x00, 0x00, 0x00, 0xA0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x06, 0x00, 0x00, 0x00, 0x00, 0xC0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

const uint8_t TelaPrincipalFrame [] PROGMEM = {
0x3F, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFC,
0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02,
0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01,
0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01,
0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01,
0x80, 0x1F, 0xC0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01,
0x80, 0x20, 0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0xF0, 0x00, 0x00, 0x00, 0x00, 0x01,
0x80, 0x35, 0x60, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x04, 0x18, 0x00, 0x00, 0x00, 0x00, 0x01,
0x80, 0x20, 0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x04, 0x28, 0x00, 0x00, 0x00, 0x00, 0x01,
0x80, 0x35, 0x60, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x04, 0xC8, 0x00, 0x00, 0x00, 0x00, 0x01,
0x80, 0x20, 0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x04, 0xC8, 0x00, 0x00, 0x00, 0x00, 0x01,
0x80, 0x35, 0x60, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x04, 0x08, 0x00, 0x00, 0x00, 0x00, 0x01,
0x80, 0x20, 0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x04, 0x08, 0x00, 0x00, 0x00, 0x00, 0x01,
0x80, 0x1F, 0xC0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0xF0, 0x00, 0x00, 0x00, 0x00, 0x01,
0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01,
0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01,
0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01,
0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01,
0x80, 0x07, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01,
0x80, 0x08, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01,
0x80, 0x08, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x30, 0x00, 0x00, 0x00, 0x00, 0x01,
0x80, 0x0A, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x48, 0x00, 0x00, 0x00, 0x00, 0x01,
0x80, 0x0A, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x48, 0x00, 0x00, 0x00, 0x00, 0x01,
0x80, 0x0A, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x30, 0x00, 0x00, 0x00, 0x00, 0x01,
0x80, 0x0A, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x04, 0x80, 0x00, 0x00, 0x00, 0x00, 0x01,
0x80, 0x0A, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x04, 0xB0, 0x00, 0x00, 0x00, 0x00, 0x01,
0x80, 0x0A, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x48, 0x00, 0x00, 0x00, 0x00, 0x01,
0x80, 0x12, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x48, 0x00, 0x00, 0x00, 0x00, 0x01,
0x80, 0x15, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x30, 0x00, 0x00, 0x00, 0x00, 0x01,
0x80, 0x12, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01,
0x80, 0x08, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01,
0x80, 0x07, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01,
0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01,
0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01,
0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01,
0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xA0, 0x00, 0x00, 0x00, 0x00, 0x01,
0x80, 0x00, 0x70, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xA0, 0x00, 0x00, 0x00, 0x00, 0x01,
0x80, 0x00, 0x88, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0xF0, 0x00, 0x00, 0x00, 0x00, 0x01,
0x80, 0x0F, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x08, 0x00, 0x00, 0x00, 0x00, 0x01,
0x80, 0x11, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x48, 0x00, 0x00, 0x00, 0x00, 0x01,
0x80, 0x20, 0xC4, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0xA8, 0x00, 0x00, 0x00, 0x00, 0x01,
0x80, 0x40, 0x28, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0xB8, 0x00, 0x00, 0x00, 0x00, 0x01,
0x81, 0xC0, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0xA8, 0x00, 0x00, 0x00, 0x00, 0x01,
0x9E, 0x11, 0x18, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0xB8, 0x00, 0x00, 0x00, 0x00, 0x01,
0xA0, 0x40, 0x44, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0xA8, 0x00, 0x00, 0x00, 0x00, 0x01,
0xA0, 0x3F, 0x82, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0xB8, 0x00, 0x00, 0x00, 0x00, 0x01,
0x90, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0xA8, 0x00, 0x00, 0x00, 0x00, 0x01,
0x8F, 0xFF, 0xFC, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x10, 0x00, 0x00, 0x00, 0x00, 0x01,
0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01,
0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01,
0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01,
0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01,
0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01,
0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01,
0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01,
0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01,
0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01,
0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01,
0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01,
0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01,
0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01,
0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01,
0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02,
0x3F, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFC
};

void u8g_prepare() 
{  
  u8g.setFont(u8g_font_5x8);  //u8g_font_5x8 //u8g_font_profont10
  //u8g.setFontRefHeightExtendedText();  
  u8g.setDefaultForegroundColor();  
  u8g.setFontPosTop();  
}

void setup ()
{
	/* Inicializações */
	Serial.begin (9600);
  esp.begin(115200);
  rtc.begin();
	dht.begin();

	/* Configurações */
  //rtc.setDOW(TUESDAY);          // Set Day-of-Week to SUNDAY
  //rtc.setTime(17, 9, 36);      // Set the time to 12:00:00 (24hr format)
  //rtc.setDate(8, 10, 2019);     // Set the date to January 1st, 2014
  digitalWrite (lampPin, 1); 
	if      (u8g.getMode() == U8G_MODE_R3G3B2)    u8g.setColorIndex(255);         // white    
	else if (u8g.getMode() == U8G_MODE_GRAY2BIT)  u8g.setColorIndex(1);           // max intensity  
	else if (u8g.getMode() == U8G_MODE_BW)        u8g.setColorIndex(1);           // pixel on  
  interval = EEPROM.read (0) * 60000;

  /* Configuração dos Pinos */
  for (int i = 0; i < 2; i++)
    pinMode (snPins[i], INPUT);
  pinMode (bombaPin, OUTPUT);
  pinMode (lampPin, OUTPUT);
  pinMode (buzzer, OUTPUT);
  pinMode (re_button, INPUT);
  digitalWrite (re_button, HIGH);
  pinMode (13, OUTPUT);

	Serial.println("Seja Bem-Vindo à Interface do Usuário!");
  encoder.setPosition (1);

  /* Assimilar as informações da EEPROM com as variáveis globais */
  // Sensibilidade Umidade do Solo
  ctrlAutoSensibSM = EEPROM.read (1);
  for (int i = 0; i < 3; i++)
  {
    if (i == ctrlAutoSensibSM)
      text_SMSensibLevel [ctrlAutoSensibSM] = "*";
  }

  // Sensibilidade Luminosidade
  ctrlAutoSensibLDR = EEPROM.read (2);
  for (int i = 0; i < 3; i++)
  {
    if (i == ctrlAutoSensibLDR)
      text_LDRSensibLevel [ctrlAutoSensibLDR] = "*";
  }

  // Controle Personalizado - Fatores a serem personalizados
  ctrlPersonFatores = EEPROM.read (3);

  // Horário dos Períodos
  if (EEPROM.read (15) != 0)
  {
    manha_hour = EEPROM.read (15);
    manha_minute = EEPROM.read (16);
  }

  if (EEPROM.read (17) != 0)
  {
    tarde_hour = EEPROM.read (17);
    tarde_minute = EEPROM.read (18);
  }

  if (EEPROM.read (19) != 0)
  {
    noite_hour = EEPROM.read (19);
    noite_minute = EEPROM.read (20);
  }

  if (EEPROM.read (21) != 0)
  {
    madrug_hour = EEPROM.read (21);
    madrug_minute = EEPROM.read (22);
  }

  // Nome e senha da rede
  ssid = read_String(25);
  password = read_String(50);

  /* Área de Testes */
  // Exibir o tipo de Controle da Estufa
  Serial.println (EEPROM.read (23));

  // Exibir o Logo e Versão do Firmware
  display = -1;
  UpdateLCDDisplay ();
  delay (5000);
  display = 0;
  UpdateLCDDisplay ();
}

bool connected;
void loop ()
{
  /* Leitura dos Sensores */
	sm_value = analogRead (soilMoistureSensorPin); // Sensor Umidade do Solo 
	ldrValue = analogRead (ldrPin);                // Sensor Luminosidade - LDR
	h = dht.readHumidity();                        // Leitura da umidade (%)
	t = dht.readTemperature();                     // Leitura da temperatura (Celsius)
	snStates[0] = !digitalRead (snPins[0]);        // Leitura do Estado do Sensor de Nível 1
	snStates[1] = digitalRead (snPins[1]);         // Leitura do Estado do Sensor de Nível 2
  re_buttonRead = digitalRead(re_button);        // Leitura do Estado do Botão do Rotary Encoder
  rtc_t = rtc.getTime();
  hours = rtc_t.hour;
  minutes = rtc_t.min;
  seconds = rtc_t.sec;

  perLDR = map (ldrValue, 0, 1023, 0, 100);
  perSM  = (100 - map (sm_value, 0, 1023, 0, 100));

  /* Controle de erros */
  /*if (stopAll)
  {
    //lcd.setCursor (0,3);
    //lcd.print ("Erro emergencial");
    return;
  }

  // Verifica se o sensor DHT esta respondendo
	if (isnan(h) || isnan(t))
	{
		Serial.println("Falha ao ler dados do sensor DHT !!!");
	}*/

  /* Condições de ações para controle da Interface do Usuário */
	// Verifica se o botão do encoder foi pressionado
	if (re_buttonRead != 1)
	{
    /* Selecionar o Menu da Interface do Usuário */
    if (!selectMenu)
    {
      TickBuzzer ();
      Serial.println ("Entrando no Options Menu");
      selectMenu = true;
      display = 1;

      UpdateLCDDisplay ();
    }
    else if (selectMenu && goBackToInfoScreen)
    {
      TickBuzzer ();
      Serial.println ("Saindo do Options Menu");

      selectMenu = false;
      goBackToInfoScreen = false;
      display = 0;

      UpdateLCDDisplay ();
    }

    // Se o usuário estiver dentro das Opções de Menu, verificar se:
    if (selectMenu)
    {
      for (int i = 0; i < 8; i++)
      {
        SelectDoWVerification (35 + i, i);
      }
      for (int i = 0; i < 5; i++)
      {
        SelectPeriodDiaVerification (44 + i, i);
      }

      MenuBtnMenuAcessControl (2, 4,  openControlMenu);
      MenuBtnMenuAcessControl (3, 7,  openConfigMenu);
      MenuBtnMenuAcessControl (5, 10, openControlAuto);
      MenuBtnMenuAcessControl (6, 13, openProgFat);
      MenuBtnMenuAcessControl (8, 17, openHorPer);
      MenuBtnMenuAcessControl (9, 22, openIot);
      MenuBtnMenuAcessControl (11, 26, openSM);
      MenuBtnMenuAcessControl (12, 30, openSL);
      MenuBtnMenuAcessControl (14, 34, openDoW);
      MenuBtnMenuAcessControl (15, 34, openDoW2);
      MenuBtnMenuAcessControl (16, 34, openDoW3);
      MenuBtnMenuAcessControl (24, 49, openConWifi);
      MenuBtnMenuAcessControl (50, 52, openAddSSID_name);
      MenuBtnMenuAcessControl (89, 91, openKeyboard_Shift);
      MenuBtnMenuAcessControl (25, 128, openMedTemp);

      // Controle Automático: Especificar os nível de sensibilidade da Umidade do Solo & Luminosidade
      for (int i = 0; i < 3; i++)
      {
        if (display == (27 + i))
        {
          // Primeiramente resetar os valores do texto para não haver conflitos
          for (int g = 0; g < 3; g++)
            text_SMSensibLevel [g] = "";

          ctrlAutoSensibSM = i;
          EEPROM.write (1, ctrlAutoSensibSM);
          text_SMSensibLevel [ctrlAutoSensibSM] = "*";

          delay (10);

          ctrl_type = 0;
          EEPROM.write (23, ctrl_type);

          UpdateLCDDisplay ();
        }
        else if (display == (31 + i))
        {
          for (int g = 0; g < 3; g++)
            text_LDRSensibLevel [g] = "";

          ctrlAutoSensibLDR = i;
          EEPROM.write (2, ctrlAutoSensibLDR);
          text_LDRSensibLevel [ctrlAutoSensibLDR] = "*";

          delay (10);

          ctrl_type = 0;
          EEPROM.write (23, ctrl_type);

          UpdateLCDDisplay ();
        }
      }

      // Para especificar se é Irrigação/Luminação/Irrigação & Luminação
      if (openDoW && !openPeriodDia)
        ctrlPersonFatores = 0;
      else if (openDoW2 && !openPeriodDia)
        ctrlPersonFatores = 1;
      else if (openDoW3 && !openPeriodDia)
        ctrlPersonFatores = 2;

      // Caso especial: de DoW para Period Dia - Verificação se Dow for vazio
      if (display == 42 && !openPeriodDia && Dow != "")
      {
        openPeriodDia = true;
        encoder.setPosition (43);
        display = 43;

        UpdateLCDDisplay ();
        TickBuzzer ();
      }
      else if (display == 43 && openPeriodDia)
      {
        openPeriodDia = false;
        encoder.setPosition (42);
        display = 42;

        UpdateLCDDisplay ();
        TickBuzzer ();
      }

      // Fora do padrão: terminar a configuração do controle personalizado e voltar para de volta e resetar as variáveis criadas
      if (display == 48 && PeriodDia != "" && openProgFat && (openDoW || openDoW2 || openDoW3) && openPeriodDia)
      {
        openProgFat = false;
        openDoW = false;
        openDoW2 = false;
        openDoW3 = false;
        openPeriodDia = false;

        Dow = "";
        PeriodDia = "";

        // Resetar as informações de DoW e Período do Dia selecionados anteriormente
        for (int i = 0; i < 11; i++)
          EEPROM.write ((4 + i), 0);

        delay (10);

        // Salvar informações na EEPROM
        EEPROM.write (3, ctrlPersonFatores);
        for (int i = 0; i < 7; i++)
          EEPROM.write ((4 + i), selection_DoW [i]);
        for (int i = 0; i < 4; i++)
          EEPROM.write ((11 + i), selection_PeriodDia [i]);

        // Definir o Tipo de Controle: Personalizado
        delay (10);

        ctrl_type = 1;
        EEPROM.write (23, ctrl_type);

        Serial.println ("==================================");

        for (int i = 0; i < 8; i++)
        {
          selection_DoW[i] = false;
          Serial.println (selection_DoW[i]);
        }

        Serial.println ("==================================");

        for (int i = 0; i < 5; i++)
        {
          selection_PeriodDia[i] = false;
          Serial.println (selection_PeriodDia[i]);
        }

        encoder.setPosition (6);
        display = 6;

        UpdateLCDDisplay ();
        TickBuzzer ();
      }

      // Mudar os Horários dos Períodos
      for (int i = 0; i < 4; i++)
        ButtonHorPerFunction ((18 + i), i);

      // Wifi Module IoT: Reset Module & configs
      if (display == 23)
      {
        Serial.println ("IoT - Reset...");
        Reset ();
        // Zerar os valores do nome do SSID e da senha
        writeString(25, ""); // SSID
        writeString(50, ""); // Password
      }
      // Conectar a rede (Se os dados da rede estiverem preenchidos)
      else if (display == 51)
      {
        String rcvDataSSID;
        rcvDataSSID = read_String(25);

        String rcvDataPassword;
        rcvDataPassword = read_String(50);

        if (rcvDataSSID != "")
          ssid = rcvDataSSID;
        else
        {
          conWifi_string = "Rede nao config";
          UpdateLCDDisplay ();
          return;
        }

        if (rcvDataPassword != "")
          password = rcvDataPassword;
        else
        {
          conWifi_string = "Rede nao config";
          UpdateLCDDisplay ();
          return;
        }

        if (ssid != "" && password != "" && !connected)
        {
          conWifi_string = "Conectando...";
          UpdateLCDDisplay ();
          Reset();
          delay(2000);
          ConnectWifi(51);
        }
        else if (connected)
        {
          conWifi_string = "Ja conectado";
          UpdateLCDDisplay ();
        }
      }

      // Wifi Teclado
      for (int i = 0; i < 38; i++)
      {
        // Teclado Normal
        if (!openKeyboard_Shift)
        {
          // Continue Button
          if (display == 90 && (53 + i) == 90)
          {
            // Terminando de escolher o nome do SSID
            if (conWifiController == 0 && ssid != "")
            {
              keyboard_header = "     Insira a senha";
              conWifiController = 1;
              Keyboard_string = password;
              UpdateLCDDisplay ();
            }
            // Terminando de escolher a Senha da Rede
            else if (conWifiController == 1 && password != "")
            {
              Serial.println (ssid);
              Serial.println (password);

              // Resetar configs do teclado
              conWifiController = 0;
              Keyboard_string = ssid;
              keyboard_header = "  Insira o nome da rede";

              // Salvar os valores na EEPROM
              writeString(25, ssid);  //Address 25 and String type data
              delay(10);
              writeString(50, password);  //Address 50 and String type data
              delay(10);

              String recivedData;
              recivedData = read_String(25);
              Serial.print("SSID Data:");
              Serial.println(recivedData);

              String recivedData2;
              recivedData2 = read_String(50);
              Serial.print("Password Data:");
              Serial.println(recivedData2);

              delay (1000);

              Reset();
              delay(2000);
              ConnectWifi(50);
            }
          }
          // Backspace
          else if (display == 62 && (53 + i) == 62)
          {
            if (conWifiController == 0 && ssid != "")
              ConnectWifi_SSIDPasswordStringFunctions (conWifiController, 1);
            else if (conWifiController == 1 && password != "")
              ConnectWifi_SSIDPasswordStringFunctions (conWifiController, 1);
          }
          // Demais chars
          else if (display == (53 + i) && display != 62 && display != 89 && display != 90)
          {
            ConnectWifi_SSIDPasswordStringFunctions (conWifiController, 0);
          }
        }
        // Teclado Shift
        else if (openKeyboard_Shift)
        {
          // Backspace
          if (display == 101 && (92 + i) == 101)
          {
            if (conWifiController == 0 && ssid != "")
              ConnectWifi_SSIDPasswordStringFunctions (conWifiController, 1);
            else if (conWifiController == 1 && password != "")
              ConnectWifi_SSIDPasswordStringFunctions (conWifiController, 1);
          }
          // Demais chars
          else if (display == (92 + i) && display != 91 && display != 101)
          {
            ConnectWifi_SSIDPasswordStringFunctions (conWifiController, 0);
          }
        }
      }

      // Button: Tempo entre medições
      for (int i = 0; i < 6; i++)
      {
        if (display == (129 + i))
        {
          switch (i)
          {
            case 0:
              interval = 120000;  // 2 minutos
              break;
            case 1:
              interval = 300000;  // 5 minutos
              break;
            case 2:
              interval = 600000;  // 10 minutos
              break;
            case 3:
              interval = 1800000; // 30 minutos
              break;
            case 4:
              interval = 3600000; // 60 minutos
              break;
            case 5:
              int value = interval/60000;
              EEPROM.write (0, value);

              Serial.println (EEPROM.read (0));
              break;
          }

          Serial.println (interval/60000);
          UpdateLCDDisplay ();
        }
      }
    }

		while (digitalRead(re_button) == 0)
	  		delay(10);
	}

  // Se estiver no menu (loop)
  if (selectMenu)
  {
    static int pos = 0;
    static int position = 0;

    if (!lockMenuRotaryEncoder)
    {
      ReadAndUpdateRotaryEncoderPosition ();
      position = 0;
    }
    else if (lockMenuRotaryEncoder)
    {
      encoder.tick();
      re_newPosHorPer = encoder.getPosition();

      if (position != re_newPosHorPer)
      {
        switch (HorPerID)
        {
          case 0:
            UpdateHorPerVariables (manha_hour, manha_minute, 6, 11, position);
            break;
          case 1:
            UpdateHorPerVariables (tarde_hour, tarde_minute, 12, 17, position);
            break;
          case 2:
            UpdateHorPerVariables (noite_hour, noite_minute, 18, 23, position);
            break;
          case 3:
            UpdateHorPerVariables (madrug_hour, madrug_minute, 0, 5, position);
            break;
        }

        position = re_newPosHorPer;
        UpdateLCDDisplay ();
      }
    }

    //Se a posicao foi alterada, mostra o valor no Serial Monitor
    if (pos != display && !lockMenuRotaryEncoder)
    {
      if (selectMenu)
      {
        // Se estiver dentro da tela de opções
        if (!openConfigMenu && !openControlMenu &&  !openControlAuto                    && 
            !openProgFat    && !openHorPer      &&  !openIot                            && 
            !openSM         && !openSL          && (!openDoW && !openDoW2 && !openDoW3) &&
            !openPeriodDia)
        {
          SetMaxMinMenuEncoderPosition (1, 3);
        }
        else if (openControlMenu && !openControlAuto && !openProgFat && !openSM && !openSL && (!openDoW && !openDoW2 && !openDoW3) && !openPeriodDia)
        {
          SetMaxMinMenuEncoderPosition (4, 6);
        }
        else if (openConfigMenu && !openHorPer && !openIot && !openConWifi && !openAddSSID_name && !openKeyboard_Shift && !openMedTemp)
        {
          SetMaxMinMenuEncoderPosition (7, 9);
        }
        else if (openControlAuto && !openSM && !openSL)
        {
          SetMaxMinMenuEncoderPosition (10, 12);
        }
        else if (openProgFat && (!openDoW && !openDoW2 && !openDoW3) && !openPeriodDia)
        {
          SetMaxMinMenuEncoderPosition (13, 16);
        }
        else if (openHorPer)
        {
          SetMaxMinMenuEncoderPosition (17, 21);
        }
        else if (openIot && !openConWifi && !openAddSSID_name && !openKeyboard_Shift && !openMedTemp)
        {
          SetMaxMinMenuEncoderPosition (22, 25);
        }
        else if (openSM)
        {
          SetMaxMinMenuEncoderPosition (26, 29);
        }
        else if (openSL)
        {
          SetMaxMinMenuEncoderPosition (30, 33);
        }
        else if ((openDoW || openDoW2 || openDoW3) && !openPeriodDia)
        {
          SetMaxMinMenuEncoderPosition (34, 42);
        }
        else if (openPeriodDia)
        {
          SetMaxMinMenuEncoderPosition (43, 48);
        }
        else if (openConWifi && !openAddSSID_name && !openKeyboard_Shift && !openMedTemp)
        {
          SetMaxMinMenuEncoderPosition (49, 51);
        }
        else if (openAddSSID_name && !openKeyboard_Shift)
        {
          SetMaxMinMenuEncoderPosition (52, 90);
        }
        else if (openKeyboard_Shift)
        {
          SetMaxMinMenuEncoderPosition (91, 127);
        }
        else if (openMedTemp)
        {
          SetMaxMinMenuEncoderPosition (128, 134);
        }
      }

      Serial.print ("Display Pos: ");
      Serial.println (display);

      pos = display;
      UpdateLCDDisplay ();
    }
  }
  else if (!selectMenu)
  {
    //Serial.println ("UpdateLCDDisplay");
    UpdateLCDDisplay ();
  }

  // Exclusivo para enviar os dados para Internet (IoT)
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval) 
  {
    // save the last time you blinked the LED
    previousMillis = currentMillis;

    if (connected)
    {
      Serial.println ("httppost();");
      esp_message1 = "enviando dados...";
      UpdateLCDDisplay ();

      String temperature  = String (t);
      String humidity     = String (h);
      
      String ldrLux       = String (perLDR);
      String soilMoisture = String (perSM);
      data = "temp=" + temperature + "&hum_ar=" + humidity + "&lux=" + ldrLux + "&hum_solo=" + soilMoisture;
      Serial.println (data);
      httppost();
    }
  }

  if (!connected)
    esp_message1 = "offline...";
  else 
    esp_message1 = "online";

  /* Determinar se o Controle é automático ou personalizado */
  switch (EEPROM.read (23))
  {
    // Controle Automático
    case 0:
    {
      /* Verificações de Emergencia */
      // Erro de lógica com os sensores de nível do reservatório. Abortar processo imediatamente!
      if (!snStates[0] && snStates[1])
      {
        //esp_message1 = "Erro, reinicie! Cod:500";
        digitalWrite (bombaPin, 0);
      }

      /* Umidade do Solo */
      // Determinar a sensibilidade do sensor e verificar quando o solo estiver seco
      for (int i = 0; i < 3; i++)
      {
        if (ctrlAutoSensibSM == i)
          SoilMoistureVerification (i);
      }

      // Se o solo estiver seco, ligar a bomba até o solo ficar úmido
      if (snStates[0] == 1 && !sm_state)
      {
        sm_timer++;
        delay (1000);

        if (sm_timer < sm_offsetTime)
        {
          digitalWrite (bombaPin, 1);
        }
        else
        {
          sm_timer = 0;
          stopAll = true;
          digitalWrite (bombaPin, 0);
        }
      }
      else
      {
        sm_timer = 0;
        digitalWrite (bombaPin, 0);
      }

      /* Iluminação */
      // Determinar a sensibilidade do sensor e verificar quando o ambiente estiver escuro
      for (int i = 0; i < 3; i++)
      {
        if (ctrlAutoSensibLDR == i)
          LDRLuxVerification (i);
      }

      // Verificar se o valor mínimo de Luz está configurado, senão alertar erro
      if (minLux > 0)
      {
        // Ligar o relé quando o LDR for menor que o valor da sensibilidade
        // Lembrar de colocar mais uma verificação aki, sobre o dia
        if (ldrValue < minLux)
        {
          // Ligar o relé
          digitalWrite (lampPin, 0);
          ca_day = rtc.getDOWStr(FORMAT_SHORT);
        }
        // Desligar o relé somente, quando alterar de dia e o valor do LDR for maior que o mínimo de luz requisitado
        else if (ldrValue >= minLux && ca_day != rtc.getDOWStr(FORMAT_SHORT))
        {
          // Desligar o relé
          digitalWrite (lampPin, 1);
          ca_day = "";
        }
      }
      else
      {
        Serial.println ("ERRO! variável: minLux está com valor nulo!");
      }
      
      break;
    }

    // Controle Personalizado
    case 1:
    {
      /* Para os controles singulares */
      switch (ctrlPersonFatores)
      {
        case 0:
          /* Iluminação Automático */
          // Determinar a sensibilidade do sensor e verificar quando o ambiente estiver escuro
          for (int i = 0; i < 3; i++)
          {
            if (ctrlAutoSensibLDR == i)
              LDRLuxVerification (i);
          }

          // Verificar se o valor mínimo de Luz está configurado, senão alertar erro
          if (minLux > 0)
          {
            // Ligar o relé quando o LDR for menor que o valor da sensibilidade
            // Lembrar de colocar mais uma verificação aki, sobre o dia
            if (ldrValue < minLux)
            {
              // Ligar o relé
              digitalWrite (lampPin, 0);
              ca_day = rtc.getDOWStr(FORMAT_SHORT);
            }
            // Desligar o relé somente, quando alterar de dia e o valor do LDR for maior que o mínimo de luz requisitado
            else if (ldrValue >= minLux && ca_day != rtc.getDOWStr(FORMAT_SHORT))
            {
              // Desligar o relé
              digitalWrite (lampPin, 1);
              ca_day = "";
            }
          }
          else
          {
            Serial.println ("ERRO! variável: minLux está com valor nulo!");
          }
          break;
        case 1:
          /* Umidade do Solo Automático */
          // Determinar a sensibilidade do sensor e verificar quando o solo estiver seco
          for (int i = 0; i < 3; i++)
          {
            if (ctrlAutoSensibSM == i)
              SoilMoistureVerification (i);
          }

          // Se o solo estiver seco, ligar a bomba até o solo ficar úmido
          if (snStates[0] == 1 && !sm_state)
          {
            sm_timer++;
            delay (1000);

            if (sm_timer < sm_offsetTime)
            {
              digitalWrite (bombaPin, 1);
            }
            else
            {
              sm_timer = 0;
              stopAll = true;
              digitalWrite (bombaPin, 0);
            }
          }
          else
          {
            sm_timer = 0;
            digitalWrite (bombaPin, 0);
          }
          break;
      }

      CtrlPersoFunction ();

      break;
    }
  }
}

void SoilMoistureVerification (int level)
{
  // Determinar o level da sensibilidade do sensor
  int min = 0;
  int max = 0;
  switch (level)
  {
    // Sensível
    case 0:
      min = 500;
      max = 650;
      break;
    // Moderado
    case 1:
      min = 400;
      max = 750;
      break;
    // Pouco sensível
    case 2:
      min = 300;
      max = 900;
      break;
  }

	if (sm_value > 0 && sm_value < min)
	{
		//Serial.println ("Solo Umido!");
		sm_state = true;
	}
	else if (sm_value > min && sm_value < max)
	{
		//Serial.println ("Solo Moderadamente Umido!");
		sm_state = true;
	}
	else if (sm_value > max && sm_value < 1024)
	{
		//Serial.println ("Solo Seco!");
		sm_state = false;
	}
}

void CtrlPersoIrrig ()
{
  if (snStates[0] == 1 && !watering)
  {
    sm_timer++;
    delay (1000);

    if (sm_timer < sm_offsetTime)
    {
      digitalWrite (bombaPin, 1);
    }
    else
    {
      Serial.println ("PARA!");
      watering = true;
      sm_timer = 0;
      stopAll = true;
      digitalWrite (bombaPin, 0);
    }
  }
  else
  {
    sm_timer = 0;
    digitalWrite (bombaPin, 0);
  }
}

void LDRLuxVerification (int level)
{
  // Determinar o level da sensibilidade do sensor
  switch (level)
  {
    // Sensível
    case 0:
      minLux = 450;
      break;
    // Moderado
    case 1:
      minLux = 300;
      break;
    // Pouco sensível
    case 2:
      minLux = 150;
      break;
  }
}

void CtrlPersoLumin ()
{
  // Ligar o relé
  if (ca_day == "")
  {
    digitalWrite (lampPin, 0);
    ca_day = rtc.getDOWStr(FORMAT_SHORT);
  }
  // Desligar o relé somente, quando alterar de dia e o valor do LDR for maior que o mínimo de luz requisitado
  else if (ldrValue >= minLux && ca_day != rtc.getDOWStr(FORMAT_SHORT))
  {
    // Desligar o relé
    digitalWrite (lampPin, 1);
    ca_day = "";
  }
}

void CtrlPersoFunction ()
{
  // Determinar o dia 
  String f_day; 
  for (int i = 4; i < 11; i++)
  {
    switch (i)
    {
    case 4:
      f_day = "Sun";
      break;
    case 5:
      f_day = "Mon";
      break;
    case 6:
      f_day = "Tue";
      break;
    case 7:
      f_day = "Wed";
      break;
    case 8:
      f_day = "Thu";
      break;
    case 9:
      f_day = "Fri";
      break;
    case 10:
      f_day = "Sat";
      break;
    
    default:
      f_day = "";
      Serial.println ("ERRO! No Ctrl Personalizado das Funções!");
      break;
    }

    if (f_day == rtc.getDOWStr(FORMAT_SHORT) && EEPROM.read (i) == 1)
    {
      for (int g = 11; g < 15; g++)
      {
        // Determinar o período
        int EEPROMHours;
        int EEPROMMinutes;
        switch (g)
        {
        case 11:
          EEPROMHours = 15;
          EEPROMMinutes = 16;
          break;
        case 12:
          EEPROMHours = 17;
          EEPROMMinutes = 18;
          break;
        case 13:
          EEPROMHours = 19;
          EEPROMMinutes = 20;
          break;
        case 14:
          EEPROMHours = 21;
          EEPROMMinutes = 22;
          break;
        }

        if (EEPROM.read (g) == 1)
        {
          if (hours == EEPROM.read (EEPROMHours) && minutes == EEPROM.read (EEPROMMinutes))
          {
            switch (ctrlPersonFatores)
            {
              case 0:
                Serial.println ("Controle Personalizado (Fatores): Irrigação");
                // Ligar a bomba até o sensor sinalizar que está ótimo e desligar
                // O sensor de iluminação continuará como automático
                CtrlPersoIrrig ();
                break;
              case 1:
                Serial.println ("Controle Personalizado (Fatores): Luminação");
                // O sensor de umidade do solo, continuará como automático
                // Iluminar até que haja quantidade de iluminação suficiente para substituí-la (Sol) ou
                // por um tempo mínimo de 30 minutos
                CtrlPersoLumin ();
                break;
              case 2:
                Serial.println ("Controle Personalizado (Fatores): Irrigação & Luminação");
                // Combinação dos dois anteriores.
                CtrlPersoLumin ();
                CtrlPersoIrrig ();
                break;
            }
          }
          // Resetar condições
          else if (hours == EEPROM.read (EEPROMHours) && minutes != EEPROM.read (EEPROMMinutes))
          {
            watering = false;
          }
          
        }
      }
    }
  }
}

void UpdateLCDDisplay ()
{
  u8g.firstPage(); 
  do 
  {  
    draw();  
  } 
  while( u8g.nextPage() );
}

void ReadAndUpdateRotaryEncoderPosition ()
{
  encoder.tick();
  re_newPos = encoder.getPosition();
  display = re_newPos;
}

void TickBuzzer ()
{
  analogWrite (buzzer, 3);
  delay (100);
  analogWrite (buzzer, 0);
}

void SetMaxMinMenuEncoderPosition (unsigned int min, unsigned int max)
{
  if (display <= min)
  {
    encoder.setPosition (min);
    display = min;
  }
  else if (display >= max)
  {
    encoder.setPosition (max);
    display = max;
  }
}

void MenuBtnMenuAcessControl (int displayIn, unsigned int displayOut, bool &control)
{
  if (display == displayIn && !control)
  {
    control = true;
    encoder.setPosition (displayOut);
    display = displayOut;

    UpdateLCDDisplay ();
    TickBuzzer ();
  }
  else if (display == displayOut && control)
  {
    control = false;
    encoder.setPosition (displayIn);
    display = displayIn;

    UpdateLCDDisplay ();
    TickBuzzer ();
  }
}

void SelectDoWVerification (unsigned int displayIn, int id)
{
  if (display == displayIn && !selection_DoW [id])
  {
    selection_DoW[id] = true;

    switch (id)
    {
      case 0:
        Dow += " DO";
        break;
      case 1:
        Dow += " SEG";
        break;
      case 2:
        Dow += " TE";
        break;
      case 3:
        Dow += " QUA";
        break;
      case 4:
        Dow += " QUI";
        break;
      case 5:
        Dow += " SEX";
        break;
      case 6:
        Dow += " SAB";
        break;
      case 7:
      {
        selection_DoW [7] = false;

        if (Dow != "")
        {
          for (int i = 0; i < 7; i++)
            Serial.println (selection_DoW[i]);

          Serial.println ("DoW: OK Button Pressed!");
        }
        break;
      }
    }

    UpdateLCDDisplay ();
  }
}

void SelectPeriodDiaVerification (unsigned int displayIn, int id)
{
  if (display == displayIn && !selection_PeriodDia [id])
  {
    selection_PeriodDia[id] = true;

    switch (id)
    {
      case 0:
        PeriodDia += " Manha";
        break;
      case 1:
        PeriodDia += " Tarde";
        break;
      case 2:
        PeriodDia += " Noite";
        break;
      case 3:
        PeriodDia += " Madrug";
        break;
      case 4:
      {
        selection_PeriodDia [4] = false;

        if (PeriodDia != "")
        {
          for (int i = 0; i < 4; i++)
            Serial.println (selection_PeriodDia[i]);
        }
        break;
      }
    }

    UpdateLCDDisplay ();
  }
}

void UpdateHorPerVariables (int &hour, int &min, int hourMin, int hourMax, int pos)
{
  if (re_newPosHorPer > pos)
    min+=1;
  else if (re_newPosHorPer < pos)
    min-=1;

  if (hour > hourMin && min <= 0)
  {
    min = 59;
    hour--;
  }
  else if (min <= 0)
    min = 0;

  if (hour < hourMax && min >= 60)
  {
    min = 0;
    hour++;
  }
  else if (min >= 59)
    min = 59;
}

void ButtonHorPerFunction (int displayAction, int id)
{
  if (display == displayAction && !lockMenuRotaryEncoder)
  {
    lockMenuRotaryEncoder = true;
    HorPerID = id;
    encoder.setPosition (0);

    Serial.println ("* Travar Rotary Encoder");
  }
  else if (display == displayAction && lockMenuRotaryEncoder)
  {
    lockMenuRotaryEncoder = false;
    display = displayAction;
    encoder.setPosition (displayAction);

    Serial.println ("* DEStravar Rotary Encoder");
    Serial.println (HorPerID);

    // Salvar informações na EEPROM
    switch (HorPerID)
    {
      case 0:
        EEPROM.write (15, manha_hour);
        EEPROM.write (16, manha_minute);
        break;
      case 1:
        EEPROM.write (17, tarde_hour);
        EEPROM.write (18, tarde_minute);
        break;
      case 2:
        EEPROM.write (19, noite_hour);
        EEPROM.write (20, noite_minute);
        break;
      case 3:
        EEPROM.write (21, madrug_hour);
        EEPROM.write (22, madrug_minute);
        break;
    }
  }
}

void ConnectWifi_SSIDPasswordStringFunctions (int type, int mode)
{
  switch (type)
  {
    case 0:
    {
      if (mode == 0)
      {
        if (ssid_length < 20)
        {
          ssid += ssid_Char;
          ssid_length++;

          Keyboard_string = ssid;
          UpdateLCDDisplay ();

          Serial.print ("SSID: ");
          Serial.println (ssid);
        }
        else
          Serial.println ("Número de caracteres do SSID atingido!");
      }
      else if (mode == 1)
      {
        ssid_length = ssid.length();

        ssid.remove (ssid_length - 1);
        ssid_length--;

        Keyboard_string = ssid;
        UpdateLCDDisplay ();

        Serial.print ("SSID: ");
        Serial.println (ssid);
      }

      break;
    }

    case 1:
    {
      if (mode == 0)
      {
        if (password_length < 20)
        {
          password += ssid_Char;
          password_length++;

          Keyboard_string = password;
          UpdateLCDDisplay ();

          Serial.print ("password: ");
          Serial.println (password);
        }
        else
          Serial.println ("Número de caracteres da Senha atingido!");
      }
      else if (mode == 1)
      {
        password_length = password.length();

        password.remove (password_length - 1);
        password_length--;

        Keyboard_string = password;
        UpdateLCDDisplay ();

        Serial.print ("password: ");
        Serial.println (password);
      }

      break;
    }
  } 
}

void writeString(char add,String data)
{
  int _size = data.length();
  int i;
  for(i=0;i<_size;i++)
  {
    EEPROM.write(add+i,data[i]);
  }
  EEPROM.write(add+_size,'\0');   //Add termination null character for String Data
  //EEPROM.commit ();
}
 
String read_String(char add)
{
  int i;
  char data[100]; //Max 100 Bytes
  int len=0;
  unsigned char k;
  k=EEPROM.read(add);
  while(k != '\0' && len<500)   //Read until null character
  {    
    k=EEPROM.read(add+len);
    data[len]=k;
    len++;
  }
  data[len]='\0';
  return String(data);
}

/* ESP8266-01 - Functions */
void Reset() 
{
  esp.println("AT+RST");
  delay(1200);
  if(esp.find("OK") ) Serial.println("Module Reset");
  connected = false;
}

void ConnectWifi(int displayOut) 
{
  if (!connected)
  {
    String cmd = "AT+CWJAP=\"" +ssid+"\",\"" + password + "\"";
    esp.println(cmd);
    
    delay(1600);

    if(esp.find("OK")) 
    {
      Serial.println("Connected!");
      connected = true;

      // Voltar a tela inicial
      openAddSSID_name = false;
      openKeyboard_Shift = false;
      display = displayOut;
      UpdateLCDDisplay ();

      // Connect to Wifi Display
      conWifi_string = "Conectado";
      UpdateLCDDisplay ();
      delay (1500);
      conWifi_string = "";
      UpdateLCDDisplay ();
    }
    else 
    {
      Serial.println("Cannot connect to wifi"); 
      ConnectWifi(displayOut);
    }
  }
}

void httppost () 
{
  // Start a TCP connection
  esp.println("AT+CIPSTART=\"TCP\",\"" + server + "\",80");

  delay(100); //500
  
  if( esp.find("OK")) 
  {
    Serial.println("TCP connection ready");
  }
  
  String postRequest =
  "POST " + uri + " HTTP/1.0\r\n" +
  "Host: " + server + "\r\n" +
  "Accept: *" + "/" + "*\r\n" +
  "Content-Length: " + data.length() + "\r\n" +
  "Content-Type: application/x-www-form-urlencoded\r\n" +
  "\r\n" + data;
  
  // Determine the number of caracters to be sent
  String sendCmd = "AT+CIPSEND=";
  esp.print(sendCmd);
  esp.println(postRequest.length());
  
  delay(400); //800

  esp.print(postRequest);

  if(esp.find(">")) 
  {
    Serial.println("Sending..."); 
    //esp_message1 = "";
    UpdateLCDDisplay ();
    //esp.print(postRequest);

    delay (100);
  
    if( esp.find("SEND OK")) 
    { 
      Serial.println("Packet sent");
    
      while (esp.available()) 
      {
        String tmpResp = esp.readString();
        Serial.println(tmpResp);
      }
    
      // Close the connection
      esp.println("AT+CIPCLOSE");
    }
  }
  esp_message1 = "";
  UpdateLCDDisplay ();
}

/* Banco de Dados do Sketch */

// Software Menu Screens
void u8g_SplashScreen ()
{
  u8g.drawRFrame (0,0,128,64,3);
  u8g.drawBitmapP(0, 0, 16, 64, CeresSoftware);
}

// Screen 1 - Information Menu
void u8g_InfoMenu() 
{
  char str[10];
  u8g.drawBitmapP(0, 0, 16, 64, TelaPrincipalFrame);

  // Data
  u8g.drawStr (24, 6, rtc.getDateStr(FORMAT_LONG, FORMAT_LITTLEENDIAN, '/'));
  // Horas
  u8g.drawStr (86, 6, rtc.getTimeStr());

  //u8g.drawStr (3, 15, "Temp: "); 
  // Temperatura
  u8g.drawStr (24, 21, dtostrf(t, 5, 2, str));
  u8g.drawBitmapP(47, 21, 1, 8, graus_bitmap);
  u8g.drawStr (54, 21, "C");

  //u8g.drawStr (68, 15, "Humi: ");
  // Umidade do Ar 
  u8g.drawStr (87, 21, dtostrf(h, 5, 2, str));
  u8g.drawStr (112, 22, "%"); 

  //u8g.drawStr (3, 25, "LDR: ");
  // LDR
  u8g.setPrintPos (26, 38);
  u8g.print (perLDR);
  u8g.drawStr (36, 39, "%");

  // Umidade do Solo
  u8g.setPrintPos (87, 38);
  u8g.print (perSM);
  u8g.drawStr (97, 39, "%");

  /*u8g.drawStr (3, 35, "SN1: ");
  u8g.setPrintPos (30, 35);
  u8g.print (snStates [0]);

  u8g.drawStr (50, 35, "SN2: ");
  u8g.setPrintPos (77, 35);
  u8g.print (snStates [1]);*/
  
  u8g.setPrintPos (5, 54);
  u8g.print (esp_message1);
}
   
// Screen 2 - Options Menu/Return to Info Menu
void u8g_OpMenu1()
{
  goBackToInfoScreen = true;

  u8g.drawRFrame(0,0,128,64,3);
  u8g.setColorIndex(1);
  u8g.drawBox(0, 4, 128, 10); 
  u8g.setColorIndex(0);
  u8g.drawStr( 10, 5, "Dados da Estufa");

  u8g.setColorIndex(1);

  u8g.drawStr( 10, 15, "Controles");
  u8g.drawStr( 10, 25, "Configuracoes");
}  

// Screen 3 - Options Menu/Return to Info Menu
void u8g_OpMenu2()  
{
  goBackToInfoScreen = false;

  u8g.drawRFrame(0,0,128,64,3);
  u8g.drawStr( 10, 5, "Dados da Estufa");
  
  u8g.setColorIndex(1);
  u8g.drawBox(0, 14, 128, 10); 
  u8g.setColorIndex(0);
  u8g.drawStr( 10, 15, "Controles");

  u8g.setColorIndex(1);

  u8g.drawStr( 10, 25, "Configuracoes");  
}  
   
// Screen 4 - Options Menu/Return to Info Menu
void u8g_OpMenu3()
{
  u8g.drawRFrame(0,0,128,64,3);
  u8g.drawStr( 10, 5, "Dados da Estufa");
  u8g.drawStr( 10, 15, "Controles");

  u8g.setColorIndex(1);
  u8g.drawBox(0, 24, 128, 10); 
  u8g.setColorIndex(0);
  u8g.drawStr( 10, 25, "Configuracoes");  
}

// Screen 5 - Configurações Menu/
void u8g_ControlMenu_GoBack ()
{ 
  u8g.drawRFrame(0,0,128,64,3);
  u8g.setColorIndex(1);
  u8g.drawBox(0, 4, 128, 10); 
  u8g.setColorIndex(0);
  u8g.drawStr( 10, 5, "Voltar");

  u8g.setColorIndex(1);

  u8g.drawStr( 10, 15, "Controle Automatizado");
  u8g.drawStr( 10, 25, "Controle Personalizado");
}

// Screen 6 - Configurações Menu/
void u8g_ControlMenu_Auto ()
{  
  u8g.drawRFrame(0,0,128,64,3);
  u8g.drawStr( 10, 5, "Voltar");

  u8g.setColorIndex(1);
  u8g.drawBox(0, 14, 128, 10); 
  u8g.setColorIndex(0);
  u8g.drawStr( 10, 15, "Controle Automatizado");

  u8g.setColorIndex(1);

  u8g.drawStr( 10, 25, "Controle Personalizado");
} 

// Screen 7 - Configurações Menu/
void u8g_ControlMenu_Perso ()
{  
  u8g.drawRFrame(0,0,128,64,3);
  u8g.drawStr( 10, 5, "Voltar");
  u8g.drawStr( 10, 15, "Controle Automatizado");
  
  u8g.setColorIndex(1);
  u8g.drawBox(0, 24, 128, 10); 
  u8g.setColorIndex(0);
  u8g.drawStr( 10, 25, "Controle Personalizado");
} 
   
// Screen 8 - Configurações Menu/
void u8g_ConfigMenu_GoBack()
{  
  u8g.drawRFrame (0,0,128,64,3);
  u8g.setColorIndex(1);
  u8g.drawBox(0, 4, 128, 10); 
  u8g.setColorIndex(0);
  u8g.drawStr( 10, 5, "Voltar");

  u8g.setColorIndex(1);

  u8g.drawStr( 10, 15, "Horarios dos Periodos");
  u8g.drawStr( 10, 25, "Internet");
}   
   
// Screen 9 - Configurações Menu/
void u8g_ConfigMenu_HorPer()
{  
  u8g.drawRFrame (0,0,128,64,3);
  u8g.drawStr( 10, 5, "Voltar");
  
  u8g.setColorIndex(1);
  u8g.drawBox(0, 14, 128, 10); 
  u8g.setColorIndex(0);
  u8g.drawStr( 10, 15, "Horarios dos Periodos");

  u8g.setColorIndex(1);

  u8g.drawStr( 10, 25, "Internet");
}

// Screen 10 - Configurações Menu/
void u8g_ConfigMenu_IoT()
{  
  u8g.drawRFrame (0,0,128,64,3);
  u8g.drawStr( 10, 5, "Voltar");
  u8g.drawStr( 10, 15, "Horarios dos Periodos");
  
  u8g.setColorIndex(1);
  u8g.drawBox(0, 24, 128, 10); 
  u8g.setColorIndex(0);
  u8g.drawStr( 10, 25, "Internet");
}    

// Screen 11 - Configurações Menu/
void u8g_AjstSensib_GoBack ()
{  
  u8g.drawRFrame (0,0,128,64,3);
  u8g.setColorIndex(1);
  u8g.drawBox(0, 4, 128, 10); 
  u8g.setColorIndex(0);
  u8g.drawStr( 10, 5, "Voltar");

  u8g.setColorIndex(1);

  u8g.drawStr( 10, 15, "Umidade do Solo");
  u8g.drawStr( 10, 25, "Sensor de Luz");
}

// Screen 12 - Configurações Menu/
void u8g_AjstSensib_SM()
{  
  u8g.drawRFrame (0,0,128,64,3);
  u8g.drawStr( 10, 5, "Voltar");

  u8g.setColorIndex(1);
  u8g.drawBox(0, 14, 128, 10); 
  u8g.setColorIndex(0);
  u8g.drawStr( 10, 15, "Umidade do Solo");

  u8g.setColorIndex(1);

  u8g.drawStr( 10, 25, "Sensor de Luz");
}

// Screen 13 - Configurações Menu/
void u8g_AjstSensib_SL()
{  
  u8g.drawRFrame (0,0,128,64,3);
  u8g.drawStr( 10, 5, "Voltar");
  u8g.drawStr( 10, 15, "Umidade do Solo");

  u8g.setColorIndex(1);
  u8g.drawBox(0, 24, 128, 10); 
  u8g.setColorIndex(0);
  u8g.drawStr( 10, 25, "Sensor de Luz");
}

// Screen 14 - Configurações Menu/
void u8g_ProgFat_GoBack ()
{  
  u8g.drawRFrame (0,0,128,64,3);
  u8g.setColorIndex(1);
  u8g.drawBox(0, 4, 128, 10); 
  u8g.setColorIndex(0);
  u8g.drawStr( 10, 5, "Voltar");

  u8g.setColorIndex(1);

  u8g.drawStr( 10, 15, "Irrigacao");
  u8g.drawStr( 10, 25, "Iluminacao");
  u8g.drawStr( 10, 35, "Irrigacao e Iluminacao");
} 

// Screen 15 - Configurações Menu/
void u8g_ProgFat_Irr ()
{  
  u8g.drawRFrame (0,0,128,64,3);
  u8g.drawStr( 10, 5, "Voltar");
  
  u8g.setColorIndex(1);
  u8g.drawBox(0, 14, 128, 10); 
  u8g.setColorIndex(0);
  u8g.drawStr( 10, 15, "Irrigacao");

  u8g.setColorIndex(1);

  u8g.drawStr( 10, 25, "Iluminacao");
  u8g.drawStr( 10, 35, "Irrigacao e Iluminacao");
}

// Screen 16 - Configurações Menu/
void u8g_ProgFat_Ilu ()
{  
  u8g.drawRFrame (0,0,128,64,3);
  u8g.drawStr( 10, 5, "Voltar");
  u8g.drawStr( 10, 15, "Irrigacao");

  u8g.setColorIndex(1);
  u8g.drawBox(0, 24, 128, 10); 
  u8g.setColorIndex(0);
  u8g.drawStr( 10, 25, "Iluminacao");

  u8g.setColorIndex(1);

  u8g.drawStr( 10, 35, "Irrigacao e Iluminacao");
}

// Screen 17 - Configurações Menu/
void u8g_ProgFat_IrrIlu ()
{  
  u8g.drawRFrame (0,0,128,64,3);
  u8g.drawStr( 10, 5, "Voltar");
  u8g.drawStr( 10, 15, "Irrigacao");
  u8g.drawStr( 10, 25, "Iluminacao");

  u8g.setColorIndex(1);
  u8g.drawBox(0, 34, 128, 10); 
  u8g.setColorIndex(0);
  u8g.drawStr( 10, 35, "Irrigacao e Iluminacao");
}

// Screen 18 - Configurações Menu/
void u8g_HorPer_GoBack ()
{  
  u8g.drawRFrame (0,0,128,64,3);
  u8g.setColorIndex(1);
  u8g.drawBox(0, 4, 128, 10); 
  u8g.setColorIndex(0);
  u8g.drawStr( 10, 5, "Voltar");

  u8g.setColorIndex(1);

  u8g.drawStr( 10, 15, "Manha:");
  u8g.setPrintPos (55, 15);
  u8g.print (manha_hour);
  u8g.drawStr(65, 15, ":");
  u8g.setPrintPos (70, 15);
  u8g.print (manha_minute);

  u8g.drawStr( 10, 25, "Tarde:");
  u8g.setPrintPos (55, 25);
  u8g.print (tarde_hour);
  u8g.drawStr(65, 25, ":");
  u8g.setPrintPos (70, 25);
  u8g.print (tarde_minute);

  u8g.drawStr( 10, 35, "Noite:");
  u8g.setPrintPos (55, 35);
  u8g.print (noite_hour);
  u8g.drawStr(65, 35, ":");
  u8g.setPrintPos (70, 35);
  u8g.print (noite_minute);

  u8g.drawStr( 10, 45, "Madrug:");
  u8g.setPrintPos (60, 45);
  u8g.print (madrug_hour);
  u8g.drawStr(65, 45, ":");
  u8g.setPrintPos (70, 45);
  u8g.print (madrug_minute);
}

// Screen 19 - Configurações Menu/
void u8g_HorPer_Manha ()
{  
  u8g.drawRFrame (0,0,128,64,3);
  u8g.drawStr( 10, 5, "Voltar");
  
  u8g.setColorIndex(1);
  u8g.drawBox(0, 14, 128, 10); 
  u8g.setColorIndex(0);
  u8g.drawStr( 10, 15, "Manha:");
  u8g.setPrintPos (55, 15);
  u8g.print (manha_hour);
  u8g.drawStr(65, 15, ":");
  u8g.setPrintPos (70, 15);
  u8g.print (manha_minute);

  u8g.setColorIndex(1);

  u8g.drawStr( 10, 25, "Tarde:");
  u8g.setPrintPos (55, 25);
  u8g.print (tarde_hour);
  u8g.drawStr(65, 25, ":");
  u8g.setPrintPos (70, 25);
  u8g.print (tarde_minute);

  u8g.drawStr( 10, 35, "Noite:");
  u8g.setPrintPos (55, 35);
  u8g.print (noite_hour);
  u8g.drawStr(65, 35, ":");
  u8g.setPrintPos (70, 35);
  u8g.print (noite_minute);

  u8g.drawStr( 10, 45, "Madrug:");
  u8g.setPrintPos (60, 45);
  u8g.print (madrug_hour);
  u8g.drawStr(65, 45, ":");
  u8g.setPrintPos (70, 45);
  u8g.print (madrug_minute);
}

// Screen 20 - Configurações Menu/
void u8g_HorPer_Tarde ()
{  
  u8g.drawRFrame (0,0,128,64,3);
  u8g.drawStr( 10, 5, "Voltar");
  u8g.drawStr( 10, 15, "Manha:");
  u8g.setPrintPos (55, 15);
  u8g.print (manha_hour);
  u8g.drawStr(65, 15, ":");
  u8g.setPrintPos (70, 15);
  u8g.print (manha_minute);

  u8g.setColorIndex(1);
  u8g.drawBox(0, 24, 128, 10); 
  u8g.setColorIndex(0);
  u8g.drawStr( 10, 25, "Tarde:");
  u8g.setPrintPos (55, 25);
  u8g.print (tarde_hour);
  u8g.drawStr(65, 25, ":");
  u8g.setPrintPos (70, 25);
  u8g.print (tarde_minute);

  u8g.setColorIndex(1);

  u8g.drawStr( 10, 35, "Noite:");
  u8g.setPrintPos (55, 35);
  u8g.print (noite_hour);
  u8g.drawStr(65, 35, ":");
  u8g.setPrintPos (70, 35);
  u8g.print (noite_minute);

  u8g.drawStr( 10, 45, "Madrug:");
  u8g.setPrintPos (60, 45);
  u8g.print (madrug_hour);
  u8g.drawStr(65, 45, ":");
  u8g.setPrintPos (70, 45);
  u8g.print (madrug_minute);
}

// Screen 21 - Configurações Menu/
void u8g_HorPer_Noite ()
{  
  u8g.drawRFrame (0,0,128,64,3);
  u8g.drawStr( 10, 5, "Voltar");
  u8g.drawStr( 10, 15, "Manha:");
  u8g.setPrintPos (55, 15);
  u8g.print (manha_hour);
  u8g.drawStr(65, 15, ":");
  u8g.setPrintPos (70, 15);
  u8g.print (manha_minute);

  u8g.drawStr( 10, 25, "Tarde:");
  u8g.setPrintPos (55, 25);
  u8g.print (tarde_hour);
  u8g.drawStr(65, 25, ":");
  u8g.setPrintPos (70, 25);
  u8g.print (tarde_minute);

  u8g.setColorIndex(1);
  u8g.drawBox(0, 34, 128, 10); 
  u8g.setColorIndex(0);
  u8g.drawStr( 10, 35, "Noite:");
  u8g.setPrintPos (55, 35);
  u8g.print (noite_hour);
  u8g.drawStr(65, 35, ":");
  u8g.setPrintPos (70, 35);
  u8g.print (noite_minute);

  u8g.setColorIndex(1);

  u8g.drawStr( 10, 45, "Madrug:");
  u8g.setPrintPos (60, 45);
  u8g.print (madrug_hour);
  u8g.drawStr(65, 45, ":");
  u8g.setPrintPos (70, 45);
  u8g.print (madrug_minute);
}

// Screen 22 - Configurações Menu/
void u8g_HorPer_Madrug ()
{  
  u8g.drawRFrame (0,0,128,64,3);
  u8g.drawStr( 10, 5, "Voltar");
  u8g.drawStr( 10, 15, "Manha:");
  u8g.setPrintPos (55, 15);
  u8g.print (manha_hour);
  u8g.drawStr(65, 15, ":");
  u8g.setPrintPos (70, 15);
  u8g.print (manha_minute);

  u8g.drawStr( 10, 25, "Tarde:");
  u8g.setPrintPos (55, 25);
  u8g.print (tarde_hour);
  u8g.drawStr(65, 25, ":");
  u8g.setPrintPos (70, 25);
  u8g.print (tarde_minute);

  u8g.drawStr( 10, 35, "Noite:");
  u8g.setPrintPos (55, 35);
  u8g.print (noite_hour);
  u8g.drawStr(65, 35, ":");
  u8g.setPrintPos (70, 35);
  u8g.print (noite_minute);

  u8g.setColorIndex(1);
  u8g.drawBox(0, 44, 128, 10); 
  u8g.setColorIndex(0);
  u8g.drawStr( 10, 45, "Madrug:");
  u8g.setPrintPos (60, 45);
  u8g.print (madrug_hour);
  u8g.drawStr(65, 45, ":");
  u8g.setPrintPos (70, 45);
  u8g.print (madrug_minute);
}

// Screen 23 - Configurações Menu/
void u8g_Iot_GoBack ()
{  
  u8g.drawRFrame (0,0,128,64,3);
  u8g.setColorIndex(1);
  u8g.drawBox(0, 4, 128, 10); 
  u8g.setColorIndex(0);
  u8g.drawStr( 10, 5, "Voltar");

  u8g.setColorIndex(1);

  u8g.drawStr( 10, 15, "Resetar");
  u8g.drawStr( 10, 25, "Conectar a internet");
  u8g.drawStr( 10, 35, "Tempo entre medicoes");
}

// Screen 24 - Configurações Menu/
void u8g_Iot_Reset ()
{  
  u8g.drawRFrame (0,0,128,64,3);
  u8g.drawStr( 10, 5, "Voltar");
  
  u8g.setColorIndex(1);
  u8g.drawBox(0, 14, 128, 10); 
  u8g.setColorIndex(0);
  u8g.drawStr( 10, 15, "Resetar");

  u8g.setColorIndex(1);

  u8g.drawStr( 10, 25, "Conectar a internet");
  u8g.drawStr( 10, 35, "Tempo entre medicoes");
}

// Screen 25 - Configurações Menu/
void u8g_Iot_ConnectWifi ()
{  
  u8g.drawRFrame (0,0,128,64,3);
  u8g.drawStr( 10, 5, "Voltar");
  u8g.drawStr( 10, 15, "Resetar");
  
  u8g.setColorIndex(1);
  u8g.drawBox(0, 24, 128, 10); 
  u8g.setColorIndex(0);
  u8g.drawStr( 10, 25, "Conectar a internet");

  u8g.setColorIndex(1);

  u8g.drawStr( 10, 35, "Tempo entre medicoes");
}

// Screen 26 - Configurações Menu/
void u8g_Iot_TempMedi ()
{  
  u8g.drawRFrame (0,0,128,64,3);
  u8g.drawStr( 10, 5, "Voltar");
  u8g.drawStr( 10, 15, "Resetar");
  u8g.drawStr( 10, 25, "Conectar a internet");

  u8g.setColorIndex(1);
  u8g.drawBox(0, 34, 128, 10); 
  u8g.setColorIndex(0);
  u8g.drawStr( 10, 35, "Tempo entre medicoes");
}

// Screen 27 - Configurações Menu/
void u8g_AjstSensibSM_GoBack ()
{ 
  u8g.drawRFrame (0,0,128,64,3);
  for (int i = 0; i < 3; i++)
  {
    if (text_SMSensibLevel[i] == "*")
      u8g.drawStr( 2, 15 + (i * 10), ">");
  }

  u8g.setColorIndex(1);
  u8g.drawBox(0, 4, 128, 10);
  u8g.setColorIndex(0);
  u8g.drawStr( 10, 5, "Voltar");

  u8g.setColorIndex(1);

  u8g.drawStr( 10, 15, "Sensivel");
  u8g.drawStr( 10, 25, "Moderado");
  u8g.drawStr( 10, 35, "Pouco sensivel");
}

// Screen 28 - Configurações Menu/
void u8g_AjstSensibSM_Lv3 ()
{  
  u8g.drawRFrame (0,0,128,64,3);
  u8g.drawStr( 10, 5, "Voltar");

  u8g.setColorIndex(1);
  u8g.drawBox(0, 14, 128, 10);
  u8g.setColorIndex(0);
  u8g.drawStr( 10, 15, "Sensivel");
  if (text_SMSensibLevel[0] == "*")
      u8g.drawStr( 2, 15, ">");

  u8g.setColorIndex(1);

  u8g.drawStr( 10, 25, "Moderado");
  if (text_SMSensibLevel[1] == "*")
      u8g.drawStr( 2, 25, ">");

  u8g.drawStr( 10, 35, "Pouco sensivel");
  if (text_SMSensibLevel[2] == "*")
      u8g.drawStr( 2, 35, ">");
}

// Screen 29 - Configurações Menu/
void u8g_AjstSensibSM_Lv2 ()
{  
  u8g.drawRFrame (0,0,128,64,3);
  u8g.drawStr( 10, 5, "Voltar");

  u8g.drawStr( 10, 15, "Sensivel");
  if (text_SMSensibLevel[0] == "*")
      u8g.drawStr( 2, 15, ">");

  u8g.setColorIndex(1);
  u8g.drawBox(0, 24, 128, 10); 
  u8g.setColorIndex(0);
  u8g.drawStr( 10, 25, "Moderado");
  if (text_SMSensibLevel[1] == "*")
    u8g.drawStr( 2, 25, ">");

  u8g.setColorIndex(1);

  u8g.drawStr( 10, 35, "Pouco sensivel");
  if (text_SMSensibLevel[2] == "*")
      u8g.drawStr( 2, 35, ">");
}

// Screen 30 - Configurações Menu/
void u8g_AjstSensibSM_Lv1 ()
{  
  u8g.drawRFrame (0,0,128,64,3);
  u8g.drawStr( 10, 5, "Voltar");

  u8g.drawStr( 10, 15, "Sensivel");
  if (text_SMSensibLevel[0] == "*")
      u8g.drawStr( 2, 15, ">");

  u8g.drawStr( 10, 25, "Moderado");
  if (text_SMSensibLevel[1] == "*")
      u8g.drawStr( 2, 25, ">");

  u8g.setColorIndex(1);
  u8g.drawBox(0, 34, 128, 10); 
  u8g.setColorIndex(0);
  u8g.drawStr( 10, 35, "Pouco sensivel");
  if (text_SMSensibLevel[2] == "*")
      u8g.drawStr( 2, 35, ">");
}

// Screen 31 - Configurações Menu/
void u8g_AjstSensibSL_GoBack ()
{
  u8g.drawRFrame (0,0,128,64,3);
  for (int i = 0; i < 3; i++)
  {
    if (text_LDRSensibLevel[i] == "*")
      u8g.drawStr( 2, 15 + (i * 10), ">");
  }

  u8g.setColorIndex(1);
  u8g.drawBox(0, 4, 128, 10); 
  u8g.setColorIndex(0);
  u8g.drawStr( 10, 5, "Voltar");

  u8g.setColorIndex(1);

  u8g.drawStr( 10, 15, "Sensivel");
  u8g.drawStr( 10, 25, "Moderado");
  u8g.drawStr( 10, 35, "Pouco sensivel");
}

// Screen 32 - Configurações Menu/
void u8g_AjstSensibSL_Lv3 ()
{  
  u8g.drawRFrame (0,0,128,64,3);
  u8g.drawStr( 10, 5, "Voltar");

  u8g.setColorIndex(1);
  u8g.drawBox(0, 14, 128, 10); 
  u8g.setColorIndex(0);
  u8g.drawStr( 10, 15, "Sensivel");
  if (text_LDRSensibLevel[0] == "*")
      u8g.drawStr( 2, 15, ">");

  u8g.setColorIndex(1);

  u8g.drawStr( 10, 25, "Moderado");
  if (text_LDRSensibLevel[1] == "*")
      u8g.drawStr( 2, 25, ">");

  u8g.drawStr( 10, 35, "Pouco sensivel");
  if (text_LDRSensibLevel[2] == "*")
      u8g.drawStr( 2, 35, ">");
}

// Screen 33 - Configurações Menu/
void u8g_AjstSensibSL_Lv2 ()
{  
  u8g.drawRFrame (0,0,128,64,3);
  u8g.drawStr( 10, 5, "Voltar");

  u8g.drawStr( 10, 15, "Sensivel");
  if (text_LDRSensibLevel[0] == "*")
      u8g.drawStr( 2, 15, ">");

  u8g.setColorIndex(1);
  u8g.drawBox(0, 24, 128, 10); 
  u8g.setColorIndex(0);
  u8g.drawStr( 10, 25, "Moderado");
  if (text_LDRSensibLevel[1] == "*")
      u8g.drawStr( 2, 25, ">");

  u8g.setColorIndex(1);

  u8g.drawStr( 10, 35, "Pouco sensivel");
  if (text_LDRSensibLevel[2] == "*")
      u8g.drawStr( 2, 35, ">");
}

// Screen 34 - Configurações Menu/
void u8g_AjstSensibSL_Lv1 ()
{  
  u8g.drawRFrame (0,0,128,64,3);
  u8g.drawStr( 10, 5, "Voltar");

  u8g.drawStr( 10, 15, "Sensivel");
  if (text_LDRSensibLevel[0] == "*")
      u8g.drawStr( 2, 15, ">");

  u8g.drawStr( 10, 25, "Moderado");
  if (text_LDRSensibLevel[1] == "*")
      u8g.drawStr( 2, 25, ">");

  u8g.setColorIndex(1);
  u8g.drawBox(0, 34, 128, 10); 
  u8g.setColorIndex(0);
  u8g.drawStr( 10, 35, "Pouco sensivel");
  if (text_LDRSensibLevel[2] == "*")
      u8g.drawStr( 2, 35, ">");
}

// Screen 35 - Configurações Menu/
void u8g_CtrlPersoDoW_GoBack ()
{  
  u8g.drawRFrame (0,0,128,64,3);
  u8g.setColorIndex(1);
  u8g.drawBox(0, 4, 128, 10); 
  u8g.setColorIndex(0);
  u8g.drawStr( 10, 5, "Voltar");

  u8g.setColorIndex(1);

  u8g.drawStr( 10, 15, "Domingo");
  u8g.drawStr( 10, 25, "Segunda");
  u8g.drawStr( 10, 35, "Terca");
  u8g.drawStr( 10, 45, "Quarta");
  u8g.drawStr( 80, 15, "Quinta");
  u8g.drawStr( 80, 25, "Sexta");
  u8g.drawStr( 80, 35, "Sabado");
  u8g.drawStr( 80, 45, "OK");

  u8g.setPrintPos (-3, 55);
  u8g.print (Dow);
}

// Screen 36 - Configurações Menu/
void u8g_CtrlPersoDoW_DOM ()
{  
  u8g.drawRFrame (0,0,128,64,3);
  u8g.drawStr( 10, 5, "Voltar");
  
  u8g.setColorIndex(1);
  u8g.drawBox(0, 14, 64, 10);
  u8g.setColorIndex(0);
  u8g.drawStr( 10, 15, "Domingo");

  u8g.setColorIndex(1);

  u8g.drawStr( 10, 25, "Segunda");
  u8g.drawStr( 10, 35, "Terca");
  u8g.drawStr( 10, 45, "Quarta");
  u8g.drawStr( 80, 15, "Quinta");
  u8g.drawStr( 80, 25, "Sexta");
  u8g.drawStr( 80, 35, "Sabado");
  u8g.drawStr( 80, 45, "OK");

  u8g.setPrintPos (-3, 55);
  u8g.print (Dow);
}

// Screen 37 - Configurações Menu/
void u8g_CtrlPersoDoW_SEG ()
{  
  u8g.drawRFrame (0,0,128,64,3);
  u8g.drawStr( 10, 5, "Voltar");
  u8g.drawStr( 10, 15, "Domingo");
  
  u8g.setColorIndex(1);
  u8g.drawBox(0, 24, 64, 10);
  u8g.setColorIndex(0);
  u8g.drawStr( 10, 25, "Segunda");

  u8g.setColorIndex(1);

  u8g.drawStr( 10, 35, "Terca");
  u8g.drawStr( 10, 45, "Quarta");
  u8g.drawStr( 80, 15, "Quinta");
  u8g.drawStr( 80, 25, "Sexta");
  u8g.drawStr( 80, 35, "Sabado");
  u8g.drawStr( 80, 45, "OK");

  u8g.setPrintPos (-3, 55);
  u8g.print (Dow);
}

// Screen 38 - Configurações Menu/
void u8g_CtrlPersoDoW_TER ()
{  
  u8g.drawRFrame (0,0,128,64,3);
  u8g.drawStr( 10, 5, "Voltar");
  u8g.drawStr( 10, 15, "Domingo");
  u8g.drawStr( 10, 25, "Segunda");
 
  u8g.setColorIndex(1);
  u8g.drawBox(0, 34, 64, 10);
  u8g.setColorIndex(0);
  u8g.drawStr( 10, 35, "Terca");

  u8g.setColorIndex(1);

  u8g.drawStr( 10, 45, "Quarta");
  u8g.drawStr( 80, 15, "Quinta");
  u8g.drawStr( 80, 25, "Sexta");
  u8g.drawStr( 80, 35, "Sabado");
  u8g.drawStr( 80, 45, "OK");

  u8g.setPrintPos (-3, 55);
  u8g.print (Dow);
}

// Screen 39 - Configurações Menu/
void u8g_CtrlPersoDoW_QUA ()
{  
  u8g.drawRFrame (0,0,128,64,3);
  u8g.drawStr( 10, 5, "Voltar");
  u8g.drawStr( 10, 15, "Domingo");
  u8g.drawStr( 10, 25, "Segunda");
  u8g.drawStr( 10, 35, "Terca");

  u8g.setColorIndex(1);
  u8g.drawBox(0, 44, 64, 10);
  u8g.setColorIndex(0);
  u8g.drawStr( 10, 45, "Quarta");

  u8g.setColorIndex(1);

  u8g.drawStr( 80, 15, "Quinta");
  u8g.drawStr( 80, 25, "Sexta");
  u8g.drawStr( 80, 35, "Sabado");
  u8g.drawStr( 80, 45, "OK");

  u8g.setPrintPos (-3, 55);
  u8g.print (Dow);
}

// Screen 40 - Configurações Menu/
void u8g_CtrlPersoDoW_QUI ()
{  
  u8g.drawRFrame (0,0,128,64,3);
  u8g.drawStr( 10, 5, "Voltar");
  u8g.drawStr( 10, 15, "Domingo");
  u8g.drawStr( 10, 25, "Segunda");
  u8g.drawStr( 10, 35, "Terca");
  u8g.drawStr( 10, 45, "Quarta");
  
  u8g.setColorIndex(1);
  u8g.drawBox(64, 14, 128, 10);
  u8g.setColorIndex(0);
  u8g.drawStr( 80, 15, "Quinta");

  u8g.setColorIndex(1);

  u8g.drawStr( 80, 25, "Sexta");
  u8g.drawStr( 80, 35, "Sabado");
  u8g.drawStr( 80, 45, "OK");

  u8g.setPrintPos (-3, 55);
  u8g.print (Dow);
}

// Screen 41 - Configurações Menu/
void u8g_CtrlPersoDoW_SEX ()
{  
  u8g.drawRFrame (0,0,128,64,3);
  u8g.drawStr( 10, 5, "Voltar");
  u8g.drawStr( 10, 15, "Domingo");
  u8g.drawStr( 10, 25, "Segunda");
  u8g.drawStr( 10, 35, "Terca");
  u8g.drawStr( 10, 45, "Quarta");
  u8g.drawStr( 80, 15, "Quinta");
  
  u8g.setColorIndex(1);
  u8g.drawBox(64, 24, 128, 10);
  u8g.setColorIndex(0);
  u8g.drawStr( 80, 25, "Sexta");

  u8g.setColorIndex(1);

  u8g.drawStr( 80, 35, "Sabado");
  u8g.drawStr( 80, 45, "OK");

  u8g.setPrintPos (-3, 55);
  u8g.print (Dow);
}

// Screen 42 - Configurações Menu/
void u8g_CtrlPersoDoW_SAB ()
{  
  u8g.drawRFrame (0,0,128,64,3);
  u8g.drawStr( 10, 5, "Voltar");
  u8g.drawStr( 10, 15, "Domingo");
  u8g.drawStr( 10, 25, "Segunda");
  u8g.drawStr( 10, 35, "Terca");
  u8g.drawStr( 10, 45, "Quarta");
  u8g.drawStr( 80, 15, "Quinta");
  u8g.drawStr( 80, 25, "Sexta");
  
  u8g.setColorIndex(1);
  u8g.drawBox(64, 34, 128, 10);
  u8g.setColorIndex(0);
  u8g.drawStr( 80, 35, "Sabado");

  u8g.setColorIndex(1);

  u8g.drawStr( 80, 45, "OK");

  u8g.setPrintPos (-3, 55);
  u8g.print (Dow);
}

// Screen 43 - Configurações Menu/
void u8g_CtrlPersoDoW_TDS ()
{  
  u8g.drawRFrame (0,0,128,64,3);
  u8g.drawStr( 10, 5, "Voltar");
  u8g.drawStr( 10, 15, "Domingo");
  u8g.drawStr( 10, 25, "Segunda");
  u8g.drawStr( 10, 35, "Terca");
  u8g.drawStr( 10, 45, "Quarta");
  u8g.drawStr( 80, 15, "Quinta");
  u8g.drawStr( 80, 25, "Sexta");
  u8g.drawStr( 80, 35, "Sabado");

  u8g.setColorIndex(1);
  u8g.drawBox(64, 44, 128, 10);
  u8g.setColorIndex(0);
  u8g.drawStr( 80, 45, "OK");

  u8g.setColorIndex(1);

  u8g.setPrintPos (-3, 55);
  u8g.print (Dow);
}

// Screen 44 - Configurações Menu/
void u8g_CtrlPersoPeriodos_GoBack ()
{  
  u8g.drawRFrame (0,0,128,64,3);
  u8g.setColorIndex(1);
  u8g.drawBox(0, 4, 128, 10);
  u8g.setColorIndex(0);
  u8g.drawStr( 10, 5, "Voltar");

  u8g.setColorIndex(1);

  u8g.drawStr( 10, 15, "Manha");
  u8g.drawStr( 10, 25, "Tarde");
  u8g.drawStr( 10, 35, "Noite");
  u8g.drawStr( 10, 45, "Madrugada");
  u8g.drawStr( 80, 45, "OK");

  u8g.setPrintPos (-3, 55);
  u8g.print (PeriodDia);
}

// Screen 45 - Configurações Menu/
void u8g_CtrlPersoPeriodos_Madru ()
{  
  u8g.drawRFrame (0,0,128,64,3);
  u8g.drawStr( 10, 5, "Voltar");
  u8g.drawStr( 10, 15, "Manha");
  u8g.drawStr( 10, 25, "Tarde");
  u8g.drawStr( 10, 35, "Noite");

  u8g.setColorIndex(1);
  u8g.drawBox(0, 44, 64, 10);
  u8g.setColorIndex(0);
  u8g.drawStr( 10, 45, "Madrugada");

  u8g.setColorIndex(1);

  u8g.drawStr( 80, 45, "OK");

  u8g.setPrintPos (-3, 55);
  u8g.print (PeriodDia);
}

// Screen 46 - Configurações Menu/
void u8g_CtrlPersoPeriodos_Manha ()
{  
  u8g.drawRFrame (0,0,128,64,3);
  u8g.drawStr( 10, 5, "Voltar");
  
  u8g.setColorIndex(1);
  u8g.drawBox(0, 14, 128, 10);
  u8g.setColorIndex(0);
  u8g.drawStr( 10, 15, "Manha");

  u8g.setColorIndex(1);

  u8g.drawStr( 10, 25, "Tarde");
  u8g.drawStr( 10, 35, "Noite");
  u8g.drawStr( 10, 45, "Madrugada");
  u8g.drawStr( 80, 45, "OK");

  u8g.setPrintPos (-3, 55);
  u8g.print (PeriodDia);
}

// Screen 47 - Configurações Menu/
void u8g_CtrlPersoPeriodos_Tarde ()
{  
  u8g.drawRFrame (0,0,128,64,3);
  u8g.drawStr( 10, 5, "Voltar");
  u8g.drawStr( 10, 15, "Manha");
  
  u8g.setColorIndex(1);
  u8g.drawBox(0, 24, 128, 10);
  u8g.setColorIndex(0);
  u8g.drawStr( 10, 25, "Tarde");

  u8g.setColorIndex(1);

  u8g.drawStr( 10, 35, "Noite");
  u8g.drawStr( 10, 45, "Madrugada");
  u8g.drawStr( 80, 45, "OK");

  u8g.setPrintPos (-3, 55);
  u8g.print (PeriodDia);
}

// Screen 48 - Configurações Menu/
void u8g_CtrlPersoPeriodos_Noite ()
{  
  u8g.drawRFrame (0,0,128,64,3);
  u8g.drawStr( 10, 5, "Voltar");
  u8g.drawStr( 10, 15, "Manha");
  u8g.drawStr( 10, 25, "Tarde");
  
  u8g.setColorIndex(1);
  u8g.drawBox(0, 34, 128, 10);
  u8g.setColorIndex(0);
  u8g.drawStr( 10, 35, "Noite");

  u8g.setColorIndex(1);

  u8g.drawStr( 10, 45, "Madrugada");
  u8g.drawStr( 80, 45, "OK");

  u8g.setPrintPos (-3, 55);
  u8g.print (PeriodDia);
}

// Screen 49 - Configurações Menu/
void u8g_CtrlPersoPeriodos_OK ()
{  
  u8g.drawRFrame (0,0,128,64,3);
  u8g.drawStr( 10, 5, "Voltar");
  u8g.drawStr( 10, 15, "Manha");
  u8g.drawStr( 10, 25, "Tarde");
  u8g.drawStr( 10, 35, "Noite");
  u8g.drawStr( 10, 45, "Madrugada");
  
  u8g.setColorIndex(1);
  u8g.drawBox(64, 44, 128, 10);
  u8g.setColorIndex(0);
  u8g.drawStr( 80, 45, "OK");

  u8g.setColorIndex(1);

  u8g.setPrintPos (-3, 55);
  u8g.print (PeriodDia);
}

// Screen 50 - Configurações Menu/
void u8g_IoTConWifi_GoBack ()
{  
  u8g.drawRFrame (0,0,128,64,3);
  u8g.setColorIndex(1);
  u8g.drawBox(0, 4, 128, 10);
  u8g.setColorIndex(0);
  u8g.drawStr( 10, 5, "Voltar");

  u8g.setColorIndex(1);

  u8g.drawStr( 10, 15, "Adicionar rede");
  u8g.drawStr( 10, 25, "Conectar rede");

  u8g.setPrintPos (10, 50); u8g.print (conWifi_string);
}

// Screen 51 - Configurações Menu/
void u8g_IoTConWifi_AddSSID ()
{  
  u8g.drawRFrame (0,0,128,64,3);
  u8g.drawStr( 10, 5, "Voltar");
  
  u8g.setColorIndex(1);
  u8g.drawBox(0, 14, 128, 10);
  u8g.setColorIndex(0);
  u8g.drawStr( 10, 15, "Adicionar rede");

  u8g.setColorIndex(1);

  u8g.drawStr( 10, 25, "Conectar rede");

  u8g.setPrintPos (10, 50); u8g.print (conWifi_string);
}

// Screen 52 - Configurações Menu/
void u8g_IoTConWifi_ConSSID ()
{  
  u8g.drawRFrame (0,0,128,64,3);
  u8g.drawStr( 10, 5, "Voltar");
  u8g.drawStr( 10, 15, "Adicionar rede");

  u8g.setColorIndex(1);
  u8g.drawBox(0, 24, 128, 10);
  u8g.setColorIndex(0);
  u8g.drawStr( 10, 25, "Conectar rede");

  u8g.setColorIndex(1);

  u8g.setPrintPos (10, 50); u8g.print (conWifi_string);
}

// Screen ?? - Configurações Menu/
void u8g_IoTConWifi_MedTemp_GoBack ()
{ 
  u8g.drawRFrame (0,0,128,64,3);
  if (interval/60000 == 2)
    u8g.drawStr(2, 15, ">");
  if (interval/60000 == 5)
    u8g.drawStr(2, 25, ">");
  if (interval/60000 == 10)
    u8g.drawStr(2, 35, ">");
  if (interval/60000 == 30)
    u8g.drawStr(72, 15, ">");
  if (interval/60000 == 60)
    u8g.drawStr(72, 25, ">");

  u8g.setColorIndex(1);
  u8g.drawBox(0, 4, 128, 10);
  u8g.setColorIndex(0);
  u8g.drawStr( 10, 5, "Voltar");

  u8g.setColorIndex(1);

  u8g.drawStr( 10, 15, "2min");
  u8g.drawStr( 10, 25, "5min");
  u8g.drawStr( 10, 35, "10min");
  u8g.drawStr( 80, 15, "30min");
  u8g.drawStr( 80, 25, "01hour");
  u8g.drawStr( 80, 35, "Salvar");
}

// Screen ?? - Configurações Menu/
void u8g_IoTConWifi_MedTemp_2Min ()
{  
  u8g.drawRFrame (0,0,128,64,3);
  u8g.drawStr( 10, 5, "Voltar");
  
  u8g.setColorIndex(1);
  u8g.drawBox(0, 14, 64, 10);
  u8g.setColorIndex(0);
  u8g.drawStr( 10, 15, "2min");
  if (interval/60000 == 2)
    u8g.drawStr(2, 15, ">");

  u8g.setColorIndex(1);

  u8g.drawStr( 10, 25, "5min");
  if (interval/60000 == 5)
    u8g.drawStr(2, 25, ">");

  u8g.drawStr( 10, 35, "10min");
  if (interval/60000 == 10)
    u8g.drawStr(2, 35, ">");

  u8g.drawStr( 80, 15, "30min");
  if (interval/60000 == 30)
    u8g.drawStr(72, 15, ">");

  u8g.drawStr( 80, 25, "01hour");
  if (interval/60000 == 60)
    u8g.drawStr(72, 25, ">");

  u8g.drawStr( 80, 35, "Salvar");
}

// Screen ?? - Configurações Menu/
void u8g_IoTConWifi_MedTemp_5Min ()
{  
  u8g.drawRFrame (0,0,128,64,3);
  u8g.drawStr( 10, 5, "Voltar");
  u8g.drawStr( 10, 15, "2min");
  if (interval/60000 == 2)
    u8g.drawStr(2, 15, ">");

  u8g.setColorIndex(1);
  u8g.drawBox(0, 24, 64, 10);
  u8g.setColorIndex(0);
  u8g.drawStr( 10, 25, "5min");
  if (interval/60000 == 5)
    u8g.drawStr(2, 25, ">");

  u8g.setColorIndex(1);

  u8g.drawStr( 10, 35, "10min");
  if (interval/60000 == 10)
    u8g.drawStr(2, 35, ">");

  u8g.drawStr( 80, 15, "30min");
  if (interval/60000 == 30)
    u8g.drawStr(72, 15, ">");

  u8g.drawStr( 80, 25, "01hour");
  if (interval/60000 == 60)
    u8g.drawStr(72, 25, ">");

  u8g.drawStr( 80, 35, "Salvar");
}

// Screen ?? - Configurações Menu/
void u8g_IoTConWifi_MedTemp_10Min ()
{  
  u8g.drawRFrame (0,0,128,64,3);
  u8g.drawStr( 10, 5, "Voltar");
  u8g.drawStr( 10, 15, "2min");
  if (interval/60000 == 2)
    u8g.drawStr(2, 15, ">");

  u8g.drawStr( 10, 25, "5min");
  if (interval/60000 == 5)
    u8g.drawStr(2, 25, ">");
  
  u8g.setColorIndex(1);
  u8g.drawBox(0, 34, 64, 10);
  u8g.setColorIndex(0);
  u8g.drawStr( 10, 35, "10min");
  if (interval/60000 == 10)
    u8g.drawStr(2, 35, ">");

  u8g.setColorIndex(1);

  u8g.drawStr( 80, 15, "30min");
  if (interval/60000 == 30)
    u8g.drawStr(72, 15, ">");

  u8g.drawStr( 80, 25, "01hour");
  if (interval/60000 == 60)
    u8g.drawStr(72, 25, ">");

  u8g.drawStr( 80, 35, "Salvar");
}

// Screen ?? - Configurações Menu/
void u8g_IoTConWifi_MedTemp_30Min ()
{  
  u8g.drawRFrame (0,0,128,64,3);
  u8g.drawStr( 10, 5, "Voltar");
  u8g.drawStr( 10, 15, "2min");
  if (interval/60000 == 2)
    u8g.drawStr(2, 15, ">");

  u8g.drawStr( 10, 25, "5min");
  if (interval/60000 == 5)
    u8g.drawStr(2, 25, ">");

  u8g.drawStr( 10, 35, "10min");
  if (interval/60000 == 10)
    u8g.drawStr(2, 35, ">");
  
  u8g.setColorIndex(1);
  u8g.drawBox(64, 14, 128, 10);
  u8g.setColorIndex(0);
  u8g.drawStr( 80, 15, "30min");
  if (interval/60000 == 30)
    u8g.drawStr(72, 15, ">");

  u8g.setColorIndex(1);

  u8g.drawStr( 80, 25, "01hour");
  if (interval/60000 == 60)
    u8g.drawStr(72, 25, ">");

  u8g.drawStr( 80, 35, "Salvar");
}

// Screen ?? - Configurações Menu/
void u8g_IoTConWifi_MedTemp_01Hour ()
{  
  u8g.drawRFrame (0,0,128,64,3);
  u8g.drawStr( 10, 5, "Voltar");
  u8g.drawStr( 10, 15, "2min");
  if (interval/60000 == 2)
    u8g.drawStr(2, 15, ">");

  u8g.drawStr( 10, 25, "5min");
  if (interval/60000 == 5)
    u8g.drawStr(2, 25, ">");

  u8g.drawStr( 10, 35, "10min");
  if (interval/60000 == 10)
    u8g.drawStr(2, 35, ">");

  u8g.drawStr( 80, 15, "30min");
  if (interval/60000 == 30)
    u8g.drawStr(72, 15, ">");
  
  u8g.setColorIndex(1);
  u8g.drawBox(64, 24, 128, 10);
  u8g.setColorIndex(0);
  u8g.drawStr( 80, 25, "01hour");
  if (interval/60000 == 60)
    u8g.drawStr(72, 25, ">");

  u8g.setColorIndex(1);

  u8g.drawStr( 80, 35, "Salvar");
}

// Screen ?? - Configurações Menu/
void u8g_IoTConWifi_MedTemp_Salvar ()
{  
  u8g.drawRFrame (0,0,128,64,3);
  u8g.drawStr( 10, 5, "Voltar");
  u8g.drawStr( 10, 15, "2min");
  if (interval/60000 == 2)
    u8g.drawStr(2, 15, ">");

  u8g.drawStr( 10, 25, "5min");
  if (interval/60000 == 5)
    u8g.drawStr(2, 25, ">");

  u8g.drawStr( 10, 35, "10min");
  if (interval/60000 == 10)
    u8g.drawStr(2, 35, ">");

  u8g.drawStr( 80, 15, "30min");
  if (interval/60000 == 30)
    u8g.drawStr(72, 15, ">");

  u8g.drawStr( 80, 25, "01hour");
  if (interval/60000 == 60)
    u8g.drawStr(72, 25, ">");
  
  u8g.setColorIndex(1);
  u8g.drawBox(64, 34, 128, 10);
  u8g.setColorIndex(0);
  u8g.drawStr( 80, 35, "Salvar");
}

void draw() //Rotina Desenho  
{
  u8g_prepare();  
  switch(display) //Carrega a tela correspondente  
  {
    default:
      u8g_SplashScreen ();
      break;
    case 0:  
      u8g_InfoMenu(); 
      break;  
    case 1:  
      u8g_OpMenu1();  
      break;  
    case 2:  
      u8g_OpMenu2();  
      break;  
    case 3:  
      u8g_OpMenu3();  
      break;  
    case 4:  
      u8g_ControlMenu_GoBack();  
      break;  
    case 5:  
      u8g_ControlMenu_Auto();  
      break;  
    case 6:  
      u8g_ControlMenu_Perso();  
      break;
    case 7:  
      u8g_ConfigMenu_GoBack();  
      break;
    case 8:  
      u8g_ConfigMenu_HorPer();  
      break; 
    case 9:  
      u8g_ConfigMenu_IoT();  
      break;
    case 10:
      u8g_AjstSensib_GoBack ();
      break;
    case 11:
      u8g_AjstSensib_SM ();
      break;
    case 12:
      u8g_AjstSensib_SL ();
      break;
    case 13:
      u8g_ProgFat_GoBack ();
      break;
    case 14:
      u8g_ProgFat_Irr ();
      break;
    case 15:
      u8g_ProgFat_Ilu ();
      break;
    case 16:
      u8g_ProgFat_IrrIlu ();
      break;
    case 17:
      u8g_HorPer_GoBack ();
      break;
    case 18:
      u8g_HorPer_Manha ();
      break;
    case 19:
      u8g_HorPer_Tarde ();
      break;
    case 20:
      u8g_HorPer_Noite ();
      break;
    case 21:
      u8g_HorPer_Madrug ();
      break;
    case 22:
      u8g_Iot_GoBack ();
      break;
    case 23:
      u8g_Iot_Reset ();
      break;
    case 24:
      u8g_Iot_ConnectWifi ();
      break;
    case 25:
      u8g_Iot_TempMedi ();
      break;
    case 26:
      u8g_AjstSensibSM_GoBack ();
      break;
    case 27:
      u8g_AjstSensibSM_Lv3 ();
      break;
    case 28:
      u8g_AjstSensibSM_Lv2 ();
      break;
    case 29:
      u8g_AjstSensibSM_Lv1 ();
      break;
    case 30:
      u8g_AjstSensibSL_GoBack ();
      break;
    case 31:
      u8g_AjstSensibSL_Lv3 ();
      break;
    case 32:
      u8g_AjstSensibSL_Lv2 ();
      break;
    case 33:
      u8g_AjstSensibSL_Lv1 ();
      break;

    case 34:
      u8g_CtrlPersoDoW_GoBack ();
      break;
    case 35:
      u8g_CtrlPersoDoW_DOM ();
      break;
    case 36:
      u8g_CtrlPersoDoW_SEG ();
      break;
    case 37:
      u8g_CtrlPersoDoW_TER ();
      break;
    case 38:
      u8g_CtrlPersoDoW_QUA ();
      break;
    case 39:
      u8g_CtrlPersoDoW_QUI ();
      break;
    case 40:
      u8g_CtrlPersoDoW_SEX ();
      break;
    case 41:
      u8g_CtrlPersoDoW_SAB ();
      break;
    case 42:
      u8g_CtrlPersoDoW_TDS ();
      break;

    case 43:
      u8g_CtrlPersoPeriodos_GoBack ();
      break;
    case 44:
      u8g_CtrlPersoPeriodos_Manha ();
      break;
    case 45:
      u8g_CtrlPersoPeriodos_Tarde ();
      break;
    case 46:
      u8g_CtrlPersoPeriodos_Noite ();
      break;
    case 47:
      u8g_CtrlPersoPeriodos_Madru ();
      break;
    case 48:
      u8g_CtrlPersoPeriodos_OK ();
      break;

    case 49:
      u8g_IoTConWifi_GoBack ();
      break;
    case 50:
      u8g_IoTConWifi_AddSSID ();
      break;
    case 51:
      u8g_IoTConWifi_ConSSID ();
      break;

    case 52:
      u8g_ConfigWifiSSIDKeyboard_GoBack ();
      break;
    case 53:
      u8g_ConfigWifiSSIDKeyboard_1 ();
      ssid_Char = ssidDB_normal [0];
      break;
    case 54:
      u8g_ConfigWifiSSIDKeyboard_2 ();
      ssid_Char = ssidDB_normal [1];
      break;
    case 55:
      u8g_ConfigWifiSSIDKeyboard_3 ();
      ssid_Char = ssidDB_normal [2];
      break;
    case 56:
      u8g_ConfigWifiSSIDKeyboard_4 ();
      ssid_Char = ssidDB_normal [3];
      break;
    case 57:
      u8g_ConfigWifiSSIDKeyboard_5 ();
      ssid_Char = ssidDB_normal [4];
      break;
    case 58:
      u8g_ConfigWifiSSIDKeyboard_6 ();
      ssid_Char = ssidDB_normal [5];
      break;
    case 59:
      u8g_ConfigWifiSSIDKeyboard_7 ();
      ssid_Char = ssidDB_normal [6];
      break;
    case 60:
      u8g_ConfigWifiSSIDKeyboard_8 ();
      ssid_Char = ssidDB_normal [7];
      break;
    case 61:
      u8g_ConfigWifiSSIDKeyboard_9 ();
      ssid_Char = ssidDB_normal [8];
      break;
    case 62:
      u8g_ConfigWifiSSIDKeyboard_BackSpace ();
      break;
    case 63:
      u8g_ConfigWifiSSIDKeyboard_q ();
      ssid_Char = ssidDB_normal [9];
      break;
    case 64:
      u8g_ConfigWifiSSIDKeyboard_w ();
      ssid_Char = ssidDB_normal [10];
      break;
    case 65:
      u8g_ConfigWifiSSIDKeyboard_e ();
      ssid_Char = ssidDB_normal [11];
      break;
    case 66:
      u8g_ConfigWifiSSIDKeyboard_r ();
      ssid_Char = ssidDB_normal [12];
      break;
    case 67:
      u8g_ConfigWifiSSIDKeyboard_t ();
      ssid_Char = ssidDB_normal [13];
      break;
    case 68:
      u8g_ConfigWifiSSIDKeyboard_y ();
      ssid_Char = ssidDB_normal [14];
      break;
    case 69:
      u8g_ConfigWifiSSIDKeyboard_u ();
      ssid_Char = ssidDB_normal [15];
      break;
    case 70:
      u8g_ConfigWifiSSIDKeyboard_i ();
      ssid_Char = ssidDB_normal [16];
      break;
    case 71:
      u8g_ConfigWifiSSIDKeyboard_o ();
      ssid_Char = ssidDB_normal [17];
      break;
    case 72:
      u8g_ConfigWifiSSIDKeyboard_p ();
      ssid_Char = ssidDB_normal [18];
      break;
    case 73:
      u8g_ConfigWifiSSIDKeyboard_a ();
      ssid_Char = ssidDB_normal [19];
      break;
    case 74:
      u8g_ConfigWifiSSIDKeyboard_s ();
      ssid_Char = ssidDB_normal [20];
      break;
    case 75:
      u8g_ConfigWifiSSIDKeyboard_d ();
      ssid_Char = ssidDB_normal [21];
      break;
    case 76:
      u8g_ConfigWifiSSIDKeyboard_f ();
      ssid_Char = ssidDB_normal [22];
      break;
    case 77:
      u8g_ConfigWifiSSIDKeyboard_g ();
      ssid_Char = ssidDB_normal [23];
      break;
    case 78:
      u8g_ConfigWifiSSIDKeyboard_h ();
      ssid_Char = ssidDB_normal [24];
      break;
    case 79:
      u8g_ConfigWifiSSIDKeyboard_j ();
      ssid_Char = ssidDB_normal [25];
      break;
    case 80:
      u8g_ConfigWifiSSIDKeyboard_k ();
      ssid_Char = ssidDB_normal [26];
      break;
    case 81:
      u8g_ConfigWifiSSIDKeyboard_l ();
      ssid_Char = ssidDB_normal [27];
      break;
    case 82:
      u8g_ConfigWifiSSIDKeyboard_z ();
      ssid_Char = ssidDB_normal [28];
      break;
    case 83:
      u8g_ConfigWifiSSIDKeyboard_x ();
      ssid_Char = ssidDB_normal [29];
      break;
    case 84:
      u8g_ConfigWifiSSIDKeyboard_c ();
      ssid_Char = ssidDB_normal [30];
      break;
    case 85:
      u8g_ConfigWifiSSIDKeyboard_v ();
      ssid_Char = ssidDB_normal [31];
      break;
    case 86:
      u8g_ConfigWifiSSIDKeyboard_b ();
      ssid_Char = ssidDB_normal [32];
      break;
    case 87:
      u8g_ConfigWifiSSIDKeyboard_n ();
      ssid_Char = ssidDB_normal [33];
      break;
    case 88:
      u8g_ConfigWifiSSIDKeyboard_m ();
      ssid_Char = ssidDB_normal [34];
      break;
    case 89:
      u8g_ConfigWifiSSIDKeyboard_Shift ();
      break;
    case 90:
      u8g_ConfigWifiSSIDKeyboard_Continue ();
      break;
    case 91:
      u8g_ConfigWifiSSIDKeyboardShift_GoBack ();
      break;
    case 92:
      u8g_ConfigWifiSSIDKeyboardShift_1 ();
      ssid_Char = ssidDB_shift [0];
      break;
    case 93:
      u8g_ConfigWifiSSIDKeyboardShift_2 ();
      ssid_Char = ssidDB_shift [1];
      break;
    case 94:
      u8g_ConfigWifiSSIDKeyboardShift_3 ();
      ssid_Char = ssidDB_shift [2];
      break;
    case 95:
      u8g_ConfigWifiSSIDKeyboardShift_4 ();
      ssid_Char = ssidDB_shift [3];
      break;
    case 96:
      u8g_ConfigWifiSSIDKeyboardShift_5 ();
      ssid_Char = ssidDB_shift [4];
      break;
    case 97:
      u8g_ConfigWifiSSIDKeyboardShift_6 ();
      ssid_Char = ssidDB_shift [5];
      break;
    case 98:
      u8g_ConfigWifiSSIDKeyboardShift_7 ();
      ssid_Char = ssidDB_shift [6];
      break;
    case 99:
      u8g_ConfigWifiSSIDKeyboardShift_8 ();
      ssid_Char = ssidDB_shift [7];
      break;
    case 100:
      u8g_ConfigWifiSSIDKeyboardShift_9 ();
      ssid_Char = ssidDB_shift [8];
      break;
    case 101:
      u8g_ConfigWifiSSIDKeyboardShift_BackSpace ();
      break;
    case 102:
      u8g_ConfigWifiSSIDKeyboardShift_q ();
      ssid_Char = ssidDB_shift [9];
      break;
    case 103:
      u8g_ConfigWifiSSIDKeyboardShift_w ();
      ssid_Char = ssidDB_shift [10];
      break;
    case 104:
      u8g_ConfigWifiSSIDKeyboardShift_e ();
      ssid_Char = ssidDB_shift [11];
      break;
    case 105:
      u8g_ConfigWifiSSIDKeyboardShift_r ();
      ssid_Char = ssidDB_shift [12];
      break;
    case 106:
      u8g_ConfigWifiSSIDKeyboardShift_t ();
      ssid_Char = ssidDB_shift [13];
      break;
    case 107:
      u8g_ConfigWifiSSIDKeyboardShift_y ();
      ssid_Char = ssidDB_shift [14];
      break;
    case 108:
      u8g_ConfigWifiSSIDKeyboardShift_u ();
      ssid_Char = ssidDB_shift [15];
      break;
    case 109:
      u8g_ConfigWifiSSIDKeyboardShift_i ();
      ssid_Char = ssidDB_shift [16];
      break;
    case 110:
      u8g_ConfigWifiSSIDKeyboardShift_o ();
      ssid_Char = ssidDB_shift [17];
      break;
    case 111:
      u8g_ConfigWifiSSIDKeyboardShift_p ();
      ssid_Char = ssidDB_shift [18];
      break;
    case 112:
      u8g_ConfigWifiSSIDKeyboardShift_a ();
      ssid_Char = ssidDB_shift [19];
      break;
    case 113:
      u8g_ConfigWifiSSIDKeyboardShift_s ();
      ssid_Char = ssidDB_shift [20];
      break;
    case 114:
      u8g_ConfigWifiSSIDKeyboardShift_d ();
      ssid_Char = ssidDB_shift [21];
      break;
    case 115:
      u8g_ConfigWifiSSIDKeyboardShift_f ();
      ssid_Char = ssidDB_shift [22];
      break;
    case 116:
      u8g_ConfigWifiSSIDKeyboardShift_g ();
      ssid_Char = ssidDB_shift [23];
      break;
    case 117:
      u8g_ConfigWifiSSIDKeyboardShift_h ();
      ssid_Char = ssidDB_shift [24];
      break;
    case 118:
      u8g_ConfigWifiSSIDKeyboardShift_j ();
      ssid_Char = ssidDB_shift [25];
      break;
    case 119:
      u8g_ConfigWifiSSIDKeyboardShift_k ();
      ssid_Char = ssidDB_shift [26];
      break;
    case 120:
      u8g_ConfigWifiSSIDKeyboardShift_l ();
      ssid_Char = ssidDB_shift [27];
      break;
    case 121:
      u8g_ConfigWifiSSIDKeyboardShift_z ();
      ssid_Char = ssidDB_shift [28];
      break;
    case 122:
      u8g_ConfigWifiSSIDKeyboardShift_x ();
      ssid_Char = ssidDB_shift [29];
      break;
    case 123:
      u8g_ConfigWifiSSIDKeyboardShift_c ();
      ssid_Char = ssidDB_shift [30];
      break;
    case 124:
      u8g_ConfigWifiSSIDKeyboardShift_v ();
      ssid_Char = ssidDB_shift [31];
      break;
    case 125:
      u8g_ConfigWifiSSIDKeyboardShift_b ();
      ssid_Char = ssidDB_shift [32];
      break;
    case 126:
      u8g_ConfigWifiSSIDKeyboardShift_n ();
      ssid_Char = ssidDB_shift [33];
      break;
    case 127:
      u8g_ConfigWifiSSIDKeyboardShift_m ();
      ssid_Char = ssidDB_shift [34];
      break;

    case 128:
      u8g_IoTConWifi_MedTemp_GoBack ();
      break;
    case 129:
      u8g_IoTConWifi_MedTemp_2Min ();
      break;
    case 130:
      u8g_IoTConWifi_MedTemp_5Min ();
      break;
    case 131:
      u8g_IoTConWifi_MedTemp_10Min ();
      break;
    case 132:
      u8g_IoTConWifi_MedTemp_30Min ();
      break;
    case 133:
      u8g_IoTConWifi_MedTemp_01Hour ();
      break;
    case 134:
      u8g_IoTConWifi_MedTemp_Salvar ();
      break;
  }  
}

// Keyboard
void u8g_ConfigWifiSSIDKeyboard_GoBack ()
{
  u8g.drawRFrame (0,0,128,64,3);
  u8g.setColorIndex(1);
  u8g.drawBox(0, 4, 128, 10);
  u8g.setColorIndex(0);
  u8g.setPrintPos (0, 5); u8g.print (keyboard_header);

  u8g.setColorIndex(1);

  u8g.drawBitmapP(1, 14, 1, 8, return_bitmap);

  u8g.drawStr( 10, 15, "1 2 3 4 5 6 7 8 9 <");  
  u8g.drawStr( 10, 25, "q w e r t y u i o p");  
  u8g.drawStr( 10, 35, "a s d f g h j k l"); 
  u8g.drawStr( 10, 45, "z x c v b n m ^ >");

  u8g.setPrintPos (2, 55); u8g.print (Keyboard_string);
}

void u8g_ConfigWifiSSIDKeyboard_1 ()
{
  u8g.drawRFrame (0,0,128,64,3);
  u8g.setColorIndex(1);
  u8g.drawBox(0, 4, 128, 10);
  u8g.setColorIndex(0);
  u8g.setPrintPos (0, 5); u8g.print (keyboard_header);

  u8g.setColorIndex(1);
  u8g.drawBox(10, 15, 5, 8);
  u8g.setColorIndex(0);
  u8g.drawStr(10, 15, "1");

  u8g.setColorIndex(1);

  u8g.drawStr( 10, 15, "  2 3 4 5 6 7 8 9 <");  
  u8g.drawStr( 10, 25, "q w e r t y u i o p");  
  u8g.drawStr( 10, 35, "a s d f g h j k l"); 
  u8g.drawStr( 10, 45, "z x c v b n m ^ >");

  u8g.setPrintPos (2, 55); u8g.print (Keyboard_string);
}

void u8g_ConfigWifiSSIDKeyboard_2 ()
{
  u8g.drawRFrame (0,0,128,64,3);
  u8g.setColorIndex(1);
  u8g.drawBox(0, 4, 128, 10);
  u8g.setColorIndex(0);
  u8g.setPrintPos (0, 5); u8g.print (keyboard_header);

  u8g.setColorIndex(1);
  u8g.drawBox(19, 15, 6, 8);
  u8g.setColorIndex(0);
  u8g.drawStr(20, 15, "2");

  u8g.setColorIndex(1);

  u8g.drawStr( 10, 15, "1   3 4 5 6 7 8 9 <");  
  u8g.drawStr( 10, 25, "q w e r t y u i o p");  
  u8g.drawStr( 10, 35, "a s d f g h j k l"); 
  u8g.drawStr( 10, 45, "z x c v b n m ^ >"); 

  u8g.setPrintPos (2, 55); u8g.print (Keyboard_string);
}

void u8g_ConfigWifiSSIDKeyboard_3 ()
{
  u8g.drawRFrame (0,0,128,64,3);
  u8g.setColorIndex(1);
  u8g.drawBox(0, 4, 128, 10);
  u8g.setColorIndex(0);
  u8g.setPrintPos (0, 5); u8g.print (keyboard_header);

  u8g.setColorIndex(1);
  u8g.drawBox(29, 15, 6, 8);
  u8g.setColorIndex(0);
  u8g.drawStr(30, 15, "3");

  u8g.setColorIndex(1);

  u8g.drawStr( 10, 15, "1 2   4 5 6 7 8 9 <");  
  u8g.drawStr( 10, 25, "q w e r t y u i o p");  
  u8g.drawStr( 10, 35, "a s d f g h j k l"); 
  u8g.drawStr( 10, 45, "z x c v b n m ^ >"); 

  u8g.setPrintPos (2, 55); u8g.print (Keyboard_string);
}

void u8g_ConfigWifiSSIDKeyboard_4 ()
{
  u8g.drawRFrame (0,0,128,64,3);
  u8g.setColorIndex(1);
  u8g.drawBox(0, 4, 128, 10);
  u8g.setColorIndex(0);
  u8g.setPrintPos (0, 5); u8g.print (keyboard_header);

  u8g.setColorIndex(1);
  u8g.drawBox(39, 15, 6, 8);
  u8g.setColorIndex(0);
  u8g.drawStr(40, 15, "4");

  u8g.setColorIndex(1);

  u8g.drawStr( 10, 15, "1 2 3   5 6 7 8 9 <");  
  u8g.drawStr( 10, 25, "q w e r t y u i o p");  
  u8g.drawStr( 10, 35, "a s d f g h j k l"); 
  u8g.drawStr( 10, 45, "z x c v b n m ^ >"); 

  u8g.setPrintPos (2, 55); u8g.print (Keyboard_string);
}

void u8g_ConfigWifiSSIDKeyboard_5 ()
{
  u8g.drawRFrame (0,0,128,64,3);
  u8g.setColorIndex(1);
  u8g.drawBox(0, 4, 128, 10);
  u8g.setColorIndex(0);
  u8g.setPrintPos (0, 5); u8g.print (keyboard_header);

  u8g.setColorIndex(1);
  u8g.drawBox(49, 15, 6, 8);
  u8g.setColorIndex(0);
  u8g.drawStr(50, 15, "5");

  u8g.setColorIndex(1);

  u8g.drawStr( 10, 15, "1 2 3 4   6 7 8 9 <");  
  u8g.drawStr( 10, 25, "q w e r t y u i o p");  
  u8g.drawStr( 10, 35, "a s d f g h j k l"); 
  u8g.drawStr( 10, 45, "z x c v b n m ^ >"); 

  u8g.setPrintPos (2, 55); u8g.print (Keyboard_string);
}

void u8g_ConfigWifiSSIDKeyboard_6 ()
{
  u8g.drawRFrame (0,0,128,64,3);
  u8g.setColorIndex(1);
  u8g.drawBox(0, 4, 128, 10);
  u8g.setColorIndex(0);
  u8g.setPrintPos (0, 5); u8g.print (keyboard_header);

  u8g.setColorIndex(1);
  u8g.drawBox(59, 15, 6, 8);
  u8g.setColorIndex(0);
  u8g.drawStr(60, 15, "6");

  u8g.setColorIndex(1);

  u8g.drawStr( 10, 15, "1 2 3 4 5   7 8 9 <");  
  u8g.drawStr( 10, 25, "q w e r t y u i o p");  
  u8g.drawStr( 10, 35, "a s d f g h j k l"); 
  u8g.drawStr( 10, 45, "z x c v b n m ^ >"); 

  u8g.setPrintPos (2, 55); u8g.print (Keyboard_string);
}

void u8g_ConfigWifiSSIDKeyboard_7 ()
{
  u8g.drawRFrame (0,0,128,64,3);
  u8g.setColorIndex(1);
  u8g.drawBox(0, 4, 128, 10);
  u8g.setColorIndex(0);
  u8g.setPrintPos (0, 5); u8g.print (keyboard_header);

  u8g.setColorIndex(1);
  u8g.drawBox(69, 15, 6, 8);
  u8g.setColorIndex(0);
  u8g.drawStr(70, 15, "7");

  u8g.setColorIndex(1);

  u8g.drawStr( 10, 15, "1 2 3 4 5 6   8 9 <");  
  u8g.drawStr( 10, 25, "q w e r t y u i o p");  
  u8g.drawStr( 10, 35, "a s d f g h j k l"); 
  u8g.drawStr( 10, 45, "z x c v b n m ^ >"); 

  u8g.setPrintPos (2, 55); u8g.print (Keyboard_string);
}

void u8g_ConfigWifiSSIDKeyboard_8 ()
{
  u8g.drawRFrame (0,0,128,64,3);
  u8g.setColorIndex(1);
  u8g.drawBox(0, 4, 128, 10);
  u8g.setColorIndex(0);
  u8g.setPrintPos (0, 5); u8g.print (keyboard_header);

  u8g.setColorIndex(1);
  u8g.drawBox(79, 15, 6, 8);
  u8g.setColorIndex(0);
  u8g.drawStr(80, 15, "8");

  u8g.setColorIndex(1);

  u8g.drawStr( 10, 15, "1 2 3 4 5 6 7   9 <");  
  u8g.drawStr( 10, 25, "q w e r t y u i o p");  
  u8g.drawStr( 10, 35, "a s d f g h j k l"); 
  u8g.drawStr( 10, 45, "z x c v b n m ^ >"); 

  u8g.setPrintPos (2, 55); u8g.print (Keyboard_string);
}

void u8g_ConfigWifiSSIDKeyboard_9 ()
{
  u8g.drawRFrame (0,0,128,64,3);
  u8g.setColorIndex(1);
  u8g.drawBox(0, 4, 128, 10);
  u8g.setColorIndex(0);
  u8g.setPrintPos (0, 5); u8g.print (keyboard_header);

  u8g.setColorIndex(1);
  u8g.drawBox(89, 15, 6, 8);
  u8g.setColorIndex(0);
  u8g.drawStr(90, 15, "9");

  u8g.setColorIndex(1);

  u8g.drawStr( 10, 15, "1 2 3 4 5 6 7 8   <");  
  u8g.drawStr( 10, 25, "q w e r t y u i o p");  
  u8g.drawStr( 10, 35, "a s d f g h j k l"); 
  u8g.drawStr( 10, 45, "z x c v b n m ^ >"); 

  u8g.setPrintPos (2, 55); u8g.print (Keyboard_string);
}

void u8g_ConfigWifiSSIDKeyboard_BackSpace ()
{
  u8g.drawRFrame (0,0,128,64,3);
  u8g.setColorIndex(1);
  u8g.drawBox(0, 4, 128, 10);
  u8g.setColorIndex(0);
  u8g.setPrintPos (0, 5); u8g.print (keyboard_header);

  u8g.setColorIndex(1);
  u8g.drawBox(100, 15, 5, 8);
  u8g.setColorIndex(0);
  u8g.drawStr(100, 15, "<");

  u8g.setColorIndex(1);

  u8g.drawStr( 10, 15, "1 2 3 4 5 6 7 8 9  ");  
  u8g.drawStr( 10, 25, "q w e r t y u i o p");  
  u8g.drawStr( 10, 35, "a s d f g h j k l"); 
  u8g.drawStr( 10, 45, "z x c v b n m ^ >"); 

  u8g.setPrintPos (2, 55); u8g.print (Keyboard_string);
}

void u8g_ConfigWifiSSIDKeyboard_q ()
{
  u8g.drawRFrame (0,0,128,64,3);
  u8g.setColorIndex(1);
  u8g.drawBox(0, 4, 128, 10);
  u8g.setColorIndex(0);
  u8g.setPrintPos (0, 5); u8g.print (keyboard_header);

  u8g.setColorIndex(1);
  u8g.drawBox(10, 25, 6, 8);
  u8g.setColorIndex(0);
  u8g.drawStr(10, 25, "q");

  u8g.setColorIndex(1);

  u8g.drawStr( 10, 15, "1 2 3 4 5 6 7 8 9 <");  
  u8g.drawStr( 10, 25, "  w e r t y u i o p");  
  u8g.drawStr( 10, 35, "a s d f g h j k l"); 
  u8g.drawStr( 10, 45, "z x c v b n m ^ >"); 

  u8g.setPrintPos (2, 55); u8g.print (Keyboard_string);
}

void u8g_ConfigWifiSSIDKeyboard_w ()
{
  u8g.drawRFrame (0,0,128,64,3);
  u8g.setColorIndex(1);
  u8g.drawBox(0, 4, 128, 10);
  u8g.setColorIndex(0);
  u8g.setPrintPos (0, 5); u8g.print (keyboard_header);

  u8g.setColorIndex(1);
  u8g.drawBox(19, 25, 6, 8);
  u8g.setColorIndex(0);
  u8g.drawStr(20, 25, "w");

  u8g.setColorIndex(1);

  u8g.drawStr( 10, 15, "1 2 3 4 5 6 7 8 9 <");  
  u8g.drawStr( 10, 25, "q   e r t y u i o p");  
  u8g.drawStr( 10, 35, "a s d f g h j k l"); 
  u8g.drawStr( 10, 45, "z x c v b n m ^ >"); 

  u8g.setPrintPos (2, 55); u8g.print (Keyboard_string);
}

void u8g_ConfigWifiSSIDKeyboard_e ()
{
  u8g.drawRFrame (0,0,128,64,3);
  u8g.setColorIndex(1);
  u8g.drawBox(0, 4, 128, 10);
  u8g.setColorIndex(0);
  u8g.setPrintPos (0, 5); u8g.print (keyboard_header);

  u8g.setColorIndex(1);
  u8g.drawBox(29, 25, 6, 8);
  u8g.setColorIndex(0);
  u8g.drawStr(30, 25, "e");

  u8g.setColorIndex(1);

  u8g.drawStr( 10, 15, "1 2 3 4 5 6 7 8 9 <");  
  u8g.drawStr( 10, 25, "q w   r t y u i o p");  
  u8g.drawStr( 10, 35, "a s d f g h j k l"); 
  u8g.drawStr( 10, 45, "z x c v b n m ^ >"); 

  u8g.setPrintPos (2, 55); u8g.print (Keyboard_string);
}

void u8g_ConfigWifiSSIDKeyboard_r ()
{
  u8g.drawRFrame (0,0,128,64,3);
  u8g.setColorIndex(1);
  u8g.drawBox(0, 4, 128, 10);
  u8g.setColorIndex(0);
  u8g.setPrintPos (0, 5); u8g.print (keyboard_header);

  u8g.setColorIndex(1);
  u8g.drawBox(39, 25, 6, 8);
  u8g.setColorIndex(0);
  u8g.drawStr(40, 25, "r");

  u8g.setColorIndex(1);

  u8g.drawStr( 10, 15, "1 2 3 4 5 6 7 8 9 <");  
  u8g.drawStr( 10, 25, "q w e   t y u i o p");  
  u8g.drawStr( 10, 35, "a s d f g h j k l"); 
  u8g.drawStr( 10, 45, "z x c v b n m ^ >"); 

  u8g.setPrintPos (2, 55); u8g.print (Keyboard_string);
}

void u8g_ConfigWifiSSIDKeyboard_t ()
{
  u8g.drawRFrame (0,0,128,64,3);
  u8g.setColorIndex(1);
  u8g.drawBox(0, 4, 128, 10);
  u8g.setColorIndex(0);
  u8g.setPrintPos (0, 5); u8g.print (keyboard_header);

  u8g.setColorIndex(1);
  u8g.drawBox(49, 25, 6, 8);
  u8g.setColorIndex(0);
  u8g.drawStr(50, 25, "t");

  u8g.setColorIndex(1);

  u8g.drawStr( 10, 15, "1 2 3 4 5 6 7 8 9 <");  
  u8g.drawStr( 10, 25, "q w e r   y u i o p");  
  u8g.drawStr( 10, 35, "a s d f g h j k l"); 
  u8g.drawStr( 10, 45, "z x c v b n m ^ >"); 

  u8g.setPrintPos (2, 55); u8g.print (Keyboard_string);
}

void u8g_ConfigWifiSSIDKeyboard_y ()
{
  u8g.drawRFrame (0,0,128,64,3);
  u8g.setColorIndex(1);
  u8g.drawBox(0, 4, 128, 10);
  u8g.setColorIndex(0);
  u8g.setPrintPos (0, 5); u8g.print (keyboard_header);

  u8g.setColorIndex(1);
  u8g.drawBox(59, 25, 6, 8);
  u8g.setColorIndex(0);
  u8g.drawStr(60, 25, "y");

  u8g.setColorIndex(1);

  u8g.drawStr( 10, 15, "1 2 3 4 5 6 7 8 9 <");  
  u8g.drawStr( 10, 25, "q w e r t   u i o p");  
  u8g.drawStr( 10, 35, "a s d f g h j k l"); 
  u8g.drawStr( 10, 45, "z x c v b n m ^ >"); 

  u8g.setPrintPos (2, 55); u8g.print (Keyboard_string);
}

void u8g_ConfigWifiSSIDKeyboard_u ()
{
  u8g.drawRFrame (0,0,128,64,3);
  u8g.setColorIndex(1);
  u8g.drawBox(0, 4, 128, 10);
  u8g.setColorIndex(0);
  u8g.setPrintPos (0, 5); u8g.print (keyboard_header);

  u8g.setColorIndex(1);
  u8g.drawBox(69, 25, 6, 8);
  u8g.setColorIndex(0);
  u8g.drawStr(70, 25, "u");

  u8g.setColorIndex(1);

  u8g.drawStr( 10, 15, "1 2 3 4 5 6 7 8 9 <");  
  u8g.drawStr( 10, 25, "q w e r t y   i o p");  
  u8g.drawStr( 10, 35, "a s d f g h j k l"); 
  u8g.drawStr( 10, 45, "z x c v b n m ^ >"); 

  u8g.setPrintPos (2, 55); u8g.print (Keyboard_string);
}

void u8g_ConfigWifiSSIDKeyboard_i ()
{
  u8g.drawRFrame (0,0,128,64,3);
  u8g.setColorIndex(1);
  u8g.drawBox(0, 4, 128, 10);
  u8g.setColorIndex(0);
  u8g.setPrintPos (0, 5); u8g.print (keyboard_header);

  u8g.setColorIndex(1);
  u8g.drawBox(79, 25, 6, 8);
  u8g.setColorIndex(0);
  u8g.drawStr(80, 25, "i");

  u8g.setColorIndex(1);

  u8g.drawStr( 10, 15, "1 2 3 4 5 6 7 8 9 <");  
  u8g.drawStr( 10, 25, "q w e r t y u   o p");  
  u8g.drawStr( 10, 35, "a s d f g h j k l"); 
  u8g.drawStr( 10, 45, "z x c v b n m ^ >"); 

  u8g.setPrintPos (2, 55); u8g.print (Keyboard_string);
}

void u8g_ConfigWifiSSIDKeyboard_o ()
{
  u8g.drawRFrame (0,0,128,64,3);
  u8g.setColorIndex(1);
  u8g.drawBox(0, 4, 128, 10);
  u8g.setColorIndex(0);
  u8g.setPrintPos (0, 5); u8g.print (keyboard_header);

  u8g.setColorIndex(1);
  u8g.drawBox(89, 25, 6, 8);
  u8g.setColorIndex(0);
  u8g.drawStr(90, 25, "o");

  u8g.setColorIndex(1);

  u8g.drawStr( 10, 15, "1 2 3 4 5 6 7 8 9 <");  
  u8g.drawStr( 10, 25, "q w e r t y u i   p");  
  u8g.drawStr( 10, 35, "a s d f g h j k l"); 
  u8g.drawStr( 10, 45, "z x c v b n m ^ >"); 

  u8g.setPrintPos (2, 55); u8g.print (Keyboard_string);
}

void u8g_ConfigWifiSSIDKeyboard_p ()
{
  u8g.drawRFrame (0,0,128,64,3);
  u8g.setColorIndex(1);
  u8g.drawBox(0, 4, 128, 10);
  u8g.setColorIndex(0);
  u8g.setPrintPos (0, 5); u8g.print (keyboard_header);

  u8g.setColorIndex(1);
  u8g.drawBox(99, 25, 6, 8);
  u8g.setColorIndex(0);
  u8g.drawStr(100, 25, "p");

  u8g.setColorIndex(1);

  u8g.drawStr( 10, 15, "1 2 3 4 5 6 7 8 9 <");  
  u8g.drawStr( 10, 25, "q w e r t y u i o  ");  
  u8g.drawStr( 10, 35, "a s d f g h j k l"); 
  u8g.drawStr( 10, 45, "z x c v b n m ^ >"); 

  u8g.setPrintPos (2, 55); u8g.print (Keyboard_string);
}

void u8g_ConfigWifiSSIDKeyboard_a ()
{
  u8g.drawRFrame (0,0,128,64,3);
  u8g.setColorIndex(1);
  u8g.drawBox(0, 4, 128, 10);
  u8g.setColorIndex(0);
  u8g.setPrintPos (0, 5); u8g.print (keyboard_header);

  u8g.setColorIndex(1);
  u8g.drawBox(10, 35, 6, 8);
  u8g.setColorIndex(0);
  u8g.drawStr(10, 35, "a");

  u8g.setColorIndex(1);

  u8g.drawStr( 10, 15, "1 2 3 4 5 6 7 8 9 <");  
  u8g.drawStr( 10, 25, "q w e r t y u i o p");  
  u8g.drawStr( 10, 35, "  s d f g h j k l"); 
  u8g.drawStr( 10, 45, "z x c v b n m ^ >"); 

  u8g.setPrintPos (2, 55); u8g.print (Keyboard_string);
}

void u8g_ConfigWifiSSIDKeyboard_s ()
{
  u8g.drawRFrame (0,0,128,64,3);
  u8g.setColorIndex(1);
  u8g.drawBox(0, 4, 128, 10);
  u8g.setColorIndex(0);
  u8g.setPrintPos (0, 5); u8g.print (keyboard_header);

  u8g.setColorIndex(1);
  u8g.drawBox(19, 35, 6, 8);
  u8g.setColorIndex(0);
  u8g.drawStr(20, 35, "s");

  u8g.setColorIndex(1);

  u8g.drawStr( 10, 15, "1 2 3 4 5 6 7 8 9 <");  
  u8g.drawStr( 10, 25, "q w e r t y u i o p");  
  u8g.drawStr( 10, 35, "a   d f g h j k l"); 
  u8g.drawStr( 10, 45, "z x c v b n m ^ >"); 

  u8g.setPrintPos (2, 55); u8g.print (Keyboard_string);
}

void u8g_ConfigWifiSSIDKeyboard_d ()
{
  u8g.drawRFrame (0,0,128,64,3);
  u8g.setColorIndex(1);
  u8g.drawBox(0, 4, 128, 10);
  u8g.setColorIndex(0);
  u8g.setPrintPos (0, 5); u8g.print (keyboard_header);

  u8g.setColorIndex(1);
  u8g.drawBox(29, 35, 6, 8);
  u8g.setColorIndex(0);
  u8g.drawStr(30, 35, "d");

  u8g.setColorIndex(1);

  u8g.drawStr( 10, 15, "1 2 3 4 5 6 7 8 9 <");  
  u8g.drawStr( 10, 25, "q w e r t y u i o p");  
  u8g.drawStr( 10, 35, "a s   f g h j k l"); 
  u8g.drawStr( 10, 45, "z x c v b n m ^ >"); 

  u8g.setPrintPos (2, 55); u8g.print (Keyboard_string);
}

void u8g_ConfigWifiSSIDKeyboard_f ()
{
  u8g.drawRFrame (0,0,128,64,3);
  u8g.setColorIndex(1);
  u8g.drawBox(0, 4, 128, 10);
  u8g.setColorIndex(0);
  u8g.setPrintPos (0, 5); u8g.print (keyboard_header);

  u8g.setColorIndex(1);
  u8g.drawBox(39, 35, 6, 8);
  u8g.setColorIndex(0);
  u8g.drawStr(40, 35, "f");

  u8g.setColorIndex(1);

  u8g.drawStr( 10, 15, "1 2 3 4 5 6 7 8 9 <");  
  u8g.drawStr( 10, 25, "q w e r t y u i o p");  
  u8g.drawStr( 10, 35, "a s d   g h j k l"); 
  u8g.drawStr( 10, 45, "z x c v b n m ^ >"); 

  u8g.setPrintPos (2, 55); u8g.print (Keyboard_string);
}

void u8g_ConfigWifiSSIDKeyboard_g ()
{
  u8g.drawRFrame (0,0,128,64,3);
  u8g.setColorIndex(1);
  u8g.drawBox(0, 4, 128, 10);
  u8g.setColorIndex(0);
  u8g.setPrintPos (0, 5); u8g.print (keyboard_header);

  u8g.setColorIndex(1);
  u8g.drawBox(49, 35, 6, 8);
  u8g.setColorIndex(0);
  u8g.drawStr(50, 35, "g");

  u8g.setColorIndex(1);

  u8g.drawStr( 10, 15, "1 2 3 4 5 6 7 8 9 <");  
  u8g.drawStr( 10, 25, "q w e r t y u i o p");  
  u8g.drawStr( 10, 35, "a s d f   h j k l"); 
  u8g.drawStr( 10, 45, "z x c v b n m ^ >"); 

  u8g.setPrintPos (2, 55); u8g.print (Keyboard_string);
}

void u8g_ConfigWifiSSIDKeyboard_h ()
{
  u8g.drawRFrame (0,0,128,64,3);
  u8g.setColorIndex(1);
  u8g.drawBox(0, 4, 128, 10);
  u8g.setColorIndex(0);
  u8g.setPrintPos (0, 5); u8g.print (keyboard_header);

  u8g.setColorIndex(1);
  u8g.drawBox(59, 35, 6, 8);
  u8g.setColorIndex(0);
  u8g.drawStr(60, 35, "h");

  u8g.setColorIndex(1);

  u8g.drawStr( 10, 15, "1 2 3 4 5 6 7 8 9 <");  
  u8g.drawStr( 10, 25, "q w e r t y u i o p");  
  u8g.drawStr( 10, 35, "a s d f g   j k l"); 
  u8g.drawStr( 10, 45, "z x c v b n m ^ >"); 

  u8g.setPrintPos (2, 55); u8g.print (Keyboard_string);
}

void u8g_ConfigWifiSSIDKeyboard_j ()
{
  u8g.drawRFrame (0,0,128,64,3);
  u8g.setColorIndex(1);
  u8g.drawBox(0, 4, 128, 10);
  u8g.setColorIndex(0);
  u8g.setPrintPos (0, 5); u8g.print (keyboard_header);

  u8g.setColorIndex(1);
  u8g.drawBox(69, 35, 6, 8);
  u8g.setColorIndex(0);
  u8g.drawStr(70, 35, "j");

  u8g.setColorIndex(1);

  u8g.drawStr( 10, 15, "1 2 3 4 5 6 7 8 9 <");  
  u8g.drawStr( 10, 25, "q w e r t y u i o p");  
  u8g.drawStr( 10, 35, "a s d f g h   k l"); 
  u8g.drawStr( 10, 45, "z x c v b n m ^ >"); 

  u8g.setPrintPos (2, 55); u8g.print (Keyboard_string);
}

void u8g_ConfigWifiSSIDKeyboard_k ()
{
  u8g.drawRFrame (0,0,128,64,3);
  u8g.setColorIndex(1);
  u8g.drawBox(0, 4, 128, 10);
  u8g.setColorIndex(0);
  u8g.setPrintPos (0, 5); u8g.print (keyboard_header);

  u8g.setColorIndex(1);
  u8g.drawBox(79, 35, 6, 8);
  u8g.setColorIndex(0);
  u8g.drawStr(80, 35, "k");

  u8g.setColorIndex(1);

  u8g.drawStr( 10, 15, "1 2 3 4 5 6 7 8 9 <");  
  u8g.drawStr( 10, 25, "q w e r t y u i o p");  
  u8g.drawStr( 10, 35, "a s d f g h j   l"); 
  u8g.drawStr( 10, 45, "z x c v b n m ^ >"); 

  u8g.setPrintPos (2, 55); u8g.print (Keyboard_string);
}

void u8g_ConfigWifiSSIDKeyboard_l ()
{
  u8g.drawRFrame (0,0,128,64,3);
  u8g.setColorIndex(1);
  u8g.drawBox(0, 4, 128, 10);
  u8g.setColorIndex(0);
  u8g.setPrintPos (0, 5); u8g.print (keyboard_header);

  u8g.setColorIndex(1);
  u8g.drawBox(89, 35, 6, 8);
  u8g.setColorIndex(0);
  u8g.drawStr(90, 35, "l");

  u8g.setColorIndex(1);

  u8g.drawStr( 10, 15, "1 2 3 4 5 6 7 8 9 <");  
  u8g.drawStr( 10, 25, "q w e r t y u i o p");  
  u8g.drawStr( 10, 35, "a s d f g h j k  "); 
  u8g.drawStr( 10, 45, "z x c v b n m ^ >"); 

  u8g.setPrintPos (2, 55); u8g.print (Keyboard_string);
}

void u8g_ConfigWifiSSIDKeyboard_z ()
{
  u8g.drawRFrame (0,0,128,64,3);
  u8g.setColorIndex(1);
  u8g.drawBox(0, 4, 128, 10);
  u8g.setColorIndex(0);
  u8g.setPrintPos (0, 5); u8g.print (keyboard_header);

  u8g.setColorIndex(1);
  u8g.drawBox(10, 45, 6, 8);
  u8g.setColorIndex(0);
  u8g.drawStr(10, 45, "z");

  u8g.setColorIndex(1);

  u8g.drawStr( 10, 15, "1 2 3 4 5 6 7 8 9 <");  
  u8g.drawStr( 10, 25, "q w e r t y u i o p");  
  u8g.drawStr( 10, 35, "a s d f g h j k l"); 
  u8g.drawStr( 10, 45, "  x c v b n m ^ >"); 

  u8g.setPrintPos (2, 55); u8g.print (Keyboard_string);
}

void u8g_ConfigWifiSSIDKeyboard_x ()
{
  u8g.drawRFrame (0,0,128,64,3);
  u8g.setColorIndex(1);
  u8g.drawBox(0, 4, 128, 10);
  u8g.setColorIndex(0);
  u8g.setPrintPos (0, 5); u8g.print (keyboard_header);

  u8g.setColorIndex(1);
  u8g.drawBox(19, 45, 6, 8);
  u8g.setColorIndex(0);
  u8g.drawStr(20, 45, "x");

  u8g.setColorIndex(1);

  u8g.drawStr( 10, 15, "1 2 3 4 5 6 7 8 9 <");  
  u8g.drawStr( 10, 25, "q w e r t y u i o p");  
  u8g.drawStr( 10, 35, "a s d f g h j k l"); 
  u8g.drawStr( 10, 45, "z   c v b n m ^ >"); 

  u8g.setPrintPos (2, 55); u8g.print (Keyboard_string);
}

void u8g_ConfigWifiSSIDKeyboard_c ()
{
  u8g.drawRFrame (0,0,128,64,3);
  u8g.setColorIndex(1);
  u8g.drawBox(0, 4, 128, 10);
  u8g.setColorIndex(0);
  u8g.setPrintPos (0, 5); u8g.print (keyboard_header);

  u8g.setColorIndex(1);
  u8g.drawBox(29, 45, 6, 8);
  u8g.setColorIndex(0);
  u8g.drawStr(30, 45, "c");

  u8g.setColorIndex(1);

  u8g.drawStr( 10, 15, "1 2 3 4 5 6 7 8 9 <");  
  u8g.drawStr( 10, 25, "q w e r t y u i o p");  
  u8g.drawStr( 10, 35, "a s d f g h j k l"); 
  u8g.drawStr( 10, 45, "z x   v b n m ^ >"); 

  u8g.setPrintPos (2, 55); u8g.print (Keyboard_string);
}

void u8g_ConfigWifiSSIDKeyboard_v ()
{
  u8g.drawRFrame (0,0,128,64,3);
  u8g.setColorIndex(1);
  u8g.drawBox(0, 4, 128, 10);
  u8g.setColorIndex(0);
  u8g.setPrintPos (0, 5); u8g.print (keyboard_header);

  u8g.setColorIndex(1);
  u8g.drawBox(39, 45, 6, 8);
  u8g.setColorIndex(0);
  u8g.drawStr(40, 45, "v");

  u8g.setColorIndex(1);

  u8g.drawStr( 10, 15, "1 2 3 4 5 6 7 8 9 <");  
  u8g.drawStr( 10, 25, "q w e r t y u i o p");  
  u8g.drawStr( 10, 35, "a s d f g h j k l"); 
  u8g.drawStr( 10, 45, "z x c   b n m ^ >"); 

  u8g.setPrintPos (2, 55); u8g.print (Keyboard_string);
}

void u8g_ConfigWifiSSIDKeyboard_b ()
{
  u8g.drawRFrame (0,0,128,64,3);
  u8g.setColorIndex(1);
  u8g.drawBox(0, 4, 128, 10);
  u8g.setColorIndex(0);
  u8g.setPrintPos (0, 5); u8g.print (keyboard_header);

  u8g.setColorIndex(1);
  u8g.drawBox(49, 45, 6, 8);
  u8g.setColorIndex(0);
  u8g.drawStr(50, 45, "b");

  u8g.setColorIndex(1);

  u8g.drawStr( 10, 15, "1 2 3 4 5 6 7 8 9 <");  
  u8g.drawStr( 10, 25, "q w e r t y u i o p");  
  u8g.drawStr( 10, 35, "a s d f g h j k l"); 
  u8g.drawStr( 10, 45, "z x c v   n m ^ >"); 

  u8g.setPrintPos (2, 55); u8g.print (Keyboard_string);
}

void u8g_ConfigWifiSSIDKeyboard_n ()
{
  u8g.drawRFrame (0,0,128,64,3);
  u8g.setColorIndex(1);
  u8g.drawBox(0, 4, 128, 10);
  u8g.setColorIndex(0);
  u8g.setPrintPos (0, 5); u8g.print (keyboard_header);

  u8g.setColorIndex(1);
  u8g.drawBox(59, 45, 6, 8);
  u8g.setColorIndex(0);
  u8g.drawStr(60, 45, "n");

  u8g.setColorIndex(1);

  u8g.drawStr( 10, 15, "1 2 3 4 5 6 7 8 9 <");  
  u8g.drawStr( 10, 25, "q w e r t y u i o p");  
  u8g.drawStr( 10, 35, "a s d f g h j k l"); 
  u8g.drawStr( 10, 45, "z x c v b   m ^ >"); 

  u8g.setPrintPos (2, 55); u8g.print (Keyboard_string);
}

void u8g_ConfigWifiSSIDKeyboard_m ()
{
  u8g.drawRFrame (0,0,128,64,3);
  u8g.setColorIndex(1);
  u8g.drawBox(0, 4, 128, 10);
  u8g.setColorIndex(0);
  u8g.setPrintPos (0, 5); u8g.print (keyboard_header);

  u8g.setColorIndex(1);
  u8g.drawBox(69, 45, 6, 8);
  u8g.setColorIndex(0);
  u8g.drawStr(70, 45, "m");

  u8g.setColorIndex(1);

  u8g.drawStr( 10, 15, "1 2 3 4 5 6 7 8 9 <");  
  u8g.drawStr( 10, 25, "q w e r t y u i o p");  
  u8g.drawStr( 10, 35, "a s d f g h j k l"); 
  u8g.drawStr( 10, 45, "z x c v b n   ^ >"); 

  u8g.setPrintPos (2, 55); u8g.print (Keyboard_string);
}

void u8g_ConfigWifiSSIDKeyboard_Shift ()
{
  u8g.drawRFrame (0,0,128,64,3);
  u8g.setColorIndex(1);
  u8g.drawBox(0, 4, 128, 10);
  u8g.setColorIndex(0);
  u8g.setPrintPos (0, 5); u8g.print (keyboard_header);

  u8g.setColorIndex(1);
  u8g.drawBox(79, 45, 6, 8);
  u8g.setColorIndex(0);
  u8g.drawStr(80, 45, "^");

  u8g.setColorIndex(1);

  u8g.drawStr( 10, 15, "1 2 3 4 5 6 7 8 9 <");  
  u8g.drawStr( 10, 25, "q w e r t y u i o p");  
  u8g.drawStr( 10, 35, "a s d f g h j k l"); 
  u8g.drawStr( 10, 45, "z x c v b n m   >"); 

  u8g.setPrintPos (2, 55); u8g.print (Keyboard_string);
}

void u8g_ConfigWifiSSIDKeyboard_Continue ()
{
  u8g.drawRFrame (0,0,128,64,3);
  u8g.setColorIndex(1);
  u8g.drawBox(0, 4, 128, 10);
  u8g.setColorIndex(0);
  u8g.setPrintPos (0, 5); u8g.print (keyboard_header);

  u8g.setColorIndex(1);
  u8g.drawBox(89, 45, 6, 8);
  u8g.setColorIndex(0);
  u8g.drawStr(90, 45, ">");

  u8g.setColorIndex(1);

  u8g.drawStr( 10, 15, "1 2 3 4 5 6 7 8 9 <");  
  u8g.drawStr( 10, 25, "q w e r t y u i o p");  
  u8g.drawStr( 10, 35, "a s d f g h j k l"); 
  u8g.drawStr( 10, 45, "z x c v b n m ^  "); 

  u8g.setPrintPos (2, 55); u8g.print (Keyboard_string);
}



void u8g_ConfigWifiSSIDKeyboardShift_GoBack ()
{
  u8g.drawRFrame (0,0,128,64,3);
  u8g.setColorIndex(1);
  u8g.drawBox(0, 4, 128, 10);
  u8g.setColorIndex(0);
  u8g.setPrintPos (0, 5); u8g.print (keyboard_header);

  u8g.setColorIndex(1);

  u8g.drawBitmapP(1, 14, 1, 8, return_bitmap);

  u8g.drawStr( 10, 15, "@ # $ _ & - * ! % <");  
  u8g.drawStr( 10, 25, "Q W E R T Y U I O P");  
  u8g.drawStr( 10, 35, "A S D F G H J K L"); 
  u8g.drawStr( 10, 45, "Z X C V B N M");

  u8g.setPrintPos (2, 55); u8g.print (Keyboard_string);
}

void u8g_ConfigWifiSSIDKeyboardShift_1 ()
{
  u8g.drawRFrame (0,0,128,64,3);
  u8g.setColorIndex(1);
  u8g.drawBox(0, 4, 128, 10);
  u8g.setColorIndex(0);
  u8g.setPrintPos (0, 5); u8g.print (keyboard_header);

  u8g.setColorIndex(1);
  u8g.drawBox(9, 14, 7, 10);
  u8g.setColorIndex(0);
  u8g.drawStr(10, 15, "@");

  u8g.setColorIndex(1);

  u8g.drawStr( 10, 15, "  # $ _ & - * ! % <");  
  u8g.drawStr( 10, 25, "Q W E R T Y U I O P");  
  u8g.drawStr( 10, 35, "A S D F G H J K L"); 
  u8g.drawStr( 10, 45, "Z X C V B N M");

  u8g.setPrintPos (2, 55); u8g.print (Keyboard_string);
}

void u8g_ConfigWifiSSIDKeyboardShift_2 ()
{
  u8g.drawRFrame (0,0,128,64,3);
  u8g.setColorIndex(1);
  u8g.drawBox(0, 4, 128, 10);
  u8g.setColorIndex(0);
  u8g.setPrintPos (0, 5); u8g.print (keyboard_header);

  u8g.setColorIndex(1);
  u8g.drawBox(19, 14, 7, 10);
  u8g.setColorIndex(0);
  u8g.drawStr(20, 15, "#");

  u8g.setColorIndex(1);

  u8g.drawStr( 10, 15, "@   $ _ & - * ! % <");  
  u8g.drawStr( 10, 25, "Q W E R T Y U I O P");  
  u8g.drawStr( 10, 35, "A S D F G H J K L"); 
  u8g.drawStr( 10, 45, "Z X C V B N M");

  u8g.setPrintPos (2, 55); u8g.print (Keyboard_string);
}

void u8g_ConfigWifiSSIDKeyboardShift_3 ()
{
  u8g.drawRFrame (0,0,128,64,3);
  u8g.setColorIndex(1);
  u8g.drawBox(0, 4, 128, 10);
  u8g.setColorIndex(0);
  u8g.setPrintPos (0, 5); u8g.print (keyboard_header);

  u8g.setColorIndex(1);
  u8g.drawBox(29, 14, 7, 10);
  u8g.setColorIndex(0);
  u8g.drawStr(30, 15, "$");

  u8g.setColorIndex(1);

  u8g.drawStr( 10, 15, "@ #   _ & - * ! % <");  
  u8g.drawStr( 10, 25, "Q W E R T Y U I O P");  
  u8g.drawStr( 10, 35, "A S D F G H J K L"); 
  u8g.drawStr( 10, 45, "Z X C V B N M");

  u8g.setPrintPos (2, 55); u8g.print (Keyboard_string);
}

void u8g_ConfigWifiSSIDKeyboardShift_4 ()
{
  u8g.drawRFrame (0,0,128,64,3);
  u8g.setColorIndex(1);
  u8g.drawBox(0, 4, 128, 10);
  u8g.setColorIndex(0);
  u8g.setPrintPos (0, 5); u8g.print (keyboard_header);

  u8g.setColorIndex(1);
  u8g.drawBox(39, 14, 7, 10);
  u8g.setColorIndex(0);
  u8g.drawStr(40, 15, "_");

  u8g.setColorIndex(1);

  u8g.drawStr( 10, 15, "@ # $   & - * ! % <");  
  u8g.drawStr( 10, 25, "Q W E R T Y U I O P");  
  u8g.drawStr( 10, 35, "A S D F G H J K L"); 
  u8g.drawStr( 10, 45, "Z X C V B N M");

  u8g.setPrintPos (2, 55); u8g.print (Keyboard_string);
}

void u8g_ConfigWifiSSIDKeyboardShift_5 ()
{
  u8g.drawRFrame (0,0,128,64,3);
  u8g.setColorIndex(1);
  u8g.drawBox(0, 4, 128, 10);
  u8g.setColorIndex(0);
  u8g.setPrintPos (0, 5); u8g.print (keyboard_header);

  u8g.setColorIndex(1);
  u8g.drawBox(49, 14, 7, 10);
  u8g.setColorIndex(0);
  u8g.drawStr(50, 15, "&");

  u8g.setColorIndex(1);

  u8g.drawStr( 10, 15, "@ # $ _   - * ! % <");  
  u8g.drawStr( 10, 25, "Q W E R T Y U I O P");  
  u8g.drawStr( 10, 35, "A S D F G H J K L"); 
  u8g.drawStr( 10, 45, "Z X C V B N M");

  u8g.setPrintPos (2, 55); u8g.print (Keyboard_string);
}

void u8g_ConfigWifiSSIDKeyboardShift_6 ()
{
  u8g.drawRFrame (0,0,128,64,3);
  u8g.setColorIndex(1);
  u8g.drawBox(0, 4, 128, 10);
  u8g.setColorIndex(0);
  u8g.setPrintPos (0, 5); u8g.print (keyboard_header);

  u8g.setColorIndex(1);
  u8g.drawBox(59, 14, 7, 10);
  u8g.setColorIndex(0);
  u8g.drawStr(60, 15, "-");

  u8g.setColorIndex(1);

  u8g.drawStr( 10, 15, "@ # $ _ &   * ! % <");  
  u8g.drawStr( 10, 25, "Q W E R T Y U I O P");  
  u8g.drawStr( 10, 35, "A S D F G H J K L"); 
  u8g.drawStr( 10, 45, "Z X C V B N M");

  u8g.setPrintPos (2, 55); u8g.print (Keyboard_string);
}

void u8g_ConfigWifiSSIDKeyboardShift_7 ()
{
  u8g.drawRFrame (0,0,128,64,3);
  u8g.setColorIndex(1);
  u8g.drawBox(0, 4, 128, 10);
  u8g.setColorIndex(0);
  u8g.setPrintPos (0, 5); u8g.print (keyboard_header);

  u8g.setColorIndex(1);
  u8g.drawBox(69, 14, 7, 10);
  u8g.setColorIndex(0);
  u8g.drawStr(70, 15, "*");

  u8g.setColorIndex(1);

  u8g.drawStr( 10, 15, "@ # $ _ & -   ! % <");  
  u8g.drawStr( 10, 25, "Q W E R T Y U I O P");  
  u8g.drawStr( 10, 35, "A S D F G H J K L"); 
  u8g.drawStr( 10, 45, "Z X C V B N M");

  u8g.setPrintPos (2, 55); u8g.print (Keyboard_string);
}

void u8g_ConfigWifiSSIDKeyboardShift_8 ()
{
  u8g.drawRFrame (0,0,128,64,3);
  u8g.setColorIndex(1);
  u8g.drawBox(0, 4, 128, 10);
  u8g.setColorIndex(0);
  u8g.setPrintPos (0, 5); u8g.print (keyboard_header);

  u8g.setColorIndex(1);
  u8g.drawBox(79, 14, 7, 10);
  u8g.setColorIndex(0);
  u8g.drawStr(80, 15, "!");

  u8g.setColorIndex(1);

  u8g.drawStr( 10, 15, "@ # $ _ & - *   % <");  
  u8g.drawStr( 10, 25, "Q W E R T Y U I O P");  
  u8g.drawStr( 10, 35, "A S D F G H J K L"); 
  u8g.drawStr( 10, 45, "Z X C V B N M");

  u8g.setPrintPos (2, 55); u8g.print (Keyboard_string);
}

void u8g_ConfigWifiSSIDKeyboardShift_9 ()
{
  u8g.drawRFrame (0,0,128,64,3);
  u8g.setColorIndex(1);
  u8g.drawBox(0, 4, 128, 10);
  u8g.setColorIndex(0);
  u8g.setPrintPos (0, 5); u8g.print (keyboard_header);

  u8g.setColorIndex(1);
  u8g.drawBox(89, 14, 7, 10);
  u8g.setColorIndex(0);
  u8g.drawStr(90, 15, "%");

  u8g.setColorIndex(1);

  u8g.drawStr( 10, 15, "@ # $ _ & - * !   <");  
  u8g.drawStr( 10, 25, "Q W E R T Y U I O P");  
  u8g.drawStr( 10, 35, "A S D F G H J K L"); 
  u8g.drawStr( 10, 45, "Z X C V B N M");

  u8g.setPrintPos (2, 55); u8g.print (Keyboard_string);
}

void u8g_ConfigWifiSSIDKeyboardShift_BackSpace ()
{
  u8g.drawRFrame (0,0,128,64,3);
  u8g.setColorIndex(1);
  u8g.drawBox(0, 4, 128, 10);
  u8g.setColorIndex(0);
  u8g.setPrintPos (0, 5); u8g.print (keyboard_header);

  u8g.setColorIndex(1);
  u8g.drawBox(99, 14, 7, 10);
  u8g.setColorIndex(0);
  u8g.drawStr(100, 15, "<");

  u8g.setColorIndex(1);

  u8g.drawStr( 10, 15, "@ # $ _ & - * ! %  ");  
  u8g.drawStr( 10, 25, "Q W E R T Y U I O P");  
  u8g.drawStr( 10, 35, "A S D F G H J K L"); 
  u8g.drawStr( 10, 45, "Z X C V B N M");

  u8g.setPrintPos (2, 55); u8g.print (Keyboard_string);
}

void u8g_ConfigWifiSSIDKeyboardShift_q ()
{
  u8g.drawRFrame (0,0,128,64,3);
  u8g.setColorIndex(1);
  u8g.drawBox(0, 4, 128, 10);
  u8g.setColorIndex(0);
  u8g.setPrintPos (0, 5); u8g.print (keyboard_header);

  u8g.setColorIndex(1);
  u8g.drawBox(9, 24, 7, 10);
  u8g.setColorIndex(0);
  u8g.drawStr(10, 25, "Q");

  u8g.setColorIndex(1);

  u8g.drawStr( 10, 15, "@ # $ _ & - * ! % <");  
  u8g.drawStr( 10, 25, "  W E R T Y U I O P");  
  u8g.drawStr( 10, 35, "A S D F G H J K L"); 
  u8g.drawStr( 10, 45, "Z X C V B N M");

  u8g.setPrintPos (2, 55); u8g.print (Keyboard_string);
}

void u8g_ConfigWifiSSIDKeyboardShift_w ()
{
  u8g.drawRFrame (0,0,128,64,3);
  u8g.setColorIndex(1);
  u8g.drawBox(0, 4, 128, 10);
  u8g.setColorIndex(0);
  u8g.setPrintPos (0, 5); u8g.print (keyboard_header);

  u8g.setColorIndex(1);
  u8g.drawBox(19, 24, 7, 10);
  u8g.setColorIndex(0);
  u8g.drawStr(20, 25, "W");

  u8g.setColorIndex(1);

  u8g.drawStr( 10, 15, "@ # $ _ & - * ! % <");  
  u8g.drawStr( 10, 25, "Q   E R T Y U I O P");  
  u8g.drawStr( 10, 35, "A S D F G H J K L"); 
  u8g.drawStr( 10, 45, "Z X C V B N M");

  u8g.setPrintPos (2, 55); u8g.print (Keyboard_string);
}

void u8g_ConfigWifiSSIDKeyboardShift_e ()
{
  u8g.drawRFrame (0,0,128,64,3);
  u8g.setColorIndex(1);
  u8g.drawBox(0, 4, 128, 10);
  u8g.setColorIndex(0);
  u8g.setPrintPos (0, 5); u8g.print (keyboard_header);

  u8g.setColorIndex(1);
  u8g.drawBox(29, 24, 7, 10);
  u8g.setColorIndex(0);
  u8g.drawStr(30, 25, "E");

  u8g.setColorIndex(1);

  u8g.drawStr( 10, 15, "@ # $ _ & - * ! % <");  
  u8g.drawStr( 10, 25, "Q W   R T Y U I O P");  
  u8g.drawStr( 10, 35, "A S D F G H J K L"); 
  u8g.drawStr( 10, 45, "Z X C V B N M");

  u8g.setPrintPos (2, 55); u8g.print (Keyboard_string);
}

void u8g_ConfigWifiSSIDKeyboardShift_r ()
{
  u8g.drawRFrame (0,0,128,64,3);
  u8g.setColorIndex(1);
  u8g.drawBox(0, 4, 128, 10);
  u8g.setColorIndex(0);
  u8g.setPrintPos (0, 5); u8g.print (keyboard_header);

  u8g.setColorIndex(1);
  u8g.drawBox(39, 24, 7, 10);
  u8g.setColorIndex(0);
  u8g.drawStr(40, 25, "R");

  u8g.setColorIndex(1);

  u8g.drawStr( 10, 15, "@ # $ _ & - * ! % <");  
  u8g.drawStr( 10, 25, "Q W E   T Y U I O P");  
  u8g.drawStr( 10, 35, "A S D F G H J K L"); 
  u8g.drawStr( 10, 45, "Z X C V B N M");

  u8g.setPrintPos (2, 55); u8g.print (Keyboard_string);
}

void u8g_ConfigWifiSSIDKeyboardShift_t ()
{
  u8g.drawRFrame (0,0,128,64,3);
  u8g.setColorIndex(1);
  u8g.drawBox(0, 4, 128, 10);
  u8g.setColorIndex(0);
  u8g.setPrintPos (0, 5); u8g.print (keyboard_header);

  u8g.setColorIndex(1);
  u8g.drawBox(49, 24, 7, 10);
  u8g.setColorIndex(0);
  u8g.drawStr(50, 25, "T");

  u8g.setColorIndex(1);

  u8g.drawStr( 10, 15, "@ # $ _ & - * ! % <");  
  u8g.drawStr( 10, 25, "Q W E R   Y U I O P");  
  u8g.drawStr( 10, 35, "A S D F G H J K L"); 
  u8g.drawStr( 10, 45, "Z X C V B N M");

  u8g.setPrintPos (2, 55); u8g.print (Keyboard_string);
}

void u8g_ConfigWifiSSIDKeyboardShift_y ()
{
  u8g.drawRFrame (0,0,128,64,3);
  u8g.setColorIndex(1);
  u8g.drawBox(0, 4, 128, 10);
  u8g.setColorIndex(0);
  u8g.setPrintPos (0, 5); u8g.print (keyboard_header);

  u8g.setColorIndex(1);
  u8g.drawBox(59, 24, 7, 10);
  u8g.setColorIndex(0);
  u8g.drawStr(60, 25, "Y");

  u8g.setColorIndex(1);

  u8g.drawStr( 10, 15, "@ # $ _ & - * ! % <");  
  u8g.drawStr( 10, 25, "Q W E R T   U I O P");  
  u8g.drawStr( 10, 35, "A S D F G H J K L"); 
  u8g.drawStr( 10, 45, "Z X C V B N M");

  u8g.setPrintPos (2, 55); u8g.print (Keyboard_string);
}

void u8g_ConfigWifiSSIDKeyboardShift_u ()
{
  u8g.drawRFrame (0,0,128,64,3);
  u8g.setColorIndex(1);
  u8g.drawBox(0, 4, 128, 10);
  u8g.setColorIndex(0);
  u8g.setPrintPos (0, 5); u8g.print (keyboard_header);

  u8g.setColorIndex(1);
  u8g.drawBox(69, 24, 7, 10);
  u8g.setColorIndex(0);
  u8g.drawStr(70, 25, "U");

  u8g.setColorIndex(1);

  u8g.drawStr( 10, 15, "@ # $ _ & - * ! % <");  
  u8g.drawStr( 10, 25, "Q W E R T Y   I O P");  
  u8g.drawStr( 10, 35, "A S D F G H J K L"); 
  u8g.drawStr( 10, 45, "Z X C V B N M");

  u8g.setPrintPos (2, 55); u8g.print (Keyboard_string);
}

void u8g_ConfigWifiSSIDKeyboardShift_i ()
{
  u8g.drawRFrame (0,0,128,64,3);
  u8g.setColorIndex(1);
  u8g.drawBox(0, 4, 128, 10);
  u8g.setColorIndex(0);
  u8g.setPrintPos (0, 5); u8g.print (keyboard_header);

  u8g.setColorIndex(1);
  u8g.drawBox(79, 24, 7, 10);
  u8g.setColorIndex(0);
  u8g.drawStr(80, 25, "I");

  u8g.setColorIndex(1);

  u8g.drawStr( 10, 15, "@ # $ _ & - * ! % <");  
  u8g.drawStr( 10, 25, "Q W E R T Y U   O P");  
  u8g.drawStr( 10, 35, "A S D F G H J K L"); 
  u8g.drawStr( 10, 45, "Z X C V B N M");

  u8g.setPrintPos (2, 55); u8g.print (Keyboard_string);
}

void u8g_ConfigWifiSSIDKeyboardShift_o ()
{
  u8g.drawRFrame (0,0,128,64,3);
  u8g.setColorIndex(1);
  u8g.drawBox(0, 4, 128, 10);
  u8g.setColorIndex(0);
  u8g.setPrintPos (0, 5); u8g.print (keyboard_header);

  u8g.setColorIndex(1);
  u8g.drawBox(89, 24, 7, 10);
  u8g.setColorIndex(0);
  u8g.drawStr(90, 25, "O");

  u8g.setColorIndex(1);

  u8g.drawStr( 10, 15, "@ # $ _ & - * ! % <");  
  u8g.drawStr( 10, 25, "Q W E R T Y U I   P");  
  u8g.drawStr( 10, 35, "A S D F G H J K L"); 
  u8g.drawStr( 10, 45, "Z X C V B N M");

  u8g.setPrintPos (2, 55); u8g.print (Keyboard_string);
}

void u8g_ConfigWifiSSIDKeyboardShift_p ()
{
  u8g.drawRFrame (0,0,128,64,3);
  u8g.setColorIndex(1);
  u8g.drawBox(0, 4, 128, 10);
  u8g.setColorIndex(0);
  u8g.setPrintPos (0, 5); u8g.print (keyboard_header);

  u8g.setColorIndex(1);
  u8g.drawBox(99, 24, 7, 10);
  u8g.setColorIndex(0);
  u8g.drawStr(100, 25, "P");

  u8g.setColorIndex(1);

  u8g.drawStr( 10, 15, "@ # $ _ & - * ! % <");  
  u8g.drawStr( 10, 25, "Q W E R T Y U I O  ");  
  u8g.drawStr( 10, 35, "A S D F G H J K L"); 
  u8g.drawStr( 10, 45, "Z X C V B N M");

  u8g.setPrintPos (2, 55); u8g.print (Keyboard_string);
}

void u8g_ConfigWifiSSIDKeyboardShift_a ()
{
  u8g.drawRFrame (0,0,128,64,3);
  u8g.setColorIndex(1);
  u8g.drawBox(0, 4, 128, 10);
  u8g.setColorIndex(0);
  u8g.setPrintPos (0, 5); u8g.print (keyboard_header);

  u8g.setColorIndex(1);
  u8g.drawBox(9, 34, 7, 10);
  u8g.setColorIndex(0);
  u8g.drawStr(10, 35, "A");

  u8g.setColorIndex(1);

  u8g.drawStr( 10, 15, "@ # $ _ & - * ! % <");  
  u8g.drawStr( 10, 25, "Q W E R T Y U I O P");  
  u8g.drawStr( 10, 35, "  S D F G H J K L"); 
  u8g.drawStr( 10, 45, "Z X C V B N M");

  u8g.setPrintPos (2, 55); u8g.print (Keyboard_string);
}

void u8g_ConfigWifiSSIDKeyboardShift_s ()
{
  u8g.drawRFrame (0,0,128,64,3);
  u8g.setColorIndex(1);
  u8g.drawBox(0, 4, 128, 10);
  u8g.setColorIndex(0);
  u8g.setPrintPos (0, 5); u8g.print (keyboard_header);

  u8g.setColorIndex(1);
  u8g.drawBox(19, 34, 7, 10);
  u8g.setColorIndex(0);
  u8g.drawStr(20, 35, "S");

  u8g.setColorIndex(1);

  u8g.drawStr( 10, 15, "@ # $ _ & - * ! % <");  
  u8g.drawStr( 10, 25, "Q W E R T Y U I O P");  
  u8g.drawStr( 10, 35, "A   D F G H J K L"); 
  u8g.drawStr( 10, 45, "Z X C V B N M");

  u8g.setPrintPos (2, 55); u8g.print (Keyboard_string);
}

void u8g_ConfigWifiSSIDKeyboardShift_d ()
{
  u8g.drawRFrame (0,0,128,64,3);
  u8g.setColorIndex(1);
  u8g.drawBox(0, 4, 128, 10);
  u8g.setColorIndex(0);
  u8g.setPrintPos (0, 5); u8g.print (keyboard_header);

  u8g.setColorIndex(1);
  u8g.drawBox(29, 34, 7, 10);
  u8g.setColorIndex(0);
  u8g.drawStr(30, 35, "D");

  u8g.setColorIndex(1);

  u8g.drawStr( 10, 15, "@ # $ _ & - * ! % <");  
  u8g.drawStr( 10, 25, "Q W E R T Y U I O P");  
  u8g.drawStr( 10, 35, "A S   F G H J K L"); 
  u8g.drawStr( 10, 45, "Z X C V B N M");

  u8g.setPrintPos (2, 55); u8g.print (Keyboard_string);
}

void u8g_ConfigWifiSSIDKeyboardShift_f ()
{
  u8g.drawRFrame (0,0,128,64,3);
  u8g.setColorIndex(1);
  u8g.drawBox(0, 4, 128, 10);
  u8g.setColorIndex(0);
  u8g.setPrintPos (0, 5); u8g.print (keyboard_header);

  u8g.setColorIndex(1);
  u8g.drawBox(39, 34, 7, 10);
  u8g.setColorIndex(0);
  u8g.drawStr(40, 35, "F");

  u8g.setColorIndex(1);

  u8g.drawStr( 10, 15, "@ # $ _ & - * ! % <");  
  u8g.drawStr( 10, 25, "Q W E R T Y U I O P");  
  u8g.drawStr( 10, 35, "A S D   G H J K L"); 
  u8g.drawStr( 10, 45, "Z X C V B N M");

  u8g.setPrintPos (2, 55); u8g.print (Keyboard_string);
}

void u8g_ConfigWifiSSIDKeyboardShift_g ()
{
  u8g.drawRFrame (0,0,128,64,3);
  u8g.setColorIndex(1);
  u8g.drawBox(0, 4, 128, 10);
  u8g.setColorIndex(0);
  u8g.setPrintPos (0, 5); u8g.print (keyboard_header);

  u8g.setColorIndex(1);
  u8g.drawBox(49, 34, 7, 10);
  u8g.setColorIndex(0);
  u8g.drawStr(50, 35, "G");

  u8g.setColorIndex(1);

  u8g.drawStr( 10, 15, "@ # $ _ & - * ! % <");  
  u8g.drawStr( 10, 25, "Q W E R T Y U I O P");  
  u8g.drawStr( 10, 35, "A S D F   H J K L"); 
  u8g.drawStr( 10, 45, "Z X C V B N M");

  u8g.setPrintPos (2, 55); u8g.print (Keyboard_string);
}

void u8g_ConfigWifiSSIDKeyboardShift_h ()
{
  u8g.drawRFrame (0,0,128,64,3);
  u8g.setColorIndex(1);
  u8g.drawBox(0, 4, 128, 10);
  u8g.setColorIndex(0);
  u8g.setPrintPos (0, 5); u8g.print (keyboard_header);

  u8g.setColorIndex(1);
  u8g.drawBox(59, 34, 7, 10);
  u8g.setColorIndex(0);
  u8g.drawStr(60, 35, "H");

  u8g.setColorIndex(1);

  u8g.drawStr( 10, 15, "@ # $ _ & - * ! % <");  
  u8g.drawStr( 10, 25, "Q W E R T Y U I O P");  
  u8g.drawStr( 10, 35, "A S D F G   J K L"); 
  u8g.drawStr( 10, 45, "Z X C V B N M");

  u8g.setPrintPos (2, 55); u8g.print (Keyboard_string);
}

void u8g_ConfigWifiSSIDKeyboardShift_j ()
{
  u8g.drawRFrame (0,0,128,64,3);
  u8g.setColorIndex(1);
  u8g.drawBox(0, 4, 128, 10);
  u8g.setColorIndex(0);
  u8g.setPrintPos (0, 5); u8g.print (keyboard_header);

  u8g.setColorIndex(1);
  u8g.drawBox(69, 34, 7, 10);
  u8g.setColorIndex(0);
  u8g.drawStr(70, 35, "J");

  u8g.setColorIndex(1);

  u8g.drawStr( 10, 15, "@ # $ _ & - * ! % <");  
  u8g.drawStr( 10, 25, "Q W E R T Y U I O P");  
  u8g.drawStr( 10, 35, "A S D F G H   K L"); 
  u8g.drawStr( 10, 45, "Z X C V B N M");

  u8g.setPrintPos (2, 55); u8g.print (Keyboard_string);
}

void u8g_ConfigWifiSSIDKeyboardShift_k ()
{
  u8g.drawRFrame (0,0,128,64,3);
  u8g.setColorIndex(1);
  u8g.drawBox(0, 4, 128, 10);
  u8g.setColorIndex(0);
  u8g.setPrintPos (0, 5); u8g.print (keyboard_header);

  u8g.setColorIndex(1);
  u8g.drawBox(79, 34, 7, 10);
  u8g.setColorIndex(0);
  u8g.drawStr(80, 35, "K");

  u8g.setColorIndex(1);

  u8g.drawStr( 10, 15, "@ # $ _ & - * ! % <");  
  u8g.drawStr( 10, 25, "Q W E R T Y U I O P");  
  u8g.drawStr( 10, 35, "A S D F G H J   L"); 
  u8g.drawStr( 10, 45, "Z X C V B N M");

  u8g.setPrintPos (2, 55); u8g.print (Keyboard_string);
}

void u8g_ConfigWifiSSIDKeyboardShift_l ()
{
  u8g.drawRFrame (0,0,128,64,3);
  u8g.setColorIndex(1);
  u8g.drawBox(0, 4, 128, 10);
  u8g.setColorIndex(0);
  u8g.setPrintPos (0, 5); u8g.print (keyboard_header);

  u8g.setColorIndex(1);
  u8g.drawBox(89, 34, 7, 10);
  u8g.setColorIndex(0);
  u8g.drawStr(90, 35, "L");

  u8g.setColorIndex(1);

  u8g.drawStr( 10, 15, "@ # $ _ & - * ! % <");  
  u8g.drawStr( 10, 25, "Q W E R T Y U I O P");  
  u8g.drawStr( 10, 35, "A S D F G H J K  "); 
  u8g.drawStr( 10, 45, "Z X C V B N M");

  u8g.setPrintPos (2, 55); u8g.print (Keyboard_string);
}

void u8g_ConfigWifiSSIDKeyboardShift_z ()
{
  u8g.drawRFrame (0,0,128,64,3);
  u8g.setColorIndex(1);
  u8g.drawBox(0, 4, 128, 10);
  u8g.setColorIndex(0);
  u8g.setPrintPos (0, 5); u8g.print (keyboard_header);

  u8g.setColorIndex(1);
  u8g.drawBox(9, 44, 7, 10);
  u8g.setColorIndex(0);
  u8g.drawStr(10, 45, "Z");

  u8g.setColorIndex(1);

  u8g.drawStr( 10, 15, "@ # $ _ & - * ! % <");  
  u8g.drawStr( 10, 25, "Q W E R T Y U I O P");  
  u8g.drawStr( 10, 35, "A S D F G H J K L"); 
  u8g.drawStr( 10, 45, "  X C V B N M");

  u8g.setPrintPos (2, 55); u8g.print (Keyboard_string);
}

void u8g_ConfigWifiSSIDKeyboardShift_x ()
{
  u8g.drawRFrame (0,0,128,64,3);
  u8g.setColorIndex(1);
  u8g.drawBox(0, 4, 128, 10);
  u8g.setColorIndex(0);
  u8g.setPrintPos (0, 5); u8g.print (keyboard_header);

  u8g.setColorIndex(1);
  u8g.drawBox(19, 44, 7, 10);
  u8g.setColorIndex(0);
  u8g.drawStr(20, 45, "X");

  u8g.setColorIndex(1);

  u8g.drawStr( 10, 15, "@ # $ _ & - * ! % <");  
  u8g.drawStr( 10, 25, "Q W E R T Y U I O P");  
  u8g.drawStr( 10, 35, "A S D F G H J K L"); 
  u8g.drawStr( 10, 45, "Z   C V B N M");

  u8g.setPrintPos (2, 55); u8g.print (Keyboard_string);
}

void u8g_ConfigWifiSSIDKeyboardShift_c ()
{
  u8g.drawRFrame (0,0,128,64,3);
  u8g.setColorIndex(1);
  u8g.drawBox(0, 4, 128, 10);
  u8g.setColorIndex(0);
  u8g.setPrintPos (0, 5); u8g.print (keyboard_header);

  u8g.setColorIndex(1);
  u8g.drawBox(29, 44, 7, 10);
  u8g.setColorIndex(0);
  u8g.drawStr(30, 45, "C");

  u8g.setColorIndex(1);

  u8g.drawStr( 10, 15, "@ # $ _ & - * ! % <");  
  u8g.drawStr( 10, 25, "Q W E R T Y U I O P");  
  u8g.drawStr( 10, 35, "A S D F G H J K L"); 
  u8g.drawStr( 10, 45, "Z X   V B N M");

  u8g.setPrintPos (2, 55); u8g.print (Keyboard_string);
}

void u8g_ConfigWifiSSIDKeyboardShift_v ()
{
  u8g.drawRFrame (0,0,128,64,3);
  u8g.setColorIndex(1);
  u8g.drawBox(0, 4, 128, 10);
  u8g.setColorIndex(0);
  u8g.setPrintPos (0, 5); u8g.print (keyboard_header);

  u8g.setColorIndex(1);
  u8g.drawBox(39, 44, 7, 10);
  u8g.setColorIndex(0);
  u8g.drawStr(40, 45, "V");

  u8g.setColorIndex(1);

  u8g.drawStr( 10, 15, "@ # $ _ & - * ! % <");  
  u8g.drawStr( 10, 25, "Q W E R T Y U I O P");  
  u8g.drawStr( 10, 35, "A S D F G H J K L"); 
  u8g.drawStr( 10, 45, "Z X C   B N M");

  u8g.setPrintPos (2, 55); u8g.print (Keyboard_string);
}

void u8g_ConfigWifiSSIDKeyboardShift_b ()
{
  u8g.drawRFrame (0,0,128,64,3);
  u8g.setColorIndex(1);
  u8g.drawBox(0, 4, 128, 10);
  u8g.setColorIndex(0);
  u8g.setPrintPos (0, 5); u8g.print (keyboard_header);

  u8g.setColorIndex(1);
  u8g.drawBox(49, 44, 7, 10);
  u8g.setColorIndex(0);
  u8g.drawStr(50, 45, "B");

  u8g.setColorIndex(1);

  u8g.drawStr( 10, 15, "@ # $ _ & - * ! % <");  
  u8g.drawStr( 10, 25, "Q W E R T Y U I O P");  
  u8g.drawStr( 10, 35, "A S D F G H J K L"); 
  u8g.drawStr( 10, 45, "Z X C V   N M");

  u8g.setPrintPos (2, 55); u8g.print (Keyboard_string);
}

void u8g_ConfigWifiSSIDKeyboardShift_n ()
{
  u8g.drawRFrame (0,0,128,64,3);
  u8g.setColorIndex(1);
  u8g.drawBox(0, 4, 128, 10);
  u8g.setColorIndex(0);
  u8g.setPrintPos (0, 5); u8g.print (keyboard_header);

  u8g.setColorIndex(1);
  u8g.drawBox(59, 44, 7, 10);
  u8g.setColorIndex(0);
  u8g.drawStr(60, 45, "N");

  u8g.setColorIndex(1);

  u8g.drawStr( 10, 15, "@ # $ _ & - * ! % <");  
  u8g.drawStr( 10, 25, "Q W E R T Y U I O P");  
  u8g.drawStr( 10, 35, "A S D F G H J K L"); 
  u8g.drawStr( 10, 45, "Z X C V B   M");

  u8g.setPrintPos (2, 55); u8g.print (Keyboard_string);
}

void u8g_ConfigWifiSSIDKeyboardShift_m ()
{
  u8g.drawRFrame (0,0,128,64,3);
  u8g.setColorIndex(1);
  u8g.drawBox(0, 4, 128, 10);
  u8g.setColorIndex(0);
  u8g.setPrintPos (0, 5); u8g.print (keyboard_header);

  u8g.setColorIndex(1);
  u8g.drawBox(69, 44, 7, 10);
  u8g.setColorIndex(0);
  u8g.drawStr(70, 45, "M");

  u8g.setColorIndex(1);

  u8g.drawStr( 10, 15, "@ # $ _ & - * ! % <");  
  u8g.drawStr( 10, 25, "Q W E R T Y U I O P");  
  u8g.drawStr( 10, 35, "A S D F G H J K L"); 
  u8g.drawStr( 10, 45, "Z X C V B N  ");

  u8g.setPrintPos (2, 55); u8g.print (Keyboard_string);
}