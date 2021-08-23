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

#include "dwgbuffer.h"
#include "../libdwgr.h"
#include "drw_textcodec.h"
#include "drw_dbg.h"

static unsigned int crctable[256] =
{
  0x0000, 0xC0C1, 0xC181, 0x0140, 0xC301, 0x03C0, 0x0280, 0xC241,
  0xC601, 0x06C0, 0x0780, 0xC741, 0x0500, 0xC5C1, 0xC481, 0x0440,
  0xCC01, 0x0CC0, 0x0D80, 0xCD41, 0x0F00, 0xCFC1, 0xCE81, 0x0E40,
  0x0A00, 0xCAC1, 0xCB81, 0x0B40, 0xC901, 0x09C0, 0x0880, 0xC841,
  0xD801, 0x18C0, 0x1980, 0xD941, 0x1B00, 0xDBC1, 0xDA81, 0x1A40,
  0x1E00, 0xDEC1, 0xDF81, 0x1F40, 0xDD01, 0x1DC0, 0x1C80, 0xDC41,
  0x1400, 0xD4C1, 0xD581, 0x1540, 0xD701, 0x17C0, 0x1680, 0xD641,
  0xD201, 0x12C0, 0x1380, 0xD341, 0x1100, 0xD1C1, 0xD081, 0x1040,
  0xF001, 0x30C0, 0x3180, 0xF141, 0x3300, 0xF3C1, 0xF281, 0x3240,
  0x3600, 0xF6C1, 0xF781, 0x3740, 0xF501, 0x35C0, 0x3480, 0xF441,
  0x3C00, 0xFCC1, 0xFD81, 0x3D40, 0xFF01, 0x3FC0, 0x3E80, 0xFE41,
  0xFA01, 0x3AC0, 0x3B80, 0xFB41, 0x3900, 0xF9C1, 0xF881, 0x3840,
  0x2800, 0xE8C1, 0xE981, 0x2940, 0xEB01, 0x2BC0, 0x2A80, 0xEA41,
  0xEE01, 0x2EC0, 0x2F80, 0xEF41, 0x2D00, 0xEDC1, 0xEC81, 0x2C40,
  0xE401, 0x24C0, 0x2580, 0xE541, 0x2700, 0xE7C1, 0xE681, 0x2640,
  0x2200, 0xE2C1, 0xE381, 0x2340, 0xE101, 0x21C0, 0x2080, 0xE041,
  0xA001, 0x60C0, 0x6180, 0xA141, 0x6300, 0xA3C1, 0xA281, 0x6240,
  0x6600, 0xA6C1, 0xA781, 0x6740, 0xA501, 0x65C0, 0x6480, 0xA441,
  0x6C00, 0xACC1, 0xAD81, 0x6D40, 0xAF01, 0x6FC0, 0x6E80, 0xAE41,
  0xAA01, 0x6AC0, 0x6B80, 0xAB41, 0x6900, 0xA9C1, 0xA881, 0x6840,
  0x7800, 0xB8C1, 0xB981, 0x7940, 0xBB01, 0x7BC0, 0x7A80, 0xBA41,
  0xBE01, 0x7EC0, 0x7F80, 0xBF41, 0x7D00, 0xBDC1, 0xBC81, 0x7C40,
  0xB401, 0x74C0, 0x7580, 0xB541, 0x7700, 0xB7C1, 0xB681, 0x7640,
  0x7200, 0xB2C1, 0xB381, 0x7340, 0xB101, 0x71C0, 0x7080, 0xB041,
  0x5000, 0x90C1, 0x9181, 0x5140, 0x9301, 0x53C0, 0x5280, 0x9241,
  0x9601, 0x56C0, 0x5780, 0x9741, 0x5500, 0x95C1, 0x9481, 0x5440,
  0x9C01, 0x5CC0, 0x5D80, 0x9D41, 0x5F00, 0x9FC1, 0x9E81, 0x5E40,
  0x5A00, 0x9AC1, 0x9B81, 0x5B40, 0x9901, 0x59C0, 0x5880, 0x9841,
  0x8801, 0x48C0, 0x4980, 0x8941, 0x4B00, 0x8BC1, 0x8A81, 0x4A40,
  0x4E00, 0x8EC1, 0x8F81, 0x4F40, 0x8D01, 0x4DC0, 0x4C80, 0x8C41,
  0x4400, 0x84C1, 0x8581, 0x4540, 0x8701, 0x47C0, 0x4680, 0x8641,
  0x8201, 0x42C0, 0x4380, 0x8341, 0x4100, 0x81C1, 0x8081, 0x4040
};

