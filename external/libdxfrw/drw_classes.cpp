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

#include "drw_classes.h"

#include "intern/dxfreader.h"
#include "intern/dxfwriter.h"
#include "intern/dwgbuffer.h"
#include "intern/drw_dbg.h"

#include "qgslogger.h"


void DRW_Class::parseCode( int code, dxfReader *reader )
{
  switch ( code )
  {
    case 1:
      recName = reader->getUtf8String();
      break;
    case 2:
      className = reader->getUtf8String();
      break;
    case 3:
      appName = reader->getUtf8String();
      break;
    case 90:
      proxyFlag = reader->getInt32();
      break;
    case 91:
      instanceCount = reader->getInt32();
      break;
    case 280:
      wasaProxyFlag = reader->getInt32();
      break;
    case 281:
      entityFlag = reader->getInt32();
      break;
    default:
      break;
  }
}

bool DRW_Class::parseDwg( DRW::Version version, dwgBuffer *buf, dwgBuffer *strBuf )
{
  QgsDebugMsg( "***************************** parsing Class *********************************************" );

  classNum = buf->getBitShort();

  QgsDebugMsg( QString( "Class number: %1" ).arg( classNum ) );

  proxyFlag = buf->getBitShort(); //in dwg specs says "version"

  appName = strBuf->getVariableText( version, false );
  className = strBuf->getVariableText( version, false );
  recName = strBuf->getVariableText( version, false );

  QgsDebugMsg( QString( "app name:%1, class name:%2, dxf rec name:%3" )
               .arg( appName.c_str() ).arg( className.c_str() ).arg( recName.c_str() )
             );

  wasaProxyFlag = buf->getBit(); //in dwg says wasazombie
  entityFlag = buf->getBitShort();
  entityFlag = entityFlag == 0x1F2 ? 1 : 0;

  QgsDebugMsg( QString( "Proxy capabilities flag:%1, proxy flag (280): %2, entity flag:%3" )
               .arg( proxyFlag ).arg( wasaProxyFlag ).arg( entityFlag )
             );

  if ( version > DRW::AC1015 )  //2004+
  {
    instanceCount = buf->getBitLong();
    duint32 dwgVersion = buf->getBitLong();
    int t = buf->getBitLong();
    int unk0 = buf->getBitLong();
    int unk1 = buf->getBitLong();

    QgsDebugMsg( QString( "Instance Count:%1, DWG version: %2, maintenance version:%3, unk0:%4, unk1:%5" )
                 .arg( instanceCount ).arg( dwgVersion ).arg( t ).arg( unk0 ).arg( unk1 )
               );
    Q_UNUSED( instanceCount );
    Q_UNUSED( dwgVersion );
    Q_UNUSED( t );
    Q_UNUSED( unk0 );
    Q_UNUSED( unk1 );
  }

  toDwgType();
  return buf->isGood();
}

void DRW_Class::write( dxfWriter *writer, DRW::Version ver )
{
  if ( ver > DRW::AC1009 )
  {
    writer->writeString( 0, "CLASS" );
    writer->writeString( 1, recName );
    writer->writeString( 2, className );
    writer->writeString( 3, appName );
    writer->writeInt32( 90, proxyFlag );
    if ( ver > DRW::AC1015 ) //2004+
    {
      writer->writeInt32( 91, instanceCount );
    }
    writer->writeInt16( 280, wasaProxyFlag );
    writer->writeInt16( 281, entityFlag );
  }
}

void DRW_Class::toDwgType()
{
  if ( recName == "LWPOLYLINE" )
    dwgType = 77;
  else if ( recName == "HATCH" )
    dwgType = 78;
  else if ( recName == "GROUP" )
    dwgType = 72;
#if 0
  else if ( recName == "GROUP" )
    dwgType = 72;
#endif
  else if ( recName == "LAYOUT" )
    dwgType = 82;
  else if ( recName == "IMAGE" )
    dwgType = 101;
  else if ( recName == "IMAGEDEF" )
    dwgType = 102;
  else
    dwgType = 0;
}
