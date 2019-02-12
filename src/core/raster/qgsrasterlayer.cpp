/***************************************************************************
        qgsrasterlayer.cpp -  description
   -------------------
begin                : Sat Jun 22 2002
copyright            : (C) 2003 by Tim Sutton, Steve Halasz and Gary E.Sherman
email                : tim at linfiniti.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "qgsapplication.h"
#include "qgsbrightnesscontrastfilter.h"
#include "qgscolorrampshader.h"
#include "qgscoordinatereferencesystem.h"
#include "qgscoordinatetransform.h"
#include "qgsdatasourceuri.h"
#include "qgshuesaturationfilter.h"
#include "qgslayermetadataformatter.h"
#include "qgslogger.h"
#include "qgsmaplayerlegend.h"
#include "qgsmaptopixel.h"
#include "qgsmessagelog.h"
#include "qgsmultibandcolorrenderer.h"
#include "qgspainting.h"
#include "qgspalettedrasterrenderer.h"
#include "qgspathresolver.h"
#include "qgsprojectfiletransform.h"
#include "qgsproviderregistry.h"
#include "qgsrasterdataprovider.h"
#include "qgsrasterdrawer.h"
#include "qgsrasteriterator.h"
#include "qgsrasterlayer.h"
#include "qgsrasterlayerrenderer.h"
#include "qgsrasterprojector.h"
#include "qgsrasterrange.h"
#include "qgsrasterrendererregistry.h"
#include "qgsrasterresamplefilter.h"
#include "qgsrastershader.h"
#include "qgsreadwritecontext.h"
#include "qgsrectangle.h"
#include "qgsrendercontext.h"
#include "qgssinglebandcolordatarenderer.h"
#include "qgssinglebandgrayrenderer.h"
#include "qgssinglebandpseudocolorrenderer.h"
#include "qgssettings.h"
#include "qgssymbollayerutils.h"

#include <cmath>
#include <cstdio>
#include <limits>
#include <typeinfo>

#include <QApplication>
#include <QCursor>
#include <QDir>
#include <QDomElement>
#include <QDomNode>
#include <QFile>
#include <QFileInfo>
#include <QFont>
#include <QFontMetrics>
#include <QFrame>
#include <QImage>
#include <QLabel>
#include <QLibrary>
#include <QList>
#include <QMatrix>
#include <QMessageBox>
#include <QPainter>
#include <QPixmap>
#include <QRegExp>
#include <QSlider>
#include <QTime>

// typedefs for provider plugin functions of interest
typedef bool isvalidrasterfilename_t( QString const &fileNameQString, QString &retErrMsg );

#define ERR(message) QGS_ERROR_MESSAGE(message,"Raster layer")

const double QgsRasterLayer::SAMPLE_SIZE = 250000;

const QgsContrastEnhancement::ContrastEnhancementAlgorithm
QgsRasterLayer::SINGLE_BAND_ENHANCEMENT_ALGORITHM = QgsContrastEnhancement::StretchToMinimumMaximum;
const QgsContrastEnhancement::ContrastEnhancementAlgorithm
QgsRasterLayer::MULTIPLE_BAND_SINGLE_BYTE_ENHANCEMENT_ALGORITHM = QgsContrastEnhancement::NoEnhancement;
const QgsContrastEnhancement::ContrastEnhancementAlgorithm
QgsRasterLayer::MULTIPLE_BAND_MULTI_BYTE_ENHANCEMENT_ALGORITHM = QgsContrastEnhancement::StretchToMinimumMaximum;

const QgsRasterMinMaxOrigin::Limits
QgsRasterLayer::SINGLE_BAND_MIN_MAX_LIMITS = QgsRasterMinMaxOrigin::MinMax;
const QgsRasterMinMaxOrigin::Limits
QgsRasterLayer::MULTIPLE_BAND_SINGLE_BYTE_MIN_MAX_LIMITS = QgsRasterMinMaxOrigin::MinMax;
const QgsRasterMinMaxOrigin::Limits
QgsRasterLayer::MULTIPLE_BAND_MULTI_BYTE_MIN_MAX_LIMITS = QgsRasterMinMaxOrigin::CumulativeCut;

QgsRasterLayer::QgsRasterLayer()
  : QgsMapLayer( RasterLayer )
  , QSTRING_NOT_SET( QStringLiteral( "Not Set" ) )
  , TRSTRING_NOT_SET( tr( "Not Set" ) )

{
  init();
  mValid = false;
}

QgsRasterLayer::QgsRasterLayer( const QString &uri,
                                const QString &baseName,
                                const QString &providerKey,
                                const LayerOptions &options )
  : QgsMapLayer( RasterLayer, baseName, uri )
    // Constant that signals property not used.
  , QSTRING_NOT_SET( QStringLiteral( "Not Set" ) )
  , TRSTRING_NOT_SET( tr( "Not Set" ) )
{
  QgsDebugMsgLevel( QStringLiteral( "Entered" ), 4 );
  setProviderType( providerKey );

  QgsDataProvider::ProviderOptions providerOptions;

  setDataSource( uri, baseName, providerKey, providerOptions, options.loadDefaultStyle );

} // QgsRasterLayer ctor

QgsRasterLayer::~QgsRasterLayer()
{
  emit willBeDeleted();

  mValid = false;
  // Note: provider and other interfaces are owned and deleted by pipe
}

QgsRasterLayer *QgsRasterLayer::clone() const
{
  QgsRasterLayer *layer = new QgsRasterLayer( source(), name(), mProviderKey );
  QgsMapLayer::clone( layer );

  // do not clone data provider which is the first element in pipe
  for ( int i = 1; i < mPipe.size(); i++ )
  {
    if ( mPipe.at( i ) )
      layer->pipe()->set( mPipe.at( i )->clone() );
  }

  return layer;
}

//////////////////////////////////////////////////////////
//
// Static Methods and members
//
/////////////////////////////////////////////////////////

bool QgsRasterLayer::isValidRasterFileName( const QString &fileNameQString, QString &retErrMsg )
{
  isvalidrasterfilename_t *pValid = reinterpret_cast< isvalidrasterfilename_t * >( cast_to_fptr( QgsProviderRegistry::instance()->function( "gdal",  "isValidRasterFileName" ) ) );
  if ( ! pValid )
  {
    QgsDebugMsg( QStringLiteral( "Could not resolve isValidRasterFileName in gdal provider library" ) );
    return false;
  }

  bool myIsValid = pValid( fileNameQString, retErrMsg );
  return myIsValid;
}

bool QgsRasterLayer::isValidRasterFileName( QString const &fileNameQString )
{
  QString retErrMsg;
  return isValidRasterFileName( fileNameQString, retErrMsg );
}

QDateTime QgsRasterLayer::lastModified( QString const &name )
{
  QgsDebugMsgLevel( "name=" + name, 4 );
  QDateTime t;

  QFileInfo fi( name );

  // Is it file?
  if ( !fi.exists() )
    return t;

  t = fi.lastModified();

  QgsDebugMsgLevel( "last modified = " + t.toString(), 4 );

  return t;
}

void QgsRasterLayer::setDataProvider( const QString &provider )
{
  setDataProvider( provider, QgsDataProvider::ProviderOptions() );
}

// typedef for the QgsDataProvider class factory
typedef QgsDataProvider *classFactoryFunction_t( const QString *, const QgsDataProvider::ProviderOptions &options );

//////////////////////////////////////////////////////////
//
// Non Static Public methods
//
/////////////////////////////////////////////////////////

int QgsRasterLayer::bandCount() const
{
  if ( !mDataProvider ) return 0;
  return mDataProvider->bandCount();
}

QString QgsRasterLayer::bandName( int bandNo ) const
{
  return dataProvider()->generateBandName( bandNo );
}

void QgsRasterLayer::setRendererForDrawingStyle( QgsRaster::DrawingStyle drawingStyle )
{
  setRenderer( QgsApplication::rasterRendererRegistry()->defaultRendererForDrawingStyle( drawingStyle, mDataProvider ) );
}

QgsRasterDataProvider *QgsRasterLayer::dataProvider()
{
  return mDataProvider;
}

const QgsRasterDataProvider *QgsRasterLayer::dataProvider() const
{
  return mDataProvider;
}

void QgsRasterLayer::reload()
{
  if ( mDataProvider )
  {
    mDataProvider->reloadData();
  }
}

QgsMapLayerRenderer *QgsRasterLayer::createMapRenderer( QgsRenderContext &rendererContext )
{
  return new QgsRasterLayerRenderer( this, rendererContext );
}


void QgsRasterLayer::draw( QPainter *theQPainter,
                           QgsRasterViewPort *rasterViewPort,
                           const QgsMapToPixel *qgsMapToPixel )
{
  QgsDebugMsgLevel( QStringLiteral( " 3 arguments" ), 4 );
  QTime time;
  time.start();
  //
  //
  // The goal here is to make as many decisions as possible early on (outside of the rendering loop)
  // so that we can maximise performance of the rendering process. So now we check which drawing
  // procedure to use :
  //

  QgsRasterProjector *projector = mPipe.projector();

  // TODO add a method to interface to get provider and get provider
  // params in QgsRasterProjector
  if ( projector )
  {
    projector->setCrs( rasterViewPort->mSrcCRS, rasterViewPort->mDestCRS, rasterViewPort->mSrcDatumTransform, rasterViewPort->mDestDatumTransform );
  }

  // Drawer to pipe?
  QgsRasterIterator iterator( mPipe.last() );
  QgsRasterDrawer drawer( &iterator );
  drawer.draw( theQPainter, rasterViewPort, qgsMapToPixel );

  QgsDebugMsgLevel( QStringLiteral( "total raster draw time (ms):     %1" ).arg( time.elapsed(), 5 ), 4 );
} //end of draw method

QgsLegendColorList QgsRasterLayer::legendSymbologyItems() const
{
  QList< QPair< QString, QColor > > symbolList;
  QgsRasterRenderer *renderer = mPipe.renderer();
  if ( renderer )
  {
    renderer->legendSymbologyItems( symbolList );
  }
  return symbolList;
}

QString QgsRasterLayer::htmlMetadata() const
{
  QgsLayerMetadataFormatter htmlFormatter( metadata() );
  QString myMetadata = QStringLiteral( "<html>\n<body>\n" );

  // Begin Provider section
  myMetadata += QStringLiteral( "<h1>" ) + tr( "Information from provider" ) + QStringLiteral( "</h1>\n<hr>\n" );
  myMetadata += QLatin1String( "<table class=\"list-view\">\n" );

  // name
  myMetadata += QStringLiteral( "<tr><td class=\"highlight\">" ) + tr( "Name" ) + QStringLiteral( "</td><td>" ) + name() + QStringLiteral( "</td></tr>\n" );

  // local path
  QVariantMap uriComponents = QgsProviderRegistry::instance()->decodeUri( mProviderKey, publicSource() );
  QString path;
  if ( uriComponents.contains( QStringLiteral( "path" ) ) )
  {
    path = uriComponents[QStringLiteral( "path" )].toString();
    if ( QFile::exists( path ) )
      myMetadata += QStringLiteral( "<tr><td class=\"highlight\">" ) + tr( "Path" ) + QStringLiteral( "</td><td>%1" ).arg( QStringLiteral( "<a href=\"%1\">%2</a>" ).arg( QUrl::fromLocalFile( path ).toString(), QDir::toNativeSeparators( path ) ) ) + QStringLiteral( "</td></tr>\n" );
  }
  if ( uriComponents.contains( QStringLiteral( "url" ) ) )
  {
    const QString url = uriComponents[QStringLiteral( "url" )].toString();
    myMetadata += QStringLiteral( "<tr><td class=\"highlight\">" ) + tr( "URL" ) + QStringLiteral( "</td><td>%1" ).arg( QStringLiteral( "<a href=\"%1\">%2</a>" ).arg( QUrl( url ).toString(), url ) ) + QStringLiteral( "</td></tr>\n" );
  }

  // data source
  if ( publicSource() != path )
    myMetadata += QStringLiteral( "<tr><td class=\"highlight\">" ) + tr( "Source" ) + QStringLiteral( "</td><td>%1" ).arg( publicSource() ) + QStringLiteral( "</td></tr>\n" );

  // data source
  if ( publicSource() != path )
    myMetadata += QStringLiteral( "<tr><td class=\"highlight\">" ) + tr( "Source" ) + QStringLiteral( "</td><td>%1" ).arg( publicSource() ) + QStringLiteral( "</td></tr>\n" );

  // EPSG
  myMetadata += QStringLiteral( "<tr><td class=\"highlight\">" ) + tr( "CRS" ) + QStringLiteral( "</td><td>" );
  if ( crs().isValid() )
  {
    myMetadata += crs().authid() + QStringLiteral( " - " );
    myMetadata += crs().description() + QStringLiteral( " - " );
    if ( crs().isGeographic() )
      myMetadata += tr( "Geographic" );
    else
      myMetadata += tr( "Projected" );
  }
  myMetadata += QLatin1String( "</td></tr>\n" );

  // Extent
  myMetadata += QStringLiteral( "<tr><td class=\"highlight\">" ) + tr( "Extent" ) + QStringLiteral( "</td><td>" ) + extent().toString() + QStringLiteral( "</td></tr>\n" );

  // unit
  myMetadata += QStringLiteral( "<tr><td class=\"highlight\">" ) + tr( "Unit" ) + QStringLiteral( "</td><td>" ) + QgsUnitTypes::toString( crs().mapUnits() ) + QStringLiteral( "</td></tr>\n" );

  // Raster Width
  myMetadata += QStringLiteral( "<tr><td class=\"highlight\">" ) + tr( "Width" ) + QStringLiteral( "</td><td>" );
  if ( dataProvider()->capabilities() & QgsRasterDataProvider::Size )
    myMetadata += QString::number( width() );
  else
    myMetadata += tr( "n/a" );
  myMetadata += QLatin1String( "</td></tr>\n" );

  // Raster height
  myMetadata += QStringLiteral( "<tr><td class=\"highlight\">" ) + tr( "Height" ) + QStringLiteral( "</td><td>" );
  if ( dataProvider()->capabilities() & QgsRasterDataProvider::Size )
    myMetadata += QString::number( height() );
  else
    myMetadata += tr( "n/a" );
  myMetadata += QLatin1String( "</td></tr>\n" );

  // Data type
  myMetadata += QStringLiteral( "<tr><td class=\"highlight\">" ) + tr( "Data type" ) + QStringLiteral( "</td><td>" );
  // Just use the first band
  switch ( mDataProvider->sourceDataType( 1 ) )
  {
    case Qgis::Byte:
      myMetadata += tr( "Byte - Eight bit unsigned integer" );
      break;
    case Qgis::UInt16:
      myMetadata += tr( "UInt16 - Sixteen bit unsigned integer " );
      break;
    case Qgis::Int16:
      myMetadata += tr( "Int16 - Sixteen bit signed integer " );
      break;
    case Qgis::UInt32:
      myMetadata += tr( "UInt32 - Thirty two bit unsigned integer " );
      break;
    case Qgis::Int32:
      myMetadata += tr( "Int32 - Thirty two bit signed integer " );
      break;
    case Qgis::Float32:
      myMetadata += tr( "Float32 - Thirty two bit floating point " );
      break;
    case Qgis::Float64:
      myMetadata += tr( "Float64 - Sixty four bit floating point " );
      break;
    case Qgis::CInt16:
      myMetadata += tr( "CInt16 - Complex Int16 " );
      break;
    case Qgis::CInt32:
      myMetadata += tr( "CInt32 - Complex Int32 " );
      break;
    case Qgis::CFloat32:
      myMetadata += tr( "CFloat32 - Complex Float32 " );
      break;
    case Qgis::CFloat64:
      myMetadata += tr( "CFloat64 - Complex Float64 " );
      break;
    default:
      myMetadata += tr( "Could not determine raster data type." );
  }
  myMetadata += QLatin1String( "</td></tr>\n" );

  // Insert provider-specific (e.g. WMS-specific) metadata
  myMetadata += mDataProvider->htmlMetadata();

  // End Provider section
  myMetadata += QLatin1String( "</table>\n<br><br>" );

  // Identification section
  myMetadata += QStringLiteral( "<h1>" ) + tr( "Identification" ) + QStringLiteral( "</h1>\n<hr>\n" );
  myMetadata += htmlFormatter.identificationSectionHtml();
  myMetadata += QLatin1String( "<br><br>\n" );

  // extent section
  myMetadata += QStringLiteral( "<h1>" ) + tr( "Extent" ) + QStringLiteral( "</h1>\n<hr>\n" );
  myMetadata += htmlFormatter.extentSectionHtml( );
  myMetadata += QLatin1String( "<br><br>\n" );

  // Start the Access section
  myMetadata += QStringLiteral( "<h1>" ) + tr( "Access" ) + QStringLiteral( "</h1>\n<hr>\n" );
  myMetadata += htmlFormatter.accessSectionHtml( );
  myMetadata += QLatin1String( "<br><br>\n" );

  // Bands section
  myMetadata += QStringLiteral( "</table>\n<br><br><h1>" ) + tr( "Bands" ) + QStringLiteral( "</h1>\n<hr>\n<table class=\"list-view\">\n" );

  // Band count
  myMetadata += QStringLiteral( "<tr><td class=\"highlight\">" ) + tr( "Band count" ) + QStringLiteral( "</td><td>" ) + QString::number( bandCount() ) + QStringLiteral( "</td></tr>\n" );

  // Band table
  myMetadata += QLatin1String( "</table>\n<br><table width=\"100%\" class=\"tabular-view\">\n" );
  myMetadata += QLatin1String( "<tr><th>" ) + tr( "Number" ) + QLatin1String( "</th><th>" ) + tr( "Band" ) + QLatin1String( "</th><th>" ) + tr( "No-Data" ) + QLatin1String( "</th><th>" ) + tr( "Min" ) + QLatin1String( "</th><th>" ) + tr( "Max" ) + QLatin1String( "</th></tr>\n" );

  QgsRasterDataProvider *provider = const_cast< QgsRasterDataProvider * >( mDataProvider );
  for ( int i = 1; i <= bandCount(); i++ )
  {
    QString rowClass;
    if ( i % 2 )
      rowClass = QStringLiteral( "class=\"odd-row\"" );
    myMetadata += QLatin1String( "<tr " ) + rowClass + QLatin1String( "><td>" ) + QString::number( i ) + QLatin1String( "</td><td>" ) + bandName( i ) + QLatin1String( "</td><td>" );

    if ( dataProvider()->sourceHasNoDataValue( i ) )
      myMetadata += QString::number( dataProvider()->sourceNoDataValue( i ) );
    else
      myMetadata += tr( "n/a" );
    myMetadata += QLatin1String( "</td>" );

    if ( provider->hasStatistics( i ) )
    {
      QgsRasterBandStats myRasterBandStats = provider->bandStatistics( i );
      myMetadata += QLatin1String( "<td>" ) + QString::number( myRasterBandStats.minimumValue, 'f', 10 ) + QLatin1String( "</td>" );
      myMetadata += QLatin1String( "<td>" ) + QString::number( myRasterBandStats.maximumValue, 'f', 10 ) + QLatin1String( "</td>" );
    }
    else
    {
      myMetadata += QLatin1String( "<td>" ) + tr( "n/a" ) + QLatin1String( "</td><td>" ) + tr( "n/a" ) + QLatin1String( "</td>" );
    }

    myMetadata += QLatin1String( "</tr>\n" );
  }

  //close previous bands table
  myMetadata += QLatin1String( "</table>\n<br><br>" );

  // Start the contacts section
  myMetadata += QStringLiteral( "<h1>" ) + tr( "Contacts" ) + QStringLiteral( "</h1>\n<hr>\n" );
  myMetadata += htmlFormatter.contactsSectionHtml( );
  myMetadata += QLatin1String( "<br><br>\n" );

  // Start the links section
  myMetadata += QStringLiteral( "<h1>" ) + tr( "References" ) + QStringLiteral( "</h1>\n<hr>\n" );
  myMetadata += htmlFormatter.linksSectionHtml( );
  myMetadata += QLatin1String( "<br><br>\n" );

  // Start the history section
  myMetadata += QStringLiteral( "<h1>" ) + tr( "History" ) + QStringLiteral( "</h1>\n<hr>\n" );
  myMetadata += htmlFormatter.historySectionHtml( );
  myMetadata += QLatin1String( "<br><br>\n" );

  myMetadata += QStringLiteral( "\n</body>\n</html>\n" );
  return myMetadata;
}

QPixmap QgsRasterLayer::paletteAsPixmap( int bandNumber )
{
  //TODO: This function should take dimensions
  QgsDebugMsgLevel( QStringLiteral( "entered." ), 4 );

  // Only do this for the GDAL provider?
  // Maybe WMS can do this differently using QImage::numColors and QImage::color()
  if ( mDataProvider->colorInterpretation( bandNumber ) == QgsRaster::PaletteIndex )
  {
    QgsDebugMsgLevel( QStringLiteral( "....found paletted image" ), 4 );
    QgsColorRampShader myShader;
    QList<QgsColorRampShader::ColorRampItem> myColorRampItemList = mDataProvider->colorTable( bandNumber );
    if ( !myColorRampItemList.isEmpty() )
    {
      QgsDebugMsgLevel( QStringLiteral( "....got color ramp item list" ), 4 );
      myShader.setColorRampItemList( myColorRampItemList );
      myShader.setColorRampType( QgsColorRampShader::Discrete );
      // Draw image
      int mySize = 100;
      QPixmap myPalettePixmap( mySize, mySize );
      QPainter myQPainter( &myPalettePixmap );

      QImage myQImage = QImage( mySize, mySize, QImage::Format_RGB32 );
      myQImage.fill( 0 );
      myPalettePixmap.fill();

      double myStep = ( static_cast< double >( myColorRampItemList.size() ) - 1 ) / static_cast< double >( mySize * mySize );
      double myValue = 0.0;
      for ( int myRow = 0; myRow < mySize; myRow++ )
      {
        QRgb *myLineBuffer = reinterpret_cast< QRgb * >( myQImage.scanLine( myRow ) );
        for ( int myCol = 0; myCol < mySize; myCol++ )
        {
          myValue = myStep * static_cast< double >( myCol + myRow * mySize );
          int c1, c2, c3, c4;
          myShader.shade( myValue, &c1, &c2, &c3, &c4 );
          myLineBuffer[ myCol ] = qRgba( c1, c2, c3, c4 );
        }
      }

      myQPainter.drawImage( 0, 0, myQImage );
      return myPalettePixmap;
    }
    QPixmap myNullPixmap;
    return myNullPixmap;
  }
  else
  {
    //invalid layer  was requested
    QPixmap myNullPixmap;
    return myNullPixmap;
  }
}

QString QgsRasterLayer::providerType() const
{
  return mProviderKey;
}

double QgsRasterLayer::rasterUnitsPerPixelX() const
{
// We return one raster pixel per map unit pixel
// One raster pixel can have several raster units...

// We can only use one of the mGeoTransform[], so go with the
// horisontal one.

  if ( mDataProvider->capabilities() & QgsRasterDataProvider::Size && !qgsDoubleNear( mDataProvider->xSize(), 0.0 ) )
  {
    return mDataProvider->extent().width() / mDataProvider->xSize();
  }
  return 1;
}

double QgsRasterLayer::rasterUnitsPerPixelY() const
{
  if ( mDataProvider->capabilities() & QgsRasterDataProvider::Size && !qgsDoubleNear( mDataProvider->ySize(), 0.0 ) )
  {
    return mDataProvider->extent().height() / mDataProvider->ySize();
  }
  return 1;
}

void QgsRasterLayer::init()
{
  mRasterType = QgsRasterLayer::GrayOrUndefined;

  setLegend( QgsMapLayerLegend::defaultRasterLegend( this ) );

  setRendererForDrawingStyle( QgsRaster::UndefinedDrawingStyle );

  //Initialize the last view port structure, should really be a class
  mLastViewPort.mWidth = 0;
  mLastViewPort.mHeight = 0;
}

void QgsRasterLayer::setDataProvider( QString const &provider, const QgsDataProvider::ProviderOptions &options )
{
  QgsDebugMsgLevel( QStringLiteral( "Entered" ), 4 );
  mValid = false; // assume the layer is invalid until we determine otherwise

  mPipe.remove( mDataProvider ); // deletes if exists
  mDataProvider = nullptr;

  // XXX should I check for and possibly delete any pre-existing providers?
  // XXX How often will that scenario occur?

  mProviderKey = provider;
  // set the layer name (uppercase first character)
  if ( ! mLayerName.isEmpty() )   // XXX shouldn't this happen in parent?
  {
    setName( mLayerName );
  }

  //mBandCount = 0;

  mDataProvider = dynamic_cast< QgsRasterDataProvider * >( QgsProviderRegistry::instance()->createProvider( mProviderKey, mDataSource, options ) );
  if ( !mDataProvider )
  {
    //QgsMessageLog::logMessage( tr( "Cannot instantiate the data provider" ), tr( "Raster" ) );
    appendError( ERR( tr( "Cannot instantiate the '%1' data provider" ).arg( mProviderKey ) ) );
    return;
  }
  QgsDebugMsgLevel( QStringLiteral( "Data provider created" ), 4 );
  mDataProvider->setParent( this );

  // Set data provider into pipe even if not valid so that it is deleted with pipe (with layer)
  mPipe.set( mDataProvider );
  if ( !mDataProvider->isValid() )
  {
    setError( mDataProvider->error() );
    appendError( ERR( tr( "Provider is not valid (provider: %1, URI: %2" ).arg( mProviderKey, mDataSource ) ) );
    return;
  }

  if ( mDataProvider->providerCapabilities() & QgsRasterDataProvider::ReadLayerMetadata )
  {
    setMetadata( mDataProvider->layerMetadata() );
    QgsDebugMsgLevel( QStringLiteral( "Set Data provider QgsLayerMetadata identifier[%1]" ).arg( metadata().identifier() ), 4 );
  }

  if ( provider == QLatin1String( "gdal" ) )
  {
    // make sure that the /vsigzip or /vsizip is added to uri, if applicable
    mDataSource = mDataProvider->dataSourceUri();
  }

  // get the extent
  QgsRectangle mbr = mDataProvider->extent();

  // show the extent
  QgsDebugMsgLevel( "Extent of layer: " + mbr.toString(), 4 );
  // store the extent
  setExtent( mbr );

  // upper case the first letter of the layer name
  QgsDebugMsgLevel( "mLayerName: " + name(), 4 );

  // set up the raster drawing style
  // Do not set any 'sensible' style here, the style is set later

  // Setup source CRS
  setCrs( QgsCoordinateReferenceSystem( mDataProvider->crs() ) );

  QgsDebugMsgLevel( "using wkt:\n" + crs().toWkt(), 4 );

  //defaults - Needs to be set after the Contrast list has been build
  //Try to read the default contrast enhancement from the config file

  //decide what type of layer this is...
  //TODO Change this to look at the color interp and palette interp to decide which type of layer it is
  QgsDebugMsgLevel( "bandCount = " + QString::number( mDataProvider->bandCount() ), 4 );
  QgsDebugMsgLevel( "dataType = " + QString::number( mDataProvider->dataType( 1 ) ), 4 );
  if ( ( mDataProvider->bandCount() > 1 ) )
  {
    // handle singleband gray with alpha
    if ( mDataProvider->bandCount() == 2
         && ( ( mDataProvider->colorInterpretation( 1 ) == QgsRaster::GrayIndex
                && mDataProvider->colorInterpretation( 2 ) == QgsRaster::AlphaBand )
              || ( mDataProvider->colorInterpretation( 1 ) == QgsRaster::AlphaBand
                   && mDataProvider->colorInterpretation( 2 ) == QgsRaster::GrayIndex ) ) )
    {
      mRasterType = GrayOrUndefined;
    }
    else
    {
      mRasterType = Multiband;
    }
  }
  else if ( mDataProvider->dataType( 1 ) == Qgis::ARGB32
            ||  mDataProvider->dataType( 1 ) == Qgis::ARGB32_Premultiplied )
  {
    mRasterType = ColorLayer;
  }
  else if ( mDataProvider->colorInterpretation( 1 ) == QgsRaster::PaletteIndex )
  {
    mRasterType = Palette;
  }
  else if ( mDataProvider->colorInterpretation( 1 ) == QgsRaster::ContinuousPalette )
  {
    mRasterType = Palette;
  }
  else
  {
    mRasterType = GrayOrUndefined;
  }

  QgsDebugMsgLevel( "mRasterType = " + QString::number( mRasterType ), 4 );
  if ( mRasterType == ColorLayer )
  {
    QgsDebugMsgLevel( "Setting drawing style to SingleBandColorDataStyle " + QString::number( QgsRaster::SingleBandColorDataStyle ), 4 );
    setRendererForDrawingStyle( QgsRaster::SingleBandColorDataStyle );
  }
  else if ( mRasterType == Palette && mDataProvider->colorInterpretation( 1 ) == QgsRaster::PaletteIndex )
  {
    setRendererForDrawingStyle( QgsRaster::PalettedColor ); //sensible default
  }
  else if ( mRasterType == Palette && mDataProvider->colorInterpretation( 1 ) == QgsRaster::ContinuousPalette )
  {
    setRendererForDrawingStyle( QgsRaster::SingleBandPseudoColor );
    // Load color table
    QList<QgsColorRampShader::ColorRampItem> colorTable = mDataProvider->colorTable( 1 );
    QgsSingleBandPseudoColorRenderer *r = dynamic_cast<QgsSingleBandPseudoColorRenderer *>( renderer() );
    if ( r )
    {
      // TODO: this should go somewhere else
      QgsRasterShader *shader = new QgsRasterShader();
      QgsColorRampShader *colorRampShader = new QgsColorRampShader();
      colorRampShader->setColorRampType( QgsColorRampShader::Interpolated );
      colorRampShader->setColorRampItemList( colorTable );
      shader->setRasterShaderFunction( colorRampShader );
      r->setShader( shader );
    }
  }
  else if ( mRasterType == Multiband )
  {
    setRendererForDrawingStyle( QgsRaster::MultiBandColor );  //sensible default
  }
  else                        //GrayOrUndefined
  {
    setRendererForDrawingStyle( QgsRaster::SingleBandGray );  //sensible default
  }

  // Auto set alpha band
  for ( int bandNo = 1; bandNo <= mDataProvider->bandCount(); bandNo++ )
  {
    if ( mDataProvider->colorInterpretation( bandNo ) == QgsRaster::AlphaBand )
    {
      if ( mPipe.renderer() )
      {
        mPipe.renderer()->setAlphaBand( bandNo );
      }
      break;
    }
  }

  // brightness filter
  QgsBrightnessContrastFilter *brightnessFilter = new QgsBrightnessContrastFilter();
  mPipe.set( brightnessFilter );

  // hue/saturation filter
  QgsHueSaturationFilter *hueSaturationFilter = new QgsHueSaturationFilter();
  mPipe.set( hueSaturationFilter );

  //resampler (must be after renderer)
  QgsRasterResampleFilter *resampleFilter = new QgsRasterResampleFilter();
  mPipe.set( resampleFilter );

  // projector (may be anywhere in pipe)
  QgsRasterProjector *projector = new QgsRasterProjector;
  mPipe.set( projector );

  // Set default identify format - use the richest format available
  int capabilities = mDataProvider->capabilities();
  QgsRaster::IdentifyFormat identifyFormat = QgsRaster::IdentifyFormatUndefined;
  if ( capabilities & QgsRasterInterface::IdentifyHtml )
  {
    // HTML is usually richest
    identifyFormat = QgsRaster::IdentifyFormatHtml;
  }
  else if ( capabilities & QgsRasterInterface::IdentifyFeature )
  {
    identifyFormat = QgsRaster::IdentifyFormatFeature;
  }
  else if ( capabilities & QgsRasterInterface::IdentifyText )
  {
    identifyFormat = QgsRaster::IdentifyFormatText;
  }
  else if ( capabilities & QgsRasterInterface::IdentifyValue )
  {
    identifyFormat = QgsRaster::IdentifyFormatValue;
  }
  setCustomProperty( QStringLiteral( "identify/format" ), QgsRasterDataProvider::identifyFormatName( identifyFormat ) );

  // Store timestamp
  // TODO move to provider
  mLastModified = lastModified( mDataSource );

  // Do a passthrough for the status bar text
  connect( mDataProvider, &QgsRasterDataProvider::statusChanged, this, &QgsRasterLayer::statusChanged );

  //mark the layer as valid
  mValid = true;

  QgsDebugMsgLevel( QStringLiteral( "exiting." ), 4 );
}

void QgsRasterLayer::setDataSource( const QString &dataSource, const QString &baseName, const QString &provider, const QgsDataProvider::ProviderOptions &options, bool loadDefaultStyleFlag )
{

  bool wasValid( isValid() );

  QDomImplementation domImplementation;
  QDomDocumentType documentType;
  QDomDocument doc;
  QString errorMsg;
  QDomElement rootNode;

  // Store the original style
  if ( wasValid && ! loadDefaultStyleFlag )
  {
    documentType = domImplementation.createDocumentType(
                     QStringLiteral( "qgis" ), QStringLiteral( "http://mrcc.com/qgis.dtd" ), QStringLiteral( "SYSTEM" ) );
    doc = QDomDocument( documentType );
    rootNode = doc.createElement( QStringLiteral( "qgis" ) );
    rootNode.setAttribute( QStringLiteral( "version" ), Qgis::QGIS_VERSION );
    doc.appendChild( rootNode );
    QgsReadWriteContext writeContext;
    if ( ! writeSymbology( rootNode, doc, errorMsg, writeContext ) )
    {
      QgsDebugMsg( QStringLiteral( "Could not store symbology for layer %1: %2" )
                   .arg( name() )
                   .arg( errorMsg ) );
    }
  }

  if ( mDataProvider )
    closeDataProvider();

  init();

  for ( int i = mPipe.size() - 1; i >= 0; --i )
  {
    mPipe.remove( i );
  }

  mDataSource = dataSource;
  mLayerName = baseName;

  setDataProvider( provider, options );

  if ( mValid )
  {
    // load default style
    bool defaultLoadedFlag = false;
    if ( loadDefaultStyleFlag )
    {
      loadDefaultStyle( defaultLoadedFlag );
    }
    else if ( wasValid && errorMsg.isEmpty() )  // Restore the style
    {
      QgsReadWriteContext readContext;
      if ( ! readSymbology( rootNode, errorMsg, readContext ) )
      {
        QgsDebugMsg( QStringLiteral( "Could not restore symbology for layer %1: %2" )
                     .arg( name() )
                     .arg( errorMsg ) );

      }
    }

    if ( !defaultLoadedFlag )
    {
      setDefaultContrastEnhancement();
    }
    emit statusChanged( tr( "QgsRasterLayer created" ) );
  }
  emit dataSourceChanged();
  emit dataChanged();
}

void QgsRasterLayer::closeDataProvider()
{
  mValid = false;
  mPipe.remove( mDataProvider );
  mDataProvider = nullptr;
}

void QgsRasterLayer::computeMinMax( int band,
                                    const QgsRasterMinMaxOrigin &mmo,
                                    QgsRasterMinMaxOrigin::Limits limits,
                                    const QgsRectangle &extent,
                                    int sampleSize,
                                    double &min, double &max )
{

  min = std::numeric_limits<double>::quiet_NaN();
  max = std::numeric_limits<double>::quiet_NaN();

  if ( limits == QgsRasterMinMaxOrigin::MinMax )
  {
    QgsRasterBandStats myRasterBandStats = mDataProvider->bandStatistics( band, QgsRasterBandStats::Min | QgsRasterBandStats::Max, extent, sampleSize );
    min = myRasterBandStats.minimumValue;
    max = myRasterBandStats.maximumValue;
  }
  else if ( limits == QgsRasterMinMaxOrigin::StdDev )
  {
    QgsRasterBandStats myRasterBandStats = mDataProvider->bandStatistics( band, QgsRasterBandStats::Mean | QgsRasterBandStats::StdDev, extent, sampleSize );
    min = myRasterBandStats.mean - ( mmo.stdDevFactor() * myRasterBandStats.stdDev );
    max = myRasterBandStats.mean + ( mmo.stdDevFactor() * myRasterBandStats.stdDev );
  }
  else if ( limits == QgsRasterMinMaxOrigin::CumulativeCut )
  {
    const double myLower = mmo.cumulativeCutLower();
    const double myUpper = mmo.cumulativeCutUpper();
    QgsDebugMsgLevel( QStringLiteral( "myLower = %1 myUpper = %2" ).arg( myLower ).arg( myUpper ), 4 );
    mDataProvider->cumulativeCut( band, myLower, myUpper, min, max, extent, sampleSize );
  }
  QgsDebugMsgLevel( QStringLiteral( "band = %1 min = %2 max = %3" ).arg( band ).arg( min ).arg( max ), 4 );

}


void QgsRasterLayer::setContrastEnhancement( QgsContrastEnhancement::ContrastEnhancementAlgorithm algorithm, QgsRasterMinMaxOrigin::Limits limits, const QgsRectangle &extent, int sampleSize, bool generateLookupTableFlag )
{
  setContrastEnhancement( algorithm,
                          limits,
                          extent,
                          sampleSize,
                          generateLookupTableFlag,
                          mPipe.renderer() );
}

void QgsRasterLayer::setContrastEnhancement( QgsContrastEnhancement::ContrastEnhancementAlgorithm algorithm,
    QgsRasterMinMaxOrigin::Limits limits,
    const QgsRectangle &extent,
    int sampleSize,
    bool generateLookupTableFlag,
    QgsRasterRenderer *rasterRenderer )
{
  QgsDebugMsgLevel( QStringLiteral( "theAlgorithm = %1 limits = %2 extent.isEmpty() = %3" ).arg( algorithm ).arg( limits ).arg( extent.isEmpty() ), 4 );
  if ( !rasterRenderer || !mDataProvider )
  {
    return;
  }

  QList<int> myBands;
  QList<QgsContrastEnhancement *> myEnhancements;
  QgsRasterMinMaxOrigin myMinMaxOrigin;
  QgsRasterRenderer *myRasterRenderer = nullptr;
  QgsSingleBandGrayRenderer *myGrayRenderer = nullptr;
  QgsSingleBandPseudoColorRenderer *myPseudoColorRenderer = nullptr;
  QgsMultiBandColorRenderer *myMultiBandRenderer = nullptr;
  QString rendererType  = rasterRenderer->type();
  if ( rendererType == QLatin1String( "singlebandgray" ) )
  {
    myGrayRenderer = dynamic_cast<QgsSingleBandGrayRenderer *>( rasterRenderer );
    if ( !myGrayRenderer )
    {
      return;
    }
    myBands << myGrayRenderer->grayBand();
    myRasterRenderer = myGrayRenderer;
    myMinMaxOrigin = myGrayRenderer->minMaxOrigin();
  }
  else if ( rendererType == QLatin1String( "multibandcolor" ) )
  {
    myMultiBandRenderer = dynamic_cast<QgsMultiBandColorRenderer *>( rasterRenderer );
    if ( !myMultiBandRenderer )
    {
      return;
    }
    myBands << myMultiBandRenderer->redBand() << myMultiBandRenderer->greenBand() << myMultiBandRenderer->blueBand();
    myRasterRenderer = myMultiBandRenderer;
    myMinMaxOrigin = myMultiBandRenderer->minMaxOrigin();
  }
  else if ( rendererType == QLatin1String( "singlebandpseudocolor" ) )
  {
    myPseudoColorRenderer = dynamic_cast<QgsSingleBandPseudoColorRenderer *>( rasterRenderer );
    if ( !myPseudoColorRenderer )
    {
      return;
    }
    myBands << myPseudoColorRenderer->band();
    myRasterRenderer = myPseudoColorRenderer;
    myMinMaxOrigin = myPseudoColorRenderer->minMaxOrigin();
  }
  else
  {
    return;
  }

  Q_FOREACH ( int myBand, myBands )
  {
    if ( myBand != -1 )
    {
      Qgis::DataType myType = static_cast< Qgis::DataType >( mDataProvider->dataType( myBand ) );
      std::unique_ptr<QgsContrastEnhancement> myEnhancement( new QgsContrastEnhancement( static_cast< Qgis::DataType >( myType ) ) );
      myEnhancement->setContrastEnhancementAlgorithm( algorithm, generateLookupTableFlag );

      double min;
      double max;
      computeMinMax( myBand, myMinMaxOrigin, limits, extent, sampleSize, min, max );

      if ( rendererType == QLatin1String( "singlebandpseudocolor" ) )
      {
        myPseudoColorRenderer->setClassificationMin( min );
        myPseudoColorRenderer->setClassificationMax( max );
        if ( myPseudoColorRenderer->shader() )
        {
          QgsColorRampShader *colorRampShader = dynamic_cast<QgsColorRampShader *>( myPseudoColorRenderer->shader()->rasterShaderFunction() );
          if ( colorRampShader )
          {
            colorRampShader->classifyColorRamp( myPseudoColorRenderer->band(), extent, myPseudoColorRenderer->input() );
          }
        }
      }
      else
      {
        myEnhancement->setMinimumValue( min );
        myEnhancement->setMaximumValue( max );
        myEnhancements.append( myEnhancement.release() );
      }
    }
    else
    {
      myEnhancements.append( nullptr );
    }
  }

  if ( rendererType == QLatin1String( "singlebandgray" ) )
  {
    if ( myEnhancements.first() ) myGrayRenderer->setContrastEnhancement( myEnhancements.takeFirst() );
  }
  else if ( rendererType == QLatin1String( "multibandcolor" ) )
  {
    if ( myEnhancements.first() ) myMultiBandRenderer->setRedContrastEnhancement( myEnhancements.takeFirst() );
    if ( myEnhancements.first() ) myMultiBandRenderer->setGreenContrastEnhancement( myEnhancements.takeFirst() );
    if ( myEnhancements.first() ) myMultiBandRenderer->setBlueContrastEnhancement( myEnhancements.takeFirst() );
  }

  //delete all remaining unused enhancements
  qDeleteAll( myEnhancements );

  myMinMaxOrigin.setLimits( limits );
  if ( extent != QgsRectangle() &&
       myMinMaxOrigin.extent() == QgsRasterMinMaxOrigin::WholeRaster )
  {
    myMinMaxOrigin.setExtent( QgsRasterMinMaxOrigin::CurrentCanvas );
  }
  if ( myRasterRenderer )
  {
    myRasterRenderer->setMinMaxOrigin( myMinMaxOrigin );
  }

  if ( rasterRenderer == renderer() )
  {
    emit repaintRequested();
    emit styleChanged();
    emit rendererChanged();
  }
}

void QgsRasterLayer::refreshContrastEnhancement( const QgsRectangle &extent )
{
  QgsSingleBandGrayRenderer *singleBandRenderer = nullptr;
  QgsMultiBandColorRenderer *multiBandRenderer = nullptr;
  const QgsContrastEnhancement *ce = nullptr;
  if ( ( singleBandRenderer = dynamic_cast<QgsSingleBandGrayRenderer *>( renderer() ) ) )
  {
    ce = singleBandRenderer->contrastEnhancement();
  }
  else if ( ( multiBandRenderer = dynamic_cast<QgsMultiBandColorRenderer *>( renderer() ) ) )
  {
    ce = multiBandRenderer->redContrastEnhancement();
  }

  if ( ce )
  {
    setContrastEnhancement( ce->contrastEnhancementAlgorithm() == QgsContrastEnhancement::NoEnhancement ?
                            QgsContrastEnhancement::StretchToMinimumMaximum : ce->contrastEnhancementAlgorithm(),
                            renderer()->minMaxOrigin().limits() == QgsRasterMinMaxOrigin::None ?
                            QgsRasterMinMaxOrigin::MinMax : renderer()->minMaxOrigin().limits(),
                            extent,
                            SAMPLE_SIZE,
                            true,
                            renderer() );
  }
  else
  {
    QgsContrastEnhancement::ContrastEnhancementAlgorithm myAlgorithm;
    QgsRasterMinMaxOrigin::Limits myLimits;
    if ( defaultContrastEnhancementSettings( myAlgorithm, myLimits ) )
    {
      setContrastEnhancement( QgsContrastEnhancement::StretchToMinimumMaximum,
                              myLimits,
                              extent,
                              SAMPLE_SIZE,
                              true,
                              renderer() );
    }
  }
}

void QgsRasterLayer::refreshRendererIfNeeded( QgsRasterRenderer *rasterRenderer,
    const QgsRectangle &extent )
{
  if ( !( mDataProvider &&
          mLastRectangleUsedByRefreshContrastEnhancementIfNeeded != extent &&
          rasterRenderer->minMaxOrigin().limits() != QgsRasterMinMaxOrigin::None &&
          rasterRenderer->minMaxOrigin().extent() == QgsRasterMinMaxOrigin::UpdatedCanvas ) )
    return;

  QgsSingleBandGrayRenderer *singleBandRenderer = nullptr;
  QgsMultiBandColorRenderer *multiBandRenderer = nullptr;
  QgsSingleBandPseudoColorRenderer *sbpcr = nullptr;
  const QgsContrastEnhancement *ce = nullptr;
  if ( ( singleBandRenderer = dynamic_cast<QgsSingleBandGrayRenderer *>( rasterRenderer ) ) )
  {
    ce = singleBandRenderer->contrastEnhancement();
  }
  else if ( ( multiBandRenderer = dynamic_cast<QgsMultiBandColorRenderer *>( rasterRenderer ) ) )
  {
    ce = multiBandRenderer->redContrastEnhancement();
  }
  else if ( ( sbpcr = dynamic_cast<QgsSingleBandPseudoColorRenderer *>( rasterRenderer ) ) )
  {
    mLastRectangleUsedByRefreshContrastEnhancementIfNeeded = extent;
    double min;
    double max;
    computeMinMax( sbpcr->band(),
                   rasterRenderer->minMaxOrigin(),
                   rasterRenderer->minMaxOrigin().limits(), extent,
                   SAMPLE_SIZE, min, max );
    sbpcr->setClassificationMin( min );
    sbpcr->setClassificationMax( max );

    if ( sbpcr->shader() )
    {
      QgsColorRampShader *colorRampShader = dynamic_cast<QgsColorRampShader *>( sbpcr->shader()->rasterShaderFunction() );
      if ( colorRampShader )
      {
        colorRampShader->classifyColorRamp( sbpcr->band(), extent, rasterRenderer->input() );
      }
    }

    QgsSingleBandPseudoColorRenderer *r = dynamic_cast<QgsSingleBandPseudoColorRenderer *>( renderer() );
    r->setClassificationMin( min );
    r->setClassificationMax( max );

    if ( r->shader() )
    {
      QgsColorRampShader *colorRampShader = dynamic_cast<QgsColorRampShader *>( r->shader()->rasterShaderFunction() );
      if ( colorRampShader )
      {
        colorRampShader->classifyColorRamp( sbpcr->band(), extent, rasterRenderer->input() );
      }
    }

    emit repaintRequested();
    emit styleChanged();
    emit rendererChanged();
    return;
  }

  if ( ce &&
       ce->contrastEnhancementAlgorithm() != QgsContrastEnhancement::NoEnhancement )
  {
    mLastRectangleUsedByRefreshContrastEnhancementIfNeeded = extent;

    setContrastEnhancement( ce->contrastEnhancementAlgorithm(),
                            rasterRenderer->minMaxOrigin().limits(),
                            extent,
                            SAMPLE_SIZE,
                            true,
                            rasterRenderer );

    // Update main renderer so that the legends get updated
    if ( singleBandRenderer )
      static_cast<QgsSingleBandGrayRenderer *>( renderer() )->setContrastEnhancement( new QgsContrastEnhancement( * singleBandRenderer->contrastEnhancement() ) );
    else if ( multiBandRenderer )
    {
      if ( multiBandRenderer->redContrastEnhancement() )
      {
        static_cast<QgsMultiBandColorRenderer *>( renderer() )->setRedContrastEnhancement( new QgsContrastEnhancement( *multiBandRenderer->redContrastEnhancement() ) );
      }
      if ( multiBandRenderer->greenContrastEnhancement() )
      {
        static_cast<QgsMultiBandColorRenderer *>( renderer() )->setGreenContrastEnhancement( new QgsContrastEnhancement( *multiBandRenderer->greenContrastEnhancement() ) );
      }
      if ( multiBandRenderer->blueContrastEnhancement() )
      {
        static_cast<QgsMultiBandColorRenderer *>( renderer() )->setBlueContrastEnhancement( new QgsContrastEnhancement( *multiBandRenderer->blueContrastEnhancement() ) );
      }
    }

    emit styleChanged();
    emit rendererChanged();
  }
}

bool QgsRasterLayer::defaultContrastEnhancementSettings(
  QgsContrastEnhancement::ContrastEnhancementAlgorithm &myAlgorithm,
  QgsRasterMinMaxOrigin::Limits &myLimits ) const
{
  QgsSettings mySettings;

  QString key;
  QString defaultAlg;
  QString defaultLimits;

  // TODO: we should not test renderer class here, move it somehow to renderers
  if ( dynamic_cast<QgsSingleBandGrayRenderer *>( renderer() ) )
  {
    key = QStringLiteral( "singleBand" );
    defaultAlg = QgsContrastEnhancement::contrastEnhancementAlgorithmString(
                   SINGLE_BAND_ENHANCEMENT_ALGORITHM );
    defaultLimits = QgsRasterMinMaxOrigin::limitsString(
                      SINGLE_BAND_MIN_MAX_LIMITS );
  }
  else if ( dynamic_cast<QgsMultiBandColorRenderer *>( renderer() ) )
  {
    if ( QgsRasterBlock::typeSize( dataProvider()->sourceDataType( 1 ) ) == 1 )
    {
      key = QStringLiteral( "multiBandSingleByte" );
      defaultAlg = QgsContrastEnhancement::contrastEnhancementAlgorithmString(
                     MULTIPLE_BAND_SINGLE_BYTE_ENHANCEMENT_ALGORITHM );
      defaultLimits = QgsRasterMinMaxOrigin::limitsString(
                        MULTIPLE_BAND_SINGLE_BYTE_MIN_MAX_LIMITS );
    }
    else
    {
      key = QStringLiteral( "multiBandMultiByte" );
      defaultAlg = QgsContrastEnhancement::contrastEnhancementAlgorithmString(
                     MULTIPLE_BAND_MULTI_BYTE_ENHANCEMENT_ALGORITHM );
      defaultLimits = QgsRasterMinMaxOrigin::limitsString(
                        MULTIPLE_BAND_MULTI_BYTE_MIN_MAX_LIMITS );
    }
  }

  if ( key.isEmpty() )
  {
    QgsDebugMsg( QStringLiteral( "No default contrast enhancement for this drawing style" ) );
    myAlgorithm = QgsContrastEnhancement::contrastEnhancementAlgorithmFromString( QString() );
    myLimits = QgsRasterMinMaxOrigin::limitsFromString( QString() );
    return false;
  }
  QgsDebugMsgLevel( "key = " + key, 4 );

  QString myAlgorithmString = mySettings.value( "/Raster/defaultContrastEnhancementAlgorithm/" + key, defaultAlg ).toString();
  QgsDebugMsgLevel( "myAlgorithmString = " + myAlgorithmString, 4 );

  myAlgorithm = QgsContrastEnhancement::contrastEnhancementAlgorithmFromString( myAlgorithmString );

  QString myLimitsString = mySettings.value( "/Raster/defaultContrastEnhancementLimits/" + key, defaultLimits ).toString();
  QgsDebugMsgLevel( "myLimitsString = " + myLimitsString, 4 );
  myLimits = QgsRasterMinMaxOrigin::limitsFromString( myLimitsString );

  return true;
}

void QgsRasterLayer::setDefaultContrastEnhancement()
{
  QgsDebugMsgLevel( QStringLiteral( "Entered" ), 4 );

  QgsContrastEnhancement::ContrastEnhancementAlgorithm myAlgorithm;
  QgsRasterMinMaxOrigin::Limits myLimits;
  defaultContrastEnhancementSettings( myAlgorithm, myLimits );

  setContrastEnhancement( myAlgorithm, myLimits );
}

void QgsRasterLayer::setLayerOrder( QStringList const &layers )
{
  QgsDebugMsgLevel( QStringLiteral( "entered." ), 4 );

  if ( mDataProvider )
  {
    QgsDebugMsgLevel( QStringLiteral( "About to mDataProvider->setLayerOrder(layers)." ), 4 );
    mDataProvider->setLayerOrder( layers );
  }

}

void QgsRasterLayer::setSubLayerVisibility( const QString &name, bool vis )
{

  if ( mDataProvider )
  {
    QgsDebugMsgLevel( QStringLiteral( "About to mDataProvider->setSubLayerVisibility(name, vis)." ), 4 );
    mDataProvider->setSubLayerVisibility( name, vis );
  }

}

QDateTime QgsRasterLayer::timestamp() const
{
  return mDataProvider->timestamp();
}


bool QgsRasterLayer::writeSld( QDomNode &node, QDomDocument &doc, QString &errorMessage, const QgsStringMap &props ) const
{
  Q_UNUSED( errorMessage );

  QgsStringMap localProps = QgsStringMap( props );
  if ( hasScaleBasedVisibility() )
  {
    // TODO: QgsSymbolLayerUtils::mergeScaleDependencies generate SE only and not SLD1.0
    QgsSymbolLayerUtils::mergeScaleDependencies( maximumScale(), minimumScale(), localProps );
  }

  if ( isSpatial() ) // TODO: does it make sense this control?
  {
    // store constraints
    QDomElement constraintElem = doc.createElement( QStringLiteral( "sld:LayerFeatureConstraints" ) );
    node.appendChild( constraintElem );

    QDomElement featureTypeConstraintElem = doc.createElement( QStringLiteral( "sld:FeatureTypeConstraint" ) );
    constraintElem.appendChild( featureTypeConstraintElem );

    QDomElement userStyleElem = doc.createElement( QStringLiteral( "sld:UserStyle" ) );
    node.appendChild( userStyleElem );

    if ( !name().isEmpty() )
    {
      QDomElement nameElem = doc.createElement( QStringLiteral( "sld:Name" ) );
      nameElem.appendChild( doc.createTextNode( name() ) );
      userStyleElem.appendChild( nameElem );
    }

    if ( !abstract().isEmpty() )
    {
      QDomElement abstractElem = doc.createElement( QStringLiteral( "sld:Abstract" ) );
      abstractElem.appendChild( doc.createTextNode( abstract() ) );
      userStyleElem.appendChild( abstractElem );
    }

    if ( !title().isEmpty() )
    {
      QDomElement titleElem = doc.createElement( QStringLiteral( "sld:Title" ) );
      titleElem.appendChild( doc.createTextNode( title() ) );
      userStyleElem.appendChild( titleElem );
    }

    QDomElement featureTypeStyleElem = doc.createElement( QStringLiteral( "sld:FeatureTypeStyle" ) );
    userStyleElem.appendChild( featureTypeStyleElem );

#if 0
    // TODO: Is there a way to fill it's value with the named style?
    // by default <sld:Name> under <sld:FeatureTypeStyle> can have 0 occurrences
    // the same happen for tags:
    // sld:Title
    // sld:Abstract
    // sld:FeatureTypeName
    // sld:SemanticTypeIdentifier
    QDomElement typeStyleNameElem = doc.createElement( QStringLiteral( "sld:Name" ) );
    featureTypeStyleElem.appendChild( typeStyleNameElem );
#endif

    QDomElement typeStyleRuleElem = doc.createElement( QStringLiteral( "sld:Rule" ) );
    featureTypeStyleElem.appendChild( typeStyleRuleElem );

    // add ScaleDenominator tags
    if ( hasScaleBasedVisibility() )
    {
      // note that denominator is the inverted value of scale
      if ( maximumScale() != 0.0 )
      {
        QDomElement minScaleElem = doc.createElement( QStringLiteral( "sld:MinScaleDenominator" ) );
        minScaleElem.appendChild( doc.createTextNode( QString::number( maximumScale() ) ) );
        typeStyleRuleElem.appendChild( minScaleElem );
      }

      QDomElement maxScaleElem = doc.createElement( QStringLiteral( "sld:MaxScaleDenominator" ) );
      maxScaleElem.appendChild( doc.createTextNode( QString::number( minimumScale() ) ) );
      typeStyleRuleElem.appendChild( maxScaleElem );
    }

    // export renderer dependent tags
    mPipe.renderer()->toSld( doc, typeStyleRuleElem, localProps );

    // inject raster layer parameters in RasterSymbolizer tag because
    // they belongs to rasterlayer and not to the renderer => avoid to
    // pass many parameters value via localProps
    QDomNodeList elements = typeStyleRuleElem.elementsByTagName( QStringLiteral( "sld:RasterSymbolizer" ) );
    if ( elements.size() != 0 )
    {
      // there SHOULD be only one
      QDomElement rasterSymbolizerElem = elements.at( 0 ).toElement();

      // lamda helper used below to reduce code redundancy
      auto vendorOptionWriter = [&]( QString name, QString value )
      {
        QDomElement vendorOptionElem = doc.createElement( QStringLiteral( "sld:VendorOption" ) );
        vendorOptionElem.setAttribute( QStringLiteral( "name" ), name );
        vendorOptionElem.appendChild( doc.createTextNode( value ) );
        rasterSymbolizerElem.appendChild( vendorOptionElem );
      };

      // add greyScale rendering mode if set
      if ( hueSaturationFilter()->grayscaleMode() != QgsHueSaturationFilter::GrayscaleOff )
      {
        QString property;
        switch ( hueSaturationFilter()->grayscaleMode() )
        {
          case QgsHueSaturationFilter::GrayscaleLightness:
            property = QStringLiteral( "lightness" );
            break;
          case QgsHueSaturationFilter::GrayscaleLuminosity:
            property = QStringLiteral( "luminosity" );
            break;
          case QgsHueSaturationFilter::GrayscaleAverage:
            property = QStringLiteral( "average" );
            break;
          case QgsHueSaturationFilter::GrayscaleOff:
            // added just to avoid travis fail
            break;
        }
        if ( !property.isEmpty() )
          vendorOptionWriter( QStringLiteral( "grayScale" ), property );
      }

      // add Hue, Saturation and Lighting values in props is Hue filter is set
      if ( hueSaturationFilter() && hueSaturationFilter()->colorizeOn() )
      {
        vendorOptionWriter( QStringLiteral( "colorizeOn" ), QString::number( hueSaturationFilter()->colorizeOn() ) );
        vendorOptionWriter( QStringLiteral( "colorizeRed" ), QString::number( hueSaturationFilter()->colorizeColor().red() ) );
        vendorOptionWriter( QStringLiteral( "colorizeGreen" ), QString::number( hueSaturationFilter()->colorizeColor().green() ) );
        vendorOptionWriter( QStringLiteral( "colorizeBlue" ), QString::number( hueSaturationFilter()->colorizeColor().blue() ) );
        if ( hueSaturationFilter()->colorizeStrength() != 100.0 )
          vendorOptionWriter( QStringLiteral( "colorizeStrength" ), QString::number( hueSaturationFilter()->colorizeStrength() / 100.0 ) );
        vendorOptionWriter( QStringLiteral( "saturation" ), QString::number( hueSaturationFilter()->colorizeColor().saturationF() ) );
      }
      else
      {
        // saturation != 0 (default value)
        if ( hueSaturationFilter()->saturation() != 0 )
        {
          // normlize value [-100:100] -> [0:1]
          int s = hueSaturationFilter()->saturation();
          double sF = ( s - ( -100.0 ) ) / ( 100.0 - ( -100.0 ) );
          vendorOptionWriter( QStringLiteral( "saturation" ), QString::number( sF ) );
        }
      }

      // brightness != 0 (default value)
      if ( brightnessFilter()->brightness() != 0 )
      {
        // normalize value [-255:255] -> [0:1]
        int b = brightnessFilter()->brightness();
        double bF = ( b - ( -255.0 ) ) / ( 255.0 - ( -255.0 ) );
        vendorOptionWriter( QStringLiteral( "brightness" ), QString::number( bF ) );
      }

      // contrast != 0 (default value)
      if ( brightnessFilter()->contrast() != 0 )
      {
        // normlize value [-100:100] -> [0:1]
        int c = brightnessFilter()->contrast();
        double cF = ( c - ( -100.0 ) ) / ( 100.0 - ( -100.0 ) );
        vendorOptionWriter( QStringLiteral( "contrast" ), QString::number( cF ) );
      }

#if 0
      // TODO: check if the below mapping formula make sense to map QGIS contrast with SLD gamma value
      //
      // add SLD1.0 ContrastEnhancement GammaValue = QGIS Contrast
      // SLD1.0 does only define 1 as neutral/center double value but does not define range.
      // because https://en.wikipedia.org/wiki/Gamma_correction assumed gamma is >0.
      // whilst QGIS has a -100/100 values centered in 0 => QGIS contrast value will be scaled in the
      // following way:
      // [-100,0] => [0,1] and [0,100] => [1,100]
      // an alternative could be scale [-100,100] => (0,2]
      //
      if ( newProps.contains( QStringLiteral( "contrast" ) ) )
      {
        double gamma;
        double contrast = newProps[ QStringLiteral( "contrast" ) ].toDouble();
        double percentage = ( contrast - ( -100.0 ) ) / ( 100.0 - ( -100.0 ) );
        if ( percentage <= 0.5 )
        {
          // stretch % to [0-1]
          gamma = percentage / 0.5;
        }
        else
        {
          gamma = contrast;
        }

        QDomElement globalContrastEnhancementElem = doc.createElement( QStringLiteral( "sld:ContrastEnhancement" ) );
        rasterSymolizerElem.appendChild( globalContrastEnhancementElem );

        QDomElement gammaValueElem = doc.createElement( QStringLiteral( "sld:GammaValue" ) );
        gammaValueElem.appendChild( doc.createTextNode( QString::number( gamma ) ) );
        globalContrastEnhancementElem.appendChild( gammaValueElem );
      }
#endif
    }
  }
  return true;
}


void QgsRasterLayer::setRenderer( QgsRasterRenderer *renderer )
{
  QgsDebugMsgLevel( QStringLiteral( "Entered" ), 4 );
  if ( !renderer )
  {
    return;
  }

  mPipe.set( renderer );
  emit rendererChanged();
  emit styleChanged();
}

void QgsRasterLayer::showStatusMessage( QString const &message )
{
  // QgsDebugMsg(QString("entered with '%1'.").arg(theMessage));

  // Pass-through
  // TODO: See if we can connect signal-to-signal.  This is a kludge according to the Qt doc.
  emit statusChanged( message );
}

QStringList QgsRasterLayer::subLayers() const
{
  return mDataProvider->subLayers();
}

// this function should be used when rendering with the MTR engine introduced in 2.3, as QPixmap is not thread safe (see bug #9626)
// note: previewAsImage and previewAsPixmap should use a common low-level fct QgsRasterLayer::previewOnPaintDevice( QSize size, QColor bgColor, QPaintDevice &device )
QImage QgsRasterLayer::previewAsImage( QSize size, const QColor &bgColor, QImage::Format format )
{
  QImage myQImage( size, format );

  if ( ! isValid( ) )
    return  QImage();

  myQImage.setColor( 0, bgColor.rgba() );
  myQImage.fill( 0 );  //defaults to white, set to transparent for rendering on a map


  QgsRasterViewPort *myRasterViewPort = new QgsRasterViewPort();

  double myMapUnitsPerPixel;
  double myX = 0.0;
  double myY = 0.0;
  QgsRectangle myExtent = mDataProvider->extent();
  if ( myExtent.width() / myExtent.height() >= static_cast< double >( myQImage.width() ) / myQImage.height() )
  {
    myMapUnitsPerPixel = myExtent.width() / myQImage.width();
    myY = ( myQImage.height() - myExtent.height() / myMapUnitsPerPixel ) / 2;
  }
  else
  {
    myMapUnitsPerPixel = myExtent.height() / myQImage.height();
    myX = ( myQImage.width() - myExtent.width() / myMapUnitsPerPixel ) / 2;
  }

  double myPixelWidth = myExtent.width() / myMapUnitsPerPixel;
  double myPixelHeight = myExtent.height() / myMapUnitsPerPixel;

  myRasterViewPort->mTopLeftPoint = QgsPointXY( myX, myY );
  myRasterViewPort->mBottomRightPoint = QgsPointXY( myPixelWidth, myPixelHeight );
  myRasterViewPort->mWidth = myQImage.width();
  myRasterViewPort->mHeight = myQImage.height();

  myRasterViewPort->mDrawnExtent = myExtent;
  myRasterViewPort->mSrcCRS = QgsCoordinateReferenceSystem(); // will be invalid
  myRasterViewPort->mDestCRS = QgsCoordinateReferenceSystem(); // will be invalid
  myRasterViewPort->mSrcDatumTransform = -1;
  myRasterViewPort->mDestDatumTransform = -1;

  QgsMapToPixel *myMapToPixel = new QgsMapToPixel( myMapUnitsPerPixel );

  QPainter *myQPainter = new QPainter( &myQImage );
  draw( myQPainter, myRasterViewPort, myMapToPixel );
  delete myRasterViewPort;
  delete myMapToPixel;
  myQPainter->end();
  delete myQPainter;

  return myQImage;
}

//////////////////////////////////////////////////////////
//
// Protected methods
//
/////////////////////////////////////////////////////////
/*
 * \param QDomNode node that will contain the symbology definition for this layer.
 * \param errorMessage reference to string that will be updated with any error messages
 * \return true in case of success.
 */
