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

#ifndef DRW_DBG_H
#define DRW_DBG_H

#include <string>
#include <iostream>

#define DRW_DBGSL(a) DRW_dbg::getInstance()->setLevel(a)
#define DRW_DBGGL DRW_dbg::getInstance()->getLevel()
#define DRW_DBG(a) DRW_dbg::getInstance()->print(a)
#define DRW_DBGH(a) DRW_dbg::getInstance()->printH(a)
#define DRW_DBGB(a) DRW_dbg::getInstance()->printB(a)
#define DRW_DBGHL(a, b, c) DRW_dbg::getInstance()->printHL(a, b ,c)
#define DRW_DBGPT(a, b, c) DRW_dbg::getInstance()->printPT(a, b, c)


class print_none;

class DRW_dbg
{
  public:
    enum LEVEL
    {
      none,
      debug
    };
    void setLevel( LEVEL lvl );
    LEVEL getLevel();
    static DRW_dbg *getInstance();
    void print( std::string s );
    void print( int i );
    void print( unsigned int i );
    void print( long long int i );
    void print( long unsigned int i );
    void print( long long unsigned int i );
    void print( double d );
    void printH( long long int i );
    void printB( int i );
    void printHL( int c, int s, int h );
    void printPT( double x, double y, double z );

  private:
    DRW_dbg();
    static DRW_dbg *instance;
    LEVEL level;
    print_none *prClass = nullptr;
};


#endif // DRW_DBG_H
