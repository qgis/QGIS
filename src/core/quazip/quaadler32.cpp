#include "quaadler32.h"

#include "zlib.h"

QuaAdler32::QuaAdler32()
{
  reset();
}

quint32 QuaAdler32::calculate( const QByteArray &data )
{
  return adler32( adler32( 0L, Z_NULL, 0 ), ( const Bytef* )data.data(), data.size() );
}

void QuaAdler32::reset()
{
  checksum = adler32( 0L, Z_NULL, 0 );
}

void QuaAdler32::update( const QByteArray &buf )
{
  checksum = adler32( checksum, ( const Bytef* )buf.data(), buf.size() );
}

quint32 QuaAdler32::value()
{
  return checksum;
}
