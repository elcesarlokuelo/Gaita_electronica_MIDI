//version final 1 desarrollada por César Núñez Bayón
//gaita gallega electronica midi

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//incluimos librerias de I2C, MPR121, midi, sensor de presion, pantalla y debounce de botones
#include <MIDI.h>
#include <Q2HX711.h>
#include <Wire.h>
#include "Adafruit_MPR121.h"
#include <LiquidCrystal.h>
#include "Adafruit_Debounce.h"
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//cosas del modulo MPR121 de Adafruit
#ifndef _BV
#define _BV(bit) (1 << (bit))
#endif
Adafruit_MPR121 cap = Adafruit_MPR121();  //defino objeto cap del tipo del modulo
uint16_t lasttouched = 0;                 //variables para poder detectar cuando un pin del MPR se ha destapado
uint16_t currtouched = 0;
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//cosas para la comunicacion MIDI
MIDI_CREATE_DEFAULT_INSTANCE();
// Definicion de las notas midi
const int SI3 = 59;
const int DO4 = 60;
const int REB4 = 61;
const int RE4 = 62;
const int MIB4 = 63;
const int MI4 = 64;
const int FA4 = 65;
const int SOLB4 = 66;
const int SOL4 = 67;
const int LAB4 = 68;
const int LA4 = 69;
const int SIB4 = 70;
const int SI4 = 71;
const int DO5 = 72;
const int RE5 = 74;
const int MIB5 = 75;
const int MI5 = 76;
const int FA5 = 77;
int nota;  //variable para reproducir cada nota en funcion de los agujeros tapados
int notaAnterior;
//variables de estado de los agujeros
int uno = 0;
int dos = 0;
int tres = 0;
int cuatro = 0;
int cinco = 0;
int seis = 0;
int siete = 0;
int ocho = 0;
bool boton[11];  //vector de los pines del MPR121
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//variables del sensor de presion del soprete
const byte pin_Datos = A2;                   //en el sensor es el pin OUT
const byte pin_Reloj = A3;                   //en el sensor es SCK
Q2HX711 hx711Soprete(pin_Datos, pin_Reloj);  //defino objeto del tipo de mi sensor
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//variables del sensor de presion del fol
const byte pin_Datos_Fol = A0;                   //en el sensor es el pin OUT
const byte pin_Reloj_Fol = A1;                   //en el sensor es SCK
Q2HX711 hx711Fol(pin_Datos_Fol, pin_Reloj_Fol);  //defino objeto del tipo de mi sensor

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//botones de la pcb. usamos la libreria debounce de adafruit
#define BOTON10 10
#define BOTON11 11
#define BOTON12 12
#define BOTON13 13

Adafruit_Debounce button1(BOTON10, HIGH);
Adafruit_Debounce button2(BOTON11, HIGH);
Adafruit_Debounce button3(BOTON12, HIGH);
Adafruit_Debounce button4(BOTON13, HIGH);
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//control del volumen mediante encoder digital
const int S1 = 2;  //pines del arduino UNO que permiten interrupcion
const int S2 = 3;
unsigned long _lastIncReadTime = micros();
unsigned long _lastDecReadTime = micros();
int _pauseLength = 25000;
int _fastIncrement = 10;
int counter;
int lastCounter;
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//cosas de la pantalla
LiquidCrystal lcd(8, 9, 4, 5, 6, 7);
byte notasimbolo[8] = {
  B00010,
  B00011,
  B00010,
  B00010,
  B01110,
  B11110,
  B11110,
  B01100
};
byte flechaArriba[8] = {
  B00000,
  B00100,
  B01110,
  B10101,
  B00100,
  B00100,
  B00100,
  B00000
};
byte flechaAbajo[8] = {
  B00000,
  B00100,
  B00100,
  B00100,
  B10101,
  B01110,
  B00100,
  B00000
};
byte flechaDerecha[8] = {
  B00000,
  B00000,
  B00100,
  B00010,
  B11111,
  B00010,
  B00100,
  B00000
};
byte flechaIzquierda[8] = {
  B00000,
  B00000,
  B00100,
  B01000,
  B11111,
  B01000,
  B00100,
  B00000
};
int contadorPantalla;  //contador para las diferentes pantallas
int contadorPantallaAnterior;
int afinacion;  //contador para el menu de afinacion
int afinacionAnterior;
int modo;  //contador para el menu de modo
int modoAnterior;
int metronomo;  //contador para el menu del metronomo
int metronomoAnterior;
int reverb;  //contador para el menu del reverb
int reverbAnterior;
int empezar;
int empezarAnterior;
int ronco;
int roncoAnterior;
int punteiro;
int punteiroAnterior;
float presionInicial;
const int volmax = 1000;  //volumen maximo de aire con que se puede llenar
double volaire;           //variable para almacenar el volumen de aire
double volaireAnterior;

