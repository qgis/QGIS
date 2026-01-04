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
#include "qgsrasterlayer.h"

#include <cmath>
#include <cstdio>
#include <limits>
#include <typeinfo>

#include "qgsapplication.h"
#include "qgsbilinearrasterresampler.h"
#include "qgsbrightnesscontrastfilter.h"
#include "qgscolorrampshader.h"
#include "qgscoordinatereferencesystem.h"
#include "qgscubicrasterresampler.h"
#include "qgsdatasourceuri.h"
#include "qgsgdalprovider.h"
#include "qgshuesaturationfilter.h"
#include "qgslayermetadataformatter.h"
#include "qgslogger.h"
#include "qgsmaplayerfactory.h"
#include "qgsmaplayerlegend.h"
#include "qgsmaptopixel.h"
#include "qgsmessagelog.h"
#include "qgsmultibandcolorrenderer.h"
#include "qgspainting.h"
#include "qgspathresolver.h"
#include "qgsprojectfiletransform.h"
#include "qgsproviderregistry.h"
#include "qgsrasterdataprovider.h"
#include "qgsrasterdrawer.h"
#include "qgsrasteriterator.h"
#include "qgsrasterlabeling.h"
#include "qgsrasterlayerelevationproperties.h"
#include "qgsrasterlayerprofilegenerator.h"
#include "qgsrasterlayerrenderer.h"
#include "qgsrasterlayertemporalproperties.h"
#include "qgsrasterpipe.h"
#include "qgsrasterprojector.h"
#include "qgsrasterrange.h"
#include "qgsrasterrendererregistry.h"
#include "qgsrasterresamplefilter.h"
#include "qgsrastershader.h"
#include "qgsreadwritecontext.h"
#include "qgsrectangle.h"
#include "qgsrendercontext.h"
#include "qgsruntimeprofiler.h"
#include "qgssettings.h"
#include "qgssettingsentryimpl.h"
#include "qgssettingstree.h"
#include "qgssinglebandgrayrenderer.h"
#include "qgssinglebandpseudocolorrenderer.h"
#include "qgssldexportcontext.h"
#include "qgssymbollayerutils.h"
#include "qgsthreadingutils.h"
#include "qgsxmlutils.h"

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
#include <QList>
#include <QPainter>
#include <QPixmap>
#include <QRegularExpression>
#include <QSlider>
#include <QUrl>

#include "moc_qgsrasterlayer.cpp"

const QgsSettingsEntryDouble *QgsRasterLayer::settingsRasterDefaultOversampling = new QgsSettingsEntryDouble( u"default-oversampling"_s, QgsSettingsTree::sTreeRaster, 2.0 );
const QgsSettingsEntryBool *QgsRasterLayer::settingsRasterDefaultEarlyResampling = new QgsSettingsEntryBool( u"default-early-resampling"_s, QgsSettingsTree::sTreeRaster, false );

#define ERR(message) QGS_ERROR_MESSAGE(message,"Raster layer")

const double QgsRasterLayer::SAMPLE_SIZE = 250000;

const QgsContrastEnhancement::ContrastEnhancementAlgorithm
QgsRasterLayer::SINGLE_BAND_ENHANCEMENT_ALGORITHM = QgsContrastEnhancement::StretchToMinimumMaximum;
const QgsContrastEnhancement::ContrastEnhancementAlgorithm
QgsRasterLayer::MULTIPLE_BAND_SINGLE_BYTE_ENHANCEMENT_ALGORITHM = QgsContrastEnhancement::NoEnhancement;
const QgsContrastEnhancement::ContrastEnhancementAlgorithm
QgsRasterLayer::MULTIPLE_BAND_MULTI_BYTE_ENHANCEMENT_ALGORITHM = QgsContrastEnhancement::StretchToMinimumMaximum;

const Qgis::RasterRangeLimit
QgsRasterLayer::SINGLE_BAND_MIN_MAX_LIMITS = Qgis::RasterRangeLimit::MinimumMaximum;
const Qgis::RasterRangeLimit
QgsRasterLayer::MULTIPLE_BAND_SINGLE_BYTE_MIN_MAX_LIMITS = Qgis::RasterRangeLimit::MinimumMaximum;
const Qgis::RasterRangeLimit
QgsRasterLayer::MULTIPLE_BAND_MULTI_BYTE_MIN_MAX_LIMITS = Qgis::RasterRangeLimit::CumulativeCut;

QgsRasterLayer::QgsRasterLayer()
  : QgsMapLayer( Qgis::LayerType::Raster )
  , QSTRING_NOT_SET( u"Not Set"_s )
  , TRSTRING_NOT_SET( tr( "Not Set" ) )
  , mTemporalProperties( new QgsRasterLayerTemporalProperties( this ) )
  , mElevationProperties( new QgsRasterLayerElevationProperties( this ) )
  , mPipe( std::make_unique< QgsRasterPipe >() )
{
  init();
  setValid( false );
}

QgsRasterLayer::QgsRasterLayer( const QString &uri,
                                const QString &baseName,
                                const QString &providerKey,
                                const LayerOptions &options )
  : QgsMapLayer( Qgis::LayerType::Raster, baseName, uri )
    // Constant that signals property not used.
  , QSTRING_NOT_SET( u"Not Set"_s )
  , TRSTRING_NOT_SET( tr( "Not Set" ) )
  , mTemporalProperties( new QgsRasterLayerTemporalProperties( this ) )
  , mElevationProperties( new QgsRasterLayerElevationProperties( this ) )
  , mPipe( std::make_unique< QgsRasterPipe >() )
{
  mShouldValidateCrs = !options.skipCrsValidation;

  QgsDebugMsgLevel( u"Entered"_s, 4 );
  setProviderType( providerKey );

  const QgsDataProvider::ProviderOptions providerOptions { options.transformContext };
  Qgis::DataProviderReadFlags providerFlags;
  if ( options.loadDefaultStyle )
  {
    providerFlags |= Qgis::DataProviderReadFlag::LoadDefaultStyle;
  }

  setDataSource( uri, baseName, providerKey, providerOptions, providerFlags );

  if ( isValid() )
  {
    mTemporalProperties->setDefaultsFromDataProviderTemporalCapabilities( mDataProvider->temporalCapabilities() );
  }

} // QgsRasterLayer ctor

QgsRasterLayer::~QgsRasterLayer()
{
  emit willBeDeleted();

  mLabeling.reset();

  setValid( false );
  // Note: provider and other interfaces are owned and deleted by pipe
}

QgsRasterLayer *QgsRasterLayer::clone() const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  QgsRasterLayer::LayerOptions options;
  if ( mDataProvider )
  {
    options.transformContext = mDataProvider->transformContext();
  }
  auto layer = std::make_unique< QgsRasterLayer >( source(), name(), mProviderKey, options );
  QgsMapLayer::clone( layer.get() );
  layer->mElevationProperties = mElevationProperties->clone();
  layer->mElevationProperties->setParent( layer.get() );
  layer->setMapTipTemplate( mapTipTemplate() );
  layer->setMapTipsEnabled( mapTipsEnabled() );

  // do not clone data provider which is the first element in pipe
  for ( int i = 1; i < mPipe->size(); i++ )
  {
    if ( mPipe->at( i ) )
      layer->pipe()->set( mPipe->at( i )->clone() );
  }
  layer->pipe()->setDataDefinedProperties( mPipe->dataDefinedProperties() );
  layer->setResamplingStage( mPipe->resamplingStage() );
  if ( mDataProvider && layer->dataProvider() )
  {
    layer->dataProvider()->setZoomedInResamplingMethod( mDataProvider->zoomedInResamplingMethod() );
    layer->dataProvider()->setZoomedOutResamplingMethod( mDataProvider->zoomedOutResamplingMethod() );
  }

  layer->setLabelsEnabled( mLabelsEnabled );
  if ( mLabeling )
    layer->setLabeling( mLabeling->clone() );

  return layer.release();
}

QgsAbstractProfileGenerator *QgsRasterLayer::createProfileGenerator( const QgsProfileRequest &request )
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  if ( !mElevationProperties->isEnabled() )
    return nullptr;

  return new QgsRasterLayerProfileGenerator( this, request );
}

//////////////////////////////////////////////////////////
//
// Static Methods and members
//
/////////////////////////////////////////////////////////

bool QgsRasterLayer::isValidRasterFileName( const QString &fileNameQString, QString &retErrMsg )
{
  const bool myIsValid = QgsGdalProvider::isValidRasterFileName( fileNameQString, retErrMsg );
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

  const QFileInfo fi( name );

  // Is it file?
  if ( !fi.exists() )
    return t;

  t = fi.lastModified();

  QgsDebugMsgLevel( "last modified = " + t.toString(), 4 );

  return t;
}

void QgsRasterLayer::setDataProvider( const QString &provider )
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

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
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  if ( !mDataProvider ) return 0;
  return mDataProvider->bandCount();
}

QString QgsRasterLayer::bandName( int bandNo ) const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  if ( !mDataProvider ) return QString();
  return mDataProvider->generateBandName( bandNo );
}

QgsRasterAttributeTable *QgsRasterLayer::attributeTable( int bandNoInt ) const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  if ( !mDataProvider )
    return nullptr;
  return mDataProvider->attributeTable( bandNoInt );
}

int QgsRasterLayer::attributeTableCount() const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  if ( !mDataProvider )
    return 0;

  int ratCount { 0 };
  for ( int bandNo = 1; bandNo <= bandCount(); ++bandNo )
  {
    if ( attributeTable( bandNo ) )
    {
      ratCount++;
    }
  }
  return ratCount;
}

bool QgsRasterLayer::canCreateRasterAttributeTable()
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  return mDataProvider && renderer() && renderer()->canCreateRasterAttributeTable();
}

void QgsRasterLayer::setRendererForDrawingStyle( Qgis::RasterDrawingStyle drawingStyle )
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  setRenderer( QgsApplication::rasterRendererRegistry()->defaultRendererForDrawingStyle( drawingStyle, mDataProvider ) );
}

QgsRasterDataProvider *QgsRasterLayer::dataProvider()
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  return mDataProvider;
}

const QgsRasterDataProvider *QgsRasterLayer::dataProvider() const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  return mDataProvider;
}

void QgsRasterLayer::reload()
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  if ( mDataProvider )
  {
    mDataProvider->reloadData();
  }
}

QgsMapLayerRenderer *QgsRasterLayer::createMapRenderer( QgsRenderContext &rendererContext )
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  return new QgsRasterLayerRenderer( this, rendererContext );
}


void QgsRasterLayer::draw( QPainter *theQPainter,
                           QgsRasterViewPort *rasterViewPort,
                           const QgsMapToPixel *qgsMapToPixel )
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  QgsDebugMsgLevel( u" 3 arguments"_s, 4 );
  QElapsedTimer time;
  time.start();
  //
  //
  // The goal here is to make as many decisions as possible early on (outside of the rendering loop)
  // so that we can maximise performance of the rendering process. So now we check which drawing
  // procedure to use :
  //

  QgsRasterProjector *projector = mPipe->projector();
  bool restoreOldResamplingStage = false;
  const Qgis::RasterResamplingStage oldResamplingState = resamplingStage();
  // TODO add a method to interface to get provider and get provider
  // params in QgsRasterProjector

  if ( projector )
  {
    // Force provider resampling if reprojection is needed
    if ( mDataProvider &&
         ( mDataProvider->providerCapabilities() & Qgis::RasterProviderCapability::ProviderHintCanPerformProviderResampling ) &&
         rasterViewPort->mSrcCRS != rasterViewPort->mDestCRS &&
         oldResamplingState != Qgis::RasterResamplingStage::Provider )
    {
      restoreOldResamplingStage = true;
      setResamplingStage( Qgis::RasterResamplingStage::Provider );
    }
    projector->setCrs( rasterViewPort->mSrcCRS, rasterViewPort->mDestCRS, rasterViewPort->mTransformContext );
  }

  // Drawer to pipe?
  QgsRasterIterator iterator( mPipe->last() );
  QgsRasterDrawer drawer( &iterator );
  drawer.draw( theQPainter, rasterViewPort, qgsMapToPixel );

  if ( restoreOldResamplingStage )
  {
    setResamplingStage( oldResamplingState );
  }

  QgsDebugMsgLevel( u"total raster draw time (ms):     %1"_s.arg( time.elapsed(), 5 ), 4 );
}

