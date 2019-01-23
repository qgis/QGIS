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
#include "qgsgeometrygeneratorsymbollayer.h"
#include "qgsvectordataprovider.h"
#include "qgspointxy.h"
#include "qgsproject.h"
#include "qgsrenderer.h"
#include "qgssymbollayer.h"
#include "qgsfillsymbollayer.h"
#include "qgsfeatureiterator.h"
#include "qgslinesymbollayer.h"
#include "qgsvectorlayer.h"
#include "qgsunittypes.h"
#include "qgstextlabelfeature.h"
#include "qgslogger.h"
#include "qgsmaplayerstyle.h"
#include "qgsmaplayerstylemanager.h"

#include "qgswkbtypes.h"
#include "qgspoint.h"
#include "qgsgeos.h"

#include "pal/feature.h"
#include "pal/pointset.h"
#include "pal/labelposition.h"

#include <QIODevice>

// dxf color palette
int QgsDxfExport::sDxfColors[][3] =
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

const char *QgsDxfExport::DXF_ENCODINGS[][2] =
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
  { "ANSI_949", "CP949" },
  { "ANSI_950", "CP950" },
//  { "ANSI_1361", "" },
//  { "ANSI_1200", "" },
  { "ANSI_1258", "CP1258" },
};

QgsDxfExport::QgsDxfExport( const QgsDxfExport &dxfExport )
{
  *this = dxfExport;
}

QgsDxfExport &QgsDxfExport::operator=( const QgsDxfExport &dxfExport )
{
  mMapSettings = dxfExport.mMapSettings;
  mLayerNameAttribute = dxfExport.mLayerNameAttribute;
  mSymbologyScale = dxfExport.mSymbologyScale;
  mSymbologyExport = dxfExport.mSymbologyExport;
  mMapUnits = dxfExport.mMapUnits;
  mLayerTitleAsName = dxfExport.mLayerTitleAsName;
  mSymbolLayerCounter = 0; // internal counter
  mNextHandleId = 0;
  mBlockCounter = 0;
  mCrs = QgsCoordinateReferenceSystem();
  mFactor = dxfExport.mFactor;
  mForce2d = dxfExport.mForce2d;
  return *this;
}

void QgsDxfExport::setMapSettings( const QgsMapSettings &settings )
{
  mMapSettings = settings;
}

void QgsDxfExport::setFlags( QgsDxfExport::Flags flags )
{
  mFlags = flags;
}

QgsDxfExport::Flags QgsDxfExport::flags() const
{
  return mFlags;
}

