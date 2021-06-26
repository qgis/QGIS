
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

#include <cstdlib>
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>

#include "dwgreader15.h"
#include "drw_textcodec.h"

#include "../libdwgr.h"

#include "qgslogger.h"

bool dwgReader15::readMetaData()
{
  QgsDebugMsg( "Entering." );
  version = parent->getVersion();
  decoder.setVersion( version, false );

  if ( ! fileBuf->setPosition( 13 ) )
    return false;
  previewImagePos = fileBuf->getRawLong32();
  QgsDebugMsg( QString( "previewImagePos (seekerImageData) = %1" ).arg( previewImagePos ) );
  /* MEASUREMENT system variable 2 bytes*/
  duint16 meas = fileBuf->getRawShort16();
  QgsDebugMsg( QString( "MEASUREMENT (0 = English, 1 = Metric)= %1" ).arg( meas ) );
  Q_UNUSED( meas );
  duint16 cp = fileBuf->getRawShort16();
  QgsDebugMsg( QString( "codepage= %1" ).arg( cp ) );

  if ( cp == 29 ) //TODO RLZ: locate wath code page and correct this
    decoder.setCodePage( "ANSI_1252", false );
  if ( cp == 30 )
    decoder.setCodePage( "ANSI_1252", false );
  return true;
}

bool dwgReader15::readFileHeader()
{
  QgsDebugMsg( "Entering." );
  bool ret = true;
  if ( ! fileBuf->setPosition( 21 ) )
    return false;
  duint32 count = fileBuf->getRawLong32();
  QgsDebugMsg( QString( "count records=%1" ).arg( count ) );

  for ( unsigned int i = 0; i < count; i++ )
  {
    duint8 rec = fileBuf->getRawChar8();
    duint32 address = fileBuf->getRawLong32();
    duint32 size = fileBuf->getRawLong32();
    dwgSectionInfo si;
    si.Id = rec;
    si.size = size;
    si.address = address;
    if ( rec == 0 )
    {
      QgsDebugMsg( QString( "Section HEADERS address=%1 size=%2" ).arg( address ).arg( size ) );
      sections[secEnum::HEADER] = si;
    }
    else if ( rec == 1 )
    {
      QgsDebugMsg( QString( "Section CLASSES address=%1 size=%2" ).arg( address ).arg( size ) );
      sections[secEnum::CLASSES] = si;
    }
    else if ( rec == 2 )
    {
      QgsDebugMsg( QString( "Section OBJECTS (handles) address=%1 size=%2" ).arg( address ).arg( size ) );
      sections[secEnum::HANDLES] = si;
    }
    else if ( rec == 3 )
    {
      QgsDebugMsg( QString( "Section UNKNOWN address=%1 size=%2" ).arg( address ).arg( size ) );
      sections[secEnum::UNKNOWNS] = si;
    }
    else if ( rec == 4 )
    {
      QgsDebugMsg( QString( "Section R14DATA (AcDb::Template) address=%1 size=%2" ).arg( address ).arg( size ) );
      sections[secEnum::TEMPLATE] = si;
    }
    else if ( rec == 5 )
    {
      QgsDebugMsg( QString( "Section R14REC5 (AcDb::AuxHeader) address=%1 size=%2" ).arg( address ).arg( size ) );
      sections[secEnum::AUXHEADER] = si;
    }
    else
    {
      std::cerr << "\nUnsupported section number\n";
    }
  }
  if ( ! fileBuf->isGood() )
    return false;

  QgsDebugMsg( QString( "position after read section locator records=%1, bit are=%2" ).arg( fileBuf->getPosition() ).arg( fileBuf->getBitPos() ) );
  duint32 ckcrc = fileBuf->crc8( 0, 0, fileBuf->getPosition() );
  QgsDebugMsg( QString( "file header crc8 0 result=%1" ).arg( ckcrc ) );
  switch ( count )
  {
    case 3:
      ckcrc = ckcrc ^ 0xA598;
      break;
    case 4:
      ckcrc = ckcrc ^ 0x8101;
      break;
    case 5:
      ckcrc = ckcrc ^ 0x3CC4;
      break;
    case 6:
      ckcrc = ckcrc ^ 0x8461;
  }

  int headercrc = fileBuf->getRawShort16();
  QgsDebugMsg( QString( "file header crc8 xor result=%1, file header CRC=%2" ).arg( ckcrc ).arg( headercrc ) );
  Q_UNUSED( headercrc );

  checkSentinel( fileBuf, secEnum::FILEHEADER, false );

  QgsDebugMsg( QString( "position after read file header sentinel=%1, bit pos=%2" ).arg( fileBuf->getPosition() ).arg( fileBuf->getBitPos() ) );

  QgsDebugMsg( "Leaving." );

  return ret;
}