QgsLegendColorList QgsRasterLayer::legendSymbologyItems() const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  QgsRasterRenderer *renderer = mPipe->renderer();
  return renderer ? renderer->legendSymbologyItems() : QList< QPair< QString, QColor > >();;
}

QString QgsRasterLayer::htmlMetadata() const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  if ( !mDataProvider )
    return QString();

  const QgsLayerMetadataFormatter htmlFormatter( metadata() );
  QString myMetadata = u"<html><head></head>\n<body>\n"_s;

  myMetadata += generalHtmlMetadata();

  // Begin Provider section
  myMetadata += u"<h1>"_s + tr( "Information from provider" ) + u"</h1>\n<hr>\n"_s + u"<table class=\"list-view\">\n"_s;

  myMetadata += u"\n"_s %
                // Extent
                u"<tr><td class=\"highlight\">"_s % tr( "Extent" ) % u"</td><td>"_s % extent().toString() % u"</td></tr>\n"_s %

                // Raster Width
                u"<tr><td class=\"highlight\">"_s % tr( "Width" ) % u"</td><td>"_s;
  if ( dataProvider()->capabilities() & Qgis::RasterInterfaceCapability::Size )
    myMetadata += QString::number( width() );
  else
    myMetadata += tr( "n/a" );
  myMetadata += u"</td></tr>\n"_s %

                // Raster height
                u"<tr><td class=\"highlight\">"_s + tr( "Height" ) + u"</td><td>"_s;
  if ( dataProvider()->capabilities() & Qgis::RasterInterfaceCapability::Size )
    myMetadata += QString::number( height() );
  else
    myMetadata += tr( "n/a" );
  myMetadata += u"</td></tr>\n"_s %

                // Data type
                u"<tr><td class=\"highlight\">"_s % tr( "Data type" ) % u"</td><td>"_s;
  // Just use the first band
  switch ( mDataProvider->sourceDataType( 1 ) )
  {
    case Qgis::DataType::Byte:
      myMetadata += tr( "Byte - Eight bit unsigned integer" );
      break;
    case Qgis::DataType::Int8:
      myMetadata += tr( "Int8 - Eight bit signed integer" );
      break;
    case Qgis::DataType::UInt16:
      myMetadata += tr( "UInt16 - Sixteen bit unsigned integer " );
      break;
    case Qgis::DataType::Int16:
      myMetadata += tr( "Int16 - Sixteen bit signed integer " );
      break;
    case Qgis::DataType::UInt32:
      myMetadata += tr( "UInt32 - Thirty two bit unsigned integer " );
      break;
    case Qgis::DataType::Int32:
      myMetadata += tr( "Int32 - Thirty two bit signed integer " );
      break;
    case Qgis::DataType::Float32:
      myMetadata += tr( "Float32 - Thirty two bit floating point " );
      break;
    case Qgis::DataType::Float64:
      myMetadata += tr( "Float64 - Sixty four bit floating point " );
      break;
    case Qgis::DataType::CInt16:
      myMetadata += tr( "CInt16 - Complex Int16 " );
      break;
    case Qgis::DataType::CInt32:
      myMetadata += tr( "CInt32 - Complex Int32 " );
      break;
    case Qgis::DataType::CFloat32:
      myMetadata += tr( "CFloat32 - Complex Float32 " );
      break;
    case Qgis::DataType::CFloat64:
      myMetadata += tr( "CFloat64 - Complex Float64 " );
      break;
    default:
      myMetadata += tr( "Could not determine raster data type." );
  }
  myMetadata += u"</td></tr>\n"_s %

                // Insert provider-specific (e.g. WMS-specific) metadata
                mDataProvider->htmlMetadata() %

                // End Provider section
                u"</table>\n<br><br>"_s;

  // CRS
  myMetadata += crsHtmlMetadata();

  // Identification section
  myMetadata += u"<h1>"_s % tr( "Identification" ) % u"</h1>\n<hr>\n"_s %
                htmlFormatter.identificationSectionHtml() %
                u"<br><br>\n"_s %

                // extent section
                u"<h1>"_s % tr( "Extent" ) % u"</h1>\n<hr>\n"_s %
                htmlFormatter.extentSectionHtml( ) %
                u"<br><br>\n"_s %

                // Start the Access section
                u"<h1>"_s % tr( "Access" ) % u"</h1>\n<hr>\n"_s %
                htmlFormatter.accessSectionHtml( ) %
                u"<br><br>\n"_s %

                // Bands section
                u"</table>\n<br><br><h1>"_s % tr( "Bands" ) % u"</h1>\n<hr>\n<table class=\"list-view\">\n"_s %

                // Band count
                u"<tr><td class=\"highlight\">"_s % tr( "Band count" ) % u"</td><td>"_s % QString::number( bandCount() ) % u"</td></tr>\n"_s;

  // Band table
  myMetadata += u"</table>\n<br><table width=\"100%\" class=\"tabular-view\">\n"_s %
                u"<tr><th>"_s % tr( "Number" ) % u"</th><th>"_s % tr( "Band" ) % u"</th><th>"_s % tr( "NoData" ) % u"</th><th>"_s %
                tr( "Min" ) % u"</th><th>"_s % tr( "Max" ) % u"</th></tr>\n"_s;

  QgsRasterDataProvider *provider = const_cast< QgsRasterDataProvider * >( mDataProvider );
  for ( int i = 1; i <= bandCount(); i++ )
  {
    QString rowClass;
    if ( i % 2 )
      rowClass = u"class=\"odd-row\""_s;

    myMetadata += u"<tr "_s % rowClass % u"><td>"_s % QString::number( i ) % u"</td><td>"_s % bandName( i ) % u"</td><td>"_s;

    if ( dataProvider()->sourceHasNoDataValue( i ) )
      myMetadata += QString::number( dataProvider()->sourceNoDataValue( i ) );
    else
      myMetadata += tr( "n/a" );
    myMetadata += "</td>"_L1;

    if ( provider->hasStatistics( i, Qgis::RasterBandStatistic::Min | Qgis::RasterBandStatistic::Max, provider->extent(), static_cast<int>( SAMPLE_SIZE ) ) )
    {
      const QgsRasterBandStats myRasterBandStats = provider->bandStatistics( i, Qgis::RasterBandStatistic::Min | Qgis::RasterBandStatistic::Max, provider->extent(), static_cast<int>( SAMPLE_SIZE ) );
      myMetadata += u"<td>"_s % QString::number( myRasterBandStats.minimumValue, 'f', 10 ) % u"</td>"_s %
                    u"<td>"_s % QString::number( myRasterBandStats.maximumValue, 'f', 10 ) % u"</td>"_s;
    }
    else
    {
      myMetadata += u"<td>"_s % tr( "n/a" ) % u"</td><td>"_s % tr( "n/a" ) % u"</td>"_s;
    }

    myMetadata += "</tr>\n"_L1;
  }

  //close previous bands table
  myMetadata +=  u"</table>\n<br><br>"_s %

                 // Start the contacts section
                 u"<h1>"_s % tr( "Contacts" ) % u"</h1>\n<hr>\n"_s %
                 htmlFormatter.contactsSectionHtml( ) %
                 u"<br><br>\n"_s %

                 // Start the links section
                 u"<h1>"_s % tr( "References" ) % u"</h1>\n<hr>\n"_s %
                 htmlFormatter.linksSectionHtml( ) %
                 u"<br><br>\n"_s %

                 // Start the history section
                 u"<h1>"_s % tr( "History" ) % u"</h1>\n<hr>\n"_s %
                 htmlFormatter.historySectionHtml( ) %
                 u"<br><br>\n"_s %
                 customPropertyHtmlMetadata() %
                 u"\n</body>\n</html>\n"_s;
  return myMetadata;
}

Qgis::MapLayerProperties QgsRasterLayer::properties() const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  Qgis::MapLayerProperties res;
  if ( mDataProvider && ( mDataProvider->flags() & Qgis::DataProviderFlag::IsBasemapSource ) )
  {
    res |= Qgis::MapLayerProperty::IsBasemapLayer;
  }
  return res;
}

QPixmap QgsRasterLayer::paletteAsPixmap( int bandNumber )
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  //TODO: This function should take dimensions
  QgsDebugMsgLevel( u"entered."_s, 4 );

  // Only do this for the GDAL provider?
  // Maybe WMS can do this differently using QImage::numColors and QImage::color()
  if ( mDataProvider &&
       mDataProvider->colorInterpretation( bandNumber ) == Qgis::RasterColorInterpretation::PaletteIndex )
  {
    QgsDebugMsgLevel( u"....found paletted image"_s, 4 );
    QgsColorRampShader myShader;
    const QList<QgsColorRampShader::ColorRampItem> myColorRampItemList = mDataProvider->colorTable( bandNumber );
    if ( !myColorRampItemList.isEmpty() )
    {
      QgsDebugMsgLevel( u"....got color ramp item list"_s, 4 );
      myShader.setColorRampItemList( myColorRampItemList );
      myShader.setColorRampType( Qgis::ShaderInterpolationMethod::Discrete );
      // Draw image
      const int mySize = 100;
      QPixmap myPalettePixmap( mySize, mySize );
      QPainter myQPainter( &myPalettePixmap );

      QImage myQImage = QImage( mySize, mySize, QImage::Format_RGB32 );
      myQImage.fill( 0 );
      myPalettePixmap.fill();

      const double myStep = ( static_cast< double >( myColorRampItemList.size() ) - 1 ) / static_cast< double >( mySize * mySize );
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
    const QPixmap myNullPixmap;
    return myNullPixmap;
  }
  else
  {
    //invalid layer  was requested
    const QPixmap myNullPixmap;
    return myNullPixmap;
  }
}

double QgsRasterLayer::rasterUnitsPerPixelX() const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

// We return one raster pixel per map unit pixel
// One raster pixel can have several raster units...

// We can only use one of the mGeoTransform[], so go with the
// horisontal one.

  if ( mDataProvider &&
       mDataProvider->capabilities() & Qgis::RasterInterfaceCapability::Size && !qgsDoubleNear( mDataProvider->xSize(), 0.0 ) )
  {
    return mDataProvider->extent().width() / mDataProvider->xSize();
  }
  return 1;
}

double QgsRasterLayer::rasterUnitsPerPixelY() const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  if ( mDataProvider &&
       mDataProvider->capabilities() & Qgis::RasterInterfaceCapability::Size && !qgsDoubleNear( mDataProvider->ySize(), 0.0 ) )
  {
    return mDataProvider->extent().height() / mDataProvider->ySize();
  }
  return 1;
}

void QgsRasterLayer::setOpacity( double opacity )
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  if ( !mPipe->renderer() || mPipe->renderer()->opacity() == opacity )
    return;

  mPipe->renderer()->setOpacity( opacity );
  emit opacityChanged( opacity );
  emitStyleChanged();
}

double QgsRasterLayer::opacity() const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  return mPipe->renderer() ? mPipe->renderer()->opacity() : 1.0;
}

void QgsRasterLayer::init()
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  mRasterType = Qgis::RasterLayerType::GrayOrUndefined;

  whileBlocking( this )->setLegend( QgsMapLayerLegend::defaultRasterLegend( this ) );

  setRendererForDrawingStyle( Qgis::RasterDrawingStyle::Undefined );

  //Initialize the last view port structure, should really be a class
  mLastViewPort.mWidth = 0;
  mLastViewPort.mHeight = 0;
}