void QgsDxfExport::addLayers( const QList<DxfLayer> &layers )
{
  QList<QgsMapLayer *> layerList;

  mLayerNameAttribute.clear();

  QList< DxfLayer >::const_iterator layerIt = layers.constBegin();
  for ( ; layerIt != layers.constEnd(); ++layerIt )
  {
    layerList << layerIt->layer();
    if ( layerIt->layerOutputAttributeIndex() >= 0 )
      mLayerNameAttribute.insert( layerIt->layer()->id(), layerIt->layerOutputAttributeIndex() );
  }

  mMapSettings.setLayers( layerList );
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

void QgsDxfExport::writeGroup( int code, const QString &s )
{
  writeGroupCode( code );
  writeString( s );
}

void QgsDxfExport::writeGroup( int code, const QgsPoint &p )
{
  writeGroup( code + 10, p.x() );
  writeGroup( code + 20, p.y() );
  if ( !mForce2d && p.is3D() && std::isfinite( p.z() ) )
    writeGroup( code + 30, p.z() );
}

void QgsDxfExport::writeGroup( const QColor &color, int exactMatchCode, int rgbCode, int transparencyCode )
{
  int minDistAt = -1;
  int minDist = std::numeric_limits<int>::max();

  for ( int i = 1; i < static_cast< int >( sizeof( sDxfColors ) / sizeof( *sDxfColors ) ) && minDist > 0; ++i )
  {
    int dist = color_distance( color.rgba(), i );
    if ( dist >= minDist )
      continue;

    minDistAt = i;
    minDist = dist;
  }

  if ( minDist == 0 && minDistAt != 7 )
  {
    // exact full opaque match, not black/white
    writeGroup( exactMatchCode, minDistAt );
    if ( color.alpha() == 255 )
      return;
  }

  int c = ( color.red() & 0xff ) * 0x10000 + ( color.green() & 0xff ) * 0x100 + ( color.blue() & 0xff );
  writeGroup( rgbCode, c );
  if ( transparencyCode != -1 && color.alpha() < 255 )
    writeGroup( transparencyCode, 0x2000000 | color.alpha() );
}

void QgsDxfExport::writeGroupCode( int code )
{
  mTextStream << QStringLiteral( "%1\n" ).arg( code, 3, 10, QChar( ' ' ) );
}

void QgsDxfExport::writeInt( int i )
{
  mTextStream << QStringLiteral( "%1\n" ).arg( i, 6, 10, QChar( ' ' ) );
}

void QgsDxfExport::writeDouble( double d )
{
  QString s( qgsDoubleToString( d ) );
  if ( !s.contains( '.' ) )
    s += QLatin1String( ".0" );
  mTextStream << s << '\n';
}

void QgsDxfExport::writeString( const QString &s )
{
  mTextStream << s << '\n';
}

int QgsDxfExport::writeToFile( QIODevice *d, const QString &encoding )
{
  if ( !d )
  {
    return 1;
  }

  if ( !d->isOpen() && !d->open( QIODevice::WriteOnly | QIODevice::Truncate ) )
  {
    return 2;
  }

  mTextStream.setDevice( d );
  mTextStream.setCodec( encoding.toLocal8Bit() );

  if ( mCrs.isValid() )
    mMapSettings.setDestinationCrs( mCrs );

  if ( mExtent.isEmpty() )
  {
    const QList< QgsMapLayer * > layers = mMapSettings.layers();
    for ( QgsMapLayer *ml : layers )
    {
      QgsVectorLayer *vl = qobject_cast<QgsVectorLayer *>( ml );
      if ( !vl )
        continue;

      QgsRectangle layerExtent = vl->extent();
      if ( layerExtent.isEmpty() )
        continue;

      layerExtent = mMapSettings.layerToMapCoordinates( vl, layerExtent );

      if ( mExtent.isEmpty() )
      {
        mExtent = layerExtent;
      }
      else
      {
        mExtent.combineExtentWith( layerExtent );
      }
    }
  }

  QgsUnitTypes::DistanceUnit mapUnits = mCrs.mapUnits();
  mMapSettings.setExtent( mExtent );

  int dpi = 96;
  mFactor = 1000 * dpi / mSymbologyScale / 25.4 * QgsUnitTypes::fromUnitToUnitFactor( mapUnits, QgsUnitTypes::DistanceMeters );
  mMapSettings.setOutputSize( QSize( mExtent.width() * mFactor, mExtent.height() * mFactor ) );
  mMapSettings.setOutputDpi( dpi );

  writeHeader( dxfEncoding( encoding ) );
  writeTables();
  writeBlocks();
  writeEntities();
  writeEndFile();

  return 0;
}

QgsUnitTypes::DistanceUnit QgsDxfExport::mapUnits() const
{
  return mMapUnits;
}

void QgsDxfExport::writeHeader( const QString &codepage )
{
  writeGroup( 999, QStringLiteral( "DXF created from QGIS" ) );

  startSection();
  writeGroup( 2, QStringLiteral( "HEADER" ) );

  // ACADVER
  writeGroup( 9, QStringLiteral( "$ACADVER" ) );
  writeGroup( 1, QStringLiteral( "AC1015" ) );

  // EXTMIN
  writeGroup( 9, QStringLiteral( "$EXTMIN" ) );
  writeGroup( 0, QgsPoint( QgsWkbTypes::PointZ, mExtent.xMinimum(), mExtent.yMinimum() ) );

  // EXTMAX
  writeGroup( 9, QStringLiteral( "$EXTMAX" ) );
  writeGroup( 0, QgsPoint( QgsWkbTypes::PointZ, mExtent.xMaximum(), mExtent.yMaximum() ) );

  // Global linetype scale
  writeGroup( 9, QStringLiteral( "$LTSCALE" ) );
  writeGroup( 40, 1.0 );

  // Point display mode (33 = circle)
  writeGroup( 9, QStringLiteral( "$PDMODE" ) );
  writeGroup( 70, 33 );

  // Point display size
  writeGroup( 9, QStringLiteral( "$PDSIZE" ) );
  writeGroup( 40, 1 );

  // Controls paper space linetype scaling (1 = No special linetype scaling, 0 = Viewport scaling governs linetype scaling)
  writeGroup( 9, QStringLiteral( "$PSLTSCALE" ) );
  writeGroup( 70, 0 );

  writeGroup( 9, QStringLiteral( "$HANDSEED" ) );
  writeGroup( 5, DXF_HANDMAX );

  writeGroup( 9, QStringLiteral( "$DWGCODEPAGE" ) );
  writeGroup( 3, codepage );

  endSection();
}

int QgsDxfExport::writeHandle( int code, int handle )
{
  if ( handle == 0 )
    handle = mNextHandleId++;

  Q_ASSERT_X( handle < DXF_HANDMAX, "QgsDxfExport::writeHandle(int, int)", "DXF handle too large" );

  writeGroup( code, QStringLiteral( "%1" ).arg( handle, 0, 16 ) );
  return handle;
}

void QgsDxfExport::writeTables()
{
  startSection();
  writeGroup( 2, QStringLiteral( "TABLES" ) );

  // Iterate through all layers and get symbol layer pointers
  QgsRenderContext context = renderContext();
  QList< QPair< QgsSymbolLayer *, QgsSymbol * > > slList;
  if ( mSymbologyExport != NoSymbology )
  {
    slList = symbolLayers( context );
  }

  // Line types
  mLineStyles.clear();
  writeGroup( 0, QStringLiteral( "TABLE" ) );
  writeGroup( 2, QStringLiteral( "LTYPE" ) );
  writeHandle();
  writeGroup( 100, QStringLiteral( "AcDbSymbolTable" ) );
  writeGroup( 70, nLineTypes( slList ) + 5 );

  writeDefaultLinetypes();

  // Add custom linestyles
  QList< QPair< QgsSymbolLayer *, QgsSymbol *> >::const_iterator slIt = slList.constBegin();
  for ( ; slIt != slList.constEnd(); ++slIt )
  {
    writeSymbolLayerLinetype( slIt->first );
  }

  writeGroup( 0, QStringLiteral( "ENDTAB" ) );

  // BLOCK_RECORD
  writeGroup( 0, QStringLiteral( "TABLE" ) );
  writeGroup( 2, QStringLiteral( "BLOCK_RECORD" ) );
  writeHandle();

  writeGroup( 100, QStringLiteral( "AcDbSymbolTable" ) );
  writeGroup( 70, 0 );

  const QStringList blockStrings = QStringList() << QStringLiteral( "*Model_Space" ) << QStringLiteral( "*Paper_Space" ) << QStringLiteral( "*Paper_Space0" );
  for ( const QString &block : blockStrings )
  {
    writeGroup( 0, QStringLiteral( "BLOCK_RECORD" ) );
    mBlockHandles.insert( block, writeHandle() );
    writeGroup( 100, QStringLiteral( "AcDbSymbolTableRecord" ) );
    writeGroup( 100, QStringLiteral( "AcDbBlockTableRecord" ) );
    writeGroup( 2, block );
  }

  int i = 0;
  slIt = slList.constBegin();
  for ( ; slIt != slList.constEnd(); ++slIt )
  {
    QgsMarkerSymbolLayer *ml = dynamic_cast< QgsMarkerSymbolLayer *>( slIt->first );
    if ( !ml )
      continue;

    if ( hasDataDefinedProperties( ml, slIt->second ) )
      continue;

    QString name = QStringLiteral( "symbolLayer%1" ).arg( i++ );
    writeGroup( 0, QStringLiteral( "BLOCK_RECORD" ) );
    mBlockHandles.insert( name, writeHandle() );
    writeGroup( 100, QStringLiteral( "AcDbSymbolTableRecord" ) );
    writeGroup( 100, QStringLiteral( "AcDbBlockTableRecord" ) );
    writeGroup( 2, name );
  }

  writeGroup( 0, QStringLiteral( "ENDTAB" ) );

  // APPID
  writeGroup( 0, QStringLiteral( "TABLE" ) );
  writeGroup( 2, QStringLiteral( "APPID" ) );
  writeHandle();
  writeGroup( 100, QStringLiteral( "AcDbSymbolTable" ) );
  writeGroup( 70, 1 );
  writeGroup( 0, QStringLiteral( "APPID" ) );
  writeHandle();
  writeGroup( 100, QStringLiteral( "AcDbSymbolTableRecord" ) );
  writeGroup( 100, QStringLiteral( "AcDbRegAppTableRecord" ) );
  writeGroup( 2, QStringLiteral( "ACAD" ) );
  writeGroup( 70, 0 );
  writeGroup( 0, QStringLiteral( "ENDTAB" ) );

  // VIEW
  writeGroup( 0, QStringLiteral( "TABLE" ) );
  writeGroup( 2, QStringLiteral( "VIEW" ) );
  writeHandle();
  writeGroup( 100, QStringLiteral( "AcDbSymbolTable" ) );
  writeGroup( 70, 0 );
  writeGroup( 0, QStringLiteral( "ENDTAB" ) );

  // UCS
  writeGroup( 0, QStringLiteral( "TABLE" ) );
  writeGroup( 2, QStringLiteral( "UCS" ) );
  writeHandle();
  writeGroup( 100, QStringLiteral( "AcDbSymbolTable" ) );
  writeGroup( 70, 0 );
  writeGroup( 0, QStringLiteral( "ENDTAB" ) );

  // VPORT
  writeGroup( 0, QStringLiteral( "TABLE" ) );
  writeGroup( 2, QStringLiteral( "VPORT" ) );
  writeHandle();
  writeGroup( 100, QStringLiteral( "AcDbSymbolTable" ) );

  writeGroup( 0, QStringLiteral( "VPORT" ) );
  writeHandle();
  writeGroup( 100, QStringLiteral( "AcDbSymbolTableRecord" ) );
  writeGroup( 100, QStringLiteral( "AcDbViewportTableRecord" ) );
  writeGroup( 2, QStringLiteral( "*ACTIVE" ) );
  writeGroup( 70, 0 );  // flags
  writeGroup( 0, QgsPoint( 0.0, 0.0 ) );                            // lower left
  writeGroup( 1, QgsPoint( 1.0, 1.0 ) );                            // upper right
  writeGroup( 2, QgsPoint( 0.0, 0.0 ) );                            // view center point
  writeGroup( 3, QgsPoint( 0.0, 0.0 ) );                            // snap base point
  writeGroup( 4, QgsPoint( 1.0, 1.0 ) );                            // snap spacing
  writeGroup( 5, QgsPoint( 1.0, 1.0 ) );                            // grid spacing
  writeGroup( 6, QgsPoint( QgsWkbTypes::PointZ, 0.0, 0.0, 1.0 ) );  // view direction from target point
  writeGroup( 7, QgsPoint( mExtent.center() ) );                    // view target point
  writeGroup( 40, mExtent.height() );                               // view height
  writeGroup( 41, mExtent.width() / mExtent.height() );             // view aspect ratio
  writeGroup( 42, 50.0 );                                           // lens length
  writeGroup( 43, 0.0 );                                            // front clipping plane
  writeGroup( 44, 0.0 );                                            // back clipping plane
  writeGroup( 50, 0.0 );                                            // snap rotation
  writeGroup( 51, 0.0 );                                            // view twist angle
  writeGroup( 71, 0 );                                              // view mode (0 = deactivates)
  writeGroup( 72, 100 );                                            // circle zoom percent
  writeGroup( 73, 1 );                                              // fast zoom setting
  writeGroup( 74, 1 );                                              // UCSICON setting
  writeGroup( 75, 0 );                                              // snapping off
  writeGroup( 76, 0 );                                              // grid off
  writeGroup( 77, 0 );                                              // snap style
  writeGroup( 78, 0 );                                              // snap isopair
  writeGroup( 281, 0 );                                             // render mode (0 = 2D optimized)
  writeGroup( 65, 1 );                                              // value of UCSVP for this viewport
  writeGroup( 100, QgsPoint( QgsWkbTypes::PointZ ) );               // UCS origin
  writeGroup( 101, QgsPoint( QgsWkbTypes::PointZ, 1.0 ) );          // UCS x axis
  writeGroup( 102, QgsPoint( QgsWkbTypes::PointZ, 0.0, 1.0 ) );     // UCS y axis
  writeGroup( 79, 0 );                                              // Orthographic type of UCS (0 = UCS is not orthographic)
  writeGroup( 146, 0.0 );                                           // Elevation

  writeGroup( 70, 0 );
  writeGroup( 0, QStringLiteral( "ENDTAB" ) );

  // DIMSTYLE
  writeGroup( 0, QStringLiteral( "TABLE" ) );
  writeGroup( 2, QStringLiteral( "DIMSTYLE" ) );
  writeHandle();
  writeGroup( 100, QStringLiteral( "AcDbSymbolTable" ) );
  writeGroup( 100, QStringLiteral( "AcDbDimStyleTable" ) );
  writeGroup( 70, 0 );
  writeGroup( 0, QStringLiteral( "ENDTAB" ) );

  QSet<QString> layerNames;
  const QList< QgsMapLayer * > layers = mMapSettings.layers();
  for ( QgsMapLayer *ml : layers )
  {
    if ( !layerIsScaleBasedVisible( ml ) )
      continue;

    QgsVectorLayer *vl = qobject_cast<QgsVectorLayer *>( ml );
    if ( !vl )
      continue;

    int attrIdx = mLayerNameAttribute.value( vl->id(), -1 );
    if ( attrIdx < 0 )
    {
      layerNames << dxfLayerName( layerName( vl ) );
    }
    else
    {
      const QSet<QVariant> values = vl->uniqueValues( attrIdx );
      for ( const QVariant &v : values )
      {
        layerNames << dxfLayerName( v.toString() );
      }
    }
  }

  // Layers
  // TODO: iterate features of all layer to produce a data-defined layer list
  writeGroup( 0, QStringLiteral( "TABLE" ) );
  writeGroup( 2, QStringLiteral( "LAYER" ) );
  writeHandle();
  writeGroup( 100, QStringLiteral( "AcDbSymbolTable" ) );
  writeGroup( 70, layerNames.size() + 1 );

  writeGroup( 0, QStringLiteral( "LAYER" ) );
  writeHandle();
  writeGroup( 100, QStringLiteral( "AcDbSymbolTableRecord" ) );
  writeGroup( 100, QStringLiteral( "AcDbLayerTableRecord" ) );
  writeGroup( 2, QStringLiteral( "0" ) );
  writeGroup( 70, 64 );
  writeGroup( 62, 1 );
  writeGroup( 6, QStringLiteral( "CONTINUOUS" ) );
  writeHandle( 390, DXF_HANDPLOTSTYLE );

  for ( const QString &layerName : qgis::as_const( layerNames ) )
  {
    writeGroup( 0, QStringLiteral( "LAYER" ) );
    writeHandle();
    writeGroup( 100, QStringLiteral( "AcDbSymbolTableRecord" ) );
    writeGroup( 100, QStringLiteral( "AcDbLayerTableRecord" ) );
    writeGroup( 2, layerName );
    writeGroup( 70, 64 );
    writeGroup( 62, 1 );
    writeGroup( 6, QStringLiteral( "CONTINUOUS" ) );
    writeHandle( 390, DXF_HANDPLOTSTYLE );
  }
  writeGroup( 0, QStringLiteral( "ENDTAB" ) );

  // Text styles
  writeGroup( 0, QStringLiteral( "TABLE" ) );
  writeGroup( 2, QStringLiteral( "STYLE" ) );
  writeHandle();
  writeGroup( 100, QStringLiteral( "AcDbSymbolTable" ) );
  writeGroup( 70, 1 );

  // Provide only standard font for the moment
  writeGroup( 0, QStringLiteral( "STYLE" ) );
  writeHandle();
  writeGroup( 100, QStringLiteral( "AcDbSymbolTableRecord" ) );
  writeGroup( 100, QStringLiteral( "AcDbTextStyleTableRecord" ) );
  writeGroup( 2, QStringLiteral( "STANDARD" ) );
  writeGroup( 70, 64 );
  writeGroup( 40, 0.0 );
  writeGroup( 41, 1.0 );
  writeGroup( 50, 0.0 );
  writeGroup( 71, 0 );
  writeGroup( 42, 5.0 );
  writeGroup( 3, QStringLiteral( "romans.shx" ) );
  writeGroup( 4, QString() );

  writeGroup( 0, QStringLiteral( "ENDTAB" ) );

  endSection();
}

void QgsDxfExport::writeBlocks()
{
  startSection();
  writeGroup( 2, QStringLiteral( "BLOCKS" ) );

  const QStringList blockStrings = QStringList() << QStringLiteral( "*Model_Space" ) << QStringLiteral( "*Paper_Space" ) << QStringLiteral( "*Paper_Space0" );
  for ( const QString &block : blockStrings )
  {
    writeGroup( 0, QStringLiteral( "BLOCK" ) );
    writeHandle();
    writeGroup( 330, QStringLiteral( "%1" ).arg( mBlockHandles[ block ], 0, 16 ) );
    writeGroup( 100, QStringLiteral( "AcDbEntity" ) );
    writeGroup( 8, QStringLiteral( "0" ) );
    writeGroup( 100, QStringLiteral( "AcDbBlockBegin" ) );
    writeGroup( 2, block );
    writeGroup( 70, 0 );
    writeGroup( 0, QgsPoint( QgsWkbTypes::PointZ ) );
    writeGroup( 3, block );
    writeGroup( 1, QString() );
    writeGroup( 0, QStringLiteral( "ENDBLK" ) );
    writeHandle();
    writeGroup( 100, QStringLiteral( "AcDbEntity" ) );
    writeGroup( 8, QStringLiteral( "0" ) );
    writeGroup( 100, QStringLiteral( "AcDbBlockEnd" ) );
  }

  QgsRenderContext ct = renderContext();

  // Iterate through all layers and get symbol layer pointers
  QList< QPair< QgsSymbolLayer *, QgsSymbol * > > slList;
  if ( mSymbologyExport != NoSymbology )
  {
    slList = symbolLayers( ct );
  }

  QList< QPair< QgsSymbolLayer *, QgsSymbol * > >::const_iterator slIt = slList.constBegin();
  for ( ; slIt != slList.constEnd(); ++slIt )
  {
    QgsMarkerSymbolLayer *ml = dynamic_cast< QgsMarkerSymbolLayer *>( slIt->first );
    if ( !ml )
      continue;

    // if point symbol layer and no data defined properties: write block
    QgsSymbolRenderContext ctx( ct, QgsUnitTypes::RenderMapUnits, slIt->second->opacity(), false, slIt->second->renderHints(), nullptr );
    ml->startRender( ctx );

    // markers with data defined properties are inserted inline
    if ( hasDataDefinedProperties( ml, slIt->second ) )
    {
      continue;
      // ml->stopRender( ctx );
    }

    QString block( QStringLiteral( "symbolLayer%1" ).arg( mBlockCounter++ ) );
    mBlockHandle = QStringLiteral( "%1" ).arg( mBlockHandles[ block ], 0, 16 );

    writeGroup( 0, QStringLiteral( "BLOCK" ) );
    writeHandle();
    writeGroup( 330, mBlockHandle );
    writeGroup( 100, QStringLiteral( "AcDbEntity" ) );
    writeGroup( 8, QStringLiteral( "0" ) );
    writeGroup( 100, QStringLiteral( "AcDbBlockBegin" ) );
    writeGroup( 2, block );
    writeGroup( 70, 0 );

    // x/y/z coordinates of reference point
    // todo: consider anchor point
    // double size = ml->size();
    // size *= mapUnitScaleFactor( mSymbologyScale, ml->sizeUnit(), mMapUnits );
    writeGroup( 0, QgsPoint( QgsWkbTypes::PointZ ) );
    writeGroup( 3, block );
    writeGroup( 1, QString() );

    // maplayer 0 -> block receives layer from INSERT statement
    ml->writeDxf( *this, mapUnitScaleFactor( mSymbologyScale, ml->sizeUnit(), mMapUnits, ctx.renderContext().mapToPixel().mapUnitsPerPixel() ), QStringLiteral( "0" ), ctx );

    writeGroup( 0, QStringLiteral( "ENDBLK" ) );
    writeHandle();
    writeGroup( 100, QStringLiteral( "AcDbEntity" ) );
    writeGroup( 8, QStringLiteral( "0" ) );
    writeGroup( 100, QStringLiteral( "AcDbBlockEnd" ) );

    mPointSymbolBlocks.insert( ml, block );
    ml->stopRender( ctx );
  }
  endSection();
}


void QgsDxfExport::writeEntities()
{
  startSection();
  writeGroup( 2, QStringLiteral( "ENTITIES" ) );

  mBlockHandle = QStringLiteral( "%1" ).arg( mBlockHandles[ QStringLiteral( "*Model_Space" )], 0, 16 );

  QImage image( 10, 10, QImage::Format_ARGB32_Premultiplied );
  image.setDotsPerMeterX( 96 / 25.4 * 1000 );
  image.setDotsPerMeterY( 96 / 25.4 * 1000 );
  QPainter painter( &image );

  QgsRenderContext ctx;
  ctx.setPainter( &painter );
  ctx.setRendererScale( mSymbologyScale );
  ctx.setExtent( mExtent );

  ctx.setScaleFactor( 96.0 / 25.4 );
  ctx.setMapToPixel( QgsMapToPixel( 1.0 / mFactor, mExtent.center().x(), mExtent.center().y(), mExtent.width() * mFactor,
                                    mExtent.height() * mFactor, 0 ) );

  ctx.expressionContext().appendScope( QgsExpressionContextUtils::projectScope( QgsProject::instance() ) );
  ctx.expressionContext().appendScope( QgsExpressionContextUtils::globalScope() );

  // label engine
  QgsLabelingEngine engine;
  engine.setMapSettings( mMapSettings );

  // iterate through the maplayers
  const QList< QgsMapLayer *> layers = mMapSettings.layers();
  for ( QgsMapLayer *ml : layers )
  {
    QgsVectorLayer *vl = qobject_cast<QgsVectorLayer *>( ml );
    if ( !vl || !layerIsScaleBasedVisible( vl ) )
    {
      continue;
    }

    QgsMapLayerStyleOverride styleOverride( vl );
    if ( mMapSettings.layerStyleOverrides().contains( vl->id() ) )
    {
      QgsDebugMsg( QStringLiteral( "%1: apply override style" ).arg( vl->id() ) );
      styleOverride.setOverrideStyle( mMapSettings.layerStyleOverrides().value( vl->id() ) );
    }
    else
    {
      QgsDebugMsg( QStringLiteral( "%1: not override style" ).arg( vl->id() ) );
    }

    if ( !vl->renderer() )
    {
      continue;
    }

    auto scopePopper = [&ctx]( QgsExpressionContextScope * scope )
    {
      Q_UNUSED( scope );
      delete ctx.expressionContext().popScope();
    };
    std::unique_ptr<QgsExpressionContextScope, decltype( scopePopper ) > layerScope( QgsExpressionContextUtils::layerScope( ml ), scopePopper );
    ctx.expressionContext().appendScope( layerScope.get() );
    QgsSymbolRenderContext sctx( ctx, QgsUnitTypes::RenderMillimeters, 1.0, false, nullptr, nullptr );

    std::unique_ptr< QgsFeatureRenderer > renderer( vl->renderer()->clone() );
    renderer->startRender( ctx, vl->fields() );

    QSet<QString> attributes = renderer->usedAttributes( ctx );
    int attrIdx = mLayerNameAttribute.value( vl->id(), -1 );
    if ( vl->fields().exists( attrIdx ) )
    {
      QString layerAttr = vl->fields().at( attrIdx ).name();
      attributes << layerAttr;
    }

    const QgsAbstractVectorLayerLabeling *labeling = vl->labelsEnabled() ? vl->labeling() : nullptr;
    QgsDxfLabelProvider *lp = nullptr;
    QgsDxfRuleBasedLabelProvider *rblp = nullptr;
    if ( const QgsRuleBasedLabeling *rbl = dynamic_cast<const QgsRuleBasedLabeling *>( labeling ) )
    {
      rblp = new QgsDxfRuleBasedLabelProvider( *rbl, vl, this );
      rblp->reinit( vl );
      engine.addProvider( rblp );

      if ( !rblp->prepare( ctx, attributes ) )
      {
        engine.removeProvider( rblp );
        rblp = nullptr;
      }
    }
    else if ( labeling )
    {
      QgsPalLayerSettings settings = labeling->settings();
      lp = new QgsDxfLabelProvider( vl, QString(), this, &settings );
      engine.addProvider( lp );

      if ( !lp->prepare( ctx, attributes ) )
      {
        engine.removeProvider( lp );
        lp = nullptr;
      }
    }

    if ( mSymbologyExport == QgsDxfExport::SymbolLayerSymbology &&
         ( renderer->capabilities() & QgsFeatureRenderer::SymbolLevels ) &&
         renderer->usingSymbolLevels() )
    {
      writeEntitiesSymbolLevels( vl );
      renderer->stopRender( ctx );

      continue;
    }

    QgsFeatureRequest freq = QgsFeatureRequest().setSubsetOfAttributes( attributes, vl->fields() ).setExpressionContext( ctx.expressionContext() );
    freq.setFilterRect( mMapSettings.mapToLayerCoordinates( vl, mExtent ) );

    QgsFeatureIterator featureIt = vl->getFeatures( freq );

    QgsCoordinateTransform ct = mMapSettings.layerTransform( vl );

    QgsFeature fet;
    while ( featureIt.nextFeature( fet ) )
    {
      ctx.expressionContext().setFeature( fet );
      QString lName( dxfLayerName( attrIdx < 0 ? layerName( vl ) : fet.attribute( attrIdx ).toString() ) );

      sctx.setFeature( &fet );
      if ( mSymbologyExport == NoSymbology )
      {
        addFeature( sctx, ct, lName, nullptr, nullptr ); // no symbology at all
      }
      else
      {
        QgsSymbolList symbolList = renderer->symbolsForFeature( fet, ctx );
        bool hasSymbology = symbolList.size() > 0;

        if ( hasSymbology && mSymbologyExport == QgsDxfExport::SymbolLayerSymbology ) // symbol layer symbology, but layer does not use symbol levels
        {
          QgsSymbolList::iterator symbolIt = symbolList.begin();
          for ( ; symbolIt != symbolList.end(); ++symbolIt )
          {
            int nSymbolLayers = ( *symbolIt )->symbolLayerCount();
            for ( int i = 0; i < nSymbolLayers; ++i )
            {
              QgsSymbolLayer *sl = ( *symbolIt )->symbolLayer( i );
              if ( !sl )
              {
                continue;
              }

              bool isGeometryGenerator = ( sl->layerType() == QLatin1String( "GeometryGenerator" ) );
              if ( isGeometryGenerator )
              {
                addGeometryGeneratorSymbolLayer( sctx, ct, lName, sl, true );
              }
              else
              {
                addFeature( sctx, ct, lName, sl, *symbolIt );
              }
            }
          }
        }
        else if ( hasSymbology )
        {
          // take first symbollayer from first symbol
          QgsSymbol *s = symbolList.first();
          if ( !s || s->symbolLayerCount() < 1 )
          {
            continue;
          }

          if ( s->symbolLayer( 0 )->layerType() == QLatin1String( "GeometryGenerator" ) )
          {
            addGeometryGeneratorSymbolLayer( sctx, ct, lName, s->symbolLayer( 0 ), false );
          }
          else
          {
            addFeature( sctx, ct, lName, s->symbolLayer( 0 ), s );
          }
        }

        if ( lp )
        {
          lp->registerDxfFeature( fet, ctx, lName );
        }
        else if ( rblp )
        {
          rblp->registerDxfFeature( fet, ctx, lName );
        }
      }
    }

    renderer->stopRender( ctx );
  }

  engine.run( ctx );

  endSection();
}

void QgsDxfExport::writeEntitiesSymbolLevels( QgsVectorLayer *layer )
{
  if ( !layer )
  {
    return;
  }

  if ( !layer->renderer() )
  {
    // TODO return error
    return;
  }
  std::unique_ptr< QgsFeatureRenderer > renderer( layer->renderer()->clone() );
  QHash< QgsSymbol *, QList<QgsFeature> > features;

  QgsRenderContext ctx = renderContext();
  ctx.expressionContext().appendScopes( QgsExpressionContextUtils::globalProjectLayerScopes( layer ) );
  QgsSymbolRenderContext sctx( ctx, QgsUnitTypes::RenderMillimeters, 1.0, false, nullptr, nullptr );
  renderer->startRender( ctx, layer->fields() );

  // get iterator
  QgsFeatureRequest req;
  if ( layer->wkbType() == QgsWkbTypes::NoGeometry )
  {
    req.setFlags( QgsFeatureRequest::NoGeometry );
  }
  req.setSubsetOfAttributes( renderer->usedAttributes( ctx ), layer->fields() );
  req.setFilterRect( mMapSettings.mapToLayerCoordinates( layer, mExtent ) );

  QgsFeatureIterator fit = layer->getFeatures( req );

  // fetch features
  QgsFeature fet;
  QgsSymbol *featureSymbol = nullptr;
  while ( fit.nextFeature( fet ) )
  {
    ctx.expressionContext().setFeature( fet );
    featureSymbol = renderer->symbolForFeature( fet, ctx );
    if ( !featureSymbol )
    {
      continue;
    }

    QHash< QgsSymbol *, QList<QgsFeature> >::iterator it = features.find( featureSymbol );
    if ( it == features.end() )
    {
      it = features.insert( featureSymbol, QList<QgsFeature>() );
    }
    it.value().append( fet );
  }

  // find out order
  QgsSymbolLevelOrder levels;
  QgsSymbolList symbols = renderer->symbols( ctx );
  for ( int i = 0; i < symbols.count(); i++ )
  {
    QgsSymbol *sym = symbols[i];
    for ( int j = 0; j < sym->symbolLayerCount(); j++ )
    {
      int level = sym->symbolLayer( j )->renderingPass();
      if ( level < 0 || level >= 1000 ) // ignore invalid levels
        continue;
      QgsSymbolLevelItem item( sym, j );
      while ( level >= levels.count() ) // append new empty levels
        levels.append( QgsSymbolLevel() );
      levels[level].append( item );
    }
  }

  QgsCoordinateTransform ct = mMapSettings.layerTransform( layer );

  // export symbol layers and symbology
  for ( int l = 0; l < levels.count(); l++ )
  {
    QgsSymbolLevel &level = levels[l];
    for ( int i = 0; i < level.count(); i++ )
    {
      QgsSymbolLevelItem &item = level[i];
      QHash< QgsSymbol *, QList<QgsFeature> >::iterator levelIt = features.find( item.symbol() );
      if ( levelIt == features.end() )
      {
        QgsDebugMsg( QStringLiteral( "No feature found for symbol on %1 %2.%3" ).arg( layer->id() ).arg( l ).arg( i ) );
        continue;
      }

      int llayer = item.layer();
      QList<QgsFeature> &featureList = levelIt.value();
      QList<QgsFeature>::iterator featureIt = featureList.begin();
      for ( ; featureIt != featureList.end(); ++featureIt )
      {
        sctx.setFeature( &*featureIt );
        addFeature( sctx, ct, layer->name(), levelIt.key()->symbolLayer( llayer ), levelIt.key() );
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

  writeGroup( 0, QStringLiteral( "EOF" ) );
}

void QgsDxfExport::startSection()
{
  writeGroup( 0, QStringLiteral( "SECTION" ) );
}

void QgsDxfExport::endSection()
{
  writeGroup( 0, QStringLiteral( "ENDSEC" ) );
}

void QgsDxfExport::writePoint( const QgsPoint &pt, const QString &layer, const QColor &color, QgsSymbolRenderContext &ctx, const QgsSymbolLayer *symbolLayer, const QgsSymbol *symbol, double angle )
{
#if 0
  // debug: draw rectangle for debugging
  const QgsMarkerSymbolLayer *msl = dynamic_cast< const QgsMarkerSymbolLayer * >( symbolLayer );
  if ( msl )
  {
    double halfSize = msl->size() * mapUnitScaleFactor( mSymbologyScale,
                      msl->sizeUnit(), mMapUnits ) / 2.0;
    writeGroup( 0, "SOLID" );
    writeGroup( 8, layer );
    writeGroup( 62, 1 );
    writeGroup( 0, QgsPoint( QgsWkbTypes::PointZ, pt.x() - halfSize, pt.y() - halfSize ) );
    writeGroup( 1, QgsPoint( QgsWkbTypes::PointZ, pt.x() + halfSize, pt.y() - halfSize ) );
    writeGroup( 2, QgsPoint( QgsWkbTypes::PointZ, pt.x() - halfSize, pt.y() + halfSize ) );
    writeGroup( 3, QgsPoint( QgsWkbTypes::PointZ, pt.x() + halfSize, pt.y() + halfSize ) );
  }
#endif // 0

  // insert block or write point directly?
  QHash< const QgsSymbolLayer *, QString >::const_iterator blockIt = mPointSymbolBlocks.constFind( symbolLayer );
  if ( !symbolLayer || blockIt == mPointSymbolBlocks.constEnd() )
  {
    // write symbol directly here
    const QgsMarkerSymbolLayer *msl = dynamic_cast< const QgsMarkerSymbolLayer * >( symbolLayer );
    if ( msl && symbol )
    {
      if ( symbolLayer->writeDxf( *this, mapUnitScaleFactor( mSymbologyScale, msl->sizeUnit(), mMapUnits, ctx.renderContext().mapToPixel().mapUnitsPerPixel() ), layer, ctx, QPointF( pt.x(), pt.y() ) ) )
      {
        return;
      }
    }
    writePoint( layer, color, pt ); // write default point symbol
  }
  else
  {
    // insert block reference
    writeGroup( 0, QStringLiteral( "INSERT" ) );
    writeHandle();
    writeGroup( 100, QStringLiteral( "AcDbEntity" ) );
    writeGroup( 100, QStringLiteral( "AcDbBlockReference" ) );
    writeGroup( 8, layer );
    writeGroup( 2, blockIt.value() ); // Block name
    writeGroup( 50, angle ); // angle
    writeGroup( 0, pt );  // Insertion point (in OCS)
  }
}

void QgsDxfExport::writePolyline( const QgsPointSequence &line, const QString &layer, const QString &lineStyleName, const QColor &color, double width )
{
  int n = line.size();
  if ( n == 0 )
  {
    QgsDebugMsg( QStringLiteral( "writePolyline: empty line layer=%1 lineStyleName=%2" ).arg( layer, lineStyleName ) );
    return;
  }

  bool polygon = line[0] == line[ line.size() - 1 ];
  if ( polygon )
    --n;
  if ( n < 2 )
  {
    QgsDebugMsg( QStringLiteral( "writePolyline: line too short layer=%1 lineStyleName=%2" ).arg( layer, lineStyleName ) );
    return;
  }

  if ( mForce2d || !line.at( 0 ).is3D() )
  {
    writeGroup( 0, QStringLiteral( "LWPOLYLINE" ) );
    writeHandle();
    writeGroup( 8, layer );
    writeGroup( 100, QStringLiteral( "AcDbEntity" ) );
    writeGroup( 100, QStringLiteral( "AcDbPolyline" ) );
    writeGroup( 6, lineStyleName );
    writeGroup( color );

    writeGroup( 90, n );
    writeGroup( 70, polygon ? 1 : 0 );
    writeGroup( 43, width );

    for ( int i = 0; i < n; i++ )
      writeGroup( 0, line[i] );
  }
  else
  {
    writeGroup( 0, QStringLiteral( "POLYLINE" ) );
    int plHandle = writeHandle();
    writeGroup( 330, mBlockHandle );
    writeGroup( 100, QStringLiteral( "AcDbEntity" ) );
    writeGroup( 8, layer );
    writeGroup( 6, lineStyleName );
    writeGroup( color );
    writeGroup( 100, QStringLiteral( "AcDb3dPolyline" ) );
    writeGroup( 0, QgsPoint( QgsWkbTypes::PointZ ) );
    writeGroup( 70, 8 );

    for ( int i = 0; i < n; i++ )
    {
      writeGroup( 0, QStringLiteral( "VERTEX" ) );
      writeHandle();
      writeGroup( 330, plHandle );
      writeGroup( 100, QStringLiteral( "AcDbEntity" ) );
      writeGroup( 8, layer );
      writeGroup( color );
      writeGroup( 100, QStringLiteral( "AcDbVertex" ) );
      writeGroup( 100, QStringLiteral( "AcDb3dPolylineVertex" ) );
      writeGroup( 0, line[i] );
      writeGroup( 70, 32 );
    }

    writeGroup( 0, QStringLiteral( "SEQEND" ) );
    writeHandle();
    writeGroup( 330, plHandle );
    writeGroup( 100, QStringLiteral( "AcDbEntity" ) );
    writeGroup( 8, layer );
    writeGroup( color );
  }
}

void QgsDxfExport::writePolygon( const QgsRingSequence &polygon, const QString &layer, const QString &hatchPattern, const QColor &color )
{
  writeGroup( 0, QStringLiteral( "HATCH" ) );       // Entity type
  writeHandle();
  writeGroup( 330, mBlockHandle );
  writeGroup( 100, QStringLiteral( "AcDbEntity" ) );
  writeGroup( 8, layer );           // Layer name
  writeGroup( color );              // Color
  writeGroup( 100, QStringLiteral( "AcDbHatch" ) );

  writeGroup( 0, QgsPoint( QgsWkbTypes::PointZ ) ); // Elevation point (in OCS)
  writeGroup( 200, QgsPoint( QgsWkbTypes::PointZ, 0.0, 0.0, 1.0 ) );

  writeGroup( 2, hatchPattern );  // Hatch pattern name
  writeGroup( 70, hatchPattern == QLatin1String( "SOLID" ) ); // Solid fill flag (solid fill = 1; pattern fill = 0)
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
      writeGroup( 0, polygon[i][j] ); // Vertex location (in OCS)
    }

    writeGroup( 97, 0 );   // Number of source boundary objects
  }

  writeGroup( 75, 0 );    // Hatch style: 0 = Hatch "odd parity" area (Normal style), 1 = Hatch outermost area only (Outer style), 2 = Hatch through entire area (Ignore style)
  writeGroup( 76, 1 );    // Hatch pattern type: 0 = User-defined; 1 = Predefined; 2 = Custom

  writeGroup( 98, 0 );    // Number of seed points
}

void QgsDxfExport::writeLine( const QgsPoint &pt1, const QgsPoint &pt2, const QString &layer, const QString &lineStyleName, const QColor &color, double width )
{
  writePolyline( QgsPointSequence() << pt1 << pt2, layer, lineStyleName, color, width );
}

void QgsDxfExport::writePoint( const QString &layer, const QColor &color, const QgsPoint &pt )
{
  writeGroup( 0, QStringLiteral( "POINT" ) );
  writeHandle();
  writeGroup( 100, QStringLiteral( "AcDbEntity" ) );
  writeGroup( 100, QStringLiteral( "AcDbPoint" ) );
  writeGroup( 8, layer );
  writeGroup( color );
  writeGroup( 0, pt );
}

void QgsDxfExport::writeFilledCircle( const QString &layer, const QColor &color, const QgsPoint &pt, double radius )
{
  writeGroup( 0, QStringLiteral( "HATCH" ) );  // Entity type
  writeHandle();
  writeGroup( 330, mBlockHandle );
  writeGroup( 100, QStringLiteral( "AcDbEntity" ) );
  writeGroup( 8, layer );    // Layer name
  writeGroup( color );       // Color (0 by block, 256 by layer)
  writeGroup( 100, QStringLiteral( "AcDbHatch" ) );

  writeGroup( 0, QgsPoint( QgsWkbTypes::PointZ ) ); // Elevation point (in OCS)
  writeGroup( 200, QgsPoint( QgsWkbTypes::PointZ, 0.0, 0.0, 1.0 ) );

  writeGroup( 2, QStringLiteral( "SOLID" ) );  // Hatch pattern name
  writeGroup( 70, 1 );       // Solid fill flag (solid fill = 1; pattern fill = 0)
  writeGroup( 71, 0 );       // Associativity flag (associative = 1; non-associative = 0)

  writeGroup( 91, 1 );       // Number of boundary paths (loops)

  writeGroup( 92, 3 );       // Boundary path type flag (bit coded): 0 = Default; 1 = External; 2 = Polyline 4 = Derived; 8 = Textbox; 16 = Outermost
  writeGroup( 72, 1 );
  writeGroup( 73, 1 );       // Is closed flag
  writeGroup( 93, 2 );       // Number of polyline vertices

  writeGroup( 0, QgsPoint( QgsWkbTypes::Point, pt.x() - radius, pt.y() ) );
  writeGroup( 42, 1.0 );

  writeGroup( 0, QgsPoint( QgsWkbTypes::Point, pt.x() + radius, pt.y() ) );
  writeGroup( 42, 1.0 );

  writeGroup( 97, 0 );       // Number of source boundary objects

  writeGroup( 75, 0 );       // Hatch style: 0 = Hatch "odd parity" area (Normal style), 1 = Hatch outermost area only (Outer style), 2 = Hatch through entire area (Ignore style)
  writeGroup( 76, 1 );       // Hatch pattern type: 0 = User-defined; 1 = Predefined; 2 = Custom
  writeGroup( 98, 0 );       // Number of seed points
}

void QgsDxfExport::writeCircle( const QString &layer, const QColor &color, const QgsPoint &pt, double radius, const QString &lineStyleName, double width )
{
  writeGroup( 0, QStringLiteral( "LWPOLYLINE" ) );
  writeHandle();
  writeGroup( 330, mBlockHandle );
  writeGroup( 8, layer );
  writeGroup( 100, QStringLiteral( "AcDbEntity" ) );
  writeGroup( 100, QStringLiteral( "AcDbPolyline" ) );
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

void QgsDxfExport::writeText( const QString &layer, const QString &text, const QgsPoint &pt, double size, double angle, const QColor &color )
{
  writeGroup( 0, QStringLiteral( "TEXT" ) );
  writeHandle();
  writeGroup( 100, QStringLiteral( "AcDbEntity" ) );
  writeGroup( 100, QStringLiteral( "AcDbText" ) );
  writeGroup( 8, layer );
  writeGroup( color );
  writeGroup( 0, pt );
  writeGroup( 40, size );
  writeGroup( 1, text );
  writeGroup( 50, angle );
  writeGroup( 7, QStringLiteral( "STANDARD" ) ); // so far only support for standard font
  writeGroup( 100, QStringLiteral( "AcDbText" ) );
}

void QgsDxfExport::writeMText( const QString &layer, const QString &text, const QgsPoint &pt, double width, double angle, const QColor &color )
{
  if ( !mTextStream.codec()->canEncode( text ) )
  {
    // TODO return error
    QgsDebugMsg( QStringLiteral( "could not encode:%1" ).arg( text ) );
    return;
  }

  writeGroup( 0, QStringLiteral( "MTEXT" ) );
  writeHandle();
  writeGroup( 100, QStringLiteral( "AcDbEntity" ) );
  writeGroup( 100, QStringLiteral( "AcDbMText" ) );
  writeGroup( 8, layer );
  writeGroup( color );

  writeGroup( 0, pt );

  QString t( text );
  while ( t.length() > 250 )
  {
    writeGroup( 3, t.left( 250 ) );
    t = t.mid( 250 );
  }
  writeGroup( 1, t );

  writeGroup( 50, angle );        // Rotation angle in radians
  writeGroup( 41, width * 1.1 );  // Reference rectangle width

  // Attachment point:
  // 1 2 3
  // 4 5 6
  // 7 8 9
  writeGroup( 71, 7 );

  writeGroup( 7, QStringLiteral( "STANDARD" ) );  // so far only support for standard font
}

void QgsDxfExport::addFeature( QgsSymbolRenderContext &ctx, const QgsCoordinateTransform &ct, const QString &layer, const QgsSymbolLayer *symbolLayer, const QgsSymbol *symbol )
{
  const QgsFeature *fet = ctx.feature();
  if ( !fet )
    return;

  if ( !fet->hasGeometry() )
    return;

  std::unique_ptr<QgsAbstractGeometry> geom( fet->geometry().constGet()->clone() );
  if ( ct.isValid() )
  {
    geom->transform( ct );
  }

  QgsWkbTypes::Type geometryType = geom->wkbType();

  QColor penColor;
  QColor brushColor;
  if ( mSymbologyExport != NoSymbology && symbolLayer )
  {
    penColor = colorFromSymbolLayer( symbolLayer, ctx );
    brushColor = symbolLayer->dxfBrushColor( ctx );
  }

  Qt::PenStyle penStyle( Qt::SolidLine );
  Qt::BrushStyle brushStyle( Qt::NoBrush );
  double width = -1;
  double offset = 0.0;
  double angle = 0.0;
  if ( mSymbologyExport != NoSymbology && symbolLayer )
  {
    width = symbolLayer->dxfWidth( *this, ctx );
    offset = symbolLayer->dxfOffset( *this, ctx );
    angle = symbolLayer->dxfAngle( ctx );
    penStyle = symbolLayer->dxfPenStyle();
    brushStyle = symbolLayer->dxfBrushStyle();

    if ( qgsDoubleNear( offset, 0.0 ) )
      offset = 0.0;
  }

  QString lineStyleName = QStringLiteral( "CONTINUOUS" );
  if ( mSymbologyExport != NoSymbology )
  {
    lineStyleName = lineStyleFromSymbolLayer( symbolLayer );
  }

  // single point
  if ( QgsWkbTypes::flatType( geometryType ) == QgsWkbTypes::Point )
  {
    writePoint( geom->coordinateSequence().at( 0 ).at( 0 ).at( 0 ), layer, penColor, ctx, symbolLayer, symbol, angle );
    return;
  }

  if ( QgsWkbTypes::flatType( geometryType ) == QgsWkbTypes::MultiPoint )
  {
    const QgsCoordinateSequence &cs = geom->coordinateSequence();
    for ( int i = 0; i < cs.size(); i++ )
    {
      writePoint( cs.at( i ).at( 0 ).at( 0 ), layer, penColor, ctx, symbolLayer, symbol, angle );
    }
    return;
  }

  if ( penStyle != Qt::NoPen )
  {
    const QgsAbstractGeometry *tempGeom = geom.get();

    switch ( QgsWkbTypes::flatType( geometryType ) )
    {
      case QgsWkbTypes::CircularString:
      case QgsWkbTypes::CompoundCurve:
        tempGeom = geom->segmentize();
        if ( !tempGeom )
          break;
        FALLTHROUGH
      case QgsWkbTypes::LineString:
        if ( !qgsDoubleNear( offset, 0.0 ) )
        {
          QgsGeos geos( tempGeom );
          if ( tempGeom != geom.get() )
            delete tempGeom;
          tempGeom = geos.offsetCurve( offset, 0, GEOSBUF_JOIN_MITRE, 2.0 );  //#spellok
          if ( !tempGeom )
            tempGeom = geom.get();
        }

        writePolyline( tempGeom->coordinateSequence().at( 0 ).at( 0 ), layer, lineStyleName, penColor, width );

        break;

      case QgsWkbTypes::MultiCurve:
        tempGeom = geom->segmentize();
        if ( !tempGeom )
          break;
        FALLTHROUGH
      case QgsWkbTypes::MultiLineString:
      {
        if ( !qgsDoubleNear( offset, 0.0 ) )
        {
          QgsGeos geos( tempGeom );
          if ( tempGeom != geom.get() )
            delete tempGeom;
          tempGeom = geos.offsetCurve( offset, 0, GEOSBUF_JOIN_MITRE, 2.0 );  //#spellok
          if ( !tempGeom )
            tempGeom = geom.get();
        }

        const QgsCoordinateSequence &cs = tempGeom->coordinateSequence();
        for ( int i = 0; i < cs.size(); i++ )
        {
          writePolyline( cs.at( i ).at( 0 ), layer, lineStyleName, penColor, width );
        }

        break;
      }

      case QgsWkbTypes::CurvePolygon:
        tempGeom = geom->segmentize();
        if ( !tempGeom )
          break;
        FALLTHROUGH
      case QgsWkbTypes::Polygon:
      {
        if ( !qgsDoubleNear( offset, 0.0 ) )
        {
          QgsGeos geos( tempGeom );
          if ( tempGeom != geom.get() )
            delete tempGeom;
          tempGeom = geos.buffer( offset, 0,  GEOSBUF_CAP_FLAT, GEOSBUF_JOIN_MITRE, 2.0 );  //#spellok
          if ( !tempGeom )
            tempGeom = geom.get();
        }

        const QgsCoordinateSequence &cs = tempGeom->coordinateSequence();
        for ( int i = 0; i < cs.at( 0 ).size(); i++ )
        {
          writePolyline( cs.at( 0 ).at( i ), layer, lineStyleName, penColor, width );
        }

        break;
      }

      case QgsWkbTypes::MultiPolygon:
      {
        if ( !qgsDoubleNear( offset, 0.0 ) )
        {
          QgsGeos geos( tempGeom );
          if ( tempGeom != geom.get() )
            delete tempGeom;
          tempGeom = geos.buffer( offset, 0,  GEOSBUF_CAP_FLAT, GEOSBUF_JOIN_MITRE, 2.0 );  //#spellok
          if ( !tempGeom )
            tempGeom = geom.get();
        }

        const QgsCoordinateSequence &cs = tempGeom->coordinateSequence();
        for ( int i = 0; i < cs.size(); i++ )
          for ( int j = 0; j < cs.at( i ).size(); j++ )
            writePolyline( cs.at( i ).at( j ), layer, lineStyleName, penColor, width );

        break;
      }

      default:
        break;
    }

    if ( tempGeom != geom.get() )
      delete tempGeom;
  }

  if ( brushStyle != Qt::NoBrush )
  {
    const QgsAbstractGeometry *tempGeom = geom.get();

    switch ( QgsWkbTypes::flatType( geometryType ) )
    {
      case QgsWkbTypes::CurvePolygon:
        tempGeom = tempGeom->segmentize();
        if ( !tempGeom )
          break;
        FALLTHROUGH
      case QgsWkbTypes::Polygon:
        writePolygon( tempGeom->coordinateSequence().at( 0 ), layer, QStringLiteral( "SOLID" ), brushColor );
        break;

      case QgsWkbTypes::MultiPolygon:
      {
        const QgsCoordinateSequence &cs = geom->coordinateSequence();
        for ( int i = 0; i < cs.size(); i++ )
        {
          writePolygon( cs.at( i ), layer, QStringLiteral( "SOLID" ), brushColor );
        }
        break;
      }

      default:
        break;

    }

    if ( tempGeom != geom.get() )
      delete tempGeom;
  }
}

QColor QgsDxfExport::colorFromSymbolLayer( const QgsSymbolLayer *symbolLayer, QgsSymbolRenderContext &ctx )
{
  if ( !symbolLayer )
    return QColor();

  return symbolLayer->dxfColor( ctx );
}

QString QgsDxfExport::lineStyleFromSymbolLayer( const QgsSymbolLayer *symbolLayer )
{
  QString lineStyleName = QStringLiteral( "CONTINUOUS" );
  if ( !symbolLayer )
  {
    return lineStyleName;
  }

  QHash< const QgsSymbolLayer *, QString >::const_iterator lineTypeIt = mLineStyles.constFind( symbolLayer );
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
  int current_distance = std::numeric_limits<int>::max();
  for ( int i = 1; i < static_cast< int >( sizeof( sDxfColors ) / sizeof( *sDxfColors ) ); ++i )
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

  double redDiff = qRed( p1 ) - sDxfColors[index][0];
  double greenDiff = qGreen( p1 ) - sDxfColors[index][1];
  double blueDiff = qBlue( p1 ) - sDxfColors[index][2];
#if 0
  QgsDebugMsg( QStringLiteral( "color_distance( r:%1 g:%2 b:%3 <=> i:%4 r:%5 g:%6 b:%7 ) => %8" )
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
  context.setRendererScale( mSymbologyScale );
  return context;
}

double QgsDxfExport::mapUnitScaleFactor( double scale, QgsUnitTypes::RenderUnit symbolUnits, QgsUnitTypes::DistanceUnit mapUnits, double mapUnitsPerPixel )
{
  if ( symbolUnits == QgsUnitTypes::RenderMapUnits )
  {
    return 1.0;
  }
  else if ( symbolUnits == QgsUnitTypes::RenderMillimeters )
  {
    return ( scale * QgsUnitTypes::fromUnitToUnitFactor( QgsUnitTypes::DistanceMeters, mapUnits ) / 1000.0 );
  }
  else if ( symbolUnits == QgsUnitTypes::RenderPixels )
  {
    return mapUnitsPerPixel;
  }
  return 1.0;
}

void QgsDxfExport::clipValueToMapUnitScale( double &value, const QgsMapUnitScale &scale, double pixelToMMFactor ) const
{
  if ( !scale.minSizeMMEnabled && !scale.maxSizeMMEnabled )
  {
    return;
  }

  double mapUnitsPerPixel = mMapSettings.mapToPixel().mapUnitsPerPixel();

  double minSizeMU = std::numeric_limits<double>::lowest();
  if ( scale.minSizeMMEnabled )
  {
    minSizeMU = scale.minSizeMM * pixelToMMFactor * mapUnitsPerPixel;
  }
  if ( !qgsDoubleNear( scale.minScale, 0.0 ) )
  {
    minSizeMU = std::max( minSizeMU, value );
  }
  value = std::max( value, minSizeMU );

  double maxSizeMU = std::numeric_limits<double>::max();
  if ( scale.maxSizeMMEnabled )
  {
    maxSizeMU = scale.maxSizeMM * pixelToMMFactor * mapUnitsPerPixel;
  }
  if ( !qgsDoubleNear( scale.maxScale, 0.0 ) )
  {
    maxSizeMU = std::min( maxSizeMU, value );
  }
  value = std::min( value, maxSizeMU );
}

QList< QPair< QgsSymbolLayer *, QgsSymbol * > > QgsDxfExport::symbolLayers( QgsRenderContext &context )
{
  QList< QPair< QgsSymbolLayer *, QgsSymbol * > > symbolLayers;

  const QList< QgsMapLayer * > layers = mMapSettings.layers();
  for ( QgsMapLayer *ml : layers )
  {
    QgsVectorLayer *vl = qobject_cast<QgsVectorLayer *>( ml );
    if ( !vl )
    {
      continue;
    }

    // get renderer
    QgsFeatureRenderer *r = vl->renderer();
    if ( !r )
    {
      continue;
    }

    // get all symbols
    QgsSymbolList symbols = r->symbols( context );
    QgsSymbolList::iterator symbolIt = symbols.begin();
    for ( ; symbolIt != symbols.end(); ++symbolIt )
    {
      int maxSymbolLayers = ( *symbolIt )->symbolLayerCount();
      if ( mSymbologyExport != SymbolLayerSymbology )
      {
        maxSymbolLayers = 1;
      }
      for ( int i = 0; i < maxSymbolLayers; ++i )
      {
        symbolLayers.append( qMakePair( ( *symbolIt )->symbolLayer( i ), *symbolIt ) );
      }
    }
  }

  return symbolLayers;
}

void QgsDxfExport::writeDefaultLinetypes()
{
  // continuous (Qt solid line)
  const QStringList blockStrings = QStringList() << QStringLiteral( "ByLayer" ) << QStringLiteral( "ByBlock" ) << QStringLiteral( "CONTINUOUS" );
  for ( const QString &ltype : blockStrings )
  {
    writeGroup( 0, QStringLiteral( "LTYPE" ) );
    writeHandle();
    writeGroup( 100, QStringLiteral( "AcDbSymbolTableRecord" ) );
    writeGroup( 100, QStringLiteral( "AcDbLinetypeTableRecord" ) );
    writeGroup( 2, ltype );
    writeGroup( 70, 64 );
    writeGroup( 3, QStringLiteral( "Defaultstyle" ) );
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
  writeLinetype( QStringLiteral( "DASH" ), dashVector, QgsUnitTypes::RenderMapUnits );

  QVector<qreal> dotVector( 2 );
  dotVector[0] = dos;
  dotVector[1] = dss;
  writeLinetype( QStringLiteral( "DOT" ), dotVector, QgsUnitTypes::RenderMapUnits );

  QVector<qreal> dashDotVector( 4 );
  dashDotVector[0] = das;
  dashDotVector[1] = dss;
  dashDotVector[2] = dos;
  dashDotVector[3] = dss;
  writeLinetype( QStringLiteral( "DASHDOT" ), dashDotVector, QgsUnitTypes::RenderMapUnits );

  QVector<qreal> dashDotDotVector( 6 );
  dashDotDotVector[0] = das;
  dashDotDotVector[1] = dss;
  dashDotDotVector[2] = dos;
  dashDotDotVector[3] = dss;
  dashDotDotVector[4] = dos;
  dashDotDotVector[5] = dss;
  writeLinetype( QStringLiteral( "DASHDOTDOT" ), dashDotDotVector, QgsUnitTypes::RenderMapUnits );
}

void QgsDxfExport::writeSymbolLayerLinetype( const QgsSymbolLayer *symbolLayer )
{
  if ( !symbolLayer )
  {
    return;
  }

  QgsUnitTypes::RenderUnit unit;
  QVector<qreal> customLinestyle = symbolLayer->dxfCustomDashPattern( unit );
  if ( !customLinestyle.isEmpty() )
  {
    QString name = QStringLiteral( "symbolLayer%1" ).arg( mSymbolLayerCounter++ );
    writeLinetype( name, customLinestyle, unit );
    mLineStyles.insert( symbolLayer, name );
  }
}

int QgsDxfExport::nLineTypes( const QList< QPair< QgsSymbolLayer *, QgsSymbol * > > &symbolLayers )
{
  int nLineTypes = 0;
  QList< QPair< QgsSymbolLayer *, QgsSymbol *> >::const_iterator slIt = symbolLayers.constBegin();
  for ( ; slIt != symbolLayers.constEnd(); ++slIt )
  {
    const QgsSimpleLineSymbolLayer *simpleLine = dynamic_cast< const QgsSimpleLineSymbolLayer * >( slIt->first );
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

void QgsDxfExport::writeLinetype( const QString &styleName, const QVector<qreal> &pattern, QgsUnitTypes::RenderUnit u )
{
  double length = 0;
  QVector<qreal>::const_iterator dashIt = pattern.constBegin();
  for ( ; dashIt != pattern.constEnd(); ++dashIt )
  {
    length += ( *dashIt * mapUnitScaleFactor( mSymbologyScale, u, mMapUnits, mMapSettings.mapToPixel().mapUnitsPerPixel() ) );
  }

  writeGroup( 0, QStringLiteral( "LTYPE" ) );
  writeHandle();
  // 330 5
  writeGroup( 100, QStringLiteral( "AcDbSymbolTableRecord" ) );
  writeGroup( 100, QStringLiteral( "AcDbLinetypeTableRecord" ) );
  writeGroup( 2, styleName );
  writeGroup( 70, 64 ); // 0?
  writeGroup( 3, QString() );
  writeGroup( 72, 65 );
  writeGroup( 73, pattern.size() );
  writeGroup( 40, length );

  dashIt = pattern.constBegin();
  bool isGap = false;
  for ( ; dashIt != pattern.constEnd(); ++dashIt )
  {
    // map units or mm?
    double segmentLength = ( isGap ? -*dashIt : *dashIt );
    segmentLength *= mapUnitScaleFactor( mSymbologyScale, u, mMapUnits, mMapSettings.mapToPixel().mapUnitsPerPixel() );
    writeGroup( 49, segmentLength );
    writeGroup( 74, 0 );
    isGap = !isGap;
  }
}

void QgsDxfExport::addGeometryGeneratorSymbolLayer( QgsSymbolRenderContext &ctx, const QgsCoordinateTransform &ct, const QString &layer, QgsSymbolLayer *symbolLayer, bool allSymbolLayers )
{
  QgsGeometryGeneratorSymbolLayer *gg = dynamic_cast<QgsGeometryGeneratorSymbolLayer *>( symbolLayer );
  if ( !gg )
  {
    return;
  }

  const QgsFeature *fet = ctx.feature();
  if ( !fet )
  {
    return;
  }

  QgsFeature f = *fet;

  QgsExpressionContext &expressionContext = ctx.renderContext().expressionContext();
  QgsExpression geomExpr( gg->geometryExpression() );
  geomExpr.prepare( &expressionContext );
  QgsGeometry geom = geomExpr.evaluate( &expressionContext ).value<QgsGeometry>();
  f.setGeometry( geom );

  QgsSymbol *symbol = gg->subSymbol();
  if ( symbol && symbol->symbolLayerCount() > 0 )
  {
    QgsExpressionContextScope *symbolExpressionContextScope = symbol->symbolRenderContext()->expressionContextScope();
    symbolExpressionContextScope->setFeature( f );

    ctx.setFeature( &f );

    int nSymbolLayers = allSymbolLayers ? symbol->symbolLayerCount() : 1;
    for ( int i = 0; i < nSymbolLayers; ++i )
    {
      addFeature( ctx, ct, layer, symbol->symbolLayer( i ), symbol );
    }

    ctx.setFeature( fet );
  }
}

bool QgsDxfExport::hasDataDefinedProperties( const QgsSymbolLayer *sl, const QgsSymbol *symbol )
{
  if ( !sl || !symbol )
  {
    return false;
  }

  if ( symbol->renderHints() & QgsSymbol::DynamicRotation )
  {
    return true;
  }

  return sl->hasDataDefinedProperties();
}

double QgsDxfExport::dashSize() const
{
  double size = mSymbologyScale * 0.002;
  return sizeToMapUnits( size );
}

double QgsDxfExport::dotSize() const
{
  double size = mSymbologyScale * 0.0006;
  return sizeToMapUnits( size );
}

double QgsDxfExport::dashSeparatorSize() const
{
  double size = mSymbologyScale * 0.0006;
  return sizeToMapUnits( size );
}

double QgsDxfExport::sizeToMapUnits( double s ) const
{
  double size = s * QgsUnitTypes::fromUnitToUnitFactor( QgsUnitTypes::DistanceMeters, mMapUnits );
  return size;
}

QString QgsDxfExport::lineNameFromPenStyle( Qt::PenStyle style )
{
  switch ( style )
  {
    case Qt::DashLine:
      return QStringLiteral( "DASH" );
    case Qt::DotLine:
      return QStringLiteral( "DOT" );
    case Qt::DashDotLine:
      return QStringLiteral( "DASHDOT" );
    case Qt::DashDotDotLine:
      return QStringLiteral( "DASHDOTDOT" );
    case Qt::SolidLine:
    default:
      return QStringLiteral( "CONTINUOUS" );
  }
}

QString QgsDxfExport::dxfLayerName( const QString &name )
{
  if ( name.isEmpty() )
    return QStringLiteral( "0" );

  // dxf layers can be max 255 characters long
  QString layerName = name.left( 255 );

  // replaced restricted characters with underscore
  // < > / \ " : ; ? * | = '
  // See http://docs.autodesk.com/ACD/2010/ENU/AutoCAD%202010%20User%20Documentation/index.html?url=WS1a9193826455f5ffa23ce210c4a30acaf-7345.htm,topicNumber=d0e41665
  layerName.replace( '<', '_' );
  layerName.replace( '>', '_' );
  layerName.replace( '/', '_' );
  layerName.replace( '\\', '_' );
  layerName.replace( '\"', '_' );
  layerName.replace( ':', '_' );
  layerName.replace( ';', '_' );
  layerName.replace( '?', '_' );
  layerName.replace( '*', '_' );
  layerName.replace( '|', '_' );
  layerName.replace( '=', '_' );
  layerName.replace( '\'', '_' );

  // also remove newline characters (#15067)
  layerName.replace( QLatin1String( "\r\n" ), QLatin1String( "_" ) );
  layerName.replace( '\r', '_' );
  layerName.replace( '\n', '_' );

  return layerName.trimmed();
}

bool QgsDxfExport::layerIsScaleBasedVisible( const QgsMapLayer *layer ) const
{
  if ( !layer )
    return false;

  if ( mSymbologyExport == QgsDxfExport::NoSymbology )
    return true;

  return layer->isInScaleRange( mSymbologyScale );
}

QString QgsDxfExport::layerName( const QString &id, const QgsFeature &f ) const
{
  const QList< QgsMapLayer * > layers = mMapSettings.layers();
  for ( QgsMapLayer *ml : layers )
  {
    QgsVectorLayer *vl = qobject_cast<QgsVectorLayer *>( ml );
    if ( vl && vl->id() == id )
    {
      int attrIdx = mLayerNameAttribute.value( vl->id(), -1 );
      return dxfLayerName( attrIdx < 0 ? layerName( vl ) : f.attribute( attrIdx ).toString() );
    }
  }

  return QStringLiteral( "0" );
}

QString QgsDxfExport::dxfEncoding( const QString &name )
{
  const QList< QByteArray > codecs = QTextCodec::availableCodecs();
  for ( const QByteArray &codec : codecs )
  {
    if ( name != codec )
      continue;

    int i;
    for ( i = 0; i < static_cast< int >( sizeof( DXF_ENCODINGS ) / sizeof( *DXF_ENCODINGS ) ) && name != DXF_ENCODINGS[i][1]; ++i )
      ;

    if ( i == static_cast< int >( sizeof( DXF_ENCODINGS ) / sizeof( *DXF_ENCODINGS ) ) )
      continue;

    return DXF_ENCODINGS[i][0];
  }

  return QString();
}

QStringList QgsDxfExport::encodings()
{
  QStringList encodings;
  const QList< QByteArray > codecs = QTextCodec::availableCodecs();
  for ( const QByteArray &codec : codecs )
  {
    int i;
    for ( i = 0; i < static_cast< int >( sizeof( DXF_ENCODINGS ) / sizeof( *DXF_ENCODINGS ) ) && strcmp( codec.data(), DXF_ENCODINGS[i][1] ) != 0; ++i )
      ;

    if ( i < static_cast< int >( sizeof( DXF_ENCODINGS ) / sizeof( *DXF_ENCODINGS ) ) )
      encodings << codec.data();
  }
  return encodings;
}

QString QgsDxfExport::layerName( QgsVectorLayer *vl ) const
{
  Q_ASSERT( vl );
  return mLayerTitleAsName && !vl->title().isEmpty() ? vl->title() : vl->name();
}

void QgsDxfExport::drawLabel( const QString &layerId, QgsRenderContext &context, pal::LabelPosition *label, const QgsPalLayerSettings &settings )
{
  Q_UNUSED( context );

  if ( !settings.drawLabels )
    return;

  QgsTextLabelFeature *lf = dynamic_cast<QgsTextLabelFeature *>( label->getFeaturePart()->feature() );

  // Copy to temp, editable layer settings
  // these settings will be changed by any data defined values, then used for rendering label components
  // settings may be adjusted during rendering of components
  QgsPalLayerSettings tmpLyr( settings );

  // apply any previously applied data defined settings for the label
  const QMap< QgsPalLayerSettings::Property, QVariant > &ddValues = lf->dataDefinedValues();

  //font
  QFont dFont = lf->definedFont();
  QgsDebugMsgLevel( QStringLiteral( "PAL font tmpLyr: %1, Style: %2" ).arg( tmpLyr.format().font().toString(), tmpLyr.format().font().styleName() ), 4 );
  QgsDebugMsgLevel( QStringLiteral( "PAL font definedFont: %1, Style: %2" ).arg( dFont.toString(), dFont.styleName() ), 4 );

  QgsTextFormat format = tmpLyr.format();
  format.setFont( dFont );
  tmpLyr.setFormat( format );

  if ( tmpLyr.multilineAlign == QgsPalLayerSettings::MultiFollowPlacement )
  {
    //calculate font alignment based on label quadrant
    switch ( label->getQuadrant() )
    {
      case pal::LabelPosition::QuadrantAboveLeft:
      case pal::LabelPosition::QuadrantLeft:
      case pal::LabelPosition::QuadrantBelowLeft:
        tmpLyr.multilineAlign = QgsPalLayerSettings::MultiRight;
        break;
      case pal::LabelPosition::QuadrantAbove:
      case pal::LabelPosition::QuadrantOver:
      case pal::LabelPosition::QuadrantBelow:
        tmpLyr.multilineAlign = QgsPalLayerSettings::MultiCenter;
        break;
      case pal::LabelPosition::QuadrantAboveRight:
      case pal::LabelPosition::QuadrantRight:
      case pal::LabelPosition::QuadrantBelowRight:
        tmpLyr.multilineAlign = QgsPalLayerSettings::MultiLeft;
        break;
    }
  }

  // update tmpLyr with any data defined text style values
  QgsPalLabeling::dataDefinedTextStyle( tmpLyr, ddValues );

  // update tmpLyr with any data defined text buffer values
  QgsPalLabeling::dataDefinedTextBuffer( tmpLyr, ddValues );

  // update tmpLyr with any data defined text formatting values
  QgsPalLabeling::dataDefinedTextFormatting( tmpLyr, ddValues );

  // add to the results
  QString txt = label->getFeaturePart()->feature()->labelText();

  QgsFeatureId fid = label->getFeaturePart()->featureId();
  QString dxfLayer = mDxfLayerNames[layerId][fid];

  QString wrapchr = tmpLyr.wrapChar.isEmpty() ? QStringLiteral( "\n" ) : tmpLyr.wrapChar;

  //add the direction symbol if needed
  if ( !txt.isEmpty() && tmpLyr.placement == QgsPalLayerSettings::Line && tmpLyr.addDirectionSymbol )
  {
    bool prependSymb = false;
    QString symb = tmpLyr.rightDirectionSymbol;

    if ( label->getReversed() )
    {
      prependSymb = true;
      symb = tmpLyr.leftDirectionSymbol;
    }

    if ( tmpLyr.reverseDirectionSymbol )
    {
      if ( symb == tmpLyr.rightDirectionSymbol )
      {
        prependSymb = true;
        symb = tmpLyr.leftDirectionSymbol;
      }
      else
      {
        prependSymb = false;
        symb = tmpLyr.rightDirectionSymbol;
      }
    }

    if ( tmpLyr.placeDirectionSymbol == QgsPalLayerSettings::SymbolAbove )
    {
      prependSymb = true;
      symb = symb + wrapchr;
    }
    else if ( tmpLyr.placeDirectionSymbol == QgsPalLayerSettings::SymbolBelow )
    {
      prependSymb = false;
      symb = wrapchr + symb;
    }

    if ( prependSymb )
    {
      txt.prepend( symb );
    }
    else
    {
      txt.append( symb );
    }
  }

  if ( mFlags & FlagNoMText )
  {
    writeText( dxfLayer, txt, QgsPoint( label->getX(), label->getY() ), label->getHeight(), label->getAlpha() * 180.0 / M_PI, tmpLyr.format().color() );
  }
  else
  {
    txt = txt.replace( wrapchr, QLatin1String( "\\P" ) );
    txt.replace( " ", "\\~" );

    if ( tmpLyr.format().font().underline() )
    {
      txt.prepend( "\\L" ).append( "\\l" );
    }

    if ( tmpLyr.format().font().overline() )
    {
      txt.prepend( "\\O" ).append( "\\o" );
    }

    if ( tmpLyr.format().font().strikeOut() )
    {
      txt.prepend( "\\K" ).append( "\\k" );
    }

    txt.prepend( QStringLiteral( "\\f%1|i%2|b%3;\\H%4;" )
                 .arg( tmpLyr.format().font().family() )
                 .arg( tmpLyr.format().font().italic() ? 1 : 0 )
                 .arg( tmpLyr.format().font().bold() ? 1 : 0 )
                 .arg( label->getHeight() / ( 1 + txt.count( QStringLiteral( "\\P" ) ) ) * 0.75 ) );
    writeMText( dxfLayer, txt, QgsPoint( label->getX(), label->getY() ), label->getWidth(), label->getAlpha() * 180.0 / M_PI, tmpLyr.format().color() );
  }
}


void QgsDxfExport::registerDxfLayer( const QString &layerId, QgsFeatureId fid, const QString &layerName )
{
  if ( !mDxfLayerNames.contains( layerId ) )
    mDxfLayerNames[ layerId ] = QMap<QgsFeatureId, QString>();

  mDxfLayerNames[layerId][fid] = layerName;
}

void QgsDxfExport::setDestinationCrs( const QgsCoordinateReferenceSystem &crs )
{
  mCrs = crs;
  mMapUnits = crs.mapUnits();
}

QgsCoordinateReferenceSystem QgsDxfExport::destinationCrs() const
{
  return mCrs;
}
