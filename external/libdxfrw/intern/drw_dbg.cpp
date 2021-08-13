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
class print_debug : public DRW::DebugPrinter {
public:
    void printS(const std::string& s) override;
    void printI(long long int i) override;
    void printUI(long long unsigned int i) override;
    void printD(double d) override;
    void printH(long long int i) override;
    void printB(int i) override;
    void printHL(int c, int s, int h) override;
    void printPT(double x, double y, double z) override;
    ~print_debug() override { QgsDebugMsgLevel( mBuf, 5 ); }
private:
    std::ios_base::fmtflags flags{std::cerr.flags()};
    QString mBuf;
    QTextStream mTS;
    void flush();
};

/********* debug class *************/
DRW_dbg *DRW_dbg::getInstance(){
    if (!instance){
        instance = new DRW_dbg;
    }
    return instance;
}

DRW_dbg::DRW_dbg(){
    debugPrinter.reset(new print_debug);
    currentPrinter = &silentDebug;
}

void DRW_dbg::setCustomDebugPrinter(std::unique_ptr<DRW::DebugPrinter> printer)
{
    debugPrinter = std::move( printer );
    if (level == Level::Debug){
        currentPrinter = debugPrinter.get();
    }
}

void DRW_dbg::setLevel(Level lvl){
    level = lvl;
    switch (level){
    case Level::Debug:
        currentPrinter = debugPrinter.get();
        break;
    case Level::None:
        currentPrinter = &silentDebug;
        break;
    }
}

DRW_dbg::Level DRW_dbg::getLevel(){
    return level;
}

void DRW_dbg::print(const std::string &s){
    currentPrinter->printS(s);
}

void DRW_dbg::print(int i){
    currentPrinter->printI(i);
}

void DRW_dbg::print(unsigned int i){
    currentPrinter->printUI(i);
}

void DRW_dbg::print(long long int i){
    currentPrinter->printI(i);
}

void DRW_dbg::print(long unsigned int i){
    currentPrinter->printUI(i);
}

void DRW_dbg::print(long long unsigned int i){
    currentPrinter->printUI(i);
}

void DRW_dbg::print(double d){
    currentPrinter->printD(d);
}

void DRW_dbg::printH(long long int i){
    currentPrinter->printH(i);
}

void DRW_dbg::printB(int i){
    currentPrinter->printB(i);
}
void DRW_dbg::printHL(int c, int s, int h){
    currentPrinter->printHL(c, s, h);
}

void DRW_dbg::printPT(double x, double y, double z){
    currentPrinter->printPT(x, y, z);
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

void print_debug::printS( const std::string& s )
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