static unsigned int crc32Table[256] =
{
  0x00000000, 0x77073096, 0xee0e612c, 0x990951ba, 0x076dc419, 0x706af48f, 0xe963a535, 0x9e6495a3,
  0x0edb8832, 0x79dcb8a4, 0xe0d5e91e, 0x97d2d988, 0x09b64c2b, 0x7eb17cbd, 0xe7b82d07, 0x90bf1d91,
  0x1db71064, 0x6ab020f2, 0xf3b97148, 0x84be41de, 0x1adad47d, 0x6ddde4eb, 0xf4d4b551, 0x83d385c7,
  0x136c9856, 0x646ba8c0, 0xfd62f97a, 0x8a65c9ec, 0x14015c4f, 0x63066cd9, 0xfa0f3d63, 0x8d080df5,
  0x3b6e20c8, 0x4c69105e, 0xd56041e4, 0xa2677172, 0x3c03e4d1, 0x4b04d447, 0xd20d85fd, 0xa50ab56b,
  0x35b5a8fa, 0x42b2986c, 0xdbbbc9d6, 0xacbcf940, 0x32d86ce3, 0x45df5c75, 0xdcd60dcf, 0xabd13d59,
  0x26d930ac, 0x51de003a, 0xc8d75180, 0xbfd06116, 0x21b4f4b5, 0x56b3c423, 0xcfba9599, 0xb8bda50f,
  0x2802b89e, 0x5f058808, 0xc60cd9b2, 0xb10be924, 0x2f6f7c87, 0x58684c11, 0xc1611dab, 0xb6662d3d,
  0x76dc4190, 0x01db7106, 0x98d220bc, 0xefd5102a, 0x71b18589, 0x06b6b51f, 0x9fbfe4a5, 0xe8b8d433,
  0x7807c9a2, 0x0f00f934, 0x9609a88e, 0xe10e9818, 0x7f6a0dbb, 0x086d3d2d, 0x91646c97, 0xe6635c01,
  0x6b6b51f4, 0x1c6c6162, 0x856530d8, 0xf262004e, 0x6c0695ed, 0x1b01a57b, 0x8208f4c1, 0xf50fc457,
  0x65b0d9c6, 0x12b7e950, 0x8bbeb8ea, 0xfcb9887c, 0x62dd1ddf, 0x15da2d49, 0x8cd37cf3, 0xfbd44c65,
  0x4db26158, 0x3ab551ce, 0xa3bc0074, 0xd4bb30e2, 0x4adfa541, 0x3dd895d7, 0xa4d1c46d, 0xd3d6f4fb,
  0x4369e96a, 0x346ed9fc, 0xad678846, 0xda60b8d0, 0x44042d73, 0x33031de5, 0xaa0a4c5f, 0xdd0d7cc9,
  0x5005713c, 0x270241aa, 0xbe0b1010, 0xc90c2086, 0x5768b525, 0x206f85b3, 0xb966d409, 0xce61e49f,
  0x5edef90e, 0x29d9c998, 0xb0d09822, 0xc7d7a8b4, 0x59b33d17, 0x2eb40d81, 0xb7bd5c3b, 0xc0ba6cad,
  0xedb88320, 0x9abfb3b6, 0x03b6e20c, 0x74b1d29a, 0xead54739, 0x9dd277af, 0x04db2615, 0x73dc1683,
  0xe3630b12, 0x94643b84, 0x0d6d6a3e, 0x7a6a5aa8, 0xe40ecf0b, 0x9309ff9d, 0x0a00ae27, 0x7d079eb1,
  0xf00f9344, 0x8708a3d2, 0x1e01f268, 0x6906c2fe, 0xf762575d, 0x806567cb, 0x196c3671, 0x6e6b06e7,
  0xfed41b76, 0x89d32be0, 0x10da7a5a, 0x67dd4acc, 0xf9b9df6f, 0x8ebeeff9, 0x17b7be43, 0x60b08ed5,
  0xd6d6a3e8, 0xa1d1937e, 0x38d8c2c4, 0x4fdff252, 0xd1bb67f1, 0xa6bc5767, 0x3fb506dd, 0x48b2364b,
  0xd80d2bda, 0xaf0a1b4c, 0x36034af6, 0x41047a60, 0xdf60efc3, 0xa867df55, 0x316e8eef, 0x4669be79,
  0xcb61b38c, 0xbc66831a, 0x256fd2a0, 0x5268e236, 0xcc0c7795, 0xbb0b4703, 0x220216b9, 0x5505262f,
  0xc5ba3bbe, 0xb2bd0b28, 0x2bb45a92, 0x5cb36a04, 0xc2d7ffa7, 0xb5d0cf31, 0x2cd99e8b, 0x5bdeae1d,
  0x9b64c2b0, 0xec63f226, 0x756aa39c, 0x026d930a, 0x9c0906a9, 0xeb0e363f, 0x72076785, 0x05005713,
  0x95bf4a82, 0xe2b87a14, 0x7bb12bae, 0x0cb61b38, 0x92d28e9b, 0xe5d5be0d, 0x7cdcefb7, 0x0bdbdf21,
  0x86d3d2d4, 0xf1d4e242, 0x68ddb3f8, 0x1fda836e, 0x81be16cd, 0xf6b9265b, 0x6fb077e1, 0x18b74777,
  0x88085ae6, 0xff0f6a70, 0x66063bca, 0x11010b5c, 0x8f659eff, 0xf862ae69, 0x616bffd3, 0x166ccf45,
  0xa00ae278, 0xd70dd2ee, 0x4e048354, 0x3903b3c2, 0xa7672661, 0xd06016f7, 0x4969474d, 0x3e6e77db,
  0xaed16a4a, 0xd9d65adc, 0x40df0b66, 0x37d83bf0, 0xa9bcae53, 0xdebb9ec5, 0x47b2cf7f, 0x30b5ffe9,
  0xbdbdf21c, 0xcabac28a, 0x53b39330, 0x24b4a3a6, 0xbad03605, 0xcdd70693, 0x54de5729, 0x23d967bf,
  0xb3667a2e, 0xc4614ab8, 0x5d681b02, 0x2a6f2b94, 0xb40bbe37, 0xc30c8ea1, 0x5a05df1b, 0x2d02ef8d
};

