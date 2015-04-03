/*
    This file is part of Konsole, an X terminal.

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

#ifndef VT102EMULATION_H
#define VT102EMULATION_H

// Standard Library
#include <stdio.h>

// Qt
#include <QtGui/QKeyEvent>
#include <QtCore/QHash>
#include <QtCore/QTimer>

// Konsole
#include "Emulation.h"
#include "Screen.h"

#define MODE_AppScreen (MODES_SCREEN+0)
#define MODE_AppCuKeys (MODES_SCREEN+1)
#define MODE_AppKeyPad (MODES_SCREEN+2)
#define MODE_Mouse1000 (MODES_SCREEN+3)
#define MODE_Mouse1001 (MODES_SCREEN+4)
#define MODE_Mouse1002 (MODES_SCREEN+5)
#define MODE_Mouse1003 (MODES_SCREEN+6)
#define MODE_Ansi      (MODES_SCREEN+7)
#define MODE_total     (MODES_SCREEN+8)

namespace Konsole
{

  struct DECpar
  {
    bool mode[MODE_total];
  };

  struct CharCodes
  {
    // coding info
    char charset[4]; //
    int  cu_cs;      // actual charset.
    bool graphic;    // Some VT100 tricks
    bool pound  ;    // Some VT100 tricks
    bool sa_graphic; // saved graphic
    bool sa_pound;   // saved pound
  };

  /**
   * Provides an xterm compatible terminal emulation based on the DEC VT102 terminal.
   * A full description of this terminal can be found at http://vt100.net/docs/vt102-ug/
   *
   * In addition, various additional xterm escape sequences are supported to provide
   * features such as mouse input handling.
   * See http://rtfm.etla.org/xterm/ctlseq.html for a description of xterm's escape
   * sequences.
   *
   */
  class Vt102Emulation : public Emulation
  {
      Q_OBJECT

    public:

      /** Constructs a new emulation */
      Vt102Emulation();
      ~Vt102Emulation();

      // reimplemented
      virtual void clearEntireScreen() override;
      virtual void reset() override;

      // reimplemented
      virtual char getErase() const override;

    public slots:

      // reimplemented
      virtual void sendString( const char*, int length = -1 ) override;
      virtual void sendText( const QString& text ) override;
      virtual void sendKeyEvent( QKeyEvent* ) override;
      virtual void sendMouseEvent( int buttons, int column, int line , int eventType ) override;

    protected:
      // reimplemented
      virtual void setMode( int mode ) override;
      virtual void resetMode( int mode ) override;

      // reimplemented
      virtual void receiveChar( int cc ) override;


    private slots:

      //causes changeTitle() to be emitted for each (int,QString) pair in pendingTitleUpdates
      //used to buffer multiple title updates
      void updateTitle();


    private:
      unsigned short applyCharset( unsigned short c );
      void setCharset( int n, int cs );
      void useCharset( int n );
      void setAndUseCharset( int n, int cs );
      void saveCursor();
      void restoreCursor();
      void resetCharset( int scrno );

      void setMargins( int top, int bottom );
      //set margins for all screens back to their defaults
      void setDefaultMargins();

      // returns true if 'mode' is set or false otherwise
      bool getMode( int mode );
      // saves the current boolean value of 'mode'
      void saveMode( int mode );
      // restores the boolean value of 'mode'
      void restoreMode( int mode );
      // resets all modes
      void resetModes();

      void resetToken();
#define MAXPBUF 80
      void pushToToken( int cc );
      int pbuf[MAXPBUF]; //FIXME: overflow?
      int ppos;
#define MAXARGS 15
      void addDigit( int dig );
      void addArgument();
      int argv[MAXARGS];
      int argc;
      void initTokenizer();
      int tbl[256];

      void scan_buffer_report(); //FIXME: rename
      void ReportErrorToken();   //FIXME: rename

      void tau( int code, int p, int q );
      void XtermHack();

      void reportTerminalType();
      void reportSecondaryAttributes();
      void reportStatus();
      void reportAnswerBack();
      void reportCursorPosition();
      void reportTerminalParms( int p );

      void onScrollLock();
      void scrollLock( const bool lock );

      // clears the screen and resizes it to the specified
      // number of columns
      void clearScreenAndSetColumns( int columnCount );

      CharCodes _charset[2];

      DECpar _currParm;
      DECpar _saveParm;

      //hash table and timer for buffering calls to the session instance
      //to update the name of the session
      //or window title.
      //these calls occur when certain escape sequences are seen in the
      //output from the terminal
      QHash<int, QString> _pendingTitleUpdates;
      QTimer* _titleUpdateTimer;

  };

}

#endif // VT102EMULATION_H