bool QgsRasterLayer::readSymbology( const QDomNode &layer_node, QString &errorMessage,
                                    QgsReadWriteContext &context, QgsMapLayer::StyleCategories categories )
{
  Q_UNUSED( errorMessage );
  // TODO: implement categories for raster layer

  QDomElement rasterRendererElem;

  QDomElement layerElement = layer_node.toElement();
  readCommonStyle( layerElement, context, categories );

  // pipe element was introduced in the end of 1.9 development when there were
  // already many project files in use so we support 1.9 backward compatibility
  // even it was never officially released -> use pipe element if present, otherwise
  // use layer node
  QDomNode pipeNode = layer_node.firstChildElement( QStringLiteral( "pipe" ) );
  if ( pipeNode.isNull() ) // old project
  {
    pipeNode = layer_node;
  }

  //rasterlayerproperties element there -> old format (1.8 and early 1.9)
  if ( !layer_node.firstChildElement( QStringLiteral( "rasterproperties" ) ).isNull() )
  {
    //copy node because layer_node is const
    QDomNode layerNodeCopy = layer_node.cloneNode();
    QDomDocument doc = layerNodeCopy.ownerDocument();
    QDomElement rasterPropertiesElem = layerNodeCopy.firstChildElement( QStringLiteral( "rasterproperties" ) );
    QgsProjectFileTransform::convertRasterProperties( doc, layerNodeCopy, rasterPropertiesElem,
        this );
    rasterRendererElem = layerNodeCopy.firstChildElement( QStringLiteral( "rasterrenderer" ) );
    QgsDebugMsgLevel( doc.toString(), 4 );
  }
  else
  {
    rasterRendererElem = pipeNode.firstChildElement( QStringLiteral( "rasterrenderer" ) );
  }

  if ( !rasterRendererElem.isNull() )
  {
    QString rendererType = rasterRendererElem.attribute( QStringLiteral( "type" ) );
    QgsRasterRendererRegistryEntry rendererEntry;
    if ( QgsApplication::rasterRendererRegistry()->rendererData( rendererType, rendererEntry ) )
    {
      QgsRasterRenderer *renderer = rendererEntry.rendererCreateFunction( rasterRendererElem, dataProvider() );
      mPipe.set( renderer );
    }
  }

  //brightness
  QgsBrightnessContrastFilter *brightnessFilter = new QgsBrightnessContrastFilter();
  mPipe.set( brightnessFilter );

  //brightness coefficient
  QDomElement brightnessElem = pipeNode.firstChildElement( QStringLiteral( "brightnesscontrast" ) );
  if ( !brightnessElem.isNull() )
  {
    brightnessFilter->readXml( brightnessElem );
  }

  //hue/saturation
  QgsHueSaturationFilter *hueSaturationFilter = new QgsHueSaturationFilter();
  mPipe.set( hueSaturationFilter );

  //saturation coefficient
  QDomElement hueSaturationElem = pipeNode.firstChildElement( QStringLiteral( "huesaturation" ) );
  if ( !hueSaturationElem.isNull() )
  {
    hueSaturationFilter->readXml( hueSaturationElem );
  }

  //resampler
  QgsRasterResampleFilter *resampleFilter = new QgsRasterResampleFilter();
  mPipe.set( resampleFilter );

  //max oversampling
  QDomElement resampleElem = pipeNode.firstChildElement( QStringLiteral( "rasterresampler" ) );
  if ( !resampleElem.isNull() )
  {
    resampleFilter->readXml( resampleElem );
  }

  // get and set the blend mode if it exists
  QDomNode blendModeNode = layer_node.namedItem( QStringLiteral( "blendMode" ) );
  if ( !blendModeNode.isNull() )
  {
    QDomElement e = blendModeNode.toElement();
    setBlendMode( QgsPainting::getCompositionMode( static_cast< QgsPainting::BlendMode >( e.text().toInt() ) ) );
  }

  readCustomProperties( layer_node );

  return true;
}