union typeCast
{
  char buf[8];
  duint16 i16;
  duint32 i32;
  duint64 i64;
  ddouble64 d64;
};

bool dwgFileStream::setPos( duint64 p )
{
  if ( p >= sz )
    return false;

  stream->seekg( p );
  return stream->good();
}

bool dwgFileStream::read( duint8 *s, duint64 n )
{
  stream->read( reinterpret_cast<char *>( s ), n );
  return stream->good();
}

bool dwgCharStream::setPos( duint64 p )
{
  if ( p > size() )
  {
    isOk = false;
    return false;
  }

  pos = p;
  return true;
}

bool dwgCharStream::read( duint8 *s, duint64 n )
{
  if ( n > ( sz - pos ) )
  {
    isOk = false;
    return false;
  }
  for ( duint64 i = 0; i < n; i++ )
  {
    s[i] = stream[pos++];
  }
  return true;
}

dwgBuffer::dwgBuffer( duint8 *buf, duint64 size, DRW_TextCodec *dc )
  : decoder{dc}
  , filestr{new dwgCharStream( buf, size )}
, maxSize{size}
{}

dwgBuffer::dwgBuffer( std::ifstream *stream, DRW_TextCodec *dc )
  : decoder{dc}
  , filestr{new dwgFileStream( stream )}
, maxSize{filestr->size()}
{}

dwgBuffer::dwgBuffer( const dwgBuffer &org )
  : decoder{org.decoder}
  , filestr{org.filestr->clone()}
  , maxSize{filestr->size()}
  , currByte{org.currByte}
  , bitPos{org.bitPos}
{}

dwgBuffer &dwgBuffer::operator=( const dwgBuffer &org )
{
  filestr.reset( org.filestr->clone() );
  decoder = org.decoder;
  maxSize = filestr->size();
  currByte = org.currByte;
  bitPos = org.bitPos;
  return *this;
}

dwgBuffer::~dwgBuffer() = default;

//! Gets the current byte position in buffer
duint64 dwgBuffer::getPosition()
{
  if ( bitPos != 0 )
    return filestr->getPos() - 1;
  return filestr->getPos();
}

//! Sets the buffer position in pos byte, reset the bit position
bool dwgBuffer::setPosition( duint64 pos )
{
  bitPos = 0;
#if 0
  if ( pos >= maxSize )
    return false;
#endif
  return filestr->setPos( pos );
#if 0
  return true;
#endif
}

//RLZ: Fails if ... ???
void dwgBuffer::setBitPos( duint8 pos )
{
  if ( pos > 7 )
    return;
  if ( pos != 0 && bitPos == 0 )
  {
    duint8 buffer;
    filestr->read( &buffer, 1 );
    currByte = buffer;
  }
  if ( pos == 0 && bitPos != 0 ) //reset current byte
  {
    filestr->setPos( filestr->getPos() - 1 );
  }
  bitPos = pos;
}

bool dwgBuffer::moveBitPos( dint32 size )
{
  if ( size == 0 ) return true;

  dint32 b = size + bitPos;
  filestr->setPos( getPosition() + ( b >> 3 ) );
  bitPos = b & 7;

  if ( bitPos != 0 )
  {
    filestr->read( &currByte, 1 );
  }
  return filestr->good();
}

//! Reads one Bit returns a char with value 0/1 (B)
duint8 dwgBuffer::getBit()
{
  duint8 buffer;
  duint8 ret = 0;
  if ( bitPos == 0 )
  {
    filestr->read( &buffer, 1 );
    currByte = buffer;
  }

  ret = ( currByte >> ( 7 - bitPos ) & 1 );
  bitPos += 1;
  if ( bitPos == 8 )
    bitPos = 0;

  return ret;
}