void QgsRasterLayer::setDataProvider( QString const &provider, const QgsDataProvider::ProviderOptions &options, Qgis::DataProviderReadFlags flags )
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  QgsDebugMsgLevel( u"Entered"_s, 4 );
  setValid( false ); // assume the layer is invalid until we determine otherwise

  // deletes pipe elements (including data provider)
  mPipe = std::make_unique< QgsRasterPipe >();
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

  if ( mPreloadedProvider )
  {
    mDataProvider = qobject_cast< QgsRasterDataProvider * >( mPreloadedProvider.release() );
  }
  else
  {
    std::unique_ptr< QgsScopedRuntimeProfile > profile;
    if ( QgsApplication::profiler()->groupIsActive( u"projectload"_s ) )
      profile = std::make_unique< QgsScopedRuntimeProfile >( tr( "Create %1 provider" ).arg( provider ), u"projectload"_s );

    mDataProvider = qobject_cast< QgsRasterDataProvider * >( QgsProviderRegistry::instance()->createProvider( mProviderKey, mDataSource, options, flags ) );
  }

  if ( !mDataProvider )
  {
    //QgsMessageLog::logMessage( tr( "Cannot instantiate the data provider" ), tr( "Raster" ) );
    appendError( ERR( tr( "Cannot instantiate the '%1' data provider" ).arg( mProviderKey ) ) );
    return;
  }
  QgsDebugMsgLevel( u"Data provider created"_s, 4 );
  mDataProvider->setParent( this );

  // Set data provider into pipe even if not valid so that it is deleted with pipe (with layer)
  if ( !mPipe->set( mDataProvider ) )
  {
    // data provider has been deleted by the pipe
    mDataProvider = nullptr;
    appendError( ERR( tr( "Could not insert provider into layer pipe (provider: %1, URI: %2)" ).arg( mProviderKey, mDataSource ) ) );
    return;
  }

  if ( !mDataProvider->isValid() )
  {
    setError( mDataProvider->error() );
    appendError( ERR( tr( "Provider is not valid (provider: %1, URI: %2)" ).arg( mProviderKey, mDataSource ) ) );
    return;
  }

  if ( mDataProvider->providerCapabilities() & Qgis::RasterProviderCapability::ReadLayerMetadata )
  {
    setMetadata( mDataProvider->layerMetadata() );
    QgsDebugMsgLevel( u"Set Data provider QgsLayerMetadata identifier[%1]"_s.arg( metadata().identifier() ), 4 );
  }

  if ( provider == "gdal"_L1 )
  {
    // make sure that the /vsigzip or /vsizip is added to uri, if applicable
    mDataSource = mDataProvider->dataSourceUri();
  }

  if ( !( flags & Qgis::DataProviderReadFlag::SkipGetExtent ) )
  {
    // get the extent
    const QgsRectangle mbr = mDataProvider->extent();

    // store the extent
    setExtent( mbr );
  }

  // upper case the first letter of the layer name
  QgsDebugMsgLevel( "mLayerName: " + name(), 4 );

  // set up the raster drawing style
  // Do not set any 'sensible' style here, the style is set later

  // Setup source CRS
  setCrs( QgsCoordinateReferenceSystem( mDataProvider->crs() ) );

  QgsDebugMsgLevel( "using wkt:\n" + crs().toWkt( Qgis::CrsWktVariant::Preferred ), 4 );

  //defaults - Needs to be set after the Contrast list has been build
  //Try to read the default contrast enhancement from the config file

  //decide what type of layer this is...
  //TODO Change this to look at the color interp and palette interp to decide which type of layer it is
  QgsDebugMsgLevel( "bandCount = " + QString::number( mDataProvider->bandCount() ), 4 );
  QgsDebugMsgLevel( "dataType = " + qgsEnumValueToKey< Qgis::DataType >( mDataProvider->dataType( 1 ) ), 4 );
  const int bandCount = mDataProvider->bandCount();
  if ( bandCount > 2 )
  {
    mRasterType = Qgis::RasterLayerType::MultiBand;
  }
  else if ( bandCount == 2 )
  {
    // handle singleband gray and paletted with alpha
    auto colorInterpretationIsGrayOrUndefined = []( Qgis::RasterColorInterpretation interpretation )
    {
      return interpretation == Qgis::RasterColorInterpretation::GrayIndex
             || interpretation == Qgis::RasterColorInterpretation::Undefined;
    };

    auto colorInterpretationIsPaletted = []( Qgis::RasterColorInterpretation interpretation )
    {
      return interpretation == Qgis::RasterColorInterpretation::PaletteIndex
             || interpretation == Qgis::RasterColorInterpretation::ContinuousPalette;
    };

    if ( ( colorInterpretationIsGrayOrUndefined( mDataProvider->colorInterpretation( 1 ) ) && mDataProvider->colorInterpretation( 2 ) == Qgis::RasterColorInterpretation::AlphaBand )
         || ( mDataProvider->colorInterpretation( 1 ) == Qgis::RasterColorInterpretation::AlphaBand && colorInterpretationIsGrayOrUndefined( mDataProvider->colorInterpretation( 2 ) ) ) )
    {
      mRasterType = Qgis::RasterLayerType::GrayOrUndefined;
    }
    else if ( ( colorInterpretationIsPaletted( mDataProvider->colorInterpretation( 1 ) ) && mDataProvider->colorInterpretation( 2 ) == Qgis::RasterColorInterpretation::AlphaBand )
              || ( mDataProvider->colorInterpretation( 1 ) == Qgis::RasterColorInterpretation::AlphaBand && colorInterpretationIsPaletted( mDataProvider->colorInterpretation( 2 ) ) ) )
    {
      mRasterType = Qgis::RasterLayerType::Palette;
    }
    else
    {
      mRasterType = Qgis::RasterLayerType::MultiBand;
    }
  }
  else if ( mDataProvider->dataType( 1 ) == Qgis::DataType::ARGB32
            ||  mDataProvider->dataType( 1 ) == Qgis::DataType::ARGB32_Premultiplied )
  {
    mRasterType = Qgis::RasterLayerType::SingleBandColorData;
  }
  else if ( mDataProvider->colorInterpretation( 1 ) == Qgis::RasterColorInterpretation::PaletteIndex
            || mDataProvider->colorInterpretation( 1 ) == Qgis::RasterColorInterpretation::ContinuousPalette )
  {
    mRasterType = Qgis::RasterLayerType::Palette;
  }
  else
  {
    mRasterType = Qgis::RasterLayerType::GrayOrUndefined;
  }

  QgsDebugMsgLevel( "mRasterType = " + qgsEnumValueToKey( mRasterType ), 4 );

  // Raster Attribute Table logic to load from provider or same basename + vat.dbf file.
  QString errorMessage;
  bool hasRat { mDataProvider->readNativeAttributeTable( &errorMessage ) };
  if ( ! hasRat )
  {
    errorMessage.clear();
    QgsDebugMsgLevel( "Native Raster Raster Attribute Table read failed " + errorMessage, 2 );
    if ( QFile::exists( mDataProvider->dataSourceUri( ) +  ".vat.dbf" ) )
    {
      auto rat = std::make_unique<QgsRasterAttributeTable>();
      hasRat = rat->readFromFile( mDataProvider->dataSourceUri( ) +  ".vat.dbf", &errorMessage );
      if ( hasRat )
      {
        if ( rat->isValid( &errorMessage ) )
        {
          mDataProvider->setAttributeTable( 1, rat.release() );
          QgsDebugMsgLevel( "DBF Raster Attribute Table successfully read from " + mDataProvider->dataSourceUri( ) +  ".vat.dbf", 2 );
        }
        else
        {
          QgsDebugMsgLevel( "DBF Raster Attribute Table is not valid, skipping: " + errorMessage, 2 );
        }
      }
      else
      {
        QgsDebugMsgLevel( "DBF Raster Attribute Table read failed " + errorMessage, 2 );
      }
    }
  }
  else
  {
    QgsDebugMsgLevel( "Native Raster Attribute Table read success", 2 );
  }

  switch ( mRasterType )
  {
    case Qgis::RasterLayerType::SingleBandColorData:
    {
      QgsDebugMsgLevel( "Setting drawing style to SingleBandColorDataStyle " + qgsEnumValueToKey( Qgis::RasterDrawingStyle::SingleBandColorData ), 4 );
      setRendererForDrawingStyle( Qgis::RasterDrawingStyle::SingleBandColorData );
      break;
    }
    case Qgis::RasterLayerType::Palette:
    {
      if ( mDataProvider->colorInterpretation( 1 ) == Qgis::RasterColorInterpretation::PaletteIndex )
      {
        setRendererForDrawingStyle( Qgis::RasterDrawingStyle::PalettedColor ); //sensible default
      }
      else if ( mDataProvider->colorInterpretation( 1 ) == Qgis::RasterColorInterpretation::ContinuousPalette )
      {
        setRendererForDrawingStyle( Qgis::RasterDrawingStyle::SingleBandPseudoColor );
        // Load color table
        const QList<QgsColorRampShader::ColorRampItem> colorTable = mDataProvider->colorTable( 1 );
        QgsSingleBandPseudoColorRenderer *r = dynamic_cast<QgsSingleBandPseudoColorRenderer *>( renderer() );
        if ( r )
        {
          // TODO: this should go somewhere else
          QgsRasterShader *shader = new QgsRasterShader();
          QgsColorRampShader *colorRampShader = new QgsColorRampShader();
          colorRampShader->setColorRampType( Qgis::ShaderInterpolationMethod::Linear );
          colorRampShader->setColorRampItemList( colorTable );
          shader->setRasterShaderFunction( colorRampShader );
          r->setShader( shader );
        }
      }
      else
      {
        setRendererForDrawingStyle( Qgis::RasterDrawingStyle::SingleBandGray );  //sensible default
      }
      break;
    }
    case Qgis::RasterLayerType::MultiBand:
    {
      setRendererForDrawingStyle( Qgis::RasterDrawingStyle::MultiBandColor );  //sensible default
      break;
    }
    case Qgis::RasterLayerType::GrayOrUndefined:
    {
      setRendererForDrawingStyle( Qgis::RasterDrawingStyle::SingleBandGray );  //sensible default
      break;
    }
  }

  // Auto set alpha band
  for ( int bandNo = 1; bandNo <= mDataProvider->bandCount(); bandNo++ )
  {
    if ( mDataProvider->colorInterpretation( bandNo ) == Qgis::RasterColorInterpretation::AlphaBand )
    {
      if ( auto *lRenderer = mPipe->renderer() )
      {
        lRenderer->setAlphaBand( bandNo );
      }
      break;
    }
  }

  // brightness filter
  QgsBrightnessContrastFilter *brightnessFilter = new QgsBrightnessContrastFilter();
  mPipe->set( brightnessFilter );

  // hue/saturation filter
  QgsHueSaturationFilter *hueSaturationFilter = new QgsHueSaturationFilter();
  mPipe->set( hueSaturationFilter );

  // resampler (must be after renderer)
  QgsRasterResampleFilter *resampleFilter = new QgsRasterResampleFilter();
  if ( mPipe->set( resampleFilter ) && mDataProvider->providerCapabilities() & Qgis::RasterProviderCapability::ProviderHintBenefitsFromResampling )
  {
    const QgsSettings settings;
    QString resampling = settings.value( u"/Raster/defaultZoomedInResampling"_s, u"nearest neighbour"_s ).toString();
    if ( resampling == "bilinear"_L1 )
    {
      resampleFilter->setZoomedInResampler( new QgsBilinearRasterResampler() );
      mDataProvider->setZoomedInResamplingMethod( Qgis::RasterResamplingMethod::Bilinear );
    }
    else if ( resampling == "cubic"_L1 )
    {
      resampleFilter->setZoomedInResampler( new QgsCubicRasterResampler() );
      mDataProvider->setZoomedInResamplingMethod( Qgis::RasterResamplingMethod::Cubic );
    }
    resampling = settings.value( u"/Raster/defaultZoomedOutResampling"_s, u"nearest neighbour"_s ).toString();
    if ( resampling == "bilinear"_L1 )
    {
      resampleFilter->setZoomedOutResampler( new QgsBilinearRasterResampler() );
      mDataProvider->setZoomedOutResamplingMethod( Qgis::RasterResamplingMethod::Bilinear );
    }

    const double maxOversampling = QgsRasterLayer::settingsRasterDefaultOversampling->value();
    resampleFilter->setMaxOversampling( maxOversampling );
    mDataProvider->setMaxOversampling( maxOversampling );

    if ( ( mDataProvider->providerCapabilities() & Qgis::RasterProviderCapability::ProviderHintCanPerformProviderResampling ) &&
         QgsRasterLayer::settingsRasterDefaultEarlyResampling->value() )
    {
      setResamplingStage( Qgis::RasterResamplingStage::Provider );
    }
    else
    {
      setResamplingStage( Qgis::RasterResamplingStage::ResampleFilter );
    }
  }

  // projector (may be anywhere in pipe)
  QgsRasterProjector *projector = new QgsRasterProjector;
  mPipe->set( projector );

  // Set default identify format - use the richest format available
  const Qgis::RasterInterfaceCapabilities capabilities = mDataProvider->capabilities();
  Qgis::RasterIdentifyFormat identifyFormat = Qgis::RasterIdentifyFormat::Undefined;
  if ( capabilities & Qgis::RasterInterfaceCapability::IdentifyHtml )
  {
    // HTML is usually richest
    identifyFormat = Qgis::RasterIdentifyFormat::Html;
  }
  else if ( capabilities & Qgis::RasterInterfaceCapability::IdentifyFeature )
  {
    identifyFormat = Qgis::RasterIdentifyFormat::Feature;
  }
  else if ( capabilities & Qgis::RasterInterfaceCapability::IdentifyText )
  {
    identifyFormat = Qgis::RasterIdentifyFormat::Text;
  }
  else if ( capabilities & Qgis::RasterInterfaceCapability::IdentifyValue )
  {
    identifyFormat = Qgis::RasterIdentifyFormat::Value;
  }
  setCustomProperty( u"identify/format"_s, QgsRasterDataProvider::identifyFormatName( identifyFormat ) );

  if ( QgsRasterDataProviderElevationProperties *properties = mDataProvider->elevationProperties() )
  {
    if ( properties->containsElevationData() )
    {
      mElevationProperties->setEnabled( true );
    }
  }

  // Store timestamp
  // TODO move to provider
  mLastModified = lastModified( mDataSource );

  // Do a passthrough for the status bar text
  connect( mDataProvider, &QgsRasterDataProvider::statusChanged, this, &QgsRasterLayer::statusChanged );

  //mark the layer as valid
  setValid( true );

  if ( mDataProvider->supportsSubsetString() )
    connect( this, &QgsRasterLayer::subsetStringChanged, this, &QgsMapLayer::configChanged, Qt::UniqueConnection );
  else
    disconnect( this, &QgsRasterLayer::subsetStringChanged, this, &QgsMapLayer::configChanged );


  QgsDebugMsgLevel( u"exiting."_s, 4 );

}