bool QgsRasterLayer::readStyle( const QDomNode &node, QString &errorMessage, QgsReadWriteContext &context, QgsMapLayer::StyleCategories categories )
{
  return readSymbology( node, errorMessage, context, categories );
}

bool QgsRasterLayer::readXml( const QDomNode &layer_node, QgsReadWriteContext &context )
{
  QgsDebugMsgLevel( QStringLiteral( "Entered" ), 4 );
  // Make sure to read the file first so stats etc are initialized properly!

  //process provider key
  QDomNode pkeyNode = layer_node.namedItem( QStringLiteral( "provider" ) );

  if ( pkeyNode.isNull() )
  {
    mProviderKey = QStringLiteral( "gdal" );
  }
  else
  {
    QDomElement pkeyElt = pkeyNode.toElement();
    mProviderKey = pkeyElt.text();
    if ( mProviderKey.isEmpty() )
    {
      mProviderKey = QStringLiteral( "gdal" );
    }
  }

  // Open the raster source based on provider and datasource

  // Go down the raster-data-provider paradigm

  // Collect provider-specific information

  QDomNode rpNode = layer_node.namedItem( QStringLiteral( "rasterproperties" ) );

  if ( mProviderKey == QLatin1String( "wms" ) )
  {
    // >>> BACKWARD COMPATIBILITY < 1.9
    // The old WMS URI format does not contain all the information, we add them here.
    if ( !mDataSource.contains( QLatin1String( "crs=" ) ) && !mDataSource.contains( QLatin1String( "format=" ) ) )
    {
      QgsDebugMsgLevel( QStringLiteral( "Old WMS URI format detected -> adding params" ), 4 );
      QgsDataSourceUri uri;
      uri.setEncodedUri( mDataSource );
      QDomElement layerElement = rpNode.firstChildElement( QStringLiteral( "wmsSublayer" ) );
      while ( !layerElement.isNull() )
      {
        // TODO: sublayer visibility - post-0.8 release timeframe

        // collect name for the sublayer
        uri.setParam( QStringLiteral( "layers" ),  layerElement.namedItem( QStringLiteral( "name" ) ).toElement().text() );

        // collect style for the sublayer
        uri.setParam( QStringLiteral( "styles" ), layerElement.namedItem( QStringLiteral( "style" ) ).toElement().text() );

        layerElement = layerElement.nextSiblingElement( QStringLiteral( "wmsSublayer" ) );
      }

      // Collect format
      QDomNode formatNode = rpNode.namedItem( QStringLiteral( "wmsFormat" ) );
      uri.setParam( QStringLiteral( "format" ), rpNode.namedItem( QStringLiteral( "wmsFormat" ) ).toElement().text() );

      // WMS CRS URL param should not be mixed with that assigned to the layer.
      // In the old WMS URI version there was no CRS and layer crs().authid() was used.
      uri.setParam( QStringLiteral( "crs" ), crs().authid() );
      mDataSource = uri.encodedUri();
    }
    // <<< BACKWARD COMPATIBILITY < 1.9
  }

  QgsDataProvider::ProviderOptions providerOptions;
  setDataProvider( mProviderKey, providerOptions );

  if ( ! mDataProvider )
  {
    QgsDebugMsg( QStringLiteral( "Raster data provider could not be created for %1" ).arg( mDataSource ) );
    return false;
  }

  QString error;
  bool res = readSymbology( layer_node, error, context );

  // old wms settings we need to correct
  if ( res && mProviderKey == QLatin1String( "wms" ) && ( !renderer() || renderer()->type() != QLatin1String( "singlebandcolordata" ) ) )
  {
    setRendererForDrawingStyle( QgsRaster::SingleBandColorDataStyle );
  }

  // Check timestamp
  // This was probably introduced to reload completely raster if data changed and
  // reset completely symbology to reflect new data type etc. It creates however
  // problems, because user defined symbology is complete lost if data file time
  // changed (the content may be the same). See also 6900.
#if 0
  QDomNode stampNode = layer_node.namedItem( "timestamp" );
  if ( !stampNode.isNull() )
  {
    QDateTime stamp = QDateTime::fromString( stampNode.toElement().text(), Qt::ISODate );
    // TODO: very bad, we have to load twice!!! Make QgsDataProvider::timestamp() static?
    if ( stamp < mDataProvider->dataTimestamp() )
    {
      QgsDebugMsg( QStringLiteral( "data changed, reload provider" ) );
      closeDataProvider();
      init();
      setDataProvider( mProviderKey );
      if ( !mValid ) return false;
    }
  }
#endif

  // Load user no data value
  QDomElement noDataElement = layer_node.firstChildElement( QStringLiteral( "noData" ) );

  QDomNodeList noDataBandList = noDataElement.elementsByTagName( QStringLiteral( "noDataList" ) );

  for ( int i = 0; i < noDataBandList.size(); ++i )
  {
    QDomElement bandElement = noDataBandList.at( i ).toElement();
    bool ok;
    int bandNo = bandElement.attribute( QStringLiteral( "bandNo" ) ).toInt( &ok );
    QgsDebugMsgLevel( QStringLiteral( "bandNo = %1" ).arg( bandNo ), 4 );
    if ( ok && ( bandNo > 0 ) && ( bandNo <= mDataProvider->bandCount() ) )
    {
      mDataProvider->setUseSourceNoDataValue( bandNo, bandElement.attribute( QStringLiteral( "useSrcNoData" ) ).toInt() );
      QgsRasterRangeList myNoDataRangeList;

      QDomNodeList rangeList = bandElement.elementsByTagName( QStringLiteral( "noDataRange" ) );

      myNoDataRangeList.reserve( rangeList.size() );
      for ( int j = 0; j < rangeList.size(); ++j )
      {
        QDomElement rangeElement = rangeList.at( j ).toElement();
        QgsRasterRange myNoDataRange( rangeElement.attribute( QStringLiteral( "min" ) ).toDouble(),
                                      rangeElement.attribute( QStringLiteral( "max" ) ).toDouble() );
        QgsDebugMsgLevel( QStringLiteral( "min = %1 %2" ).arg( rangeElement.attribute( "min" ) ).arg( myNoDataRange.min() ), 4 );
        myNoDataRangeList << myNoDataRange;
      }
      mDataProvider->setUserNoDataValue( bandNo, myNoDataRangeList );
    }
  }

  readStyleManager( layer_node );

  return res;
} // QgsRasterLayer::readXml( QDomNode & layer_node )

