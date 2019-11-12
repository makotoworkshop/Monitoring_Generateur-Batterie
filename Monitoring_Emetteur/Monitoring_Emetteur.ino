#include <SoftwareSerial.h>

/****************/
/* DÉCLARATIONS */
/****************/
SoftwareSerial HC12(10, 11); // HC-12 TX Pin, HC-12 RX Pin

char MessageTensionBatterie[] = " VOL / "; // message to be sent; '\n' is a forced terminator char
char MessageCourant[] = " AMP\n"; // message to be sent; '\n' is a forced terminator char
String chaine;

#define ATpin 7 // used to switch HC-12 to AT mode

float Courant=0;
float SupplyVoltage=12;
const int Voie_0=0; // declaration constante de broche analogique
int mesure_brute=0; // Variable pour acquisition résultat brut de conversion analogique numérique
float mesuref=0.0;  // Variable pour calcul résultat décimal de conversion analogique numérique
float tension=0.0;  // Variable tension mesurée
float tension_batterie=0.0; // Variable tension batterie calculée
float tension_regulateur=8925.0;  // Variable tension réelle aux bornes du régulateur -9V (en mV)

int PIN_ACS712 = A1;
double Voltage = 0;
double Current = 0;

/*********/
/* SETUP */
/*********/  
void setup() {
//  Serial.begin(9600); // Debug
  HC12.begin(9600);               // Serial port to HC12
  // Pin capteurs
  pinMode(PIN_ACS712, INPUT);
  pinMode(ATpin, OUTPUT);
  digitalWrite(ATpin, LOW); // Set HC-12 into AT Command mode
  delay(500);
  HC12.print("AT+C056");  // passer sur le canal 006 (433.4Mhz + 56x400KHz)
  delay(500);
  digitalWrite(ATpin, HIGH); // HC-12 en normal mode
}

/*************/
/* PROGRAMME */
/*************/
void loop() {
  MesureCourant();
  MesureBrute();
  TensionMesuree();
  TensionBatterie();

//  for (int i = 0; i < 10; i++) {
//    Courant = Courant+0.01;
//    if (Courant >= 12){
//      Courant = 0;
//    }
//  }
  
  chaine = String(tension_batterie,3) + MessageTensionBatterie + String(Current,3) + MessageCourant;  // construction du message
  Serial.println ( "chaine String : " +chaine );
  HC12.print(chaine); // send radio data
  delay(100);
}

/*************/
/* FONCTIONS */
/*************/
void MesureCourant() {
//  Serial.print("Courant : ");
//  Serial.print(Courant);
//  Serial.print(" A | Puissance : ");
//  Serial.print(Courant*SupplyVoltage);
//  Serial.println(" Watt");

// Voltage is Sensed 1000 Times for precision
for(int i = 0; i < 1000; i++) { // ça ralentis tout, mais c'est indispensable !
Voltage = (Voltage + (0.004882812 * analogRead(PIN_ACS712))); // (5 V / 1024 (Analog) = 0.0049) which converter Measured analog input voltage to 5 V Range
delay(1);
}
Voltage = Voltage /1000;

Current = (Voltage -2.5)/ 0.100; // Sensed voltage is converter to current (0.100 pour modèle 20A)

Serial.print("\n Voltage Sensed (V) = "); // shows the measured voltage
Serial.print(Voltage,3); // the ‘2’ after voltage allows you to display 2 digits after decimal point
Serial.print("\t Current (A) = "); // shows the voltage measured
Serial.println(Current,3); // the ‘2’ after voltage allows you to display 2 digits after decimal point



}

void MesureBrute() {
//-------- mesure brute --------
  mesure_brute=analogRead(Voie_0);
//  Serial.print ("Valeur brute = "); 
//  Serial.print (mesure_brute); 
//  Serial.println (" "); // espace de propreté
}

void TensionMesuree() {
//---------- tension mesurée ---------
  mesuref=float(mesure_brute)*5000.0/1024.0;
  tension=mesuref/1000.0; // en Volts
//  Serial.print ("Tension = "); 
//  Serial.print(tension,3);  // float avec 2 décimales 
//  Serial.println(" V ");  // unité et espace de propreté
}

void TensionBatterie() {
//---------- tension batterie ---------
  tension_batterie=mesuref+tension_regulateur;
  tension_batterie=tension_batterie/1000.0; // en Volts
//  lcd.setCursor(0,1) ; // positionne le curseur à l'endroit voulu (colonne, ligne)
//  lcd.print ("Batt:"); 
//  lcd.print (tension_batterie,2); // float avec 2 décimales
//  lcd.print ("V "); // unité et espace de propreté
  Serial.print ("Batterie = "); 
  Serial.print(tension_batterie,3); // float avec 2 décimales 
  Serial.println(" V ");
//  Serial.println("  ");
}