void QgsRasterLayer::setDataSourcePrivate( const QString &dataSource, const QString &baseName, const QString &provider,
    const QgsDataProvider::ProviderOptions &options, Qgis::DataProviderReadFlags flags )
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  const bool hadRenderer( renderer() );

  QDomImplementation domImplementation;
  QDomDocumentType documentType;
  QString errorMsg;

  bool loadDefaultStyleFlag = false;
  if ( flags & Qgis::DataProviderReadFlag::LoadDefaultStyle )
  {
    loadDefaultStyleFlag = true;
  }

  // Store the original style
  if ( hadRenderer && ! loadDefaultStyleFlag )
  {
    documentType = domImplementation.createDocumentType(
                     u"qgis"_s, u"http://mrcc.com/qgis.dtd"_s, u"SYSTEM"_s );

    QDomDocument doc = QDomDocument( documentType );
    QDomElement styleElem = doc.createElement( u"qgis"_s );
    styleElem.setAttribute( u"version"_s, Qgis::version() );
    const QgsReadWriteContext writeContext;
    if ( ! writeSymbology( styleElem, doc, errorMsg, writeContext ) )
    {
      QgsDebugError( u"Could not store symbology for layer %1: %2"_s
                     .arg( name(),
                           errorMsg ) );
    }
    else
    {
      doc.appendChild( styleElem );

      mOriginalStyleDocument = doc;
      mOriginalStyleElement = styleElem;
    }
  }

  if ( mDataProvider )
    closeDataProvider();

  init();

  for ( int i = mPipe->size() - 1; i >= 0; --i )
  {
    mPipe->remove( i );
  }

  mDataSource = dataSource;
  mLayerName = baseName;

  setDataProvider( provider, options, flags );

  if ( mDataProvider )
    mDataProvider->setDataSourceUri( mDataSource );

  if ( isValid() )
  {
    // load default style
    bool defaultLoadedFlag = false;
    bool restoredStyle = false;
    if ( loadDefaultStyleFlag )
    {
      loadDefaultStyle( defaultLoadedFlag );
    }
    else if ( !mOriginalStyleElement.isNull() )  // Restore the style
    {
      QgsReadWriteContext readContext;
      if ( ! readSymbology( mOriginalStyleElement, errorMsg, readContext ) )
      {
        QgsDebugError( u"Could not restore symbology for layer %1: %2"_s
                       .arg( name() )
                       .arg( errorMsg ) );

      }
      else
      {
        restoredStyle = true;
        emit repaintRequested();
        emitStyleChanged();
        emit rendererChanged();
      }
    }

    if ( !defaultLoadedFlag && !restoredStyle )
    {
      setDefaultContrastEnhancement();
    }
  }
}

void QgsRasterLayer::writeRasterAttributeTableExternalPaths( QDomNode &layerNode, QDomDocument &doc, const QgsReadWriteContext &context ) const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  if ( attributeTableCount() > 0 )
  {
    QDomElement elem = doc.createElement( u"FileBasedAttributeTables"_s );
    for ( int bandNo = 1; bandNo <= bandCount(); bandNo++ )
    {
      if ( QgsRasterAttributeTable *rat = attributeTable( bandNo ); rat && ! rat->filePath().isEmpty() )
      {
        QDomElement ratElem = doc.createElement( u"AttributeTable"_s );
        ratElem.setAttribute( u"band"_s, bandNo );
        ratElem.setAttribute( u"path"_s, context.pathResolver().writePath( rat->filePath( ) ) );
        elem.appendChild( ratElem );
      }
    }
    layerNode.appendChild( elem );
  }
}

void QgsRasterLayer::readRasterAttributeTableExternalPaths( const QDomNode &layerNode, QgsReadWriteContext &context ) const
{
  const QDomElement ratsElement = layerNode.firstChildElement( u"FileBasedAttributeTables"_s );
  if ( !ratsElement.isNull() && ratsElement.childNodes().count() > 0 )
  {
    const QDomNodeList ratElements { ratsElement.childNodes() };
    for ( int idx = 0; idx < ratElements.count(); idx++ )
    {
      const QDomNode ratElement { ratElements.at( idx ) };
      if ( ratElement.attributes().contains( u"band"_s )
           && ratElement.attributes().contains( u"path"_s ) )
      {
        bool ok;
        const int band { ratElement.attributes().namedItem( u"band"_s ).nodeValue().toInt( &ok ) };

        // Check band is ok
        if ( ! ok ||  band <= 0 || band > bandCount() )
        {
          QgsMessageLog::logMessage( tr( "Error reading raster attribute table: invalid band %1." ).arg( band ), tr( "Raster" ) );
          continue;
        }

        const QString path { context.pathResolver().readPath( ratElement.attributes().namedItem( u"path"_s ).nodeValue() ) };
        if ( ! QFile::exists( path ) )
        {
          QgsMessageLog::logMessage( tr( "Error loading raster attribute table, file not found: %1." ).arg( path ), tr( "Raster" ) );
          continue;
        }

        auto rat = std::make_unique<QgsRasterAttributeTable>();
        QString errorMessage;
        if ( ! rat->readFromFile( path, &errorMessage ) )
        {
          QgsMessageLog::logMessage( tr( "Error loading raster attribute table from path %1: %2" ).arg( path, errorMessage ), tr( "Raster" ) );
          continue;
        }

        // All good, set the RAT
        mDataProvider->setAttributeTable( band, rat.release() );
      }
    }
  }
}

void QgsRasterLayer::closeDataProvider()
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  setValid( false );
  mPipe->remove( mDataProvider );
  mDataProvider = nullptr;
}

void QgsRasterLayer::computeMinMax( int band,
                                    const QgsRasterMinMaxOrigin &mmo,
                                    Qgis::RasterRangeLimit limits,
                                    const QgsRectangle &extent,
                                    int sampleSize,
                                    double &min, double &max )
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  min = std::numeric_limits<double>::quiet_NaN();
  max = std::numeric_limits<double>::quiet_NaN();
  if ( !mDataProvider )
    return;

  if ( limits == Qgis::RasterRangeLimit::MinimumMaximum )
  {
    QgsRasterBandStats myRasterBandStats = mDataProvider->bandStatistics( band, Qgis::RasterBandStatistic::Min | Qgis::RasterBandStatistic::Max, extent, sampleSize );
    // Check if statistics were actually gathered, None means a failure
    if ( myRasterBandStats.statsGathered == static_cast< int >( Qgis::RasterBandStatistic::NoStatistic ) )
    {
      // Best guess we can do
      switch ( mDataProvider->dataType( band ) )
      {
        case Qgis::DataType::Byte:
        {
          myRasterBandStats.minimumValue = 0;
          myRasterBandStats.maximumValue = 255;
          break;
        }
        case Qgis::DataType::Int8:
        {
          myRasterBandStats.minimumValue = std::numeric_limits<int8_t>::lowest();
          myRasterBandStats.maximumValue = std::numeric_limits<int8_t>::max();
          break;
        }
        case Qgis::DataType::UInt16:
        {
          myRasterBandStats.minimumValue = 0;
          myRasterBandStats.maximumValue = std::numeric_limits<uint16_t>::max();
          break;
        }
        case Qgis::DataType::UInt32:
        {
          myRasterBandStats.minimumValue = 0;
          myRasterBandStats.maximumValue = std::numeric_limits<uint32_t>::max();
          break;
        }
        case Qgis::DataType::Int16:
        case Qgis::DataType::CInt16:
        {
          myRasterBandStats.minimumValue = std::numeric_limits<int16_t>::lowest();
          myRasterBandStats.maximumValue = std::numeric_limits<int16_t>::max();
          break;
        }
        case Qgis::DataType::Int32:
        case Qgis::DataType::CInt32:
        {
          myRasterBandStats.minimumValue = std::numeric_limits<int32_t>::lowest();
          myRasterBandStats.maximumValue = std::numeric_limits<int32_t>::max();
          break;
        }
        case Qgis::DataType::Float32:
        case Qgis::DataType::CFloat32:
        {
          myRasterBandStats.minimumValue = std::numeric_limits<float_t>::lowest();
          myRasterBandStats.maximumValue = std::numeric_limits<float_t>::max();
          break;
        }
        case Qgis::DataType::Float64:
        case Qgis::DataType::CFloat64:
        {
          myRasterBandStats.minimumValue = std::numeric_limits<double_t>::lowest();
          myRasterBandStats.maximumValue = std::numeric_limits<double_t>::max();
          break;
        }
        case Qgis::DataType::ARGB32:
        case Qgis::DataType::ARGB32_Premultiplied:
        case Qgis::DataType::UnknownDataType:
        {
          // Nothing to guess
          break;
        }
      }
    }
    min = myRasterBandStats.minimumValue;
    max = myRasterBandStats.maximumValue;
  }
  else if ( limits == Qgis::RasterRangeLimit::StdDev )
  {
    const QgsRasterBandStats myRasterBandStats = mDataProvider->bandStatistics( band, Qgis::RasterBandStatistic::Mean | Qgis::RasterBandStatistic::StdDev, extent, sampleSize );
    min = myRasterBandStats.mean - ( mmo.stdDevFactor() * myRasterBandStats.stdDev );
    max = myRasterBandStats.mean + ( mmo.stdDevFactor() * myRasterBandStats.stdDev );
  }
  else if ( limits == Qgis::RasterRangeLimit::CumulativeCut )
  {
    const double myLower = mmo.cumulativeCutLower();
    const double myUpper = mmo.cumulativeCutUpper();
    QgsDebugMsgLevel( u"myLower = %1 myUpper = %2"_s.arg( myLower ).arg( myUpper ), 4 );
    mDataProvider->cumulativeCut( band, myLower, myUpper, min, max, extent, sampleSize );
  }
  QgsDebugMsgLevel( u"band = %1 min = %2 max = %3"_s.arg( band ).arg( min ).arg( max ), 4 );

}