/*
 * \param QDomNode the node that will have the style element added to it.
 * \param QDomDocument the document that will have the QDomNode added.
 * \param errorMessage reference to string that will be updated with any error messages
 * \return true in case of success.
 */
bool QgsRasterLayer::writeSymbology( QDomNode &layer_node, QDomDocument &document, QString &errorMessage,
                                     const QgsReadWriteContext &context, QgsMapLayer::StyleCategories categories ) const
{
  Q_UNUSED( errorMessage );
  // TODO: implement categories for raster layer

  QDomElement layerElement = layer_node.toElement();
  writeCommonStyle( layerElement, document, context, categories );

  // Store pipe members (except provider) into pipe element, in future, it will be
  // possible to add custom filters into the pipe
  QDomElement pipeElement  = document.createElement( QStringLiteral( "pipe" ) );

  for ( int i = 1; i < mPipe.size(); i++ )
  {
    QgsRasterInterface *interface = mPipe.at( i );
    if ( !interface ) continue;
    interface->writeXml( document, pipeElement );
  }

  layer_node.appendChild( pipeElement );

  // add blend mode node
  QDomElement blendModeElement  = document.createElement( QStringLiteral( "blendMode" ) );
  QDomText blendModeText = document.createTextNode( QString::number( QgsPainting::getBlendModeEnum( blendMode() ) ) );
  blendModeElement.appendChild( blendModeText );
  layer_node.appendChild( blendModeElement );

  return true;
}

