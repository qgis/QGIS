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

#include <iostream>
#include <cmath>

#include "drw_objects.h"
#include "intern/dxfreader.h"
#include "intern/dxfwriter.h"
#include "intern/dwgbuffer.h"
#include "intern/drw_dbg.h"
#include "intern/dwgutil.h"

#undef QGISDEBUG
#include "qgslogger.h"
#include <QStringList>

#define RESERVE( vector, size ) try { \
    vector.reserve(size); \
  } catch(const std::exception &e) { \
    QgsDebugMsg( QString( "allocation exception (size=%1; error=%2)" ).arg( size ).arg( e.what() ) ); \
    throw e; \
  }

/*!
 *  Base class for tables entries
 *  @author Rallaz
 */
void DRW_TableEntry::parseCode( int code, dxfReader *reader )
{
  switch ( code )
  {
    case 5:
      handle = reader->getHandleString();
      break;
    case 330:
      parentHandle = reader->getHandleString();
      break;
    case 2:
      name = reader->getUtf8String();
      break;
    case 70:
      flags = reader->getInt32();
      break;
    case 1000:
    case 1001:
    case 1002:
    case 1003:
    case 1004:
    case 1005:
      extData.push_back( new DRW_Variant( code, reader->getString() ) );
      break;
    case 1010:
    case 1011:
    case 1012:
    case 1013:
      curr = new DRW_Variant( code, DRW_Coord( reader->getDouble(), 0.0, 0.0 ) );
      extData.push_back( curr );
      break;
    case 1020:
    case 1021:
    case 1022:
    case 1023:
      if ( curr )
        curr->setCoordY( reader->getDouble() );
      break;
    case 1030:
    case 1031:
    case 1032:
    case 1033:
      if ( curr )
        curr->setCoordZ( reader->getDouble() );
      curr = nullptr;
      break;
    case 1040:
    case 1041:
    case 1042:
      extData.push_back( new DRW_Variant( code, reader->getDouble() ) );
      break;
    case 1070:
    case 1071:
      extData.push_back( new DRW_Variant( code, reader->getInt32() ) );
      break;
    default:
      break;
  }
}

bool DRW_TableEntry::parseDwg( DRW::Version version, dwgBuffer *buf, dwgBuffer *strBuf, duint32 bs )
{
  QgsDebugMsg( "***************************** parsing table entry *********************************************" );

  objSize = 0;
  oType = buf->getObjType( version );
  QgsDebugMsg( QString( "Object type: %1, 0x%2" ).arg( oType ).arg( oType, 0, 16 ) );

  if ( version > DRW::AC1014 && version < DRW::AC1024 )  //2000 to 2007
  {
    objSize = buf->getRawLong32();  //RL 32bits object size in bits
    QgsDebugMsg( QString( "Object size: %1" ).arg( objSize ) );
  }
  if ( version > DRW::AC1021 )  //2010+
  {
    duint32 ms = buf->size();
    objSize = ms * 8 - bs;
    QgsDebugMsg( QString( "Object size: %1" ).arg( objSize ) );
  }
  if ( strBuf && version > DRW::AC1018 )  //2007+
  {
    strBuf->moveBitPos( objSize - 1 );
    QgsDebugMsg( QString( "strBuf strbit pos 2007: %1; strBuf bpos 2007:%2" ).arg( strBuf->getPosition() ).arg( strBuf->getBitPos() ) );

    if ( strBuf->getBit() == 1 )
    {
      QgsDebugMsg( "string bit is 1" );
      strBuf->moveBitPos( -17 );
      duint16 strDataSize = strBuf->getRawShort16();
      QgsDebugMsg( QString( "string strDataSize: 0x%1" ).arg( strDataSize, 0, 16 ) );

      if ( ( strDataSize & 0x8000 ) == 0x8000 )
      {
        QgsDebugMsg( "string 0x8000 bit is set" );
        strBuf->moveBitPos( -33 );//RLZ pending to verify
        duint16 hiSize = strBuf->getRawShort16();
        strDataSize = ( ( strDataSize & 0x7fff ) | ( hiSize << 15 ) );
      }
      strBuf->moveBitPos( -strDataSize - 16 ); //-14

      QgsDebugMsg( QString( "strBuf strbit pos 2007: %1; strBuf bpos 2007:%2" ).arg( strBuf->getPosition() ).arg( strBuf->getBitPos() ) );
    }
    else
    {
      QgsDebugMsg( "string bit is 0" );
    }

    QgsDebugMsg( QString( "strBuf start pos 2007: %1; strBuf bpos 2007:%2" ).arg( strBuf->getPosition() ).arg( strBuf->getBitPos() ) );
  }

  dwgHandle ho = buf->getHandle();
  handle = ho.ref;
  QgsDebugMsg( QString( "TableEntry Handle: %1.%2 0x%3" ).arg( ho.code ).arg( ho.size ).arg( ho.ref, 0, 16 ) );

  dint16 extDataSize = buf->getBitShort(); //BS
  QgsDebugMsg( QString( " ext data size: %1" ).arg( extDataSize ) );
  while ( extDataSize > 0 && buf->isGood() )
  {
    /* RLZ: TODO */
    dwgHandle ah = buf->getHandle();
    QgsDebugMsg( QString( "App Handle: %1.%2 0x%3" ).arg( ah.code ).arg( ah.size ).arg( ah.ref, 0, 16 ) );
    duint8 *tmpExtData = new duint8[extDataSize];
    buf->getBytes( tmpExtData, extDataSize );
    dwgBuffer tmpExtDataBuf( tmpExtData, extDataSize, buf->decoder );
    int pos = tmpExtDataBuf.getPosition();
    int bpos = tmpExtDataBuf.getBitPos();
    QgsDebugMsg( QString( "ext data pos:%1.%2" ).arg( pos ).arg( bpos ) );
    Q_UNUSED( pos );
    Q_UNUSED( bpos );
    duint8 dxfCode = tmpExtDataBuf.getRawChar8();
    QgsDebugMsg( QString( "dxfCode:%1" ).arg( dxfCode ) );
    Q_UNUSED( dxfCode );
    switch ( dxfCode )
    {
      case 0:
      {
        duint8 strLength = tmpExtDataBuf.getRawChar8();
        QgsDebugMsg( QString( "strLength:%1" ).arg( strLength ) );
        duint16 cp = tmpExtDataBuf.getBERawShort16();
        QgsDebugMsg( QString( "str codepage:%1" ).arg( cp ) );
        Q_UNUSED( cp );
        for ( int i = 0; i < strLength + 1; i++ ) //string length + null terminating char
        {
          duint8 dxfChar = tmpExtDataBuf.getRawChar8();
          QgsDebugMsg( QString( " dxfChar:%1" ).arg( dxfChar ) );
          Q_UNUSED( dxfChar );
        }
        break;
      }
      default:
        /* RLZ: TODO */
        break;
    }
    QgsDebugMsg( QString( "ext data pos:%1.%2" ).arg( tmpExtDataBuf.getPosition() ).arg( tmpExtDataBuf.getBitPos() ) );
    delete[]tmpExtData;
    extDataSize = buf->getBitShort(); //BS
    QgsDebugMsg( QString( " ext data size:%1" ).arg( extDataSize ) );
  } //end parsing extData (EED)
  if ( version < DRW::AC1015 )  //14-
  {
    objSize = buf->getRawLong32();  //RL 32bits size in bits
  }
  numReactors = buf->getBitLong(); //BL

  QgsDebugMsg( QString( " objSize in bits size:%1, numReactors:%2" ).arg( objSize ).arg( numReactors ) );
  if ( version > DRW::AC1015 )  //2004+
  {
    xDictFlag = buf->getBit();
    QgsDebugMsg( QString( " xDictFlag:%1" ).arg( xDictFlag ) );
  }
  if ( version > DRW::AC1024 )  //2013+
  {
    duint8 bd = buf->getBit();
    QgsDebugMsg( QString( " Have binary data:%1" ).arg( bd ) );
    Q_UNUSED( bd );
  }
  return buf->isGood();
}