//! Reads one Bit returns a bool value 0==false 1==true (B)
bool dwgBuffer::getBoolBit()
{
  return ( getBit() != 0 );
}

//! Reads two Bits returns a char (BB)
duint8 dwgBuffer::get2Bits()
{
  duint8 buffer;
  duint8 ret = 0;
  if ( bitPos == 0 )
  {
    filestr->read( &buffer, 1 );
    currByte = buffer;
  }

  bitPos += 2;
  if ( bitPos < 9 )
    ret = currByte >> ( 8 - bitPos );
  else  //read one bit per byte
  {
    ret = currByte << 1;
    filestr->read( &buffer, 1 );
    currByte = buffer;
    bitPos = 1;
    ret = ret | currByte >> 7;
  }
  if ( bitPos == 8 )
    bitPos = 0;
  ret = ret & 3;
  return ret;
}

//! Reads thee Bits returns a char (3B)
//RLZ: todo verify this
duint8 dwgBuffer::get3Bits()
{
  duint8 buffer;
  duint8 ret = 0;
  if ( bitPos == 0 )
  {
    filestr->read( &buffer, 1 );
    currByte = buffer;
  }

  bitPos += 3;
  if ( bitPos < 9 )
    ret = currByte >> ( 8 - bitPos );
  else  //read one bit per byte
  {
    ret = currByte << 1;
    filestr->read( &buffer, 1 );
    currByte = buffer;
    bitPos = 1;
    ret = ret | currByte >> 7;
  }
  if ( bitPos == 8 )
    bitPos = 0;
  ret = ret & 7;
  return ret;
}

//! Reads tree Bits returns a char (3B) for R24
//to be written

//! Reads compressed Short (max. 16 + 2 bits) little-endian order, returns a UNsigned 16 bits (BS)
duint16 dwgBuffer::getBitShort()
{
  duint8 b = get2Bits();
  if ( b == 0 )
    return getRawShort16();
  else if ( b == 1 )
    return getRawChar8();
  else if ( b == 2 )
    return 0;
  else
    return 256;
}
//! Reads compressed Short (max. 16 + 2 bits) little-endian order, returns a signed 16 bits (BS)
dint16 dwgBuffer::getSBitShort()
{
  duint8 b = get2Bits();
  if ( b == 0 )
    return static_cast<dint16>( getRawShort16() );
  else if ( b == 1 )
    return static_cast<dint16>( getRawChar8() );
  else if ( b == 2 )
    return 0;
  else
    return 256;
}

//! Reads compressed 32 bits Int (max. 32 + 2 bits) little-endian order, returns a signed 32 bits (BL)
//to be written
dint32 dwgBuffer::getBitLong()
{
  dint8 b = get2Bits();
  if ( b == 0 )
    return getRawLong32();
  else if ( b == 1 )
    return getRawChar8();
  else //if (b == 2)
    return 0;
}

//! Reads compressed 64 bits Int (max. 56 + 3 bits) little-endian order, returns a unsigned 64 bits (BLL)
duint64 dwgBuffer::getBitLongLong()
{
  dint8 b = get3Bits();
  duint64 ret = 0;
  for ( duint8 i = 0; i < b; i++ )
  {
    ret = ret << 8;
    ret |= getRawChar8();
  }
  return ret;
}

//! Reads compressed Double (max. 64 + 2 bits) returns a floating point double of 64 bits (BD)
double dwgBuffer::getBitDouble()
{
  dint8 b = get2Bits();
  if ( b == 1 )
    return 1.0;
  else if ( b == 0 )
  {
    duint8 buffer[8];
    if ( bitPos != 0 )
    {
      for ( int i = 0; i < 8; i++ )
        buffer[i] = getRawChar8();
    }
    else
    {
      filestr->read( buffer, 8 );
    }
    double *ret = reinterpret_cast<double *>( buffer );
    return *ret;
  }
  //    if (b == 2)
  return 0.0;
}

//! Reads 3 compressed Double (max. 64 + 2 bits) returns a DRW_Coord of floating point double of 64 bits (3BD)
DRW_Coord dwgBuffer::get3BitDouble()
{
  DRW_Coord crd;
  crd.x = getBitDouble();
  crd.y = getBitDouble();
  crd.z = getBitDouble();
  return crd;
}

//! Reads raw char 8 bits returns a unsigned char (RC)
duint8 dwgBuffer::getRawChar8()
{
  duint8 ret = 0;
  duint8 buffer = 0;
  filestr->read( &buffer, 1 );
  if ( bitPos == 0 )
    return buffer;
  else
  {
    ret = currByte << bitPos;
    currByte = buffer;
    ret = ret | ( currByte >> ( 8 - bitPos ) );
  }
  return ret;
}

