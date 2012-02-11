/*
    This file is part of Konsole, an X terminal.
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

// Own
#include "Screen.h"

// Standard
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include <string.h>
#include <ctype.h>

// Qt
#include <QtCore/QTextStream>
#include <QtCore/QDate>

// Konsole
#include "konsole_wcwidth.h"
#include "TerminalCharacterDecoder.h"

using namespace Konsole;

//FIXME: this is emulation specific. Use false for xterm, true for ANSI.
//FIXME: see if we can get this from terminfo.
#define BS_CLEARS false

//Macro to convert x,y position on screen to position within an image.
//
//Originally the image was stored as one large contiguous block of
//memory, so a position within the image could be represented as an
//offset from the beginning of the block.  For efficiency reasons this
//is no longer the case.
//Many internal parts of this class still use this representation for parameters and so on,
//notably moveImage() and clearImage().
//This macro converts from an X,Y position into an image offset.
#ifndef loc
#define loc(X,Y) ((Y)*columns+(X))
#endif


Character Screen::defaultChar = Character( ' ',
                                CharacterColor( COLOR_SPACE_DEFAULT, DEFAULT_FORE_COLOR ),
                                CharacterColor( COLOR_SPACE_DEFAULT, DEFAULT_BACK_COLOR ),
                                DEFAULT_RENDITION );

//#define REVERSE_WRAPPED_LINES  // for wrapped line debug

Screen::Screen( int l, int c )
    : lines( l ),
    columns( c ),
    screenLines( new ImageLine[lines+1] ),
    _scrolledLines( 0 ),
    _droppedLines( 0 ),
    hist( new HistoryScrollNone() ),
    cuX( 0 ), cuY( 0 ),
    cu_re( 0 ),
    tmargin( 0 ), bmargin( 0 ),
    tabstops( 0 ),
    sel_begin( 0 ), sel_TL( 0 ), sel_BR( 0 ),
    sel_busy( false ),
    columnmode( false ),
    ef_fg( CharacterColor() ), ef_bg( CharacterColor() ), ef_re( 0 ),
    sa_cuX( 0 ), sa_cuY( 0 ),
    sa_cu_re( 0 ),
    lastPos( -1 )
{
  lineProperties.resize( lines + 1 );
  for ( int i = 0; i < lines + 1; i++ )
    lineProperties[i] = LINE_DEFAULT;

  initTabStops();
  clearSelection();
  reset();
}

/*! Destructor
*/

Screen::~Screen()
{
  delete[] screenLines;
  delete[] tabstops;
  delete hist;
}

/* ------------------------------------------------------------------------- */
/*                                                                           */
/* Normalized                    Screen Operations                           */
/*                                                                           */
/* ------------------------------------------------------------------------- */

// Cursor Setting --------------------------------------------------------------

/*! \section Cursor

    The `cursor' is a location within the screen that is implicitely used in
    many operations. The operations within this section allow to manipulate
    the cursor explicitly and to obtain it's value.

    The position of the cursor is guarantied to be between (including) 0 and
    `columns-1' and `lines-1'.
*/

/*!
    Move the cursor up.

    The cursor will not be moved beyond the top margin.
*/

void Screen::cursorUp( int n )
//=CUU
{
  if ( n == 0 ) n = 1; // Default
  int stop = cuY < tmargin ? 0 : tmargin;
  cuX = qMin( columns - 1, cuX ); // nowrap!
  cuY = qMax( stop, cuY - n );
}

/*!
    Move the cursor down.

    The cursor will not be moved beyond the bottom margin.
*/

void Screen::cursorDown( int n )
//=CUD
{
  if ( n == 0 ) n = 1; // Default
  int stop = cuY > bmargin ? lines - 1 : bmargin;
  cuX = qMin( columns - 1, cuX ); // nowrap!
  cuY = qMin( stop, cuY + n );
}

/*!
    Move the cursor left.

    The cursor will not move beyond the first column.
*/

void Screen::cursorLeft( int n )
//=CUB
{
  if ( n == 0 ) n = 1; // Default
  cuX = qMin( columns - 1, cuX ); // nowrap!
  cuX = qMax( 0, cuX - n );
}

/*!
    Move the cursor left.

    The cursor will not move beyond the rightmost column.
*/

void Screen::cursorRight( int n )
//=CUF
{
  if ( n == 0 ) n = 1; // Default
  cuX = qMin( columns - 1, cuX + n );
}

void Screen::setMargins( int top, int bot )
//=STBM
{
  if ( top == 0 ) top = 1;    // Default
  if ( bot == 0 ) bot = lines;  // Default
  top = top - 1;              // Adjust to internal lineno
  bot = bot - 1;              // Adjust to internal lineno
  if ( !( 0 <= top && top < bot && bot < lines ) )
  {
    qDebug() << " setRegion(" << top << "," << bot << ") : bad range.";
    return;                   // Default error action: ignore
  }
  tmargin = top;
  bmargin = bot;
  cuX = 0;
  cuY = getMode( MODE_Origin ) ? top : 0;

}

int Screen::topMargin() const
{
  return tmargin;
}
int Screen::bottomMargin() const
{
  return bmargin;
}

void Screen::index()
//=IND
{
  if ( cuY == bmargin )
  {
    scrollUp( 1 );
  }
  else if ( cuY < lines - 1 )
    cuY += 1;
}

void Screen::reverseIndex()
//=RI
{
  if ( cuY == tmargin )
    scrollDown( tmargin, 1 );
  else if ( cuY > 0 )
    cuY -= 1;
}

