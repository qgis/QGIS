/*
    This file is part of Konsole, KDE's terminal.

    Copyright (C) 2007 by Robert Knight <robertknight@gmail.com>
    Copyright (C) 1997,1998 by Lars Doelle <lars.doelle@on-line.de>

    Rewritten for QT4 by e_k <e_k at users.sourceforge.net>, Copyright (C)2008

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
    02110-1301  USA.
*/

#ifndef CHARACTERCOLOR_H
#define CHARACTERCOLOR_H

// Qt
#include <QtGui/QColor>

namespace Konsole
{

  /**
   * An entry in a terminal display's color palette.
   *
   * A color palette is an array of 16 ColorEntry instances which map
   * system color indexes (from 0 to 15) into actual colors.
   *
   * Each entry can be set as bold, in which case any text
   * drawn using the color should be drawn in bold.
   *
   * Each entry can also be transparent, in which case the terminal
   * display should avoid drawing the background for any characters
   * using the entry as a background.
   */
  class ColorEntry
  {
    public:
      /**
       * Constructs a new color palette entry.
       *
       * @param c The color value for this entry.
       * @param tr Specifies that the color should be transparent when used as a background color.
       * @param b Specifies that text drawn with this color should be bold.
       */
      ColorEntry( QColor c, bool tr, bool b ) : color( c ), transparent( tr ), bold( b ) {}

      /**
       * Constructs a new color palette entry with an undefined color, and
       * with the transparent and bold flags set to false.
       */
      ColorEntry() : transparent( false ), bold( false ) {}

      /**
       * Sets the color, transparency and boldness of this color to those of @p rhs.
       */
      ColorEntry &operator=( const ColorEntry& rhs )
      {
        color = rhs.color;
        transparent = rhs.transparent;
        bold = rhs.bold;
        return *this;
      }

      /** The color value of this entry for display. */
      QColor color;

      /**
       * If true character backgrounds using this color should be transparent.
       * This is not applicable when the color is used to render text.
       */
      bool   transparent;
      /**
       * If true characters drawn using this color should be bold.
       * This is not applicable when the color is used to draw a character's background.
       */
      bool   bold;
  };


// Attributed Character Representations ///////////////////////////////

// Colors

#define BASE_COLORS   (2+8)
#define INTENSITIES   2
#define TABLE_COLORS  (INTENSITIES*BASE_COLORS)

#define DEFAULT_FORE_COLOR 0
#define DEFAULT_BACK_COLOR 1

//a standard set of colors using black text on a white background.
//defined in TerminalDisplay.cpp

