// inspiré du code de  X. HINAULT - Le 12/03/2010 www.mon-club-elec.fr 
// --- Que fait ce programme ? ---
// - Mesure la tension d'une batterie au plomb 12V. 
// - Affichage de la valeur de la mesure continue sur un écran LCD 
// - La tension de la Batterie est abaissé à l'aide d'un régulateur -9V (7909)
// - Ajout d'une jauge de capacité batterie et de la mesure du courant de recharge dans la batterie
// --- Fonctionnalités utilisées ---
// Utilise un afficheur LCD alphanumérique2x20 en mode 4 bits 
// Utilise la conversion analogique numérique 10bits sur les voies analogiques  analog 0, 
// --- Circuit à réaliser ---
// Connecter  sur la broche 12 la broche RS du LCD
// Connecter  sur la broche 11 la broche E du LCD
// Connecter  sur la broche 5 la broche D4 du LCD
// Connecter  sur la broche 4 la broche D5 du LCD
// Connecter  sur la broche 3 la broche D6 du LCD
// Connecter  sur la broche 2 la broche D7 du LCD
// Broche Analog 0 (=broche 14) en entrée Analogique < régulateur
// +++ Réglages possibles +++
// La variable « tension_regulateur » est à ajuster en mesurant au multimétre la tension entre les broche 1 et 3 
// du régulateur, ou en mesurant directement la tension de la batterie pour faire correspondre la mesure Arduino.

#include <LiquidCrystal.h> // Inclusion de la librairie pour afficheur LCD 

//################
//# DÉCLARATIONS #
//################
float Courant=0;
float SupplyVoltage=12;
const int rs = 12, en = 11, d4 = 5, d5 = 4, d6 = 3, d7 = 2; // Déclaration LCD
const int Voie_0=0; // declaration constante de broche analogique
int mesure_brute=0; // Variable pour acquisition résultat brut de conversion analogique numérique
float mesuref=0.0;  // Variable pour calcul résultat décimal de conversion analogique numérique
float tension=0.0;  // Variable tension mesurée
float tension_batterie=0.0; // Variable tension batterie calculée
float tension_regulateur=8925.0;  // Variable tension réelle aux bornes du régulateur -9V (en mV)
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);  // initialisation LCD en mode 4 bits 
byte carre00[] = {  // Déclaration de caractères personnalisés
  B11111,
  B00000,
  B00000,
  B00000,
  B00000,
  B00000,
  B00000,
  B11111
};
byte carre01[] = {
  B11111,
  B00000,
  B10000,
  B10000,
  B10000,
  B10000,
  B00000,
  B11111
};
byte carre02[] = {
  B11111,
  B00000,
  B11000,
  B11000,
  B11000,
  B11000,
  B00000,
  B11111
};
byte carre03[] = {
  B11111,
  B00000,
  B11100,
  B11100,
  B11100,
  B11100,
  B00000,
  B11111
};
byte carre04[] = {
  B11111,
  B00000,
  B11110,
  B11110,
  B11110,
  B11110,
  B00000,
  B11111
};
byte carre05[] = {
  B11111,
  B00000,
  B11111,
  B11111,
  B11111,
  B11111,
  B00000,
  B11111
};
byte crochetouvrant[] = {
  B00011,
  B00010,
  B00010,
  B00010,
  B00010,
  B00010,
  B00010,
  B00011
};
byte crochetfermant[] = {
  B11000,
  B01000,
  B01110,
  B01110,
  B01110,
  B01110,
  B01000,
  B11000
};
byte fleche[] = {
  B00000,
  B00100,
  B00010,
  B11111,
  B00010,
  B00100,
  B00000,
  B00000
};