//! Class to handle dimstyle entries
/*!
*  Class to handle ldim style symbol table entries
*  @author Rallaz
*/
void DRW_Dimstyle::parseCode( int code, dxfReader *reader )
{
  switch ( code )
  {
    case 105:
      handle = reader->getHandleString();
      break;
    case 3:
      dimpost = reader->getUtf8String();
      break;
    case 4:
      dimapost = reader->getUtf8String();
      break;
    case 5:
      dimblk = reader->getUtf8String();
      break;
    case 6:
      dimblk1 = reader->getUtf8String();
      break;
    case 7:
      dimblk2 = reader->getUtf8String();
      break;
    case 40:
      dimscale = reader->getDouble();
      break;
    case 41:
      dimasz = reader->getDouble();
      break;
    case 42:
      dimexo = reader->getDouble();
      break;
    case 43:
      dimdli = reader->getDouble();
      break;
    case 44:
      dimexe = reader->getDouble();
      break;
    case 45:
      dimrnd = reader->getDouble();
      break;
    case 46:
      dimdle = reader->getDouble();
      break;
    case 47:
      dimtp = reader->getDouble();
      break;
    case 48:
      dimtm = reader->getDouble();
      break;
    case 49:
      dimfxl = reader->getDouble();
      break;
    case 140:
      dimtxt = reader->getDouble();
      break;
    case 141:
      dimcen = reader->getDouble();
      break;
    case 142:
      dimtsz = reader->getDouble();
      break;
    case 143:
      dimaltf = reader->getDouble();
      break;
    case 144:
      dimlfac = reader->getDouble();
      break;
    case 145:
      dimtvp = reader->getDouble();
      break;
    case 146:
      dimtfac = reader->getDouble();
      break;
    case 147:
      dimgap = reader->getDouble();
      break;
    case 148:
      dimaltrnd = reader->getDouble();
      break;
    case 71:
      dimtol = reader->getInt32();
      break;
    case 72:
      dimlim = reader->getInt32();
      break;
    case 73:
      dimtih = reader->getInt32();
      break;
    case 74:
      dimtoh = reader->getInt32();
      break;
    case 75:
      dimse1 = reader->getInt32();
      break;
    case 76:
      dimse2 = reader->getInt32();
      break;
    case 77:
      dimtad = reader->getInt32();
      break;
    case 78:
      dimzin = reader->getInt32();
      break;
    case 79:
      dimazin = reader->getInt32();
      break;
    case 170:
      dimalt = reader->getInt32();
      break;
    case 171:
      dimaltd = reader->getInt32();
      break;
    case 172:
      dimtofl = reader->getInt32();
      break;
    case 173:
      dimsah = reader->getInt32();
      break;
    case 174:
      dimtix = reader->getInt32();
      break;
    case 175:
      dimsoxd = reader->getInt32();
      break;
    case 176:
      dimclrd = reader->getInt32();
      break;
    case 177:
      dimclre = reader->getInt32();
      break;
    case 178:
      dimclrt = reader->getInt32();
      break;
    case 179:
      dimadec = reader->getInt32();
      break;
    case 270:
      dimunit = reader->getInt32();
      break;
    case 271:
      dimdec = reader->getInt32();
      break;
    case 272:
      dimtdec = reader->getInt32();
      break;
    case 273:
      dimaltu = reader->getInt32();
      break;
    case 274:
      dimalttd = reader->getInt32();
      break;
    case 275:
      dimaunit = reader->getInt32();
      break;
    case 276:
      dimfrac = reader->getInt32();
      break;
    case 277:
      dimlunit = reader->getInt32();
      break;
    case 278:
      dimdsep = reader->getInt32();
      break;
    case 279:
      dimtmove = reader->getInt32();
      break;
    case 280:
      dimjust = reader->getInt32();
      break;
    case 281:
      dimsd1 = reader->getInt32();
      break;
    case 282:
      dimsd2 = reader->getInt32();
      break;
    case 283:
      dimtolj = reader->getInt32();
      break;
    case 284:
      dimtzin = reader->getInt32();
      break;
    case 285:
      dimaltz = reader->getInt32();
      break;
    case 286:
      dimaltttz = reader->getInt32();
      break;
    case 287:
      dimfit = reader->getInt32();
      break;
    case 288:
      dimupt = reader->getInt32();
      break;
    case 289:
      dimatfit = reader->getInt32();
      break;
    case 290:
      dimfxlon = reader->getInt32();
      break;
    case 340:
      dimtxsty = reader->getUtf8String();
      break;
    case 341:
      dimldrblk = reader->getUtf8String();
      break;
    case 342:
      dimblk = reader->getUtf8String();
      break;
    case 343:
      dimblk1 = reader->getUtf8String();
      break;
    case 344:
      dimblk2 = reader->getUtf8String();
      break;
    default:
      DRW_TableEntry::parseCode( code, reader );
      break;
  }
}

bool DRW_Dimstyle::parseDwg( DRW::Version version, dwgBuffer *buf, duint32 bs )
{
  dwgBuffer sBuff = *buf;
  dwgBuffer *sBuf = buf;
  if ( version > DRW::AC1018 )  //2007+
  {
    sBuf = &sBuff; //separate buffer for strings
  }
  bool ret = DRW_TableEntry::parseDwg( version, buf, sBuf, bs );
  QgsDebugMsg( "***************************** parsing dimension style **************************************" );
  if ( !ret )
    return ret;
  name = sBuf->getVariableText( version, false );
  QgsDebugMsg( QString( "dimension style name: %1; remaining bytes:%2" ).arg( name.c_str() ).arg( buf->numRemainingBytes() ) );

  //    RS crc;   //RS */
  return buf->isGood();
}


//! Class to handle line type entries
/*!
*  Class to handle line type symbol table entries
*  @author Rallaz
*/
void DRW_LType::parseCode( int code, dxfReader *reader )
{
  switch ( code )
  {
    case 3:
      desc = reader->getUtf8String();
      break;
    case 73:
      size = reader->getInt32();
      RESERVE( path, size );
      break;
    case 40:
      length = reader->getDouble();
      break;
    case 49:
      path.push_back( reader->getDouble() );
      pathIdx++;
      break;
#if 0
    case 74:
      haveShape = reader->getInt32();
      break;
#endif
    default:
      DRW_TableEntry::parseCode( code, reader );
      break;
  }
}

