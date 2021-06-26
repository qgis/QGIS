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


#include "libdwgr.h"

#include <fstream>
#include <algorithm>
#include <sstream>
#include "intern/drw_dbg.h"

#include "qgslogger.h"

#include "intern/drw_textcodec.h"
#include "intern/dwgreader.h"
#include "intern/dwgreader15.h"
#include "intern/dwgreader18.h"
#include "intern/dwgreader21.h"
#include "intern/dwgreader24.h"
#include "intern/dwgreader27.h"

#define FIRSTHANDLE 48

#if 0
enum sections
{
  secUnknown,
  secHeader,
  secTables,
  secBlocks,
  secEntities,
  secObjects
};
#endif

dwgR::dwgR( const char *name )
  : version( DRW::UNKNOWNV )
  , error( DRW::BAD_NONE )
  , fileName( name )
  , applyExt( false )
  , iface( nullptr )
  , reader( nullptr )
#if 0
  , writer( nullptr )
#endif
{
  DRW_DBGSL( DRW_dbg::none );
}

dwgR::~dwgR()
{
  delete reader;
}

void dwgR::setDebug( DRW::DBG_LEVEL lvl )
{
  switch ( lvl )
  {
    case DRW::debug:
      DRW_DBGSL( DRW_dbg::debug );
      break;
    default:
      DRW_DBGSL( DRW_dbg::none );
  }
}

/*reads metadata and loads image preview*/
bool dwgR::getPreview()
{
  std::ifstream filestr;

  bool isOk = openFile( &filestr );
  if ( !isOk )
    return false;

  isOk = reader->readMetaData();
  if ( isOk )
  {
    isOk = reader->readPreview();
  }
  else
    error = DRW::BAD_READ_METADATA;

  filestr.close();

  delete reader;
  reader = nullptr;

  return isOk;
}

bool dwgR::testReader()
{
  bool isOk = false;

  std::ifstream filestr;
  filestr.open( fileName.c_str(), std::ios_base::in | std::ios::binary );
  if ( !filestr.is_open() || !filestr.good() )
  {
    error = DRW::BAD_OPEN;
    return isOk;
  }

  dwgBuffer fileBuf( &filestr );
  duint8 *tmpStrData = new duint8[fileBuf.size()];
  fileBuf.getBytes( tmpStrData, fileBuf.size() );
  dwgBuffer dataBuf( tmpStrData, fileBuf.size() );
  fileBuf.setPosition( 0 );

  QgsDebugMsg( QString( "filebuf size:%1, dataBuf size:%2, filebuf pos:%3, dataBuf pos:%4, filebuf bitpos:%5, dataBuf bitpos:%6, filebuf first byte:0x%7, databuf first byte:0x%8" )
               .arg( fileBuf.size() ).arg( dataBuf.size() )
               .arg( fileBuf.getPosition() ).arg( dataBuf.getPosition() )
               .arg( fileBuf.getBitPos() ).arg( dataBuf.getBitPos() )
               .arg( fileBuf.getRawChar8(), 0, 16 )
               .arg( dataBuf.getRawChar8(), 0, 16 )
             );

  fileBuf.setBitPos( 4 );
  dataBuf.setBitPos( 4 );

  QgsDebugMsg( QString( "filebuf first byte:0x%1, databuf first byte:0x%2, filebuf pos:%3, databuf pos:%4, filebuf bitpos:%5, databuf bitpos:%6" )
               .arg( fileBuf.getRawChar8(), 0, 16 )
               .arg( dataBuf.getRawChar8(), 0, 16 )
               .arg( fileBuf.getPosition() ).arg( dataBuf.getPosition() )
               .arg( fileBuf.getBitPos() ).arg( dataBuf.getBitPos() )
             );

  fileBuf.setBitPos( 6 );
  dataBuf.setBitPos( 6 );

  QgsDebugMsg( QString( "filebuf pos:%1, databuf pos:%1, filebuf bitpos:%3, databuf bitpos:%4, filebuf first byte:%5, databuf first byte:%6" )
               .arg( fileBuf.getPosition() ).arg( dataBuf.getPosition() )
               .arg( fileBuf.getBitPos() ).arg( dataBuf.getBitPos() )
               .arg( fileBuf.getRawChar8(), 0, 16 )
               .arg( dataBuf.getRawChar8(), 0, 16 )
             );

  fileBuf.setBitPos( 0 );
  dataBuf.setBitPos( 0 );

  QgsDebugMsg( QString( "filebuf first byte:0x%1, databuf first byte:0x%2, filebuf pos:%3, databuf pos:%4, filebuf bitpos:%5, databuf bitpos:%6" )
               .arg( fileBuf.getRawChar8(), 0, 16 )
               .arg( dataBuf.getRawChar8(), 0, 16 )
               .arg( fileBuf.getPosition() ).arg( dataBuf.getPosition() )
               .arg( fileBuf.getBitPos() ).arg( dataBuf.getBitPos() )
             );

  delete [] tmpStrData;
  filestr.close();

  return isOk;
}

/*start reading dwg file header and, if can read it, continue reading all*/
bool dwgR::read( DRW_Interface *interface_, bool ext )
{
  applyExt = ext;
  iface = interface_;

#if 0
  testReader();
  return false;
#endif

  std::ifstream filestr;
  bool isOk = openFile( &filestr );
  if ( !isOk )
    return false;

  isOk = reader->readMetaData();
  if ( isOk )
  {
    isOk = reader->readFileHeader();
    if ( isOk )
    {
      isOk = processDwg();
    }
    else
      error = DRW::BAD_READ_FILE_HEADER;
  }
  else
    error = DRW::BAD_READ_METADATA;

  filestr.close();

  delete reader;
  reader = nullptr;

  return isOk;
}

