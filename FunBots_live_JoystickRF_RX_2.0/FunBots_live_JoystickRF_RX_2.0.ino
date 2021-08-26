/*
      Programa para RX de um controle RF com módulo NRF24L01 - RECEPTOR

      Componentes:
        - Arduino Uno;
        - Módulo NRF24L01 com adaptador;

      Pinos:
      RF      UNO
      CE      9
      CSN     10
      SCK     13
      MISO    12
      MOSI    11

      Versão 1.0 - Versão inicial com leitura multicanais de dados recebidos via rádio - 15/mai/2021; programa base fonte: by Dejan Nedelkovski, www.HowToMechatronics.com
             2.0 - Versão de teste do controle de PS2 - 05/06/21

 *    * Criado por Cleber Borges - FunBots - @cleber.funbots  *     *

      Instagram: https://www.instagram.com/cleber.funbots/
      Facebook: https://www.facebook.com/cleber.funbots
      YouTube: https://www.youtube.com/c/funbots
      Telegram: https://t.me/cleberfunbots

*/

#include <Servo.h>
#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

Servo Servo1;

RF24 radio(10, 9); // CE, CSN

uint64_t address[6] = {0x7878787878LL,
                       0xB3B4B5B6F1LL,
                       0xB3B4B5B6CDLL,
                       0xB3B4B5B6A3LL,
                       0xB3B4B5B60FLL,
                       0xB3B4B5B605LL
                      };

struct joystick {
  byte potLX;
  byte potLY;
  byte potRX;
  byte potRY;
  byte buttons1;
  byte buttons2;
};

joystick joystickData;

//int button0 = 0;
//int button1 = 0;
int buttonX = 0;
int buttonR2 = 0;
int buttonL2 = 0;
int servo1pos = 0;

// Define pinos e steps
//#define dirPin A0
//#define stepPin A1
//#define stepsVolta 48

// Motor Direita
#define pino_enableDIR   9
#define pino_motorDIR_A  5
#define pino_motorDIR_B  4

// Motor Esquerda
#define pino_enableESQ   8
#define pino_motorESQ_A  2
#define pino_motorESQ_B  3

#define pino_Servo  6

// Variáveis
int velGiro = 127;
int velY = 0;
int sentido = 0;

void setup() {
  // Inicia Serial
  Serial.begin(57600);
  Serial.println("Iniciando");

  Servo1.attach(pino_Servo);

  // Configura Pinos dos Motores como saída
  pinMode(pino_enableDIR, OUTPUT);
  pinMode(pino_enableESQ, OUTPUT);
  pinMode(pino_motorDIR_A, OUTPUT);
  pinMode(pino_motorDIR_B, OUTPUT);
  pinMode(pino_motorESQ_A, OUTPUT);
  pinMode(pino_motorESQ_B, OUTPUT);

  // configura motor de passo
 // pinMode(stepPin, OUTPUT);
  //pinMode(dirPin, OUTPUT);


  // Configura e inicializa RF
  if (!radio.begin()) {
    Serial.println(F("radio não responde!!"));
    while (1) {} // preso em loop
  }

  radio.setPALevel(RF24_PA_LOW);

  for (uint8_t i = 0; i < 6; ++i)
    radio.openReadingPipe(i, address[i]);

  radio.startListening();

  //zeraVariaveis();
}

