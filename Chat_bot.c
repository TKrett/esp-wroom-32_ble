#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>


// inicializace modulu z knihovny
BLECharacteristic *pCharacteristic;
// proměnná pro kontrolu připojených zařízení
bool dev_connected = false;
// proměnná pro ukládání přijaté zprávy
std::string prijataZprava;
std::string inp_string = "0";
String odesl_Zprava = "0";
String odeslZprava = "0" ;

//UUID
#define SERVICE_UUID           "6E400001-B5A3-F393-E0A9-E50E24DCCA9E"
#define CHARACTERISTIC_UUID_RX "6E400002-B5A3-F393-E0A9-E50E24DCCA9E"
#define CHARACTERISTIC_UUID_TX "6E400003-B5A3-F393-E0A9-E50E24DCCA9E"

// třída pro kontrolu připojení
class MyServerCallbacks: public BLEServerCallbacks
{
    // při spojení zařízení nastav proměnnou na log1
    void onConnect(BLEServer* pServer)
    {
      dev_connected = true;
    };
    // při odpojení zařízení nastav proměnnou na log0
    void onDisconnect(BLEServer* pServer) {
      dev_connected = false;
    }
};

// třída pro příjem zprávy
class MyCallbacks: public BLECharacteristicCallbacks {
    // při příjmu zprávy proveď následující
    void onWrite(BLECharacteristic *pCharacteristic) {
      // načti přijatou zprávu do proměnné
      prijataZprava = pCharacteristic->getValue();
      inp_string = prijataZprava;
      // pokud není zpráva prázdná, vypiš její obsah
      // po znacích po sériové lince
      if (prijataZprava.length() > 0) {
        Serial.print(">CLIENT: ");
        for (int i = 0; i < prijataZprava.length(); i++) {
          Serial.print(prijataZprava[i]);
        }
      Serial.print("\n");
      }
    }
};

void BLEsend(String zprava);

void setup() {
  Serial.begin(115200);
  Serial.setTimeout(1000);

  BLEDevice::init("ESP32 Chat_Bot");
  // vytvoření BLE serveru
  BLEServer *pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());
  // vytvoření BLE služby
  BLEService *pService = pServer->createService(SERVICE_UUID);
  // vytvoření BLE komunikačního kanálu pro odesílání (TX)
  pCharacteristic = pService->createCharacteristic(
                      CHARACTERISTIC_UUID_TX,
                      BLECharacteristic::PROPERTY_NOTIFY
                    );
  pCharacteristic->addDescriptor(new BLE2902());
  // vytvoření BLE komunikačního kanálu pro příjem (RX)
  BLECharacteristic *pCharacteristic = pService->createCharacteristic(
                                         CHARACTERISTIC_UUID_RX,
                                         BLECharacteristic::PROPERTY_WRITE
                                       );
  pCharacteristic->setCallbacks(new MyCallbacks());
  // zahájení BLE služby
  pService->start();
  // zapnutí viditelnosti BLE
  pServer->getAdvertising()->start();
  Serial.println("BLE nastaveno, ceka na pripojeni..");
  while (dev_connected == false)
  {
      delay(500);
  }
  Serial.println(">HOST: Pripojeno k zarizeni");
  String zprava = ">HOST: Pripojeno k zarizeni";
  BLEsend(zprava);
}
void loop() {
  
  if (dev_connected == true) {
    while(Serial.available())
    {
    odeslZprava= Serial.readString();
     }
     if (odeslZprava!="0")
     {
       //strcpy(odesl_Zprava,odeslZprava);
    if (odeslZprava.startsWith(">HOST")==false && odeslZprava.startsWith(">CLIENT")==false)
    {
      String zprava = ">HOST: ";
      zprava += odeslZprava;
      zprava += "\n";
      BLEsend(zprava);
      Serial.print(zprava);
      Serial.println(" ");
    }
     }
  inp_string = "0";
  odeslZprava = "0";
  }
  Serial.flush();

  delay(200);
}

void BLEsend(String zprava)
{
  char zpravaChar[zprava.length() + 1];
      zprava.toCharArray(zpravaChar, zprava.length() + 1);
      // přepsání zprávy do BLE služby
      pCharacteristic->setValue(zpravaChar);
      // odeslání zprávy skrze BLE do připojeného zařízení
      pCharacteristic->notify();
}