/* Open the file and stores it in filestr, install the correct reader version.
 * If fail opening file, error are set as DRW::BAD_OPEN
 * If not are DWG or are unsupported version, error are set as DRW::BAD_VERSION
 * and closes filestr.
 * Return true on succeed or false on fail
*/
bool dwgR::openFile( std::ifstream *filestr )
{
  QgsDebugMsg( "Entering." );

  bool isOk = false;

  filestr->open( fileName.c_str(), std::ios_base::in | std::ios::binary );
  if ( !filestr->is_open() || !filestr->good() )
  {
    error = DRW::BAD_OPEN;
    return isOk;
  }

  char line[7];
  filestr->read( line, 6 );
  line[6] = '\0';

  QgsDebugMsg( QString( "line version:%1" ).arg( line ) );

  if ( strcmp( line, "AC1006" ) == 0 )
    version = DRW::AC1006;
  else if ( strcmp( line, "AC1009" ) == 0 )
  {
    version = DRW::AC1009;
//        reader = new dwgReader09(&filestr, this);
  }
  else if ( strcmp( line, "AC1012" ) == 0 )
  {
    version = DRW::AC1012;
    reader = new dwgReader15( filestr, this );
  }
  else if ( strcmp( line, "AC1014" ) == 0 )
  {
    version = DRW::AC1014;
    reader = new dwgReader15( filestr, this );
  }
  else if ( strcmp( line, "AC1015" ) == 0 )
  {
    version = DRW::AC1015;
    reader = new dwgReader15( filestr, this );
  }
  else if ( strcmp( line, "AC1018" ) == 0 )
  {
    version = DRW::AC1018;
    reader = new dwgReader18( filestr, this );
  }
  else if ( strcmp( line, "AC1021" ) == 0 )
  {
    version = DRW::AC1021;
    reader = new dwgReader21( filestr, this );
  }
  else if ( strcmp( line, "AC1024" ) == 0 )
  {
    version = DRW::AC1024;
    reader = new dwgReader24( filestr, this );
  }
  else if ( strcmp( line, "AC1027" ) == 0 )
  {
    version = DRW::AC1027;
    reader = new dwgReader27( filestr, this );
  }
  else
    version = DRW::UNKNOWNV;

  if ( !reader )
  {
    error = DRW::BAD_VERSION;
    filestr->close();
  }
  else
    isOk = true;

  return isOk;
}

/********* Reader Process *********/

bool dwgR::processDwg()
{
  QgsDebugMsg( "Entering." );

  bool ret;
  bool ret2;
  DRW_Header hdr;
  ret = reader->readDwgHeader( hdr );
  if ( !ret )
  {
    error = DRW::BAD_READ_HEADER;
  }

  ret2 = reader->readDwgClasses();
  if ( ret && !ret2 )
  {
    error = DRW::BAD_READ_CLASSES;
    ret = ret2;
  }

  ret2 = reader->readDwgHandles();
  if ( ret && !ret2 )
  {
    error = DRW::BAD_READ_HANDLES;
    ret = ret2;
  }

  ret2 = reader->readDwgTables( hdr );
  if ( ret && !ret2 )
  {
    error = DRW::BAD_READ_TABLES;
    ret = ret2;
  }

  iface->addHeader( &hdr );

  for ( std::map<duint32, DRW_LType *>::iterator it = reader->ltypemap.begin(); it != reader->ltypemap.end(); ++it )
  {
    DRW_LType *lt = it->second;
    iface->addLType( const_cast<DRW_LType &>( *lt ) );
  }
  for ( std::map<duint32, DRW_Layer *>::iterator it = reader->layermap.begin(); it != reader->layermap.end(); ++it )
  {
    DRW_Layer *ly = it->second;
    iface->addLayer( const_cast<DRW_Layer &>( *ly ) );
  }

  for ( std::map<duint32, DRW_Textstyle *>::iterator it = reader->stylemap.begin(); it != reader->stylemap.end(); ++it )
  {
    DRW_Textstyle *ly = it->second;
    iface->addTextStyle( const_cast<DRW_Textstyle &>( *ly ) );
  }

  for ( std::map<duint32, DRW_Dimstyle *>::iterator it = reader->dimstylemap.begin(); it != reader->dimstylemap.end(); ++it )
  {
    DRW_Dimstyle *ly = it->second;
    iface->addDimStyle( const_cast<DRW_Dimstyle &>( *ly ) );
  }

  for ( std::map<duint32, DRW_Vport *>::iterator it = reader->vportmap.begin(); it != reader->vportmap.end(); ++it )
  {
    DRW_Vport *ly = it->second;
    iface->addVport( const_cast<DRW_Vport &>( *ly ) );
  }

  for ( std::map<duint32, DRW_AppId *>::iterator it = reader->appIdmap.begin(); it != reader->appIdmap.end(); ++it )
  {
    DRW_AppId *ly = it->second;
    iface->addAppId( const_cast<DRW_AppId &>( *ly ) );
  }

  ret2 = reader->readDwgBlocks( *iface );
  if ( ret && !ret2 )
  {
    error = DRW::BAD_READ_BLOCKS;
    ret = ret2;
  }

  ret2 = reader->readDwgEntities( *iface );
  if ( ret && !ret2 )
  {
    error = DRW::BAD_READ_ENTITIES;
    ret = ret2;
  }

  ret2 = reader->readDwgObjects( *iface );
  if ( ret && !ret2 )
  {
    error = DRW::BAD_READ_OBJECTS;
    ret = ret2;
  }

  return ret;
}