bool QgsRasterLayer::writeStyle( QDomNode &node, QDomDocument &doc, QString &errorMessage,
                                 const QgsReadWriteContext &context, QgsMapLayer::StyleCategories categories ) const
{
  return writeSymbology( node, doc, errorMessage, context, categories );
} // bool QgsRasterLayer::writeSymbology

/*
 *  virtual
 *  \note Called by QgsMapLayer::writeXml().
 */
bool QgsRasterLayer::writeXml( QDomNode &layer_node,
                               QDomDocument &document,
                               const QgsReadWriteContext &context ) const
{
  // first get the layer element so that we can append the type attribute

  QDomElement mapLayerNode = layer_node.toElement();

  if ( mapLayerNode.isNull() || "maplayer" != mapLayerNode.nodeName() )
  {
    QgsMessageLog::logMessage( tr( "<maplayer> not found." ), tr( "Raster" ) );
    return false;
  }

  mapLayerNode.setAttribute( QStringLiteral( "type" ), QStringLiteral( "raster" ) );

  // add provider node

  QDomElement provider  = document.createElement( QStringLiteral( "provider" ) );
  QDomText providerText = document.createTextNode( mProviderKey );
  provider.appendChild( providerText );
  layer_node.appendChild( provider );

  // User no data
  QDomElement noData  = document.createElement( QStringLiteral( "noData" ) );

  for ( int bandNo = 1; bandNo <= mDataProvider->bandCount(); bandNo++ )
  {
    QDomElement noDataRangeList = document.createElement( QStringLiteral( "noDataList" ) );
    noDataRangeList.setAttribute( QStringLiteral( "bandNo" ), bandNo );
    noDataRangeList.setAttribute( QStringLiteral( "useSrcNoData" ), mDataProvider->useSourceNoDataValue( bandNo ) );

    Q_FOREACH ( QgsRasterRange range, mDataProvider->userNoDataValues( bandNo ) )
    {
      QDomElement noDataRange = document.createElement( QStringLiteral( "noDataRange" ) );

      noDataRange.setAttribute( QStringLiteral( "min" ), QgsRasterBlock::printValue( range.min() ) );
      noDataRange.setAttribute( QStringLiteral( "max" ), QgsRasterBlock::printValue( range.max() ) );
      noDataRangeList.appendChild( noDataRange );
    }

    noData.appendChild( noDataRangeList );

  }
  if ( noData.hasChildNodes() )
  {
    layer_node.appendChild( noData );
  }

  writeStyleManager( layer_node, document );

  //write out the symbology
  QString errorMsg;
  return writeSymbology( layer_node, document, errorMsg, context );
}