bool QgsRasterLayer::ignoreExtents() const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  return mDataProvider ? mDataProvider->ignoreExtents() : false;
}

QgsMapLayerTemporalProperties *QgsRasterLayer::temporalProperties()
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  return mTemporalProperties;
}

QgsMapLayerElevationProperties *QgsRasterLayer::elevationProperties()
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  return mElevationProperties;
}

void QgsRasterLayer::setContrastEnhancement( QgsContrastEnhancement::ContrastEnhancementAlgorithm algorithm, Qgis::RasterRangeLimit limits, const QgsRectangle &extent, int sampleSize, bool generateLookupTableFlag )
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  setContrastEnhancement( algorithm,
                          limits,
                          extent,
                          sampleSize,
                          generateLookupTableFlag,
                          mPipe->renderer() );
}

void QgsRasterLayer::setContrastEnhancement( QgsContrastEnhancement::ContrastEnhancementAlgorithm algorithm,
    Qgis::RasterRangeLimit limits,
    const QgsRectangle &extent,
    int sampleSize,
    bool generateLookupTableFlag,
    QgsRasterRenderer *rasterRenderer )
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  QgsDebugMsgLevel( u"theAlgorithm = %1 limits = %2 extent.isEmpty() = %3"_s.arg( algorithm ).arg( qgsEnumValueToKey( limits ) ).arg( extent.isEmpty() ), 4 );
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
  const QString rendererType  = rasterRenderer->type();
  if ( rendererType == "singlebandgray"_L1 )
  {
    myGrayRenderer = dynamic_cast<QgsSingleBandGrayRenderer *>( rasterRenderer );
    if ( !myGrayRenderer )
    {
      return;
    }
    myBands << myGrayRenderer->inputBand();
    myRasterRenderer = myGrayRenderer;
    myMinMaxOrigin = myGrayRenderer->minMaxOrigin();
  }
  else if ( rendererType == "multibandcolor"_L1 )
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
  else if ( rendererType == "singlebandpseudocolor"_L1 )
  {
    myPseudoColorRenderer = dynamic_cast<QgsSingleBandPseudoColorRenderer *>( rasterRenderer );
    if ( !myPseudoColorRenderer )
    {
      return;
    }
    myBands << myPseudoColorRenderer->inputBand();
    myRasterRenderer = myPseudoColorRenderer;
    myMinMaxOrigin = myPseudoColorRenderer->minMaxOrigin();
  }
  else
  {
    return;
  }

  for ( const int myBand : std::as_const( myBands ) )
  {
    if ( myBand != -1 )
    {
      const Qgis::DataType myType = static_cast< Qgis::DataType >( mDataProvider->dataType( myBand ) );
      auto myEnhancement = std::make_unique<QgsContrastEnhancement>( static_cast< Qgis::DataType >( myType ) );
      myEnhancement->setContrastEnhancementAlgorithm( algorithm, generateLookupTableFlag );

      double min;
      double max;
      computeMinMax( myBand, myMinMaxOrigin, limits, extent, sampleSize, min, max );

      if ( rendererType == "singlebandpseudocolor"_L1 )
      {
        // This is not necessary, but clang-tidy thinks it is.
        if ( ! myPseudoColorRenderer )
        {
          return;
        }
        // Do not overwrite min/max with NaN if they were already set,
        // for example when the style was already loaded from a raster attribute table
        // in that case we need to respect the style from the attribute table and do
        // not perform any reclassification.
        bool minMaxChanged { false };
        if ( ! std::isnan( min ) )
        {
          myPseudoColorRenderer->setClassificationMin( min );
          minMaxChanged = true;
        }

        if ( ! std::isnan( max ) )
        {
          myPseudoColorRenderer->setClassificationMax( max );
          minMaxChanged = true;
        }

        if ( minMaxChanged && myPseudoColorRenderer->shader() )
        {
          QgsColorRampShader *colorRampShader = dynamic_cast<QgsColorRampShader *>( myPseudoColorRenderer->shader()->rasterShaderFunction() );
          if ( colorRampShader )
          {
            colorRampShader->classifyColorRamp( myPseudoColorRenderer->inputBand(), extent, myPseudoColorRenderer->input() );
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

  // Again, second check is redundant but clang-tidy doesn't agree
  if ( rendererType == "singlebandgray"_L1 && myGrayRenderer )
  {
    if ( myEnhancements.first() ) myGrayRenderer->setContrastEnhancement( myEnhancements.takeFirst() );
  }
  else if ( rendererType == "multibandcolor"_L1 && myMultiBandRenderer )
  {
    if ( myEnhancements.first() ) myMultiBandRenderer->setRedContrastEnhancement( myEnhancements.takeFirst() );
    if ( myEnhancements.first() ) myMultiBandRenderer->setGreenContrastEnhancement( myEnhancements.takeFirst() );
    if ( myEnhancements.first() ) myMultiBandRenderer->setBlueContrastEnhancement( myEnhancements.takeFirst() );
  }

  //delete all remaining unused enhancements
  qDeleteAll( myEnhancements );

  myMinMaxOrigin.setLimits( limits );
  if ( extent != QgsRectangle() &&
       myMinMaxOrigin.extent() == Qgis::RasterRangeExtent::WholeRaster )
  {
    myMinMaxOrigin.setExtent( Qgis::RasterRangeExtent::FixedCanvas );
  }
  if ( myRasterRenderer )
  {
    myRasterRenderer->setMinMaxOrigin( myMinMaxOrigin );
  }

  if ( rasterRenderer == renderer() )
  {
    emit repaintRequested();
    emitStyleChanged();
    emit rendererChanged();
  }
}

void QgsRasterLayer::refreshContrastEnhancement( const QgsRectangle &extent )
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

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
                            renderer()->minMaxOrigin().limits() == Qgis::RasterRangeLimit::NotSet ?
                            Qgis::RasterRangeLimit::MinimumMaximum : renderer()->minMaxOrigin().limits(),
                            extent,
                            static_cast<int>( SAMPLE_SIZE ),
                            true,
                            renderer() );
  }
  else
  {
    QgsContrastEnhancement::ContrastEnhancementAlgorithm myAlgorithm;
    Qgis::RasterRangeLimit myLimits;
    if ( defaultContrastEnhancementSettings( myAlgorithm, myLimits ) )
    {
      setContrastEnhancement( QgsContrastEnhancement::StretchToMinimumMaximum,
                              myLimits,
                              extent,
                              static_cast<int>( SAMPLE_SIZE ),
                              true,
                              renderer() );
    }
  }
}

QString QgsRasterLayer::subsetString() const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  if ( !isValid() || !mDataProvider )
  {
    QgsDebugMsgLevel( u"invoked with invalid layer or null mDataProvider"_s, 3 );
    return customProperty( u"storedSubsetString"_s ).toString();
  }
  if ( !mDataProvider->supportsSubsetString() )
  {
    return QString();
  }
  return mDataProvider->subsetString();
}

bool QgsRasterLayer::setSubsetString( const QString &subset )
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  if ( !isValid() || !mDataProvider )
  {
    QgsDebugMsgLevel( u"invoked with invalid layer or null mDataProvider or while editing"_s, 3 );
    setCustomProperty( u"storedSubsetString"_s, subset );
    return false;
  }

  if ( !mDataProvider->supportsSubsetString() )
  {
    return false;
  }

  if ( subset == mDataProvider->subsetString() )
    return true;

  const bool res = mDataProvider->setSubsetString( subset );

  // get the updated data source string from the provider
  mDataSource = mDataProvider->dataSourceUri();

  if ( res )
  {
    setExtent( mDataProvider->extent() );
    QList<double> minValues;
    QList<double> maxValues;
    const QgsRasterMinMaxOrigin &minMaxOrigin = renderer()->minMaxOrigin();
    for ( const int bandIdx : renderer()->usesBands() )
    {
      double min;
      double max;

      computeMinMax( bandIdx, minMaxOrigin, minMaxOrigin.limits(),
                     extent(), static_cast<int>( QgsRasterLayer::SAMPLE_SIZE ),
                     min, max );
      minValues.append( min );
      maxValues.append( max );
    }
    renderer()->refresh( extent(), minValues, maxValues, true );
    emit subsetStringChanged();
  }

  return res;
}

bool QgsRasterLayer::defaultContrastEnhancementSettings(
  QgsContrastEnhancement::ContrastEnhancementAlgorithm &myAlgorithm,
  Qgis::RasterRangeLimit &myLimits ) const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  const QgsSettings mySettings;

  QString key;
  QString defaultAlg;
  QString defaultLimits;

  // TODO: we should not test renderer class here, move it somehow to renderers
  if ( dynamic_cast<QgsSingleBandGrayRenderer *>( renderer() ) )
  {
    key = u"singleBand"_s;
    defaultAlg = QgsContrastEnhancement::contrastEnhancementAlgorithmString(
                   SINGLE_BAND_ENHANCEMENT_ALGORITHM );
    defaultLimits = QgsRasterMinMaxOrigin::limitsString(
                      SINGLE_BAND_MIN_MAX_LIMITS );
  }
  else if ( dynamic_cast<QgsMultiBandColorRenderer *>( renderer() ) )
  {
    if ( QgsRasterBlock::typeSize( dataProvider()->sourceDataType( 1 ) ) == 1 )
    {
      key = u"multiBandSingleByte"_s;
      defaultAlg = QgsContrastEnhancement::contrastEnhancementAlgorithmString(
                     MULTIPLE_BAND_SINGLE_BYTE_ENHANCEMENT_ALGORITHM );
      defaultLimits = QgsRasterMinMaxOrigin::limitsString(
                        MULTIPLE_BAND_SINGLE_BYTE_MIN_MAX_LIMITS );
    }
    else
    {
      key = u"multiBandMultiByte"_s;
      defaultAlg = QgsContrastEnhancement::contrastEnhancementAlgorithmString(
                     MULTIPLE_BAND_MULTI_BYTE_ENHANCEMENT_ALGORITHM );
      defaultLimits = QgsRasterMinMaxOrigin::limitsString(
                        MULTIPLE_BAND_MULTI_BYTE_MIN_MAX_LIMITS );
    }
  }

  if ( key.isEmpty() )
  {
    QgsDebugMsgLevel( u"No default contrast enhancement for this drawing style"_s, 2 );
    myAlgorithm = QgsContrastEnhancement::contrastEnhancementAlgorithmFromString( QString() );
    myLimits = QgsRasterMinMaxOrigin::limitsFromString( QString() );
    return false;
  }
  QgsDebugMsgLevel( "key = " + key, 4 );

  const QString myAlgorithmString = mySettings.value( "/Raster/defaultContrastEnhancementAlgorithm/" + key, defaultAlg ).toString();
  QgsDebugMsgLevel( "myAlgorithmString = " + myAlgorithmString, 4 );

  myAlgorithm = QgsContrastEnhancement::contrastEnhancementAlgorithmFromString( myAlgorithmString );

  const QString myLimitsString = mySettings.value( "/Raster/defaultContrastEnhancementLimits/" + key, defaultLimits ).toString();
  QgsDebugMsgLevel( "myLimitsString = " + myLimitsString, 4 );
  myLimits = QgsRasterMinMaxOrigin::limitsFromString( myLimitsString );

  return true;
}