// Fonction pour pemettre le map avec variable float
float mapfloat(float x, float in_min, float in_max, float out_min, float out_max) {
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

//#########
//# SETUP #
//#########
void setup() {
  lcd.begin(20,4); // Initialise le LCD avec 20 colonnes x 4 lignes 
  delay(10); // pause rapide pour laisser temps initialisation
  lcd.print("LCD OK") ; // affiche la chaîne texte - message de test
  delay(2000); // pause de 2 secondes
  lcd.clear(); // // efface écran et met le curseur en haut à gauche
  delay(10); // pour laisser temps effacer écran
  Serial.begin(9600); // Debug
  lcd.createChar(0, carre00); // Création de caractères personnalisés
  lcd.createChar(1, carre01);
  lcd.createChar(2, carre02);
  lcd.createChar(3, carre03);
  lcd.createChar(4, carre04);
  lcd.createChar(5, carre05);                       
  lcd.createChar(6, crochetouvrant);
  lcd.createChar(7, crochetfermant);
  lcd.createChar(9, fleche);
} 

//#############
//# PROGRAMME #
//#############
void loop() {
  MesureBrute();
  TensionMesuree();
  TensionBatterie();
  MesureCourant();
  Jauge();

  for (int i = 0; i < 10; i++) {
    Courant = Courant+0.01;
    if (Courant >= 12){
      Courant = 0;
    }
  }
  
  delay(1000);
} 

//#############
//# FONCTIONS #
//#############
void MesureCourant() {
  Serial.print("Courant : ");
  Serial.print(Courant);
  Serial.print("   Puissance : ");
  Serial.println(Courant*SupplyVoltage);
  lcd.setCursor(0,0) ; // positionne le curseur à l'endroit voulu (colonne, ligne)
  lcd.print ("Power:"); 
  lcd.print (Courant,2); // float avec 2 décimales
  lcd.print ("A "); // unité et espace de propreté
  lcd.setCursor(12,0) ; // positionne le curseur à l'endroit voulu (colonne, ligne)
//  lcd.print ("> ");
  lcd.write(byte(9)); // fléche  
  lcd.print (" ");
  lcd.print (Courant*SupplyVoltage,0); // float avec 0 décimales
  lcd.print ("Watt ");
}

void MesureBrute() {
//-------- mesure brute --------
  mesure_brute=analogRead(Voie_0);
  Serial.print ("Valeur brute = "); 
  Serial.print (mesure_brute); 
  Serial.println (" "); // espace de propreté
}

void TensionMesuree() {
//---------- tension mesurée ---------
  mesuref=float(mesure_brute)*5000.0/1024.0;
  tension=mesuref/1000.0; // en Volts
  Serial.print ("Tension = "); 
  Serial.print(tension,3);  // float avec 2 décimales 
  Serial.println(" V ");  // unité et espace de propreté
}

void TensionBatterie() {
//---------- tension batterie ---------
  tension_batterie=mesuref+tension_regulateur;
  tension_batterie=tension_batterie/1000.0; // en Volts
  lcd.setCursor(0,1) ; // positionne le curseur à l'endroit voulu (colonne, ligne)
  lcd.print ("Batt:"); 
  lcd.print (tension_batterie,2); // float avec 2 décimales
  lcd.print ("V "); // unité et espace de propreté
  Serial.print ("Batterie = "); 
  Serial.print(tension_batterie,3); // float avec 2 décimales 
  Serial.println(" V ");
  Serial.println("  ");
}

void Jauge() {
//float test = 13;
  lcd.setCursor(12, 1);
  lcd.write(byte(6)); // crochet ouvrant
  lcd.setCursor(19, 1);
  lcd.write(byte(7)); // crochet fermant
//  float NiveauBatterie = mapfloat(test, 11.4, 12.73, 0, 30);   // Discrétise la valeur de la tension batterie
  float NiveauBatterie = mapfloat(tension_batterie, 11.4, 12.73, 0, 30);   // Discrétise la valeur de la tension batterie
  int NiveauBatterieBarre = (int)NiveauBatterie;  // conversion en entier

  // en cas de dépassement des limites haute et basse et permettre l'affichage non-erroné
  if (NiveauBatterie > 30) {
    NiveauBatterieBarre = 30;
    lcd.setCursor(13, 1);
    lcd.print ("Pleine"); 
    delay (1000);
  }
  else if (NiveauBatterie < 0) {
    NiveauBatterieBarre = 0; 
    lcd.setCursor(13, 1);
    lcd.print ("Erreur"); 
    delay (1000);
  }
  Serial.print ("NiveauBatterieBarre = "); 
  Serial.print(NiveauBatterieBarre);
  Serial.println("  ");
  
  switch (NiveauBatterieBarre) {
    case 0:
    lcd.setCursor(13, 1);
    lcd.write(byte(0)); // carre00  
    lcd.setCursor(14, 1);
    lcd.write(byte(0)); // carre00
    lcd.setCursor(15, 1);
    lcd.write(byte(0)); // carre00
    lcd.setCursor(16, 1);
    lcd.write(byte(0)); // carre00
    lcd.setCursor(17, 1);
    lcd.write(byte(0)); // carre00
    lcd.setCursor(18, 1);
    lcd.write(byte(0)); // carre00
      break;
    case 1:
    lcd.setCursor(13, 1);
    lcd.write(byte(1)); // carre01
    lcd.setCursor(14, 1);
    lcd.write(byte(0)); // carre00
    lcd.setCursor(15, 1);
    lcd.write(byte(0)); // carre00
    lcd.setCursor(16, 1);
    lcd.write(byte(0)); // carre00
    lcd.setCursor(17, 1);
    lcd.write(byte(0)); // carre00
    lcd.setCursor(18, 1);
    lcd.write(byte(0)); // carre00
      break;
    case 2:
    lcd.setCursor(13, 1);
    lcd.write(byte(2)); // carre02
    lcd.setCursor(14, 1);
    lcd.write(byte(0)); // carre00
    lcd.setCursor(15, 1);
    lcd.write(byte(0)); // carre00
    lcd.setCursor(16, 1);
    lcd.write(byte(0)); // carre00
    lcd.setCursor(17, 1);
    lcd.write(byte(0)); // carre00
    lcd.setCursor(18, 1);
    lcd.write(byte(0)); // carre00
      break;
    case 3:
    lcd.setCursor(13, 1);
    lcd.write(byte(3)); // carre03
    lcd.setCursor(14, 1);
    lcd.write(byte(0)); // carre00
    lcd.setCursor(15, 1);
    lcd.write(byte(0)); // carre00
    lcd.setCursor(16, 1);
    lcd.write(byte(0)); // carre00
    lcd.setCursor(17, 1);
    lcd.write(byte(0)); // carre00
    lcd.setCursor(18, 1);
    lcd.write(byte(0)); // carre00
      break;
    case 4:
    lcd.setCursor(13, 1);
    lcd.write(byte(4)); // carre04
    lcd.setCursor(14, 1);
    lcd.write(byte(0)); // carre00
    lcd.setCursor(15, 1);
    lcd.write(byte(0)); // carre00
    lcd.setCursor(16, 1);
    lcd.write(byte(0)); // carre00
    lcd.setCursor(17, 1);
    lcd.write(byte(0)); // carre00
    lcd.setCursor(18, 1);
    lcd.write(byte(0)); // carre00
      break;
    case 5:
    lcd.setCursor(13, 1);
    lcd.write(byte(5)); // carre05
    lcd.setCursor(14, 1);
    lcd.write(byte(0)); // carre00
    lcd.setCursor(15, 1);
    lcd.write(byte(0)); // carre00
    lcd.setCursor(16, 1);
    lcd.write(byte(0)); // carre00
    lcd.setCursor(17, 1);
    lcd.write(byte(0)); // carre00
    lcd.setCursor(18, 1);
    lcd.write(byte(0)); // carre00
      break;
    case 6:
    lcd.setCursor(13, 1);
    lcd.write(byte(5)); // carre05
    lcd.setCursor(14, 1);
    lcd.write(byte(1)); // carre01
    lcd.setCursor(15, 1);
    lcd.write(byte(0)); // carre00
    lcd.setCursor(16, 1);
    lcd.write(byte(0)); // carre00
    lcd.setCursor(17, 1);
    lcd.write(byte(0)); // carre00
    lcd.setCursor(18, 1);
    lcd.write(byte(0)); // carre00
      break;
    case 7:
    lcd.setCursor(13, 1);
    lcd.write(byte(5)); // carre05
    lcd.setCursor(14, 1);
    lcd.write(byte(2)); // carre02
    lcd.setCursor(15, 1);
    lcd.write(byte(0)); // carre00
    lcd.setCursor(16, 1);
    lcd.write(byte(0)); // carre00
    lcd.setCursor(17, 1);
    lcd.write(byte(0)); // carre00
    lcd.setCursor(18, 1);
    lcd.write(byte(0)); // carre00
      break;
    case 8:
    lcd.setCursor(13, 1);
    lcd.write(byte(5)); // carre05
    lcd.setCursor(14, 1);
    lcd.write(byte(3)); // carre03
    lcd.setCursor(15, 1);
    lcd.write(byte(0)); // carre00
    lcd.setCursor(16, 1);
    lcd.write(byte(0)); // carre00
    lcd.setCursor(17, 1);
    lcd.write(byte(0)); // carre00
    lcd.setCursor(18, 1);
    lcd.write(byte(0)); // carre00
      break;
    case 9:
    lcd.setCursor(13, 1);
    lcd.write(byte(5)); // carre05
    lcd.setCursor(14, 1);
    lcd.write(byte(4)); // carre04
    lcd.setCursor(15, 1);
    lcd.write(byte(0)); // carre00
    lcd.setCursor(16, 1);
    lcd.write(byte(0)); // carre00
    lcd.setCursor(17, 1);
    lcd.write(byte(0)); // carre00
    lcd.setCursor(18, 1);
    lcd.write(byte(0)); // carre00
      break;
    case 10:
    lcd.setCursor(13, 1);
    lcd.write(byte(5)); // carre05
    lcd.setCursor(14, 1);
    lcd.write(byte(5)); // carre05
    lcd.setCursor(15, 1);
    lcd.write(byte(0)); // carre00
    lcd.setCursor(16, 1);
    lcd.write(byte(0)); // carre00
    lcd.setCursor(17, 1);
    lcd.write(byte(0)); // carre00
    lcd.setCursor(18, 1);
    lcd.write(byte(0)); // carre00
      break;
    case 11:
    lcd.setCursor(13, 1);
    lcd.write(byte(5)); // carre05
    lcd.setCursor(14, 1);
    lcd.write(byte(5)); // carre05
    lcd.setCursor(15, 1);
    lcd.write(byte(1)); // carre01
    lcd.setCursor(16, 1);
    lcd.write(byte(0)); // carre00
    lcd.setCursor(17, 1);
    lcd.write(byte(0)); // carre00
    lcd.setCursor(18, 1);
    lcd.write(byte(0)); // carre00
      break;
    case 12:
    lcd.setCursor(13, 1);
    lcd.write(byte(5)); // carre05
    lcd.setCursor(14, 1);
    lcd.write(byte(5)); // carre05
    lcd.setCursor(15, 1);
    lcd.write(byte(2)); // carre02
    lcd.setCursor(16, 1);
    lcd.write(byte(0)); // carre00
    lcd.setCursor(17, 1);
    lcd.write(byte(0)); // carre00
    lcd.setCursor(18, 1);
    lcd.write(byte(0)); // carre00
      break;
    case 13:
    lcd.setCursor(13, 1);
    lcd.write(byte(5)); // carre05
    lcd.setCursor(14, 1);
    lcd.write(byte(5)); // carre05
    lcd.setCursor(15, 1);
    lcd.write(byte(3)); // carre03
    lcd.setCursor(16, 1);
    lcd.write(byte(0)); // carre00
    lcd.setCursor(17, 1);
    lcd.write(byte(0)); // carre00
    lcd.setCursor(18, 1);
    lcd.write(byte(0)); // carre00
      break;
    case 14:
    lcd.setCursor(13, 1);
    lcd.write(byte(5)); // carre05
    lcd.setCursor(14, 1);
    lcd.write(byte(5)); // carre05
    lcd.setCursor(15, 1);
    lcd.write(byte(4)); // carre04
    lcd.setCursor(16, 1);
    lcd.write(byte(0)); // carre00
    lcd.setCursor(17, 1);
    lcd.write(byte(0)); // carre00
    lcd.setCursor(18, 1);
    lcd.write(byte(0)); // carre00
      break;
    case 15:
    lcd.setCursor(13, 1);
    lcd.write(byte(5)); // carre05
    lcd.setCursor(14, 1);
    lcd.write(byte(5)); // carre05
    lcd.setCursor(15, 1);
    lcd.write(byte(5)); // carre05
    lcd.setCursor(16, 1);
    lcd.write(byte(0)); // carre00
    lcd.setCursor(17, 1);
    lcd.write(byte(0)); // carre00
    lcd.setCursor(18, 1);
    lcd.write(byte(0)); // carre00
      break;
    case 16:
    lcd.setCursor(13, 1);
    lcd.write(byte(5)); // carre05
    lcd.setCursor(14, 1);
    lcd.write(byte(5)); // carre05
    lcd.setCursor(15, 1);
    lcd.write(byte(5)); // carre05
    lcd.setCursor(16, 1);
    lcd.write(byte(1)); // carre01
    lcd.setCursor(17, 1);
    lcd.write(byte(0)); // carre00
    lcd.setCursor(18, 1);
    lcd.write(byte(0)); // carre00
      break;
     case 17:
    lcd.setCursor(13, 1);
    lcd.write(byte(5)); // carre05
    lcd.setCursor(14, 1);
    lcd.write(byte(5)); // carre05
    lcd.setCursor(15, 1);
    lcd.write(byte(5)); // carre05
    lcd.setCursor(16, 1);
    lcd.write(byte(2)); // carre02
    lcd.setCursor(17, 1);
    lcd.write(byte(0)); // carre00
    lcd.setCursor(18, 1);
    lcd.write(byte(0)); // carre00
      break;
     case 18:
    lcd.setCursor(13, 1);
    lcd.write(byte(5)); // carre05
    lcd.setCursor(14, 1);
    lcd.write(byte(5)); // carre05
    lcd.setCursor(15, 1);
    lcd.write(byte(5)); // carre05
    lcd.setCursor(16, 1);
    lcd.write(byte(3)); // carre03
    lcd.setCursor(17, 1);
    lcd.write(byte(0)); // carre00
    lcd.setCursor(18, 1);
    lcd.write(byte(0)); // carre00
      break;
     case 19:
    lcd.setCursor(13, 1);
    lcd.write(byte(5)); // carre05
    lcd.setCursor(14, 1);
    lcd.write(byte(5)); // carre05
    lcd.setCursor(15, 1);
    lcd.write(byte(5)); // carre05
    lcd.setCursor(16, 1);
    lcd.write(byte(4)); // carre04
    lcd.setCursor(17, 1);
    lcd.write(byte(0)); // carre00
    lcd.setCursor(18, 1);
    lcd.write(byte(0)); // carre00
      break;
     case 20:
    lcd.setCursor(13, 1);
    lcd.write(byte(5)); // carre05
    lcd.setCursor(14, 1);
    lcd.write(byte(5)); // carre05
    lcd.setCursor(15, 1);
    lcd.write(byte(5)); // carre05
    lcd.setCursor(16, 1);
    lcd.write(byte(5)); // carre05
    lcd.setCursor(17, 1);
    lcd.write(byte(0)); // carre00
    lcd.setCursor(18, 1);
    lcd.write(byte(0)); // carre00
      break;
     case 21:
    lcd.setCursor(13, 1);
    lcd.write(byte(5)); // carre05
    lcd.setCursor(14, 1);
    lcd.write(byte(5)); // carre05
    lcd.setCursor(15, 1);
    lcd.write(byte(5)); // carre05
    lcd.setCursor(16, 1);
    lcd.write(byte(5)); // carre05
    lcd.setCursor(17, 1);
    lcd.write(byte(1)); // carre01
    lcd.setCursor(18, 1);
    lcd.write(byte(0)); // carre00
      break;
     case 22:
    lcd.setCursor(13, 1);
    lcd.write(byte(5)); // carre05
    lcd.setCursor(14, 1);
    lcd.write(byte(5)); // carre05
    lcd.setCursor(15, 1);
    lcd.write(byte(5)); // carre05
    lcd.setCursor(16, 1);
    lcd.write(byte(5)); // carre05
    lcd.setCursor(17, 1);
    lcd.write(byte(2)); // carre02
    lcd.setCursor(18, 1);
    lcd.write(byte(0)); // carre00
      break;
     case 23:
    lcd.setCursor(13, 1);
    lcd.write(byte(5)); // carre05
    lcd.setCursor(14, 1);
    lcd.write(byte(5)); // carre05
    lcd.setCursor(15, 1);
    lcd.write(byte(5)); // carre05
    lcd.setCursor(16, 1);
    lcd.write(byte(5)); // carre05
    lcd.setCursor(17, 1);
    lcd.write(byte(3)); // carre03
    lcd.setCursor(18, 1);
    lcd.write(byte(0)); // carre00
      break;
     case 24:
    lcd.setCursor(13, 1);
    lcd.write(byte(5)); // carre05
    lcd.setCursor(14, 1);
    lcd.write(byte(5)); // carre05
    lcd.setCursor(15, 1);
    lcd.write(byte(5)); // carre05
    lcd.setCursor(16, 1);
    lcd.write(byte(5)); // carre05
    lcd.setCursor(17, 1);
    lcd.write(byte(4)); // carre04
    lcd.setCursor(18, 1);
    lcd.write(byte(0)); // carre00
      break;
     case 25:
    lcd.setCursor(13, 1);
    lcd.write(byte(5)); // carre05
    lcd.setCursor(14, 1);
    lcd.write(byte(5)); // carre05
    lcd.setCursor(15, 1);
    lcd.write(byte(5)); // carre05
    lcd.setCursor(16, 1);
    lcd.write(byte(5)); // carre05
    lcd.setCursor(17, 1);
    lcd.write(byte(5)); // carre05
    lcd.setCursor(18, 1);
    lcd.write(byte(0)); // carre00
      break;
     case 26:
    lcd.setCursor(13, 1);
    lcd.write(byte(5)); // carre05
    lcd.setCursor(14, 1);
    lcd.write(byte(5)); // carre05
    lcd.setCursor(15, 1);
    lcd.write(byte(5)); // carre05
    lcd.setCursor(16, 1);
    lcd.write(byte(5)); // carre05
    lcd.setCursor(17, 1);
    lcd.write(byte(5)); // carre05
    lcd.setCursor(18, 1);
    lcd.write(byte(1)); // carre01
      break;
     case 27:
    lcd.setCursor(13, 1);
    lcd.write(byte(5)); // carre05
    lcd.setCursor(14, 1);
    lcd.write(byte(5)); // carre05
    lcd.setCursor(15, 1);
    lcd.write(byte(5)); // carre05
    lcd.setCursor(16, 1);
    lcd.write(byte(5)); // carre05
    lcd.setCursor(17, 1);
    lcd.write(byte(5)); // carre05
    lcd.setCursor(18, 1);
    lcd.write(byte(2)); // carre02
      break;
     case 28:
    lcd.setCursor(13, 1);
    lcd.write(byte(5)); // carre05
    lcd.setCursor(14, 1);
    lcd.write(byte(5)); // carre05
    lcd.setCursor(15, 1);
    lcd.write(byte(5)); // carre05
    lcd.setCursor(16, 1);
    lcd.write(byte(5)); // carre05
    lcd.setCursor(17, 1);
    lcd.write(byte(5)); // carre05
    lcd.setCursor(18, 1);
    lcd.write(byte(3)); // carre03
      break;
      case 29:
    lcd.setCursor(13, 1);
    lcd.write(byte(5)); // carre05
    lcd.setCursor(14, 1);
    lcd.write(byte(5)); // carre05
    lcd.setCursor(15, 1);
    lcd.write(byte(5)); // carre05
    lcd.setCursor(16, 1);
    lcd.write(byte(5)); // carre05
    lcd.setCursor(17, 1);
    lcd.write(byte(5)); // carre05
    lcd.setCursor(18, 1);
    lcd.write(byte(4)); // carre04
      break;    
      case 30:
    lcd.setCursor(13, 1);
    lcd.write(byte(5)); // carre05
    lcd.setCursor(14, 1);
    lcd.write(byte(5)); // carre05
    lcd.setCursor(15, 1);
    lcd.write(byte(5)); // carre05
    lcd.setCursor(16, 1);
    lcd.write(byte(5)); // carre05
    lcd.setCursor(17, 1);
    lcd.write(byte(5)); // carre05
    lcd.setCursor(18, 1);
    lcd.write(byte(5)); // carre05
      break;       
    default:
      // statements
      break;
  }
}