void loop() {
  // Checa se RF recebeu dados
  if (radio.available()) {
    radio.read(&joystickData, sizeof(joystick));

  }

  // Transfere dados dos botões para variáveis
  // Criar conforme necessidade para cada botão
  //button0 = !bitRead(joystickData.buttons1, 0);
  //button1 = !bitRead(joystickData.buttons1, 1);

  //  bit   7   6  5  4  3  2  1  0
  // valor  128 64 32 16 8  4  2  1
  //  botao  0  X  0  0  0  0 R2 L2

  buttonX = bitRead(joystickData.buttons2, 6);
  buttonR2 = bitRead(joystickData.buttons2, 1);
  buttonL2 = bitRead(joystickData.buttons2, 0);

  // Imprime na Serial dados recebidos
  Serial.print("Pot LX: ");
  Serial.print(joystickData.potLX);
  Serial.print("; Pot LY: ");
  Serial.print(joystickData.potLY);
  Serial.print("; Pot RX: ");
  Serial.print(joystickData.potRX);
  Serial.print("; Pot RY: ");
  Serial.print(joystickData.potRY);

  Serial.print("; Botão 1: ");
  Serial.print(joystickData.buttons1);
  //Serial.print("; Botão 2: ");
  //Serial.println(joystickData.buttons2);
  Serial.print("; Botão X: ");
  Serial.println(buttonX);

  if (joystickData.potLY < 130) {
    velY = map(joystickData.potLY, 128, 0, 0, 255);
    sentido = 0; // frente
  } else if (joystickData.potLY > 130) {
    velY = map(joystickData.potLY, 128, 255, 0, 255);
    sentido = 1; // tras
  } else {
    velY = 0;
  }

  if (buttonR2 == 1 && buttonL2 == 0) {
    girarR(velGiro);
    //Serial.print(" primeiro if!");
  } else if (buttonL2 == 1 && buttonR2 == 0) {
    girarL(velGiro);
    //Serial.print(" segundo if!");
  } else if (buttonL2 == 0 && buttonR2 == 0) {
    Serial.print(velY);
    if ( sentido == 0) moveFrente(velY);
    if ( sentido == 1) moveTras(velY);
  } else {
    paraCarro();
  }

/*  if (buttonX == 1) {
    Serial.print("motor de passo!");
    digitalWrite(dirPin, HIGH);
    // Faz um giro com o motor
    digitalWrite(stepPin, HIGH);
    delayMicroseconds(250);
    digitalWrite(stepPin, LOW);
    delayMicroseconds(250);
  }
*/
  servo1pos = map(joystickData.potRX, 0, 255, 0, 180);
  Servo1.write(servo1pos);
  delay(50);
}


void moveFrente (int vel) {
  digitalWrite(pino_motorESQ_A, HIGH);
  digitalWrite(pino_motorESQ_B, LOW);
  analogWrite(pino_enableESQ, vel);

  digitalWrite(pino_motorDIR_A, HIGH);
  digitalWrite(pino_motorDIR_B, LOW);
  analogWrite(pino_enableDIR, vel);
}

void moveTras (int vel) {
  digitalWrite(pino_motorESQ_A, LOW);
  digitalWrite(pino_motorESQ_B, HIGH);
  analogWrite(pino_enableESQ, vel);

  digitalWrite(pino_motorDIR_A, LOW);
  digitalWrite(pino_motorDIR_B, HIGH);
  analogWrite(pino_enableDIR, vel);
}

void girarR (int vel) {
  digitalWrite(pino_motorESQ_A, HIGH);
  digitalWrite(pino_motorESQ_B, LOW);
  analogWrite(pino_enableESQ, vel);

  digitalWrite(pino_motorDIR_A, LOW);
  digitalWrite(pino_motorDIR_B, HIGH);
  analogWrite(pino_enableDIR, vel);
}

void girarL (int vel) {
  digitalWrite(pino_motorESQ_A, LOW);
  digitalWrite(pino_motorESQ_B, HIGH);
  analogWrite(pino_enableESQ, vel);

  digitalWrite(pino_motorDIR_A, HIGH);
  digitalWrite(pino_motorDIR_B, LOW);
  analogWrite(pino_enableDIR, vel);
}

void paraCarro() {
  digitalWrite(pino_motorESQ_A, LOW);
  digitalWrite(pino_motorESQ_B, LOW);
  analogWrite(pino_enableESQ, 0);

  digitalWrite(pino_motorDIR_A, LOW);
  digitalWrite(pino_motorDIR_B, LOW);
  analogWrite(pino_enableDIR, 0);
}
