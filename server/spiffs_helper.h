#pragma once

#include <Arduino.h>
#include <SPIFFS.h>

void initFS()
{
  if (!SPIFFS.begin())
    Serial.println("SPIFFS could not initialize");
}

String readFile(const char *path)
{
  Serial.printf("Reading file: %s\r\n", path);

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
  Serial.printf("- read from file: %s\r\n", fileContent.c_str());
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