//! Update line type
/*!
*  Update the size and length of line type according to the path
*  @author Rallaz
*/
/*TODO: control max length permitted */
void DRW_LType::update()
{
  double d = 0;
  size = path.size();
  for ( std::vector<double>::size_type i = 0;  i < size; i++ )
  {
    d += std::fabs( path.at( i ) );
  }
  length = d;
}

bool DRW_LType::parseDwg( DRW::Version version, dwgBuffer *buf, duint32 bs )
{
  dwgBuffer sBuff = *buf;
  dwgBuffer *sBuf = buf;
  if ( version > DRW::AC1018 )  //2007+
  {
    sBuf = &sBuff; //separate buffer for strings
  }
  bool ret = DRW_TableEntry::parseDwg( version, buf, sBuf, bs );
  QgsDebugMsg( "***************************** parsing line type *********************************************" );
  if ( !ret )
    return ret;
  name = sBuf->getVariableText( version, false );
  flags = buf->getBit() << 6;

  QgsDebugMsg( QString( "linetype name: %1; flags: 0x%2" ).arg( name.c_str() ).arg( flags, 0, 16 ) );

  if ( version > DRW::AC1018 )  //2007+
  {
  }
  else  //2004- //RLZ: verify in 2004, 2010 &2013
  {
    dint16 xrefindex = buf->getBitShort();
    QgsDebugMsg( QString( "xrefindex: %1" ).arg( xrefindex ) );
    Q_UNUSED( xrefindex );
  }

  duint8 xdep = buf->getBit();
  flags |= xdep << 4;
  desc = sBuf->getVariableText( version, false );
  length = buf->getBitDouble();
  char align = buf->getRawChar8();
  size = buf->getRawChar8();

  QgsDebugMsg( QString( "xdep: %1; flags:0x%2; desc:%3; pattern length:%4 align:%5; num dashes, size %6" )
               .arg( xdep ).arg( flags, 0, 16 ).arg( desc.c_str() ).arg( length ).arg( align ).arg( size )
             );
  Q_UNUSED( xdep );
  Q_UNUSED( align );

  bool haveStrArea = false;
  for ( std::vector<double>::size_type i = 0; i < size; i++ )
  {
    path.push_back( buf->getBitDouble() );
    /*int bs1 =*/
    buf->getBitShort();
    /*double d1= */
    buf->getRawDouble();
    /*double d2=*/
    buf->getRawDouble();
    /*double d3= */
    buf->getBitDouble();
    /*double d4= */
    buf->getBitDouble();
    int bs2 = buf->getBitShort();
    if ( ( bs2 & 2 ) != 0 ) haveStrArea = true;
  }

  QStringList l;
  for ( unsigned i = 0; i < path.size() ; i++ )
  {
    l << QStringLiteral( "%1" ).arg( path[i] );
  }
  QgsDebugMsg( QString( "path=%1 rem:%2" ).arg( l.join( " " ) ).arg( buf->numRemainingBytes() ) );

  if ( version < DRW::AC1021 ) //2004-
  {
    duint8 strarea[256];
    buf->getBytes( strarea, 256 );
    QgsDebugMsg( QString( "string area 256 bytes:\n%1" ).arg( reinterpret_cast<char *>( strarea ) ) );
  }
  else   //2007+
  {
    //first verify flag
    if ( haveStrArea )
    {
      duint8 strarea[512];
      buf->getBytes( strarea, 512 );
      QgsDebugMsg( QString( "string area 256 bytes:\n%1" ).arg( reinterpret_cast<char *>( strarea ) ) );
    }
    else
    {
      QgsDebugMsg( "string area 512 bytes not present" );
    }
  }

  if ( version > DRW::AC1021 )  //2007+ skip string area
  {
    QgsDebugMsgLevel( QString( "ltype end of object data pos 2010: %1; strBuf bpos 2007:%2" ).arg( buf->getPosition() ).arg( buf->getBitPos() ), 4 );
  }

  if ( version > DRW::AC1018 )  //2007+ skip string area
  {
    buf->setPosition( objSize >> 3 );
    buf->setBitPos( objSize & 7 );
  }

  if ( version > DRW::AC1021 )  //2007+ skip string area
  {
    QgsDebugMsgLevel( QString( "ltype start of handle data pos 2010: %1; strBuf bpos 2007:%2" ).arg( buf->getPosition() ).arg( buf->getBitPos() ), 4 );
  }

  dwgHandle ltControlH = buf->getHandle();
  QgsDebugMsgLevel( QString( "line type control handle: %1.%2 0x%3; rem:%4" )
                    .arg( ltControlH.code ).arg( ltControlH.size ).arg( ltControlH.ref, 0, 16 ).arg( buf->numRemainingBytes() ), 4 );
  parentHandle = ltControlH.ref;

  for ( int i = 0; i < numReactors; ++i )
  {
    dwgHandle reactorsH = buf->getHandle();
    QgsDebugMsgLevel( QString( "reactorsH control handle: %1.%2 0x%3" ).arg( reactorsH.code ).arg( reactorsH.size ).arg( reactorsH.ref, 0, 16 ), 4 );
  }
  if ( xDictFlag != 1 ) //linetype in 2004 seems not have XDicObjH or NULL handle
  {
    dwgHandle XDicObjH = buf->getHandle();
    QgsDebugMsg( QString( "XDicObjH control handle: %1.%2 0x%3 rem:%4" ).arg( XDicObjH.code ).arg( XDicObjH.size ).arg( XDicObjH.ref, 0, 16 ).arg( buf->numRemainingBytes() ) );
  }
  if ( size > 0 )
  {
    dwgHandle XRefH = buf->getHandle();
    QgsDebugMsg( QString( "XRefH control handle: %1.%2 0x%3" ).arg( XRefH.code ).arg( XRefH.size ).arg( XRefH.ref, 0, 16 ) );
    dwgHandle shpHandle = buf->getHandle();
    QgsDebugMsg( QString( "shapeFile handle: %1.%2 0x%3; rem:%4" ).arg( shpHandle.code ).arg( shpHandle.size ).arg( shpHandle.ref, 0, 16 ).arg( buf->numRemainingBytes() ) );
  }
  dwgHandle shpHandle = buf->getHandle();
  QgsDebugMsg( QString( "shapeFile +1 handle ??: %1.%2 0x%3; rem:%4" ).arg( shpHandle.code ).arg( shpHandle.size ).arg( shpHandle.ref, 0, 16 ).arg( buf->numRemainingBytes() ) );

//    RS crc;   //RS */
  return buf->isGood();
}

//! Class to handle layer entries
/*!
*  Class to handle layer symbol table entries
*  @author Rallaz
*/
void DRW_Layer::parseCode( int code, dxfReader *reader )
{
  switch ( code )
  {
    case 6:
      lineType = reader->getUtf8String();
      break;
    case 62:
      color = reader->getInt32();
      break;
    case 290:
      plotF = reader->getBool();
      break;
    case 370:
      lWeight = DRW_LW_Conv::dxfInt2lineWidth( reader->getInt32() );
      break;
    case 390:
      handlePlotS = reader->getString();
      break;
    case 347:
      handleMaterialS = reader->getString();
      break;
    case 420:
      color24 = reader->getInt32();
      break;
    case 440:
      transparency = reader->getInt32();
      break;
    default:
      DRW_TableEntry::parseCode( code, reader );
      break;
  }
}