// TODO: this should ideally go to gdal provider (together with most of encodedSource() + decodedSource())
static bool _parseGpkgColons( const QString &src, QString &filename, QString &tablename )
{
  // GDAL accepts the following input format:  GPKG:filename:table
  // (GDAL won't accept quoted filename)

  QStringList lst = src.split( ':' );
  if ( lst.count() != 3 && lst.count() != 4 )
    return false;

  tablename = lst.last();
  if ( lst.count() == 3 )
  {
    filename = lst[1];
    return true;
  }
  else if ( lst.count() == 4 && lst[1].count() == 1 && ( lst[2][0] == '/' || lst[2][0] == '\\' ) )
  {
    // a bit of handling to make sure that filename C:\hello.gpkg is parsed correctly
    filename = lst[1] + ":" + lst[2];
    return true;
  }
  return false;
}


QString QgsRasterLayer::encodedSource( const QString &source, const QgsReadWriteContext &context ) const
{
  QString src( source );
  bool handled = false;

  // Update path for subdataset
  if ( providerType() == QLatin1String( "gdal" ) )
  {
    if ( src.startsWith( QLatin1String( "NETCDF:" ) ) )
    {
      // NETCDF:filename:variable
      // filename can be quoted with " as it can contain colons
      QRegExp r( "NETCDF:(.+):([^:]+)" );
      if ( r.exactMatch( src ) )
      {
        QString filename = r.cap( 1 );
        if ( filename.startsWith( '"' ) && filename.endsWith( '"' ) )
          filename = filename.mid( 1, filename.length() - 2 );
        src = "NETCDF:\"" + context.pathResolver().writePath( filename ) + "\":" + r.cap( 2 );
        handled = true;
      }
    }
    else if ( src.startsWith( QLatin1String( "GPKG:" ) ) )
    {
      // GPKG:filename:table
      QString filename, tablename;
      if ( _parseGpkgColons( src, filename, tablename ) )
      {
        filename = context.pathResolver().writePath( filename );
        src = QStringLiteral( "GPKG:%1:%2" ).arg( filename, tablename );
        handled = true;
      }
    }
    else if ( src.startsWith( QLatin1String( "HDF4_SDS:" ) ) )
    {
      // HDF4_SDS:subdataset_type:file_name:subdataset_index
      // filename can be quoted with " as it can contain colons
      QRegExp r( "HDF4_SDS:([^:]+):(.+):([^:]+)" );
      if ( r.exactMatch( src ) )
      {
        QString filename = r.cap( 2 );
        if ( filename.startsWith( '"' ) && filename.endsWith( '"' ) )
          filename = filename.mid( 1, filename.length() - 2 );
        src = "HDF4_SDS:" + r.cap( 1 ) + ":\"" + context.pathResolver().writePath( filename ) + "\":" + r.cap( 3 );
        handled = true;
      }
    }
    else if ( src.startsWith( QLatin1String( "HDF5:" ) ) )
    {
      // HDF5:file_name:subdataset
      // filename can be quoted with " as it can contain colons
      QRegExp r( "HDF5:(.+):([^:]+)" );
      if ( r.exactMatch( src ) )
      {
        QString filename = r.cap( 1 );
        if ( filename.startsWith( '"' ) && filename.endsWith( '"' ) )
          filename = filename.mid( 1, filename.length() - 2 );
        src = "HDF5:\"" + context.pathResolver().writePath( filename ) + "\":" + r.cap( 2 );
        handled = true;
      }
    }
    else if ( src.contains( QRegExp( "^(NITF_IM|RADARSAT_2_CALIB):" ) ) )
    {
      // NITF_IM:0:filename
      // RADARSAT_2_CALIB:?:filename
      QRegExp r( "([^:]+):([^:]+):(.+)" );
      if ( r.exactMatch( src ) )
      {
        src = r.cap( 1 ) + ':' + r.cap( 2 ) + ':' + context.pathResolver().writePath( r.cap( 3 ) );
        handled = true;
      }
    }
  }

  if ( !handled )
    src = context.pathResolver().writePath( src );

  return src;
}