/*!
    Move the cursor to the begin of the next line.

    If cursor is on bottom margin, the region between the
    actual top and bottom margin is scrolled up.
*/

void Screen::NextLine()
//=NEL
{
  Return(); index();
}

void Screen::eraseChars( int n )
{
  if ( n == 0 ) n = 1; // Default
  int p = qMax( 0, qMin( cuX + n - 1, columns - 1 ) );
  clearImage( loc( cuX, cuY ), loc( p, cuY ), ' ' );
}

void Screen::deleteChars( int n )
{
  Q_ASSERT( n >= 0 );

  // always delete at least one char
  if ( n == 0 )
    n = 1;

  // if cursor is beyond the end of the line there is nothing to do
  if ( cuX >= screenLines[cuY].count() )
    return;

  if ( cuX + n >= screenLines[cuY].count() )
    n = screenLines[cuY].count() - 1 - cuX;

  Q_ASSERT( n >= 0 );
  Q_ASSERT( cuX + n < screenLines[cuY].count() );

  screenLines[cuY].remove( cuX, n );
}

void Screen::insertChars( int n )
{
  if ( n == 0 ) n = 1; // Default

  if ( screenLines[cuY].size() < cuX )
    screenLines[cuY].resize( cuX );

  screenLines[cuY].insert( cuX, n, ' ' );

  if ( screenLines[cuY].count() > columns )
    screenLines[cuY].resize( columns );
}

void Screen::deleteLines( int n )
{
  if ( n == 0 ) n = 1; // Default
  scrollUp( cuY, n );
}

/*! insert `n' lines at the cursor position.

    The cursor is not moved by the operation.
*/

void Screen::insertLines( int n )
{
  if ( n == 0 ) n = 1; // Default
  scrollDown( cuY, n );
}

// Mode Operations -----------------------------------------------------------

/*! Set a specific mode. */

void Screen::setMode( int m )
{
  currParm.mode[m] = true;
  switch ( m )
  {
    case MODE_Origin : cuX = 0; cuY = tmargin; break; //FIXME: home
  }
}

/*! Reset a specific mode. */

void Screen::resetMode( int m )
{
  currParm.mode[m] = false;
  switch ( m )
  {
    case MODE_Origin : cuX = 0; cuY = 0; break; //FIXME: home
  }
}

/*! Save a specific mode. */

void Screen::saveMode( int m )
{
  saveParm.mode[m] = currParm.mode[m];
}

/*! Restore a specific mode. */

void Screen::restoreMode( int m )
{
  currParm.mode[m] = saveParm.mode[m];
}

bool Screen::getMode( int m ) const
{
  return currParm.mode[m];
}

void Screen::saveCursor()
{
  sa_cuX     = cuX;
  sa_cuY     = cuY;
  sa_cu_re   = cu_re;
  sa_cu_fg   = cu_fg;
  sa_cu_bg   = cu_bg;
}

void Screen::restoreCursor()
{
  cuX     = qMin( sa_cuX, columns - 1 );
  cuY     = qMin( sa_cuY, lines - 1 );
  cu_re   = sa_cu_re;
  cu_fg   = sa_cu_fg;
  cu_bg   = sa_cu_bg;
  effectiveRendition();
}

/* ------------------------------------------------------------------------- */
/*                                                                           */
/*                             Screen Operations                             */
/*                                                                           */
/* ------------------------------------------------------------------------- */

/*! Resize the screen image

    The topmost left position is maintained, while lower lines
    or right hand side columns might be removed or filled with
    spaces to fit the new size.

    The region setting is reset to the whole screen and the
    tab positions reinitialized.

    If the new image is narrower than the old image then text on lines
    which extends past the end of the new image is preserved so that it becomes
    visible again if the screen is later resized to make it larger.
*/

void Screen::resizeImage( int new_lines, int new_columns )
{
  if (( new_lines == lines ) && ( new_columns == columns ) ) return;

  if ( cuY > new_lines - 1 )
  { // attempt to preserve focus and lines
    bmargin = lines - 1; //FIXME: margin lost
    for ( int i = 0; i < cuY - ( new_lines - 1 ); i++ )
    {
      addHistLine(); scrollUp( 0, 1 );
    }
  }

  // create new screen lines and copy from old to new

  ImageLine* newScreenLines = new ImageLine[new_lines+1];
  for ( int i = 0; i < qMin( lines - 1, new_lines + 1 ) ; i++ )
    newScreenLines[i] = screenLines[i];
  for ( int i = lines; ( i > 0 ) && ( i < new_lines + 1 ); i++ )
    newScreenLines[i].resize( new_columns );

  lineProperties.resize( new_lines + 1 );
  for ( int i = lines; ( i > 0 ) && ( i < new_lines + 1 ); i++ )
    lineProperties[i] = LINE_DEFAULT;

  clearSelection();

  delete[] screenLines;
  screenLines = newScreenLines;

  lines = new_lines;
  columns = new_columns;
  cuX = qMin( cuX, columns - 1 );
  cuY = qMin( cuY, lines - 1 );

  // FIXME: try to keep values, evtl.
  tmargin = 0;
  bmargin = lines - 1;
  initTabStops();
  clearSelection();
}

void Screen::setDefaultMargins()
{
  tmargin = 0;
  bmargin = lines - 1;
}


