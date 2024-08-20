#pragma once

#include <Arduino.h>
#include <SPIFFS.h>

bool _spiffsStarted = false;

void initFS()
{
  if (!_spiffsStarted)
  {
    if (!SPIFFS.begin())
      Serial.println("SPIFFS could not initialize");
    else
      _spiffsStarted = true;
  }
}

String readFile(const char *path)
{

  File file = SPIFFS.open(path);
  if (!file || file.isDirectory())
  {
    Serial.println("- failed to open file for reading");
    return String();
  }

  String fileContent;
  while (file.available())
  {
    fileContent = file.readStringUntil('\n');
    break;
  }
  Serial.printf("Reading file: %s\r - contents: '%s'\r\n", path, fileContent.c_str());
  return fileContent;
}

void writeFile(const char *path, const char *message)
{
  Serial.printf("Writing file: %s\r\n", path);

  File file = SPIFFS.open(path, FILE_WRITE);
  if (!file)
  {
    Serial.println("- failed to open file for writing");
    return;
  }
  Serial.println(file.print(message) ? "- file written" : "- write failed");
}