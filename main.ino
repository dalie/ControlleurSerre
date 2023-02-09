

#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <DallasTemperature.h>
#include <DS3231.h>
#include <SD.h>
#include <SPI.h>

#define ONE_WIRE_BUS 9

DS3231  rtc(SDA, SCL);
OneWire oneWire(ONE_WIRE_BUS);

DallasTemperature sensors(&oneWire);

File dataSerre;//Non de la variable fichier texte

Time t;//variable qui prend le temps
int mm = 0;//variable qui déclare le nombre de minutes
int hh = 0;//variable qui déclare le nombre d'heures

float T1 = 0;//Température ambiante 1
float T2 = 0;//Température ambiante 2
float T3 = 0;//Température ambiante 3 
float Geo1 = 0;//Température système de géothermie 1 
float Geo2 = 0;// Température système de géothermie 2
float TMoy = 0;// Moyenne des températures ambiantes

int pinSD = 10;//pin de la carte SD

int fanGeo1 = 1;//Socket première fan géothermie
int fanGeo2 = 2;//Socket deuxième fan géothermie
int entAir = 7;//Socket entrée air
int jetFan = 6;//Socket jet fan
int fil = 3;//Socket fil chauffant
int air = 4;//Socket circulateur air

int delayTime = 5000;
int delayDebut = 10000;
int delayData = 15000;
bool isWriting = false;


int condEntAir = 26;
int condJetFan = 30;
int condGeo = 22;
int condMin = 5;
int condFil = 3;
int condAir = 50;
int airDebut = 5;//heure de début du circulateur d'air
int airFin = 22; //heure de fin du circulateur d'air
int airMin = 50; //nombre de minutes à l'heure du circulateur d'air

//Déclaration de variables boolean permettant de voir si les systèmes sont actifs
bool sys1;//entrée air fonctionnelle ?
bool sys2;//jet fan fonctionnelle ?
bool sys3;//géothermie 1
bool sys4;//géothermie 2
bool sys5;//fil chauffant
bool sys6;//circulateur air

LiquidCrystal_I2C lcd(0x27, 16, 3);

void setup() {
  // put your setup code here, to run once:
  lcd.begin();
  lcd.backlight();
  sensors.begin();
  rtc.begin();
  
  pinMode(fanGeo1,OUTPUT);
  pinMode(fanGeo2,OUTPUT);
  pinMode(entAir,OUTPUT);
  pinMode(jetFan,OUTPUT);
  pinMode(fil,OUTPUT);
  pinMode(air,OUTPUT);
  
  pinMode(pinSD,OUTPUT);
  
  lcd.setCursor(0,0);
  if (SD.begin())
  {
    lcd.print("La carte SD est");
    lcd.setCursor(0,1);
    lcd.print("fonctionnelle");
    delay(delayDebut);
  } 
  else
  {
    lcd.print("Carte SD non initialisée");
    delay(delayDebut);
    return;
  }
  
    
}

