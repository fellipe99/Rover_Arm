/*
   Pilotare due motori passo passo a distanza con Arduino

   Autore  : Andrea Lombardo
   Web     : http://www.lombardoandrea.com
   Post    : https://wp.me/p27dYH-Oz
*/

//Inclusione delle librerie
#include <Bounce2.h>
#include <nRF24L01.h>
#include <RF24_config.h>
#include <RF24.h>

//Costanti e PIN

//Pin pulsante incorporato nel modulo joystick che abilita o disabilita il controllo dei motori
const unsigned int pinSwEnable = 2;

//Pin pulsante esterno che controllerà il servo
const unsigned int pinSwTrigger = 3;

//Pin per il LED di stato
const unsigned int ledEnable = 7;

//Chip Select e Chip Enable della Radio
const unsigned int radioCE = 9;
const unsigned int radioCS = 10;

//Pin analogici per il joystick
const unsigned int jX = A0;
const unsigned int jY = A1;

//Definizione indirizzo sul quale stabilire la comunicazione radio
const byte address[6] = "00001";

/*
  Variabili utilizzate per definire min e max speed ed eseguire il mapping sui valori del joystick.
  Stando alla documentazione della libreria il valore max può essere impostato fino a 4000 per un Arduino UNO.
*/
const unsigned int maxSpeed = 1000;
const unsigned int minSpeed = 0;

/*
  La lettura dei potenziometri non è mai affidabile al 100%.
  Questo valore aiuta a determinare il punto da considerare come "Stai fermo" nei movimenti.
*/
const unsigned int treshold = 30;

//Millisecondi per il debonuce del bottone
const unsigned long debounceDelay = 10;

//Definisco struttura pacchetto da inviare
struct Packet {
  boolean muoviX;
  long speedX;
  boolean muoviY;
  long speedY;
  boolean enable;
  boolean trigger;
};

//Variabili di appoggio
long  valX, mapX, valY, mapY, tresholdUp, tresholdDown;

//Creo istanze dei bottoni
Bounce btnEnable = Bounce();  //istanzia un bottone dalla libreria Bounce
Bounce btnTrigger = Bounce();  //istanzia un bottone dalla libreria Bounce

//Creo istanza della "radio" passandogli il numero dei pin collegati a CE e CSN del modulo
RF24 radio(radioCE, radioCS);

//Creo ed inizializzo istanza pacchetto da inviare
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

  //LED Enable
  pinMode(ledEnable, OUTPUT);

  //Tasto enable
  pinMode(pinSwEnable, INPUT_PULLUP);

  //Tasto grilletto
  pinMode(pinSwTrigger, INPUT_PULLUP);


  //Inizializzo la radio
  radio.begin();
  radio.openWritingPipe(address);
  radio.setPALevel(RF24_PA_LOW);
  radio.stopListening();

  //Configuro istanze dei pulsanti
  btnEnable.attach(pinSwEnable);
  btnEnable.interval(debounceDelay);

  btnTrigger.attach(pinSwTrigger);
  btnTrigger.interval(debounceDelay);

  //Calcolo range valori entro i quali considerare la posizione del joystick come "Stai fermo"
  tresholdDown = (maxSpeed / 2) - treshold;
  tresholdUp = (maxSpeed / 2) + treshold;

  //Invio stato di enable al LED
  digitalWrite(ledEnable, pkt.enable);
}

void loop() {

  //gestisci stato dei pulsanti
  handlePulsanti();

  //gestisci valori dei potenziometri
  handleJoystick();

  //Invia dati tramite la radio
  radio.write(&pkt, sizeof(pkt));

}

/*
  Si occupa di leggere i valori del joystick, mapparli ed aggiornare le variabili nel Packet
*/
void handleJoystick() {

  //esegui lettura analogica dei valori provenienti dai potenziometri del joystick
  valX = analogRead(jX);
  valY = analogRead(jY);

  //mappa i valori letti in funzione della velocità inima e massima
  mapX = map(valX, 0, 1023, minSpeed, maxSpeed);
  mapY = map(valY, 0, 1023, minSpeed, maxSpeed);

  if (mapX <= tresholdDown) {
    //x va indietro
    pkt.speedX = -map(mapX, tresholdDown, minSpeed,   minSpeed, maxSpeed);
    pkt.muoviX = true;
  } else if (mapX >= tresholdUp) {
    //x va avanti
    pkt.speedX = map(mapX,  maxSpeed, tresholdUp,  maxSpeed, minSpeed);
    pkt.muoviX = true;
  } else {
    //x sta fermo
    pkt.speedX = 0;
    pkt.muoviX = false;
  }

  if (mapY <= tresholdDown) {
    //y va giù
    pkt.speedY = -map(mapY, tresholdDown, minSpeed,   minSpeed, maxSpeed);
    pkt.muoviY = true;
  } else if (mapY >= tresholdUp) {
    //y va su
    pkt.speedY = map(mapY,  maxSpeed, tresholdUp,  maxSpeed, minSpeed);
    pkt.muoviY = true;
  } else {
    //y sta fermo
    pkt.speedY = 0;
    pkt.muoviY = false;
  }

}


/*
  Si occupa di leggere lo stato dei pulsanti ed aggiornare le variabili nel Packet
*/
void handlePulsanti() {

  btnEnable.update();
  if (btnEnable.fell()) {
    pkt.enable = !pkt.enable;
  }

  //Mostra lo stato di enable con il LED
  digitalWrite(ledEnable, pkt.enable);

  //Aggiorna stato di pressione del "grilletto"
  btnTrigger.update();
  pkt.trigger = !btnTrigger.read();

}
