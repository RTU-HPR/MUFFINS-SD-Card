#include <MUFFINS_SD_Card.h>

SD_Card::SD_Card(String component_name, void (*info_function)(String), void (*error_function)(String)) : Component_Base(component_name, info_function, error_function)
{
  return;
}

bool SD_Card::begin(const Config &config)
{
  // Save the SD card configuration locally
  _sd_card_config = config;

  // Initialize the SD card
  SDFSConfig sd_config;
  sd_config.setCSPin(_sd_card_config.cs_pin);
  sd_config.setSPI(*_sd_card_config.spi_bus);

  if (!_sd->setConfig(sd_config))
  {
    error("Setting SPI configuration failed!");
    return false;
  }

  if (!_sd->begin())
  {
    error("Failed to initialize filesystem!");
    return false;
  }

  // Create files and write headers
  if (!_begin_files())
  {
    error("Failed to initialize files!");
    return false;
  }

  info("Initialized!");
  set_initialized(true);

  return true;
}

bool SD_Card::card_info()
{
  if (!initialized())
  {
    error("Not initialized!");
    return false;
  }

  // Get info about the flash
  FSInfo64 fs_info64;
  _sd->info64(fs_info64);
  // 64 bit integers can only be converted to strings using sprintf
  char total_bytes_string_buffer[21];
  char used_bytes_string_buffer[21];
  char free_bytes_string_buffer[21];
  sprintf(total_bytes_string_buffer, "%", PRIu64, fs_info64.totalBytes);
  sprintf(used_bytes_string_buffer, "%", PRIu64, fs_info64.usedBytes);
  sprintf(free_bytes_string_buffer, "%", PRIu64, fs_info64.totalBytes - fs_info64.usedBytes);
  info("Total space: " + String(total_bytes_string_buffer) + " bytes");
  info("Used space: " + String(used_bytes_string_buffer) + " bytes");
  info("Free space: " + String(free_bytes_string_buffer) + " bytes");

  return true;
}

bool SD_Card::format()
{
  if (!initialized())
  {
    error("Not initialized!");
    return false;
  }

  if (!_sd->format())
  {
    error("Failed to format SD card!");
    return false;
  }

  info("Formatted SD card!");
  info("SD card is not initialized anymore!");

  set_initialized(false);

  return true;
}

bool SD_Card::write_telemetry(const String &msg)
{
  if (!initialized())
  {
    error("Not initialized!");
    return false;
  }

  return _write_to_file(_file_paths.telemetry, msg);
}

bool SD_Card::write_info(const String &msg)
{
  if (!initialized())
  {
    error("Not initialized!");
    return false;
  }

  return _write_to_file(_file_paths.info, msg);
}

bool SD_Card::write_error(const String &msg)
{
  if (!initialized())
  {
    error("Not initialized!");
    return false;
  }

  return _write_to_file(_file_paths.error, msg);
}

bool SD_Card::_begin_files()
{
  // Determine file name index for final path
  int file_name_nr = 0;
  while (_sd->exists(_sd_card_config.telemetry_file_path_base + String(file_name_nr) + ".csv"))
  {
    file_name_nr++;
  }

  // Get the file full path
  _file_paths.telemetry = _sd_card_config.telemetry_file_path_base + String(file_name_nr) + ".csv";
  _file_paths.info = _sd_card_config.info_file_path_base + String(file_name_nr) + ".csv";
  _file_paths.error = _sd_card_config.error_file_path_base + String(file_name_nr) + ".csv";

  File telemetry_file = _sd->open(_file_paths.telemetry, "w+");

  if (!telemetry_file)
  {
    error("Telemetry file did not open!");
    return false;
  }
  telemetry_file.println(_sd_card_config.telemetry_file_header);
  telemetry_file.close();
  info("Telemetry file created!");

  File info_file = _sd->open(_file_paths.info, "w+");

  if (!info_file)
  {
    error("Info file did not open!");
    return false;
  }
  info_file.println(_sd_card_config.info_file_header);
  info_file.close();
  info("Info file created!");

  File error_file = _sd->open(_file_paths.error, "w+");

  if (!error_file)
  {
    error("Error file did not open!");
    return false;
  }
  error_file.println(_sd_card_config.error_file_header);
  error_file.close();
  info("Error file created!");

  return true;
}

bool SD_Card::_write_to_file(const String &msg, const String &file_path)
{
  if (!initialized())
  {
    return false;
  }

  File file = _sd->open(file_path, "a");
  if (!file)
  {
    return false;
  }
  file.println(msg);
  file.close();

  return true;
}

bool SD_Card::read_last_line_from_file(String &msg, const String &file_path)
{
  if (!initialized())
  {
    return false;
  }

  File file = _sd->open(file_path, "r");
  if (!file)
  {
    error(file_path + " could not be opened!");
    return false;
  }

  if (file.size() <= 2)
  {
    file.close();
    error(file_path + " only contains the header!");
    return false;
  }

  // Seek to the end of the file
  unsigned long seek_location = file.size() - 2; // ignore the last \n
  file.seek(seek_location);

  // Locate last \n
  while (file.available())
  {
    char c = file.read();
    if (c == '\n')
    {
      break;
    }
    // Prevent reading the header
    if (seek_location == 0)
    {
      error(file_path + " trying to read header!");
      return false;
    }
    seek_location--;
    file.seek(seek_location);
  }
  seek_location++; // Move to the start of the line instead of \n

  // Read string
  file.seek(seek_location);
  msg = file.readStringUntil('\n');
  file.close();

  return true;
}