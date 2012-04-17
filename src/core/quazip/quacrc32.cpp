#include "quacrc32.h"

#include "zlib.h"

QuaCrc32::QuaCrc32()
{
  reset();
}

quint32 QuaCrc32::calculate( const QByteArray &data )
{
  return crc32( crc32( 0L, Z_NULL, 0 ), ( const Bytef* )data.data(), data.size() );
}

void QuaCrc32::reset()
{
  checksum = crc32( 0L, Z_NULL, 0 );
}

void QuaCrc32::update( const QByteArray &buf )
{
  checksum = crc32( checksum, ( const Bytef* )buf.data(), buf.size() );
}

quint32 QuaCrc32::value()
{
  return checksum;
}