bool dwgReader15::readDwgHeader( DRW_Header &hdr )
{
  QgsDebugMsg( "Entering." );

  dwgSectionInfo si = sections[secEnum::HEADER];
  if ( si.Id < 0 )//not found, ends
    return false;
  if ( !fileBuf->setPosition( si.address ) )
    return false;
  duint8 *tmpByteStr = new duint8[si.size];
  fileBuf->getBytes( tmpByteStr, si.size );
  dwgBuffer buff( tmpByteStr, si.size, &decoder );

  QgsDebugMsg( "checksentinel" );
  checkSentinel( &buff, secEnum::HEADER, true );
  bool ret = dwgReader::readDwgHeader( hdr, &buff, &buff );
  delete[]tmpByteStr;
  return ret;
}


bool dwgReader15::readDwgClasses()
{
  QgsDebugMsg( "Entering." );

  dwgSectionInfo si = sections[secEnum::CLASSES];
  if ( si.Id < 0 )//not found, ends
    return false;
  if ( !fileBuf->setPosition( si.address ) )
    return false;

  QgsDebugMsg( "classes section sentinel" );
  checkSentinel( fileBuf, secEnum::CLASSES, true );

  duint32 size = fileBuf->getRawLong32();
  if ( size != ( si.size - 38 ) )
  {
    QgsDebugMsg( QString( "size is %1 and secSize - 38 are %1" ).arg( size ).arg( si.size - 38 ) );
  }
  duint8 *tmpByteStr = new duint8[size];
  fileBuf->getBytes( tmpByteStr, size );
  dwgBuffer buff( tmpByteStr, size, &decoder );
  size--; //reduce 1 byte instead of check pos + bitPos
  while ( size > buff.getPosition() )
  {
    DRW_Class *cl = new DRW_Class();
    cl->parseDwg( version, &buff, &buff );
    classesmap[cl->classNum] = cl;
  }
  int crc = fileBuf->getRawShort16();
  QgsDebugMsg( QString( "crc=%1, classes section end sentinel" ).arg( crc ) );
  Q_UNUSED( crc );

  checkSentinel( fileBuf, secEnum::CLASSES, false );

  bool ret = buff.isGood();
  delete[]tmpByteStr;
  return ret;
}

bool dwgReader15::readDwgHandles()
{
  QgsDebugMsg( "Entering." );
  dwgSectionInfo si = sections[secEnum::HANDLES];
  if ( si.Id < 0 )//not found, ends
    return false;

  bool ret = dwgReader::readDwgHandles( fileBuf, si.address, si.size );
  return ret;
}

/*********** objects ************************/

/**
 * Reads all the object referenced in the object map section of the DWG file
 * (using their object file offsets)
 */
bool dwgReader15::readDwgTables( DRW_Header &hdr )
{
  bool ret = dwgReader::readDwgTables( hdr, fileBuf );

  return ret;
}

/**
 * Reads all the object referenced in the object map section of the DWG file
 * (using their object file offsets)
 */
bool dwgReader15::readDwgBlocks( DRW_Interface &intfa )
{
  bool ret = true;
  ret = dwgReader::readDwgBlocks( intfa, fileBuf );
  return ret;
}

