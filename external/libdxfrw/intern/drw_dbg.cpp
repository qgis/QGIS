/******************************************************************************
**  libDXFrw - Library to read/write DXF files (ascii & binary)              **
**                                                                           **
**  Copyright (C) 2011-2015 José F. Soriano, rallazz@gmail.com               **
**                                                                           **
**  This library is free software, licensed under the terms of the GNU       **
**  General Public License as published by the Free Software Foundation,     **
**  either version 2 of the License, or (at your option) any later version.  **
**  You should have received a copy of the GNU General Public License        **
**  along with this program.  If not, see <http://www.gnu.org/licenses/>.    **
******************************************************************************/

#include "drw_dbg.h"

#include "qgslogger.h"


#include <QTextStream>
#include <QStringList>

DRW_dbg *DRW_dbg::instance = nullptr;

/*********private classes*************/
class print_none
{
  public:
    virtual void printS( std::string s ) {( void )s;}
    virtual void printI( long long int i ) {( void )i;}
    virtual void printUI( long long unsigned int i ) {( void )i;}
    virtual void printD( double d ) {( void )d;}
    virtual void printH( long long int i ) {( void )i;}
    virtual void printB( int i ) {( void )i;}
    virtual void printHL( int c, int s, int h ) {( void )c; ( void )s; ( void )h;}
    virtual void printPT( double x, double y, double z ) {( void )x; ( void )y; ( void )z;}
    print_none() {}
    virtual ~print_none() = default;
};

class print_debug : public print_none
{
  public:
    virtual void printS( std::string s );
    virtual void printI( long long int i );
    virtual void printUI( long long unsigned int i );
    virtual void printD( double d );
    virtual void printH( long long int i );
    virtual void printB( int i );
    virtual void printHL( int c, int s, int h );
    virtual void printPT( double x, double y, double z );
    print_debug();
    virtual ~print_debug() { QgsDebugMsgLevel( mBuf, 5 ); }
  private:
    QString mBuf;
    QTextStream mTS;
    void flush();
};

/********* debug class *************/
DRW_dbg *DRW_dbg::getInstance()
{
  if ( !instance )
  {
    instance = new DRW_dbg;
  }
  return instance;
}

DRW_dbg::DRW_dbg()
{
  level = none;
  prClass = new print_none;
}

void DRW_dbg::setLevel( LEVEL lvl )
{
  level = lvl;
  delete prClass;
  switch ( level )
  {
    case debug:
      prClass = new print_debug;
      break;
    default:
      prClass = new print_none;
  }
}

DRW_dbg::LEVEL DRW_dbg::getLevel()
{
  return level;
}

void DRW_dbg::print( std::string s )
{
  prClass->printS( s );
}

void DRW_dbg::print( int i )
{
  prClass->printI( i );
}

void DRW_dbg::print( unsigned int i )
{
  prClass->printUI( i );
}

void DRW_dbg::print( long long int i )
{
  prClass->printI( i );
}

void DRW_dbg::print( long unsigned int i )
{
  prClass->printUI( i );
}

void DRW_dbg::print( long long unsigned int i )
{
  prClass->printUI( i );
}

void DRW_dbg::print( double d )
{
  prClass->printD( d );
}

void DRW_dbg::printH( long long int i )
{
  prClass->printH( i );
}

void DRW_dbg::printB( int i )
{
  prClass->printB( i );
}
void DRW_dbg::printHL( int c, int s, int h )
{
  prClass->printHL( c, s, h );
}

void DRW_dbg::printPT( double x, double y, double z )
{
  prClass->printPT( x, y, z );
}

print_debug::print_debug() : mTS( &mBuf )
{
}

void print_debug::flush()
{
  QStringList lines = mBuf.split( '\n' );
  for ( int i = 0; i < lines.size() - 1; i++ )
  {
    QgsDebugMsgLevel( lines[i], 4 );
  }
  mBuf = lines.last();
}

void print_debug::printS( std::string s )
{
  mTS << QString::fromStdString( s );
  flush();
}

void print_debug::printI( long long int i )
{
  mTS << i;
  flush();
}

void print_debug::printUI( long long unsigned int i )
{
  mTS << i;
  flush();
}

void print_debug::printD( double d )
{
  mTS << QStringLiteral( "%1 " ).arg( d, 0, 'g' );
  flush();
}

void print_debug::printH( long long  i )
{
  mTS << QStringLiteral( "0x%1" ).arg( i, 0, 16 );
  flush();
}

void print_debug::printB( int i )
{
  mTS << QStringLiteral( "0%1" ).arg( i, 0, 8 );
  flush();
}

void print_debug::printHL( int c, int s, int h )
{
  mTS << QStringLiteral( "%1.%2 0x%3" ).arg( c ).arg( s ).arg( h, 0, 16 );
  flush();
}

void print_debug::printPT( double x, double y, double z )
{
  mTS << QStringLiteral( "x:%1 y:%2 z:%3" ).arg( x, 0, 'g' ).arg( y, 0, 'g' ).arg( z, 0, 'g' );
  flush();
}
