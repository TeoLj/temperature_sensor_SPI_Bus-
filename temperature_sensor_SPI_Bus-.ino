//SPI PINS
//SS = PB2 Arduino Pin 10 //Slave Select
//MOSI = PB3 Arduino Pin 11
//MISO = PB4 Arduino Pin 12
//SCK = PB5 Arduino Pin 13
// Mode 0,0 ADC outputs data on the falling edge of clock

int main() {
  init();
  Serial.begin(9600);

  // SS,MOSI und SCK als Ausgänge festlegen
  // MISO als Eingang konfiguriert
  DDRB = (1 << DDB2) | (1 << DDB3) | (1 << DDB5);

  // SS auf HIGH
  PORTB = (1 << PORTB2);

  // Dataorder MSB zuerst, Modus 0,0 so ist SPI-Bus konfiguriert
  //Modus 00: CPHA-Daten bei 2.Flanke ausgewertet
  SPCR &= ~((1 << DORD) | (1 << CPOL) | (1 << CPHA));

  // SPI enable, Arduino = Master, Fck/8= 2MHz
  SPCR = (1 << SPE) | (1 << MSTR) | (1 << SPR0);
  SPSR = (1 << SPI2X);



  uint8_t fragdata_in1 ;
  uint8_t fragdata_in2 ;

  while (1) {
    // D2 D1 D0 zur Kanalfestlegung am AD-Wandler
    // Gebildet aus 5 leading "0"->Startbit->SGL->D2
    uint8_t data_SetupOut1 = 0b00000110;
    // Gebildet aus D1->D0-> 6 * Don`t care
    uint8_t data_SetupOut2 = 0b01000000; //channel selection: CH0 aus D2 D1 D0 -> Pin1

    // START der Übertragung
    // Slave Select (SS LOW)
    PORTB &= ~(1 << PORTB2);

    // Register zur Übertragung füllen
    SPDR = data_SetupOut1;

    //Auf das Ende der Übertragung warten
    while (!(SPSR & (1 << SPIF)));



    //Übertragung des 2.Bytes , Datenregister SPDR
    SPDR = data_SetupOut2;

    //Warten auf Übertragungsende des 2.byte, und Empfangen der 5 MSB`s
    while (!(SPSR & (1 << SPIF)));


    // Speichern des 1. empfangenen Bytes
    fragdata_in1 = SPDR ;


    // Gesendetes 3.Byte ohne Bedeutung -> don't cares
    SPDR = 1;
    //Warten auf Übertragungsende des 3.byte, und Empfangen der 8 letzten Stellen der AD Auswertung
    while (!(SPSR & (1 << SPIF))); // SPIF: Interrupt Flag

    // Speichern des 2. empfangenen Bytes
    fragdata_in2 = SPDR ;
    // Deselect Slave, da Übertragung beendet ist
    PORTB |= (1 << PORTB2);

    //Auswertung :
    // Die beliebig bits 15-13 müssen auf 0 gesetzt werden.
    // Die bits 11-8 in ihre eigentliche Wertigkeit bringen durch Multiplikation des 1.Bytes mit (2^8)
    // Addieren der beiden 8 bit Fragmente zu einem 12 bit Wert

    uint16_t Analogread = (fragdata_in1 & 0x0F) * 256 + fragdata_in2 ;





    //temperaturberechnung
    //Ausgangsspannung, die über den analogen Eingang des Arduino gemessen wird
    // unsigned long SensorValue = analogRead(A0);  Test des Sensors
    float temperature = ((5.0 * Analogread) / 4096.) * 100;
    Serial.println(Analogread);
    Serial.println("Temperatur");
    Serial.print(temperature);
    delay(1000);

  }
}





