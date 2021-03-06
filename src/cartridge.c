/*
 * This file is part of homoGBA.
 * Copyright (C) 2019  name1e5s<name1e5s@qq.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include <cartridge.h>
#include <log.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

const static uint8_t nintendo_logo[156] = {
    0x24, 0xFF, 0xAE, 0x51, 0x69, 0x9A, 0xA2, 0x21, 0x3D, 0x84, 0x82, 0x0A,
    0x84, 0xE4, 0x09, 0xAD, 0x11, 0x24, 0x8B, 0x98, 0xC0, 0x81, 0x7F, 0x21,
    0xA3, 0x52, 0xBE, 0x19, 0x93, 0x09, 0xCE, 0x20, 0x10, 0x46, 0x4A, 0x4A,
    0xF8, 0x27, 0x31, 0xEC, 0x58, 0xC7, 0xE8, 0x33, 0x82, 0xE3, 0xCE, 0xBF,
    0x85, 0xF4, 0xDF, 0x94, 0xCE, 0x4B, 0x09, 0xC1, 0x94, 0x56, 0x8A, 0xC0,
    0x13, 0x72, 0xA7, 0xFC, 0x9F, 0x84, 0x4D, 0x73, 0xA3, 0xCA, 0x9A, 0x61,
    0x58, 0x97, 0xA3, 0x27, 0xFC, 0x03, 0x98, 0x76, 0x23, 0x1D, 0xC7, 0x61,
    0x03, 0x04, 0xAE, 0x56, 0xBF, 0x38, 0x84, 0x00, 0x40, 0xA7, 0x0E, 0xFD,
    0xFF, 0x52, 0xFE, 0x03, 0x6F, 0x95, 0x30, 0xF1, 0x97, 0xFB, 0xC0, 0x85,
    0x60, 0xD6, 0x80, 0x25, 0xA9, 0x63, 0xBE, 0x03, 0x01, 0x4E, 0x38, 0xE2,
    0xF9, 0xA2, 0x34, 0xFF, 0xBB, 0x3E, 0x03, 0x44, 0x78, 0x00, 0x90, 0xCB,
    0x88, 0x11, 0x3A, 0x94, 0x65, 0xC0, 0x7C, 0x63, 0x87, 0xF0, 0x3C, 0xAF,
    0xD6, 0x25, 0xE4, 0x8B, 0x38, 0x0A, 0xAC, 0x72, 0x21, 0xD4, 0xF8, 0x07,
};

static bool check_header(uint8_t* rom);

uint8_t* cartridge_load(const char* path) {
  FILE* fp = fopen(path, "r");
  if (fp == NULL) {
    log_error("Load file %s failed.", path);
    return NULL;
  }
  fseek(fp, 0L, SEEK_END);

  long size = ftell(fp);
  if (size > 0x02000000) {
    log_error("ROM too big!");
    return NULL;
  }
  fseek(fp, 0L, SEEK_SET);
  uint8_t* cartridge = malloc(sizeof(uint8_t) * size);
  if (fread(cartridge, 1, size, fp) != size) {
    log_error("Load ROM failed!");
    return cartridge;
  }
  if (!check_header(cartridge)) {
    log_error("Header check failed!");
    return cartridge;
  }
  return cartridge;
}

uint8_t* bios_load(const char* path) {
  FILE* fp = fopen(path, "r");
  if (fp == NULL) {
    log_error("Load file %s failed.", path);
    return NULL;
  }
  fseek(fp, 0L, SEEK_END);

  long size = ftell(fp);
  if (size > 0x02000000) {
    log_error("ROM too big!");
    return NULL;
  }
  fseek(fp, 0L, SEEK_SET);
  uint8_t* cartridge = malloc(sizeof(uint8_t) * size);
  if (fread(cartridge, 1, size, fp) != size) {
    log_error("Load ROM failed!");
    return cartridge;
  }
  return cartridge;
}

static bool check_header(uint8_t* rom) {
  cartridge_header header = *(cartridge_header*)rom;
  log_info("Checking ROM Header...");

  char temp[13];
  memcpy(temp, header.title, 12);
  temp[12] = '\0';
  log_info("Game Title:\t\t%s", temp);

  memcpy(temp, header.game_code, 4);
  temp[4] = '\0';
  log_info("Game Code:\t\t%s", temp);

  log_info("Maker Code:\t\t%c%c", header.maker_code[0], header.maker_code[1]);

  bool logo_ok = true;
  for (int i = 0; i < 156; i++) {
    if (nintendo_logo[i] != header.logo[i]) {
      logo_ok = false;
      break;
    }
  }
  log_info("Logo Check:\t\t%s", logo_ok ? "OK" : "FAILED");
  log_info("0x96:\t\t\t%s", header.fixed_value == 0x96 ? "OK" : "FAILED");
  log_info("Unit Code:\t\t0x%02x", header.unit);
  log_info("Device Type:\t\t0x%02x", header.device);
  log_info("Version:\t\t\t0x%02x", header.version);
  uint8_t hash = 0;
  for (int i = 0xA0; i < 0xBD; i++) {
    hash -= rom[i];
  }
  hash -= 0x19;
  bool hash_ok = hash == header.checksum;
  log_info("Hash Check:\t\t%s", hash_ok ? "OK" : "FAILED");
  return (logo_ok && hash_ok);
}