/*
   Clarifying rendition here and in the display.

   currently, the display's color table is
     0       1       2 .. 9    10 .. 17
     dft_fg, dft_bg, dim 0..7, intensive 0..7

   cu_fg, cu_bg contain values 0..8;
   - 0    = default color
   - 1..8 = ansi specified color

   re_fg, re_bg contain values 0..17
   due to the TerminalDisplay's color table

   rendition attributes are

      attr           widget screen
      -------------- ------ ------
      RE_UNDERLINE     XX     XX    affects foreground only
      RE_BLINK         XX     XX    affects foreground only
      RE_BOLD          XX     XX    affects foreground only
      RE_REVERSE       --     XX
      RE_TRANSPARENT   XX     --    affects background only
      RE_INTENSIVE     XX     --    affects foreground only

   Note that RE_BOLD is used in both widget
   and screen rendition. Since xterm/vt102
   is to poor to distinguish between bold
   (which is a font attribute) and intensive
   (which is a color attribute), we translate
   this and RE_BOLD in falls eventually appart
   into RE_BOLD and RE_INTENSIVE.
*/

void Screen::reverseRendition( Character& p ) const
{
  CharacterColor f = p.foregroundColor;
  CharacterColor b = p.backgroundColor;

  p.foregroundColor = b;
  p.backgroundColor = f; //p->r &= ~RE_TRANSPARENT;
}

void Screen::effectiveRendition()
// calculate rendition
{
  //copy "current rendition" straight into "effective rendition", which is then later copied directly
  //into the image[] array which holds the characters and their appearance properties.
  //- The old version below filtered out all attributes other than underline and blink at this stage,
  //so that they would not be copied into the image[] array and hence would not be visible by TerminalDisplay
  //which actually paints the screen using the information from the image[] array.
  //I don't know why it did this, but I'm fairly sure it was the wrong thing to do.  The net result
  //was that bold text wasn't printed in bold by Konsole.
  ef_re = cu_re;

  //OLD VERSION:
  //ef_re = cu_re & (RE_UNDERLINE | RE_BLINK);

  if ( cu_re & RE_REVERSE )
  {
    ef_fg = cu_bg;
    ef_bg = cu_fg;
  }
  else
  {
    ef_fg = cu_fg;
    ef_bg = cu_bg;
  }

  if ( cu_re & RE_BOLD )
    ef_fg.toggleIntensive();
}

/*!
    returns the image.

    Get the size of the image by \sa getLines and \sa getColumns.

    NOTE that the image returned by this function must later be
    freed.

*/

void Screen::copyFromHistory( Character* dest, int startLine, int count ) const
{
  Q_ASSERT( startLine >= 0 && count > 0 && startLine + count <= hist->getLines() );

  for ( int line = startLine; line < startLine + count; line++ )
  {
    const int length = qMin( columns, hist->getLineLen( line ) );
    const int destLineOffset  = ( line - startLine ) * columns;

    hist->getCells( line, 0, length, dest + destLineOffset );

    for ( int column = length; column < columns; column++ )
      dest[destLineOffset+column] = defaultChar;

    // invert selected text
    if ( sel_begin != -1 )
    {
      for ( int column = 0; column < columns; column++ )
      {
        if ( isSelected( column, line ) )
        {
          reverseRendition( dest[destLineOffset + column] );
        }
      }
    }
  }
}

void Screen::copyFromScreen( Character* dest , int startLine , int count ) const
{
  Q_ASSERT( startLine >= 0 && count > 0 && startLine + count <= lines );

  for ( int line = startLine; line < ( startLine + count ) ; line++ )
  {
    int srcLineStartIndex  = line * columns;
    int destLineStartIndex = ( line - startLine ) * columns;

    for ( int column = 0; column < columns; column++ )
    {
      int srcIndex = srcLineStartIndex + column;
      int destIndex = destLineStartIndex + column;

      dest[destIndex] = screenLines[srcIndex/columns].value( srcIndex % columns, defaultChar );

      // invert selected text
      if ( sel_begin != -1 && isSelected( column, line + hist->getLines() ) )
        reverseRendition( dest[destIndex] );
    }

  }
}

void Screen::getImage( Character* dest, int size, int startLine, int endLine ) const
{
  Q_UNUSED( size );
  Q_ASSERT( startLine >= 0 );
  Q_ASSERT( endLine >= startLine && endLine < hist->getLines() + lines );

  const int mergedLines = endLine - startLine + 1;

  Q_ASSERT( size >= mergedLines * columns );

  const int linesInHistoryBuffer = qBound( 0, hist->getLines() - startLine, mergedLines );
  const int linesInScreenBuffer = mergedLines - linesInHistoryBuffer;

  // copy lines from history buffer
  if ( linesInHistoryBuffer > 0 )
  {
    copyFromHistory( dest, startLine, linesInHistoryBuffer );
  }

  // copy lines from screen buffer
  if ( linesInScreenBuffer > 0 )
  {
    copyFromScreen( dest + linesInHistoryBuffer*columns,
                    startLine + linesInHistoryBuffer - hist->getLines(),
                    linesInScreenBuffer );
  }

  // invert display when in screen mode
  if ( getMode( MODE_Screen ) )
  {
    for ( int i = 0; i < mergedLines*columns; i++ )
      reverseRendition( dest[i] ); // for reverse display
  }

  // mark the character at the current cursor position
  int cursorIndex = loc( cuX, cuY + linesInHistoryBuffer );
  if ( getMode( MODE_Cursor ) && cursorIndex < columns*mergedLines )
    dest[cursorIndex].rendition |= RE_CURSOR;
}