//! Reads raw short 16 bits little-endian order, returns a unsigned short (RS)
duint16 dwgBuffer::getRawShort16()
{
  duint8 buffer[2] = {0, 0};
  duint16 ret = 0;

  filestr->read( buffer, 2 );
  if ( bitPos == 0 )
  {
    /* no offset directly swap bytes for little-endian */
    ret = ( buffer[1] << 8 ) | ( buffer[0] & 0x00FF );
  }
  else
  {
    ret = ( buffer[0] << 8 ) | ( buffer[1] & 0x00FF );
    /* apply offset */
    ret = ret >> ( 8 - bitPos );
    ret = ret | ( currByte << ( 8 + bitPos ) );
    currByte = buffer[1];
    /* swap bytes for little-endian */
    ret = ( ret << 8 ) | ( ret >> 8 );
  }
  return ret;
}

//! Reads raw double IEEE standard 64 bits returns a double (RD)
double dwgBuffer::getRawDouble()
{
  duint8 buffer[8];
  memset( buffer, 0, sizeof( buffer ) );
  if ( bitPos == 0 )
    filestr->read( buffer, 8 );
  else
  {
    for ( int i = 0; i < 8; i++ )
      buffer[i] = getRawChar8();
  }
  double *nOffset = reinterpret_cast<double *>( buffer );
  return *nOffset;
}

//! Reads 2 raw double IEEE standard 64 bits returns a DRW_Coord of floating point double 64 bits (2RD)
DRW_Coord dwgBuffer::get2RawDouble()
{
  DRW_Coord crd;
  crd.x = getRawDouble();
  crd.y = getRawDouble();
  return crd;
}


//! Reads raw int 32 bits little-endian order, returns a unsigned int (RL)
duint32 dwgBuffer::getRawLong32()
{
  duint16 tmp1 = getRawShort16();
  duint16 tmp2 = getRawShort16();
  duint32 ret = ( tmp2 << 16 ) | ( tmp1 & 0x0000FFFF );

  return ret;
}

//! Reads raw int 64 bits little-endian order, returns a unsigned long long (RLL)
duint64 dwgBuffer::getRawLong64()
{
  duint32 tmp1 = getRawLong32();
  duint64 tmp2 = getRawLong32();
  duint64 ret = ( tmp2 << 32 ) | ( tmp1 & 0x00000000FFFFFFFF );

  return ret;
}


//! Reads modular unsigner int, char based, compressed form, little-endian order, returns a unsigned int (U-MC)
duint32 dwgBuffer::getUModularChar()
{
  std::vector<duint8> buffer;
  duint32 result = 0;
  for ( int i = 0; i < 4; i++ )
  {
    duint8 b = getRawChar8();
    buffer.push_back( b & 0x7F );
    if ( !( b & 0x80 ) )
      break;
  }
  int offset = 0;
  for ( unsigned int i = 0; i < buffer.size(); i++ )
  {
    result += buffer[i] << offset;
    offset += 7;
  }
//RLZ: WARNING!!! needed to verify on read handles
  //result = result & 0x7F;
  return result;
}

//! Reads modular int, char based, compressed form, little-endian order, returns a signed int (MC)
dint32 dwgBuffer::getModularChar()
{
  bool negative = false;
  std::vector<dint8> buffer;
  dint32 result = 0;
  for ( int i = 0; i < 4; i++ )
  {
    duint8 b = getRawChar8();
    buffer.push_back( b & 0x7F );
    if ( !( b & 0x80 ) )
      break;
  }
  dint8 b = buffer.back();
  if ( b & 0x40 )
  {
    negative = true;
    buffer.pop_back();
    buffer.push_back( b & 0x3F );
  }

  int offset = 0;
  for ( unsigned int i = 0; i < buffer.size(); i++ )
  {
    result += buffer[i] << offset;
    offset += 7;
  }
  if ( negative )
    result = -result;
  return result;
}

//! Reads modular int, short based, compressed form, little-endian order, returns a unsigned int (MC)
dint32 dwgBuffer::getModularShort()
{
//    bool negative = false;
  std::vector<dint16> buffer;
  dint32 result = 0;
  for ( int i = 0; i < 2; i++ )
  {
    duint16 b = getRawShort16();
    buffer.push_back( b & 0x7FFF );
    if ( !( b & 0x8000 ) )
      break;
  }

  //only positive ?
  /*    dint8 b= buffer.back();
      if (! (b & 0x40)) {
          negative = true;
          buffer.pop_back();
          buffer.push_back(b & 0x3F);
      }*/

  int offset = 0;
  for ( unsigned int i = 0; i < buffer.size(); i++ )
  {
    result += buffer[i] << offset;
    offset += 15;
  }
  /*    if (negative)
          result = -result;*/
  return result;
}

