#include <AccelStepper.h>
#include <Servo.h>
#include <nRF24L01.h>
#include <RF24_config.h>
#include <RF24.h>

const unsigned int radioCS = 48;
const unsigned int radioCE = 46;

//Pin direzione e step per il motore X
const unsigned int dirX = 6;
const unsigned int stepX = 7;
const unsigned int dirY = 8;
const unsigned int stepY = 9;
const unsigned int pinEnable = 10;
const unsigned int pinServo = 11;
const byte address[6] = "00001";
struct Packet {
  boolean muoviX;
  long speedX;
  boolean muoviY;
  long speedY;
  boolean enable;
  boolean trigger;
};
const unsigned int maxSpeed = 1000;
const unsigned int minSpeed = 0;
const float accelerazione = 50.0; 

//Creo istanze dei motori
AccelStepper motoreX(AccelStepper::DRIVER, stepX, dirX);
AccelStepper motoreY(AccelStepper::DRIVER, stepY, dirY);

//Creo istanza della "radio" passandogli il numero dei pin collegati a CE e CSN del modulo
RF24 radio(radioCE, radioCS);

//Creo istanza del servo motor
Servo servo;

//Creo ed inizializzo istanza pacchetto che userò per i dati ricevuti
Packet pkt = {
  //boolean muoviX;
  false,
  //long speedX;
  0,
  //boolean muoviY;
  false,
  //long speedY;
  0,
  //boolean enable;
  false,
  //boolean trigger;
  false
};

void setup() {
  //Definizione delle modalità dei pin

  //Pin Enable dei Driver
  pinMode(pinEnable, OUTPUT);

  //Dico ll'istanza del servo su quale pin interfacciarsi
  servo.attach(pinServo);

  //Configura parametri dei motori
  motoreX.setMaxSpeed(maxSpeed);
  motoreX.setSpeed(minSpeed);
  motoreX.setAcceleration(accelerazione);

  motoreY.setMaxSpeed(maxSpeed);
  motoreY.setSpeed(minSpeed);
  motoreY.setAcceleration(accelerazione);

  //Inizialmente lascio i driver disabilitati
  digitalWrite(pinEnable, !pkt.enable);

  //Inizializzo la radio
  radio.begin();
  radio.openReadingPipe(0, address);
  radio.setDataRate(RF24_250KBPS);
  radio.setPALevel(RF24_PA_LOW);
  radio.startListening();
}

void loop() {

  //Se ci sono dati in ricezione sulla radio
  if (radio.available()) {

    //Leggo i dati sul buffer e li scrivo nell'istanza Packet precedentemente creata
    radio.read(&pkt, sizeof(pkt));

  } else {

    //Se non ricevo dati per un qualsiasi motivo, azzero tutto nell'istanza Packet.
    pkt = {
      false,
      0,
      false,
      0,
      false,
      false
    };

  }

  //Se nel pkt il valore del trigger è 1 ruoto di 90° il servo motor
  if (pkt.trigger) {
    servo.write(90);
  } else {
    //Altrimenti lo rimetto a 0
    servo.write(0);
  }

  //Interpreta i valori ricevuti ed aziona i motori di conseguenza
  pilotaMotori(pkt);

  delay(15);
}

/*
  Interpreta i valori contenuti nella struttura Packet
  ed aziona i motori di conseguenza
*/
void pilotaMotori(Packet pkt) {

  //abilita o disabilita i driver
  digitalWrite(pinEnable, !pkt.enable);

  if (pkt.muoviX) {
    motoreX.setSpeed(pkt.speedX);
    motoreX.run();
  } else {
    motoreX.stop();
  }

  if (pkt.muoviY) {
    motoreY.setSpeed(pkt.speedY);
    motoreY.run();
  } else {
    motoreY.stop();
  }

}
