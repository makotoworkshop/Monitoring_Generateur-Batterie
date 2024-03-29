// Ressources :
//https://forum.arduino.cc/index.php?topic=553743.0
//https://plaisirarduino.fr/moniteur-serie/

#include "ESPinfluxdb.h"    // lib ESP InfluxDB
#include <SoftwareSerial.h> // lib Transmission série
#include <ESP8266WiFi.h>    // lib ESP Wifi
#include <ESP8266WiFiMulti.h>

//################
//# DÉCLARATIONS #
//################
//——— Radio HC-12 ———//
SoftwareSerial HC12(D5, D6);  // HC-12 TX Pin et RX Pin
char acquis_data;
String chaine;
float Voltage=12;
float MiniC=0.08;             // courant minimum mesuré en dessous duquel la valeur est forcée à 0.
float tension_batterie_float;
float Courant_float;

//——— InfluxDB ———//
const char *INFLUXDB_HOST = "xx.org";
const uint16_t INFLUXDB_PORT = xx;
const char *DATABASE = "xx";
const char *DB_USER = "xx";
const char *DB_PASSWORD = "xxxx";
Influxdb influxdb(INFLUXDB_HOST, INFLUXDB_PORT);  //https://github.com/projetsdiy/grafana-dashboard-influxdb-exp8266-ina219-solar-panel-monitoring/tree/master/solar_panel_monitoring
int compteur = 1;

//——— WiFi ———//
char ssid[] = "xx";
char password[] = "xxxxx";
ESP8266WiFiMulti WiFiMulti;

//#########
//# SETUP #
//#########
void setup() {
  Serial.begin(9600); // Debug
//——— HC12 ———//
  HC12.begin(9600);           // Serial port to HC12
        
//——— WiFi ———//
  WiFiMulti.addAP(ssid, password);  // Connection au réseau WiFi
  while (WiFiMulti.run() != WL_CONNECTED) {
    delay(100);
  }
  Serial.println ("WiFi Connecté"); 
  
//——— InfluxDB ———//
  while (influxdb.opendb(DATABASE, DB_USER, DB_PASSWORD)!=DB_SUCCESS) { // Connexion à la base InfluxDB
    delay(10000);
  }
  Serial.println ("Connexion DataBase OK"); 
}

//#############
//# PROGRAMME #
//#############
void loop() {
  Reception();
  if (compteur > 100) { // envoie la data toutes les 6 secondes (delay 10 x 100 = 1000 ms, et l'opération prend 5 secondes à transférer/écrire en base)
    SendDataToInfluxdbServer();  
    compteur = 1;
  }
  else {
    compteur++;
  }
  delay(10);  // refresh la data sur l'afficheur toutes les 10 ms
} 

//#############
//# FONCTIONS #
//#############
void Reception() {
// surtout pas de delay dans cette boucle, sinon les data reçues sont erronnées.
  while (HC12.available()) {        // If HC-12 has data
    acquis_data = HC12.read();
    chaine = chaine + acquis_data;
//    Serial.println (chaine);      // Attention, chaine est donc une String
/* message reçu de la forme 
12.665 VOL / 0.045 AMP
pour chaque ligne on fait :*/

    if (chaine.endsWith("\n")) {      //détection de fin de ligne : méthodes String
//      Serial.println ("fin de ligne");          // debug
//      String phrase = "12.665 VOL / 0.045 AMP";  //debug
      char tension_batterie[5];   // chaine de 6 caractères pour stocker le texte avant le mot VOL
      char Courant[3];            // chaine de 4 caractères pour stocker le texte avant le mot AMP
//      sscanf(phrase.c_str(), "%s VOL / %s AMP", tension_batterie, &Courant);  //debug
      sscanf(chaine.c_str(), "%s VOL / %s AMP", tension_batterie, &Courant);  // la chaine à parser est dans une String, avec la méthode c_str()

      tension_batterie_float = atof(tension_batterie),3;  // char convertie en Float, avec 3 décimales
      Courant_float = atof(Courant),2;                    // char convertie en Float, avec 2 décimales

      Serial.print("VOLTS: ");
      Serial.println(tension_batterie_float,3); // float avec 3 décimales
      Serial.print("AMPERES: ");
      Serial.println(Courant_float,3);
      Serial.print("Watt: ");
      Serial.println(Courant_float*Voltage,0); // float avec 0 décimales
      Serial.println(' ');
      
// Affichage LCD courant et Puissance
      if ( Courant_float < MiniC ){   // remise à zero forcée si valeur mesurée très petite
        Courant_float = 0;
      }

      chaine = "";    // vide la String chaine
    }
  }
}

void SendDataToInfluxdbServer() { //Writing data with influxdb HTTP API: https://docs.influxdata.com/influxdb/v1.5/guides/writing_data/
                                  //Querying Data: https://docs.influxdata.com/influxdb/v1.5/query_language/
  dbMeasurement rowDATA("Data");
  rowDATA.addField("tension_batterie", tension_batterie_float);
  rowDATA.addField("Courant", Courant_float);
//  Serial.println(influxdb.write(rowDATA) == DB_SUCCESS ? " - rowDATA write success" : " - Writing failed");
  influxdb.write(rowDATA);

  // Vide les données - Empty field object.
  rowDATA.empty();
}
