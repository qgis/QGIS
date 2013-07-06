/***************************************************************************
    ColorTables.h
    ---------------------
    begin                : April 2009
    copyright            : (C) 2009 by Paolo Cavallini
    email                : cavallini at faunalia dot it
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef _COLOR_TABLE_H
#define _COLOR_TABLE_H

#include "CharacterColor.h"

using namespace Konsole;

static const ColorEntry whiteonblack_color_table[TABLE_COLORS] =
{
  // normal
  ColorEntry( QColor( 0xFF, 0xFF, 0xFF ), 0, 0 ), ColorEntry( QColor( 0x00, 0x00, 0x00 ), 1, 0 ), // Dfore, Dback
  ColorEntry( QColor( 0x00, 0x00, 0x00 ), 0, 0 ), ColorEntry( QColor( 0xB2, 0x18, 0x18 ), 0, 0 ), // Black, Red
  ColorEntry( QColor( 0x18, 0xB2, 0x18 ), 0, 0 ), ColorEntry( QColor( 0xB2, 0x68, 0x18 ), 0, 0 ), // Green, Yellow
  ColorEntry( QColor( 0x18, 0x18, 0xB2 ), 0, 0 ), ColorEntry( QColor( 0xB2, 0x18, 0xB2 ), 0, 0 ), // Blue, Magenta
  ColorEntry( QColor( 0x18, 0xB2, 0xB2 ), 0, 0 ), ColorEntry( QColor( 0xB2, 0xB2, 0xB2 ), 0, 0 ), // Cyan, White
  // intensiv
  ColorEntry( QColor( 0x00, 0x00, 0x00 ), 0, 1 ), ColorEntry( QColor( 0xFF, 0xFF, 0xFF ), 1, 0 ),
  ColorEntry( QColor( 0x68, 0x68, 0x68 ), 0, 0 ), ColorEntry( QColor( 0xFF, 0x54, 0x54 ), 0, 0 ),
  ColorEntry( QColor( 0x54, 0xFF, 0x54 ), 0, 0 ), ColorEntry( QColor( 0xFF, 0xFF, 0x54 ), 0, 0 ),
  ColorEntry( QColor( 0x54, 0x54, 0xFF ), 0, 0 ), ColorEntry( QColor( 0xFF, 0x54, 0xFF ), 0, 0 ),
  ColorEntry( QColor( 0x54, 0xFF, 0xFF ), 0, 0 ), ColorEntry( QColor( 0xFF, 0xFF, 0xFF ), 0, 0 )
};

static const ColorEntry greenonblack_color_table[TABLE_COLORS] =
{
  ColorEntry( QColor( 24, 240,  24 ),  0, 0 ), ColorEntry( QColor( 0,   0,   0 ),  1, 0 ),
  ColorEntry( QColor( 0,   0,   0 ),  0, 0 ), ColorEntry( QColor( 178,  24,  24 ),  0, 0 ),
  ColorEntry( QColor( 24, 178,  24 ),  0, 0 ), ColorEntry( QColor( 178, 104,  24 ),  0, 0 ),
  ColorEntry( QColor( 24,  24, 178 ),  0, 0 ), ColorEntry( QColor( 178,  24, 178 ),  0, 0 ),
  ColorEntry( QColor( 24, 178, 178 ),  0, 0 ), ColorEntry( QColor( 178, 178, 178 ),  0, 0 ),
  // intensive colors
  ColorEntry( QColor( 24, 240,  24 ),  0, 1 ), ColorEntry( QColor( 0,   0,   0 ),  1, 0 ),
  ColorEntry( QColor( 104, 104, 104 ),  0, 0 ), ColorEntry( QColor( 255,  84,  84 ),  0, 0 ),
  ColorEntry( QColor( 84, 255,  84 ),  0, 0 ), ColorEntry( QColor( 255, 255,  84 ),  0, 0 ),
  ColorEntry( QColor( 84,  84, 255 ),  0, 0 ), ColorEntry( QColor( 255,  84, 255 ),  0, 0 ),
  ColorEntry( QColor( 84, 255, 255 ),  0, 0 ), ColorEntry( QColor( 255, 255, 255 ),  0, 0 )
};

static const ColorEntry blackonlightyellow_color_table[TABLE_COLORS] =
{
  ColorEntry( QColor( 0,   0,   0 ),  0, 0 ),  ColorEntry( QColor( 255, 255, 221 ),  1, 0 ),
  ColorEntry( QColor( 0,   0,   0 ),  0, 0 ),  ColorEntry( QColor( 178,  24,  24 ),  0, 0 ),
  ColorEntry( QColor( 24, 178,  24 ),  0, 0 ),  ColorEntry( QColor( 178, 104,  24 ),  0, 0 ),
  ColorEntry( QColor( 24,  24, 178 ),  0, 0 ),  ColorEntry( QColor( 178,  24, 178 ),  0, 0 ),
  ColorEntry( QColor( 24, 178, 178 ),  0, 0 ),  ColorEntry( QColor( 178, 178, 178 ),  0, 0 ),
  ColorEntry( QColor( 0,   0,   0 ),  0, 1 ),  ColorEntry( QColor( 255, 255, 221 ),  1, 0 ),
  ColorEntry( QColor( 104, 104, 104 ),  0, 0 ),  ColorEntry( QColor( 255,  84,  84 ),  0, 0 ),
  ColorEntry( QColor( 84, 255,  84 ),  0, 0 ),  ColorEntry( QColor( 255, 255,  84 ),  0, 0 ),
  ColorEntry( QColor( 84,  84, 255 ),  0, 0 ),  ColorEntry( QColor( 255,  84, 255 ),  0, 0 ),
  ColorEntry( QColor( 84, 255, 255 ),  0, 0 ),  ColorEntry( QColor( 255, 255, 255 ),  0, 0 )
};

// copy of blackonlightyellow_color_table with background set to white
static const ColorEntry blackonwhite_color_table[TABLE_COLORS] =
{
  ColorEntry( QColor( 0,   0,   0 ),  0, 0 ),  ColorEntry( QColor( 255, 255, 255 ),  1, 0 ),
  ColorEntry( QColor( 0,   0,   0 ),  0, 0 ),  ColorEntry( QColor( 178,  24,  24 ),  0, 0 ),
  ColorEntry( QColor( 24, 178,  24 ),  0, 0 ),  ColorEntry( QColor( 178, 104,  24 ),  0, 0 ),
  ColorEntry( QColor( 24,  24, 178 ),  0, 0 ),  ColorEntry( QColor( 178,  24, 178 ),  0, 0 ),
  ColorEntry( QColor( 24, 178, 178 ),  0, 0 ),  ColorEntry( QColor( 178, 178, 178 ),  0, 0 ),
  ColorEntry( QColor( 0,   0,   0 ),  0, 1 ),  ColorEntry( QColor( 255, 255, 221 ),  1, 0 ),
  ColorEntry( QColor( 104, 104, 104 ),  0, 0 ),  ColorEntry( QColor( 255,  84,  84 ),  0, 0 ),
  ColorEntry( QColor( 84, 255,  84 ),  0, 0 ),  ColorEntry( QColor( 255, 255,  84 ),  0, 0 ),
  ColorEntry( QColor( 84,  84, 255 ),  0, 0 ),  ColorEntry( QColor( 255,  84, 255 ),  0, 0 ),
  ColorEntry( QColor( 84, 255, 255 ),  0, 0 ),  ColorEntry( QColor( 255, 255, 255 ),  0, 0 )
};

#endif

