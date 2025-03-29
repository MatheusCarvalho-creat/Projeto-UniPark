#include <SPI.h>
#include <MFRC522.h>

#define SS_PIN 5
#define RST_PIN 2

MFRC522 rfid(SS_PIN, RST_PIN);
MFRC522::MIFARE_Key key;

// Definição dos blocos para armazenar os dados
int blockName = 4;
int blockSector = 5;
int blockMatricula = 6;

// Dados para gravar
byte nameData[] = { 'P', 'a', 't', 'r', 'i', 'c', 'i', 'a', ' ', 'A', 'p', 'a', 'r', 'e', 'c', 'i', 'd', 'a' }; // Deixa o compilador contar
byte sectorData[] = { 'P', 'a', 'r', 'a', 's', 'i', 't', 'o', 'l', 'o', 'g', 'i', 'a', ' ', ' ', ' ' };
byte matriculaData[] = { '2', '0', '2', '4', '0', '0', '7', '4', '3', '2', ' ', ' ', ' ', ' ', ' ', ' ' };

// Buffers para leitura
byte readName[18], readSector[18], readMatricula[18];
byte bufferLen = 18;

void setup() {
  Serial.begin(9600);
  SPI.begin();
  rfid.PCD_Init();

  // Configura a chave padrão
  for (byte i = 0; i < 6; i++) {
    key.keyByte[i] = 0xFF;
  }

  Serial.println(F("Aproxime um cartão para gravar e verificar dados."));
}

void loop() {
  if (!rfid.PICC_IsNewCardPresent()) return;
  if (!rfid.PICC_ReadCardSerial()) return;

  Serial.println(F("Cartão detectado."));
  Serial.print(F("UID: "));
  printHex(rfid.uid.uidByte, rfid.uid.size);
  Serial.println();

  // Grava os dados nos blocos
  if (writeToBlock(blockName, nameData) &&
      writeToBlock(blockSector, sectorData) &&
      writeToBlock(blockMatricula, matriculaData)) {
    Serial.println(F("Dados gravados com sucesso!"));

    // Lê e verifica os dados gravados
    if (readFromBlock(blockName, readName) &&
        readFromBlock(blockSector, readSector) &&
        readFromBlock(blockMatricula, readMatricula)) {
      Serial.println(F("Dados lidos com sucesso!"));

      Serial.print(F("Nome: "));
      printASCII(readName);

      Serial.print(F("Setor: "));
      printASCII(readSector);

      Serial.print(F("Matrícula: "));
      printASCII(readMatricula);
    }
  } else {
    Serial.println(F("Falha ao gravar os dados."));
  }

  // Finaliza a comunicação com o cartão
  rfid.PICC_HaltA();
  rfid.PCD_StopCrypto1();
}

// Função para escrever em um bloco
bool writeToBlock(int blockNum, byte *data) {
  MFRC522::StatusCode status = rfid.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, blockNum, &key, &(rfid.uid));
  if (status != MFRC522::STATUS_OK) {
    Serial.print(F("Falha na autenticação: "));
    Serial.println(rfid.GetStatusCodeName(status));
    return false;
  }

  status = rfid.MIFARE_Write(blockNum, data, 16);
  if (status != MFRC522::STATUS_OK) {
    Serial.print(F("Falha ao gravar: "));
    Serial.println(rfid.GetStatusCodeName(status));
    return false;
  }

  return true;
}

// Função para ler um bloco
bool readFromBlock(int blockNum, byte *buffer) {
  MFRC522::StatusCode status = rfid.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, blockNum, &key, &(rfid.uid));
  if (status != MFRC522::STATUS_OK) {
    Serial.print(F("Falha na autenticação: "));
    Serial.println(rfid.GetStatusCodeName(status));
    return false;
  }

  status = rfid.MIFARE_Read(blockNum, buffer, &bufferLen);
  if (status != MFRC522::STATUS_OK) {
    Serial.print(F("Falha ao ler: "));
    Serial.println(rfid.GetStatusCodeName(status));
    return false;
  }

  return true;
}

// Função para imprimir bytes em ASCII
void printASCII(byte *buffer) {
  for (int i = 0; i < 16; i++) {
    Serial.write(buffer[i]);
  }
  Serial.println();
}

// Função para imprimir bytes em hexadecimal
void printHex(byte *buffer, byte bufferSize) {
  for (byte i = 0; i < bufferSize; i++) {
    Serial.print(buffer[i] < 0x10 ? " 0" : " ");
    Serial.print(buffer[i], HEX);
  }
  Serial.println();
}

