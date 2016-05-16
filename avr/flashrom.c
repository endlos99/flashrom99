/*
 * FlashROM 99
 *
 * A cartridge for the TI 99 that allows for running ROM images stored
 * on an SD card
 *
 * Copyright (c) 2016 Ralph Benzinger <r@0x01.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
 */


#include <stdint.h>

#include "loader.h"


int main(void)
{
  // setup cart
  init();

  // create menu loader
  uint8_t dl = genMenu();
  
  // wait for menu selection
  if (dl == 0)
    selectImage();

  // load selected image
  loadImage();

  // good night and good luck!
  while (1);
}
