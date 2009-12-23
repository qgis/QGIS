/*
    This file is part of Konsole, an X terminal.

    Copyright (C) 2006-7 by Robert Knight <robertknight@gmail.com>

    Rewritten for QT4 by e_k <e_k at users.sourceforge.net>, Copyright (C)2008

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
    02110-1301  USA.
*/

#ifndef TERMINAL_CHARACTER_DECODER_H
#define TERMINAL_CHARACTER_DECODER_H

#include "Character.h"

class QTextStream;

namespace Konsole
{

  /**
   * Base class for terminal character decoders
   *
   * The decoder converts lines of terminal characters which consist of a unicode character, foreground
   * and background colors and other appearance-related properties into text strings.
   *
   * Derived classes may produce either plain text with no other color or appearance information, or
   * they may produce text which incorporates these additional properties.
   */
  class TerminalCharacterDecoder
  {
    public:
      virtual ~TerminalCharacterDecoder() {}

      /** Begin decoding characters.  The resulting text is appended to @p output. */
      virtual void begin( QTextStream* output ) = 0;
      /** End decoding. */
      virtual void end() = 0;

      /**
       * Converts a line of terminal characters with associated properties into a text string
       * and writes the string into an output QTextStream.
       *
       * @param characters An array of characters of length @p count.
       * @param properties Additional properties which affect all characters in the line
       * @param output The output stream which receives the decoded text
       */
      virtual void decodeLine( const Character* const characters,
                               int count,
                               LineProperty properties ) = 0;
  };

  /**
   * A terminal character decoder which produces plain text, ignoring colors and other appearance-related
   * properties of the original characters.
   */
  class PlainTextDecoder : public TerminalCharacterDecoder
  {
    public:
      PlainTextDecoder();

      /**
       * Set whether trailing whitespace at the end of lines should be included
       * in the output.
       * Defaults to true.
       */
      void setTrailingWhitespace( bool enable );
      /**
       * Returns whether trailing whitespace at the end of lines is included
       * in the output.
       */
      bool trailingWhitespace() const;

      virtual void begin( QTextStream* output );
      virtual void end();

      virtual void decodeLine( const Character* const characters,
                               int count,
                               LineProperty properties );


    private:
      QTextStream* _output;
      bool _includeTrailingWhitespace;
  };

  /**
   * A terminal character decoder which produces pretty HTML markup
   */
  class HTMLDecoder : public TerminalCharacterDecoder
  {
    public:
      /**
       * Constructs an HTML decoder using a default black-on-white color scheme.
       */
      HTMLDecoder();

      /**
       * Sets the color table which the decoder uses to produce the HTML color codes in its
       * output
       */
      void setColorTable( const ColorEntry* table );

      virtual void decodeLine( const Character* const characters,
                               int count,
                               LineProperty properties );

      virtual void begin( QTextStream* output );
      virtual void end();

    private:
      void openSpan( QString& text , const QString& style );
      void closeSpan( QString& text );

      QTextStream* _output;
      const ColorEntry* _colorTable;
      bool _innerSpanOpen;
      quint8 _lastRendition;
      CharacterColor _lastForeColor;
      CharacterColor _lastBackColor;

  };

}

#endif