QVector<LineProperty> Screen::getLineProperties( int startLine , int endLine ) const
{
  Q_ASSERT( startLine >= 0 );
  Q_ASSERT( endLine >= startLine && endLine < hist->getLines() + lines );

  const int mergedLines = endLine - startLine + 1;
  const int linesInHistory = qBound( 0, hist->getLines() - startLine, mergedLines );
  const int linesInScreen = mergedLines - linesInHistory;

  QVector<LineProperty> result( mergedLines );
  int index = 0;

  // copy properties for lines in history
  for ( int line = startLine; line < startLine + linesInHistory; line++ )
  {
    //TODO Support for line properties other than wrapped lines
    if ( hist->isWrappedLine( line ) )
    {
      result[index] = ( LineProperty )( result[index] | LINE_WRAPPED );
    }
    index++;
  }

  // copy properties for lines in screen buffer
  const int firstScreenLine = startLine + linesInHistory - hist->getLines();
  for ( int line = firstScreenLine; line < firstScreenLine + linesInScreen; line++ )
  {
    result[index] = lineProperties[line];
    index++;
  }

  return result;
}

/*!
*/

void Screen::reset( bool clearScreen )
{
  setMode( MODE_Wrap ); saveMode( MODE_Wrap );  // wrap at end of margin
  resetMode( MODE_Origin ); saveMode( MODE_Origin );  // position refere to [1,1]
  resetMode( MODE_Insert ); saveMode( MODE_Insert );  // overstroke
  setMode( MODE_Cursor );                       // cursor visible
  resetMode( MODE_Screen );                       // screen not inverse
  resetMode( MODE_NewLine );

  tmargin = 0;
  bmargin = lines - 1;

  setDefaultRendition();
  saveCursor();

  if ( clearScreen )
    clear();
}

/*! Clear the entire screen and home the cursor.
*/

void Screen::clear()
{
  clearEntireScreen();
  home();
}

void Screen::BackSpace()
{
  cuX = qMin( columns - 1, cuX ); // nowrap!
  cuX = qMax( 0, cuX - 1 );
// if (BS_CLEARS) image[loc(cuX,cuY)].character = ' ';

  if ( screenLines[cuY].size() < cuX + 1 )
    screenLines[cuY].resize( cuX + 1 );

  if ( BS_CLEARS ) screenLines[cuY][cuX].character = ' ';
}

void Screen::Tabulate( int n )
{
  // note that TAB is a format effector (does not write ' ');
  if ( n == 0 ) n = 1;
  while (( n > 0 ) && ( cuX < columns - 1 ) )
  {
    cursorRight( 1 ); while (( cuX < columns - 1 ) && !tabstops[cuX] ) cursorRight( 1 );
    n--;
  }
}

void Screen::backTabulate( int n )
{
  // note that TAB is a format effector (does not write ' ');
  if ( n == 0 ) n = 1;
  while (( n > 0 ) && ( cuX > 0 ) )
  {
    cursorLeft( 1 ); while (( cuX > 0 ) && !tabstops[cuX] ) cursorLeft( 1 );
    n--;
  }
}

void Screen::clearTabStops()
{
  for ( int i = 0; i < columns; i++ ) tabstops[i] = false;
}

void Screen::changeTabStop( bool set )
{
  if ( cuX >= columns ) return;
  tabstops[cuX] = set;
}

void Screen::initTabStops()
{
  delete[] tabstops;
  tabstops = new bool[columns];

  // Arrg! The 1st tabstop has to be one longer than the other.
  // i.e. the kids start counting from 0 instead of 1.
  // Other programs might behave correctly. Be aware.
  for ( int i = 0; i < columns; i++ ) tabstops[i] = ( i % 8 == 0 && i != 0 );
}

/*!
   This behaves either as IND (Screen::Index) or as NEL (Screen::NextLine)
   depending on the NewLine Mode (LNM). This mode also
   affects the key sequence returned for newline ([CR]LF).
*/

void Screen::NewLine()
{
  if ( getMode( MODE_NewLine ) ) Return();
  index();
}

/*! put `c' literally onto the screen at the current cursor position.

    VT100 uses the convention to produce an automatic newline (am)
    with the *first* character that would fall onto the next line (xenl).
*/

void Screen::checkSelection( int from, int to )
{
  if ( sel_begin == -1 ) return;
  int scr_TL = loc( 0, hist->getLines() );
  //Clear entire selection if it overlaps region [from, to]
  if (( sel_BR > ( from + scr_TL ) ) && ( sel_TL < ( to + scr_TL ) ) )
  {
    clearSelection();
  }
}