bool DRW_Layer::parseDwg( DRW::Version version, dwgBuffer *buf, duint32 bs )
{
  dwgBuffer sBuff = *buf;
  dwgBuffer *sBuf = buf;
  if ( version > DRW::AC1018 )  //2007+
  {
    sBuf = &sBuff; //separate buffer for strings
  }
  bool ret = DRW_TableEntry::parseDwg( version, buf, sBuf, bs );
  QgsDebugMsg( "***************************** parsing layer *********************************************" );
  if ( !ret )
    return ret;
  name = sBuf->getVariableText( version, false );
  QgsDebugMsg( QString( "layer name: %1" ).arg( name.c_str() ) );

  flags |= buf->getBit() << 6;//layer have entity
  if ( version < DRW::AC1021 )  //2004-
  {
    int t = buf->getBitShort();
    QgsDebugMsg( QString( "xrefindex = %1" ).arg( t ) );
    Q_UNUSED( t );
    //dint16 xrefindex = buf->getBitShort();
  }
  flags |= buf->getBit() << 4;//is refx dependent
  if ( version < DRW::AC1015 )  //14-
  {
    flags |= buf->getBit(); //layer frozen
    /*flags |=*/
    buf->getBit(); //unused, negate the color
    flags |= buf->getBit() << 1;//frozen in new
    flags |= buf->getBit() << 3;//locked
  }
  if ( version > DRW::AC1014 )  //2000+
  {
    dint16 f = buf->getSBitShort();//bit2 are layer on
    QgsDebugMsg( QString( "flags 2000+: %1" ).arg( f ) );
    flags |= f & 0x0001; //layer frozen
    flags |= ( f >> 1 ) & 0x0002;//frozen in new
    flags |= ( f >> 1 ) & 0x0004;//locked
    plotF = ( f >> 4 ) & 0x0001;
    lWeight = DRW_LW_Conv::dwgInt2lineWidth( ( f & 0x03E0 ) >> 5 );
  }
  color = buf->getCmColor( version ); //BS or CMC //ok for R14 or negate
  QgsDebugMsg( QString( "entity color: %1" ).arg( color ) );

  if ( version > DRW::AC1018 )  //2007+ skip string area
  {
    buf->setPosition( objSize >> 3 );
    buf->setBitPos( objSize & 7 );
  }
  dwgHandle layerControlH = buf->getHandle();
  QgsDebugMsg( QString( "layer control handle: %1.%2 0x%3" ).arg( layerControlH.code ).arg( layerControlH.size ).arg( layerControlH.ref, 0, 16 ) );
  parentHandle = layerControlH.ref;

  if ( xDictFlag != 1 ) //linetype in 2004 seems not have XDicObjH or NULL handle
  {
    dwgHandle XDicObjH = buf->getHandle();
    QgsDebugMsg( QString( "XDicObjH control handle: %1.%2 0x%3" ).arg( XDicObjH.code ).arg( XDicObjH.size ).arg( XDicObjH.ref, 0, 16 ) );
  }
  dwgHandle XRefH = buf->getHandle();
  QgsDebugMsg( QString( "XRefH control handle: %1.%2 0x%3" ).arg( XRefH.code ).arg( XRefH.size ).arg( XRefH.ref, 0, 16 ) );

  if ( version > DRW::AC1014 )  //2000+
  {
    dwgHandle plotStyH = buf->getHandle();
    QgsDebugMsg( QString( "Plot style control handle: %1.%2 0x%3" ).arg( plotStyH.code ).arg( plotStyH.size ).arg( plotStyH.ref, 0, 16 ) );
    handlePlotS = DRW::toHexStr( plotStyH.ref );// std::string(plotStyH.ref);//RLZ: verify conversion
  }
  if ( version > DRW::AC1018 )  //2007+
  {
    dwgHandle materialH = buf->getHandle();
    QgsDebugMsg( QString( "Material control handle: %1.%2 0x%3" ).arg( materialH.code ).arg( materialH.size ).arg( materialH.ref, 0, 16 ) );
    handleMaterialS = DRW::toHexStr( materialH.ref );//RLZ: verify conversion
  }

  QgsDebugMsg( QString( "Remaining bytes: %1" ).arg( buf->numRemainingBytes() ) );

  //lineType handle
  lTypeH = buf->getHandle();
  QgsDebugMsg( QString( "line type handle: %1.%2 0x%3; rem:%4" ).arg( lTypeH.code ).arg( lTypeH.size ).arg( lTypeH.ref, 0, 16 ).arg( buf->numRemainingBytes() ) );

//    RS crc;   //RS */
  return buf->isGood();
}

