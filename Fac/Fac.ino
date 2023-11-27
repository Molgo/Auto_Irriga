#include <ArduinoJson.h>
#include <HTTPClient.h>
#include <WiFi.h>
#include <EEPROM.h>

#define EEPROM_SIZE 100

const char ssid[] = "******";
const char password[] = "*******";

String URL1 = "http://api.openweathermap.org/data/2.5/weather?";
String ApiKey = "1533749c88ee44f23d5b7aa178fad20a";

String lat = "-22.748369";
String lon = "-42.868664";

String URL2 = "https://worldtimeapi.org/api/timezone/America/Sao_Paulo";

unsigned long int time_skip = 0;

String datetime;
String main_humidity;
String main_temp;

void save_eeprom(const String &str, int addr) {
  for (int i = 0; i < str.length(); i++) {
    EEPROM.write(addr + i, str[i]);
  }
  EEPROM.write(addr + str.length(), '\0');
}

String read_eeprom(int addr) {
  String readString = "";
  char c;
  while ((c = EEPROM.read(addr)) != '\0') {
    readString += c;
    addr++;
  }
  return readString;
}

void setup() {
  Serial.begin(115200);
  pinMode(2, OUTPUT);
  pinMode(4, INPUT);

  EEPROM.begin(EEPROM_SIZE);

  delay(2000);

  wifi_connect();
  load_json1();
  load_json2();
}

void rele() {
  if (WiFi.status() != WL_CONNECTED) {
    wifi_connect();
    load_json1();
    load_json2();
  }

  digitalWrite(2, HIGH);
  Serial.println("Irrigando");
  delay(5000);
  digitalWrite(2, LOW);
  Serial.println("Pronto");

  save_eeprom(datetime, 0);
    save_eeprom(main_temp, 30);
  save_eeprom(main_humidity, 60);
  
  Serial.println(read_eeprom(0));
  Serial.println(read_eeprom(30));
  Serial.println(read_eeprom(60));
}

void wifi_connect() {
  int attempts = 0;
  
  Serial.print("Se conectando à ");
  Serial.println(ssid);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  
  while (WiFi.status() != WL_CONNECTED && attempts <= 50) {
    delay(500);
    Serial.print(".");
    attempts++;
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("");
    Serial.println("Wifi conectado");
    Serial.print("Endereço IP:");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("");
    Serial.println("Falha na conexão à rede WiFi.");
    Serial.println("Verifique as configurações");
  }
}

void load_json1() {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    http.begin(URL1 + "lat=" + lat + "&lon=" + lon + "&units=metric&appid=" + ApiKey);

    int httpCode = http.GET();

    if (httpCode > 0) {
      String JSON_Data1 = http.getString();

      StaticJsonDocument<1024> doc;
      DeserializationError error = deserializeJson(doc, JSON_Data1);

      if (error) {
        Serial.print("Falha na desserialização do arquivo Json de clima(): ");
        Serial.println(error.c_str());
        return;
      }

      float coord_lon = doc["coord"]["lon"];
      float coord_lat = doc["coord"]["lat"];

      const char* weather_0_description = doc["weather"][0]["description"];
      const char* base = doc["base"];

      JsonObject main = doc["main"];
      float main_temp_json = main["temp"];
      float main_feels_like = main["feels_like"];
      float main_temp_min = main["temp_min"];
      int main_temp_max = main["temp_max"];
      int main_humidity_json = main["humidity"];

      main_humidity = "Última humidade: " + String(main_humidity_json);
      main_temp = "Última temperatura: " + String(main_temp_json);
      
      const char* loc = doc["name"];

      Serial.print("Lat: ");
      Serial.println(coord_lat);
      Serial.print("Lon: ");
      Serial.println(coord_lon);

      Serial.print("Descrição: ");
      if (strcmp(weather_0_description, "overcast clouds") == 0) {
      Serial.println("Nublado");
      } else if (strcmp(weather_0_description, "clear sky") == 0) {
      Serial.println("Céu limpo");
      } else if (strcmp(weather_0_description, "light rain") == 0) {
      Serial.println("Chuva leve");
      } else {
      Serial.println(weather_0_description);
      }

      Serial.print("Temperatura: ");
      Serial.println(main_temp_json);
      Serial.print("Temperatura máxima: ");
      Serial.println(main_temp_max);
      Serial.print("Temperatura mínima: ");
      Serial.println(main_temp_min);
      Serial.print("Sensação térmica: ");
      Serial.println(main_feels_like);
      Serial.print("Humidade: ");
      Serial.println(main_humidity_json);

      Serial.print("Local: ");
      Serial.println(loc);

      Serial.print("Base: ");
      Serial.println(base);
    }

    http.end();
  }
}

void load_json2() {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    http.begin(URL2);

    int httpCode = http.GET();

    if (httpCode > 0) {
      String JSON_Data2 = http.getString();
      StaticJsonDocument<768> doc;

      DeserializationError error = deserializeJson(doc, JSON_Data2);

      if (error) {
        Serial.print("Falha na desserialização do arquivo Json de Horário(): ");
        Serial.println(error.c_str());
        return;
      }

      String datetime_json = doc["datetime"];  // "2023-09-15T08:08:36.772304-03:00"
      int day_of_week = doc["day_of_week"];

      datetime = ("Último datetime" + datetime_json.substring(0,19));

      Serial.print("Dia da semana: ");
      switch (day_of_week) {
        case 1: Serial.println("Segunda-feira"); break;
        case 2: Serial.println("Terça-feira"); break;
        case 3: Serial.println("Quarta-feira"); break;
        case 4: Serial.println("Quinta-feira"); break;
        case 5: Serial.println("Sexta-feira"); break;
        case 6: Serial.println("Sábado"); break;
        case 7: Serial.println("Domingo"); break;
      }

      Serial.print("Data: ");
      Serial.print(datetime.substring(8, 10));
      Serial.print("/");
      Serial.print(datetime.substring(5, 7));
      Serial.print("/");
      Serial.println(datetime.substring(0, 4));

      Serial.print("Horas: ");
      Serial.println(datetime.substring(11, 19));
    }

    http.end();
  }
}

void loop() {
  if(analogRead(4) >= 3000){
    rele();
  }
  if(millis() - time_skip >= 1800000){
    time_skip = millis();
    wifi_connect();
    load_json1();
    load_json2();
  } 
}