unsigned long previousMillis = 0;  // Almacena la última vez que se ejecutó la función
const long interval = 500;         // Intervalo de tiempo en milisegundos

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void setup() {

  pinMode(S1, INPUT_PULLUP);                                         //pin del encoder S1
  pinMode(S2, INPUT_PULLUP);                                         //pin del encoder S2
  attachInterrupt(digitalPinToInterrupt(S1), read_encoder, CHANGE);  //habilito interrupcion en el pin 2
  attachInterrupt(digitalPinToInterrupt(S2), read_encoder, CHANGE);  //habilito interrupcion en el pin 3

  button1.begin();
  button2.begin();
  button3.begin();
  button4.begin();

  MIDI.begin();          //inicio de la comunicacion MIDI
  Serial.begin(115200);  //puerto serie a 115200 baudios
  cap.begin(0x5A);       //inicio de la comunicacion con MPR121

  lcd.createChar(0, notasimbolo);
  lcd.createChar(1, flechaArriba);
  lcd.createChar(2, flechaAbajo);
  lcd.createChar(3, flechaDerecha);
  lcd.createChar(4, flechaIzquierda);
  lcd.begin(16, 2);
  pantallaInicial();

  contadorPantalla = 0;
  contadorPantallaAnterior = 0;
  afinacion = 0;
  afinacionAnterior = 0;
  modo = 0;
  modoAnterior = 0;
  counter = 0;
  lastCounter = 0;
  metronomo = 0;
  metronomoAnterior = 0;
  reverb = 0;
  reverbAnterior = 0;
  nota = 72;
  notaAnterior = 70;
  empezar = 0;
  empezarAnterior = 0;
  ronco = 0;
  roncoAnterior = 0;
  punteiro = 0;
  punteiroAnterior = 0;
  volaire = 0;  //inicializo el volumen de aire en 0
  volaireAnterior = 0;
  nota = DO5;  //incializo la variable nota en DO5s

  presionInicial = hx711Fol.read() / 1000.0;

  MIDI.sendNoteOn(nota, 60, 1);
  delay(10);
  MIDI.sendNoteOff(nota, 60, 1);
  MIDI.sendControlChange(16, 62, 1);
}