bool DRW_Block_Record::parseDwg( DRW::Version version, dwgBuffer *buf, duint32 bs )
{
  dwgBuffer sBuff = *buf;
  dwgBuffer *sBuf = buf;
  if ( version > DRW::AC1018 )  //2007+
  {
    sBuf = &sBuff; //separate buffer for strings
  }
  bool ret = DRW_TableEntry::parseDwg( version, buf, sBuf, bs );

  QgsDebugMsg( "***************************** parsing block record ******************************************" );

  if ( !ret )
    return ret;
  duint32 insertCount = 0;//only 2000+
  duint32 objectCount = 0; //only 2004+

  name = sBuf->getVariableText( version, false );
  QgsDebugMsg( QString( "block record name: %1" ).arg( name.c_str() ) );

  flags |= buf->getBit() << 6;//referenced external reference, block code 70, bit 7 (64)
  if ( version > DRW::AC1018 )  //2007+
  {
  }
  else  //2004- //RLZ: verify in 2004, 2010 &2013
  {
    dint16 xrefindex = buf->getBitShort();
    QgsDebugMsg( QString( "xrefindex: %1" ).arg( xrefindex ) );
    Q_UNUSED( xrefindex );
  }
  flags |= buf->getBit() << 4;//is refx dependent, block code 70, bit 5 (16)
  flags |= buf->getBit(); //if is anonimous block (*U) block code 70, bit 1 (1)
  flags |= buf->getBit() << 1; //if block contains attdefs, block code 70, bit 2 (2)
  bool blockIsXref = buf->getBit(); //if is a Xref, block code 70, bit 3 (4)
  bool xrefOverlaid = buf->getBit(); //if is a overlaid Xref, block code 70, bit 4 (8)
  flags |= blockIsXref << 2; //if is a Xref, block code 70, bit 3 (4)
  flags |= xrefOverlaid << 3; //if is a overlaid Xref, block code 70, bit 4 (8)
  if ( version > DRW::AC1014 )  //2000+
  {
    flags |= buf->getBit() << 5; //if is a loaded Xref, block code 70, bit 6 (32)
  }
  QgsDebugMsg( QString( "flags: 0x%1" ).arg( flags, 0, 16 ) );

  if ( version > DRW::AC1015 )  //2004+ fails in 2007
  {
    objectCount = buf->getBitLong(); //Number of objects owned by this block
    RESERVE( entMap, objectCount );
  }
  basePoint.x = buf->getBitDouble();
  basePoint.y = buf->getBitDouble();
  basePoint.z = buf->getBitDouble();
  UTF8STRING path = sBuf->getVariableText( version, false );

  QgsDebugMsg( QString( "insertion point: %1,%2,%3; Xref path name:%4" )
               .arg( basePoint.x ).arg( basePoint.y ).arg( basePoint.z )
               .arg( path.c_str() )
             );

  if ( version > DRW::AC1014 )  //2000+
  {
    insertCount = 0;
    while ( duint8 i = buf->getRawChar8() != 0 )
      insertCount += i;
    UTF8STRING bkdesc = sBuf->getVariableText( version, false );

    QgsDebugMsg( QString( "Block description: %1" ).arg( bkdesc.c_str() ) );

    duint32 prevData = buf->getBitLong();
    for ( unsigned int j = 0; j < prevData; ++j )
      buf->getRawChar8();
  }
  if ( version > DRW::AC1018 )  //2007+
  {
    duint16 insUnits = buf->getBitShort();
    bool canExplode = buf->getBit(); //if block can be exploded
    duint8 bkScaling = buf->getRawChar8();

    DRW_UNUSED( insUnits );
    DRW_UNUSED( canExplode );
    DRW_UNUSED( bkScaling );
  }

  if ( version > DRW::AC1018 )  //2007+ skip string area
  {
    buf->setPosition( objSize >> 3 );
    buf->setBitPos( objSize & 7 );
  }

  dwgHandle blockControlH = buf->getHandle();
  QgsDebugMsg( QString( "block control handle: %1.%2 0x%3" ).arg( blockControlH.code ).arg( blockControlH.size ).arg( blockControlH.ref, 0, 16 ) );
  parentHandle = blockControlH.ref;

  for ( int i = 0; i < numReactors; i++ )
  {
    dwgHandle reactorH = buf->getHandle();
    QgsDebugMsgLevel( QString( "reactor handle %1: %2.%3 0x%4" ).arg( i ).arg( reactorH.code ).arg( reactorH.size ).arg( reactorH.ref, 0, 16 ), 5 );
  }
  if ( xDictFlag != 1 )  //R14+ //seems present in 2000
  {
    dwgHandle XDicObjH = buf->getHandle();
    QgsDebugMsg( QString( "XDicObj block control handle: %1.%2 0x%3" ).arg( XDicObjH.code ).arg( XDicObjH.size ).arg( XDicObjH.ref, 0, 16 ) );
  }
  if ( version != DRW::AC1021 )  //2007+ XDicObjH or NullH not present
  {
  }
  dwgHandle NullH = buf->getHandle();
  QgsDebugMsg( QString( "NullH control handle: %1.%2 0x%3" ).arg( NullH.code ).arg( NullH.size ).arg( NullH.ref, 0, 16 ) );
  dwgHandle blockH = buf->getOffsetHandle( handle );
  QgsDebugMsg( QString( "blockH handle: %1.%2 0x%3" ).arg( blockH.code ).arg( blockH.size ).arg( blockH.ref, 0, 16 ) );
  block = blockH.ref;

  if ( version > DRW::AC1015 )  //2004+
  {
    for ( unsigned int i = 0; i < objectCount; i++ )
    {
      dwgHandle entityH = buf->getHandle();
      QgsDebugMsgLevel( QString( "entityH handle %1: %2.%3 0x%4" ).arg( i ).arg( entityH.code ).arg( entityH.size ).arg( entityH.ref, 0, 16 ), 5 );
      entMap.push_back( entityH.ref );
    }
  }
  else  //2000-
  {
    if ( !blockIsXref && !xrefOverlaid )
    {
      dwgHandle firstH = buf->getHandle();
      QgsDebugMsgLevel( QString( "firstH entity handle %1.%2 0x%3" ).arg( firstH.code ).arg( firstH.size ).arg( firstH.ref, 0, 16 ), 5 );
      firstEH = firstH.ref;

      dwgHandle lastH = buf->getHandle();
      QgsDebugMsg( QString( "lastH entity handle %1.%2 0x%3" ).arg( lastH.code ).arg( lastH.size ).arg( lastH.ref, 0, 16 ) );
      lastEH = lastH.ref;
    }
  }
  QgsDebugMsg( QString( "Remaining bytes: %1" ).arg( buf->numRemainingBytes() ) );

  dwgHandle endBlockH = buf->getOffsetHandle( handle );
  QgsDebugMsg( QString( "endBlockH handle %1.%2 0x%3" ).arg( endBlockH.code ).arg( endBlockH.size ).arg( endBlockH.ref, 0, 16 ) );
  endBlock = endBlockH.ref;

  if ( version > DRW::AC1014 )  //2000+
  {
    for ( unsigned int i = 0; i < insertCount; i++ )
    {
      dwgHandle insertsH = buf->getHandle();
      QgsDebugMsgLevel( QString( "insertsH handle %1: %2.%3 0x%4" ).arg( i ).arg( insertsH.code ).arg( insertsH.size ).arg( insertsH.ref, 0, 16 ), 5 );
    }

    QgsDebugMsg( QString( "Remaining bytes: %1" ).arg( buf->numRemainingBytes() ) );
    dwgHandle layoutH = buf->getHandle();
    QgsDebugMsg( QString( "layoutH handle %1.%2 0x%3" ).arg( layoutH.code ).arg( layoutH.size ).arg( layoutH.ref, 0, 16 ) );
  }

//    RS crc;   //RS */
  return buf->isGood();
}

//! Class to handle text style entries
/*!
*  Class to handle text style symbol table entries
*  @author Rallaz
*/
void DRW_Textstyle::parseCode( int code, dxfReader *reader )
{
  switch ( code )
  {
    case 3:
      font = reader->getUtf8String();
      break;
    case 4:
      bigFont = reader->getUtf8String();
      break;
    case 40:
      height = reader->getDouble();
      break;
    case 41:
      width = reader->getDouble();
      break;
    case 50:
      oblique = reader->getDouble();
      break;
    case 42:
      lastHeight = reader->getDouble();
      break;
    case 71:
      genFlag = reader->getInt32();
      break;
    case 1071:
      fontFamily = reader->getInt32();
      break;
    default:
      DRW_TableEntry::parseCode( code, reader );
      break;
  }
}