void Screen::ShowCharacter( unsigned short c )
{
  // Note that VT100 does wrapping BEFORE putting the character.
  // This has impact on the assumption of valid cursor positions.
  // We indicate the fact that a newline has to be triggered by
  // putting the cursor one right to the last column of the screen.

  int w = konsole_wcwidth( c );

  if ( w <= 0 )
    return;

  if ( cuX + w > columns )
  {
    if ( getMode( MODE_Wrap ) )
    {
      lineProperties[cuY] = ( LineProperty )( lineProperties[cuY] | LINE_WRAPPED );
      NextLine();
    }
    else
      cuX = columns - w;
  }

  // ensure current line vector has enough elements
  int size = screenLines[cuY].size();
  if ( size == 0 && cuY > 0 )
  {
    screenLines[cuY].resize( qMax( screenLines[cuY-1].size() , cuX + w ) );
  }
  else
  {
    if ( size < cuX + w )
    {
      screenLines[cuY].resize( cuX + w );
    }
  }

  if ( getMode( MODE_Insert ) ) insertChars( w );

  lastPos = loc( cuX, cuY );

  // check if selection is still valid.
  checkSelection( cuX, cuY );

  Character& currentChar = screenLines[cuY][cuX];

  currentChar.character = c;
  currentChar.foregroundColor = ef_fg;
  currentChar.backgroundColor = ef_bg;
  currentChar.rendition = ef_re;

  int i = 0;
  int newCursorX = cuX + w--;
  while ( w )
  {
    i++;

    if ( screenLines[cuY].size() < cuX + i + 1 )
      screenLines[cuY].resize( cuX + i + 1 );

    Character& ch = screenLines[cuY][cuX + i];
    ch.character = 0;
    ch.foregroundColor = ef_fg;
    ch.backgroundColor = ef_bg;
    ch.rendition = ef_re;

    w--;
  }
  cuX = newCursorX;
}

void Screen::compose( const QString& /*compose*/ )
{
  Q_ASSERT( 0 /*Not implemented yet*/ );

  /*  if (lastPos == -1)
       return;

    QChar c(image[lastPos].character);
    compose.prepend(c);
    //compose.compose(); ### FIXME!
    image[lastPos].character = compose[0].unicode();*/
}

int Screen::scrolledLines() const
{
  return _scrolledLines;
}
int Screen::droppedLines() const
{
  return _droppedLines;
}
void Screen::resetDroppedLines()
{
  _droppedLines = 0;
}
void Screen::resetScrolledLines()
{
  //kDebug() << "scrolled lines reset";

  _scrolledLines = 0;
}

// Region commands -------------------------------------------------------------

void Screen::scrollUp( int n )
{
  if ( n == 0 ) n = 1; // Default
  if ( tmargin == 0 ) addHistLine(); // hist.history
  scrollUp( tmargin, n );
}

/*! scroll up `n' lines within current region.
    The `n' new lines are cleared.
    \sa setRegion \sa scrollDown
*/

QRect Screen::lastScrolledRegion() const
{
  return _lastScrolledRegion;
}

void Screen::scrollUp( int from, int n )
{
  if ( n <= 0 || from + n > bmargin ) return;

  _scrolledLines -= n;
  _lastScrolledRegion = QRect( 0, tmargin, columns - 1, ( bmargin - tmargin ) );

  //FIXME: make sure `tmargin', `bmargin', `from', `n' is in bounds.
  moveImage( loc( 0, from ), loc( 0, from + n ), loc( columns - 1, bmargin ) );
  clearImage( loc( 0, bmargin - n + 1 ), loc( columns - 1, bmargin ), ' ' );
}

void Screen::scrollDown( int n )
{
  if ( n == 0 ) n = 1; // Default
  scrollDown( tmargin, n );
}

/*! scroll down `n' lines within current region.
    The `n' new lines are cleared.
    \sa setRegion \sa scrollUp
*/

void Screen::scrollDown( int from, int n )
{

  //kDebug() << "Screen::scrollDown( from: " << from << " , n: " << n << ")";

  _scrolledLines += n;

//FIXME: make sure `tmargin', `bmargin', `from', `n' is in bounds.
  if ( n <= 0 ) return;
  if ( from > bmargin ) return;
  if ( from + n > bmargin ) n = bmargin - from;
  moveImage( loc( 0, from + n ), loc( 0, from ), loc( columns - 1, bmargin - n ) );
  clearImage( loc( 0, from ), loc( columns - 1, from + n - 1 ), ' ' );
}

void Screen::setCursorYX( int y, int x )
{
  setCursorY( y ); setCursorX( x );
}

void Screen::setCursorX( int x )
{
  if ( x == 0 ) x = 1; // Default
  x -= 1; // Adjust
  cuX = qMax( 0, qMin( columns - 1, x ) );
}

void Screen::setCursorY( int y )
{
  if ( y == 0 ) y = 1; // Default
  y -= 1; // Adjust
  cuY = qMax( 0, qMin( lines  - 1, y + ( getMode( MODE_Origin ) ? tmargin : 0 ) ) );
}

void Screen::home()
{
  cuX = 0;
  cuY = 0;
}

void Screen::Return()
{
  cuX = 0;
}

int Screen::getCursorX() const
{
  return cuX;
}

int Screen::getCursorY() const
{
  return cuY;
}

// Erasing ---------------------------------------------------------------------

/*! \section Erasing

    This group of operations erase parts of the screen contents by filling
    it with spaces colored due to the current rendition settings.

    Althought the cursor position is involved in most of these operations,
    it is never modified by them.
*/

/*! fill screen between (including) `loca' (start) and `loce' (end) with spaces.

    This is an internal helper functions. The parameter types are internal
    addresses of within the screen image and make use of the way how the
    screen matrix is mapped to the image vector.
*/

