#pragma once
#include <Arduino.h>
#include <SPI.h>
#include <SDFS.h>

#include <MUFFINS_Component_Base.h>

#define DEFAULT_INFO_FILE_HEADER "time_on,info_message"
#define DEFAULT_ERROR_FILE_HEADER "time_on,error_message"

class SD_Card : public Component_Base
{
public:
  struct Config
  {
    SPIClass *spi_bus;
    int cs_pin;
    String telemetry_file_path_base;
    String info_file_path_base;
    String error_file_path_base;
    String telemetry_file_header;
    String info_file_header;
    String error_file_header;
  };

private:
  // Configuration
  Config _sd_card_config;

  // SD Card
  FS *_sd;

  /**
   * @brief File paths for telemetry, info, and error data
   */
  struct FilePaths
  {
    String telemetry;
    String info;
    String error;
  } _file_paths;

  /**
   * @brief Initialize the file system
   */
  bool _begin_files();

  /**
   * @brief Write to a file
   * @param file_path The path to the file
   * @param data The data to write
   */
  bool _write_to_file(const String &file_path, const String &data);

public:
  /**
   * @brief Construct a new SD Card Wrapper object
   */
  SD_Card(String component_name = "SD Card", void (*info_function)(String) = nullptr, void (*error_function)(String) = nullptr);

  /**
   * @brief Initialize the SD card
   * @param config The SD_Card_Wrapper configuration
   */
  bool begin(const Config &config);

  /**
   * @brief Get data about SD card storage
   */
  bool card_info();

  /**
   * @brief Format the SD card
   */
  bool format();

  /**
   * @brief Write data to the telemetry file
   * @param msg The data to write
   */
  bool write_telemetry(const String &msg);

  /**
   * @brief Write info to the info message file
   * @param msg The info message
   */
  bool write_info(const String &msg);

  /**
   * @brief Write error to the error message file
   * @param msg The error message
   */
  bool write_error(const String &msg);

  /**
   * @brief Read the last line from a file
   * @param msg A pointer to a string where to store the read line
   * @param file_path The path to the file
   * @note Ignores the header
   */
  bool read_last_line_from_file(String &msg, const String &file_path);
};