bool DRW_Textstyle::parseDwg( DRW::Version version, dwgBuffer *buf, duint32 bs )
{
  dwgBuffer sBuff = *buf;
  dwgBuffer *sBuf = buf;
  if ( version > DRW::AC1018 )  //2007+
  {
    sBuf = &sBuff; //separate buffer for strings
  }
  bool ret = DRW_TableEntry::parseDwg( version, buf, sBuf, bs );

  QgsDebugMsg( "***************************** parsing text style *********************************************" );

  if ( !ret )
    return ret;
  name = sBuf->getVariableText( version, false );

  QgsDebugMsg( QString( "text style name: %1" ).arg( name.c_str() ) );

  flags |= buf->getBit() << 6;//style are referenced for a entity, style code 70, bit 7 (64)
  /*dint16 xrefindex =*/
  buf->getBitShort();
  flags |= buf->getBit() << 4; //is refx dependent, style code 70, bit 5 (16)
  flags |= buf->getBit() << 2; //vertical text, stile code 70, bit 3 (4)
  flags |= buf->getBit(); //if is a shape file instead of text, style code 70, bit 1 (1)
  height = buf->getBitDouble();
  width = buf->getBitDouble();
  oblique = buf->getBitDouble();
  genFlag = buf->getRawChar8();
  lastHeight = buf->getBitDouble();
  font = sBuf->getVariableText( version, false );
  bigFont = sBuf->getVariableText( version, false );
  if ( version > DRW::AC1018 )  //2007+ skip string area
  {
    buf->setPosition( objSize >> 3 );
    buf->setBitPos( objSize & 7 );
  }
  dwgHandle shpControlH = buf->getHandle();
  QgsDebugMsg( QString( "shpControlH handle %1.%2 0x%3" ).arg( shpControlH.code ).arg( shpControlH.size ).arg( shpControlH.ref, 0, 16 ) );
  parentHandle = shpControlH.ref;
  QgsDebugMsg( QString( "Remaining bytes: %1" ).arg( buf->numRemainingBytes() ) );

  if ( xDictFlag != 1 ) //linetype in 2004 seems not have XDicObjH or NULL handle
  {
    dwgHandle XDicObjH = buf->getHandle();
    QgsDebugMsg( QString( "XDicObjH handle %1.%2 0x%3" ).arg( XDicObjH.code ).arg( XDicObjH.size ).arg( XDicObjH.ref, 0, 16 ) );
    QgsDebugMsg( QString( "Remaining bytes: %1" ).arg( buf->numRemainingBytes() ) );
  }
  /*RLZ: fails verify this part*/
  dwgHandle XRefH = buf->getHandle();
  QgsDebugMsg( QString( "XRefH handle %1.%2 0x%3" ).arg( XRefH.code ).arg( XRefH.size ).arg( XRefH.ref, 0, 16 ) );
  QgsDebugMsg( QString( "Remaining bytes: %1" ).arg( buf->numRemainingBytes() ) );

  //    RS crc;   //RS */
  return buf->isGood();
}

//! Class to handle vport entries
/*!
*  Class to handle vport symbol table entries
*  @author Rallaz
*/
void DRW_Vport::parseCode( int code, dxfReader *reader )
{
  switch ( code )
  {
    case 10:
      lowerLeft.x = reader->getDouble();
      break;
    case 20:
      lowerLeft.y = reader->getDouble();
      break;
    case 11:
      UpperRight.x = reader->getDouble();
      break;
    case 21:
      UpperRight.y = reader->getDouble();
      break;
    case 12:
      center.x = reader->getDouble();
      break;
    case 22:
      center.y = reader->getDouble();
      break;
    case 13:
      snapBase.x = reader->getDouble();
      break;
    case 23:
      snapBase.y = reader->getDouble();
      break;
    case 14:
      snapSpacing.x = reader->getDouble();
      break;
    case 24:
      snapSpacing.y = reader->getDouble();
      break;
    case 15:
      gridSpacing.x = reader->getDouble();
      break;
    case 25:
      gridSpacing.y = reader->getDouble();
      break;
    case 16:
      viewDir.x = reader->getDouble();
      break;
    case 26:
      viewDir.y = reader->getDouble();
      break;
    case 36:
      viewDir.z = reader->getDouble();
      break;
    case 17:
      viewTarget.x = reader->getDouble();
      break;
    case 27:
      viewTarget.y = reader->getDouble();
      break;
    case 37:
      viewTarget.z = reader->getDouble();
      break;
    case 40:
      height = reader->getDouble();
      break;
    case 41:
      ratio = reader->getDouble();
      break;
    case 42:
      lensHeight = reader->getDouble();
      break;
    case 43:
      frontClip = reader->getDouble();
      break;
    case 44:
      backClip = reader->getDouble();
      break;
    case 50:
      snapAngle = reader->getDouble();
      break;
    case 51:
      twistAngle = reader->getDouble();
      break;
    case 71:
      viewMode = reader->getInt32();
      break;
    case 72:
      circleZoom = reader->getInt32();
      break;
    case 73:
      fastZoom = reader->getInt32();
      break;
    case 74:
      ucsIcon = reader->getInt32();
      break;
    case 75:
      snap = reader->getInt32();
      break;
    case 76:
      grid = reader->getInt32();
      break;
    case 77:
      snapStyle = reader->getInt32();
      break;
    case 78:
      snapIsopair = reader->getInt32();
      break;
    default:
      DRW_TableEntry::parseCode( code, reader );
      break;
  }
}