  static const ColorEntry base_color_table[TABLE_COLORS] =
// The following are almost IBM standard color codes, with some slight
// gamma correction for the dim colors to compensate for bright X screens.
// It contains the 8 ansiterm/xterm colors in 2 intensities.
  {
    // Fixme: could add faint colors here, also.
    // normal
    ColorEntry( QColor( 0x00, 0x00, 0x00 ), 0, 0 ), ColorEntry( QColor( 0xB2, 0xB2, 0xB2 ), 1, 0 ), // Dfore, Dback
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

  /* CharacterColor is a union of the various color spaces.

     Assignment is as follows:

     Type  - Space        - Values

     0     - Undefined   - u:  0,      v:0        w:0
     1     - Default     - u:  0..1    v:intense  w:0
     2     - System      - u:  0..7    v:intense  w:0
     3     - Index(256)  - u: 16..255  v:0        w:0
     4     - RGB         - u:  0..255  v:0..256   w:0..256

     Default color space has two separate colors, namely
     default foreground and default background color.
  */

#define COLOR_SPACE_UNDEFINED   0
#define COLOR_SPACE_DEFAULT     1
#define COLOR_SPACE_SYSTEM      2
#define COLOR_SPACE_256         3
#define COLOR_SPACE_RGB         4

  /**
   * Describes the color of a single character in the terminal.
   */
  class CharacterColor
  {
      friend class Character;

    public:
      /** Constructs a new CharacterColor whoose color and color space are undefined. */
      CharacterColor()
          : _colorSpace( COLOR_SPACE_UNDEFINED ),
          _u( 0 ),
          _v( 0 ),
          _w( 0 )
      {}

      /**
       * Constructs a new CharacterColor using the specified @p colorSpace and with
       * color value @p co
       *
       * The meaning of @p co depends on the @p colorSpace used.
       *
       * TODO : Document how @p co relates to @p colorSpace
       *
       * TODO : Add documentation about available color spaces.
       */
      CharacterColor( quint8 colorSpace, int co )
          : _colorSpace( colorSpace ),
          _u( 0 ),
          _v( 0 ),
          _w( 0 )
      {
        switch ( colorSpace )
        {
          case COLOR_SPACE_DEFAULT:
            _u = co & 1;
            break;
          case COLOR_SPACE_SYSTEM:
            _u = co & 7;
            _v = ( co >> 3 ) & 1;
            break;
          case COLOR_SPACE_256:
            _u = co & 255;
            break;
          case COLOR_SPACE_RGB:
            _u = co >> 16;
            _v = co >> 8;
            _w = co;
            break;
          default:
            _colorSpace = COLOR_SPACE_UNDEFINED;
        }
      }

      /**
       * Returns true if this character color entry is valid.
       */
      bool isValid()
      {
        return _colorSpace != COLOR_SPACE_UNDEFINED;
      }

      /**
       * Toggles the value of this color between a normal system color and the corresponding intensive
       * system color.
       *
       * This is only applicable if the color is using the COLOR_SPACE_DEFAULT or COLOR_SPACE_SYSTEM
       * color spaces.
       */
      void toggleIntensive();

      /**
       * Returns the color within the specified color @palette
       *
       * The @p palette is only used if this color is one of the 16 system colors, otherwise
       * it is ignored.
       */
      QColor color( const ColorEntry* palette ) const;

      /**
       * Compares two colors and returns true if they represent the same color value and
       * use the same color space.
       */
      friend bool operator == ( const CharacterColor& a, const CharacterColor& b );
      /**
       * Compares two colors and returns true if they represent different color values
       * or use different color spaces.
       */
      friend bool operator != ( const CharacterColor& a, const CharacterColor& b );

    private:
      quint8 _colorSpace;

      // bytes storing the character color
      quint8 _u;
      quint8 _v;
      quint8 _w;
  };

  inline bool operator == ( const CharacterColor& a, const CharacterColor& b )
  {
    return *reinterpret_cast<const quint32*>( &a._colorSpace ) ==
           *reinterpret_cast<const quint32*>( &b._colorSpace );
  }

  inline bool operator != ( const CharacterColor& a, const CharacterColor& b )
  {
    return *reinterpret_cast<const quint32*>( &a._colorSpace ) !=
           *reinterpret_cast<const quint32*>( &b._colorSpace );
  }

  inline const QColor color256( quint8 u, const ColorEntry* base )
  {
    //   0.. 16: system colors
    if ( u <   8 ) return base[u+2            ].color; u -= 8;
    if ( u <   8 ) return base[u+2+BASE_COLORS].color; u -= 8;

    //  16..231: 6x6x6 rgb color cube
    if ( u < 216 ) return QColor( 255*(( u / 36 ) % 6 ) / 5,
                                    255*(( u / 6 ) % 6 ) / 5,
                                    255*(( u / 1 ) % 6 ) / 5 ); u -= 216;

    // 232..255: gray, leaving out black and white
    int gray = u * 10 + 8; return QColor( gray, gray, gray );
  }

  inline QColor CharacterColor::color( const ColorEntry* base ) const
  {
    switch ( _colorSpace )
    {
      case COLOR_SPACE_DEFAULT: return base[_u+0+( _v?BASE_COLORS:0 )].color;
      case COLOR_SPACE_SYSTEM: return base[_u+2+( _v?BASE_COLORS:0 )].color;
      case COLOR_SPACE_256: return color256( _u, base );
      case COLOR_SPACE_RGB: return QColor( _u, _v, _w );
      case COLOR_SPACE_UNDEFINED: return QColor();
    }

    Q_ASSERT( false ); // invalid color space

    return QColor();
  }

  inline void CharacterColor::toggleIntensive()
  {
    if ( _colorSpace == COLOR_SPACE_SYSTEM || _colorSpace == COLOR_SPACE_DEFAULT )
    {
      _v = !_v;
    }
  }


}

#endif // CHARACTERCOLOR_H