void Screen::clearImage( int loca, int loce, char c )
{
  int scr_TL = loc( 0, hist->getLines() );
  //FIXME: check positions

  //Clear entire selection if it overlaps region to be moved...
  if (( sel_BR > ( loca + scr_TL ) ) && ( sel_TL < ( loce + scr_TL ) ) )
  {
    clearSelection();
  }

  int topLine = loca / columns;
  int bottomLine = loce / columns;

  Character clearCh( c, cu_fg, cu_bg, DEFAULT_RENDITION );

  //if the character being used to clear the area is the same as the
  //default character, the affected lines can simply be shrunk.
  bool isDefaultCh = ( clearCh == Character() );

  for ( int y = topLine; y <= bottomLine; y++ )
  {
    lineProperties[y] = 0;

    int endCol = ( y == bottomLine ) ? loce % columns : columns - 1;
    int startCol = ( y == topLine ) ? loca % columns : 0;

    QVector<Character>& line = screenLines[y];

    if ( isDefaultCh && endCol == columns - 1 )
    {
      line.resize( startCol );
    }
    else
    {
      if ( line.size() < endCol + 1 )
        line.resize( endCol + 1 );

      Character* data = line.data();
      for ( int i = startCol; i <= endCol; i++ )
        data[i] = clearCh;
    }
  }
}

/*! move image between (including) `sourceBegin' and `sourceEnd' to 'dest'.

    The 'dest', 'sourceBegin' and 'sourceEnd' parameters can be generated using
    the loc(column,line) macro.

NOTE:  moveImage() can only move whole lines.

    This is an internal helper functions. The parameter types are internal
    addresses of within the screen image and make use of the way how the
    screen matrix is mapped to the image vector.
*/

void Screen::moveImage( int dest, int sourceBegin, int sourceEnd )
{
  //kDebug() << "moving image from (" << (sourceBegin/columns)
  //    << "," << (sourceEnd/columns) << ") to " <<
  //    (dest/columns);

  Q_ASSERT( sourceBegin <= sourceEnd );

  int lines = ( sourceEnd - sourceBegin ) / columns;

  //move screen image and line properties:
  //the source and destination areas of the image may overlap,
  //so it matters that we do the copy in the right order -
  //forwards if dest < sourceBegin or backwards otherwise.
  //(search the web for 'memmove implementation' for details)
  if ( dest < sourceBegin )
  {
    for ( int i = 0; i <= lines; i++ )
    {
      screenLines[( dest/columns )+i ] = screenLines[( sourceBegin/columns )+i ];
      lineProperties[( dest/columns )+i] = lineProperties[( sourceBegin/columns )+i];
    }
  }
  else
  {
    for ( int i = lines; i >= 0; i-- )
    {
      screenLines[( dest/columns )+i ] = screenLines[( sourceBegin/columns )+i ];
      lineProperties[( dest/columns )+i] = lineProperties[( sourceBegin/columns )+i];
    }
  }

  if ( lastPos != -1 )
  {
    int diff = dest - sourceBegin; // Scroll by this amount
    lastPos += diff;
    if (( lastPos < 0 ) || ( lastPos >= ( lines*columns ) ) )
      lastPos = -1;
  }

  // Adjust selection to follow scroll.
  if ( sel_begin != -1 )
  {
    bool beginIsTL = ( sel_begin == sel_TL );
    int diff = dest - sourceBegin; // Scroll by this amount
    int scr_TL = loc( 0, hist->getLines() );
    int srca = sourceBegin + scr_TL; // Translate index from screen to global
    int srce = sourceEnd + scr_TL; // Translate index from screen to global
    int desta = srca + diff;
    int deste = srce + diff;

    if (( sel_TL >= srca ) && ( sel_TL <= srce ) )
      sel_TL += diff;
    else if (( sel_TL >= desta ) && ( sel_TL <= deste ) )
      sel_BR = -1; // Clear selection (see below)

    if (( sel_BR >= srca ) && ( sel_BR <= srce ) )
      sel_BR += diff;
    else if (( sel_BR >= desta ) && ( sel_BR <= deste ) )
      sel_BR = -1; // Clear selection (see below)

    if ( sel_BR < 0 )
    {
      clearSelection();
    }
    else
    {
      if ( sel_TL < 0 )
        sel_TL = 0;
    }

    if ( beginIsTL )
      sel_begin = sel_TL;
    else
      sel_begin = sel_BR;
  }
}

void Screen::clearToEndOfScreen()
{
  clearImage( loc( cuX, cuY ), loc( columns - 1, lines - 1 ), ' ' );
}

void Screen::clearToBeginOfScreen()
{
  clearImage( loc( 0, 0 ), loc( cuX, cuY ), ' ' );
}

void Screen::clearEntireScreen()
{
  // Add entire screen to history
  for ( int i = 0; i < ( lines - 1 ); i++ )
  {
    addHistLine(); scrollUp( 0, 1 );
  }

  clearImage( loc( 0, 0 ), loc( columns - 1, lines - 1 ), ' ' );
}

/*! fill screen with 'E'
    This is to aid screen alignment
*/

void Screen::helpAlign()
{
  clearImage( loc( 0, 0 ), loc( columns - 1, lines - 1 ), 'E' );
}

void Screen::clearToEndOfLine()
{
  clearImage( loc( cuX, cuY ), loc( columns - 1, cuY ), ' ' );
}

void Screen::clearToBeginOfLine()
{
  clearImage( loc( 0, cuY ), loc( cuX, cuY ), ' ' );
}

void Screen::clearEntireLine()
{
  clearImage( loc( 0, cuY ), loc( columns - 1, cuY ), ' ' );
}

void Screen::setRendition( int re )
{
  cu_re |= re;
  effectiveRendition();
}

void Screen::resetRendition( int re )
{
  cu_re &= ~re;
  effectiveRendition();
}

