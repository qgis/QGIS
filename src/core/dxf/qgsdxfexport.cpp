/***************************************************************************
                         qgsdxfexport.cpp
                         ----------------
    begin                : September 2013
    copyright            : (C) 2013 by Marco Hugentobler
    email                : marco at sourcepole dot ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

// Specs:
// AutoCAD 2000: http://www.autodesk.com/techpubs/autocad/acad2000/dxf/
// AutoCAD 2002: http://www.autodesk.com/techpubs/autocad/dxf/dxf2002.pdf
// AutoCAD 2004: http://atrey.karlin.mff.cuni.cz/projekty/vrr/doc/dxf14.pdf
// AutoCAD 2006: http://images.autodesk.com/adsk/files/dxf_format.pdf
// AutoCAD 2008: http://images.autodesk.com/adsk/files/acad_dxf0.pdf
// AutoCAD 2009: http://images.autodesk.com/adsk/files/acad_dxf.pdf
// AutoCAD 2011: http://images.autodesk.com/adsk/files/acad_dxf2.pdf
// AutoCAD 2012: http://images.autodesk.com/adsk/files/autocad_2012_pdf_dxf-reference_enu.pdf
// AutoCAD 2014: http://images.autodesk.com/adsk/files/autocad_2014_pdf_dxf_reference_enu.pdf

#include "qgsdxfexport.h"
#include "qgsdxfpallabeling.h"
#include "qgsvectordataprovider.h"
#include "qgspoint.h"
#include "qgsrendererv2.h"
#include "qgssymbollayerv2.h"
#include "qgsfillsymbollayerv2.h"
#include "qgslinesymbollayerv2.h"
#include "qgsvectorlayer.h"
#include "qgsmaplayerregistry.h"

#include <QIODevice>

#define DXF_HANDSEED 100
#define DXF_HANDMAX 9999999
#define DXF_HANDPLOTSTYLE 0xf

// dxf color palette
int QgsDxfExport::mDxfColors[][3] =
{
  { 255, 255, 255 },
  { 255, 0, 0 },
  { 255, 255, 0 },
  { 0, 255, 0 },
  { 0, 255, 255 },
  { 0, 0, 255 },
  { 255, 0, 255 },
  { 0, 0, 0 },
  { 128, 128, 128 },
  { 192, 192, 192 },
  { 255, 0, 0 },
  { 255, 127, 127 },
  { 204, 0, 0 },
  { 204, 102, 102 },
  { 153, 0, 0 },
  { 153, 76, 76 },
  { 127, 0, 0 },
  { 127, 63, 63 },
  { 76, 0, 0 },
  { 76, 38, 38 },
  { 255, 63, 0 },
  { 255, 159, 127 },
  { 204, 51, 0 },
  { 204, 127, 102 },
  { 153, 38, 0 },
  { 153, 95, 76 },
  { 127, 31, 0 },
  { 127, 79, 63 },
  { 76, 19, 0 },
  { 76, 47, 38 },
  { 255, 127, 0 },
  { 255, 191, 127 },
  { 204, 102, 0 },
  { 204, 153, 102 },
  { 153, 76, 0 },
  { 153, 114, 76 },
  { 127, 63, 0 },
  { 127, 95, 63 },
  { 76, 38, 0 },
  { 76, 57, 38 },
  { 255, 191, 0 },
  { 255, 223, 127 },
  { 204, 153, 0 },
  { 204, 178, 102 },
  { 153, 114, 0 },
  { 153, 133, 76 },
  { 127, 95, 0 },
  { 127, 111, 63 },
  { 76, 57, 0 },
  { 76, 66, 38 },
  { 255, 255, 0 },
  { 255, 255, 127 },
  { 204, 204, 0 },
  { 204, 204, 102 },
  { 153, 153, 0 },
  { 153, 153, 76 },
  { 127, 127, 0 },
  { 127, 127, 63 },
  { 76, 76, 0 },
  { 76, 76, 38 },
  { 191, 255, 0 },
  { 223, 255, 127 },
  { 153, 204, 0 },
  { 178, 204, 102 },
  { 114, 153, 0 },
  { 133, 153, 76 },
  { 95, 127, 0 },
  { 111, 127, 63 },
  { 57, 76, 0 },
  { 66, 76, 38 },
  { 127, 255, 0 },
  { 191, 255, 127 },
  { 102, 204, 0 },
  { 153, 204, 102 },
  { 76, 153, 0 },
  { 114, 153, 76 },
  { 63, 127, 0 },
  { 95, 127, 63 },
  { 38, 76, 0 },
  { 57, 76, 38 },
  { 63, 255, 0 },
  { 159, 255, 127 },
  { 51, 204, 0 },
  { 127, 204, 102 },
  { 38, 153, 0 },
  { 95, 153, 76 },
  { 31, 127, 0 },
  { 79, 127, 63 },
  { 19, 76, 0 },
  { 47, 76, 38 },
  { 0, 255, 0 },
  { 127, 255, 127 },
  { 0, 204, 0 },
  { 102, 204, 102 },
  { 0, 153, 0 },
  { 76, 153, 76 },
  { 0, 127, 0 },
  { 63, 127, 63 },
  { 0, 76, 0 },
  { 38, 76, 38 },
  { 0, 255, 63 },
  { 127, 255, 159 },
  { 0, 204, 51 },
  { 102, 204, 127 },
  { 0, 153, 38 },
  { 76, 153, 95 },
  { 0, 127, 31 },
  { 63, 127, 79 },
  { 0, 76, 19 },
  { 38, 76, 47 },
  { 0, 255, 127 },
  { 127, 255, 191 },
  { 0, 204, 102 },
  { 102, 204, 153 },
  { 0, 153, 76 },
  { 76, 153, 114 },
  { 0, 127, 63 },
  { 63, 127, 95 },
  { 0, 76, 38 },
  { 38, 76, 57 },
  { 0, 255, 191 },
  { 127, 255, 223 },
  { 0, 204, 153 },
  { 102, 204, 178 },
  { 0, 153, 114 },
  { 76, 153, 133 },
  { 0, 127, 95 },
  { 63, 127, 111 },
  { 0, 76, 57 },
  { 38, 76, 66 },
  { 0, 255, 255 },
  { 127, 255, 255 },
  { 0, 204, 204 },
  { 102, 204, 204 },
  { 0, 153, 153 },
  { 76, 153, 153 },
  { 0, 127, 127 },
  { 63, 127, 127 },
  { 0, 76, 76 },
  { 38, 76, 76 },
  { 0, 191, 255 },
  { 127, 223, 255 },
  { 0, 153, 204 },
  { 102, 178, 204 },
  { 0, 114, 153 },
  { 76, 133, 153 },
  { 0, 95, 127 },
  { 63, 111, 127 },
  { 0, 57, 76 },
  { 38, 66, 76 },
  { 0, 127, 255 },
  { 127, 191, 255 },
  { 0, 102, 204 },
  { 102, 153, 204 },
  { 0, 76, 153 },
  { 76, 114, 153 },
  { 0, 63, 127 },
  { 63, 95, 127 },
  { 0, 38, 76 },
  { 38, 57, 76 },
  { 0, 63, 255 },
  { 127, 159, 255 },
  { 0, 51, 204 },
  { 102, 127, 204 },
  { 0, 38, 153 },
  { 76, 95, 153 },
  { 0, 31, 127 },
  { 63, 79, 127 },
  { 0, 19, 76 },
  { 38, 47, 76 },
  { 0, 0, 255 },
  { 127, 127, 255 },
  { 0, 0, 204 },
  { 102, 102, 204 },
  { 0, 0, 153 },
  { 76, 76, 153 },
  { 0, 0, 127 },
  { 63, 63, 127 },
  { 0, 0, 76 },
  { 38, 38, 76 },
  { 63, 0, 255 },
  { 159, 127, 255 },
  { 51, 0, 204 },
  { 127, 102, 204 },
  { 38, 0, 153 },
  { 95, 76, 153 },
  { 31, 0, 127 },
  { 79, 63, 127 },
  { 19, 0, 76 },
  { 47, 38, 76 },
  { 127, 0, 255 },
  { 191, 127, 255 },
  { 102, 0, 204 },
  { 153, 102, 204 },
  { 76, 0, 153 },
  { 114, 76, 153 },
  { 63, 0, 127 },
  { 95, 63, 127 },
  { 38, 0, 76 },
  { 57, 38, 76 },
  { 191, 0, 255 },
  { 223, 127, 255 },
  { 153, 0, 204 },
  { 178, 102, 204 },
  { 114, 0, 153 },
  { 133, 76, 153 },
  { 95, 0, 127 },
  { 111, 63, 127 },
  { 57, 0, 76 },
  { 66, 38, 76 },
  { 255, 0, 255 },
  { 255, 127, 255 },
  { 204, 0, 204 },
  { 204, 102, 204 },
  { 153, 0, 153 },
  { 153, 76, 153 },
  { 127, 0, 127 },
  { 127, 63, 127 },
  { 76, 0, 76 },
  { 76, 38, 76 },
  { 255, 0, 191 },
  { 255, 127, 223 },
  { 204, 0, 153 },
  { 204, 102, 178 },
  { 153, 0, 114 },
  { 153, 76, 133 },
  { 127, 0, 95 },
  { 127, 63, 111 },
  { 76, 0, 57 },
  { 76, 38, 66 },
  { 255, 0, 127 },
  { 255, 127, 191 },
  { 204, 0, 102 },
  { 204, 102, 153 },
  { 153, 0, 76 },
  { 153, 76, 114 },
  { 127, 0, 63 },
  { 127, 63, 95 },
  { 76, 0, 38 },
  { 76, 38, 57 },
  { 255, 0, 63 },
  { 255, 127, 159 },
  { 204, 0, 51 },
  { 204, 102, 127 },
  { 153, 0, 38 },
  { 153, 76, 95 },
  { 127, 0, 31 },
  { 127, 63, 79 },
  { 76, 0, 19 },
  { 76, 38, 47 },
  { 51, 51, 51 },
  { 91, 91, 91 },
  { 132, 132, 132 },
  { 173, 173, 173 },
  { 214, 214, 214 },
  { 255, 255, 255 },
};

const char *QgsDxfExport::mDxfEncodings[][2] =
{
  { "ASCII", "" },
  { "8859_1", "ISO-8859-1" },
  { "8859_2", "ISO-8859-2" },
  { "8859_3", "ISO-8859-3" },
  { "8859_4", "ISO-8859-4" },
  { "8859_5", "ISO-8859-5" },
  { "8859_6", "ISO-8859-6" },
  { "8859_7", "ISO-8859-7" },
  { "8859_8", "ISO-8859-8" },
  { "8859_9", "ISO-8859-9" },
//  { "DOS437", "" },
  { "DOS850", "CP850" },
//  { "DOS852", "" },
//  { "DOS855", "" },
//  { "DOS857", "" },
//  { "DOS860", "" },
//  { "DOS861", "" },
//  { "DOS863", "" },
//  { "DOS864", "" },
//  { "DOS865", "" },
//  { "DOS869", "" },
//  { "DOS932", "" },
  { "MACINTOSH", "MacRoman" },
  { "BIG5", "Big5" },
  { "KSC5601", "ksc5601.1987-0" },
//   { "JOHAB", "" },
  { "DOS866", "CP866" },
  { "ANSI_1250", "CP1250" },
  { "ANSI_1251", "CP1251" },
  { "ANSI_1252", "CP1252" },
  { "GB2312", "GB2312" },
  { "ANSI_1253", "CP1253" },
  { "ANSI_1254", "CP1254" },
  { "ANSI_1255", "CP1255" },
  { "ANSI_1256", "CP1256" },
  { "ANSI_1257", "CP1257" },
  { "ANSI_874", "CP874" },
  { "ANSI_932", "Shift_JIS" },
  { "ANSI_936", "CP936" },
  { "ANSI_949", "cp949" },
  { "ANSI_950", "CP950" },
//  { "ANSI_1361", "" },
//  { "ANSI_1200", "" },
  { "ANSI_1258", "CP1258" },
};

QgsDxfExport::QgsDxfExport()
    : mSymbologyScaleDenominator( 1.0 )
    , mSymbologyExport( NoSymbology )
    , mMapUnits( QGis::Meters )
    , mSymbolLayerCounter( 0 )
    , mNextHandleId( DXF_HANDSEED )
    , mBlockCounter( 0 )
    , mModelSpaceBR( 0 )
{
}

QgsDxfExport::QgsDxfExport( const QgsDxfExport& dxfExport )
    : mModelSpaceBR( 0 )
{
  *this = dxfExport;
}

QgsDxfExport& QgsDxfExport::operator=( const QgsDxfExport & dxfExport )
{
  mLayers = dxfExport.mLayers;
  mSymbologyScaleDenominator = dxfExport.mSymbologyScaleDenominator;
  mSymbologyExport = dxfExport.mSymbologyExport;
  mMapUnits = dxfExport.mMapUnits;
  mSymbolLayerCounter = 0; // internal counter
  mNextHandleId = 0;
  mBlockCounter = 0;
  return *this;
}

QgsDxfExport::~QgsDxfExport()
{
}

void QgsDxfExport::addLayers( const QList< QPair< QgsVectorLayer *, int > > &layers )
{
  mLayers = layers;
}

void QgsDxfExport::writeGroup( int code, int i )
{
  writeGroupCode( code );
  writeInt( i );
}

void QgsDxfExport::writeGroup( int code, double d )
{
  writeGroupCode( code );
  writeDouble( d );
}

void QgsDxfExport::writeGroup( int code, const QString& s )
{
  writeGroupCode( code );
  writeString( s );
}

void QgsDxfExport::writeGroup( int code, const QgsPoint &p, double z, bool skipz )
{
  writeGroup( code + 10, p.x() );
  writeGroup( code + 20, p.y() );
  if ( !skipz )
    writeGroup( code + 30, z );
}

void QgsDxfExport::writeGroup( QColor color, int exactMatchCode, int rgbCode, int transparencyCode )
{
  int minDistAt = -1;
  int minDist = INT_MAX;

  for ( int i = 1; i < ( int )( sizeof( mDxfColors ) / sizeof( *mDxfColors ) ) && minDist > 0; ++i )
  {
    int dist = color_distance( color.rgba(), i );
    if ( dist >= minDist )
      continue;

    minDistAt = i;
    minDist = dist;
  }

  if ( minDist == 0 && color.alpha() == 255 && minDistAt != 7 )
  {
    // exact full opaque match, not black/white
    writeGroup( exactMatchCode, minDistAt );
    return;
  }

  int c = ( color.red() & 0xff ) * 0x10000 + ( color.green() & 0xff ) * 0x100 + ( color.blue() & 0xff );
  writeGroup( rgbCode, c );
  if ( transparencyCode != -1 && color.alpha() < 255 )
    writeGroup( transparencyCode, 0x2000000 | color.alpha() );
}

void QgsDxfExport::writeGroupCode( int code )
{
  mTextStream << QString( "%1\n" ).arg( code, 3, 10, QChar( ' ' ) );
}

void QgsDxfExport::writeInt( int i )
{
  mTextStream << QString( "%1\n" ).arg( i, 6, 10, QChar( ' ' ) );
}

void QgsDxfExport::writeDouble( double d )
{
  QString s( qgsDoubleToString( d ) );
  if ( !s.contains( "." ) )
    s += ".0";
  mTextStream << s << "\n";
}

void QgsDxfExport::writeString( const QString& s )
{
  mTextStream << s << "\n";
}

int QgsDxfExport::writeToFile( QIODevice* d, QString encoding )
{
  if ( !d )
  {
    return 1;
  }

  if ( !d->isOpen() && !d->open( QIODevice::WriteOnly ) )
  {
    return 2;
  }

  mTextStream.setDevice( d );
  mTextStream.setCodec( encoding.toLocal8Bit() );

  writeHeader( dxfEncoding( encoding ) );
  writeTables();
  writeBlocks();
  writeEntities();
  writeEndFile();

  return 0;
}

void QgsDxfExport::writeHeader( QString codepage )
{
  writeGroup( 999, "DXF created from QGIS" );

  startSection();
  writeGroup( 2, "HEADER" );

  // ACADVER
  writeGroup( 9, "$ACADVER" );
  writeGroup( 1, "AC1015" );

  QgsRectangle ext( mExtent.isEmpty() ? dxfExtent() : mExtent );
  if ( !ext.isEmpty() )
  {
    // EXTMIN
    writeGroup( 9, "$EXTMIN" );
    writeGroup( 0, QgsPoint( ext.xMinimum(), ext.yMinimum() ) );

    // EXTMAX
    writeGroup( 9, "$EXTMAX" );
    writeGroup( 0, QgsPoint( ext.xMaximum(), ext.yMaximum() ) );
  }

  // Global linetype scale
  writeGroup( 9, "$LTSCALE" );
  writeGroup( 40, 1.0 );

  // Point display mode (33 = circle)
  writeGroup( 9, "$PDMODE" );
  writeGroup( 70, 33 );

  // Point display size
  writeGroup( 9, "$PDSIZE" );
  writeGroup( 40, 1 );

  // Controls paper space linetype scaling (1 = No special linetype scaling, 0 = Viewport scaling governs linetype scaling)
  writeGroup( 9, "$PSLTSCALE" );
  writeGroup( 70, 0 );

  writeGroup( 9, "$HANDSEED" );
  writeGroup( 5, DXF_HANDMAX );

  writeGroup( 9, "$DWGCODEPAGE" );
  writeGroup( 3, codepage );

  endSection();
}

int QgsDxfExport::writeHandle( int code, int handle )
{
  if ( handle == 0 )
    handle = mNextHandleId++;

  Q_ASSERT_X( handle < DXF_HANDMAX, "QgsDxfExport::writeHandle(int, int)", "DXF handle too large" );

  writeGroup( code, QString( "%1" ).arg( handle, 0, 16 ) );
  return handle;
}

void QgsDxfExport::writeTables()
{
  startSection();
  writeGroup( 2, "TABLES" );

  // Iterate through all layers and get symbol layer pointers
  QList< QPair< QgsSymbolLayerV2*, QgsSymbolV2* > > slList;
  if ( mSymbologyExport != NoSymbology )
  {
    slList = symbolLayers();
  }

  // Line types
  mLineStyles.clear();
  writeGroup( 0, "TABLE" );
  writeGroup( 2, "LTYPE" );
  writeHandle();
  writeGroup( 100, "AcDbSymbolTable" );
  writeGroup( 70, nLineTypes( slList ) + 5 );

  writeDefaultLinetypes();

  // Add custom linestyles
  QList< QPair< QgsSymbolLayerV2*, QgsSymbolV2*> >::const_iterator slIt = slList.constBegin();
  for ( ; slIt != slList.constEnd(); ++slIt )
  {
    writeSymbolLayerLinetype( slIt->first );
  }

  writeGroup( 0, "ENDTAB" );

  // BLOCK_RECORD
  writeGroup( 0, "TABLE" );
  writeGroup( 2, "BLOCK_RECORD" );
  writeHandle();

  writeGroup( 100, "AcDbSymbolTable" );
  writeGroup( 70, 0 );
  writeGroup( 0, "BLOCK_RECORD" );
  mModelSpaceBR = writeHandle();
  writeGroup( 100, "AcDbSymbolTableRecord" );
  writeGroup( 100, "AcDbBlockTableRecord" );
  writeGroup( 2, "*Model_Space" );

  writeGroup( 0, "BLOCK_RECORD" );
  writeHandle();
  writeGroup( 100, "AcDbSymbolTableRecord" );
  writeGroup( 100, "AcDbBlockTableRecord" );
  writeGroup( 2, "*Paper_Space" );

  writeGroup( 0, "BLOCK_RECORD" );
  writeHandle();
  writeGroup( 100, "AcDbSymbolTableRecord" );
  writeGroup( 100, "AcDbBlockTableRecord" );
  writeGroup( 2, "*Paper_Space0" );

  int i = 0;
  slIt = slList.constBegin();
  for ( ; slIt != slList.constEnd(); ++slIt )
  {
    QgsMarkerSymbolLayerV2 *ml = dynamic_cast< QgsMarkerSymbolLayerV2*>( slIt->first );
    if ( !ml )
      continue;

    if ( hasDataDefinedProperties( ml, slIt->second ) )
      continue;

    writeGroup( 0, "BLOCK_RECORD" );
    writeHandle();
    writeGroup( 100, "AcDbSymbolTableRecord" );
    writeGroup( 100, "AcDbBlockTableRecord" );
    writeGroup( 2, QString( "symbolLayer%1" ).arg( i++ ) );
  }

  writeGroup( 0, "ENDTAB" );

  // APPID
  writeGroup( 0, "TABLE" );
  writeGroup( 2, "APPID" );
  writeHandle();
  writeGroup( 100, "AcDbSymbolTable" );
  writeGroup( 70, 1 );
  writeGroup( 0, "APPID" );
  writeHandle();
  writeGroup( 100, "AcDbSymbolTableRecord" );
  writeGroup( 100, "AcDbRegAppTableRecord" );
  writeGroup( 2, "ACAD" );
  writeGroup( 70, 0 );
  writeGroup( 0, "ENDTAB" );

  // VIEW
  writeGroup( 0, "TABLE" );
  writeGroup( 2, "VIEW" );
  writeHandle();
  writeGroup( 100, "AcDbSymbolTable" );
  writeGroup( 70, 0 );
  writeGroup( 0, "ENDTAB" );

  // UCS
  writeGroup( 0, "TABLE" );
  writeGroup( 2, "UCS" );
  writeHandle();
  writeGroup( 100, "AcDbSymbolTable" );
  writeGroup( 70, 0 );
  writeGroup( 0, "ENDTAB" );

  // VPORT
  writeGroup( 0, "TABLE" );
  writeGroup( 2, "VPORT" );
  writeHandle();
  writeGroup( 100, "AcDbSymbolTable" );

  QgsRectangle ext( mExtent.isEmpty() ? dxfExtent() : mExtent );

  writeGroup( 0, "VPORT" );
  writeHandle();
  writeGroup( 100, "AcDbSymbolTableRecord" );
  writeGroup( 100, "AcDbViewportTableRecord" );
  writeGroup( 2, "*ACTIVE" );
  writeGroup( 70, 0 );  // flags
  writeGroup( 0, QgsPoint( 0.0, 0.0 ), 0.0, true ); // lower left
  writeGroup( 1, QgsPoint( 1.0, 1.0 ), 0.0, true ); // upper right
  writeGroup( 2, QgsPoint( 0.0, 0.0 ), 0.0, true ); // view center point
  writeGroup( 3, QgsPoint( 0.0, 0.0 ), 0.0, true ); // snap base point
  writeGroup( 4, QgsPoint( 1.0, 1.0 ), 0.0, true ); // snap spacing
  writeGroup( 5, QgsPoint( 1.0, 1.0 ), 0.0, true ); // grid spacing
  writeGroup( 6, QgsPoint( 0.0, 0.0 ), 1.0 );       // view direction from target point
  writeGroup( 7, ext.center(), 0.0, true );         // view target point
  writeGroup( 40, ext.height() );                   // view height
  writeGroup( 41, ext.width() / ext.height() );     // view aspect ratio
  writeGroup( 42, 50.0 );                           // lens length
  writeGroup( 43, 0.0 );                            // front clipping plane
  writeGroup( 44, 0.0 );                            // back clipping plane
  writeGroup( 50, 0.0 );                            // snap rotation
  writeGroup( 51, 0.0 );                            // view twist angle
  writeGroup( 71, 0 );                              // view mode (0 = deactivates)
  writeGroup( 72, 100 );                            // circle zoom percent
  writeGroup( 73, 1 );                              // fast zoom setting
  writeGroup( 74, 1 );                              // UCSICON setting
  writeGroup( 75, 0 );                              // snapping off
  writeGroup( 76, 0 );                              // grid off
  writeGroup( 77, 0 );                              // snap style
  writeGroup( 78, 0 );                              // snap isopair
  writeGroup( 281, 0 );                             // render mode (0 = 2D optimized)
  writeGroup( 65, 1 );                              // value of UCSVP for this viewport
  writeGroup( 100, QgsPoint( 0.0, 0.0 ) );          // UCS origin
  writeGroup( 101, QgsPoint( 1.0, 0.0 ) );          // UCS x axis
  writeGroup( 102, QgsPoint( 0.0, 1.0 ) );          // UCS y axis
  writeGroup( 79, 0 );                              // Orthographic type of UCS (0 = UCS is not orthographic)
  writeGroup( 146, 0.0 );                           // Elevation

  writeGroup( 70, 0 );
  writeGroup( 0, "ENDTAB" );

  // DIMSTYLE
  writeGroup( 0, "TABLE" );
  writeGroup( 2, "DIMSTYLE" );
  writeHandle();
  writeGroup( 100, "AcDbSymbolTable" );
  writeGroup( 100, "AcDbDimStyleTable" );
  writeGroup( 70, 0 );
  writeGroup( 0, "ENDTAB" );

  QList< QPair<QgsVectorLayer*, int> >::const_iterator layerIt = mLayers.constBegin();
  QSet<QString> layerNames;
  for ( ; layerIt != mLayers.constEnd(); ++layerIt )
  {
    if ( !layerIsScaleBasedVisible( layerIt->first ) )
      continue;

    if ( layerIt->first )
    {
      if ( layerIt->second < 0 )
      {
        layerNames << dxfLayerName( layerIt->first->name() );
      }
      else
      {
        QList<QVariant> values;
        layerIt->first->uniqueValues( layerIt->second, values );
        foreach ( QVariant v, values )
        {
          layerNames << dxfLayerName( v.toString() );
        }
      }
    }
  }

  // Layers
  // TODO: iterate features of all layer to produce a data-defined layer list
  writeGroup( 0, "TABLE" );
  writeGroup( 2, "LAYER" );
  writeHandle();
  writeGroup( 100, "AcDbSymbolTable" );
  writeGroup( 70, layerNames.size() + 1 );

  writeGroup( 0, "LAYER" );
  writeHandle();
  writeGroup( 100, "AcDbSymbolTableRecord" );
  writeGroup( 100, "AcDbLayerTableRecord" );
  writeGroup( 2, "0" );
  writeGroup( 70, 64 );
  writeGroup( 62, 1 );
  writeGroup( 6, "CONTINUOUS" );
  writeHandle( 390, DXF_HANDPLOTSTYLE );

  foreach ( QString layerName, layerNames )
  {
    writeGroup( 0, "LAYER" );
    writeHandle();
    writeGroup( 100, "AcDbSymbolTableRecord" );
    writeGroup( 100, "AcDbLayerTableRecord" );
    writeGroup( 2, layerName );
    writeGroup( 70, 64 );
    writeGroup( 62, 1 );
    writeGroup( 6, "CONTINUOUS" );
    writeHandle( 390, DXF_HANDPLOTSTYLE );
  }
  writeGroup( 0, "ENDTAB" );

  // Text styles
  writeGroup( 0, "TABLE" );
  writeGroup( 2, "STYLE" );
  writeHandle();
  writeGroup( 100, "AcDbSymbolTable" );
  writeGroup( 70, 1 );

  // Provide only standard font for the moment
  writeGroup( 0, "STYLE" );
  writeHandle();
  writeGroup( 100, "AcDbSymbolTableRecord" );
  writeGroup( 100, "AcDbTextStyleTableRecord" );
  writeGroup( 2, "STANDARD" );
  writeGroup( 70, 64 );
  writeGroup( 40, 0.0 );
  writeGroup( 41, 1.0 );
  writeGroup( 50, 0.0 );
  writeGroup( 71, 0 );
  writeGroup( 42, 5.0 );
  writeGroup( 3, "romans.shx" );
  writeGroup( 4, "" );

  writeGroup( 0, "ENDTAB" );

  endSection();
}

void QgsDxfExport::writeBlocks()
{
  startSection();
  writeGroup( 2, "BLOCKS" );

  writeGroup( 0, "BLOCK" );
  writeHandle();
  writeGroup( 100, "AcDbEntity" );
  writeGroup( 8, "0" );
  writeGroup( 100, "AcDbBlockBegin" );
  writeGroup( 2, "*Model_Space" );
  writeGroup( 70, 0 );
  writeGroup( 0, QgsPoint( 0.0, 0.0 ) );
  writeGroup( 3, "*Model_Space" );
  writeGroup( 1, "" );
  writeGroup( 0, "ENDBLK" );
  writeHandle();
  writeGroup( 100, "AcDbEntity" );
  writeGroup( 8, "0" );
  writeGroup( 100, "AcDbBlockEnd" );

  writeGroup( 0, "BLOCK" );
  writeHandle();
  writeGroup( 100, "AcDbEntity" );
  writeGroup( 8, "0" );
  writeGroup( 100, "AcDbBlockBegin" );
  writeGroup( 2, "*Paper_Space" );
  writeGroup( 70, 0 );
  writeGroup( 0, QgsPoint( 0.0, 0.0 ) );
  writeGroup( 3, "*Paper_Space" );
  writeGroup( 1, "" );
  writeGroup( 0, "ENDBLK" );
  writeHandle();
  writeGroup( 100, "AcDbEntity" );
  writeGroup( 8, "0" );
  writeGroup( 100, "AcDbBlockEnd" );

  writeGroup( 0, "BLOCK" );
  writeHandle();
  writeGroup( 100, "AcDbEntity" );
  writeGroup( 8, "0" );
  writeGroup( 100, "AcDbBlockBegin" );
  writeGroup( 2, "*Paper_Space0" );
  writeGroup( 70, 0 );
  writeGroup( 0, QgsPoint( 0.0, 0.0 ) );
  writeGroup( 3, "*Paper_Space0" );
  writeGroup( 1, "" );
  writeGroup( 0, "ENDBLK" );
  writeHandle();
  writeGroup( 100, "AcDbEntity" );
  writeGroup( 8, "0" );
  writeGroup( 100, "AcDbBlockEnd" );

  // Iterate through all layers and get symbol layer pointers
  QList< QPair< QgsSymbolLayerV2*, QgsSymbolV2* > > slList;
  if ( mSymbologyExport != NoSymbology )
  {
    slList = symbolLayers();
  }

  QList< QPair< QgsSymbolLayerV2*, QgsSymbolV2* > >::const_iterator slIt = slList.constBegin();
  for ( ; slIt != slList.constEnd(); ++slIt )
  {
    QgsMarkerSymbolLayerV2 *ml = dynamic_cast< QgsMarkerSymbolLayerV2*>( slIt->first );
    if ( !ml )
      continue;

    // if point symbol layer and no data defined properties: write block
    QgsRenderContext ct;
    QgsSymbolV2RenderContext ctx( ct, QgsSymbolV2::MapUnit, slIt->second->alpha(), false, slIt->second->renderHints(), 0 );
    ml->startRender( ctx );

    // markers with data defined properties are inserted inline
    if ( hasDataDefinedProperties( ml, slIt->second ) )
    {
      continue;
      // ml->stopRender( ctx );
    }

    writeGroup( 0, "BLOCK" );
    writeHandle();
    writeGroup( 100, "AcDbEntity" );
    writeGroup( 8, "0" );
    writeGroup( 100, "AcDbBlockBegin" );

    QString blockName = QString( "symbolLayer%1" ).arg( mBlockCounter++ );
    writeGroup( 2, blockName );
    writeGroup( 70, 0 );
    writeGroup( 1, "" );

    // x/y/z coordinates of reference point
    // todo: consider anchor point
    // double size = ml->size();
    // size *= mapUnitScaleFactor( mSymbologyScaleDenominator, ml->sizeUnit(), mMapUnits );
    writeGroup( 0, QgsPoint( 0.0, 0.0 ) );
    writeGroup( 3, blockName );

    ml->writeDxf( *this, mapUnitScaleFactor( mSymbologyScaleDenominator, ml->sizeUnit(), mMapUnits ), "0", &ctx, 0 ); // maplayer 0 -> block receives layer from INSERT statement

    writeGroup( 0, "ENDBLK" );
    writeHandle();
    writeGroup( 100, "AcDbEntity" );
    writeGroup( 100, "AcDbBlockEnd" );
    writeGroup( 8, "0" );

    mPointSymbolBlocks.insert( ml, blockName );
    ml->stopRender( ctx );
  }
  endSection();
}

void QgsDxfExport::writeEntities()
{
  startSection();
  writeGroup( 2, "ENTITIES" );

  // label engine
  QgsDxfPalLabeling labelEngine( this, mExtent.isEmpty() ? dxfExtent() : mExtent, mSymbologyScaleDenominator, mMapUnits );
  QgsRenderContext& ctx = labelEngine.renderContext();

  // iterate through the maplayers
  QList< QPair< QgsVectorLayer*, int > >::iterator layerIt = mLayers.begin();
  for ( ; layerIt != mLayers.end(); ++layerIt )
  {
    QgsVectorLayer* vl = layerIt->first;
    if ( !vl || !layerIsScaleBasedVisible( vl ) )
    {
      continue;
    }

    QgsSymbolV2RenderContext sctx( ctx, QgsSymbolV2::MM, 1.0, false, 0, 0 );
    QgsFeatureRendererV2* renderer = vl->rendererV2();
    if ( !renderer )
    {
      continue;
    }
    renderer->startRender( ctx, vl->pendingFields() );

    QStringList attributes = renderer->usedAttributes();
    if ( vl->pendingFields().exists( layerIt->second ) )
    {
      QString layerAttr = vl->pendingFields().at( layerIt->second ).name();
      if ( !attributes.contains( layerAttr ) )
        attributes << layerAttr;
    }

    bool labelLayer = labelEngine.prepareLayer( vl, attributes, ctx ) != 0;

    if ( mSymbologyExport == QgsDxfExport::SymbolLayerSymbology &&
         ( renderer->capabilities() & QgsFeatureRendererV2::SymbolLevels ) &&
         renderer->usingSymbolLevels() )
    {
      writeEntitiesSymbolLevels( vl );
      renderer->stopRender( ctx );
      continue;
    }

    QgsFeatureRequest freq = QgsFeatureRequest().setSubsetOfAttributes( attributes, vl->pendingFields() );
    if ( !mExtent.isEmpty() )
    {
      freq.setFilterRect( mExtent );
    }

    QgsFeatureIterator featureIt = vl->getFeatures( freq );
    QgsFeature fet;
    while ( featureIt.nextFeature( fet ) )
    {
      QString layerName( dxfLayerName( layerIt->second == -1 ? vl->name() : fet.attribute( layerIt->second ).toString() ) );

      sctx.setFeature( &fet );
      if ( mSymbologyExport == NoSymbology )
      {
        addFeature( sctx, layerName, 0, 0 ); // no symbology at all
      }
      else
      {
        QgsSymbolV2List symbolList = renderer->symbolsForFeature( fet );
        if ( symbolList.size() < 1 )
        {
          continue;
        }

        if ( mSymbologyExport == QgsDxfExport::SymbolLayerSymbology ) // symbol layer symbology, but layer does not use symbol levels
        {
          QgsSymbolV2List::iterator symbolIt = symbolList.begin();
          for ( ; symbolIt != symbolList.end(); ++symbolIt )
          {
            int nSymbolLayers = ( *symbolIt )->symbolLayerCount();
            for ( int i = 0; i < nSymbolLayers; ++i )
            {
              addFeature( sctx, layerName, ( *symbolIt )->symbolLayer( i ), *symbolIt );
            }
          }
        }
        else
        {
          // take first symbollayer from first symbol
          QgsSymbolV2* s = symbolList.first();
          if ( !s || s->symbolLayerCount() < 1 )
          {
            continue;
          }
          addFeature( sctx, layerName, s->symbolLayer( 0 ), s );
        }

        if ( labelLayer )
        {
          labelEngine.registerFeature( vl->id(), fet, ctx, layerName );
        }
      }
    }

    renderer->stopRender( ctx );
  }

  labelEngine.drawLabeling( ctx );
  endSection();
}

void QgsDxfExport::writeEntitiesSymbolLevels( QgsVectorLayer* layer )
{
  if ( !layer )
  {
    return;
  }

  QgsFeatureRendererV2* renderer = layer->rendererV2();
  if ( !renderer )
  {
    // TODO return error
    return;
  }
  QHash< QgsSymbolV2*, QList<QgsFeature> > features;

  QgsRenderContext ctx = renderContext();
  QgsSymbolV2RenderContext sctx( ctx, QgsSymbolV2::MM, 1.0, false, 0, 0 );
  renderer->startRender( ctx, layer->pendingFields() );

  // get iterator
  QgsFeatureRequest req;
  if ( layer->wkbType() == QGis::WKBNoGeometry )
  {
    req.setFlags( QgsFeatureRequest::NoGeometry );
  }
  req.setSubsetOfAttributes( QStringList( renderer->usedAttributes() ), layer->pendingFields() );
  if ( !mExtent.isEmpty() )
  {
    req.setFilterRect( mExtent );
  }
  QgsFeatureIterator fit = layer->getFeatures( req );

  // fetch features
  QgsFeature fet;
  QgsSymbolV2* featureSymbol = 0;
  while ( fit.nextFeature( fet ) )
  {
    featureSymbol = renderer->symbolForFeature( fet );
    if ( !featureSymbol )
    {
      continue;
    }

    QHash< QgsSymbolV2*, QList<QgsFeature> >::iterator it = features.find( featureSymbol );
    if ( it == features.end() )
    {
      it = features.insert( featureSymbol, QList<QgsFeature>() );
    }
    it.value().append( fet );
  }

  // find out order
  QgsSymbolV2LevelOrder levels;
  QgsSymbolV2List symbols = renderer->symbols();
  for ( int i = 0; i < symbols.count(); i++ )
  {
    QgsSymbolV2* sym = symbols[i];
    for ( int j = 0; j < sym->symbolLayerCount(); j++ )
    {
      int level = sym->symbolLayer( j )->renderingPass();
      if ( level < 0 || level >= 1000 ) // ignore invalid levels
        continue;
      QgsSymbolV2LevelItem item( sym, j );
      while ( level >= levels.count() ) // append new empty levels
        levels.append( QgsSymbolV2Level() );
      levels[level].append( item );
    }
  }

  // export symbol layers and symbology
  for ( int l = 0; l < levels.count(); l++ )
  {
    QgsSymbolV2Level& level = levels[l];
    for ( int i = 0; i < level.count(); i++ )
    {
      QgsSymbolV2LevelItem& item = level[i];
      QHash< QgsSymbolV2*, QList<QgsFeature> >::iterator levelIt = features.find( item.symbol() );
      if ( levelIt == features.end() )
      {
        QgsDebugMsg( QString( "No feature found for symbol on %1 %2.%3" ).arg( layer->id() ).arg( l ).arg( i ) );
        continue;
      }

      int llayer = item.layer();
      QList<QgsFeature>& featureList = levelIt.value();
      QList<QgsFeature>::iterator featureIt = featureList.begin();
      for ( ; featureIt != featureList.end(); ++featureIt )
      {
        sctx.setFeature( &*featureIt );
        addFeature( sctx, layer->name(), levelIt.key()->symbolLayer( llayer ), levelIt.key() );
      }
    }
  }
  renderer->stopRender( ctx );
}

void QgsDxfExport::writeEndFile()
{
  // From GDAL trailer.dxf
  mTextStream << "\
  0\n\
SECTION\n\
  2\n\
OBJECTS\n\
  0\n\
DICTIONARY\n\
  5\n\
C\n\
330\n\
0\n\
100\n\
AcDbDictionary\n\
281\n\
     1\n\
  3\n\
ACAD_GROUP\n\
350\n\
D\n\
  3\n\
ACAD_LAYOUT\n\
350\n\
1A\n\
  3\n\
ACAD_MLEADERSTYLE\n\
350\n\
43\n\
  3\n\
ACAD_MLINESTYLE\n\
350\n\
17\n\
  3\n\
ACAD_PLOTSETTINGS\n\
350\n\
19\n\
  3\n\
ACAD_PLOTSTYLENAME\n\
350\n\
E\n\
  3\n\
ACAD_TABLESTYLE\n\
350\n\
42\n\
  3\n\
ACAD_VISUALSTYLE\n\
350\n\
2A\n\
  0\n\
DICTIONARY\n\
  5\n\
D\n\
102\n\
{ACAD_REACTORS\n\
330\n\
C\n\
102\n\
}\n\
330\n\
C\n\
100\n\
AcDbDictionary\n\
281\n\
     1\n\
  0\n\
DICTIONARY\n\
  5\n\
1A\n\
102\n\
{ACAD_REACTORS\n\
330\n\
C\n\
102\n\
}\n\
330\n\
C\n\
100\n\
AcDbDictionary\n\
281\n\
     1\n\
  3\n\
Layout1\n\
350\n\
1E\n\
  3\n\
Layout2\n\
350\n\
26\n\
  3\n\
Model\n\
350\n\
22\n\
  0\n\
DICTIONARY\n\
  5\n\
43\n\
102\n\
{ACAD_REACTORS\n\
330\n\
C\n\
102\n\
}\n\
330\n\
C\n\
100\n\
AcDbDictionary\n\
281\n\
     1\n\
  0\n\
DICTIONARY\n\
  5\n\
17\n\
102\n\
{ACAD_REACTORS\n\
330\n\
C\n\
102\n\
}\n\
330\n\
C\n\
100\n\
AcDbDictionary\n\
281\n\
     1\n\
  3\n\
Standard\n\
350\n\
18\n\
  0\n\
DICTIONARY\n\
  5\n\
19\n\
102\n\
{ACAD_REACTORS\n\
330\n\
C\n\
102\n\
}\n\
330\n\
C\n\
100\n\
AcDbDictionary\n\
281\n\
     1\n\
  0\n\
ACDBDICTIONARYWDFLT\n\
  5\n\
E\n\
102\n\
{ACAD_REACTORS\n\
330\n\
C\n\
102\n\
}\n\
330\n\
C\n\
100\n\
AcDbDictionary\n\
281\n\
     1\n\
  3\n\
Normal\n\
350\n\
F\n\
100\n\
AcDbDictionaryWithDefault\n\
340\n\
F\n\
  0\n\
DICTIONARY\n\
  5\n\
42\n\
102\n\
{ACAD_REACTORS\n\
330\n\
C\n\
102\n\
}\n\
330\n\
C\n\
100\n\
AcDbDictionary\n\
281\n\
     1\n\
  0\n\
DICTIONARY\n\
  5\n\
2A\n\
102\n\
{ACAD_REACTORS\n\
330\n\
C\n\
102\n\
}\n\
330\n\
C\n\
100\n\
AcDbDictionary\n\
281\n\
     1\n\
  3\n\
2dWireframe\n\
350\n\
2F\n\
  3\n\
3D Hidden\n\
350\n\
31\n\
  3\n\
3dWireframe\n\
350\n\
30\n\
  3\n\
Basic\n\
350\n\
32\n\
  3\n\
Brighten\n\
350\n\
36\n\
  3\n\
ColorChange\n\
350\n\
3A\n\
  3\n\
Conceptual\n\
350\n\
34\n\
  3\n\
Dim\n\
350\n\
35\n\
  3\n\
Facepattern\n\
350\n\
39\n\
  3\n\
Flat\n\
350\n\
2B\n\
  3\n\
FlatWithEdges\n\
350\n\
2C\n\
  3\n\
Gouraud\n\
350\n\
2D\n\
  3\n\
GouraudWithEdges\n\
350\n\
2E\n\
  3\n\
Linepattern\n\
350\n\
38\n\
  3\n\
Realistic\n\
350\n\
33\n\
  3\n\
Thicken\n\
350\n\
37\n\
  0\n\
LAYOUT\n\
  5\n\
1E\n\
102\n\
{ACAD_REACTORS\n\
330\n\
1A\n\
102\n\
}\n\
330\n\
1A\n\
100\n\
AcDbPlotSettings\n\
  1\n\
\n\
  2\n\
none_device\n\
  4\n\
\n\
  6\n\
\n\
 40\n\
0.0\n\
 41\n\
0.0\n\
 42\n\
0.0\n\
 43\n\
0.0\n\
 44\n\
0.0\n\
 45\n\
0.0\n\
 46\n\
0.0\n\
 47\n\
0.0\n\
 48\n\
0.0\n\
 49\n\
0.0\n\
140\n\
0.0\n\
141\n\
0.0\n\
142\n\
1.0\n\
143\n\
1.0\n\
 70\n\
   688\n\
 72\n\
     0\n\
 73\n\
     0\n\
 74\n\
     5\n\
  7\n\
\n\
 75\n\
    16\n\
 76\n\
     0\n\
 77\n\
     2\n\
 78\n\
   300\n\
147\n\
1.0\n\
148\n\
0.0\n\
149\n\
0.0\n\
100\n\
AcDbLayout\n\
  1\n\
Layout1\n\
 70\n\
     1\n\
 71\n\
     1\n\
 10\n\
0.0\n\
 20\n\
0.0\n\
 11\n\
12.0\n\
 21\n\
9.0\n\
 12\n\
0.0\n\
 22\n\
0.0\n\
 32\n\
0.0\n\
 14\n\
1.000000000000000E+20\n\
 24\n\
1.000000000000000E+20\n\
 34\n\
1.000000000000000E+20\n\
 15\n\
-1.000000000000000E+20\n\
 25\n\
-1.000000000000000E+20\n\
 35\n\
-1.000000000000000E+20\n\
146\n\
0.0\n\
 13\n\
0.0\n\
 23\n\
0.0\n\
 33\n\
0.0\n\
 16\n\
1.0\n\
 26\n\
0.0\n\
 36\n\
0.0\n\
 17\n\
0.0\n\
 27\n\
1.0\n\
 37\n\
0.0\n\
 76\n\
     0\n\
330\n\
1B\n\
  0\n\
LAYOUT\n\
  5\n\
26\n\
102\n\
{ACAD_REACTORS\n\
330\n\
1A\n\
102\n\
}\n\
330\n\
1A\n\
100\n\
AcDbPlotSettings\n\
  1\n\
\n\
  2\n\
none_device\n\
  4\n\
\n\
  6\n\
\n\
 40\n\
0.0\n\
 41\n\
0.0\n\
 42\n\
0.0\n\
 43\n\
0.0\n\
 44\n\
0.0\n\
 45\n\
0.0\n\
 46\n\
0.0\n\
 47\n\
0.0\n\
 48\n\
0.0\n\
 49\n\
0.0\n\
140\n\
0.0\n\
141\n\
0.0\n\
142\n\
1.0\n\
143\n\
1.0\n\
 70\n\
   688\n\
 72\n\
     0\n\
 73\n\
     0\n\
 74\n\
     5\n\
  7\n\
\n\
 75\n\
    16\n\
 76\n\
     0\n\
 77\n\
     2\n\
 78\n\
   300\n\
147\n\
1.0\n\
148\n\
0.0\n\
149\n\
0.0\n\
100\n\
AcDbLayout\n\
  1\n\
Layout2\n\
 70\n\
     1\n\
 71\n\
     2\n\
 10\n\
0.0\n\
 20\n\
0.0\n\
 11\n\
0.0\n\
 21\n\
0.0\n\
 12\n\
0.0\n\
 22\n\
0.0\n\
 32\n\
0.0\n\
 14\n\
0.0\n\
 24\n\
0.0\n\
 34\n\
0.0\n\
 15\n\
0.0\n\
 25\n\
0.0\n\
 35\n\
0.0\n\
146\n\
0.0\n\
 13\n\
0.0\n\
 23\n\
0.0\n\
 33\n\
0.0\n\
 16\n\
1.0\n\
 26\n\
0.0\n\
 36\n\
0.0\n\
 17\n\
0.0\n\
 27\n\
1.0\n\
 37\n\
0.0\n\
 76\n\
     0\n\
330\n\
23\n\
  0\n\
LAYOUT\n\
  5\n\
22\n\
102\n\
{ACAD_REACTORS\n\
330\n\
1A\n\
102\n\
}\n\
330\n\
1A\n\
100\n\
AcDbPlotSettings\n\
  1\n\
\n\
  2\n\
none_device\n\
  4\n\
\n\
  6\n\
\n\
 40\n\
0.0\n\
 41\n\
0.0\n\
 42\n\
0.0\n\
 43\n\
0.0\n\
 44\n\
0.0\n\
 45\n\
0.0\n\
 46\n\
0.0\n\
 47\n\
0.0\n\
 48\n\
0.0\n\
 49\n\
0.0\n\
140\n\
0.0\n\
141\n\
0.0\n\
142\n\
1.0\n\
143\n\
1.0\n\
 70\n\
  1712\n\
 72\n\
     0\n\
 73\n\
     0\n\
 74\n\
     0\n\
  7\n\
\n\
 75\n\
     0\n\
 76\n\
     0\n\
 77\n\
     2\n\
 78\n\
   300\n\
147\n\
1.0\n\
148\n\
0.0\n\
149\n\
0.0\n\
100\n\
AcDbLayout\n\
  1\n\
Model\n\
 70\n\
     1\n\
 71\n\
     0\n\
 10\n\
0.0\n\
 20\n\
0.0\n\
 11\n\
12.0\n\
 21\n\
9.0\n\
 12\n\
0.0\n\
 22\n\
0.0\n\
 32\n\
0.0\n\
 14\n\
30.0\n\
 24\n\
49.75\n\
 34\n\
0.0\n\
 15\n\
130.5\n\
 25\n\
163.1318914119703\n\
 35\n\
0.0\n\
146\n\
0.0\n\
 13\n\
0.0\n\
 23\n\
0.0\n\
 33\n\
0.0\n\
 16\n\
1.0\n\
 26\n\
0.0\n\
 36\n\
0.0\n\
 17\n\
0.0\n\
 27\n\
1.0\n\
 37\n\
0.0\n\
 76\n\
     0\n\
330\n\
1F\n\
331\n\
29\n\
  0\n\
MLINESTYLE\n\
  5\n\
18\n\
102\n\
{ACAD_REACTORS\n\
330\n\
17\n\
102\n\
}\n\
330\n\
17\n\
100\n\
AcDbMlineStyle\n\
  2\n\
Standard\n\
 70\n\
     0\n\
  3\n\
\n\
 62\n\
   256\n\
 51\n\
90.0\n\
 52\n\
90.0\n\
 71\n\
     2\n\
 49\n\
0.5\n\
 62\n\
   256\n\
  6\n\
BYLAYER\n\
 49\n\
-0.5\n\
 62\n\
   256\n\
  6\n\
BYLAYER\n\
  0\n\
ACDBPLACEHOLDER\n\
  5\n\
F\n\
102\n\
{ACAD_REACTORS\n\
330\n\
E\n\
102\n\
}\n\
330\n\
E\n\
  0\n\
VISUALSTYLE\n\
  5\n\
2F\n\
102\n\
{ACAD_REACTORS\n\
330\n\
2A\n\
102\n\
}\n\
330\n\
2A\n\
100\n\
AcDbVisualStyle\n\
  2\n\
2dWireframe\n\
 70\n\
     4\n\
 71\n\
     0\n\
 72\n\
     2\n\
 73\n\
     0\n\
 90\n\
        0\n\
 40\n\
-0.6\n\
 41\n\
-30.0\n\
 62\n\
     5\n\
 63\n\
     7\n\
421\n\
 16777215\n\
 74\n\
     1\n\
 91\n\
        4\n\
 64\n\
     7\n\
 65\n\
   257\n\
 75\n\
     1\n\
175\n\
     1\n\
 42\n\
1.0\n\
 92\n\
        0\n\
 66\n\
   257\n\
 43\n\
1.0\n\
 76\n\
     1\n\
 77\n\
     6\n\
 78\n\
     2\n\
 67\n\
     7\n\
 79\n\
     5\n\
170\n\
     0\n\
171\n\
     0\n\
290\n\
     0\n\
174\n\
     0\n\
 93\n\
        1\n\
 44\n\
0.0\n\
173\n\
     0\n\
291\n\
     0\n\
 45\n\
0.0\n\
1001\n\
ACAD\n\
1000\n\
AcDbSavedByObjectVersion\n\
1070\n\
     0\n\
  0\n\
VISUALSTYLE\n\
  5\n\
31\n\
102\n\
{ACAD_REACTORS\n\
330\n\
2A\n\
102\n\
}\n\
330\n\
2A\n\
100\n\
AcDbVisualStyle\n\
  2\n\
3D Hidden\n\
 70\n\
     6\n\
 71\n\
     1\n\
 72\n\
     2\n\
 73\n\
     2\n\
 90\n\
        0\n\
 40\n\
-0.6\n\
 41\n\
-30.0\n\
 62\n\
     5\n\
 63\n\
     7\n\
421\n\
 16777215\n\
 74\n\
     2\n\
 91\n\
        2\n\
 64\n\
     7\n\
 65\n\
   257\n\
 75\n\
     2\n\
175\n\
     1\n\
 42\n\
40.0\n\
 92\n\
        0\n\
 66\n\
   257\n\
 43\n\
1.0\n\
 76\n\
     1\n\
 77\n\
     6\n\
 78\n\
     2\n\
 67\n\
     7\n\
 79\n\
     3\n\
170\n\
     0\n\
171\n\
     0\n\
290\n\
     0\n\
174\n\
     0\n\
 93\n\
        1\n\
 44\n\
0.0\n\
173\n\
     0\n\
291\n\
     0\n\
 45\n\
0.0\n\
1001\n\
ACAD\n\
1000\n\
AcDbSavedByObjectVersion\n\
1070\n\
     0\n\
  0\n\
VISUALSTYLE\n\
  5\n\
30\n\
102\n\
{ACAD_REACTORS\n\
330\n\
2A\n\
102\n\
}\n\
330\n\
2A\n\
100\n\
AcDbVisualStyle\n\
  2\n\
3dWireframe\n\
 70\n\
     5\n\
 71\n\
     0\n\
 72\n\
     2\n\
 73\n\
     0\n\
 90\n\
        0\n\
 40\n\
-0.6\n\
 41\n\
-30.0\n\
 62\n\
     5\n\
 63\n\
     7\n\
421\n\
 16777215\n\
 74\n\
     1\n\
 91\n\
        4\n\
 64\n\
     7\n\
 65\n\
   257\n\
 75\n\
     1\n\
175\n\
     1\n\
 42\n\
1.0\n\
 92\n\
        0\n\
 66\n\
   257\n\
 43\n\
1.0\n\
 76\n\
     1\n\
 77\n\
     6\n\
 78\n\
     2\n\
 67\n\
     7\n\
 79\n\
     5\n\
170\n\
     0\n\
171\n\
     0\n\
290\n\
     0\n\
174\n\
     0\n\
 93\n\
        1\n\
 44\n\
0.0\n\
173\n\
     0\n\
291\n\
     0\n\
 45\n\
0.0\n\
1001\n\
ACAD\n\
1000\n\
AcDbSavedByObjectVersion\n\
1070\n\
     0\n\
  0\n\
VISUALSTYLE\n\
  5\n\
32\n\
102\n\
{ACAD_REACTORS\n\
330\n\
2A\n\
102\n\
}\n\
330\n\
2A\n\
100\n\
AcDbVisualStyle\n\
  2\n\
Basic\n\
 70\n\
     7\n\
 71\n\
     1\n\
 72\n\
     0\n\
 73\n\
     1\n\
 90\n\
        0\n\
 40\n\
-0.6\n\
 41\n\
-30.0\n\
 62\n\
     5\n\
 63\n\
     7\n\
421\n\
 16777215\n\
 74\n\
     0\n\
 91\n\
        4\n\
 64\n\
     7\n\
 65\n\
   257\n\
 75\n\
     1\n\
175\n\
     1\n\
 42\n\
1.0\n\
 92\n\
        8\n\
 66\n\
     7\n\
 43\n\
1.0\n\
 76\n\
     1\n\
 77\n\
     6\n\
 78\n\
     2\n\
 67\n\
     7\n\
 79\n\
     5\n\
170\n\
     0\n\
171\n\
     0\n\
290\n\
     0\n\
174\n\
     0\n\
 93\n\
        1\n\
 44\n\
0.0\n\
173\n\
     0\n\
291\n\
     1\n\
 45\n\
0.0\n\
1001\n\
ACAD\n\
1000\n\
AcDbSavedByObjectVersion\n\
1070\n\
     0\n\
  0\n\
VISUALSTYLE\n\
  5\n\
36\n\
102\n\
{ACAD_REACTORS\n\
330\n\
2A\n\
102\n\
}\n\
330\n\
2A\n\
100\n\
AcDbVisualStyle\n\
  2\n\
Brighten\n\
 70\n\
    12\n\
 71\n\
     2\n\
 72\n\
     2\n\
 73\n\
     0\n\
 90\n\
        0\n\
 40\n\
-0.6\n\
 41\n\
-30.0\n\
 62\n\
     5\n\
 63\n\
     7\n\
421\n\
 16777215\n\
 74\n\
     1\n\
 91\n\
        4\n\
 64\n\
     7\n\
 65\n\
   257\n\
 75\n\
     1\n\
175\n\
     1\n\
 42\n\
1.0\n\
 92\n\
        8\n\
 66\n\
     7\n\
 43\n\
1.0\n\
 76\n\
     1\n\
 77\n\
     6\n\
 78\n\
     2\n\
 67\n\
     7\n\
 79\n\
     5\n\
170\n\
     0\n\
171\n\
     0\n\
290\n\
     0\n\
174\n\
     0\n\
 93\n\
        1\n\
 44\n\
50.0\n\
173\n\
     0\n\
291\n\
     1\n\
 45\n\
0.0\n\
1001\n\
ACAD\n\
1000\n\
AcDbSavedByObjectVersion\n\
1070\n\
     0\n\
  0\n\
VISUALSTYLE\n\
  5\n\
3A\n\
102\n\
{ACAD_REACTORS\n\
330\n\
2A\n\
102\n\
}\n\
330\n\
2A\n\
100\n\
AcDbVisualStyle\n\
  2\n\
ColorChange\n\
 70\n\
    16\n\
 71\n\
     2\n\
 72\n\
     2\n\
 73\n\
     3\n\
 90\n\
        0\n\
 40\n\
-0.6\n\
 41\n\
-30.0\n\
 62\n\
     5\n\
 63\n\
     8\n\
421\n\
  8421504\n\
 74\n\
     1\n\
 91\n\
        4\n\
 64\n\
     7\n\
 65\n\
   257\n\
 75\n\
     1\n\
175\n\
     1\n\
 42\n\
1.0\n\
 92\n\
        8\n\
 66\n\
     8\n\
424\n\
  8421504\n\
 43\n\
1.0\n\
 76\n\
     1\n\
 77\n\
     6\n\
 78\n\
     2\n\
 67\n\
     7\n\
 79\n\
     5\n\
170\n\
     0\n\
171\n\
     0\n\
290\n\
     0\n\
174\n\
     0\n\
 93\n\
        1\n\
 44\n\
0.0\n\
173\n\
     0\n\
291\n\
     1\n\
 45\n\
0.0\n\
1001\n\
ACAD\n\
1000\n\
AcDbSavedByObjectVersion\n\
1070\n\
     0\n\
  0\n\
VISUALSTYLE\n\
  5\n\
34\n\
102\n\
{ACAD_REACTORS\n\
330\n\
2A\n\
102\n\
}\n\
330\n\
2A\n\
100\n\
AcDbVisualStyle\n\
  2\n\
Conceptual\n\
 70\n\
     9\n\
 71\n\
     3\n\
 72\n\
     2\n\
 73\n\
     0\n\
 90\n\
        0\n\
 40\n\
-0.6\n\
 41\n\
-30.0\n\
 62\n\
     5\n\
 63\n\
     7\n\
421\n\
 16777215\n\
 74\n\
     2\n\
 91\n\
        2\n\
 64\n\
     7\n\
 65\n\
   257\n\
 75\n\
     1\n\
175\n\
     1\n\
 42\n\
40.0\n\
 92\n\
        8\n\
 66\n\
     7\n\
 43\n\
1.0\n\
 76\n\
     1\n\
 77\n\
     6\n\
 78\n\
     2\n\
 67\n\
     7\n\
 79\n\
     3\n\
170\n\
     0\n\
171\n\
     0\n\
290\n\
     0\n\
174\n\
     0\n\
 93\n\
        1\n\
 44\n\
0.0\n\
173\n\
     0\n\
291\n\
     0\n\
 45\n\
0.0\n\
1001\n\
ACAD\n\
1000\n\
AcDbSavedByObjectVersion\n\
1070\n\
     0\n\
  0\n\
VISUALSTYLE\n\
  5\n\
35\n\
102\n\
{ACAD_REACTORS\n\
330\n\
2A\n\
102\n\
}\n\
330\n\
2A\n\
100\n\
AcDbVisualStyle\n\
  2\n\
Dim\n\
 70\n\
    11\n\
 71\n\
     2\n\
 72\n\
     2\n\
 73\n\
     0\n\
 90\n\
        0\n\
 40\n\
-0.6\n\
 41\n\
-30.0\n\
 62\n\
     5\n\
 63\n\
     7\n\
421\n\
 16777215\n\
 74\n\
     1\n\
 91\n\
        4\n\
 64\n\
     7\n\
 65\n\
   257\n\
 75\n\
     1\n\
175\n\
     1\n\
 42\n\
1.0\n\
 92\n\
        8\n\
 66\n\
     7\n\
 43\n\
1.0\n\
 76\n\
     1\n\
 77\n\
     6\n\
 78\n\
     2\n\
 67\n\
     7\n\
 79\n\
     5\n\
170\n\
     0\n\
171\n\
     0\n\
290\n\
     0\n\
174\n\
     0\n\
 93\n\
        1\n\
 44\n\
-50.0\n\
173\n\
     0\n\
291\n\
     1\n\
 45\n\
0.0\n\
1001\n\
ACAD\n\
1000\n\
AcDbSavedByObjectVersion\n\
1070\n\
     0\n\
  0\n\
VISUALSTYLE\n\
  5\n\
39\n\
102\n\
{ACAD_REACTORS\n\
330\n\
2A\n\
102\n\
}\n\
330\n\
2A\n\
100\n\
AcDbVisualStyle\n\
  2\n\
Facepattern\n\
 70\n\
    15\n\
 71\n\
     2\n\
 72\n\
     2\n\
 73\n\
     0\n\
 90\n\
        0\n\
 40\n\
-0.6\n\
 41\n\
-30.0\n\
 62\n\
     5\n\
 63\n\
     7\n\
421\n\
 16777215\n\
 74\n\
     1\n\
 91\n\
        4\n\
 64\n\
     7\n\
 65\n\
   257\n\
 75\n\
     1\n\
175\n\
     1\n\
 42\n\
1.0\n\
 92\n\
        8\n\
 66\n\
     7\n\
 43\n\
1.0\n\
 76\n\
     1\n\
 77\n\
     6\n\
 78\n\
     2\n\
 67\n\
     7\n\
 79\n\
     5\n\
170\n\
     0\n\
171\n\
     0\n\
290\n\
     0\n\
174\n\
     0\n\
 93\n\
        1\n\
 44\n\
0.0\n\
173\n\
     0\n\
291\n\
     1\n\
 45\n\
0.0\n\
1001\n\
ACAD\n\
1000\n\
AcDbSavedByObjectVersion\n\
1070\n\
     0\n\
  0\n\
VISUALSTYLE\n\
  5\n\
2B\n\
102\n\
{ACAD_REACTORS\n\
330\n\
2A\n\
102\n\
}\n\
330\n\
2A\n\
100\n\
AcDbVisualStyle\n\
  2\n\
Flat\n\
 70\n\
     0\n\
 71\n\
     2\n\
 72\n\
     1\n\
 73\n\
     1\n\
 90\n\
        2\n\
 40\n\
-0.6\n\
 41\n\
30.0\n\
 62\n\
     5\n\
 63\n\
     7\n\
421\n\
 16777215\n\
 74\n\
     0\n\
 91\n\
        4\n\
 64\n\
     7\n\
 65\n\
   257\n\
 75\n\
     1\n\
175\n\
     1\n\
 42\n\
1.0\n\
 92\n\
        8\n\
 66\n\
     7\n\
 43\n\
1.0\n\
 76\n\
     1\n\
 77\n\
     6\n\
 78\n\
     2\n\
 67\n\
     7\n\
 79\n\
     5\n\
170\n\
     0\n\
171\n\
     0\n\
290\n\
     0\n\
174\n\
     0\n\
 93\n\
       13\n\
 44\n\
0.0\n\
173\n\
     0\n\
291\n\
     1\n\
 45\n\
0.0\n\
1001\n\
ACAD\n\
1000\n\
AcDbSavedByObjectVersion\n\
1070\n\
     0\n\
  0\n\
VISUALSTYLE\n\
  5\n\
2C\n\
102\n\
{ACAD_REACTORS\n\
330\n\
2A\n\
102\n\
}\n\
330\n\
2A\n\
100\n\
AcDbVisualStyle\n\
  2\n\
FlatWithEdges\n\
 70\n\
     1\n\
 71\n\
     2\n\
 72\n\
     1\n\
 73\n\
     1\n\
 90\n\
        2\n\
 40\n\
-0.6\n\
 41\n\
30.0\n\
 62\n\
     5\n\
 63\n\
     7\n\
421\n\
 16777215\n\
 74\n\
     1\n\
 91\n\
        4\n\
 64\n\
     7\n\
 65\n\
   257\n\
 75\n\
     1\n\
175\n\
     1\n\
 42\n\
1.0\n\
 92\n\
        0\n\
 66\n\
   257\n\
 43\n\
1.0\n\
 76\n\
     1\n\
 77\n\
     6\n\
 78\n\
     2\n\
 67\n\
     7\n\
 79\n\
     5\n\
170\n\
     0\n\
171\n\
     0\n\
290\n\
     0\n\
174\n\
     0\n\
 93\n\
       13\n\
 44\n\
0.0\n\
173\n\
     0\n\
291\n\
     1\n\
 45\n\
0.0\n\
1001\n\
ACAD\n\
1000\n\
AcDbSavedByObjectVersion\n\
1070\n\
     0\n\
  0\n\
VISUALSTYLE\n\
  5\n\
2D\n\
102\n\
{ACAD_REACTORS\n\
330\n\
2A\n\
102\n\
}\n\
330\n\
2A\n\
100\n\
AcDbVisualStyle\n\
  2\n\
Gouraud\n\
 70\n\
     2\n\
 71\n\
     2\n\
 72\n\
     2\n\
 73\n\
     1\n\
 90\n\
        2\n\
 40\n\
-0.6\n\
 41\n\
30.0\n\
 62\n\
     5\n\
 63\n\
     7\n\
421\n\
 16777215\n\
 74\n\
     0\n\
 91\n\
        4\n\
 64\n\
     7\n\
 65\n\
   257\n\
 75\n\
     1\n\
175\n\
     1\n\
 42\n\
1.0\n\
 92\n\
        0\n\
 66\n\
     7\n\
 43\n\
1.0\n\
 76\n\
     1\n\
 77\n\
     6\n\
 78\n\
     2\n\
 67\n\
     7\n\
 79\n\
     5\n\
170\n\
     0\n\
171\n\
     0\n\
290\n\
     0\n\
174\n\
     0\n\
 93\n\
       13\n\
 44\n\
0.0\n\
173\n\
     0\n\
291\n\
     1\n\
 45\n\
0.0\n\
1001\n\
ACAD\n\
1000\n\
AcDbSavedByObjectVersion\n\
1070\n\
     0\n\
  0\n\
VISUALSTYLE\n\
  5\n\
2E\n\
102\n\
{ACAD_REACTORS\n\
330\n\
2A\n\
102\n\
}\n\
330\n\
2A\n\
100\n\
AcDbVisualStyle\n\
  2\n\
GouraudWithEdges\n\
 70\n\
     3\n\
 71\n\
     2\n\
 72\n\
     2\n\
 73\n\
     1\n\
 90\n\
        2\n\
 40\n\
-0.6\n\
 41\n\
30.0\n\
 62\n\
     5\n\
 63\n\
     7\n\
421\n\
 16777215\n\
 74\n\
     1\n\
 91\n\
        4\n\
 64\n\
     7\n\
 65\n\
   257\n\
 75\n\
     1\n\
175\n\
     1\n\
 42\n\
1.0\n\
 92\n\
        0\n\
 66\n\
   257\n\
 43\n\
1.0\n\
 76\n\
     1\n\
 77\n\
     6\n\
 78\n\
     2\n\
 67\n\
     7\n\
 79\n\
     5\n\
170\n\
     0\n\
171\n\
     0\n\
290\n\
     0\n\
174\n\
     0\n\
 93\n\
       13\n\
 44\n\
0.0\n\
173\n\
     0\n\
291\n\
     1\n\
 45\n\
0.0\n\
1001\n\
ACAD\n\
1000\n\
AcDbSavedByObjectVersion\n\
1070\n\
     0\n\
  0\n\
VISUALSTYLE\n\
  5\n\
38\n\
102\n\
{ACAD_REACTORS\n\
330\n\
2A\n\
102\n\
}\n\
330\n\
2A\n\
100\n\
AcDbVisualStyle\n\
  2\n\
Linepattern\n\
 70\n\
    14\n\
 71\n\
     2\n\
 72\n\
     2\n\
 73\n\
     0\n\
 90\n\
        0\n\
 40\n\
-0.6\n\
 41\n\
-30.0\n\
 62\n\
     5\n\
 63\n\
     7\n\
421\n\
 16777215\n\
 74\n\
     1\n\
 91\n\
        4\n\
 64\n\
     7\n\
 65\n\
   257\n\
 75\n\
     7\n\
175\n\
     7\n\
 42\n\
1.0\n\
 92\n\
        8\n\
 66\n\
     7\n\
 43\n\
1.0\n\
 76\n\
     1\n\
 77\n\
     6\n\
 78\n\
     2\n\
 67\n\
     7\n\
 79\n\
     5\n\
170\n\
     0\n\
171\n\
     0\n\
290\n\
     0\n\
174\n\
     0\n\
 93\n\
        1\n\
 44\n\
0.0\n\
173\n\
     0\n\
291\n\
     1\n\
 45\n\
0.0\n\
1001\n\
ACAD\n\
1000\n\
AcDbSavedByObjectVersion\n\
1070\n\
     0\n\
  0\n\
VISUALSTYLE\n\
  5\n\
33\n\
102\n\
{ACAD_REACTORS\n\
330\n\
2A\n\
102\n\
}\n\
330\n\
2A\n\
100\n\
AcDbVisualStyle\n\
  2\n\
Realistic\n\
 70\n\
     8\n\
 71\n\
     2\n\
 72\n\
     2\n\
 73\n\
     0\n\
 90\n\
        0\n\
 40\n\
-0.6\n\
 41\n\
-30.0\n\
 62\n\
     5\n\
 63\n\
     7\n\
421\n\
 16777215\n\
 74\n\
     1\n\
 91\n\
        0\n\
 64\n\
     7\n\
 65\n\
   257\n\
 75\n\
     1\n\
175\n\
     1\n\
 42\n\
1.0\n\
 92\n\
        8\n\
 66\n\
     8\n\
424\n\
  7895160\n\
 43\n\
1.0\n\
 76\n\
     1\n\
 77\n\
     6\n\
 78\n\
     2\n\
 67\n\
     7\n\
 79\n\
     5\n\
170\n\
     0\n\
171\n\
     0\n\
290\n\
     0\n\
174\n\
     0\n\
 93\n\
       13\n\
 44\n\
0.0\n\
173\n\
     0\n\
291\n\
     0\n\
 45\n\
0.0\n\
1001\n\
ACAD\n\
1000\n\
AcDbSavedByObjectVersion\n\
1070\n\
     0\n\
  0\n\
VISUALSTYLE\n\
  5\n\
37\n\
102\n\
{ACAD_REACTORS\n\
330\n\
2A\n\
102\n\
}\n\
330\n\
2A\n\
100\n\
AcDbVisualStyle\n\
  2\n\
Thicken\n\
 70\n\
    13\n\
 71\n\
     2\n\
 72\n\
     2\n\
 73\n\
     0\n\
 90\n\
        0\n\
 40\n\
-0.6\n\
 41\n\
-30.0\n\
 62\n\
     5\n\
 63\n\
     7\n\
421\n\
 16777215\n\
 74\n\
     1\n\
 91\n\
        4\n\
 64\n\
     7\n\
 65\n\
   257\n\
 75\n\
     1\n\
175\n\
     1\n\
 42\n\
1.0\n\
 92\n\
       12\n\
 66\n\
     7\n\
 43\n\
1.0\n\
 76\n\
     1\n\
 77\n\
     6\n\
 78\n\
     2\n\
 67\n\
     7\n\
 79\n\
     5\n\
170\n\
     0\n\
171\n\
     0\n\
290\n\
     0\n\
174\n\
     0\n\
 93\n\
        1\n\
 44\n\
0.0\n\
173\n\
     0\n\
291\n\
     1\n\
 45\n\
0.0\n\
1001\n\
ACAD\n\
1000\n\
AcDbSavedByObjectVersion\n\
1070\n\
     0\n\
  0\n\
ENDSEC\n\
";

  writeGroup( 0, "EOF" );
}

void QgsDxfExport::startSection()
{
  writeGroup( 0, "SECTION" );
}

void QgsDxfExport::endSection()
{
  writeGroup( 0, "ENDSEC" );
}

void QgsDxfExport::writePoint( const QgsPoint& pt, const QString& layer, QColor color, const QgsFeature* f, const QgsSymbolLayerV2* symbolLayer, const QgsSymbolV2* symbol )
{
#if 0
  // debug: draw rectangle for debugging
  const QgsMarkerSymbolLayerV2* msl = dynamic_cast< const QgsMarkerSymbolLayerV2* >( symbolLayer );
  if ( msl )
  {
    double halfSize = msl->size() * mapUnitScaleFactor( mSymbologyScaleDenominator,
                      msl->sizeUnit(), mMapUnits ) / 2.0;
    writeGroup( 0, "SOLID" );
    writeGroup( 8, layer );
    writeGroup( 62, 1 );
    writeGroup( 0, pt + QgsVector( -halfSize, -halfSize ) );
    writeGroup( 1, pt + QgsVector( halfSize, -halfSize ) );
    writeGroup( 2, pt + QgsVector( -halfSize, halfSize ) );
    writeGroup( 3, pt + QgsVector( halfSize,  halfSize ) );
  }
#endif // 0

  // insert block or write point directly?
  QHash< const QgsSymbolLayerV2*, QString >::const_iterator blockIt = mPointSymbolBlocks.find( symbolLayer );
  if ( !symbolLayer || blockIt == mPointSymbolBlocks.constEnd() )
  {
    // write symbol directly here
    const QgsMarkerSymbolLayerV2* msl = dynamic_cast< const QgsMarkerSymbolLayerV2* >( symbolLayer );
    if ( msl && symbol )
    {
      QgsRenderContext ct;
      QgsSymbolV2RenderContext ctx( ct, QgsSymbolV2::MapUnit, symbol->alpha(), false, symbol->renderHints(), f );
      if ( symbolLayer->writeDxf( *this, mapUnitScaleFactor( mSymbologyScaleDenominator, msl->sizeUnit(), mMapUnits ), layer, &ctx, f, QPointF( pt.x(), pt.y() ) ) )
      {
        return;
      }
    }
    writePoint( layer, color, pt ); // write default point symbol
  }
  else
  {
    // insert block reference
    writeGroup( 0, "INSERT" );
    writeHandle();
    writeGroup( 100, "AcDbEntity" );
    writeGroup( 100, "AcDbBlockReference" );
    writeGroup( 8, layer );
    writeGroup( 2, blockIt.value() ); // Block name
    writeGroup( 0, pt );  // Insertion point (in OCS)
  }
}

void QgsDxfExport::writePolyline( const QgsPolyline& line, const QString& layer, const QString& lineStyleName, QColor color,
                                  double width, bool polygon )
{
  writeGroup( 0, "LWPOLYLINE" );
  writeHandle();
  writeGroup( 8, layer );
  writeGroup( 100, "AcDbEntity" );
  writeGroup( 100, "AcDbPolyline" );
  writeGroup( 6, lineStyleName );
  writeGroup( color );

  writeGroup( 90, line.size() );

  writeGroup( 70, polygon ? 1 : 0 );
  writeGroup( 43, width );

  QgsPolyline::const_iterator lineIt = line.constBegin();
  for ( ; lineIt != line.constEnd(); ++lineIt )
  {
    writeGroup( 0, *lineIt );
  }
}

void QgsDxfExport::writePolygon( const QgsPolygon& polygon, const QString& layer, const QString& hatchPattern, QColor color )
{
  writeGroup( 0, "HATCH" );         // Entity type
  writeHandle();
  writeGroup( 330, mModelSpaceBR );
  writeGroup( 100, "AcDbEntity" );
  writeGroup( 8, layer );           // Layer name
  writeGroup( color );              // Color
  writeGroup( 100, "AcDbHatch" );

  writeGroup( 0, QgsPoint( 0, 0 ) ); // Elevation point (in OCS)
  writeGroup( 200, QgsPoint( 0, 0 ), 1.0 );

  writeGroup( 2, hatchPattern );  // Hatch pattern name
  writeGroup( 70, hatchPattern == "SOLID" ); // Solid fill flag (solid fill = 1; pattern fill = 0)
  writeGroup( 71, 0 );    // Associativity flag (associative = 1; non-associative = 0)

  writeGroup( 91, polygon.size() );  // Number of boundary paths (loops)
  for ( int i = 0; i < polygon.size(); ++i )
  {
    writeGroup( 92, 2 );   // Boundary path type flag (bit coded): 0 = Default; 1 = External; 2 = Polyline 4 = Derived; 8 = Textbox; 16 = Outermost
    writeGroup( 72, 0 );   // Has bulge flag
    writeGroup( 73, 1 );   // Is closed flag
    writeGroup( 93, polygon[i].size() ); // Number of edges in this boundary path (only if boundary is not a polyline

    for ( int j = 0; j < polygon[i].size(); ++j )
    {
      writeGroup( 0, polygon[i][j], 0.0, true ); // Vertex location (in OCS)
    }

    writeGroup( 97, 0 );   // Number of source boundary objects
  }

  writeGroup( 75, 0 );    // Hatch style: 0 = Hatch "odd parity" area (Normal style), 1 = Hatch outermost area only (Outer style), 2 = Hatch through entire area (Ignore style)
  writeGroup( 76, 1 );    // Hatch pattern type: 0 = User-defined; 1 = Predefined; 2 = Custom

  writeGroup( 98, 0 );    // Number of seed points
}

void QgsDxfExport::writeLine( const QgsPoint& pt1, const QgsPoint& pt2, const QString& layer, const QString& lineStyleName, QColor color, double width )
{
  QgsPolyline line( 2 );
  line[0] = pt1;
  line[1] = pt2;
  writePolyline( line, layer, lineStyleName, color, width, false );
}

void QgsDxfExport::writePoint( const QString& layer, QColor color, const QgsPoint& pt )
{
  writeGroup( 0, "POINT" );
  writeHandle();
  writeGroup( 100, "AcDbEntity" );
  writeGroup( 100, "AcDbPoint" );
  writeGroup( 8, layer );
  writeGroup( color );
  writeGroup( 0, pt );
}

void QgsDxfExport::writeFilledCircle( const QString &layer, QColor color, const QgsPoint &pt, double radius )
{
  writeGroup( 0, "HATCH" );                     // Entity type
  writeHandle();
  writeGroup( 330, mModelSpaceBR );
  writeGroup( 100, "AcDbEntity" );
  writeGroup( 100, "AcDbHatch" );

  writeGroup( 8, layer );   // Layer name
  writeGroup( 0, QgsPoint( 0, 0 ), 0.0 );  // Elevation point (in OCS)
  writeGroup( 200, QgsPoint( 0, 0 ), 1.0 );

  writeGroup( 2, "SOLID" );  // Hatch pattern name
  writeGroup( 70, 1 );       // Solid fill flag (solid fill = 1; pattern fill = 0)
  writeGroup( 71, 0 );       // Associativity flag (associative = 1; non-associative = 0)

  writeGroup( color );       // Color (0 by block, 256 by layer)

  writeGroup( 91, 1 );       // Number of boundary paths (loops)

  writeGroup( 92, 7 );       // Boundary path type flag (bit coded): 0 = Default; 1 = External; 2 = Polyline 4 = Derived; 8 = Textbox; 16 = Outermost
  writeGroup( 72, 2 );
  writeGroup( 73, 1 );       // Is closed flag
  writeGroup( 93, 2 );       // Number of polyline vertices

  writeGroup( 0, QgsPoint( pt.x() - radius, pt.y() ) );
  writeGroup( 42, 1.0 );

  writeGroup( 0, QgsPoint( pt.x() + radius, pt.y() ) );
  writeGroup( 42, 1.0 );

  writeGroup( 97, 0 );       // Number of source boundary objects

  writeGroup( 75, 1 );       // Hatch style: 0 = Hatch "odd parity" area (Normal style), 1 = Hatch outermost area only (Outer style), 2 = Hatch through entire area (Ignore style)
  writeGroup( 76, 1 );       // Hatch pattern type: 0 = User-defined; 1 = Predefined; 2 = Custom
  writeGroup( 47, 0.0059696789328105 ); // Pixel size

  writeGroup( 98, 0 );       // Number of seed points
}

void QgsDxfExport::writeCircle( const QString& layer, QColor color, const QgsPoint& pt, double radius, const QString &lineStyleName, double width )
{
  writeGroup( 0, "LWPOLYLINE" );
  writeHandle();
  writeGroup( 330, mModelSpaceBR );
  writeGroup( 8, layer );
  writeGroup( 100, "AcDbEntity" );
  writeGroup( 100, "AcDbPolyline" );
  writeGroup( 6, lineStyleName );
  writeGroup( color );

  writeGroup( 90, 2 );

  writeGroup( 70, 1 );
  writeGroup( 43, width );

  writeGroup( 0, QgsPoint( pt.x() - radius, pt.y() ) );
  writeGroup( 42, 1.0 );
  writeGroup( 0, QgsPoint( pt.x() + radius, pt.y() ) );
  writeGroup( 42, 1.0 );
}

void QgsDxfExport::writeText( const QString& layer, const QString& text, const QgsPoint& pt, double size, double angle, QColor color )
{
  writeGroup( 0, "TEXT" );
  writeHandle();
  writeGroup( 100, "AcDbEntity" );
  writeGroup( 100, "AcDbText" );
  writeGroup( 8, layer );
  writeGroup( color );
  writeGroup( 0, pt );
  writeGroup( 40, size );
  writeGroup( 1, text );
  writeGroup( 50, angle );
  writeGroup( 7, "STANDARD" ); // so far only support for standard font
}

void QgsDxfExport::writeMText( const QString& layer, const QString& text, const QgsPoint& pt, double width, double angle, QColor color )
{
  if ( !mTextStream.codec()->canEncode( text ) )
  {
    // TODO return error
    return;
  }

  writeGroup( 0, "MTEXT" );
  writeHandle();
  writeGroup( 100, "AcDbEntity" );
  writeGroup( 100, "AcDbMText" );
  writeGroup( 8, layer );
  writeGroup( color );

  writeGroup( 0, pt );

  QString t( text );
  while ( t.length() > 250 )
  {
    writeGroup( 3, t.left( 250 ) );
    t = t.mid( 250 );
  }
  writeGroup( 1, text );

  writeGroup( 50, angle );  // Rotation angle in radians
  writeGroup( 41, width * 1.1 );  // Reference rectangle width

  // Attachment point:
  // 1 2 3
  // 4 5 6
  // 7 8 9
  writeGroup( 71, 7 );

  writeGroup( 7, "STANDARD" ); // so far only support for standard font
}

void QgsDxfExport::writeSolid( const QString& layer, QColor color, const QgsPoint& pt1, const QgsPoint& pt2, const QgsPoint& pt3, const QgsPoint& pt4 )
{
  writeGroup( 0, "SOLID" );
  writeHandle();
  writeGroup( 100, "AcDbEntity" );
  writeGroup( 100, "AcDbTrace" );
  writeGroup( 8, layer );
  writeGroup( color );
  writeGroup( 0, pt1 );
  writeGroup( 1, pt2 );
  writeGroup( 2, pt3 );
  writeGroup( 3, pt4 );
}

void QgsDxfExport::writeVertex( const QgsPoint& pt, const QString& layer )
{
  writeGroup( 0, "VERTEX" );
  writeHandle();
  writeGroup( 100, "AcDbEntity" );
  writeGroup( 100, "AcDbVertex" );
  writeGroup( 100, "AcDb2dVertex" );
  writeGroup( 8, layer );
  writeGroup( 0, pt, 0.0, false );
}

QgsRectangle QgsDxfExport::dxfExtent() const
{
  QgsRectangle extent;
  QList< QPair<QgsVectorLayer*, int> >::const_iterator layerIt = mLayers.constBegin();
  for ( ; layerIt != mLayers.constEnd(); ++layerIt )
  {
    if ( layerIt->first )
    {
      if ( extent.isEmpty() )
      {
        extent = layerIt->first->extent();
      }
      else
      {
        QgsRectangle layerExtent = layerIt->first->extent();
        extent.combineExtentWith( &layerExtent );
      }
    }
  }
  return extent;
}

void QgsDxfExport::addFeature( const QgsSymbolV2RenderContext& ctx, const QString& layer, const QgsSymbolLayerV2* symbolLayer, const QgsSymbolV2* symbol )
{
  const QgsFeature* fet = ctx.feature();
  if ( !fet )
    return;

  QgsGeometry *geom = fet->geometry();
  if ( !geom )
    return;

  QGis::WkbType geometryType = geom->wkbType();

  QColor penColor;
  QColor brushColor;
  if ( mSymbologyExport != NoSymbology )
  {
    penColor = colorFromSymbolLayer( symbolLayer, ctx );
    brushColor = symbolLayer->dxfBrushColor( ctx );
  }

  Qt::PenStyle penStyle( Qt::SolidLine );
  Qt::BrushStyle brushStyle( Qt::NoBrush );
  double width = -1;
  double offset = 0.0;
  if ( mSymbologyExport != NoSymbology && symbolLayer )
  {
    width = symbolLayer->dxfWidth( *this, ctx );
    offset = symbolLayer->dxfOffset( *this, ctx );
    penStyle = symbolLayer->dxfPenStyle();
    brushStyle = symbolLayer->dxfBrushStyle();

    if ( qgsDoubleNear( offset, 0.0 ) )
      offset = 0.0;
  }

  QString lineStyleName = "CONTINUOUS";
  if ( mSymbologyExport != NoSymbology )
  {
    lineStyleName = lineStyleFromSymbolLayer( symbolLayer );
  }

  // single point
  if ( geometryType == QGis::WKBPoint || geometryType == QGis::WKBPoint25D )
  {
    writePoint( geom->asPoint(), layer, penColor, fet, symbolLayer, symbol );
    return;
  }

  // multipoint
  if ( geometryType == QGis::WKBMultiPoint || geometryType == QGis::WKBMultiPoint25D )
  {
    QgsMultiPoint multiPoint = geom->asMultiPoint();
    QgsMultiPoint::const_iterator it = multiPoint.constBegin();
    for ( ; it != multiPoint.constEnd(); ++it )
    {
      writePoint( *it, layer, penColor, fet, symbolLayer, symbol );
    }

    return;
  }

  if ( penStyle != Qt::NoPen )
  {
    // single line
    if ( geometryType == QGis::WKBLineString || geometryType == QGis::WKBLineString25D )
    {
      QgsGeometry *offsetLine = offset == 0.0 ? geom : geom->offsetCurve( offset, 0, GEOSBUF_JOIN_MITRE, 2.0 );
      if ( !offsetLine )
        offsetLine = geom;

      writePolyline( offsetLine->asPolyline(), layer, lineStyleName, penColor, width, false );

      if ( offsetLine != geom )
        delete offsetLine;
    }

    // multiline
    if ( geometryType == QGis::WKBMultiLineString || geometryType == QGis::WKBMultiLineString25D )
    {
      QgsGeometry *offsetLine = offset == 0.0 ? geom : geom->offsetCurve( offset, 0, GEOSBUF_JOIN_MITRE, 2.0 );
      if ( !offsetLine )
        offsetLine = geom;

      QgsMultiPolyline multiLine = offsetLine->asMultiPolyline();
      QgsMultiPolyline::const_iterator lIt = multiLine.constBegin();
      for ( ; lIt != multiLine.constEnd(); ++lIt )
      {
        writePolyline( *lIt, layer, lineStyleName, penColor, width, false );
      }

      if ( offsetLine != geom )
        delete offsetLine;
    }

    // polygon
    if ( geometryType == QGis::WKBPolygon || geometryType == QGis::WKBPolygon25D )
    {
      QgsGeometry *offsetPolygon = offset == 0.0 ? geom : geom->buffer( -offset, 0, GEOSBUF_CAP_FLAT, GEOSBUF_JOIN_MITRE, 2.0 );
      if ( !offsetPolygon )
        offsetPolygon = geom;

      QgsPolygon polygon = offsetPolygon->asPolygon();
      QgsPolygon::const_iterator polyIt = polygon.constBegin();
      for ( ; polyIt != polygon.constEnd(); ++polyIt ) // iterate over rings
      {
        writePolyline( *polyIt, layer, lineStyleName, penColor, width, false );
      }

      if ( offsetPolygon != geom )
        delete offsetPolygon;
    }

    // multipolygon or polygon
    if ( geometryType == QGis::WKBMultiPolygon || geometryType == QGis::WKBMultiPolygon25D )
    {
      QgsGeometry *offsetPolygon = offset == 0.0 ? geom : geom->buffer( -offset, 0, GEOSBUF_CAP_FLAT, GEOSBUF_JOIN_MITRE, 2.0 );
      if ( !offsetPolygon )
        offsetPolygon = geom;

      QgsMultiPolygon mp = offsetPolygon->asMultiPolygon();
      QgsMultiPolygon::const_iterator mpIt = mp.constBegin();
      for ( ; mpIt != mp.constEnd(); ++mpIt )
      {
        QgsPolygon::const_iterator polyIt = mpIt->constBegin();
        for ( ; polyIt != mpIt->constEnd(); ++polyIt )
        {
          writePolyline( *polyIt, layer, lineStyleName, penColor, width, true );
        }
      }

      if ( offsetPolygon != geom )
        delete offsetPolygon;
    }
  }

  if ( brushStyle != Qt::NoBrush )
  {
    // polygon
    if ( geometryType == QGis::WKBPolygon || geometryType == QGis::WKBPolygon25D )
    {
      QgsPolygon polygon = geom->asPolygon();
      writePolygon( polygon, layer, "SOLID", brushColor );
    }

    // multipolygon or polygon
    if ( geometryType == QGis::WKBMultiPolygon || geometryType == QGis::WKBMultiPolygon25D )
    {
      QgsMultiPolygon mp = geom->asMultiPolygon();
      QgsMultiPolygon::const_iterator mpIt = mp.constBegin();
      for ( ; mpIt != mp.constEnd(); ++mpIt )
      {
        writePolygon( *mpIt, layer, "SOLID", brushColor );
      }
    }
  }
}

QColor QgsDxfExport::colorFromSymbolLayer( const QgsSymbolLayerV2* symbolLayer, const QgsSymbolV2RenderContext& ctx )
{
  if ( !symbolLayer )
    return QColor();

  return symbolLayer->dxfColor( ctx );
}

QString QgsDxfExport::lineStyleFromSymbolLayer( const QgsSymbolLayerV2* symbolLayer )
{
  QString lineStyleName = "CONTINUOUS";
  if ( !symbolLayer )
  {
    return lineStyleName;
  }

  QHash< const QgsSymbolLayerV2*, QString >::const_iterator lineTypeIt = mLineStyles.find( symbolLayer );
  if ( lineTypeIt != mLineStyles.constEnd() )
  {
    lineStyleName = lineTypeIt.value();
    return lineStyleName;
  }
  else
  {
    return lineNameFromPenStyle( symbolLayer->dxfPenStyle() );
  }
}

int QgsDxfExport::closestColorMatch( QRgb pixel )
{
  int idx = 0;
  int current_distance = INT_MAX;
  for ( size_t i = 1; i < sizeof( mDxfColors ) / sizeof( *mDxfColors ); ++i )
  {
    int dist = color_distance( pixel, i );
    if ( dist < current_distance )
    {
      current_distance = dist;
      idx = i;
      if ( dist == 0 )
        break;
    }
  }
  return idx;
}

int QgsDxfExport::color_distance( QRgb p1, int index )
{
  if ( index > 255 || index < 0 )
  {
    return 0;
  }

  double redDiff = qRed( p1 ) - mDxfColors[index][0];
  double greenDiff = qGreen( p1 ) - mDxfColors[index][1];
  double blueDiff = qBlue( p1 ) - mDxfColors[index][2];
#if 0
  QgsDebugMsg( QString( "color_distance( r:%1 g:%2 b:%3 <=> i:%4 r:%5 g:%6 b:%7 ) => %8" )
               .arg( qRed( p1 ) ).arg( qGreen( p1 ) ).arg( qBlue( p1 ) )
               .arg( index )
               .arg( mDxfColors[index][0] )
               .arg( mDxfColors[index][1] )
               .arg( mDxfColors[index][2] )
               .arg( redDiff * redDiff + greenDiff * greenDiff + blueDiff * blueDiff ) );
#endif
  return redDiff * redDiff + greenDiff * greenDiff + blueDiff * blueDiff;
}

QRgb QgsDxfExport::createRgbEntry( qreal r, qreal g, qreal b )
{
  return QColor::fromRgbF( r, g, b ).rgb();
}

QgsRenderContext QgsDxfExport::renderContext() const
{
  QgsRenderContext context;
  context.setRendererScale( mSymbologyScaleDenominator );
  return context;
}

double QgsDxfExport::mapUnitScaleFactor( double scaleDenominator, QgsSymbolV2::OutputUnit symbolUnits, QGis::UnitType mapUnits )
{
  if ( symbolUnits == QgsSymbolV2::MapUnit )
  {
    return 1.0;
  }
  // MM symbol unit
  return scaleDenominator * QGis::fromUnitToUnitFactor( QGis::Meters, mapUnits ) / 1000.0;
}

QList< QPair< QgsSymbolLayerV2*, QgsSymbolV2* > > QgsDxfExport::symbolLayers()
{
  QList< QPair< QgsSymbolLayerV2*, QgsSymbolV2* > > symbolLayers;

  QList< QPair< QgsVectorLayer*, int> >::iterator lIt = mLayers.begin();
  for ( ; lIt != mLayers.end(); ++lIt )
  {
    // cast to vector layer
    QgsVectorLayer* vl = lIt->first;
    if ( !vl )
    {
      continue;
    }

    // get rendererv2
    QgsFeatureRendererV2* r = vl->rendererV2();
    if ( !r )
    {
      continue;
    }

    // get all symbols
    QgsSymbolV2List symbols = r->symbols();
    QgsSymbolV2List::iterator symbolIt = symbols.begin();
    for ( ; symbolIt != symbols.end(); ++symbolIt )
    {
      int maxSymbolLayers = ( *symbolIt )->symbolLayerCount();
      if ( mSymbologyExport != SymbolLayerSymbology )
      {
        maxSymbolLayers = 1;
      }
      for ( int i = 0; i < maxSymbolLayers; ++i )
      {
        symbolLayers.append( qMakePair(( *symbolIt )->symbolLayer( i ), *symbolIt ) );
      }
    }
  }

  return symbolLayers;
}

void QgsDxfExport::writeDefaultLinetypes()
{
  // continuous (Qt solid line)
  foreach ( QString ltype, QStringList() << "ByLayer" << "ByBlock" << "CONTINUOUS" )
  {
    writeGroup( 0, "LTYPE" );
    writeHandle();
    writeGroup( 100, "AcDbSymbolTableRecord" );
    writeGroup( 100, "AcDbLinetypeTableRecord" );
    writeGroup( 2, ltype );
    writeGroup( 70, 64 );
    writeGroup( 3, "Defaultstyle" );
    writeGroup( 72, 65 );
    writeGroup( 73, 0 );
    writeGroup( 40, 0.0 );
  }

  double das = dashSize();
  double dss = dashSeparatorSize();
  double dos = dotSize();

  QVector<qreal> dashVector( 2 );
  dashVector[0] = das;
  dashVector[1] = dss;
  writeLinetype( "DASH", dashVector, QgsSymbolV2::MapUnit );

  QVector<qreal> dotVector( 2 );
  dotVector[0] = dos;
  dotVector[1] = dss;
  writeLinetype( "DOT", dotVector, QgsSymbolV2::MapUnit );

  QVector<qreal> dashDotVector( 4 );
  dashDotVector[0] = das;
  dashDotVector[1] = dss;
  dashDotVector[2] = dos;
  dashDotVector[3] = dss;
  writeLinetype( "DASHDOT", dashDotVector, QgsSymbolV2::MapUnit );

  QVector<qreal> dashDotDotVector( 6 );
  dashDotDotVector[0] = das;
  dashDotDotVector[1] = dss;
  dashDotDotVector[2] = dos;
  dashDotDotVector[3] = dss;
  dashDotDotVector[4] = dos;
  dashDotDotVector[5] = dss;
  writeLinetype( "DASHDOTDOT", dashDotDotVector, QgsSymbolV2::MapUnit );
}

void QgsDxfExport::writeSymbolLayerLinetype( const QgsSymbolLayerV2* symbolLayer )
{
  if ( !symbolLayer )
  {
    return;
  }

  QgsSymbolV2::OutputUnit unit;
  QVector<qreal> customLinestyle = symbolLayer->dxfCustomDashPattern( unit );
  if ( customLinestyle.size() > 0 )
  {
    QString name = QString( "symbolLayer%1" ).arg( mSymbolLayerCounter++ );
    writeLinetype( name, customLinestyle, unit );
    mLineStyles.insert( symbolLayer, name );
  }
}

int QgsDxfExport::nLineTypes( const QList< QPair< QgsSymbolLayerV2*, QgsSymbolV2* > >& symbolLayers )
{
  int nLineTypes = 0;
  QList< QPair< QgsSymbolLayerV2*, QgsSymbolV2*> >::const_iterator slIt = symbolLayers.constBegin();
  for ( ; slIt != symbolLayers.constEnd(); ++slIt )
  {
    const QgsSimpleLineSymbolLayerV2* simpleLine = dynamic_cast< const QgsSimpleLineSymbolLayerV2* >( slIt->first );
    if ( simpleLine )
    {
      if ( simpleLine->useCustomDashPattern() )
      {
        ++nLineTypes;
      }
    }
  }
  return nLineTypes;
}

void QgsDxfExport::writeLinetype( const QString& styleName, const QVector<qreal>& pattern, QgsSymbolV2::OutputUnit u )
{
  double length = 0;
  QVector<qreal>::const_iterator dashIt = pattern.constBegin();
  for ( ; dashIt != pattern.constEnd(); ++dashIt )
  {
    length += ( *dashIt * mapUnitScaleFactor( mSymbologyScaleDenominator, u, mMapUnits ) );
  }

  writeGroup( 0, "LTYPE" );
  writeHandle();
  // 330 5
  writeGroup( 100, "AcDbSymbolTableRecord" );
  writeGroup( 100, "AcDbLinetypeTableRecord" );
  writeGroup( 2, styleName );
  writeGroup( 70, 64 ); // 0?
  writeGroup( 3, "" );
  writeGroup( 72, 65 );
  writeGroup( 73, pattern.size() );
  writeGroup( 40, length );

  dashIt = pattern.constBegin();
  bool isGap = false;
  for ( ; dashIt != pattern.constEnd(); ++dashIt )
  {
    // map units or mm?
    double segmentLength = ( isGap ? -*dashIt : *dashIt );
    segmentLength *= mapUnitScaleFactor( mSymbologyScaleDenominator, u, mMapUnits );
    writeGroup( 49, segmentLength );
    writeGroup( 74, 0 );
    isGap = !isGap;
  }
}

bool QgsDxfExport::hasDataDefinedProperties( const QgsSymbolLayerV2* sl, const QgsSymbolV2* symbol )
{
  if ( !sl || !symbol )
  {
    return false;
  }

  if ( symbol->renderHints() & QgsSymbolV2::DataDefinedSizeScale ||
       symbol->renderHints() & QgsSymbolV2::DataDefinedRotation )
  {
    return true;
  }

  return sl->hasDataDefinedProperties();
}

double QgsDxfExport::dashSize() const
{
  double size = mSymbologyScaleDenominator * 0.002;
  return sizeToMapUnits( size );
}

double QgsDxfExport::dotSize() const
{
  double size = mSymbologyScaleDenominator * 0.0006;
  return sizeToMapUnits( size );
}

double QgsDxfExport::dashSeparatorSize() const
{
  double size = mSymbologyScaleDenominator * 0.0006;
  return sizeToMapUnits( size );
}

double QgsDxfExport::sizeToMapUnits( double s ) const
{
  double size = s * QGis::fromUnitToUnitFactor( QGis::Meters, mMapUnits );
  return size;
}

QString QgsDxfExport::lineNameFromPenStyle( Qt::PenStyle style )
{
  switch ( style )
  {
    case Qt::DashLine:
      return "DASH";
    case Qt::DotLine:
      return "DOT";
    case Qt::DashDotLine:
      return "DASHDOT";
    case Qt::DashDotDotLine:
      return "DASHDOTDOT";
    case Qt::SolidLine:
    default:
      return "CONTINUOUS";
  }
}

QString QgsDxfExport::dxfLayerName( const QString& name )
{
  if ( name.isEmpty() )
    return "0";

  // dxf layers can be max 255 characters long
  QString layerName = name.left( 255 );

  // replaced restricted characters with underscore
  // < > / \ " : ; ? * | = '
  // See http://docs.autodesk.com/ACD/2010/ENU/AutoCAD%202010%20User%20Documentation/index.html?url=WS1a9193826455f5ffa23ce210c4a30acaf-7345.htm,topicNumber=d0e41665
  layerName.replace( "<", "_" );
  layerName.replace( ">", "_" );
  layerName.replace( "/", "_" );
  layerName.replace( "\\", "_" );
  layerName.replace( "\"", "_" );
  layerName.replace( ":", "_" );
  layerName.replace( ";", "_" );
  layerName.replace( "?", "_" );
  layerName.replace( "*", "_" );
  layerName.replace( "|", "_" );
  layerName.replace( "=", "_" );
  layerName.replace( "\'", "_" );

  return layerName;
}

bool QgsDxfExport::layerIsScaleBasedVisible( const QgsMapLayer* layer ) const
{
  if ( !layer )
    return false;

  if ( mSymbologyExport == QgsDxfExport::NoSymbology || !layer->hasScaleBasedVisibility() )
    return true;

  return layer->minimumScale() < mSymbologyScaleDenominator &&
         layer->maximumScale() > mSymbologyScaleDenominator;
}

QString QgsDxfExport::layerName( const QString &id, const QgsFeature &f ) const
{
  QList< QPair<QgsVectorLayer*, int> >::const_iterator layerIt = mLayers.constBegin();
  for ( ; layerIt != mLayers.constEnd(); ++layerIt )
  {
    if ( layerIt->first && layerIt->first->id() == id )
      return dxfLayerName( layerIt->second < 0 ? layerIt->first->name() : f.attribute( layerIt->second ).toString() );
  }

  return "0";
}

QString QgsDxfExport::dxfEncoding( const QString &name )
{
  foreach ( QByteArray codec, QTextCodec::availableCodecs() )
  {
    if ( name != codec )
      continue;

    int i;
    for ( i = 0; i < ( int )( sizeof( mDxfEncodings ) / sizeof( *mDxfEncodings ) ) && name != mDxfEncodings[i][1]; ++i )
      ;

    if ( i == ( int )( sizeof( mDxfEncodings ) / sizeof( *mDxfEncodings ) ) )
      continue;

    return mDxfEncodings[i][0];
  }

  return QString::null;
}

QStringList QgsDxfExport::encodings()
{
  QStringList encodings;
  foreach ( QByteArray codec, QTextCodec::availableCodecs() )
  {
    int i;
    for ( i = 0; i < ( int )( sizeof( mDxfEncodings ) / sizeof( *mDxfEncodings ) ) && strcmp( codec.data(), mDxfEncodings[i][1] ) != 0; ++i )
      ;

    if ( i < ( int )( sizeof( mDxfEncodings ) / sizeof( *mDxfEncodings ) ) )
      encodings << codec.data();
  }
  return encodings;
}