dwgHandle dwgBuffer::getHandle()  //H
{
  dwgHandle hl;
  duint8 data = getRawChar8();
  hl.code = ( data >> 4 ) & 0x0F;
  hl.size = data & 0x0F;
  hl.ref = 0;
  for ( int i = 0; i < hl.size; i++ )
  {
    hl.ref = ( hl.ref << 8 ) | getRawChar8();
  }
  return hl;
}

dwgHandle dwgBuffer::getOffsetHandle( duint32 href )  //H
{
  dwgHandle hl = getHandle();

  if ( hl.code > 5 )
  {
    if ( hl.code == 0x0C )
      hl.ref = href - hl.ref;
    else if ( hl.code == 0x0A )
      hl.ref = href + hl.ref;
    else if ( hl.code == 0x08 )
      hl.ref = href - 1;
    else if ( hl.code == 0x06 )
      hl.ref = href + 1;
//all are soft pointer reference change to 7 (without offset)
    hl.code = 7;
  }
  return hl;
}

//internal until 2004
std::string dwgBuffer::get8bitStr()
{
  duint16 textSize = getBitShort();
  if ( textSize == 0 )
    return std::string();
  duint8 *tmpBuffer = new duint8[textSize];
  bool good = getBytes( tmpBuffer, textSize );
  if ( !good )
    return std::string();

#if 0
  filestr->read( buffer, textSize );
  if ( !filestr->good() )
    return std::string();

  duint8 tmp;
  if ( bitPos != 0 )
  {
    for ( int i = 0; i < textSize; i++ )
    {
      tmp =  buffer[i];
      buffer[i] = ( currByte << bitPos ) | ( tmp >> ( 8 - bitPos ) );
      currByte = tmp;
    }
  }
#endif

  std::string str( reinterpret_cast<char *>( tmpBuffer ), textSize );
  delete[]tmpBuffer;

  return str;
}

//internal since 2007 //pending: are 2 bytes null terminated??
//nullTerm = true if string are 2 bytes null terminated from the stream
std::string dwgBuffer::get16bitStr( duint16 textSize, bool nullTerm )
{
  if ( textSize == 0 )
    return std::string();
  textSize *= 2;
  duint16 ts = textSize;
  if ( nullTerm )
    ts += 2;
  duint8 *tmpBuffer = new duint8[textSize + 2];
  bool good = getBytes( tmpBuffer, ts );
  if ( !good )
    return std::string();
  if ( !nullTerm )
  {
    tmpBuffer[textSize] = '\0';
    tmpBuffer[textSize + 1] = '\0';
  }
  std::string str( reinterpret_cast<char *>( tmpBuffer ), ts );
  delete[]tmpBuffer;

  return str;
}

//T 8 bit text converted from codepage to utf8
std::string dwgBuffer::getCP8Text()
{
  std::string strData;
  strData = get8bitStr();//RLZ correct these function
  if ( !decoder )
    return strData;

  return decoder->toUtf8( strData );
}

//TU unicode 16 bit (UCS) text converted to utf8

//! Reads 2-bytes char (UCS2, nullptr terminated) and convert to std::string (only for Latin-1) ts= total input size in bytes. **/
std::string dwgBuffer::getUCSStr( duint16 ts )
{
  std::string strData;
  if ( ts < 4 ) //at least 1 char
    return std::string();
  strData = get16bitStr( ts / 2, false );
  if ( !decoder )
    return strData;

  return decoder->toUtf8( strData );
}

//TU unicode 16 bit (UCS) text converted to utf8
//nullTerm = true if string are 2 bytes null terminated from the stream
std::string dwgBuffer::getUCSText( bool nullTerm )
{
  std::string strData;
  duint16 ts = getBitShort();
  if ( ts == 0 )
    return std::string();

  strData = get16bitStr( ts, nullTerm );
  if ( !decoder )
    return strData;

  return decoder->toUtf8( strData );
}

//RLZ: read a T or TU if version is 2007+
//nullTerm = true if string are 2 bytes null terminated from the stream
std::string dwgBuffer::getVariableText( DRW::Version v, bool nullTerm ) //TV
{
  if ( v > DRW::AC1018 )
    return getUCSText( nullTerm );
  return getCP8Text();
}
duint16 dwgBuffer::getObjType( DRW::Version v ) //OT
{
  if ( v > DRW::AC1021 )
  {
    duint8 b = get2Bits();
    if ( b == 0 )
      return getRawChar8();
    else if ( b == 1 )
    {
      return ( getRawChar8() + 0x01F0 );
    }
    else //b == 2
      return getRawShort16();
  }
  return getBitShort();
}

