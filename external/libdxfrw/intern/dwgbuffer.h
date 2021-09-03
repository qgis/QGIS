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

#ifndef DWGBUFFER_H
#define DWGBUFFER_H

#include <fstream>
#include <sstream>
#include <memory>
#include "../drw_base.h"

class DRW_Coord;
class DRW_TextCodec;

class dwgBasicStream
{
  protected:
    dwgBasicStream() {}
  public:
    virtual ~dwgBasicStream() = default;
    virtual bool read( duint8 *s, duint64 n ) = 0;
    virtual duint64 size() = 0;
    virtual duint64 getPos() = 0;
    virtual bool setPos( duint64 p ) = 0;
    virtual bool good() = 0;
    virtual dwgBasicStream *clone() = 0;
};

class dwgFileStream: public dwgBasicStream
{
  public:
    explicit dwgFileStream( std::ifstream *s )
    {
      stream = s;
      stream->seekg( 0, std::ios::end );
      sz = stream->tellg();
      stream->seekg( 0, std::ios_base::beg );
    }
    virtual bool read( duint8 *s, duint64 n );
    virtual duint64 size() {return sz;}
    virtual duint64 getPos() {return stream->tellg();}
    virtual bool setPos( duint64 p );
    virtual bool good() {return stream->good();}
    virtual dwgBasicStream *clone() {return new dwgFileStream( stream );}
  private:
    std::ifstream *stream{nullptr};
    duint64 sz{0};
};

class dwgCharStream: public dwgBasicStream
{
  public:
    dwgCharStream( duint8 *buf, duint64 s )
        :stream{buf}
        ,sz{s}
    {}
    virtual bool read( duint8 *s, duint64 n );
    virtual duint64 size() {return sz;}
    virtual duint64 getPos() {return pos;}
    virtual bool setPos( duint64 p );
    virtual bool good() {return isOk;}
    virtual dwgBasicStream *clone() {return new dwgCharStream( stream, sz );}
  private:
    duint8 *stream{nullptr};
    duint64 sz{0};
    duint64 pos{0};
    bool isOk{true};
};

class dwgBuffer
{
  public:
    dwgBuffer( std::ifstream *stream, DRW_TextCodec *decoder = nullptr );
    dwgBuffer( duint8 *buf, duint64 size, DRW_TextCodec *decoder = nullptr );
    dwgBuffer( const dwgBuffer &org );
    dwgBuffer &operator=( const dwgBuffer &org );
    ~dwgBuffer();
    duint64 size() {return filestr->size();}
    bool setPosition( duint64 pos );
    duint64 getPosition();
    void resetPosition() {setPosition( 0 ); setBitPos( 0 );}
    void setBitPos( duint8 pos );
    duint8 getBitPos() {return bitPos;}
    bool moveBitPos( dint32 size );

    duint8 getBit();  //B
    bool getBoolBit();  //B as bool
    duint8 get2Bits(); //BB
    duint8 get3Bits(); //3B
    duint16 getBitShort(); //BS
    dint16 getSBitShort(); //BS
    dint32 getBitLong(); //BL
    duint64 getBitLongLong();  //BLL (R24)
    double getBitDouble(); //BD
    //2BD => call BD 2 times
    DRW_Coord get3BitDouble(); //3BD
    duint8 getRawChar8();  //RC
    duint16 getRawShort16();  //RS
    double getRawDouble(); //RD
    duint32 getRawLong32();   //RL
    duint64 getRawLong64();   //RLL
    DRW_Coord get2RawDouble(); //2RD
    //3RD => call RD 3 times
    duint32 getUModularChar(); //UMC, unsigned for offsets in 1015
    dint32 getModularChar(); //MC
    dint32 getModularShort(); //MS
    dwgHandle getHandle(); //H
    dwgHandle getOffsetHandle( duint32 href ); //H converted to hard
    UTF8STRING getVariableText( DRW::Version v, bool nullTerm = true ); //TV => call TU for 2007+ or T for previous versions
    UTF8STRING getCP8Text(); //T 8 bit text converted from codepage to utf8
    UTF8STRING getUCSText( bool nullTerm = true ); //TU unicode 16 bit (UCS) text converted to utf8
    UTF8STRING getUCSStr( duint16 ts );

    duint16 getObjType( DRW::Version v );  //OT

    //X, U, SN,

    DRW_Coord getExtrusion( bool b_R2000_style, bool &haveExtrusion ); //BE
    double getDefaultDouble( double d ); //DD
    double getThickness( bool b_R2000_style );//BT
    //3DD
    duint32 getCmColor( DRW::Version v ); //CMC
    duint32 getEnColor( DRW::Version v, int &rgb, int &transparency ); //ENC
    //TC

    duint16 getBERawShort16();  //RS big-endian order

    bool isGood() {return filestr->good();}
    bool getBytes( duint8 *buf, int size );
    int numRemainingBytes() {return ( maxSize - filestr->getPos() );}

    duint16 crc8( duint16 dx, dint32 start, dint32 end );
    duint32 crc32( duint32 seed, dint32 start, dint32 end );

//    duint8 getCurrByte(){return currByte;}
    DRW_TextCodec *decoder{nullptr};

  private:
    std::unique_ptr<dwgBasicStream> filestr;
    duint64 maxSize{0};
    duint8 currByte{0};
    duint8 bitPos{0};

    UTF8STRING get8bitStr();
    UTF8STRING get16bitStr( duint16 textSize, bool nullTerm = true );
};

#endif // DWGBUFFER_H
