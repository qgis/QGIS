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
#include "qgsgeometrygeneratorsymbollayer.h"
#include "qgsgeometrycollection.h"
#include "qgscurvepolygon.h"
#include "qgscompoundcurve.h"
#include "qgscircularstring.h"
#include "qgslinestring.h"
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
#include "qgsexpressioncontextutils.h"
#include "qgsdxfexport_p.h"
#include "qgssymbol.h"

#include "qgswkbtypes.h"
#include "qgspoint.h"
#include "qgsgeos.h"

#include "pal/feature.h"
#include "pal/pointset.h"
#include "pal/labelposition.h"

#include <QIODevice>
#include <QTextCodec>

QgsDxfExport::QgsDxfExport() = default;

QgsDxfExport::~QgsDxfExport()
{
  qDeleteAll( mJobs );
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

  layerList.reserve( layers.size() );
  for ( const DxfLayer &dxfLayer : layers )
  {
    layerList << dxfLayer.layer();
    if ( dxfLayer.layerOutputAttributeIndex() >= 0 )
      mLayerNameAttribute.insert( dxfLayer.layer()->id(), dxfLayer.layerOutputAttributeIndex() );
  }

  mMapSettings.setLayers( layerList );
}

void QgsDxfExport::writeGroup( int code, int i )
{
  writeGroupCode( code );
  writeInt( i );
}

void QgsDxfExport::writeGroup( int code, long long i )
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