/* Bit Extrusion
* For R2000+, this is a single bit, If the single bit is 1,
* the extrusion value is assumed to be 0,0,1 and no explicit
* extrusion is stored. If the single bit is 0, then it will
* be followed by 3BD.
* For R13-R14 this is 3BD.
*/
DRW_Coord dwgBuffer::getExtrusion( bool b_R2000_style, bool &haveExtrusion )
{
  DRW_Coord ext( 0.0, 0.0, 1.0 );
  haveExtrusion = false;
  if ( b_R2000_style )
    /* If the bit is one, the extrusion value is assumed to be 0,0,1*/
    if ( getBit() == 1 )
      return ext;

  /*R13-R14 or bit == 0*/
  ext.x = getBitDouble();
  ext.y = getBitDouble();
  ext.z = getBitDouble();
  haveExtrusion = ext.x != 0.0 || ext.y != 0.0 || ext.z != 1.0;
  return ext;
}

//! Reads compressed Double with default (max. 64 + 2 bits) returns a floating point double of 64 bits (DD)
double dwgBuffer::getDefaultDouble( double d )
{
  dint8 b = get2Bits();
  if ( b == 0 )
    return d;
  else if ( b == 1 )
  {
    duint8 buffer[4];
    char *tmp = nullptr;
    if ( bitPos != 0 )
    {
      for ( int i = 0; i < 4; i++ )
        buffer[i] = getRawChar8();
    }
    else
    {
      filestr->read( buffer, 4 );
    }
    tmp = reinterpret_cast<char *>( &d );
    for ( int i = 0; i < 4; i++ )
      tmp[i] = buffer[i];
    double ret = *reinterpret_cast<double *>( tmp );
    return ret;
  }
  else if ( b == 2 )
  {
    duint8 buffer[6];
    char *tmp = nullptr;
    if ( bitPos != 0 )
    {
      for ( int i = 0; i < 6; i++ )
        buffer[i] = getRawChar8();
    }
    else
    {
      filestr->read( buffer, 6 );
    }
    tmp = reinterpret_cast<char *>( &d );
    for ( int i = 2; i < 6; i++ )
      tmp[i - 2] = buffer[i];
    tmp[4] = buffer[0];
    tmp[5] = buffer[1];
    double ret = *reinterpret_cast<double *>( tmp );
    return ret;
  }
  //    if (b == 3) return a full raw double
  return getRawDouble();
}


/* BitThickness
* For R13-R14, this is a BD.
* For R2000+, this is a single bit, If the bit is one,
* the thickness value is assumed to be 0.0, if not a BD follow
*/
double dwgBuffer::getThickness( bool b_R2000_style )
{
  if ( b_R2000_style )
    /* If the bit is one, the thickness value is assumed to be 0.0.*/
    if ( getBit() == 1 )
      return 0.0;
  /*R13-R14 or bit == 0*/
  return getBitDouble();
}

/* CmColor (CMC)
* For R15 and earlier call directly BS as ACIS color.
* For R2004+, can be CMC or ENC
* RGB value, first 4bits 0xC0 => ByLayer, 0xC1 => ByBlock, 0xC2 => RGB,  0xC3 => last 4 are ACIS
*/
duint32 dwgBuffer::getCmColor( DRW::Version v )
{
  if ( v < DRW::AC1018 ) //2000-
    return getSBitShort();
  duint16 idx = getBitShort();
  duint32 rgb = getBitLong();
  duint8 cb = getRawChar8();
  duint8 type = rgb >> 24;
  DRW_DBG( "type COLOR:" ); DRW_DBG( type ); DRW_DBG( " index COLOR:" ); DRW_DBG( idx );
  DRW_DBG( " RGB COLOR:" ); DRW_DBGH( rgb ); DRW_DBG( " byte COLOR:" ); DRW_DBG( cb ); DRW_DBG( "\n" );
  if ( cb & 1 )
  {
    std::string colorName = getVariableText( v, false );
    DRW_DBG( "colorName:" ); DRW_DBG( colorName ); DRW_DBG( "\n" );
  }
  if ( cb & 2 )
  {
    std::string bookName = getVariableText( v, false );
    DRW_DBG( "bookName:" ); DRW_DBG( bookName ); DRW_DBG( "\n" );
  }

  switch ( type )
  {
    case 0xC0:
      return 256;//ByLayer
    case 0xC1:
      return 0;//ByBlock
    case 0xC2:
      return 256;//RGB RLZ TODO
    case 0xC3:
      return rgb & 0xFF; //ACIS
    default:
      break;
  }
  //check cb if strings follows RLZ TODO
  return 256; //default return ByLayer
}

