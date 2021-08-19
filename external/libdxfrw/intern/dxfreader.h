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

#ifndef DXFREADER_H
#define DXFREADER_H

#include "drw_textcodec.h"

class dxfReader
{
  public:
    enum TYPE
    {
      STRING,
      INT32,
      INT64,
      DOUBLE,
      BOOL,
      INVALID
    };
    enum TYPE type;
  public:
    explicit dxfReader( std::ifstream *stream, bool aSkip = false )
      : doubleData( 0. )
      , intData( 0 )
      , int64( 0 )
      , skip( aSkip )
    {
      filestr = stream;
      type = INVALID;
    }
    virtual ~dxfReader() = default;
    bool readRec( int *code );

    std::string getString() {return strData;}
    int getHandleString();//Convert hex string to int
    std::string toUtf8String( std::string t ) {return decoder.toUtf8( t );}
    std::string getUtf8String() {return decoder.toUtf8( strData );}
    double getDouble() {return doubleData;}
    int getInt32() {return intData;}
    unsigned long long int getInt64() {return int64;}
    bool getBool() { return ( intData == 0 ) ? false : true;}
    int getVersion() {return decoder.getVersion();}
    void setVersion(const std::string &v, bool dxfFormat){decoder.setVersion(v, dxfFormat);}
    void setCodePage(const std::string &c){decoder.setCodePage(c, true);}
    std::string getCodePage() { return decoder.getCodePage();}

  protected:
    virtual bool readCode( int *code ) = 0; //return true if successful (not EOF)
    virtual bool readString( std::string *text ) = 0;
    virtual bool readString() = 0;
    virtual bool readInt16() = 0;
    virtual bool readInt32() = 0;
    virtual bool readInt64() = 0;
    virtual bool readDouble() = 0;
    virtual bool readBool() = 0;

  protected:
    std::ifstream *filestr = nullptr;
    std::string strData;
    double doubleData;
    signed int intData; //32 bits integer
    unsigned long long int int64; //64 bits integer
    bool skip; //set to true for ascii dxf, false for binary
  private:
    DRW_TextCodec decoder;
};

class dxfReaderBinary : public dxfReader
{
  public:
    explicit dxfReaderBinary( std::ifstream *stream )
      : dxfReader( stream, false )
    {}
    virtual bool readCode( int *code );
    virtual bool readString( std::string *text );
    virtual bool readString();
    virtual bool readInt16();
    virtual bool readInt32();
    virtual bool readInt64();
    virtual bool readDouble();
    virtual bool readBool();
};

class dxfReaderAscii : public dxfReader
{
  public:
    explicit dxfReaderAscii( std::ifstream *stream )
      : dxfReader( stream, true )
    {}
    virtual bool readCode( int *code );
    virtual bool readString( std::string *text );
    virtual bool readString();
    virtual bool readInt16();
    virtual bool readDouble();
    virtual bool readInt32();
    virtual bool readInt64();
    virtual bool readBool();
};

#endif // DXFREADER_H