void Screen::setDefaultRendition()
{
  setForeColor( COLOR_SPACE_DEFAULT, DEFAULT_FORE_COLOR );
  setBackColor( COLOR_SPACE_DEFAULT, DEFAULT_BACK_COLOR );
  cu_re   = DEFAULT_RENDITION;
  effectiveRendition();
}

void Screen::setForeColor( int space, int color )
{
  cu_fg = CharacterColor( space, color );

  if ( cu_fg.isValid() )
    effectiveRendition();
  else
    setForeColor( COLOR_SPACE_DEFAULT, DEFAULT_FORE_COLOR );
}

void Screen::setBackColor( int space, int color )
{
  cu_bg = CharacterColor( space, color );

  if ( cu_bg.isValid() )
    effectiveRendition();
  else
    setBackColor( COLOR_SPACE_DEFAULT, DEFAULT_BACK_COLOR );
}

/* ------------------------------------------------------------------------- */
/*                                                                           */
/*                            Marking & Selection                            */
/*                                                                           */
/* ------------------------------------------------------------------------- */

void Screen::clearSelection()
{
  sel_BR = -1;
  sel_TL = -1;
  sel_begin = -1;
}

void Screen::getSelectionStart( int& column , int& line )
{
  if ( sel_TL != -1 )
  {
    column = sel_TL % columns;
    line = sel_TL / columns;
  }
  else
  {
    column = cuX + getHistLines();
    line = cuY + getHistLines();
  }
}
void Screen::getSelectionEnd( int& column , int& line )
{
  if ( sel_BR != -1 )
  {
    column = sel_BR % columns;
    line = sel_BR / columns;
  }
  else
  {
    column = cuX + getHistLines();
    line = cuY + getHistLines();
  }
}
void Screen::setSelectionStart( /*const ScreenCursor& viewCursor ,*/ const int x, const int y, const bool mode )
{
//  kDebug(1211) << "setSelBeginXY(" << x << "," << y << ")";
  sel_begin = loc( x, y ); //+histCursor) ;

  /* FIXME, HACK to correct for x too far to the right... */
  if ( x == columns ) sel_begin--;

  sel_BR = sel_begin;
  sel_TL = sel_begin;
  columnmode = mode;
}

void Screen::setSelectionEnd( const int x, const int y )
{
//  kDebug(1211) << "setSelExtentXY(" << x << "," << y << ")";
  if ( sel_begin == -1 ) return;
  int l =  loc( x, y ); // + histCursor);

  if ( l < sel_begin )
  {
    sel_TL = l;
    sel_BR = sel_begin;
  }
  else
  {
    /* FIXME, HACK to correct for x too far to the right... */
    if ( x == columns ) l--;

    sel_TL = sel_begin;
    sel_BR = l;
  }
}

bool Screen::isSelected( const int x, const int y ) const
{
  if ( columnmode )
  {
    int sel_Left, sel_Right;
    if ( sel_TL % columns < sel_BR % columns )
    {
      sel_Left = sel_TL; sel_Right = sel_BR;
    }
    else
    {
      sel_Left = sel_BR; sel_Right = sel_TL;
    }
    return ( x >= sel_Left % columns ) && ( x <= sel_Right % columns ) &&
           ( y >= sel_TL / columns ) && ( y <= sel_BR / columns );
    //( y+histCursor >= sel_TL / columns ) && ( y+histCursor <= sel_BR / columns );
  }
  else
  {
    //int pos = loc(x,y+histCursor);
    int pos = loc( x, y );
    return ( pos >= sel_TL && pos <= sel_BR );
  }
}

QString Screen::selectedText( bool preserveLineBreaks )
{
  QString result;
  QTextStream stream( &result, QIODevice::ReadWrite );

  PlainTextDecoder decoder;
  decoder.begin( &stream );
  writeSelectionToStream( &decoder , preserveLineBreaks );
  decoder.end();

  return result;
}

bool Screen::isSelectionValid() const
{
  return ( sel_TL >= 0 && sel_BR >= 0 );
}

void Screen::writeSelectionToStream( TerminalCharacterDecoder* decoder ,
                                     bool preserveLineBreaks )
{
  // do nothing if selection is invalid
  if ( !isSelectionValid() )
    return;

  int top = sel_TL / columns;
  int left = sel_TL % columns;

  int bottom = sel_BR / columns;
  int right = sel_BR % columns;

  Q_ASSERT( top >= 0 && left >= 0 && bottom >= 0 && right >= 0 );

  //kDebug() << "sel_TL = " << sel_TL;
  //kDebug() << "columns = " << columns;

  for ( int y = top; y <= bottom; y++ )
  {
    int start = 0;
    if ( y == top || columnmode ) start = left;

    int count = -1;
    if ( y == bottom || columnmode ) count = right - start + 1;

    const bool appendNewLine = ( y != bottom );
    copyLineToStream( y,
                      start,
                      count,
                      decoder,
                      appendNewLine,
                      preserveLineBreaks );
  }
}