void loop() {

  unsigned long currentMillis = millis();
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  //pantallas y menus
  button1.update();
  button2.update();
  button3.update();
  button4.update();
  if (contadorPantalla < 0) {
    contadorPantalla = 7;
  }
  if (contadorPantalla > 7) {
    contadorPantalla = 0;
  }
  if (button4.justPressed()) {  //programacion de cambio de pantalla mediante los botones laterales
    contadorPantalla = contadorPantalla + 1;
  } else if (button2.justPressed()) {
    contadorPantalla = contadorPantalla - 1;
  }
  if (contadorPantalla == 2) {  //en la pantalla 2 si pulso los botones verticales cambio la afinacion
    if (button1.justPressed()) {
      afinacion = afinacion + 1;
      if (afinacion < 0) { afinacion = 4; }
      if (afinacion > 4) { afinacion = 0; }
    } else if (button3.justPressed()) {
      afinacion = afinacion - 1;
      if (afinacion < 0) { afinacion = 4; }
      if (afinacion > 4) { afinacion = 0; }
    }
  }

  if (contadorPantalla == 3) {  //en la pantalla 3 si pulso los botones verticales cambio el modo
    if (button1.justPressed()) {
      modo = modo + 1;
      if (modo < 0) { modo = 2; }
      if (modo > 2) { modo = 0; }
    } else if (button3.justPressed()) {
      modo = modo - 1;
      if (modo < 0) { modo = 2; }
      if (modo > 2) { modo = 0; }
    }
  }

  if (contadorPantalla == 5) {  //en la pantalla 5 si pulso los botones verticales cambio el metronomo de ON a OFF
    if (button1.justPressed()) {
      metronomo = metronomo + 1;
      if (metronomo < 0) { metronomo = 1; }
      if (metronomo > 1) { metronomo = 0; }
    } else if (button3.justPressed()) {
      metronomo = metronomo - 1;
      if (metronomo < 0) { metronomo = 1; }
      if (metronomo > 1) { metronomo = 0; }
    }
  }

  if (contadorPantalla == 6) {  //en la pantalla 6 si pulso los botones verticales cambio el reverb de ON a OFF
    if (button1.justPressed()) {
      reverb = reverb + 1;
      if (reverb < 0) { reverb = 1; }
      if (reverb > 1) { reverb = 0; }
    } else if (button3.justPressed()) {
      reverb = reverb - 1;
      if (reverb < 0) { reverb = 1; }
      if (reverb > 1) { reverb = 0; }
    }
  }

  if (contadorPantalla == 7) {  //en la pantalla 7 empiezo o acabo de tocar
    if (button1.justPressed()) {
      empezar = empezar + 1;
      if (empezar < 0) { empezar = 1; }
      if (empezar > 1) { empezar = 0; }
    } else if (button3.justPressed()) {
      empezar = empezar - 1;
      if (empezar < 0) { empezar = 1; }
      if (empezar > 1) { empezar = 0; }
    }
  }

  switch (contadorPantalla) {
    case 0:
      if (contadorPantalla != contadorPantallaAnterior) {
        lcd.clear();
        pantallaInicial();
      } else {
      }
      break;
    case 1:
      if (contadorPantalla != contadorPantallaAnterior) {
        lcd.clear();
        pantallaInstrucciones();
      } else {
      }
      break;
    case 2:
      if (contadorPantalla != contadorPantallaAnterior || afinacion != afinacionAnterior) {
        lcd.clear();
        pantallaAfinacion();
      } else {
      }
      break;
    case 3:
      if (contadorPantalla != contadorPantallaAnterior || modo != modoAnterior) {
        lcd.clear();
        pantallaModo();
      } else {
      }
      break;
    case 4:
      if (contadorPantalla != contadorPantallaAnterior || counter != lastCounter) {
        lcd.clear();
        pantallaVolumen();
      } else {
      }
      break;
    case 5:
      if (contadorPantalla != contadorPantallaAnterior || metronomo != metronomoAnterior) {
        lcd.clear();
        pantallaMetronomo();
      } else {
      }
      break;
    case 6:
      if (contadorPantalla != contadorPantallaAnterior || reverb != reverbAnterior) {
        lcd.clear();
        pantallaReverb();
      } else {
      }
      break;
    case 7:
      if (contadorPantalla != contadorPantallaAnterior || empezar != empezarAnterior) {
        lcd.clear();
        pantallaEmpezar();
      } else {
      }
      break;
  }

  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  //funciones MIDI de los menus
  if (metronomo != metronomoAnterior) {
    if (metronomo == 0) {
      MIDI.sendControlChange(14, 0, 1);
    } else if (metronomo == 1) {
      MIDI.sendControlChange(14, 127, 1);
    }
  }

  if (reverb != reverbAnterior) {
    if (reverb == 0) {
      MIDI.sendControlChange(15, 0, 1);
    } else if (reverb == 1) {
      MIDI.sendControlChange(15, 127, 1);
    }
  }

  if (afinacion != afinacionAnterior) {
    if (afinacion == 0) {
      MIDI.sendControlChange(16, 62, 1);
    } else if (afinacion == 1) {
      MIDI.sendControlChange(16, 72, 1);
    } else if (afinacion == 2) {
      MIDI.sendControlChange(16, 55, 1);
    } else if (afinacion == 3) {
      MIDI.sendControlChange(16, 60, 1);
    } else if (afinacion == 4) {
      MIDI.sendControlChange(16, 50, 1);
    }
  }

  // /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  //reading del modulo mpr121
  currtouched = cap.touched();                                //leo que electrodos se estan tocando
  for (uint8_t i = 0; i < 8; i++) {                           //recorro las 11 entradas del modulo mpr121
    if ((currtouched & _BV(i)) && !(lasttouched & _BV(i))) {  //si se ha tocado y antes no lo estaba
      boton[i] = 1;                                           //pongo a 1 la componente del vector correspondiente a el electrodo tocado
    }
    if (!(currtouched & _BV(i)) && (lasttouched & _BV(i))) {  //si estaba tocado y ahora ya no
      boton[i] = 0;                                           //pongo a 0 la componente del vector correspondiente a el electrodo soltado
    }
  }
  lasttouched = currtouched;  //actualizo la variable de tocado

  // //traduzco de las componentes del vector a variables mas intuitivas
  ocho = boton[0];
  siete = boton[1];
  seis = boton[2];
  cinco = boton[3];
  cuatro = boton[4];
  tres = boton[5];
  dos = boton[6];
  uno = boton[7];

  //modo cero solo toco mediante el punteiro
  if (modo == 0) {
    if (empezar == 1) {
      nota = generaNotas();
      if (nota != notaAnterior) {
        MIDI.sendNoteOn(nota, 60, 1);
      }
    }
  }
  if (modo == 0) {
    if (empezar == 1 && empezarAnterior == 0) {
      MIDI.sendControlChange(3, 127, 1);
    } else if (empezar == 0 && empezarAnterior == 1) {
      MIDI.sendNoteOff(nota, 60, 1);
      MIDI.sendControlChange(3, 0, 1);
    }
  }

  //modo 1 toco mediante el punteiro y el fol apretandolo
  if (modo == 1) {
    if (empezar == 1) {
      nota = generaNotas();
      if (currentMillis - previousMillis >= interval) {
        previousMillis = currentMillis;
        ronco = compruebaPresionFol();
        punteiro = compruebaPresionPunteiro();
      }
      if (ronco != roncoAnterior && ronco == HIGH) {
        MIDI.sendControlChange(3, 127, 1);
      } else if (ronco != roncoAnterior && ronco == LOW) {
        MIDI.sendControlChange(3, 0, 1);
      }
      if (punteiro == HIGH && punteiroAnterior == LOW) {
        MIDI.sendNoteOn(nota, 60, 1);
      } else if (punteiro == LOW && punteiroAnterior == HIGH) {
        MIDI.sendNoteOff(nota, 60, 1);
      }
      if (punteiro == HIGH) {
        if (nota != notaAnterior) {
          MIDI.sendNoteOn(nota, 60, 1);
        }
      }
    } else if (empezar == 0 && empezarAnterior == 1) {
      MIDI.sendNoteOff(nota, 60, 1);
    }
  }



  //modo 2 toco tapando los agujeros del punteiro, apretando el fol y soplando por el tubo
  if (modo == 2) {
    if (empezar == 1) {
      nota = generaNotas();
      if (currentMillis - previousMillis >= interval) {
        previousMillis = currentMillis;
        volaire = compruebaPresionSoprete();
        ronco = compruebaPresionFol();
        punteiro = compruebaPresionPunteiro();
      }
      if (volaire > 0) {
        if (ronco != roncoAnterior && ronco == HIGH) {
          MIDI.sendControlChange(3, 127, 1);
        } else if (ronco != roncoAnterior && ronco == LOW) {
          MIDI.sendControlChange(3, 0, 1);
        }
        if (punteiro == HIGH && punteiroAnterior == LOW) {
          MIDI.sendNoteOn(nota, 60, 1);
        } else if (punteiro == LOW && punteiroAnterior == HIGH) {
          MIDI.sendNoteOff(nota, 60, 1);
        }
        if (punteiro == HIGH) {
          if (nota != notaAnterior) {
            MIDI.sendNoteOn(nota, 60, 1);
          }
        }
      } else if (volaire == 0 && volaireAnterior > 0) {
        MIDI.sendControlChange(3, 0, 1);
        MIDI.sendNoteOff(nota, 60, 1);
      }
    } else if (empezar == 0 && empezarAnterior == 1) {
      MIDI.sendNoteOff(nota, 60, 1);
      MIDI.sendControlChange(3, 0, 1);
    }
  }

  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  //encoder para control del volumen
  if (counter != lastCounter) {
    lastCounter = counter;
    MIDI.sendControlChange(1, counter, 1);
  }
  //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  contadorPantallaAnterior = contadorPantalla;
  afinacionAnterior = afinacion;
  modoAnterior = modo;
  metronomoAnterior = metronomo;
  reverbAnterior = reverb;
  notaAnterior = nota;
  empezarAnterior = empezar;
  roncoAnterior = ronco;
  punteiroAnterior = punteiro;
  volaireAnterior = volaire;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int generaNotas() {
  // //generacion de las notas en funcion del agujero tapado
  // //en funcion de si tapamos agujero 8 tenemos la 1a octava o la 2a
  if (ocho == 1)  //primera octava, Seguimos el esquema dado en dixitacion
  {
    if (uno == 1 && dos == 1 && tres == 1 && cuatro == 1 && cinco == 1 && seis == 1 && siete == 1) {
      nota = SI3;  //
    } else if (uno == 0 && dos == 1 && tres == 1 && cuatro == 1 && cinco == 1 && seis == 1 && siete == 1) {
      nota = DO4;  //
    } else if ((uno == 0 && dos == 0 && tres == 1 && cuatro == 1 && cinco == 1 && seis == 1 && siete == 1) || (uno == 1 && dos == 0 && tres == 1 && cuatro == 0 && cinco == 1 && seis == 1 && siete == 1)) {
      nota = RE4;  //
    } else if (uno == 0 && dos == 1 && tres == 0 && cuatro == 1 && cinco == 1 && seis == 1 && siete == 1) {
      nota = MIB4;
    } else if ((uno == 0 && dos == 0 && tres == 0 && cuatro == 1 && cinco == 1 && seis == 1 && siete == 1) || (uno == 1 && dos == 0 && tres == 0 && cuatro == 1 && cinco == 1 && seis == 1 && siete == 1)) {
      nota = MI4;  //
    } else if ((uno == 1 && dos == 1 && tres == 1 && cuatro == 0 && cinco == 1 && seis == 1 && siete == 1) || (uno == 0 && dos == 1 && tres == 1 && cuatro == 0 && cinco == 1 && seis == 1 && siete == 1) || (uno == 0 && dos == 0 && tres == 1 && cuatro == 0 && cinco == 1 && seis == 1 && siete == 1)) {
      nota = MI4;  //
    } else if ((uno == 0 && dos == 0 && tres == 0 && cuatro == 0 && cinco == 1 && seis == 1 && siete == 1) || (uno == 1 && dos == 0 && tres == 0 && cuatro == 0 && cinco == 1 && seis == 1 && siete == 1)) {
      nota = FA4;  //
    } else if (uno == 0 && dos == 0 && tres == 0 && cuatro == 1 && cinco == 0 && seis == 1 && siete == 1) {
      nota = SOLB4;
    } else if ((uno == 0 && dos == 0 && tres == 0 && cuatro == 0 && cinco == 0 && seis == 1 && siete == 1) || (uno == 1 && dos == 0 && tres == 0 && cuatro == 0 && cinco == 0 && seis == 1 && siete == 1)) {
      nota = SOL4;  //
    } else if ((uno == 0 && dos == 0 && tres == 0 && cuatro == 0 && cinco == 1 && seis == 0 && siete == 1) || (uno == 1 && dos == 0 && tres == 0 && cuatro == 0 && cinco == 1 && seis == 0 && siete == 1)) {
      nota = LAB4;
    } else if ((uno == 0 && dos == 0 && tres == 0 && cuatro == 0 && cinco == 0 && seis == 0 && siete == 1) || (uno == 1 && dos == 0 && tres == 0 && cuatro == 0 && cinco == 0 && seis == 0 && siete == 1)) {
      nota = LA4;  //
    } else if ((uno == 0 && dos == 0 && tres == 0 && cuatro == 0 && cinco == 0 && seis == 1 && siete == 0) || (uno == 1 && dos == 0 && tres == 0 && cuatro == 0 && cinco == 0 && seis == 1 && siete == 0)) {
      nota = SIB4;
    } else if ((uno == 0 && dos == 0 && tres == 0 && cuatro == 0 && cinco == 0 && seis == 0 && siete == 0) || (uno == 1 && dos == 0 && tres == 0 && cuatro == 0 && cinco == 0 && seis == 0 && siete == 0)) {
      nota = SI4;  //
    } else if (uno == 1 && dos == 0 && tres == 0 && cuatro == 0 && cinco == 0 && seis == 0 && siete == 0) {
      nota = SI4;  //
    }
    //permitir picados  a partir del MI4
    else if ((uno == 0 && dos == 0 && tres == 0 && cuatro == 1 && cinco == 1 && seis == 1 && siete == 0) || (uno == 0 && dos == 0 && tres == 0 && cuatro == 0 && cinco == 1 && seis == 1 && siete == 0)) {
      nota = SIB4;
    } else {
      nota = SIB4;  //para cualquier combinacion de agujeros no contemplada de momento
    }
  } else if (ocho == 0)  //segunda octava
  {
    if (uno == 0 && dos == 0 && tres == 0 && cuatro == 0 && cinco == 0 && seis == 0 && siete == 0) {
      nota = DO5;  //
    } else if (uno == 1 && dos == 1 && tres == 1 && cuatro == 1 && cinco == 1 && seis == 1 && siete == 1) {
      nota = SI4;
    } else if (uno == 0 && dos == 0 && tres == 1 && cuatro == 1 && cinco == 1 && seis == 1 && siete == 1) {
      nota = RE5;
    } else if (uno == 0 && dos == 1 && tres == 0 && cuatro == 1 && cinco == 1 && seis == 1 && siete == 1) {
      nota = MIB5;
    } else if (uno == 0 && dos == 0 && tres == 0 && cuatro == 1 && cinco == 1 && seis == 1 && siete == 1) {
      nota = MI5;
    } else if (uno == 0 && dos == 0 && tres == 0 && cuatro == 0 && cinco == 1 && seis == 1 && siete == 1) {
      nota = FA5;
    } else {
      nota = DO5;
    }
  }
  return nota;
}

int compruebaPresionFol() {
  if ((hx711Fol.read() / 1000.0) > presionInicial + 900) {
    ronco = 1;
  } else {
    ronco = 0;
  }
  return ronco;
}

bool compruebaPresionPunteiro() {
  if ((hx711Fol.read() / 1000.0) > presionInicial + 1000) {
    punteiro = HIGH;
  } else {
    punteiro = LOW;
  }
  return punteiro;
}

int compruebaPresionSoprete() {
  if (hx711Soprete.read() / 1000.0 > 10500) {
    volaire = volaire + 100;
    if (volaire >= volmax) {
      volaire = volmax;
    }
  } else {
    volaire = volaire - 25;
    if (volaire < 0) {
      volaire = 0;
    }
  }
  return volaire;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//funcion del encoder digital para variar el volumen
void read_encoder() {
  // Encoder interrupt routine for both pins. Updates counter
  // if they are valid and have rotated a full indent
  static uint8_t old_AB = 3;                                                                  // Lookup table index
  static int8_t encval = 0;                                                                   // Encoder value
  static const int8_t enc_states[] = { 0, -1, 1, 0, 1, 0, 0, -1, -1, 0, 0, 1, 0, 1, -1, 0 };  // Lookup table

  old_AB <<= 2;  // Remember previous state

  if (digitalRead(S1)) old_AB |= 0x02;  // Add current state of pin A
  if (digitalRead(S2)) old_AB |= 0x01;  // Add current state of pin B

  encval += enc_states[(old_AB & 0x0f)];

  // Update counter if encoder has rotated a full indent, that is at least 4 steps
  if (encval > 3) {  // Four steps forward
    int changevalue = 1;
    if ((micros() - _lastIncReadTime) < _pauseLength) {
      changevalue = _fastIncrement * changevalue;
    }
    _lastIncReadTime = micros();
    counter = counter + changevalue;  // Update counter
    if (counter < 0) {
      counter = 0;
    }
    if (counter > 127) {
      counter = 127;
    }
    encval = 0;
  } else if (encval < -3) {  // Four steps backward
    int changevalue = -1;
    if ((micros() - _lastDecReadTime) < _pauseLength) {
      changevalue = _fastIncrement * changevalue;
    }
    _lastDecReadTime = micros();
    counter = counter + changevalue;  // Update counter
    if (counter < 0) {
      counter = 0;
    }
    if (counter > 127) {
      counter = 127;
    }
    encval = 0;
  }
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//funciones para las pantallas
void pantallaInicial() {
  lcd.setCursor(0, 0);
  lcd.print("Gaita");
  lcd.setCursor(7, 0);
  lcd.write(byte(0));
  lcd.setCursor(0, 1);
  lcd.print("electronica MIDI");
}
void pantallaInstrucciones() {
  lcd.setCursor(0, 0);
  lcd.print("Cambia menu");
  lcd.setCursor(13, 0);
  lcd.write(byte(4));
  lcd.setCursor(14, 0);
  lcd.write(byte(3));

  lcd.setCursor(0, 1);
  lcd.print("Ajusta valor");
  lcd.setCursor(13, 1);
  lcd.write(byte(1));
  lcd.setCursor(14, 1);
  lcd.write(byte(2));
}

void pantallaEmpezar() {
  lcd.setCursor(0, 0);
  lcd.print("Empezar");
  lcd.setCursor(0, 1);
  lcd.print("a tocar:");
  lcd.setCursor(9, 1);
  lcd.print("OFF");
  if (empezar != empezarAnterior || empezar != 0) {
    if (empezar == 0) {
      lcd.setCursor(9, 1);
      lcd.print("OFF");
    } else if (empezar == 1) {
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Empezar");
      lcd.setCursor(0, 1);
      lcd.print("a tocar:");
      lcd.setCursor(9, 1);
      lcd.print("ON");
    }
  }
}

void pantallaAfinacion() {
  lcd.setCursor(0, 0);
  lcd.print("Afinacion:");
  lcd.setCursor(11, 0);
  lcd.print("DO");
  if (afinacion != afinacionAnterior || afinacion != 0) {
    if (afinacion == 0) {
      lcd.setCursor(11, 0);
      lcd.print("DO");
    } else if (afinacion == 1) {
      lcd.setCursor(11, 0);
      lcd.print("RE");
    } else if (afinacion == 2) {
      lcd.setCursor(11, 0);
      lcd.print("SIb");
    } else if (afinacion == 3) {
      lcd.setCursor(11, 0);
      lcd.print("SI");
    } else if (afinacion == 4) {
      lcd.setCursor(11, 0);
      lcd.print("LA");
    }
  }
}

void pantallaModo() {
  lcd.setCursor(0, 0);
  lcd.print("Modo:");
  lcd.setCursor(6, 0);
  lcd.print("punteiro");
  if (modo != modoAnterior || modo != 0) {
    if (modo == 0) {
      lcd.setCursor(6, 0);
      lcd.print("punteiro");
    } else if (modo == 1) {
      lcd.setCursor(6, 0);
      lcd.print("punteiro");
      lcd.setCursor(0, 1);
      lcd.print("y fol");
    } else if (modo == 2) {
      lcd.setCursor(6, 0);
      lcd.print("punteiro");
      lcd.setCursor(0, 1);
      lcd.print("fol y soprete");
    }
  }
}

void pantallaVolumen() {
  lcd.setCursor(0, 0);
  lcd.print("Volumen:");
  lcd.setCursor(0, 1);
  lcd.print("gira el encoder");
  if (counter != lastCounter) {
    lcd.setCursor(9, 0);
    lcd.print(int(counter * 100 / 127));
    lcd.setCursor(13, 0);
    lcd.print("%");
  }
}

void pantallaMetronomo() {
  lcd.setCursor(0, 0);
  lcd.print("Metronomo:");
  lcd.setCursor(11, 0);
  lcd.print("OFF");
  if (metronomo != metronomoAnterior || metronomo != 0) {
    if (metronomo == 0) {
      lcd.setCursor(11, 0);
      lcd.print("OFF");
    } else if (metronomo == 1) {
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Metronomo:");
      lcd.setCursor(11, 0);
      lcd.print("ON");
    }
  }
}

void pantallaReverb() {
  lcd.setCursor(0, 0);
  lcd.print("2a voz:");
  lcd.setCursor(8, 0);
  lcd.print("OFF");
  if (reverb != reverbAnterior || reverb != 0) {
    if (reverb == 0) {
      lcd.setCursor(8, 0);
      lcd.print("OFF");
    } else if (reverb == 1) {
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("2a voz:");
      lcd.setCursor(8, 0);
      lcd.print("ON");
    }
  }
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
