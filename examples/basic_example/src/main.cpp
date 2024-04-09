#include <Arduino.h>
#include <SPI.h>
#include <SDFS.h>

#include <MUFFINS_Component_Base.h>
#include <MUFFINS_SD_Card.h>

const int SENSOR_POWER_ENABLE_PIN = 17;
const int SPI0_RX = 4;
const int SPI0_TX = 3;
const int SPI0_SCK = 2;

SD_Card sd_card;

SD_Card::Config sd_card_config = {
  .spi_bus = &SPI,
  .cs_pin = 14,
  .telemetry_file_path_base = "/BFC_TELEMETRY_",
  .info_file_path_base = "/BFC_INFO_",
  .error_file_path_base = "/BFC_ERROR_",
  .telemetry_file_header = "index,one,two,three",
  .info_file_header = "index,four,five,six",
  .error_file_header = "index,seven,eight,nine"
};

void setup()
{
  Serial.begin(115200);
  while (!Serial)
  {
    delay(1000);
  }

  pinMode(SENSOR_POWER_ENABLE_PIN, OUTPUT_12MA);
  digitalWrite(SENSOR_POWER_ENABLE_PIN, HIGH);

  if (SPI.setRX(SPI0_RX) && SPI.setTX(SPI0_TX) && SPI.setSCK(SPI0_SCK))
  {
    SPI.begin();
  }

  if (!sd_card.begin(sd_card_config))
  {
    while (1)
      ;
  }

  sd_card.card_info();
  sd_card.write_telemetry("1,2,3");
  sd_card.write_info("4,5,6");
  sd_card.write_error("7,8,9");
  Serial.println("Done");
};

void loop()
{
  // Empty
}