bool DRW_Vport::parseDwg( DRW::Version version, dwgBuffer *buf, duint32 bs )
{
  dwgBuffer sBuff = *buf;
  dwgBuffer *sBuf = buf;
  if ( version > DRW::AC1018 )  //2007+
  {
    sBuf = &sBuff; //separate buffer for strings
  }
  bool ret = DRW_TableEntry::parseDwg( version, buf, sBuf, bs );
  QgsDebugMsg( "***************************** parsing VPort ************************************************" );
  if ( !ret )
    return ret;
  name = sBuf->getVariableText( version, false );
  QgsDebugMsg( QString( "vport name: %1" ).arg( name.c_str() ) );

  flags |= buf->getBit() << 6;// code 70, bit 7 (64)
  if ( version < DRW::AC1021 ) //2004-
  {
    /*dint16 xrefindex =*/
    buf->getBitShort();
  }
  flags |= buf->getBit() << 4; //is refx dependent, style code 70, bit 5 (16)
  height = buf->getBitDouble();
  ratio = buf->getBitDouble();

  QgsDebugMsg( QString( "flags:%1 height:%2 ratio:%3" ).arg( flags ).arg( height ).arg( ratio ) );

  center = buf->get2RawDouble();
  QgsDebugMsg( QString( "view center:%1,%2,%3" ).arg( center.x ).arg( center.y ).arg( center.z ) );

  viewTarget.x = buf->getBitDouble();
  viewTarget.y = buf->getBitDouble();
  viewTarget.z = buf->getBitDouble();
  QgsDebugMsg( QString( "view target:%1,%2,%3" ).arg( viewTarget.x ).arg( viewTarget.y ).arg( viewTarget.z ) );

  viewDir.x = buf->getBitDouble();
  viewDir.y = buf->getBitDouble();
  viewDir.z = buf->getBitDouble();
  QgsDebugMsg( QString( "view dir:%1,%2,%3" ).arg( viewDir.x ).arg( viewDir.y ).arg( viewDir.z ) );

  twistAngle = buf->getBitDouble();
  lensHeight = buf->getBitDouble();
  frontClip = buf->getBitDouble();
  backClip = buf->getBitDouble();

  QgsDebugMsg( QString( "twistAngle:%1 lensHeight:%2 frontClip:%3 backClip:%4" ).arg( twistAngle ).arg( lensHeight ).arg( frontClip ).arg( backClip ) );

  viewMode = buf->getBit(); //view mode, code 71, bit 0 (1)
  viewMode |= buf->getBit() << 1; //view mode, code 71, bit 1 (2)
  viewMode |= buf->getBit() << 2; //view mode, code 71, bit 2 (4)
  viewMode |= buf->getBit() << 4; //view mode, code 71, bit 4 (16)
  if ( version > DRW::AC1014 ) //2000+
  {
    duint8 renderMode = buf->getRawChar8();
    QgsDebugMsg( QString( "renderMode: %1" ).arg( renderMode ) );
    Q_UNUSED( renderMode );

    if ( version > DRW::AC1018 ) //2007+
    {
      int t;
      double d;

      t = buf->getBit();
      QgsDebugMsg( QString( "use default lights:%1" ).arg( t ) );
      Q_UNUSED( t );
      t = buf->getRawChar8();
      QgsDebugMsg( QString( "default lighting type:%1" ).arg( t ) );
      d = buf->getBitDouble();
      QgsDebugMsg( QString( "brightness:%1" ).arg( d ) );
      Q_UNUSED( d );
      d = buf->getBitDouble();
      QgsDebugMsg( QString( "contrast:%1" ).arg( d ) );
      t = buf->getCmColor( version );
      QgsDebugMsg( QString( "color:%1" ).arg( t ) );
    }
  }
  lowerLeft = buf->get2RawDouble();
  QgsDebugMsg( QString( "lowerLeft:%1,%2,%3" ).arg( lowerLeft.x ).arg( lowerLeft.y ).arg( lowerLeft.z ) );
  UpperRight = buf->get2RawDouble();
  QgsDebugMsg( QString( "UpperRight:%1,%2,%3" ).arg( UpperRight.x ).arg( UpperRight.y ).arg( UpperRight.z ) );

  viewMode |= buf->getBit() << 3; //UCSFOLLOW, view mode, code 71, bit 3 (8)
  circleZoom = buf->getBitShort();
  fastZoom = buf->getBit();
  QgsDebugMsg( QString( "viewMode:%1 circleZoom:%2 fastZoom:%3" ).arg( viewMode ).arg( circleZoom ).arg( fastZoom ) );

  ucsIcon = buf->getBit(); //ucs Icon, code 74, bit 0 (1)
  ucsIcon |= buf->getBit() << 1; //ucs Icon, code 74, bit 1 (2)
  grid = buf->getBit();

  QgsDebugMsg( QString( "ucsIcon:%1 grid:%2" ).arg( ucsIcon ).arg( grid ) );

  gridSpacing = buf->get2RawDouble();
  QgsDebugMsg( QString( "grid Spacing:%1,%2,%3" ).arg( gridSpacing.x ).arg( gridSpacing.y ).arg( gridSpacing.z ) );

  snap = buf->getBit();
  snapStyle = buf->getBit();
  QgsDebugMsg( QString( "snap on/off:%1 snap Style:%2" ).arg( snap ).arg( snapStyle ) );
  snapIsopair = buf->getBitShort();
  snapAngle = buf->getBitDouble();
  QgsDebugMsg( QString( "snap Isopair::%1 snap Angle::%2" ).arg( snapIsopair ).arg( snapAngle ) );

  snapBase = buf->get2RawDouble();
  QgsDebugMsg( QString( "snap Base:%1,%2,%3" ).arg( snapBase.x ).arg( snapBase.y ).arg( snapBase.z ) );

  snapSpacing = buf->get2RawDouble();
  QgsDebugMsg( QString( "snap Base:%1,%2,%3" ).arg( snapSpacing.x ).arg( snapSpacing.y ).arg( snapSpacing.z ) );

  if ( version > DRW::AC1014 ) //2000+
  {
    int t;
    double d;
    double x, y, z;

    t = buf->getBit();
    QgsDebugMsg( QString( "Unknown %1" ).arg( t ) );
    Q_UNUSED( t );
    t = buf->getBit();
    QgsDebugMsg( QString( "UCS per Viewport: %1" ).arg( t ) );

    x = buf->getBitDouble();
    y = buf->getBitDouble();
    z = buf->getBitDouble();
    QgsDebugMsg( QString( "UCS origin:%1,%2,%3" ).arg( x, y, z ) );
    Q_UNUSED( x );
    Q_UNUSED( y );
    Q_UNUSED( z );

    x = buf->getBitDouble();
    y = buf->getBitDouble();
    z = buf->getBitDouble();
    QgsDebugMsg( QString( "UCS X Axis:%1,%2,%3" ).arg( x, y, z ) );

    x = buf->getBitDouble();
    y = buf->getBitDouble();
    z = buf->getBitDouble();
    QgsDebugMsg( QString( "UCS Y Axis:%1,%2,%3" ).arg( x, y, z ) );

    d = buf->getBitDouble();
    QgsDebugMsg( QString( "UCS elevation: %1" ).arg( d ) );
    Q_UNUSED( d );

    t = buf->getBitShort();
    QgsDebugMsg( QString( " UCS Orthographic type: %1" ).arg( t ) );

    if ( version > DRW::AC1018 ) //2007+
    {
      gridBehavior = buf->getBitShort();
      QgsDebugMsg( QString( "Grid behavior: %1" ).arg( gridBehavior ) );
      t = buf->getBitShort();
      QgsDebugMsg( QString( "Grid major: %1" ).arg( t ) );
    }
  }

  //common handles
  if ( version > DRW::AC1018 )  //2007+ skip string area
  {
    buf->setPosition( objSize >> 3 );
    buf->setBitPos( objSize & 7 );
  }
  dwgHandle vpControlH = buf->getHandle();
  QgsDebugMsg( QString( "vpControlH Handle: %1.%2 0x%3" ).arg( vpControlH.code ).arg( vpControlH.size ).arg( vpControlH.ref, 0, 16 ) );
  parentHandle = vpControlH.ref;
  QgsDebugMsg( QString( "Remaining bytes: %1" ).arg( buf->numRemainingBytes() ) );

  if ( xDictFlag != 1 )
  {
    dwgHandle XDicObjH = buf->getHandle();
    QgsDebugMsg( QString( "XDicObjH Handle: %1.%2 0x%3" ).arg( XDicObjH.code ).arg( XDicObjH.size ).arg( XDicObjH.ref, 0, 16 ) );
    QgsDebugMsg( QString( "Remaining bytes: %1" ).arg( buf->numRemainingBytes() ) );
  }
  /*RLZ: fails verify this part*/
  dwgHandle XRefH = buf->getHandle();
  QgsDebugMsg( QString( "XRefH Handle: %1.%2 0x%3" ).arg( XRefH.code ).arg( XRefH.size ).arg( XRefH.ref, 0, 16 ) );

  if ( version > DRW::AC1014 ) //2000+
  {
    QgsDebugMsg( QString( "Remaining bytes: %1" ).arg( buf->numRemainingBytes() ) );

    if ( version > DRW::AC1018 ) //2007+
    {
      dwgHandle bkgrdH = buf->getHandle();
      QgsDebugMsg( QString( "background Handle: %1.%2 0x%3" ).arg( bkgrdH.code ).arg( bkgrdH.size ).arg( bkgrdH.ref, 0, 16 ) );
      QgsDebugMsg( QString( "Remaining bytes: %1" ).arg( buf->numRemainingBytes() ) );

      dwgHandle visualStH = buf->getHandle();
      QgsDebugMsg( QString( "visual style Handle: %1.%2 0x%3" ).arg( visualStH.code ).arg( visualStH.size ).arg( visualStH.ref, 0, 16 ) );
      QgsDebugMsg( QString( "Remaining bytes: %1" ).arg( buf->numRemainingBytes() ) );

      dwgHandle sunH = buf->getHandle();
      QgsDebugMsg( QString( "sun Handle: %1.%2 0x%3" ).arg( sunH.code ).arg( sunH.size ).arg( sunH.ref, 0, 16 ) );
      QgsDebugMsg( QString( "Remaining bytes: %1" ).arg( buf->numRemainingBytes() ) );
    }

    dwgHandle namedUCSH = buf->getHandle();
    QgsDebugMsg( QString( "name UCS handle: %1.%2 0x%3" ).arg( namedUCSH.code ).arg( namedUCSH.size ).arg( namedUCSH.ref, 0, 16 ) );
    QgsDebugMsg( QString( "Remaining bytes: %1" ).arg( buf->numRemainingBytes() ) );

    dwgHandle baseUCSH = buf->getHandle();
    QgsDebugMsg( QString( "base UCS handle: %1.%2 0x%3" ).arg( baseUCSH.code ).arg( baseUCSH.size ).arg( baseUCSH.ref, 0, 16 ) );
  }

  QgsDebugMsg( QString( "Remaining bytes: %1" ).arg( buf->numRemainingBytes() ) );

  //    RS crc;   //RS */
  return buf->isGood();
}