QString QgsRasterLayer::decodedSource( const QString &source, const QString &provider, const QgsReadWriteContext &context ) const
{
  QString src( source );

  if ( provider == QLatin1String( "wms" ) )
  {
    // >>> BACKWARD COMPATIBILITY < 1.9
    // For project file backward compatibility we must support old format:
    // 1. mode: <url>
    //    example: http://example.org/wms?
    // 2. mode: tiled=<width>;<height>;<resolution>;<resolution>...,ignoreUrl=GetMap;GetFeatureInfo,featureCount=<count>,username=<name>,password=<password>,url=<url>
    //    example: tiled=256;256;0.703;0.351,url=http://example.org/tilecache?
    //    example: featureCount=10,http://example.org/wms?
    //    example: ignoreUrl=GetMap;GetFeatureInfo,username=cimrman,password=jara,url=http://example.org/wms?
    // This is modified version of old QgsWmsProvider::parseUri
    // The new format has always params crs,format,layers,styles and that params
    // should not appear in old format url -> use them to identify version
    // XYZ tile layers do not need to contain crs,format params, but they have type=xyz
    if ( !src.contains( QLatin1String( "type=" ) ) &&
         !src.contains( QLatin1String( "crs=" ) ) && !src.contains( QLatin1String( "format=" ) ) )
    {
      QgsDebugMsg( QStringLiteral( "Old WMS URI format detected -> converting to new format" ) );
      QgsDataSourceUri uri;
      if ( !src.startsWith( QLatin1String( "http:" ) ) )
      {
        QStringList parts = src.split( ',' );
        QStringListIterator iter( parts );
        while ( iter.hasNext() )
        {
          QString item = iter.next();
          if ( item.startsWith( QLatin1String( "username=" ) ) )
          {
            uri.setParam( QStringLiteral( "username" ), item.mid( 9 ) );
          }
          else if ( item.startsWith( QLatin1String( "password=" ) ) )
          {
            uri.setParam( QStringLiteral( "password" ), item.mid( 9 ) );
          }
          else if ( item.startsWith( QLatin1String( "tiled=" ) ) )
          {
            // in < 1.9 tiled= may apper in to variants:
            // tiled=width;height - non tiled mode, specifies max width and max height
            // tiled=width;height;resolutions-1;resolution2;... - tile mode

            QStringList params = item.mid( 6 ).split( ';' );

            if ( params.size() == 2 ) // non tiled mode
            {
              uri.setParam( QStringLiteral( "maxWidth" ), params.takeFirst() );
              uri.setParam( QStringLiteral( "maxHeight" ), params.takeFirst() );
            }
            else if ( params.size() > 2 ) // tiled mode
            {
              // resolutions are no more needed and size limit is not used for tiles
              // we have to tell to the provider however that it is tiled
              uri.setParam( QStringLiteral( "tileMatrixSet" ), QString() );
            }
          }
          else if ( item.startsWith( QLatin1String( "featureCount=" ) ) )
          {
            uri.setParam( QStringLiteral( "featureCount" ), item.mid( 13 ) );
          }
          else if ( item.startsWith( QLatin1String( "url=" ) ) )
          {
            uri.setParam( QStringLiteral( "url" ), item.mid( 4 ) );
          }
          else if ( item.startsWith( QLatin1String( "ignoreUrl=" ) ) )
          {
            uri.setParam( QStringLiteral( "ignoreUrl" ), item.mid( 10 ).split( ';' ) );
          }
        }
      }
      else
      {
        uri.setParam( QStringLiteral( "url" ), src );
      }
      src = uri.encodedUri();
      // At this point, the URI is obviously incomplete, we add additional params
      // in QgsRasterLayer::readXml
    }
    // <<< BACKWARD COMPATIBILITY < 1.9
  }
  else
  {
    bool handled = false;

    if ( provider == QLatin1String( "gdal" ) )
    {
      if ( src.startsWith( QLatin1String( "NETCDF:" ) ) )
      {
        // NETCDF:filename:variable
        // filename can be quoted with " as it can contain colons
        QRegExp r( "NETCDF:(.+):([^:]+)" );
        if ( r.exactMatch( src ) )
        {
          QString filename = r.cap( 1 );
          if ( filename.startsWith( '"' ) && filename.endsWith( '"' ) )
            filename = filename.mid( 1, filename.length() - 2 );
          src = "NETCDF:\"" + context.pathResolver().readPath( filename ) + "\":" + r.cap( 2 );
          handled = true;
        }
      }
      else if ( src.startsWith( QLatin1String( "GPKG:" ) ) )
      {
        // GPKG:filename:table
        QString filename, tablename;
        if ( _parseGpkgColons( src, filename, tablename ) )
        {
          filename = context.pathResolver().readPath( filename );
          src = QStringLiteral( "GPKG:%1:%2" ).arg( filename, tablename );
          handled = true;
        }
      }
      else if ( src.startsWith( QLatin1String( "HDF4_SDS:" ) ) )
      {
        // HDF4_SDS:subdataset_type:file_name:subdataset_index
        // filename can be quoted with " as it can contain colons
        QRegExp r( "HDF4_SDS:([^:]+):(.+):([^:]+)" );
        if ( r.exactMatch( src ) )
        {
          QString filename = r.cap( 2 );
          if ( filename.startsWith( '"' ) && filename.endsWith( '"' ) )
            filename = filename.mid( 1, filename.length() - 2 );
          src = "HDF4_SDS:" + r.cap( 1 ) + ":\"" + context.pathResolver().readPath( filename ) + "\":" + r.cap( 3 );
          handled = true;
        }
      }
      else if ( src.startsWith( QLatin1String( "HDF5:" ) ) )
      {
        // HDF5:file_name:subdataset
        // filename can be quoted with " as it can contain colons
        QRegExp r( "HDF5:(.+):([^:]+)" );
        if ( r.exactMatch( src ) )
        {
          QString filename = r.cap( 1 );
          if ( filename.startsWith( '"' ) && filename.endsWith( '"' ) )
            filename = filename.mid( 1, filename.length() - 2 );
          src = "HDF5:\"" + context.pathResolver().readPath( filename ) + "\":" + r.cap( 2 );
          handled = true;
        }
      }
      else if ( src.contains( QRegExp( "^(NITF_IM|RADARSAT_2_CALIB):" ) ) )
      {
        // NITF_IM:0:filename
        // RADARSAT_2_CALIB:?:filename
        QRegExp r( "([^:]+):([^:]+):(.+)" );
        if ( r.exactMatch( src ) )
        {
          src = r.cap( 1 ) + ':' + r.cap( 2 ) + ':' + context.pathResolver().readPath( r.cap( 3 ) );
          handled = true;
        }
      }
    }

    if ( !handled )
      src = context.pathResolver().readPath( src );
  }

  return src;
}

int QgsRasterLayer::width() const
{
  if ( !mDataProvider ) return 0;
  return mDataProvider->xSize();
}

int QgsRasterLayer::height() const
{
  if ( !mDataProvider ) return 0;
  return mDataProvider->ySize();
}

//////////////////////////////////////////////////////////
//
// Private methods
//
/////////////////////////////////////////////////////////
bool QgsRasterLayer::update()
{
  QgsDebugMsgLevel( QStringLiteral( "entered." ), 4 );
  // Check if data changed
  if ( mDataProvider->dataTimestamp() > mDataProvider->timestamp() )
  {
    QgsDebugMsgLevel( QStringLiteral( "reload data" ), 4 );
    closeDataProvider();
    init();
    QgsDataProvider::ProviderOptions providerOptions;
    setDataProvider( mProviderKey, providerOptions );
    emit dataChanged();
  }
  return mValid;
}