/* EnColor (ENC)
* For R15 and earlier call directly BS as ACIS color.
* For R2004+, can be CMC or ENC
* RGB value, first 4bits 0xC0 => ByLayer, 0xC1 => ByBlock, 0xC2 => RGB,  0xC3 => last 4 are ACIS
*/
duint32 dwgBuffer::getEnColor( DRW::Version v, int &rgb, int &transparency )
{
  if ( v < DRW::AC1018 ) //2000-
    return getSBitShort();

  rgb = -1;
  transparency = 0;

  duint16 idx = getBitShort();
  DRW_DBG( "idx reads COLOR: " ); DRW_DBGH( idx ); DRW_DBG( "\n" );

  duint16 flags = idx >> 8;

  idx = idx & 0x1FF; //RLZ: warning this is correct?
  DRW_DBG( "flag COLOR:" ); DRW_DBGH( flags ); DRW_DBG( ", index COLOR:" ); DRW_DBGH( idx ); DRW_DBG( "\n" );
  if ( flags & 0x80 )
  {
    // complex color (rgb)
    rgb = getBitLong() & 0xffffff;

    DRW_DBG( "RGB COLOR:" ); DRW_DBGH( rgb ); DRW_DBG( "\n" );
    if ( flags & 0x80 )
    {
      DRW_DBG( "acdbColor COLOR are present\n" );
    }
  }

  if ( flags & 0x20 )
  {
    transparency = getBitLong();
    DRW_DBG( "Transparency COLOR:" ); DRW_DBGH( transparency ); DRW_DBG( "\n" );
  }

  return idx; //default return ByLayer
}


//! Reads raw short 16 bits big-endian order, returns a unsigned short crc & size
duint16 dwgBuffer::getBERawShort16()
{
  char buffer[2];
  buffer[0] = getRawChar8();
  buffer[1] = getRawChar8();
  duint16 size = ( buffer[0] << 8 ) | ( buffer[1] & 0xFF );
  return size;
}

/* reads "size" bytes and stores in "buf" return false if fail */
bool dwgBuffer::getBytes( unsigned char *buf, int size )
{
  duint8 tmp;
  int pos = filestr->getPos();
  filestr->read( buf, size );
  if ( !filestr->good() && ( int ) filestr->getPos() - pos != size )
  {
    DRW_DBG( "short read: wanted " ); DRW_DBG( size ); DRW_DBG( "; got " ); DRW_DBGH( filestr->getPos() - pos ); DRW_DBG( " (at " ); DRW_DBG( filestr->getPos() ); DRW_DBG( "\n" );
    return false;
  }

  if ( bitPos != 0 )
  {
    for ( int i = 0; i < size; i++ )
    {
      tmp =  buf[i];
      buf[i] = ( currByte << bitPos ) | ( tmp >> ( 8 - bitPos ) );
      currByte = tmp;
    }
  }
  return true;
}

duint16 dwgBuffer::crc8( duint16 dx, dint32 start, dint32 end )
{
  duint64 pos = filestr->getPos();
  filestr->setPos( start );
  int n = end - start;
  duint8 *tmpBuf = new duint8[n];
  duint8 *p = tmpBuf;
  filestr->read( tmpBuf, n );
  filestr->setPos( pos );
  if ( !filestr->good() )
    return 0;

  duint8 al;

  while ( n-- > 0 )
  {
    al = ( duint8 )( ( *p ) ^ ( ( dint8 )( dx & 0xFF ) ) );
    dx = ( dx >> 8 ) & 0xFF;
    dx = dx ^ crctable[al & 0xFF];
    p++;
  }
  delete[]tmpBuf;
  return ( dx );
}

duint32 dwgBuffer::crc32( duint32 seed, dint32 start, dint32 end )
{
  duint64 pos = filestr->getPos();
  filestr->setPos( start );
  int n = end - start;
  duint8 *tmpBuf = new duint8[n];
  duint8 *p = tmpBuf;
  filestr->read( tmpBuf, n );
  filestr->setPos( pos );
  if ( !filestr->good() )
    return 0;

  duint32 invertedCrc = ~seed;
  while ( n-- > 0 )
  {
    duint8 data = *p++;
    invertedCrc = ( invertedCrc >> 8 ) ^ crc32Table[( invertedCrc ^ data ) & 0xff];
  }
  delete[]tmpBuf;
  return ~invertedCrc;
}


#if 0
std::string dwgBuffer::getBytes( int size )
{
  char buffer[size];
  char tmp;
  filestr->read( buffer, size );
  if ( !filestr->good() )
    return nullptr;

  if ( bitPos != 0 )
  {
    for ( int i = 0; i <= size; i++ )
    {
      tmp =  buffer[i];
      buffer[i] = ( currByte << bitPos ) | ( tmp >> ( 8 - bitPos ) );
      currByte = tmp;
    }
  }
  std::string st;
  for ( int i = 0; i < size; i++ )
  {
    st.push_back( buffer[i] );
  }
  return st;
//    return std::string(buffer);
}
#endif