QgsDxfExport::ExportResult QgsDxfExport::writeToFile( QIODevice *d, const QString &encoding )
{
  if ( !d )
  {
    return ExportResult::InvalidDeviceError;
  }

  if ( !d->isOpen() && !d->open( QIODevice::WriteOnly | QIODevice::Truncate ) )
  {
    return ExportResult::DeviceNotWritableError;
  }

  mTextStream.setDevice( d );
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
  mTextStream.setCodec( encoding.toLocal8Bit() );
#else
  mTextStream.setEncoding( QStringConverter::encodingForName( encoding.toLocal8Bit() ).value_or( QStringConverter::Utf8 ) );
#endif

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

  if ( mExtent.isEmpty() )
    return ExportResult::EmptyExtentError;

  QgsUnitTypes::DistanceUnit mapUnits = mCrs.mapUnits();
  mMapSettings.setExtent( mExtent );

  int dpi = 96;
  mFactor = 1000 * dpi / mSymbologyScale / 25.4 * QgsUnitTypes::fromUnitToUnitFactor( mapUnits, QgsUnitTypes::DistanceMeters );
  mMapSettings.setOutputSize( QSize( mExtent.width() * mFactor, mExtent.height() * mFactor ) );
  mMapSettings.setOutputDpi( dpi );

  writeHeader( dxfEncoding( encoding ) );
  prepareRenderers();
  writeTables();
  writeBlocks();
  writeEntities();
  writeEndFile();
  stopRenderers();

  return ExportResult::Success;
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
  writeGroup( 0, QgsPoint( QgsWkbTypes::PointZ, mExtent.xMinimum(), mExtent.yMinimum(), 0.0 ) );

  // EXTMAX
  writeGroup( 9, QStringLiteral( "$EXTMAX" ) );
  writeGroup( 0, QgsPoint( QgsWkbTypes::PointZ, mExtent.xMaximum(), mExtent.yMaximum(), 0.0 ) );

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

  writeGroup( code, QString::number( handle, 16 ) );
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
  for ( const auto &symbolLayer : std::as_const( slList ) )
  {
    writeSymbolLayerLinetype( symbolLayer.first );
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
  for ( const auto &symbolLayer : std::as_const( slList ) )
  {
    QgsMarkerSymbolLayer *ml = dynamic_cast< QgsMarkerSymbolLayer *>( symbolLayer.first );
    if ( !ml )
      continue;

    if ( hasDataDefinedProperties( ml, symbolLayer.second ) )
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
  writeGroup( 100, QgsPoint( QgsWkbTypes::PointZ, 0.0, 0.0, 0.0 ) );// UCS origin
  writeGroup( 101, QgsPoint( QgsWkbTypes::PointZ, 1.0, 0.0, 0.0 ) );// UCS x axis
  writeGroup( 102, QgsPoint( QgsWkbTypes::PointZ, 0.0, 1.0, 0.0 ) );// UCS y axis
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

  for ( const QString &layerName : std::as_const( layerNames ) )
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

  static const QStringList blockStrings = QStringList() << QStringLiteral( "*Model_Space" ) << QStringLiteral( "*Paper_Space" ) << QStringLiteral( "*Paper_Space0" );
  for ( const QString &block : blockStrings )
  {
    writeGroup( 0, QStringLiteral( "BLOCK" ) );
    writeHandle();
    writeGroup( 330, QString::number( mBlockHandles[ block ], 16 ) );
    writeGroup( 100, QStringLiteral( "AcDbEntity" ) );
    writeGroup( 8, QStringLiteral( "0" ) );
    writeGroup( 100, QStringLiteral( "AcDbBlockBegin" ) );
    writeGroup( 2, block );
    writeGroup( 70, 0 );
    writeGroup( 0, QgsPoint( QgsWkbTypes::PointZ, 0.0, 0.0, 0.0 ) );
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

  for ( const auto &symbolLayer : std::as_const( slList ) )
  {
    QgsMarkerSymbolLayer *ml = dynamic_cast< QgsMarkerSymbolLayer *>( symbolLayer.first );
    if ( !ml )
      continue;

    // if point symbol layer and no data defined properties: write block
    QgsSymbolRenderContext ctx( ct, QgsUnitTypes::RenderMapUnits, symbolLayer.second->opacity(), false, symbolLayer.second->renderHints(), nullptr );

    // markers with data defined properties are inserted inline
    if ( hasDataDefinedProperties( ml, symbolLayer.second ) )
    {
      continue;
    }

    QString block( QStringLiteral( "symbolLayer%1" ).arg( mBlockCounter++ ) );
    mBlockHandle = QString::number( mBlockHandles[ block ], 16 );

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
    writeGroup( 0, QgsPoint( QgsWkbTypes::PointZ, 0.0, 0.0, 0.0 ) );
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
  }
  endSection();
}


void QgsDxfExport::writeEntities()
{
  startSection();
  writeGroup( 2, QStringLiteral( "ENTITIES" ) );

  mBlockHandle = QString::number( mBlockHandles[ QStringLiteral( "*Model_Space" )], 16 );

  // iterate through the maplayers
  for ( DxfLayerJob *job : std::as_const( mJobs ) )
  {
    QgsSymbolRenderContext sctx( mRenderContext, QgsUnitTypes::RenderMillimeters, 1.0, false, Qgis::SymbolRenderHints(), nullptr );

    if ( mSymbologyExport == QgsDxfExport::SymbolLayerSymbology &&
         ( job->renderer->capabilities() & QgsFeatureRenderer::SymbolLevels ) &&
         job->renderer->usingSymbolLevels() )
    {
      writeEntitiesSymbolLevels( job );

      continue;
    }

    const QgsCoordinateTransform ct( job->crs, mMapSettings.destinationCrs(), mMapSettings.transformContext() );

    QgsFeatureRequest request = QgsFeatureRequest().setSubsetOfAttributes( job->attributes, job->fields ).setExpressionContext( job->renderContext.expressionContext() );
    QgsCoordinateTransform extentTransform = ct;
    extentTransform.setBallparkTransformsAreAppropriate( true );
    request.setFilterRect( extentTransform.transformBoundingBox( mExtent, Qgis::TransformDirection::Reverse ) );

    QgsFeatureIterator featureIt = job->featureSource.getFeatures( request );

    QgsFeature fet;
    while ( featureIt.nextFeature( fet ) )
    {
      mRenderContext.expressionContext().setFeature( fet );
      QString lName( dxfLayerName( job->splitLayerAttribute.isNull() ? job->layerTitle : fet.attribute( job->splitLayerAttribute ).toString() ) );

      sctx.setFeature( &fet );

      if ( !job->renderer->willRenderFeature( fet, mRenderContext ) )
        continue;

      if ( mSymbologyExport == NoSymbology )
      {
        addFeature( sctx, ct, lName, nullptr, nullptr ); // no symbology at all
      }
      else
      {
        const QgsSymbolList symbolList = job->renderer->symbolsForFeature( fet, mRenderContext );
        bool hasSymbology = symbolList.size() > 0;

        if ( hasSymbology && mSymbologyExport == QgsDxfExport::SymbolLayerSymbology ) // symbol layer symbology, but layer does not use symbol levels
        {
          for ( QgsSymbol *symbol : symbolList )
          {
            const QgsSymbolLayerList symbolLayers = symbol->symbolLayers();
            for ( QgsSymbolLayer *symbolLayer : symbolLayers )
            {
              if ( !symbolLayer )
                continue;

              bool isGeometryGenerator = ( symbolLayer->layerType() == QLatin1String( "GeometryGenerator" ) );
              if ( isGeometryGenerator )
              {
                addGeometryGeneratorSymbolLayer( sctx, ct, lName, symbolLayer, true );
              }
              else
              {
                addFeature( sctx, ct, lName, symbolLayer, symbol );
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

        if ( job->labelProvider )
        {
          job->labelProvider->registerFeature( fet, mRenderContext );
          Q_NOWARN_DEPRECATED_PUSH
          registerDxfLayer( job->featureSource.id(), fet.id(), lName );
          Q_NOWARN_DEPRECATED_POP
        }
        else if ( job->ruleBasedLabelProvider )
        {
          job->ruleBasedLabelProvider->registerFeature( fet, mRenderContext );
          Q_NOWARN_DEPRECATED_PUSH
          registerDxfLayer( job->featureSource.id(), fet.id(), lName );
          Q_NOWARN_DEPRECATED_POP
        }
      }
    }
  }

  QImage image( 10, 10, QImage::Format_ARGB32_Premultiplied );
  image.setDotsPerMeterX( 96 / 25.4 * 1000 );
  image.setDotsPerMeterY( 96 / 25.4 * 1000 );
  QPainter painter( &image );
  mRenderContext.setPainter( &painter );

  mRenderContext.labelingEngine()->run( mRenderContext );

  endSection();
}

void QgsDxfExport::prepareRenderers()
{
  Q_ASSERT( mJobs.empty() ); // If this fails, stopRenderers() was not called after the last job

  mRenderContext = QgsRenderContext();
  mRenderContext.setRendererScale( mSymbologyScale );
  mRenderContext.setExtent( mExtent );

  mRenderContext.setScaleFactor( 96.0 / 25.4 );
  mRenderContext.setMapToPixel( QgsMapToPixel( 1.0 / mFactor, mExtent.center().x(), mExtent.center().y(), mExtent.width() * mFactor,
                                mExtent.height() * mFactor, 0 ) );

  mRenderContext.expressionContext().appendScope( QgsExpressionContextUtils::projectScope( QgsProject::instance() ) );
  mRenderContext.expressionContext().appendScope( QgsExpressionContextUtils::globalScope() );
  mRenderContext.expressionContext().appendScope( QgsExpressionContextUtils::mapSettingsScope( mMapSettings ) );

  mLabelingEngine = std::make_unique<QgsDefaultLabelingEngine>();
  mLabelingEngine->setMapSettings( mMapSettings );
  mRenderContext.setLabelingEngine( mLabelingEngine.get() );

  const QList< QgsMapLayer * > layers = mMapSettings.layers();
  for ( QgsMapLayer *ml : layers )
  {
    QgsVectorLayer *vl = qobject_cast<QgsVectorLayer *>( ml );
    if ( !vl )
      continue;

    if ( !vl->renderer() )
      continue;

    if ( !layerIsScaleBasedVisible( vl ) )
      continue;

    QString splitLayerAttribute;
    int splitLayerAttributeIndex = mLayerNameAttribute.value( vl->id(), -1 );
    const QgsFields fields = vl->fields();
    if ( splitLayerAttributeIndex >= 0 && splitLayerAttributeIndex < fields.size() )
      splitLayerAttribute = fields.at( splitLayerAttributeIndex ).name();
    DxfLayerJob *job = new DxfLayerJob( vl, mMapSettings.layerStyleOverrides().value( vl->id() ), mRenderContext, this, splitLayerAttribute );
    mJobs.append( job );
  }
}

void QgsDxfExport::writeEntitiesSymbolLevels( DxfLayerJob *job )
{
  QHash< QgsSymbol *, QList<QgsFeature> > features;

  QgsRenderContext ctx = renderContext();
  const QList<QgsExpressionContextScope *> scopes = job->renderContext.expressionContext().scopes();
  for ( QgsExpressionContextScope *scope : scopes )
    ctx.expressionContext().appendScope( new QgsExpressionContextScope( *scope ) );
  QgsSymbolRenderContext sctx( ctx, QgsUnitTypes::RenderMillimeters, 1.0, false, Qgis::SymbolRenderHints(), nullptr );

  // get iterator
  QgsFeatureRequest req;
  req.setSubsetOfAttributes( job->renderer->usedAttributes( ctx ), job->featureSource.fields() );
  QgsCoordinateTransform ct( mMapSettings.destinationCrs(), job->crs, mMapSettings.transformContext() );
  req.setFilterRect( ct.transform( mExtent ) );

  QgsFeatureIterator fit = job->featureSource.getFeatures( req );

  // fetch features
  QgsFeature fet;
  QgsSymbol *featureSymbol = nullptr;
  while ( fit.nextFeature( fet ) )
  {
    ctx.expressionContext().setFeature( fet );
    featureSymbol = job->renderer->symbolForFeature( fet, ctx );
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
  const QgsSymbolList symbols = job->renderer->symbols( ctx );
  for ( QgsSymbol *symbol : symbols )
  {
    for ( int j = 0; j < symbol->symbolLayerCount(); j++ )
    {
      int level = symbol->symbolLayer( j )->renderingPass();
      if ( level < 0 || level >= 1000 ) // ignore invalid levels
        continue;
      QgsSymbolLevelItem item( symbol, j );
      while ( level >= levels.count() ) // append new empty levels
        levels.append( QgsSymbolLevel() );
      levels[level].append( item );
    }
  }

  // export symbol layers and symbology
  for ( const QgsSymbolLevel &level : std::as_const( levels ) )
  {
    for ( const QgsSymbolLevelItem &item : level )
    {
      QHash< QgsSymbol *, QList<QgsFeature> >::iterator levelIt = features.find( item.symbol() );
      if ( levelIt == features.end() )
      {
        continue;
      }

      int llayer = item.layer();
      const QList<QgsFeature> &featureList = levelIt.value();
      for ( const QgsFeature &feature : featureList )
      {
        sctx.setFeature( &feature );
        addFeature( sctx, ct, job->layerName, levelIt.key()->symbolLayer( llayer ), levelIt.key() );
      }
    }
  }
}

void QgsDxfExport::stopRenderers()
{
  qDeleteAll( mJobs );
  mJobs.clear();
}

void QgsDxfExport::writeEndFile()
{
  mTextStream << DXF_TRAILER;

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
      if ( msl->writeDxf( *this, mapUnitScaleFactor( mSymbologyScale, msl->sizeUnit(), mMapUnits, ctx.renderContext().mapToPixel().mapUnitsPerPixel() ), layer, ctx, QPointF( pt.x(), pt.y() ) ) )
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

  if ( n < 2 )
  {
    QgsDebugMsg( QStringLiteral( "writePolyline: line too short layer=%1 lineStyleName=%2" ).arg( layer, lineStyleName ) );
    return;
  }

  if ( mForce2d || !line.at( 0 ).is3D() )
  {
    bool polygon = line[0] == line[ line.size() - 1 ];
    if ( polygon )
      --n;

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
    writeGroup( 0, QgsPoint( QgsWkbTypes::PointZ, 0.0, 0.0, 0.0 ) );
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

void QgsDxfExport::appendCurve( const QgsCurve &c, QVector<QgsPoint> &points, QVector<double> &bulges )
{
  switch ( QgsWkbTypes::flatType( c.wkbType() ) )
  {
    case QgsWkbTypes::LineString:
      appendLineString( *dynamic_cast<const QgsLineString *>( &c ), points, bulges );
      break;

    case QgsWkbTypes::CircularString:
      appendCircularString( *dynamic_cast<const QgsCircularString *>( &c ), points, bulges );
      break;

    case QgsWkbTypes::CompoundCurve:
      appendCompoundCurve( *dynamic_cast<const QgsCompoundCurve *>( &c ), points, bulges );
      break;

    default:
      QgsDebugMsg( QStringLiteral( "Unexpected curve type %1" ).arg( c.wktTypeStr() ) );
      break;
  }
}

void QgsDxfExport::appendLineString( const QgsLineString &ls, QVector<QgsPoint> &points, QVector<double> &bulges )
{
  for ( int i = 0; i < ls.numPoints(); i++ )
  {
    const QgsPoint &p = ls.pointN( i );
    if ( !points.isEmpty() && points.last() == p )
      continue;

    points << p;
    bulges << 0.0;
  }
}

void QgsDxfExport::appendCircularString( const QgsCircularString &cs, QVector<QgsPoint> &points, QVector<double> &bulges )
{
  for ( int i = 0; i < cs.numPoints() - 2; i += 2 )
  {
    const QgsPoint &p1 = cs.pointN( i );
    const QgsPoint &p2 = cs.pointN( i + 1 );
    const QgsPoint &p3 = cs.pointN( i + 2 );

    if ( points.isEmpty() || points.last() != p1 )
      points << p1;
    else if ( !bulges.isEmpty() )
      bulges.removeLast();

    double a = ( M_PI - ( p1 - p2 ).angle() + ( p3 - p2 ).angle() ) / 2.0;
    bulges << sin( a ) / cos( a );

    points << p3;
    bulges << 0.0;
  }
}

void QgsDxfExport::appendCompoundCurve( const QgsCompoundCurve &cc, QVector<QgsPoint> &points, QVector<double> &bulges )
{
  for ( int i = 0; i < cc.nCurves(); i++ )
  {
    const QgsCurve *c = cc.curveAt( i );
    Q_ASSERT( c );
    appendCurve( *c, points, bulges );
  }
}

void QgsDxfExport::writePolyline( const QgsCurve &curve, const QString &layer, const QString &lineStyleName, const QColor &color, double width )
{
  int n = curve.numPoints();
  if ( n == 0 )
  {
    QgsDebugMsg( QStringLiteral( "writePolyline: empty line layer=%1 lineStyleName=%2" ).arg( layer, lineStyleName ) );
    return;
  }

  if ( n < 2 )
  {
    QgsDebugMsg( QStringLiteral( "writePolyline: line too short layer=%1 lineStyleName=%2" ).arg( layer, lineStyleName ) );
    return;
  }

  QVector<QgsPoint> points;
  QVector<double> bulges;
  appendCurve( curve, points, bulges );

  if ( mForce2d || !curve.is3D() )
  {
    writeGroup( 0, QStringLiteral( "LWPOLYLINE" ) );
    writeHandle();
    writeGroup( 8, layer );
    writeGroup( 100, QStringLiteral( "AcDbEntity" ) );
    writeGroup( 100, QStringLiteral( "AcDbPolyline" ) );
    writeGroup( 6, lineStyleName );
    writeGroup( color );

    writeGroup( 90, points.size() );
    QgsDxfExport::DxfPolylineFlags polylineFlags;
    if ( curve.isClosed() )
      polylineFlags.setFlag( QgsDxfExport::DxfPolylineFlag::Closed );
    if ( curve.hasCurvedSegments() )
      polylineFlags.setFlag( QgsDxfExport::DxfPolylineFlag::Curve );

    // Might need to conditional once this feature is implemented
    //   https://github.com/qgis/QGIS/issues/32468
    polylineFlags.setFlag( QgsDxfExport::DxfPolylineFlag::ContinuousPattern );

    writeGroup( 70, static_cast<int>( polylineFlags ) );
    writeGroup( 43, width );

    for ( int i = 0; i < points.size(); i++ )
    {
      writeGroup( 0, points[i] );
      if ( bulges[i] != 0.0 )
        writeGroup( 42, bulges[i] );
    }
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
    writeGroup( 0, QgsPoint( QgsWkbTypes::PointZ, 0.0, 0.0, 0.0 ) );
    writeGroup( 70, 8 );

    for ( int i = 0; i < points.size(); i++ )
    {
      writeGroup( 0, QStringLiteral( "VERTEX" ) );
      writeHandle();
      writeGroup( 330, plHandle );
      writeGroup( 100, QStringLiteral( "AcDbEntity" ) );
      writeGroup( 8, layer );
      writeGroup( color );
      writeGroup( 100, QStringLiteral( "AcDbVertex" ) );
      writeGroup( 100, QStringLiteral( "AcDb3dPolylineVertex" ) );
      writeGroup( 0, points[i] );
      if ( bulges[i] != 0.0 )
        writeGroup( 42, bulges[i] );
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

  writeGroup( 0, QgsPoint( QgsWkbTypes::PointZ, 0.0, 0.0, 0.0 ) ); // Elevation point (in OCS)
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
    writeGroup( 93, polygon[i].size() ); // Number of edges in this boundary path (only if boundary is not a polyline)

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

void QgsDxfExport::writePolygon( const QgsCurvePolygon &polygon, const QString &layer, const QString &hatchPattern, const QColor &color )
{
  writeGroup( 0, QStringLiteral( "HATCH" ) );       // Entity type
  writeHandle();
  writeGroup( 330, mBlockHandle );
  writeGroup( 100, QStringLiteral( "AcDbEntity" ) );
  writeGroup( 8, layer );           // Layer name
  writeGroup( color );              // Color
  writeGroup( 100, QStringLiteral( "AcDbHatch" ) );

  writeGroup( 0, QgsPoint( QgsWkbTypes::PointZ, 0.0, 0.0, 0.0 ) ); // Elevation point (in OCS)
  writeGroup( 200, QgsPoint( QgsWkbTypes::PointZ, 0.0, 0.0, 1.0 ) );

  writeGroup( 2, hatchPattern );  // Hatch pattern name
  writeGroup( 70, hatchPattern == QLatin1String( "SOLID" ) ); // Solid fill flag (solid fill = 1; pattern fill = 0)
  writeGroup( 71, 0 );    // Associativity flag (associative = 1; non-associative = 0)

  QVector<QVector<QgsPoint>> points;
  QVector<QVector<double>> bulges;

  const int ringCount = polygon.numInteriorRings();
  points.reserve( ringCount + 1 );
  bulges.reserve( ringCount + 1 );

  points << QVector<QgsPoint>();
  bulges << QVector<double>();
  appendCurve( *polygon.exteriorRing(), points.last(), bulges.last() );

  for ( int i = 0; i < ringCount; i++ )
  {
    points << QVector<QgsPoint>();
    bulges << QVector<double>();
    appendCurve( *polygon.interiorRing( i ), points.last(), bulges.last() );
  }

  bool hasBulges = false;
  for ( int i = 0; i < points.size() && !hasBulges; ++i )
    for ( int j = 0; j < points[i].size() && !hasBulges; ++j )
      hasBulges = bulges[i][j] != 0.0;

  writeGroup( 91, points.size() );  // Number of boundary paths (loops)

  for ( int i = 0; i < points.size(); ++i )
  {
    writeGroup( 92, 2 );   // Boundary path type flag (bit coded): 0 = Default; 1 = External; 2 = Polyline 4 = Derived; 8 = Textbox; 16 = Outermost
    writeGroup( 72, hasBulges ? 1 : 0 );   // Has bulge flag
    writeGroup( 73, 1 );   // Is closed flag
    writeGroup( 93, points[i].size() ); // Number of edges in this boundary path (only if boundary is not a polyline)

    for ( int j = 0; j < points[i].size(); ++j )
    {
      writeGroup( 0, points[i][j] ); // Vertex location (in OCS)
      if ( hasBulges )
        writeGroup( 42, bulges[i][j] );
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

void QgsDxfExport::writeText( const QString &layer, const QString &text, pal::LabelPosition *label, const QgsPalLayerSettings &layerSettings, const QgsExpressionContext &expressionContext )
{

  double lblX = label->getX();
  double lblY = label->getY();

  QgsLabelFeature *labelFeature = label->getFeaturePart()->feature();

  HAlign hali = HAlign::Undefined;
  VAlign vali = VAlign::Undefined;

  const QgsPropertyCollection &props = layerSettings.dataDefinedProperties();

  if ( layerSettings.placement == QgsPalLayerSettings::Placement::OverPoint )
  {
    lblX = labelFeature->anchorPosition().x();
    lblY = labelFeature->anchorPosition().y();

    QgsPalLayerSettings::QuadrantPosition offsetQuad = layerSettings.quadOffset;

    if ( props.isActive( QgsPalLayerSettings::OffsetQuad ) )
    {
      const QVariant exprVal = props.value( QgsPalLayerSettings::OffsetQuad, expressionContext );
      if ( !exprVal.isNull() )
      {
        offsetQuad = static_cast<QgsPalLayerSettings::QuadrantPosition>( exprVal.toInt() );
      }
    }

    switch ( offsetQuad )
    {
      case QgsPalLayerSettings::QuadrantPosition::QuadrantAboveLeft:
        hali = HAlign::HRight;
        vali = VAlign::VBottom;
        break;
      case QgsPalLayerSettings::QuadrantPosition::QuadrantAbove:
        hali = HAlign::HCenter;
        vali = VAlign::VBottom;
        break;
      case QgsPalLayerSettings::QuadrantPosition::QuadrantAboveRight:
        hali = HAlign::HLeft;
        vali = VAlign::VBottom;
        break;
      case QgsPalLayerSettings::QuadrantPosition::QuadrantLeft:
        hali = HAlign::HRight;
        vali = VAlign::VMiddle;
        break;
      case QgsPalLayerSettings::QuadrantPosition::QuadrantOver:
        hali = HAlign::HCenter;
        vali = VAlign::VMiddle;
        break;
      case QgsPalLayerSettings::QuadrantPosition::QuadrantRight:
        hali = HAlign::HLeft;
        vali = VAlign::VMiddle;
        break;
      case QgsPalLayerSettings::QuadrantPosition::QuadrantBelowLeft:
        hali = HAlign::HRight;
        vali = VAlign::VTop;
        break;
      case QgsPalLayerSettings::QuadrantPosition::QuadrantBelow:
        hali = HAlign::HCenter;
        vali = VAlign::VTop;
        break;
      case QgsPalLayerSettings::QuadrantPosition::QuadrantBelowRight:
        hali = HAlign::HLeft;
        vali = VAlign::VTop;
        break;
    }
  }

  if ( props.isActive( QgsPalLayerSettings::Hali ) )
  {
    lblX = labelFeature->anchorPosition().x();
    lblY = labelFeature->anchorPosition().y();

    hali = HAlign::HLeft;
    QVariant exprVal = props.value( QgsPalLayerSettings::Hali, expressionContext );
    if ( !exprVal.isNull() )
    {
      const QString haliString = exprVal.toString();
      if ( haliString.compare( QLatin1String( "Center" ), Qt::CaseInsensitive ) == 0 )
      {
        hali = HAlign::HCenter;
      }
      else if ( haliString.compare( QLatin1String( "Right" ), Qt::CaseInsensitive ) == 0 )
      {
        hali = HAlign::HRight;
      }
    }
  }

  //vertical alignment
  if ( props.isActive( QgsPalLayerSettings::Vali ) )
  {
    vali = VAlign::VBottom;
    QVariant exprVal = props.value( QgsPalLayerSettings::Vali, expressionContext );
    if ( !exprVal.isNull() )
    {
      const QString valiString = exprVal.toString();
      if ( valiString.compare( QLatin1String( "Bottom" ), Qt::CaseInsensitive ) != 0 )
      {
        if ( valiString.compare( QLatin1String( "Base" ), Qt::CaseInsensitive ) == 0 )
        {
          vali = VAlign::VBaseLine;
        }
        else if ( valiString.compare( QLatin1String( "Half" ), Qt::CaseInsensitive ) == 0 )
        {
          vali = VAlign::VMiddle;
        }
        else  //'Cap' or 'Top'
        {
          vali = VAlign::VTop;
        }
      }
    }
  }

  writeText( layer, text, QgsPoint( lblX, lblY ), label->getHeight(), label->getAlpha() * 180.0 / M_PI, layerSettings.format().color(), hali, vali );
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

  writeGroup( 0, QgsPoint( QgsWkbTypes::PointZ, 0.0, 0.0, 0.0 ) ); // Elevation point (in OCS)
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

void QgsDxfExport::writeText( const QString &layer, const QString &text, const QgsPoint &pt, double size, double angle, const QColor &color, HAlign hali, VAlign vali )
{
  writeGroup( 0, QStringLiteral( "TEXT" ) );
  writeHandle();
  writeGroup( 100, QStringLiteral( "AcDbEntity" ) );
  // writeGroup( 6, "Continuous" ); // Line style
  // writeGroup( 370, 18 ); // Line weight
  writeGroup( 100, QStringLiteral( "AcDbText" ) );
  writeGroup( 8, layer );
  writeGroup( color );
  writeGroup( 0, pt );
  if ( hali != HAlign::Undefined || vali != VAlign::Undefined )
    writeGroup( 1, pt ); // Second alignment point
  writeGroup( 40, size );
  writeGroup( 1, text );
  writeGroup( 50, fmod( angle, 360 ) );
  if ( hali != HAlign::Undefined )
    writeGroup( 72, static_cast<int>( hali ) );
  writeGroup( 7, QStringLiteral( "STANDARD" ) ); // so far only support for standard font
  writeGroup( 100, QStringLiteral( "AcDbText" ) );
  if ( vali != VAlign::Undefined )
  {
    writeGroup( 73, static_cast<int>( vali ) );
  }
}

void QgsDxfExport::writeMText( const QString &layer, const QString &text, const QgsPoint &pt, double width, double angle, const QColor &color )
{
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
  if ( !mTextStream.codec()->canEncode( text ) )
  {
    // TODO return error
    QgsDebugMsg( QStringLiteral( "could not encode:%1" ).arg( text ) );
    return;
  }
#endif

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

  QgsGeometry geom( fet->geometry() );
  if ( ct.isValid() )
  {
    geom.transform( ct );
  }

  QgsWkbTypes::Type geometryType = geom.wkbType();

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
    writePoint( geom.constGet()->coordinateSequence().at( 0 ).at( 0 ).at( 0 ), layer, penColor, ctx, symbolLayer, symbol, angle );
    return;
  }

  if ( QgsWkbTypes::flatType( geometryType ) == QgsWkbTypes::MultiPoint )
  {
    const QgsCoordinateSequence &cs = geom.constGet()->coordinateSequence();
    for ( int i = 0; i < cs.size(); i++ )
    {
      writePoint( cs.at( i ).at( 0 ).at( 0 ), layer, penColor, ctx, symbolLayer, symbol, angle );
    }
    return;
  }

  if ( penStyle != Qt::NoPen )
  {
    const QgsAbstractGeometry *sourceGeom = geom.constGet();
    std::unique_ptr< QgsAbstractGeometry > tempGeom;

    switch ( QgsWkbTypes::flatType( geometryType ) )
    {
      case QgsWkbTypes::CircularString:
      case QgsWkbTypes::CompoundCurve:
      case QgsWkbTypes::LineString:
      case QgsWkbTypes::MultiCurve:
      case QgsWkbTypes::MultiLineString:
      {
        if ( !qgsDoubleNear( offset, 0.0 ) )
        {
          QgsGeos geos( sourceGeom );
          tempGeom.reset( geos.offsetCurve( offset, 0, Qgis::JoinStyle::Miter, 2.0 ) );  //#spellok
          if ( tempGeom )
            sourceGeom = tempGeom.get();
          else
            sourceGeom = geom.constGet();
        }

        const QgsCurve *curve = dynamic_cast<const QgsCurve *>( sourceGeom );
        if ( curve )
        {
          writePolyline( *curve, layer, lineStyleName, penColor, width );
        }
        else
        {
          const QgsGeometryCollection *gc = dynamic_cast<const QgsGeometryCollection *>( sourceGeom );
          Q_ASSERT( gc );
          if ( gc )
          {
            for ( int i = 0; i < gc->numGeometries(); i++ )
            {
              const QgsCurve *curve = dynamic_cast<const QgsCurve *>( gc->geometryN( i ) );
              Q_ASSERT( curve );
              writePolyline( *curve, layer, lineStyleName, penColor, width );
            }
          }
        }
        break;
      }

      case QgsWkbTypes::CurvePolygon:
      case QgsWkbTypes::Polygon:
      case QgsWkbTypes::MultiSurface:
      case QgsWkbTypes::MultiPolygon:
      {
        if ( !qgsDoubleNear( offset, 0.0 ) )
        {
          QgsGeos geos( sourceGeom );
          tempGeom.reset( geos.buffer( offset, 0, Qgis::EndCapStyle::Flat, Qgis::JoinStyle::Miter, 2.0 ) );  //#spellok
          if ( tempGeom )
            sourceGeom = tempGeom.get();
          else
            sourceGeom = geom.constGet();
        }

        const QgsCurvePolygon *polygon = dynamic_cast<const QgsCurvePolygon *>( sourceGeom );
        if ( polygon )
        {
          writePolyline( *polygon->exteriorRing(), layer, lineStyleName, penColor, width );
          for ( int i = 0; i < polygon->numInteriorRings(); i++ )
            writePolyline( *polygon->interiorRing( i ), layer, lineStyleName, penColor, width );
        }
        else
        {
          const QgsGeometryCollection *gc = dynamic_cast<const QgsGeometryCollection *>( sourceGeom );
          Q_ASSERT( gc );
          if ( gc )
          {
            for ( int i = 0; i < gc->numGeometries(); i++ )
            {
              const QgsCurvePolygon *polygon = dynamic_cast<const QgsCurvePolygon *>( gc->geometryN( i ) );
              Q_ASSERT( polygon );

              writePolyline( *polygon->exteriorRing(), layer, lineStyleName, penColor, width );
              for ( int j = 0; j < polygon->numInteriorRings(); j++ )
                writePolyline( *polygon->interiorRing( j ), layer, lineStyleName, penColor, width );
            }
          }
        }

        break;
      }

      default:
        break;
    }

  }

  if ( brushStyle != Qt::NoBrush )
  {
    const QgsAbstractGeometry *sourceGeom = geom.constGet();
    std::unique_ptr< QgsAbstractGeometry > tempGeom;

    switch ( QgsWkbTypes::flatType( geometryType ) )
    {
      case QgsWkbTypes::CurvePolygon:
      case QgsWkbTypes::Polygon:
      {
        const QgsCurvePolygon *polygon = dynamic_cast<const QgsCurvePolygon *>( sourceGeom );
        Q_ASSERT( polygon );
        writePolygon( *polygon, layer, QStringLiteral( "SOLID" ), brushColor );
        break;
      }

      case QgsWkbTypes::MultiSurface:
      case QgsWkbTypes::MultiPolygon:
      {
        const QgsGeometryCollection *gc = dynamic_cast<const QgsGeometryCollection *>( sourceGeom );
        Q_ASSERT( gc );

        for ( int i = 0; i < gc->numGeometries(); i++ )
        {
          const QgsCurvePolygon *polygon = dynamic_cast<const QgsCurvePolygon *>( gc->geometryN( i ) );
          Q_ASSERT( polygon );
          writePolygon( *polygon, layer, QStringLiteral( "SOLID" ), brushColor );
        }
        break;
      }

      default:
        break;

    }
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
  return mRenderContext;
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

  for ( DxfLayerJob *job : std::as_const( mJobs ) )
  {
    const QgsSymbolList symbols = job->renderer->symbols( context );

    for ( QgsSymbol *symbol : symbols )
    {
      int maxSymbolLayers = symbol->symbolLayerCount();
      if ( mSymbologyExport != SymbolLayerSymbology )
      {
        maxSymbolLayers = 1;
      }
      for ( int i = 0; i < maxSymbolLayers; ++i )
      {
        symbolLayers.append( qMakePair( symbol->symbolLayer( i ), symbol ) );
      }
    }
  }

  return symbolLayers;
}

void QgsDxfExport::writeDefaultLinetypes()
{
  // continuous (Qt solid line)
  for ( const QString &ltype : { QStringLiteral( "ByLayer" ), QStringLiteral( "ByBlock" ), QStringLiteral( "CONTINUOUS" ) } )
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
  for ( const auto &symbolLayer : symbolLayers )
  {
    const QgsSimpleLineSymbolLayer *simpleLine = dynamic_cast< const QgsSimpleLineSymbolLayer * >( symbolLayer.first );
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
  for ( qreal size : pattern )
  {
    length += ( size * mapUnitScaleFactor( mSymbologyScale, u, mMapUnits, mMapSettings.mapToPixel().mapUnitsPerPixel() ) );
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

  bool isGap = false;
  for ( qreal size : pattern )
  {
    // map units or mm?
    double segmentLength = ( isGap ? -size : size );
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

  if ( symbol->renderHints() & Qgis::SymbolRenderHint::DynamicRotation )
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
  // if layer name contains comma, resulting file is unreadable in AutoCAD
  // see https://github.com/qgis/QGIS/issues/47381
  layerName.replace( ',', '_' );

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
  // TODO: make this thread safe
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
  encodings.reserve( codecs.size() );
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
  Q_UNUSED( context )

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
  if ( !txt.isEmpty() && tmpLyr.placement == QgsPalLayerSettings::Line && tmpLyr.lineSettings().addDirectionSymbol() )
  {
    bool prependSymb = false;
    QString symb = tmpLyr.lineSettings().rightDirectionSymbol();

    if ( label->getReversed() )
    {
      prependSymb = true;
      symb = tmpLyr.lineSettings().leftDirectionSymbol();
    }

    if ( tmpLyr.lineSettings().reverseDirectionSymbol() )
    {
      if ( symb == tmpLyr.lineSettings().rightDirectionSymbol() )
      {
        prependSymb = true;
        symb = tmpLyr.lineSettings().leftDirectionSymbol();
      }
      else
      {
        prependSymb = false;
        symb = tmpLyr.lineSettings().rightDirectionSymbol();
      }
    }

    switch ( tmpLyr.lineSettings().directionSymbolPlacement() )
    {
      case QgsLabelLineSettings::DirectionSymbolPlacement::SymbolAbove:
        prependSymb = true;
        symb = symb + wrapchr;
        break;

      case QgsLabelLineSettings::DirectionSymbolPlacement::SymbolBelow:
        prependSymb = false;
        symb = wrapchr + symb;
        break;

      case QgsLabelLineSettings::DirectionSymbolPlacement::SymbolLeftRight:
        break;
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
    txt.replace( QChar( QChar::LineFeed ), ' ' );
    txt.replace( QChar( QChar::CarriageReturn ), ' ' );
    writeText( dxfLayer, txt, label, tmpLyr, context.expressionContext() );
  }
  else
  {
    txt.replace( QString( QChar( QChar::CarriageReturn ) ) + QString( QChar( QChar::LineFeed ) ), QStringLiteral( "\\P" ) );
    txt.replace( QChar( QChar::CarriageReturn ), QStringLiteral( "\\P" ) );
    txt = txt.replace( wrapchr, QLatin1String( "\\P" ) );
    txt.replace( QLatin1String( " " ), QLatin1String( "\\~" ) );

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

QString QgsDxfExport::DxfLayer::splitLayerAttribute() const
{
  QString splitLayerFieldName;
  const QgsFields fields = mLayer->fields();
  if ( mLayerOutputAttributeIndex >= 0 && mLayerOutputAttributeIndex < fields.size() )
  {
    splitLayerFieldName = fields.at( mLayerOutputAttributeIndex ).name();
  }

  return splitLayerFieldName;
}