void QgsRasterLayer::setDefaultContrastEnhancement()
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  QgsDebugMsgLevel( u"Entered"_s, 4 );

  QgsContrastEnhancement::ContrastEnhancementAlgorithm myAlgorithm;
  Qgis::RasterRangeLimit myLimits;
  defaultContrastEnhancementSettings( myAlgorithm, myLimits );

  setContrastEnhancement( myAlgorithm, myLimits );
}

void QgsRasterLayer::setLayerOrder( QStringList const &layers )
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  QgsDebugMsgLevel( u"entered."_s, 4 );

  if ( mDataProvider )
  {
    QgsDebugMsgLevel( u"About to mDataProvider->setLayerOrder(layers)."_s, 4 );
    mDataProvider->setLayerOrder( layers );
  }

}

void QgsRasterLayer::setSubLayerVisibility( const QString &name, bool vis )
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  if ( mDataProvider )
  {
    QgsDebugMsgLevel( u"About to mDataProvider->setSubLayerVisibility(name, vis)."_s, 4 );
    mDataProvider->setSubLayerVisibility( name, vis );
  }
}

QDateTime QgsRasterLayer::timestamp() const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  if ( !mDataProvider )
    return QDateTime();
  return mDataProvider->timestamp();
}

bool QgsRasterLayer::accept( QgsStyleEntityVisitorInterface *visitor ) const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  if ( auto *lRenderer = mPipe->renderer() )
  {
    if ( !lRenderer->accept( visitor ) )
      return false;
  }
  return true;
}

bool QgsRasterLayer::labelsEnabled() const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  return mLabelsEnabled && static_cast< bool >( mLabeling );
}

void QgsRasterLayer::setLabelsEnabled( bool enabled )
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  mLabelsEnabled = enabled;
}

const QgsAbstractRasterLayerLabeling *QgsRasterLayer::labeling() const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  return mLabeling.get();
}

QgsAbstractRasterLayerLabeling *QgsRasterLayer::labeling()
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  return mLabeling.get();
}

void QgsRasterLayer::setLabeling( QgsAbstractRasterLayerLabeling *labeling )
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  if ( mLabeling.get() == labeling )
    return;

  mLabeling.reset( labeling );
  triggerRepaint();
}

bool QgsRasterLayer::writeSld( QDomNode &node, QDomDocument &doc, QString &, const QVariantMap &props ) const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS
  QgsSldExportContext context;
  context.setExtraProperties( props );
  writeSld( node, doc, context );
  return true;
}

bool QgsRasterLayer::writeSld( QDomNode &node, QDomDocument &doc, QgsSldExportContext &context ) const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  QVariantMap localProps = context.extraProperties();
  if ( hasScaleBasedVisibility() )
  {
    // TODO: QgsSymbolLayerUtils::mergeScaleDependencies generate SE only and not SLD1.0
    QgsSymbolLayerUtils::mergeScaleDependencies( maximumScale(), minimumScale(), localProps );
  }
  context.setExtraProperties( localProps );

  if ( isSpatial() ) // TODO: does it make sense this control?
  {
    // store constraints
    QDomElement constraintElem = doc.createElement( u"sld:LayerFeatureConstraints"_s );
    node.appendChild( constraintElem );

    const QDomElement featureTypeConstraintElem = doc.createElement( u"sld:FeatureTypeConstraint"_s );
    constraintElem.appendChild( featureTypeConstraintElem );

    QDomElement userStyleElem = doc.createElement( u"sld:UserStyle"_s );
    node.appendChild( userStyleElem );

    if ( !name().isEmpty() )
    {
      QDomElement nameElem = doc.createElement( u"sld:Name"_s );
      nameElem.appendChild( doc.createTextNode( name() ) );
      userStyleElem.appendChild( nameElem );
    }

    const QString abstract = !metadata().abstract().isEmpty() ? metadata().abstract() : serverProperties()->abstract();
    if ( !abstract.isEmpty() )
    {
      QDomElement abstractElem = doc.createElement( u"sld:Abstract"_s );
      abstractElem.appendChild( doc.createTextNode( abstract ) );
      userStyleElem.appendChild( abstractElem );
    }

    const QString title = !metadata().title().isEmpty() ? metadata().title() : serverProperties()->title();
    if ( !title.isEmpty() )
    {
      QDomElement titleElem = doc.createElement( u"sld:Title"_s );
      titleElem.appendChild( doc.createTextNode( title ) );
      userStyleElem.appendChild( titleElem );
    }

    QDomElement featureTypeStyleElem = doc.createElement( u"sld:FeatureTypeStyle"_s );
    userStyleElem.appendChild( featureTypeStyleElem );

#if 0
    // TODO: Is there a way to fill it's value with the named style?
    // by default <sld:Name> under <sld:FeatureTypeStyle> can have 0 occurrences
    // the same happen for tags:
    // sld:Title
    // sld:Abstract
    // sld:FeatureTypeName
    // sld:SemanticTypeIdentifier
    QDomElement typeStyleNameElem = doc.createElement( u"sld:Name"_s );
    featureTypeStyleElem.appendChild( typeStyleNameElem );
#endif

    QDomElement typeStyleRuleElem = doc.createElement( u"sld:Rule"_s );
    featureTypeStyleElem.appendChild( typeStyleRuleElem );

    // add ScaleDenominator tags
    if ( hasScaleBasedVisibility() )
    {
      // note that denominator is the inverted value of scale
      if ( maximumScale() != 0.0 )
      {
        QDomElement minScaleElem = doc.createElement( u"sld:MinScaleDenominator"_s );
        minScaleElem.appendChild( doc.createTextNode( QString::number( maximumScale() ) ) );
        typeStyleRuleElem.appendChild( minScaleElem );
      }

      QDomElement maxScaleElem = doc.createElement( u"sld:MaxScaleDenominator"_s );
      maxScaleElem.appendChild( doc.createTextNode( QString::number( minimumScale() ) ) );
      typeStyleRuleElem.appendChild( maxScaleElem );
    }

    // export renderer dependent tags
    mPipe->renderer()->toSld( doc, typeStyleRuleElem, context );

    // inject raster layer parameters in RasterSymbolizer tag because
    // they belongs to rasterlayer and not to the renderer => avoid to
    // pass many parameters value via localProps
    const QDomNodeList elements = typeStyleRuleElem.elementsByTagName( u"sld:RasterSymbolizer"_s );
    if ( elements.size() != 0 )
    {
      // there SHOULD be only one
      QDomElement rasterSymbolizerElem = elements.at( 0 ).toElement();

      // lamda helper used below to reduce code redundancy
      auto vendorOptionWriter = [&]( QString name, QString value )
      {
        QDomElement vendorOptionElem = doc.createElement( u"sld:VendorOption"_s );
        vendorOptionElem.setAttribute( u"name"_s, name );
        vendorOptionElem.appendChild( doc.createTextNode( value ) );
        rasterSymbolizerElem.appendChild( vendorOptionElem );
      };

      if ( hueSaturationFilter()->invertColors() )
      {
        vendorOptionWriter( u"invertColors"_s, QString::number( 1 ) );
      }

      // add greyScale rendering mode if set
      if ( hueSaturationFilter()->grayscaleMode() != QgsHueSaturationFilter::GrayscaleOff )
      {
        QString property;
        switch ( hueSaturationFilter()->grayscaleMode() )
        {
          case QgsHueSaturationFilter::GrayscaleLightness:
            property = u"lightness"_s;
            break;
          case QgsHueSaturationFilter::GrayscaleLuminosity:
            property = u"luminosity"_s;
            break;
          case QgsHueSaturationFilter::GrayscaleAverage:
            property = u"average"_s;
            break;
          case QgsHueSaturationFilter::GrayscaleOff:
            // added just to avoid travis fail
            break;
        }
        if ( !property.isEmpty() )
          vendorOptionWriter( u"grayScale"_s, property );
      }

      // add Hue, Saturation and Lighting values in props is Hue filter is set
      if ( hueSaturationFilter() && hueSaturationFilter()->colorizeOn() )
      {
        vendorOptionWriter( u"colorizeOn"_s, QString::number( hueSaturationFilter()->colorizeOn() ) );
        vendorOptionWriter( u"colorizeRed"_s, QString::number( hueSaturationFilter()->colorizeColor().red() ) );
        vendorOptionWriter( u"colorizeGreen"_s, QString::number( hueSaturationFilter()->colorizeColor().green() ) );
        vendorOptionWriter( u"colorizeBlue"_s, QString::number( hueSaturationFilter()->colorizeColor().blue() ) );
        if ( hueSaturationFilter()->colorizeStrength() != 100.0 )
          vendorOptionWriter( u"colorizeStrength"_s, QString::number( hueSaturationFilter()->colorizeStrength() / 100.0 ) );
        vendorOptionWriter( u"saturation"_s, QString::number( hueSaturationFilter()->colorizeColor().saturationF() ) );
      }
      else
      {
        // saturation != 0 (default value)
        if ( hueSaturationFilter()->saturation() != 0 )
        {
          // normlize value [-100:100] -> [0:1]
          const int s = hueSaturationFilter()->saturation();
          const double sF = ( s - ( -100.0 ) ) / ( 100.0 - ( -100.0 ) );
          vendorOptionWriter( u"saturation"_s, QString::number( sF ) );
        }
      }

      // brightness != 0 (default value)
      if ( brightnessFilter()->brightness() != 0 )
      {
        // normalize value [-255:255] -> [0:1]
        const int b = brightnessFilter()->brightness();
        const double bF = ( b - ( -255.0 ) ) / ( 255.0 - ( -255.0 ) );
        vendorOptionWriter( u"brightness"_s, QString::number( bF ) );
      }

      // contrast != 0 (default value)
      if ( brightnessFilter()->contrast() != 0 )
      {
        // normlize value [-100:100] -> [0:1]
        const int c = brightnessFilter()->contrast();
        const double cF = ( c - ( -100.0 ) ) / ( 100.0 - ( -100.0 ) );
        vendorOptionWriter( u"contrast"_s, QString::number( cF ) );
      }

#if 0
      // TODO: check if the below mapping formula make sense to map QGIS contrast with SLD gamma value
      // add SLD1.0 ContrastEnhancement GammaValue = QGIS Contrast
      // SLD1.0 does only define 1 as neutral/center double value but does not define range.
      // because https://en.wikipedia.org/wiki/Gamma_correction assumed gamma is >0.
      // whilst QGIS has a -100/100 values centered in 0 => QGIS contrast value will be scaled in the
      // following way:
      // [-100,0] => [0,1] and [0,100] => [1,100]
      // an alternative could be scale [-100,100] => (0,2]
      //
      if ( newProps.contains( u"contrast"_s ) )
      {
        double gamma;
        double contrast = newProps[ u"contrast"_s ].toDouble();
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

        QDomElement globalContrastEnhancementElem = doc.createElement( u"sld:ContrastEnhancement"_s );
        rasterSymolizerElem.appendChild( globalContrastEnhancementElem );

        QDomElement gammaValueElem = doc.createElement( u"sld:GammaValue"_s );
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
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  QgsDebugMsgLevel( u"Entered"_s, 4 );
  if ( !renderer )
  {
    return;
  }

  mPipe->set( renderer );
  emit rendererChanged();
  emitStyleChanged();
}

QgsRasterRenderer *QgsRasterLayer::renderer() const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  return mPipe->renderer();
}

QgsRasterResampleFilter *QgsRasterLayer::resampleFilter() const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  return mPipe->resampleFilter();
}

