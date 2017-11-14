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
#include "qgsrectangle.h"
#include "qgsrendercontext.h"
#include "qgssinglebandcolordatarenderer.h"
#include "qgssinglebandgrayrenderer.h"
#include "qgssinglebandpseudocolorrenderer.h"
#include "qgssettings.h"

#include <cmath>
#include <cstdio>
#include <limits>
#include <typeinfo>

#include <QApplication>
#include <QCursor>
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
  , mProviderKey( providerKey )
{
  QgsDebugMsgLevel( "Entered", 4 );
  init();
  setDataProvider( providerKey );
  if ( !mValid ) return;

  // load default style
  bool defaultLoadedFlag = false;
  if ( mValid && options.loadDefaultStyle )
  {
    loadDefaultStyle( defaultLoadedFlag );
  }
  if ( !defaultLoadedFlag )
  {
    setDefaultContrastEnhancement();
  }

  // TODO: Connect signals from the dataprovider to the qgisapp

  emit statusChanged( tr( "QgsRasterLayer created" ) );
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

/**
 * This helper checks to see whether the file name appears to be a valid raster file name
 */
bool QgsRasterLayer::isValidRasterFileName( const QString &fileNameQString, QString &retErrMsg )
{
  isvalidrasterfilename_t *pValid = reinterpret_cast< isvalidrasterfilename_t * >( cast_to_fptr( QgsProviderRegistry::instance()->function( "gdal",  "isValidRasterFileName" ) ) );
  if ( ! pValid )
  {
    QgsDebugMsg( "Could not resolve isValidRasterFileName in gdal provider library" );
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

// typedef for the QgsDataProvider class factory
typedef QgsDataProvider *classFactoryFunction_t( const QString * );

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

/**
 * @return 0 if not using the data provider model (i.e. directly using GDAL)
 */
QgsRasterDataProvider *QgsRasterLayer::dataProvider()
{
  return mDataProvider;
}

/**
 * @return 0 if not using the data provider model (i.e. directly using GDAL)
 */
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
  QgsDebugMsgLevel( " 3 arguments", 4 );
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

  QgsDebugMsgLevel( QString( "total raster draw time (ms):     %1" ).arg( time.elapsed(), 5 ), 4 );
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

  // Identification section
  myMetadata += QStringLiteral( "<h1>" ) + tr( "Identification" ) + QStringLiteral( "</h1>\n<hr>\n" );
  myMetadata += htmlFormatter.identificationSectionHtml();
  myMetadata += QLatin1String( "<br><br>\n" );

  // Begin Provider section
  myMetadata += QStringLiteral( "<h1>" ) + tr( "Information from provider" ) + QStringLiteral( "</h1>\n<hr>\n" );
  myMetadata += QLatin1String( "<table class=\"list-view\">\n" );

  // original name
  myMetadata += QStringLiteral( "<tr><td class=\"highlight\">" ) + tr( "Original" ) + QStringLiteral( "</td><td>" ) + name() + QStringLiteral( "</td></tr>\n" );

  // name
  myMetadata += QStringLiteral( "<tr><td class=\"highlight\">" ) + tr( "Name" ) + QStringLiteral( "</td><td>" ) + name() + QStringLiteral( "</td></tr>\n" );

  // data source
  myMetadata += QStringLiteral( "<tr><td class=\"highlight\">" ) + tr( "Source" ) + QStringLiteral( "</td><td>" ) + publicSource() + QStringLiteral( "</td></tr>\n" );

  // storage type
  myMetadata += QStringLiteral( "<tr><td class=\"highlight\">" ) + tr( "Provider" ) + QStringLiteral( "</td><td>" ) + providerType() + QStringLiteral( "</td></tr>\n" );

  // EPSG
  myMetadata += QStringLiteral( "<tr><td class=\"highlight\">" ) + tr( "CRS" ) + QStringLiteral( "</td><td>" ) + crs().authid() + QStringLiteral( " - " );
  myMetadata += crs().description() + QStringLiteral( " - " );
  if ( crs().isGeographic() )
    myMetadata += tr( "Geographic" );
  else
    myMetadata += tr( "Projected" );
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

  // End Provider section
  myMetadata += QLatin1String( "</table>\n<br><br>" );

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
  myMetadata += "<tr><th>" + tr( "Number" ) + "</th><th>" + tr( "Band" ) + "</th><th>" + tr( "No-Data" ) + "</th><th>" + tr( "Min" ) + "</th><th>" + tr( "Max" ) + "</th></tr>\n";

  QgsRasterDataProvider *provider = const_cast< QgsRasterDataProvider * >( mDataProvider );
  for ( int i = 1; i <= bandCount(); i++ )
  {
    QString rowClass;
    if ( i % 2 )
      rowClass = QStringLiteral( "class=\"odd-row\"" );
    myMetadata += "<tr " + rowClass + "><td>" + QString::number( i ) + "</td><td>" + bandName( i ) + "</td><td>";

    if ( dataProvider()->sourceHasNoDataValue( i ) )
      myMetadata += QString::number( dataProvider()->sourceNoDataValue( i ) );
    else
      myMetadata += tr( "n/a" );
    myMetadata += QLatin1String( "</td>" );

    if ( provider->hasStatistics( i ) )
    {
      QgsRasterBandStats myRasterBandStats = provider->bandStatistics( i );
      myMetadata += "<td>" + QString::number( myRasterBandStats.minimumValue, 'f', 10 ) + "</td>";
      myMetadata += "<td>" + QString::number( myRasterBandStats.maximumValue, 'f', 10 ) + "</td>";
    }
    else
    {
      myMetadata += "<td>" + tr( "n/a" ) + "</td><td>" + tr( "n/a" ) + "</td>";
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


/**
 * @param bandNumber the number of the band to use for generating a pixmap of the associated palette
 * @return a 100x100 pixel QPixmap of the bands palette
 */
QPixmap QgsRasterLayer::paletteAsPixmap( int bandNumber )
{
  //TODO: This function should take dimensions
  QgsDebugMsgLevel( "entered.", 4 );

  // Only do this for the GDAL provider?
  // Maybe WMS can do this differently using QImage::numColors and QImage::color()
  if ( mDataProvider->colorInterpretation( bandNumber ) == QgsRaster::PaletteIndex )
  {
    QgsDebugMsgLevel( "....found paletted image", 4 );
    QgsColorRampShader myShader;
    QList<QgsColorRampShader::ColorRampItem> myColorRampItemList = mDataProvider->colorTable( bandNumber );
    if ( !myColorRampItemList.isEmpty() )
    {
      QgsDebugMsgLevel( "....got color ramp item list", 4 );
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

/**
 * @return the horizontal units per pixel as reported in the  GDAL geotramsform[1]
 */
double QgsRasterLayer::rasterUnitsPerPixelX() const
{
// We return one raster pixel per map unit pixel
// One raster pixel can have several raster units...

// We can only use one of the mGeoTransform[], so go with the
// horisontal one.

  if ( mDataProvider->capabilities() & QgsRasterDataProvider::Size && mDataProvider->xSize() > 0 )
  {
    return mDataProvider->extent().width() / mDataProvider->xSize();
  }
  return 1;
}

double QgsRasterLayer::rasterUnitsPerPixelY() const
{
  if ( mDataProvider->capabilities() & QgsRasterDataProvider::Size && mDataProvider->xSize() > 0 )
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

void QgsRasterLayer::setDataProvider( QString const &provider )
{
  QgsDebugMsgLevel( "Entered", 4 );
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

  mDataProvider = dynamic_cast< QgsRasterDataProvider * >( QgsProviderRegistry::instance()->createProvider( mProviderKey, mDataSource ) );
  if ( !mDataProvider )
  {
    //QgsMessageLog::logMessage( tr( "Cannot instantiate the data provider" ), tr( "Raster" ) );
    appendError( ERR( tr( "Cannot instantiate the '%1' data provider" ).arg( mProviderKey ) ) );
    return;
  }
  QgsDebugMsgLevel( "Data provider created", 4 );

  // Set data provider into pipe even if not valid so that it is deleted with pipe (with layer)
  mPipe.set( mDataProvider );
  if ( !mDataProvider->isValid() )
  {
    setError( mDataProvider->error() );
    appendError( ERR( tr( "Provider is not valid (provider: %1, URI: %2" ).arg( mProviderKey, mDataSource ) ) );
    return;
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

  QgsDebugMsgLevel( "exiting.", 4 );
} // QgsRasterLayer::setDataProvider

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
    QgsDebugMsgLevel( QString( "myLower = %1 myUpper = %2" ).arg( myLower ).arg( myUpper ), 4 );
    mDataProvider->cumulativeCut( band, myLower, myUpper, min, max, extent, sampleSize );
  }
  QgsDebugMsgLevel( QString( "band = %1 min = %2 max = %3" ).arg( band ).arg( min ).arg( max ), 4 );

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
  QgsDebugMsgLevel( QString( "theAlgorithm = %1 limits = %2 extent.isEmpty() = %3" ).arg( algorithm ).arg( limits ).arg( extent.isEmpty() ), 4 );
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
    QgsDebugMsg( "No default contrast enhancement for this drawing style" );
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
  QgsDebugMsgLevel( "Entered", 4 );

  QgsContrastEnhancement::ContrastEnhancementAlgorithm myAlgorithm;
  QgsRasterMinMaxOrigin::Limits myLimits;
  defaultContrastEnhancementSettings( myAlgorithm, myLimits );

  setContrastEnhancement( myAlgorithm, myLimits );
}

void QgsRasterLayer::setLayerOrder( QStringList const &layers )
{
  QgsDebugMsgLevel( "entered.", 4 );

  if ( mDataProvider )
  {
    QgsDebugMsgLevel( "About to mDataProvider->setLayerOrder(layers).", 4 );
    mDataProvider->setLayerOrder( layers );
  }

}

void QgsRasterLayer::setSubLayerVisibility( const QString &name, bool vis )
{

  if ( mDataProvider )
  {
    QgsDebugMsgLevel( "About to mDataProvider->setSubLayerVisibility(name, vis).", 4 );
    mDataProvider->setSubLayerVisibility( name, vis );
  }

}

QDateTime QgsRasterLayer::timestamp() const
{
  return mDataProvider->timestamp();
}

void QgsRasterLayer::setRenderer( QgsRasterRenderer *renderer )
{
  QgsDebugMsgLevel( "Entered", 4 );
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
 * @param QDomNode node that will contain the symbology definition for this layer.
 * @param errorMessage reference to string that will be updated with any error messages
 * @return true in case of success.
 */
bool QgsRasterLayer::readSymbology( const QDomNode &layer_node, QString &errorMessage, const QgsReadWriteContext &context )
{
  Q_UNUSED( errorMessage );
  Q_UNUSED( context );
  QDomElement rasterRendererElem;

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

bool QgsRasterLayer::readStyle( const QDomNode &node, QString &errorMessage, const QgsReadWriteContext &context )
{
  return readSymbology( node, errorMessage, context );
} //readSymbology

/**

  Raster layer project file XML of form:

  \note Called by QgsMapLayer::readXml().
*/
bool QgsRasterLayer::readXml( const QDomNode &layer_node, const QgsReadWriteContext &context )
{
  QgsDebugMsgLevel( "Entered", 4 );
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
      QgsDebugMsgLevel( "Old WMS URI format detected -> adding params", 4 );
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

  setDataProvider( mProviderKey );
  if ( !mValid ) return false;

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
      QgsDebugMsg( "data changed, reload provider" );
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
    QgsDebugMsgLevel( QString( "bandNo = %1" ).arg( bandNo ), 4 );
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
        QgsDebugMsgLevel( QString( "min = %1 %2" ).arg( rangeElement.attribute( "min" ) ).arg( myNoDataRange.min() ), 4 );
        myNoDataRangeList << myNoDataRange;
      }
      mDataProvider->setUserNoDataValue( bandNo, myNoDataRangeList );
    }
  }

  readStyleManager( layer_node );

  return res;
} // QgsRasterLayer::readXml( QDomNode & layer_node )

/*
 * @param QDomNode the node that will have the style element added to it.
 * @param QDomDocument the document that will have the QDomNode added.
 * @param errorMessage reference to string that will be updated with any error messages
 * @return true in case of success.
 */
bool QgsRasterLayer::writeSymbology( QDomNode &layer_node, QDomDocument &document, QString &errorMessage, const QgsReadWriteContext &context ) const
{
  Q_UNUSED( errorMessage );
  Q_UNUSED( context );

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

bool QgsRasterLayer::writeStyle( QDomNode &node, QDomDocument &doc, QString &errorMessage, const QgsReadWriteContext &context ) const
{
  return writeSymbology( node, doc, errorMessage, context );

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
  QgsDebugMsgLevel( "entered.", 4 );
  // Check if data changed
  if ( mDataProvider->dataTimestamp() > mDataProvider->timestamp() )
  {
    QgsDebugMsgLevel( "reload data", 4 );
    closeDataProvider();
    init();
    setDataProvider( mProviderKey );
    emit dataChanged();
  }
  return mValid;
}
