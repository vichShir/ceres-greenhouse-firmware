# Ceres Firmware - vichShir

A Ceres Firmware v1.8.0 foi desenvolvida por vichShir em 2019 em um projeto de TCC em eletrônica pela ETESP para suprir os requisitos de funcionamento da Ceres Greenhouse.
A Ceres Greenhouse é uma estufa automatizada com propósitos de expandir para conectividade remota.

A Ceres Firmaware está sob licença da GNU General Public License v3.0.

## Infomações do Projeto
<p>Para mais detalhes do projeto, a monografia entregue à banca examinadora pode ser verificada <a href="https://drive.google.com/file/d/1BrXHpduY5U_aX7sag3ThyosPkBuHM6GF/view?usp=sharing" target="_blank">Aqui</a></p>

## Créditos aos demais membros participantes:
	• ARTHUR RIO VERDE MELO ROSIN
	• CESAR EDUARDO BOTONI LIMA
	• ELDEN RUFINO DOS SANTOS
	• HENRIQUE MARUITI
	• LUAN ANDRÉ SBARDELLINI

## Documentação Interna
### Pinout do Circuito com Arduino

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

### Bibliotecas necessárias
	#include <Wire.h>
	#include <DS3231.h>
	#include <DHT.h>
	#include "U8glib.h"
	#include <RotaryEncoder.h>
	#include "SoftwareSerial.h"
	#include <EEPROM.h>