QgsBrightnessContrastFilter *QgsRasterLayer::brightnessFilter() const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  return mPipe->brightnessFilter();
}

QgsHueSaturationFilter *QgsRasterLayer::hueSaturationFilter() const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  return mPipe->hueSaturationFilter();
}

void QgsRasterLayer::showStatusMessage( QString const &message )
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  // QgsDebugMsgLevel(QString("entered with '%1'.").arg(theMessage), 2);

  // Pass-through
  // TODO: See if we can connect signal-to-signal.  This is a kludge according to the Qt doc.
  emit statusChanged( message );
}

void QgsRasterLayer::setTransformContext( const QgsCoordinateTransformContext &transformContext )
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  if ( mDataProvider )
    mDataProvider->setTransformContext( transformContext );
  invalidateWgs84Extent();
}

QStringList QgsRasterLayer::subLayers() const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  if ( ! mDataProvider )
    return QStringList();
  return mDataProvider->subLayers();
}

// this function should be used when rendering with the MTR engine introduced in 2.3, as QPixmap is not thread safe (see bug #9626)
// note: previewAsImage and previewAsPixmap should use a common low-level fct QgsRasterLayer::previewOnPaintDevice( QSize size, QColor bgColor, QPaintDevice &device )
QImage QgsRasterLayer::previewAsImage( QSize size, const QColor &bgColor, QImage::Format format )
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  QImage image( size, format );

  if ( ! isValid( ) )
    return  QImage();

  if ( image.format() == QImage::Format_Indexed8 )
  {
    image.setColor( 0, bgColor.rgba() );
    image.fill( 0 );  //defaults to white, set to transparent for rendering on a map
  }
  else
  {
    image.fill( bgColor );
  }

  QgsRasterViewPort *rasterViewPort = new QgsRasterViewPort();

  double mapUnitsPerPixel;
  double x = 0.0;
  double y = 0.0;
  const QgsRectangle extent = mDataProvider->extent();
  if ( extent.width() / extent.height() >= static_cast< double >( image.width() ) / image.height() )
  {
    mapUnitsPerPixel = extent.width() / image.width();
    y = ( image.height() - extent.height() / mapUnitsPerPixel ) / 2;
  }
  else
  {
    mapUnitsPerPixel = extent.height() / image.height();
    x = ( image.width() - extent.width() / mapUnitsPerPixel ) / 2;
  }

  const double pixelWidth = extent.width() / mapUnitsPerPixel;
  const double pixelHeight = extent.height() / mapUnitsPerPixel;

  rasterViewPort->mTopLeftPoint = QgsPointXY( x, y );
  rasterViewPort->mBottomRightPoint = QgsPointXY( pixelWidth, pixelHeight );
  rasterViewPort->mWidth = image.width();
  rasterViewPort->mHeight = image.height();

  rasterViewPort->mDrawnExtent = extent;
  rasterViewPort->mSrcCRS = QgsCoordinateReferenceSystem(); // will be invalid
  rasterViewPort->mDestCRS = QgsCoordinateReferenceSystem(); // will be invalid

  QgsMapToPixel *mapToPixel = new QgsMapToPixel( mapUnitsPerPixel );

  QPainter *painter = new QPainter( &image );
  draw( painter, rasterViewPort, mapToPixel );
  delete rasterViewPort;
  delete mapToPixel;

  painter->end();
  delete painter;

  return image;
}

bool QgsRasterLayer::readSymbology( const QDomNode &layer_node, QString &errorMessage,
                                    QgsReadWriteContext &context, QgsMapLayer::StyleCategories categories )
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  Q_UNUSED( errorMessage )
  // TODO: implement categories for raster layer

  QDomElement rasterRendererElem;

  const QDomElement layerElement = layer_node.toElement();
  readCommonStyle( layerElement, context, categories );

  // pipe considers Symbology and Rendering components, why it's always handled as one
  if ( categories.testFlag( Symbology ) || categories.testFlag( Rendering ) )
  {
    // pipe element was introduced in the end of 1.9 development when there were
    // already many project files in use so we support 1.9 backward compatibility
    // even it was never officially released -> use pipe element if present, otherwise
    // use layer node
    QDomNode pipeNode = layer_node.firstChildElement( u"pipe"_s );
    if ( pipeNode.isNull() ) // old project
    {
      pipeNode = layer_node;
    }

    //rasterlayerproperties element there -> old format (1.8 and early 1.9)
    if ( !layer_node.firstChildElement( u"rasterproperties"_s ).isNull() )
    {
      //copy node because layer_node is const
      QDomNode layerNodeCopy = layer_node.cloneNode();
      QDomDocument doc = layerNodeCopy.ownerDocument();
      QDomElement rasterPropertiesElem = layerNodeCopy.firstChildElement( u"rasterproperties"_s );
      QgsProjectFileTransform::convertRasterProperties( doc, layerNodeCopy, rasterPropertiesElem,
          this );
      rasterRendererElem = layerNodeCopy.firstChildElement( u"rasterrenderer"_s );
      QgsDebugMsgLevel( doc.toString(), 4 );
    }
    else
    {
      rasterRendererElem = pipeNode.firstChildElement( u"rasterrenderer"_s );
    }

    if ( !rasterRendererElem.isNull() )
    {
      const QString rendererType = rasterRendererElem.attribute( u"type"_s );
      QgsRasterRendererRegistryEntry rendererEntry;
      if ( mDataProvider && QgsApplication::rasterRendererRegistry()->rendererData( rendererType, rendererEntry ) )
      {
        QgsRasterRenderer *renderer = rendererEntry.rendererCreateFunction( rasterRendererElem, mDataProvider );
        mPipe->set( renderer );
      }
    }

    //brightness
    QgsBrightnessContrastFilter *brightnessFilter = new QgsBrightnessContrastFilter();
    if ( mPipe->set( brightnessFilter ) )
    {
      const QDomElement brightnessElem = pipeNode.firstChildElement( u"brightnesscontrast"_s );
      if ( !brightnessElem.isNull() )
      {
        brightnessFilter->readXml( brightnessElem );
      }
    }

    //hue/saturation
    QgsHueSaturationFilter *hueSaturationFilter = new QgsHueSaturationFilter();
    if ( mPipe->set( hueSaturationFilter ) )
    {
      const QDomElement hueSaturationElem = pipeNode.firstChildElement( u"huesaturation"_s );
      if ( !hueSaturationElem.isNull() )
      {
        hueSaturationFilter->readXml( hueSaturationElem );
      }
    }

    //resampler
    QgsRasterResampleFilter *resampleFilter = new QgsRasterResampleFilter();
    if ( mPipe->set( resampleFilter ) )
    {
      const QDomElement resampleElem = pipeNode.firstChildElement( u"rasterresampler"_s );
      if ( !resampleElem.isNull() )
      {
        resampleFilter->readXml( resampleElem );
      }
    }

    //provider
    if ( mDataProvider )
    {
      const QDomElement providerElem = pipeNode.firstChildElement( u"provider"_s );
      if ( !providerElem.isNull() )
      {
        mDataProvider->readXml( providerElem );
      }
    }

    // Resampling stage
    const QDomNode resamplingStageElement = pipeNode.namedItem( u"resamplingStage"_s );
    if ( !resamplingStageElement.isNull() )
    {
      const QDomElement e = resamplingStageElement.toElement();
      if ( e.text() == "provider"_L1 )
        setResamplingStage( Qgis::RasterResamplingStage::Provider );
      else if ( e.text() == "resamplingFilter"_L1 )
        setResamplingStage( Qgis::RasterResamplingStage::ResampleFilter );
    }

    // get and set the blend mode if it exists
    const QDomNode blendModeNode = layer_node.namedItem( u"blendMode"_s );
    if ( !blendModeNode.isNull() )
    {
      const QDomElement e = blendModeNode.toElement();
      setBlendMode( QgsPainting::getCompositionMode( static_cast< Qgis::BlendMode >( e.text().toInt() ) ) );
    }

    const QDomElement elemDataDefinedProperties = layer_node.firstChildElement( u"pipe-data-defined-properties"_s );
    if ( !elemDataDefinedProperties.isNull() )
      mPipe->dataDefinedProperties().readXml( elemDataDefinedProperties, QgsRasterPipe::propertyDefinitions() );
  }

  // read labeling definition
  if ( categories.testFlag( Labeling ) )
  {
    QgsReadWriteContextCategoryPopper p = context.enterCategory( tr( "Labeling" ) );

    QDomElement labelingElement = layer_node.firstChildElement( u"labeling"_s );
    if ( !labelingElement.isNull() )
    {
      QgsAbstractRasterLayerLabeling *labeling = QgsAbstractRasterLayerLabeling::createFromElement( labelingElement, context );
      mLabelsEnabled = layer_node.toElement().attribute( u"labelsEnabled"_s, u"0"_s ).toInt();
      setLabeling( labeling );
    }
  }

  if ( categories.testFlag( MapTips ) )
  {
    QDomElement mapTipElem = layer_node.namedItem( u"mapTip"_s ).toElement();
    setMapTipTemplate( mapTipElem.text() );
    setMapTipsEnabled( mapTipElem.attribute( u"enabled"_s, u"1"_s ).toInt() == 1 );
  }

  // read attribute table attribute table paths
  if ( categories.testFlag( AttributeTable ) )
  {
    readRasterAttributeTableExternalPaths( layer_node, context );
  }

  if ( categories.testFlag( Legend ) )
  {
    QgsReadWriteContextCategoryPopper p = context.enterCategory( tr( "Legend" ) );

    const QDomElement legendElem = layer_node.firstChildElement( u"legend"_s );
    QgsMapLayerLegend *rasterLegend = legend();
    if ( rasterLegend && !legendElem.isNull() )
    {
      rasterLegend->readXml( legendElem, context );
    }
    else if ( rasterLegend )
    {
      // Temporary fix for #63346 because legend was not saved if empty
      rasterLegend->setFlag( Qgis::MapLayerLegendFlag::ExcludeByDefault, false );
    }
  }

  emit rendererChanged();
  emitStyleChanged();

  return true;
}

bool QgsRasterLayer::readStyle( const QDomNode &node, QString &errorMessage, QgsReadWriteContext &context, QgsMapLayer::StyleCategories categories )
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  return readSymbology( node, errorMessage, context, categories );
}