void loop() {
  // put your main code here, to run repeatedly:
  sensors.requestTemperatures();
  T1 = sensors.getTempCByIndex(0);
  T2 = sensors.getTempCByIndex(1);
  T3 = sensors.getTempCByIndex(2);
  Geo1 = sensors.getTempCByIndex(3);
  Geo2 = sensors.getTempCByIndex(4);
  // Code servant à aller chercher les températures ambiantes et géothermiques. Donc au début du code, on prend les données de température
  // Très important de placer dans l'ordre les trois températures ambiantes et ensuite les deux géothermie
  TMoy = (T1+T2+T3)/3;// Sert à calculer la moyenne des températures ambiantes. Les sensors pouvant être plus ou moins à l'ombre
  //en hauteur ou au soleil pourraient fausser les résultats si pris individuellement

  lcd.clear();//On vide le lcd
  lcd.setCursor(0,0);
  lcd.print("T1 = ");
  lcd.print(T1);
  lcd.write(223);
  lcd.print("C");
  lcd.setCursor(0,1);
  lcd.print("T2 = ");
  lcd.print(T2);
  lcd.write(223);
  lcd.print("C");
  lcd.setCursor(0,2);
  lcd.print("T3 = ");
  lcd.print(T3);
  lcd.write(223);
  lcd.print("C");
  lcd.setCursor(12,3);
  lcd.print(rtc.getTimeStr());
  delay(delayTime);

  lcd.clear();//On vide le lcd
  lcd.setCursor(0,1);
  lcd.print("Temp Moy = ");
  lcd.print(TMoy);
  lcd.write(223);
  lcd.print("C");
  lcd.setCursor(12,3);
  lcd.print(rtc.getTimeStr());
  delay(delayTime);

  lcd.clear();//On vide le lcd
  lcd.setCursor(0,0);
  lcd.print("TGeo1 = ");
  lcd.print(Geo1);
  lcd.write(223);
  lcd.print("C");
  lcd.setCursor(0,1);
  lcd.print("TGeo2 = ");
  lcd.print(Geo2);
  lcd.write(223);
  lcd.print("C");
  lcd.setCursor(12,3);
  lcd.print(rtc.getTimeStr());  
  delay(delayTime);

  //Écriture du code servant à activer l'entrée d'air selon la température désirée (condEntAir)
  if (TMoy>condEntAir){
    digitalWrite(entAir,HIGH);
    sys1 = true;
  }
  else{
    digitalWrite(entAir,LOW);
    sys1 = false;
  }

  //Écriture du code servant à activer la jet fan selon la température désirée (condJetFan)
  if (TMoy>condJetFan){
    digitalWrite(jetFan,HIGH);
    sys2 = true;
  }
  else{
    digitalWrite(jetFan,LOW);
    sys2 = false;
  }

  //Écriture du code servant à activer les systèmes de géothermie selon la température désirée (condGeo) 
  //et selon le différentiel des températures ambiantes et des températures du sol
  if ((TMoy>condGeo) or ((Geo1>TMoy)and(TMoy<condMin))){
    digitalWrite(fanGeo1,HIGH);
    sys3 = true;
  }
  else {
    digitalWrite(fanGeo1,LOW);
    sys3 = false;
  }

  if ((TMoy>condGeo) or ((Geo2>TMoy)and(TMoy<condMin))){
    digitalWrite(fanGeo2,HIGH);
    sys4 = true;
  }
  else {
    digitalWrite(fanGeo2,LOW);
    sys4 = false;
  }

  //Écriture du code servant à activer le fil chauffant selon la température désirée (condFil)
  if (TMoy<condFil){
    digitalWrite(fil,HIGH);
    sys5 = true;
  }
  
  else{
    digitalWrite(fil,LOW);
    sys5 = false;
  }  

  //partie du code servant à activer la fan de circulation d'air, soit à toutes les heures pour les 50 premières minutes et pas entre 22 heures et 6 heures le matin
  t = rtc.getTime();
  mm = t.min;
  hh = t.hour;
  
  if ((mm<airMin) and ((hh>airDebut)and(hh<airFin))){
    digitalWrite(air,HIGH);
    sys6 = true;
  }
  
  else{
    digitalWrite(air,LOW);
    sys6 = false;
  }

  //partie du code servant à vérifier si les systèmes électriques fonctionnent. Le but de cette partie est de se dire que
  //si le code fonctionne, mais que l'élément électrique n'est pas allumé, cela voudrait dire que c'est le branchement le problème
  //Donc sert à anticiper des problèmes de connexion et isoler le problème.
  
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Ent air = ");

  if (sys1){
    lcd.print("ON");
  }
  else {
    lcd.print("OFF");
  }
  
  lcd.setCursor(0,1);
  lcd.print("Jet fan = ");
  if (sys2){
    lcd.print("ON");
  }
  else{
    lcd.print("OFF");
  }
  lcd.setCursor(0,2);
  lcd.print("Fil chauffant = ");
  
  if (sys5){
    lcd.print("ON");
  }
  else{
    lcd.print("OFF");
  }
  lcd.setCursor(12,3);
  lcd.print(rtc.getTimeStr());
  delay(delayTime);

  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Geothermie 1 = ");
  
  if (sys3){
    lcd.print("ON");
  }
  else{
    lcd.print("OFF");
  }
  lcd.setCursor(0,1);
  lcd.print("Geothermie 2 = ");
  
  if (sys4){
    lcd.print("ON");
  }
  else{
    lcd.print("OFF");
  }
  lcd.setCursor(0,2);
  lcd.print("circulateur = ");
  
  if (sys6){
    lcd.print("ON");
  }
  else{
    lcd.print("OFF");
  }  
  
  lcd.setCursor(12,3);
  lcd.print(rtc.getTimeStr());
  delay(delayTime);

  //Partie du code servant à transmettre les données de la serre sur une carte microSD
  //Transmet sur la carte une fois au 15 minutes
  if (mm == 0 or mm == 15 or mm == 30 or mm == 45) {
    if(!isWriting){
      dataSerre = SD.open("data.txt", FILE_WRITE);
      if(dataSerre){
        dataSerre.print(rtc.getDateStr());
        dataSerre.print(",");  
        dataSerre.print(rtc.getTimeStr());
        dataSerre.print(",");    
        dataSerre.print(TMoy);
        dataSerre.print(",");
        dataSerre.print(Geo1);
        dataSerre.print(",");
        dataSerre.print(Geo2);  
        dataSerre.close();// close the file
        lcd.clear();
        lcd.setCursor(0,0);
        lcd.print("Transcription data");
        lcd.setCursor(0,1);
        lcd.print("en cours");
        isWriting = true;
        delay(delayData);
      }
    }
  } else if(isWriting){
    isWriting = false;
  }
}