void Screen::copyLineToStream( int line ,
                               int start,
                               int count,
                               TerminalCharacterDecoder* decoder,
                               bool appendNewLine,
                               bool preserveLineBreaks )
{
  //buffer to hold characters for decoding
  //the buffer is static to avoid initialising every
  //element on each call to copyLineToStream
  //(which is unnecessary since all elements will be overwritten anyway)
  static const int MAX_CHARS = 1024;
  static Character characterBuffer[MAX_CHARS];

  assert( count < MAX_CHARS );

  LineProperty currentLineProperties = 0;

  //determine if the line is in the history buffer or the screen image
  if ( line < hist->getLines() )
  {
    const int lineLength = hist->getLineLen( line );

    // ensure that start position is before end of line
    start = qMin( start, qMax( 0, lineLength - 1 ) );

    //retrieve line from history buffer
    if ( count == -1 )
    {
      count = lineLength - start;
    }
    else
    {
      count = qMin( start + count, lineLength ) - start;
    }

    // safety checks
    assert( start >= 0 );
    assert( count >= 0 );
    assert(( start + count ) <= hist->getLineLen( line ) );

    hist->getCells( line, start, count, characterBuffer );

    if ( hist->isWrappedLine( line ) )
      currentLineProperties |= LINE_WRAPPED;
  }
  else
  {
    if ( count == -1 )
      count = columns - start;

    assert( count >= 0 );

    const int screenLine = line - hist->getLines();

    Character* data = screenLines[screenLine].data();
    int length = screenLines[screenLine].count();

    //retrieve line from screen image
    for ( int i = start; i < qMin( start + count, length ); i++ )
    {
      characterBuffer[i-start] = data[i];
    }

    // count cannot be any greater than length
    count = qBound( 0, count, length - start );

    Q_ASSERT( screenLine < lineProperties.count() );
    currentLineProperties |= lineProperties[screenLine];
  }

  //do not decode trailing whitespace characters
  for ( int i = count - 1 ; i >= 0; i-- )
    if ( QChar( characterBuffer[i].character ).isSpace() )
      count--;
    else
      break;

  // add new line character at end
  const bool omitLineBreak = ( currentLineProperties & LINE_WRAPPED ) ||
                             !preserveLineBreaks;

  if ( !omitLineBreak && appendNewLine && ( count + 1 < MAX_CHARS ) )
  {
    characterBuffer[count] = '\n';
    count++;
  }

  //decode line and write to text stream
  decoder->decodeLine(( Character* ) characterBuffer ,
                      count, currentLineProperties );
}

// Method below has been removed because of its reliance on 'histCursor'
// and I want to restrict the methods which have knowledge of the scroll position
// to just those which deal with selection and supplying final screen images.
//
/*void Screen::writeToStream(QTextStream* stream , TerminalCharacterDecoder* decoder) {
  sel_begin = 0;
  sel_BR = sel_begin;
  sel_TL = sel_begin;
  setSelectionEnd(columns-1,lines-1+hist->getLines()-histCursor);

  writeSelectionToStream(stream,decoder);

  clearSelection();
}*/

void Screen::writeToStream( TerminalCharacterDecoder* decoder, int from, int to )
{
  sel_begin = loc( 0, from );
  sel_TL = sel_begin;
  sel_BR = loc( columns - 1, to );
  writeSelectionToStream( decoder );
  clearSelection();
}

QString Screen::getHistoryLine( int no )
{
  sel_begin = loc( 0, no );
  sel_TL = sel_begin;
  sel_BR = loc( columns - 1, no );
  return selectedText( false );
}

void Screen::addHistLine()
{
  // add line to history buffer
  // we have to take care about scrolling, too...

  if ( hasScroll() )
  {
    int oldHistLines = hist->getLines();

    hist->addCellsVector( screenLines[0] );
    hist->addLine( lineProperties[0] & LINE_WRAPPED );

    int newHistLines = hist->getLines();

    bool beginIsTL = ( sel_begin == sel_TL );

    // If the history is full, increment the count
    // of dropped lines
    if ( newHistLines == oldHistLines )
      _droppedLines++;

    // Adjust selection for the new point of reference
    if ( newHistLines > oldHistLines )
    {
      if ( sel_begin != -1 )
      {
        sel_TL += columns;
        sel_BR += columns;
      }
    }

    if ( sel_begin != -1 )
    {
      // Scroll selection in history up
      int top_BR = loc( 0, 1 + newHistLines );

      if ( sel_TL < top_BR )
        sel_TL -= columns;

      if ( sel_BR < top_BR )
        sel_BR -= columns;

      if ( sel_BR < 0 )
      {
        clearSelection();
      }
      else
      {
        if ( sel_TL < 0 )
          sel_TL = 0;
      }

      if ( beginIsTL )
        sel_begin = sel_TL;
      else
        sel_begin = sel_BR;
    }
  }

}

int Screen::getHistLines()
{
  return hist->getLines();
}

void Screen::setScroll( const HistoryType& t , bool copyPreviousScroll )
{
  clearSelection();

  if ( copyPreviousScroll )
    hist = t.scroll( hist );
  else
  {
    HistoryScroll* oldScroll = hist;
    hist = t.scroll( 0 );
    delete oldScroll;
  }
}

bool Screen::hasScroll()
{
  return hist->hasScroll();
}

const HistoryType& Screen::getScroll()
{
  return hist->getType();
}

void Screen::setLineProperty( LineProperty property , bool enable )
{
  if ( enable )
  {
    lineProperties[cuY] = ( LineProperty )( lineProperties[cuY] | property );
  }
  else
  {
    lineProperties[cuY] = ( LineProperty )( lineProperties[cuY] & ~property );
  }
}
void Screen::fillWithDefaultChar( Character* dest, int count )
{
  for ( int i = 0; i < count; i++ )
    dest[i] = defaultChar;
}