bool QgsRasterLayer::readXml( const QDomNode &layer_node, QgsReadWriteContext &context )
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  QgsDebugMsgLevel( u"Entered"_s, 4 );
  // Make sure to read the file first so stats etc are initialized properly!

  //process provider key
  const QDomNode pkeyNode = layer_node.namedItem( u"provider"_s );

  if ( pkeyNode.isNull() )
  {
    mProviderKey = u"gdal"_s;
  }
  else
  {
    const QDomElement pkeyElt = pkeyNode.toElement();
    mProviderKey = pkeyElt.text();
    if ( mProviderKey.isEmpty() )
    {
      mProviderKey = u"gdal"_s;
    }
  }

  // Open the raster source based on provider and datasource

  // Go down the raster-data-provider paradigm

  // Collect provider-specific information

  const QDomNode rpNode = layer_node.namedItem( u"rasterproperties"_s );

  if ( mProviderKey == "wms"_L1 )
  {
    // >>> BACKWARD COMPATIBILITY < 1.9
    // The old WMS URI format does not contain all the information, we add them here.
    if ( !mDataSource.contains( "crs="_L1 ) && !mDataSource.contains( "format="_L1 ) )
    {
      QgsDebugMsgLevel( u"Old WMS URI format detected -> adding params"_s, 4 );
      QgsDataSourceUri uri;
      uri.setEncodedUri( mDataSource );
      QDomElement layerElement = rpNode.firstChildElement( u"wmsSublayer"_s );
      while ( !layerElement.isNull() )
      {
        // TODO: sublayer visibility - post-0.8 release timeframe

        // collect name for the sublayer
        uri.setParam( u"layers"_s,  layerElement.namedItem( u"name"_s ).toElement().text() );

        // collect style for the sublayer
        uri.setParam( u"styles"_s, layerElement.namedItem( u"style"_s ).toElement().text() );

        layerElement = layerElement.nextSiblingElement( u"wmsSublayer"_s );
      }

      // Collect format
      uri.setParam( u"format"_s, rpNode.namedItem( u"wmsFormat"_s ).toElement().text() );

      // WMS CRS URL param should not be mixed with that assigned to the layer.
      // In the old WMS URI version there was no CRS and layer crs().authid() was used.
      uri.setParam( u"crs"_s, crs().authid() );
      mDataSource = uri.encodedUri();
    }
    // <<< BACKWARD COMPATIBILITY < 1.9
  }

  if ( !( mReadFlags & QgsMapLayer::FlagDontResolveLayers ) )
  {
    const QgsDataProvider::ProviderOptions providerOptions { context.transformContext() };
    Qgis::DataProviderReadFlags flags = providerReadFlags( layer_node, mReadFlags );

    if ( mReadFlags & QgsMapLayer::FlagReadExtentFromXml )
    {
      const QDomNode extentNode = layer_node.namedItem( u"extent"_s );
      if ( !extentNode.isNull() )
      {
        // get the extent
        const QgsRectangle mbr = QgsXmlUtils::readRectangle( extentNode.toElement() );

        // store the extent
        setExtent( mbr );
      }
    }
    setDataProvider( mProviderKey, providerOptions, flags );
  }

  mOriginalStyleElement = layer_node.namedItem( u"originalStyle"_s ).firstChildElement();
  if ( mOriginalStyleElement.isNull() )
    mOriginalStyleElement = layer_node.toElement();
  mOriginalStyleDocument = layer_node.ownerDocument();

  if ( ! mDataProvider )
  {
    if ( !( mReadFlags & QgsMapLayer::FlagDontResolveLayers ) )
    {
      QgsDebugError( u"Raster data provider could not be created for %1"_s.arg( mDataSource ) );
      return false;
    }
  }

  QString error;
  const bool res = readSymbology( layer_node, error, context );

  // old wms settings we need to correct
  if ( res && mProviderKey == "wms"_L1 && !renderer() )
  {
    setRendererForDrawingStyle( Qgis::RasterDrawingStyle::SingleBandColorData );
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
      QgsDebugMsgLevel( u"data changed, reload provider"_s, 3 );
      closeDataProvider();
      init();
      setDataProvider( mProviderKey );
      if ( !isValid() ) return false;
    }
  }
#endif

  // Load user no data value
  const QDomElement noDataElement = layer_node.firstChildElement( u"noData"_s );

  const QDomNodeList noDataBandList = noDataElement.elementsByTagName( u"noDataList"_s );

  for ( int i = 0; mDataProvider && i < noDataBandList.size(); ++i )
  {
    const QDomElement bandElement = noDataBandList.at( i ).toElement();
    bool ok;
    const int bandNo = bandElement.attribute( u"bandNo"_s ).toInt( &ok );
    QgsDebugMsgLevel( u"bandNo = %1"_s.arg( bandNo ), 4 );
    if ( ok && ( bandNo > 0 ) && ( bandNo <= mDataProvider->bandCount() ) )
    {
      mDataProvider->setUseSourceNoDataValue( bandNo, bandElement.attribute( u"useSrcNoData"_s ).toInt() );
      QgsRasterRangeList myNoDataRangeList;

      const QDomNodeList rangeList = bandElement.elementsByTagName( u"noDataRange"_s );

      myNoDataRangeList.reserve( rangeList.size() );
      for ( int j = 0; j < rangeList.size(); ++j )
      {
        const QDomElement rangeElement = rangeList.at( j ).toElement();
        const QgsRasterRange myNoDataRange( rangeElement.attribute( u"min"_s ).toDouble(),
                                            rangeElement.attribute( u"max"_s ).toDouble() );
        QgsDebugMsgLevel( u"min = %1 %2"_s.arg( rangeElement.attribute( "min" ) ).arg( myNoDataRange.min() ), 4 );
        myNoDataRangeList << myNoDataRange;
      }
      mDataProvider->setUserNoDataValue( bandNo, myNoDataRangeList );
    }
  }

  readRasterAttributeTableExternalPaths( layer_node, context );

  readStyleManager( layer_node );

  return res;
}

bool QgsRasterLayer::writeSymbology( QDomNode &layer_node, QDomDocument &document, QString &errorMessage,
                                     const QgsReadWriteContext &context, QgsMapLayer::StyleCategories categories ) const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  Q_UNUSED( errorMessage )
  // TODO: implement categories for raster layer

  QDomElement layerElement = layer_node.toElement();

  writeCommonStyle( layerElement, document, context, categories );

  if ( categories.testFlag( Labeling ) )
  {
    if ( mLabeling )
    {
      QDomElement labelingElement = mLabeling->save( document, context );
      layerElement.appendChild( labelingElement );
    }
    if ( mLabelsEnabled )
      layerElement.setAttribute( u"labelsEnabled"_s, u"1"_s );
  }

  // save map tip
  if ( categories.testFlag( MapTips ) )
  {
    QDomElement mapTipElem = document.createElement( u"mapTip"_s );
    mapTipElem.setAttribute( u"enabled"_s, mapTipsEnabled() );
    QDomText mapTipText = document.createTextNode( mapTipTemplate() );
    mapTipElem.appendChild( mapTipText );
    layer_node.toElement().appendChild( mapTipElem );
  }

  // save attribute table attribute table paths
  if ( categories.testFlag( AttributeTable ) )
  {
    writeRasterAttributeTableExternalPaths( layer_node, document, context );
  }

  // pipe considers Symbology and Rendering components, why it's always handled as one
  if ( categories.testFlag( Symbology ) || categories.testFlag( Rendering ) )
  {
    // Store pipe members into pipe element, in future, it will be
    // possible to add custom filters into the pipe
    QDomElement pipeElement  = document.createElement( u"pipe"_s );

    for ( int i = 0; i < mPipe->size(); i++ )
    {
      QgsRasterInterface *interface = mPipe->at( i );
      if ( !interface ) continue;
      interface->writeXml( document, pipeElement );
    }

    QDomElement elemDataDefinedProperties = document.createElement( u"pipe-data-defined-properties"_s );
    mPipe->dataDefinedProperties().writeXml( elemDataDefinedProperties, QgsRasterPipe::propertyDefinitions() );
    layer_node.appendChild( elemDataDefinedProperties );

    QDomElement resamplingStageElement = document.createElement( u"resamplingStage"_s );
    const QDomText resamplingStageText = document.createTextNode( resamplingStage() == Qgis::RasterResamplingStage::Provider ? u"provider"_s : u"resamplingFilter"_s );
    resamplingStageElement.appendChild( resamplingStageText );
    pipeElement.appendChild( resamplingStageElement );

    layer_node.appendChild( pipeElement );

    if ( !isValid() && !mOriginalStyleElement.isNull() )
    {
      QDomElement originalStyleElement = document.createElement( u"originalStyle"_s );
      originalStyleElement.appendChild( mOriginalStyleElement );
      layer_node.appendChild( originalStyleElement );
    }

    // add blend mode node
    QDomElement blendModeElement  = document.createElement( u"blendMode"_s );
    const QDomText blendModeText = document.createTextNode( QString::number( static_cast< int >( QgsPainting::getBlendModeEnum( blendMode() ) ) ) );
    blendModeElement.appendChild( blendModeText );
    layer_node.appendChild( blendModeElement );
  }

  if ( categories.testFlag( Legend ) && legend() )
  {
    QDomElement legendElement = legend()->writeXml( document, context );
    if ( !legendElement.isNull() )
      layer_node.appendChild( legendElement );
  }

  return true;
}

bool QgsRasterLayer::writeStyle( QDomNode &node, QDomDocument &doc, QString &errorMessage,
                                 const QgsReadWriteContext &context, QgsMapLayer::StyleCategories categories ) const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  return writeSymbology( node, doc, errorMessage, context, categories );
}

bool QgsRasterLayer::writeXml( QDomNode &layer_node,
                               QDomDocument &document,
                               const QgsReadWriteContext &context ) const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  if ( !mDataProvider )
    return false;

  // first get the layer element so that we can append the type attribute

  QDomElement mapLayerNode = layer_node.toElement();

  if ( mapLayerNode.isNull() || "maplayer" != mapLayerNode.nodeName() )
  {
    QgsMessageLog::logMessage( tr( "<maplayer> not found." ), tr( "Raster" ) );
    return false;
  }

  mapLayerNode.setAttribute( u"type"_s, QgsMapLayerFactory::typeToString( Qgis::LayerType::Raster ) );

  // add provider node

  QDomElement provider  = document.createElement( u"provider"_s );
  const QDomText providerText = document.createTextNode( mProviderKey );
  provider.appendChild( providerText );
  layer_node.appendChild( provider );

  // User no data
  QDomElement noData  = document.createElement( u"noData"_s );

  for ( int bandNo = 1; bandNo <= mDataProvider->bandCount(); bandNo++ )
  {
    QDomElement noDataRangeList = document.createElement( u"noDataList"_s );
    noDataRangeList.setAttribute( u"bandNo"_s, bandNo );
    noDataRangeList.setAttribute( u"useSrcNoData"_s, mDataProvider->useSourceNoDataValue( bandNo ) );

    const auto constUserNoDataValues = mDataProvider->userNoDataValues( bandNo );
    for ( const QgsRasterRange &range : constUserNoDataValues )
    {
      QDomElement noDataRange = document.createElement( u"noDataRange"_s );

      noDataRange.setAttribute( u"min"_s, QgsRasterBlock::printValue( range.min() ) );
      noDataRange.setAttribute( u"max"_s, QgsRasterBlock::printValue( range.max() ) );
      noDataRangeList.appendChild( noDataRange );
    }

    noData.appendChild( noDataRangeList );

  }
  if ( noData.hasChildNodes() )
  {
    layer_node.appendChild( noData );
  }

  // Store file-based raster attribute table paths (if any)
  writeRasterAttributeTableExternalPaths( layer_node, document, context );

  writeStyleManager( layer_node, document );

  //write out the symbology
  QString errorMsg;
  return writeSymbology( layer_node, document, errorMsg, context );
}


QString QgsRasterLayer::encodedSource( const QString &source, const QgsReadWriteContext &context ) const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  return QgsProviderRegistry::instance()->absoluteToRelativeUri( mProviderKey, source, context );
}

QString QgsRasterLayer::decodedSource( const QString &source, const QString &provider, const QgsReadWriteContext &context ) const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  return QgsProviderRegistry::instance()->relativeToAbsoluteUri( provider, source, context );
}

int QgsRasterLayer::width() const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  if ( !mDataProvider ) return 0;
  return mDataProvider->xSize();
}

int QgsRasterLayer::height() const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  if ( !mDataProvider ) return 0;
  return mDataProvider->ySize();
}

void QgsRasterLayer::setResamplingStage( Qgis::RasterResamplingStage stage )
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  mPipe->setResamplingStage( stage );
}

Qgis::RasterResamplingStage QgsRasterLayer::resamplingStage() const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  return mPipe->resamplingStage();
}

bool QgsRasterLayer::update()
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  QgsDebugMsgLevel( u"entered."_s, 4 );
  // Check if data changed
  if ( mDataProvider && mDataProvider->dataTimestamp() > mDataProvider->timestamp() )
  {
    QgsDebugMsgLevel( u"reload data"_s, 4 );
    closeDataProvider();
    init();
    const QgsDataProvider::ProviderOptions providerOptions;
    Qgis::DataProviderReadFlags flags;
    if ( mReadFlags & QgsMapLayer::FlagTrustLayerMetadata )
    {
      flags |= Qgis::DataProviderReadFlag::TrustDataSource;
    }
    setDataProvider( mProviderKey, providerOptions, flags );
    emit dataChanged();
  }
  return isValid();
}

#undef ERR