void DRW_ImageDef::parseCode( int code, dxfReader *reader )
{
  switch ( code )
  {
    case 1:
      name = reader->getUtf8String();
      break;
    case 5:
      handle = reader->getHandleString();
      break;
    case 10:
      u = reader->getDouble();
      break;
    case 20:
      v = reader->getDouble();
      break;
    case 11:
      up = reader->getDouble();
      break;
    case 12:
      vp = reader->getDouble();
      break;
    case 21:
      vp = reader->getDouble();
      break;
    case 280:
      loaded = reader->getInt32();
      break;
    case 281:
      resolution = reader->getInt32();
      break;
    default:
      break;
  }
}

bool DRW_ImageDef::parseDwg( DRW::Version version, dwgBuffer *buf, duint32 bs )
{
  dwgBuffer sBuff = *buf;
  dwgBuffer *sBuf = buf;
  if ( version > DRW::AC1018 )  //2007+
  {
    sBuf = &sBuff; //separate buffer for strings
  }
  bool ret = DRW_TableEntry::parseDwg( version, buf, sBuf, bs );

  QgsDebugMsg( "***************************** parsing Image Def *********************************************" );
  if ( !ret )
    return ret;

  dint32 imgVersion = buf->getBitLong();
  QgsDebugMsg( QString( "class Version:%1" ).arg( imgVersion ) );
  Q_UNUSED( imgVersion );

  DRW_Coord size = buf->get2RawDouble();
  DRW_UNUSED( size );//RLZ: temporary, complete API

  name = sBuf->getVariableText( version, false );
  QgsDebugMsg( QString( "appId name:%1" ).arg( name.c_str() ) );

  loaded = buf->getBit();
  resolution = buf->getRawChar8();
  up = buf->getRawDouble();
  vp = buf->getRawDouble();

  dwgHandle parentH = buf->getHandle();
  QgsDebugMsg( QString( "parentH Handle: %1.%2 0x%3" ).arg( parentH.code ).arg( parentH.size ).arg( parentH.ref, 0, 16 ) );
  parentHandle = parentH.ref;

  QgsDebugMsg( QString( "Remaining bytes: %1" ).arg( buf->numRemainingBytes() ) );

  //RLZ: Reactors handles
  if ( xDictFlag != 1 )
  {
    dwgHandle XDicObjH = buf->getHandle();
    QgsDebugMsg( QString( "XDicObjH Handle: %1.%2 0x%3" ).arg( XDicObjH.code ).arg( XDicObjH.size ).arg( XDicObjH.ref, 0, 16 ) );
    QgsDebugMsg( QString( "Remaining bytes: %1" ).arg( buf->numRemainingBytes() ) );
  }
  /*RLZ: fails verify this part*/
  dwgHandle XRefH = buf->getHandle();
  QgsDebugMsg( QString( "XRefH Handle: %1.%2 0x%3" ).arg( XRefH.code ).arg( XRefH.size ).arg( XRefH.ref, 0, 16 ) );
  QgsDebugMsg( QString( "Remaining bytes: %1" ).arg( buf->numRemainingBytes() ) );

  //    RS crc;   //RS */
  return buf->isGood();
}

bool DRW_AppId::parseDwg( DRW::Version version, dwgBuffer *buf, duint32 bs )
{
  dwgBuffer sBuff = *buf;
  dwgBuffer *sBuf = buf;
  if ( version > DRW::AC1018 )  //2007+
  {
    sBuf = &sBuff; //separate buffer for strings
  }
  bool ret = DRW_TableEntry::parseDwg( version, buf, sBuf, bs );
  QgsDebugMsg( "***************************** parsing app Id *********************************************" );
  if ( !ret )
    return ret;

  name = sBuf->getVariableText( version, false );

  QgsDebugMsg( QString( "appId name:%1" ).arg( name.c_str() ) );

  flags |= buf->getBit() << 6;// code 70, bit 7 (64)
  /*dint16 xrefindex =*/
  buf->getBitShort();
  flags |= buf->getBit() << 4; //is refx dependent, style code 70, bit 5 (16)
  duint8 unknown = buf->getRawChar8(); // unknown code 71
  QgsDebugMsg( QString( "unknown code 71:%1" ).arg( unknown ) );
  Q_UNUSED( unknown );

  if ( version > DRW::AC1018 )  //2007+ skip string area
  {
    buf->setPosition( objSize >> 3 );
    buf->setBitPos( objSize & 7 );
  }

  dwgHandle appIdControlH = buf->getHandle();
  QgsDebugMsg( QString( "appIdControlH Handle: %1.%2 0x%3" ).arg( appIdControlH.code ).arg( appIdControlH.size ).arg( appIdControlH.ref, 0, 16 ) );
  parentHandle = appIdControlH.ref;

  QgsDebugMsg( QString( "Remaining bytes: %1" ).arg( buf->numRemainingBytes() ) );

  if ( xDictFlag != 1 ) //linetype in 2004 seems not have XDicObjH or NULL handle
  {
    dwgHandle XDicObjH = buf->getHandle();
    QgsDebugMsg( QString( "XDicObjH Handle: %1.%2 0x%3" ).arg( XDicObjH.code ).arg( XDicObjH.size ).arg( XDicObjH.ref, 0, 16 ) );
    QgsDebugMsg( QString( "Remaining bytes: %1" ).arg( buf->numRemainingBytes() ) );
  }
  /*RLZ: fails verify this part*/
  dwgHandle XRefH = buf->getHandle();
  QgsDebugMsg( QString( "XRefH Handle: %1.%2 0x%3" ).arg( XRefH.code ).arg( XRefH.size ).arg( XRefH.ref, 0, 16 ) );
  QgsDebugMsg( QString( "Remaining bytes: %1" ).arg( buf->numRemainingBytes() ) );

  //    RS crc;   //RS */
  return buf->isGood();
}
