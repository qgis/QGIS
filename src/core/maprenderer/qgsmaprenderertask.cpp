/***************************************************************************
                          qgsmaprenderertask.h
                          -------------------------
    begin                : Apr 2017
    copyright            : (C) 2017 by Mathieu Pellerin
    email                : nirvn dot asia at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsannotation.h"
#include "qgsannotationmanager.h"
#include "qgsmaprenderertask.h"
#include "qgsmapsettingsutils.h"
#include "qgsogrutils.h"
#include "qgslogger.h"
#include "qgsabstractgeopdfexporter.h"
#include "qgsmaprendererstagedrenderjob.h"
#include "qgsrenderedfeaturehandlerinterface.h"
#include "qgsfeaturerequest.h"
#include "qgsvectorlayer.h"

#include <QFile>
#include <QImageWriter>
#include <QTextStream>
#include <QTimeZone>
#ifndef QT_NO_PRINTER
#include <QPrinter>
#endif

#include "gdal.h"
#include "cpl_conv.h"

///@cond PRIVATE

class QgsMapRendererTaskGeoPdfExporter : public QgsAbstractGeoPdfExporter
{

  public:

    QgsMapRendererTaskGeoPdfExporter( const QgsMapSettings &ms )
    {
      // collect details upfront, while we are still in the main thread
      const QList< QgsMapLayer * > layers = ms.layers();
      for ( const QgsMapLayer *layer : layers )
      {
        VectorComponentDetail detail;
        detail.name = layer->name();
        detail.mapLayerId = layer->id();
        if ( const QgsVectorLayer *vl = qobject_cast< const QgsVectorLayer * >( layer ) )
        {
          detail.displayAttribute = vl->displayField();
        }
        mLayerDetails[ layer->id() ] = detail;
      }
    }

  private:

    QgsAbstractGeoPdfExporter::VectorComponentDetail componentDetailForLayerId( const QString &layerId ) override
    {
      return mLayerDetails.value( layerId );
    }

    QMap< QString, VectorComponentDetail > mLayerDetails;
};


class QgsMapRendererTaskRenderedFeatureHandler : public QgsRenderedFeatureHandlerInterface
{
  public:

    QgsMapRendererTaskRenderedFeatureHandler( QgsMapRendererTaskGeoPdfExporter *exporter, const QgsMapSettings &settings )
      : mExporter( exporter )
      , mMapSettings( settings )
    {
      // PDF coordinate space uses a hardcoded DPI of 72, also vertical dimension is flipped from QGIS dimension
      const double pageHeightPdfUnits = settings.outputSize().height() * 72.0 / settings.outputDpi();
      mTransform = QTransform::fromTranslate( 0, pageHeightPdfUnits ).scale( 72.0 / mMapSettings.outputDpi(), -72.0 / mMapSettings.outputDpi() );
    }

    void handleRenderedFeature( const QgsFeature &feature, const QgsGeometry &renderedBounds, const QgsRenderedFeatureHandlerInterface::RenderedFeatureContext &context ) override
    {
      // is it a hack retrieving the layer ID from an expression context like this? possibly... BUT
      // the alternative is adding a layer ID member to QgsRenderContext, and that's just asking for people to abuse it
      // and use it to retrieve QgsMapLayers mid-way through a render operation. Lesser of two evils it is!
      const QString layerId = context.renderContext.expressionContext().variable( QStringLiteral( "layer_id" ) ).toString();

      QgsGeometry transformed = renderedBounds;
      transformed.transform( mTransform );

      // always convert to multitype, to make things consistent
      transformed.convertToMultiType();

      mExporter->pushRenderedFeature( layerId, QgsAbstractGeoPdfExporter::RenderedFeature( feature, transformed ) );
    }

    QSet<QString> usedAttributes( QgsVectorLayer *, const QgsRenderContext & ) const override
    {
      return QSet< QString >() << QgsFeatureRequest::ALL_ATTRIBUTES;
    }

  private:

    QgsMapRendererTaskGeoPdfExporter *mExporter = nullptr;
    QgsMapSettings mMapSettings;
    //! Transform from output space (pixels) to PDF space (pixels at 72 dpi)
    QTransform mTransform;

};

///@endcond

QgsMapRendererTask::QgsMapRendererTask( const QgsMapSettings &ms, const QString &fileName, const QString &fileFormat, const bool forceRaster, QgsTask::Flags flags,
                                        const bool geoPDF, const QgsAbstractGeoPdfExporter::ExportDetails &geoPdfExportDetails )
  : QgsTask( fileFormat == QLatin1String( "PDF" ) ? tr( "Saving as PDF" ) : tr( "Saving as image" ), flags )
  , mMapSettings( ms )
  , mFileName( fileName )
  , mFileFormat( fileFormat )
  , mForceRaster( forceRaster )
  , mGeoPDF( geoPDF && mFileFormat == QLatin1String( "PDF" ) && QgsAbstractGeoPdfExporter::geoPDFCreationAvailable() )
  , mGeoPdfExportDetails( geoPdfExportDetails )
{
  prepare();
}

QgsMapRendererTask::QgsMapRendererTask( const QgsMapSettings &ms, QPainter *p )
  : QgsTask( tr( "Rendering to painter" ) )
  , mMapSettings( ms )
  , mPainter( p )
{
  prepare();
}

QgsMapRendererTask::~QgsMapRendererTask() = default;

void QgsMapRendererTask::addAnnotations( const QList< QgsAnnotation * > &annotations )
{
  qDeleteAll( mAnnotations );
  mAnnotations.clear();

  const auto constAnnotations = annotations;
  for ( const QgsAnnotation *a : constAnnotations )
  {
    mAnnotations << a->clone();
  }
}

void QgsMapRendererTask::addDecorations( const QList< QgsMapDecoration * > &decorations )
{
  mDecorations = decorations;
}


void QgsMapRendererTask::cancel()
{
  mJobMutex.lock();
  if ( mJob )
    mJob->cancelWithoutBlocking();
  mJobMutex.unlock();

  QgsTask::cancel();
}

bool QgsMapRendererTask::run()
{
  if ( mErrored )
    return false;

  if ( mGeoPDF )
  {
#ifdef QT_NO_PRINTER
    return false;
#else
    QList< QgsAbstractGeoPdfExporter::ComponentLayerDetail > pdfComponents;

    QgsMapRendererStagedRenderJob *job = static_cast< QgsMapRendererStagedRenderJob * >( mJob.get() );
    int outputLayer = 1;
    while ( !job->isFinished() )
    {
      QgsAbstractGeoPdfExporter::ComponentLayerDetail component;

      component.name = QStringLiteral( "layer_%1" ).arg( outputLayer );
      component.mapLayerId = job->currentLayerId();
      component.opacity = job->currentLayerOpacity();
      component.compositionMode = job->currentLayerCompositionMode();
      component.sourcePdfPath = mGeoPdfExporter->generateTemporaryFilepath( QStringLiteral( "layer_%1.pdf" ).arg( outputLayer ) );
      pdfComponents << component;

      QPrinter printer;
      printer.setOutputFileName( component.sourcePdfPath );
      printer.setOutputFormat( QPrinter::PdfFormat );
#if QT_VERSION < QT_VERSION_CHECK(5, 15, 0)
      printer.setOrientation( QPrinter::Portrait );
      // paper size needs to be given in millimeters in order to be able to set a resolution to pass onto the map renderer
      QSizeF outputSize = mMapSettings.outputSize();
      printer.setPaperSize( outputSize  * 25.4 / mMapSettings.outputDpi(), QPrinter::Millimeter );
      printer.setPageMargins( 0, 0, 0, 0, QPrinter::Millimeter );
#else
      printer.setPageOrientation( QPageLayout::Orientation::Portrait );
      // paper size needs to be given in millimeters in order to be able to set a resolution to pass onto the map renderer
      const QSizeF outputSize = mMapSettings.outputSize();
      const QPageSize pageSize( outputSize  * 25.4 / mMapSettings.outputDpi(), QPageSize::Unit::Millimeter );
      printer.setPageSize( pageSize );
      printer.setPageMargins( QMarginsF( 0, 0, 0, 0 ) );
#endif
      printer.setResolution( mMapSettings.outputDpi() );

      QPainter p( &printer );
      job->renderCurrentPart( &p );
      p.end();

      outputLayer++;
      job->nextPart();
    }
    QgsAbstractGeoPdfExporter::ExportDetails exportDetails = mGeoPdfExportDetails;
    const double pageWidthMM = mMapSettings.outputSize().width() * 25.4 / mMapSettings.outputDpi();
    const double pageHeightMM = mMapSettings.outputSize().height() * 25.4 / mMapSettings.outputDpi();
    exportDetails.pageSizeMm = QSizeF( pageWidthMM, pageHeightMM );
    exportDetails.dpi = mMapSettings.outputDpi();

    exportDetails.layerIdToPdfLayerTreeNameMap = mLayerIdToLayerNameMap;
    exportDetails.layerOrder = mMapLayerOrder;

    if ( mSaveWorldFile )
    {
      // setup georeferencing
      QgsAbstractGeoPdfExporter::GeoReferencedSection georef;
      georef.crs = mMapSettings.destinationCrs();
      georef.pageBoundsMm = QgsRectangle( 0, 0, pageWidthMM, pageHeightMM );
      georef.controlPoints.reserve( 4 );
      georef.controlPoints << QgsAbstractGeoPdfExporter::ControlPoint( QgsPointXY( 0, 0 ), mMapSettings.mapToPixel().toMapCoordinates( 0, 0 ) );
      georef.controlPoints << QgsAbstractGeoPdfExporter::ControlPoint( QgsPointXY( pageWidthMM, 0 ), mMapSettings.mapToPixel().toMapCoordinates( mMapSettings.outputSize().width(), 0 ) );
      georef.controlPoints << QgsAbstractGeoPdfExporter::ControlPoint( QgsPointXY( pageWidthMM, pageHeightMM ), mMapSettings.mapToPixel().toMapCoordinates( mMapSettings.outputSize().width(), mMapSettings.outputSize().height() ) );
      georef.controlPoints << QgsAbstractGeoPdfExporter::ControlPoint( QgsPointXY( 0, pageHeightMM ), mMapSettings.mapToPixel().toMapCoordinates( 0, mMapSettings.outputSize().height() ) );
      exportDetails.georeferencedSections << georef;
    }

    const bool res = mGeoPdfExporter->finalize( pdfComponents, mFileName, exportDetails );
    mGeoPdfExporter.reset();
    mTempPainter.reset();
    mPrinter.reset();
    return res;
#endif
  }
  else
    static_cast< QgsMapRendererCustomPainterJob *>( mJob.get() )->renderPrepared();

  mJobMutex.lock();
  mJob.reset( nullptr );
  mJobMutex.unlock();

  if ( isCanceled() )
    return false;

  QgsRenderContext context = QgsRenderContext::fromMapSettings( mMapSettings );
  context.setPainter( mDestPainter );

  const auto constMDecorations = mDecorations;
  for ( QgsMapDecoration *decoration : constMDecorations )
  {
    decoration->render( mMapSettings, context );
  }

  const auto constMAnnotations = mAnnotations;
  for ( QgsAnnotation *annotation : constMAnnotations )
  {
    if ( isCanceled() )
      return false;

    if ( !annotation || !annotation->isVisible() )
    {
      continue;
    }
    if ( annotation->mapLayer() && !mMapSettings.layers().contains( annotation->mapLayer() ) )
    {
      continue;
    }

    const QgsScopedQPainterState painterState( context.painter() );
    context.setPainterFlagsUsingContext();

    double itemX, itemY;
    if ( annotation->hasFixedMapPosition() )
    {
      itemX = mMapSettings.outputSize().width() * ( annotation->mapPosition().x() - mMapSettings.extent().xMinimum() ) / mMapSettings.extent().width();
      itemY = mMapSettings.outputSize().height() * ( 1 - ( annotation->mapPosition().y() - mMapSettings.extent().yMinimum() ) / mMapSettings.extent().height() );
    }
    else
    {
      itemX = annotation->relativePosition().x() * mMapSettings.outputSize().width();
      itemY = annotation->relativePosition().y() * mMapSettings.outputSize().height();
    }

    context.painter()->translate( itemX, itemY );

    annotation->render( context );
  }

  if ( !mFileName.isEmpty() )
  {
    mDestPainter->end();

    if ( mFileFormat == QLatin1String( "PDF" ) )
    {
#ifndef QT_NO_PRINTER
      if ( mForceRaster )
      {
        QPainter pp;
        pp.begin( mPrinter.get() );
        const QRectF rect( 0, 0, mImage.width(), mImage.height() );
        pp.drawImage( rect, mImage, rect );
        pp.end();
      }

      if ( mSaveWorldFile || mExportMetadata )
      {
        CPLSetThreadLocalConfigOption( "GDAL_PDF_DPI", QString::number( mMapSettings.outputDpi() ).toLocal8Bit().constData() );
        const gdal::dataset_unique_ptr outputDS( GDALOpen( mFileName.toLocal8Bit().constData(), GA_Update ) );
        if ( outputDS )
        {
          if ( mSaveWorldFile )
          {
            double a, b, c, d, e, f;
            QgsMapSettingsUtils::worldFileParameters( mMapSettings, a, b, c, d, e, f );
            c -= 0.5 * a;
            c -= 0.5 * b;
            f -= 0.5 * d;
            f -= 0.5 * e;
            double geoTransform[6] = { c, a, b, f, d, e };
            GDALSetGeoTransform( outputDS.get(), geoTransform );
            GDALSetProjection( outputDS.get(), mMapSettings.destinationCrs().toWkt( QgsCoordinateReferenceSystem::WKT_PREFERRED_GDAL ).toLocal8Bit().constData() );
          }

          if ( mExportMetadata )
          {
            QString creationDateString;
            const QDateTime creationDateTime = mGeoPdfExportDetails.creationDateTime;
            if ( creationDateTime.isValid() )
            {
              creationDateString = QStringLiteral( "D:%1" ).arg( mGeoPdfExportDetails.creationDateTime.toString( QStringLiteral( "yyyyMMddHHmmss" ) ) );
              if ( creationDateTime.timeZone().isValid() )
              {
                int offsetFromUtc = creationDateTime.timeZone().offsetFromUtc( creationDateTime );
                creationDateString += ( offsetFromUtc >= 0 ) ? '+' : '-';
                offsetFromUtc = std::abs( offsetFromUtc );
                const int offsetHours = offsetFromUtc / 3600;
                const int offsetMins = ( offsetFromUtc % 3600 ) / 60;
                creationDateString += QStringLiteral( "%1'%2'" ).arg( offsetHours ).arg( offsetMins );
              }
            }
            GDALSetMetadataItem( outputDS.get(), "CREATION_DATE", creationDateString.toUtf8().constData(), nullptr );

            GDALSetMetadataItem( outputDS.get(), "AUTHOR", mGeoPdfExportDetails.author.toUtf8().constData(), nullptr );
            const QString creator = QStringLiteral( "QGIS %1" ).arg( Qgis::version() );
            GDALSetMetadataItem( outputDS.get(), "CREATOR", creator.toUtf8().constData(), nullptr );
            GDALSetMetadataItem( outputDS.get(), "PRODUCER", creator.toUtf8().constData(), nullptr );
            GDALSetMetadataItem( outputDS.get(), "SUBJECT", mGeoPdfExportDetails.subject.toUtf8().constData(), nullptr );
            GDALSetMetadataItem( outputDS.get(), "TITLE", mGeoPdfExportDetails.title.toUtf8().constData(), nullptr );

            const QgsAbstractMetadataBase::KeywordMap keywords = mGeoPdfExportDetails.keywords;
            QStringList allKeywords;
            for ( auto it = keywords.constBegin(); it != keywords.constEnd(); ++it )
            {
              allKeywords.append( QStringLiteral( "%1: %2" ).arg( it.key(), it.value().join( ',' ) ) );
            }
            const QString keywordString = allKeywords.join( ';' );
            GDALSetMetadataItem( outputDS.get(), "KEYWORDS", keywordString.toUtf8().constData(), nullptr );
          }
        }
        CPLSetThreadLocalConfigOption( "GDAL_PDF_DPI", nullptr );
      }
#else
      mError = ImageUnsupportedFormat;
      return false;
#endif // !QT_NO_PRINTER
    }
    else if ( mFileFormat != QLatin1String( "PDF" ) )
    {
      QImageWriter writer( mFileName, mFileFormat.toLocal8Bit().data() );
      if ( mFileFormat == QLatin1String( "TIF" ) || mFileFormat == QLatin1String( "TIFF" ) )
      {
        // Enable LZW compression
        writer.setCompression( 1 );
      }
      const bool success = writer.write( mImage );
      if ( !success )
      {
        mError = ImageSaveFail;
        return false;
      }

      if ( mSaveWorldFile )
      {
        const QFileInfo info  = QFileInfo( mFileName );

        // build the world file name
        const QString outputSuffix = info.suffix();
        bool skipWorldFile = false;
        if ( outputSuffix == QLatin1String( "tif" ) || outputSuffix == QLatin1String( "tiff" ) )
        {
          const gdal::dataset_unique_ptr outputDS( GDALOpen( mFileName.toLocal8Bit().constData(), GA_Update ) );
          if ( outputDS )
          {
            skipWorldFile = true;
            double a, b, c, d, e, f;
            QgsMapSettingsUtils::worldFileParameters( mMapSettings, a, b, c, d, e, f );
            c -= 0.5 * a;
            c -= 0.5 * b;
            f -= 0.5 * d;
            f -= 0.5 * e;
            double geoTransform[] = { c, a, b, f, d, e };
            GDALSetGeoTransform( outputDS.get(), geoTransform );
            GDALSetProjection( outputDS.get(), mMapSettings.destinationCrs().toWkt( QgsCoordinateReferenceSystem::WKT_PREFERRED_GDAL ).toLocal8Bit().constData() );
          }
        }

        if ( !skipWorldFile )
        {
          const QString worldFileName = info.absolutePath() + '/' + info.completeBaseName() + '.'
                                        + outputSuffix.at( 0 ) + outputSuffix.at( info.suffix().size() - 1 ) + 'w';
          QFile worldFile( worldFileName );

          if ( worldFile.open( QIODevice::WriteOnly | QIODevice::Truncate ) ) //don't use QIODevice::Text
          {
            QTextStream stream( &worldFile );
            stream << QgsMapSettingsUtils::worldFileContent( mMapSettings );
          }
        }
      }
    }
  }

  mTempPainter.reset();
#ifndef QT_NO_PRINTER
  mPrinter.reset();
#endif

  return true;
}

void QgsMapRendererTask::finished( bool result )
{
  qDeleteAll( mAnnotations );
  mAnnotations.clear();

  if ( result )
    emit renderingComplete();
  else
    emit errorOccurred( mError );
}

void QgsMapRendererTask::prepare()
{
  if ( mGeoPDF )
  {
    mGeoPdfExporter = std::make_unique< QgsMapRendererTaskGeoPdfExporter >( mMapSettings );
    if ( mGeoPdfExportDetails.includeFeatures )
    {
      mRenderedFeatureHandler = std::make_unique< QgsMapRendererTaskRenderedFeatureHandler >( static_cast< QgsMapRendererTaskGeoPdfExporter * >( mGeoPdfExporter.get() ), mMapSettings );
      mMapSettings.addRenderedFeatureHandler( mRenderedFeatureHandler.get() );
    }

    const QList< QgsMapLayer * > layers = mMapSettings.layers();
    for ( const QgsMapLayer *layer : layers )
    {
      mLayerIdToLayerNameMap.insert( layer->id(), layer->name() );
      mMapLayerOrder << layer->id();
    }

    mJob.reset( new QgsMapRendererStagedRenderJob( mMapSettings, QgsMapRendererStagedRenderJob::RenderLabelsByMapLayer ) );
    mJob->start();
    return;
  }

  mDestPainter = mPainter;

  if ( mFileFormat == QLatin1String( "PDF" ) )
  {
#ifndef QT_NO_PRINTER
    mPrinter.reset( new QPrinter() );
    mPrinter->setOutputFileName( mFileName );
    mPrinter->setOutputFormat( QPrinter::PdfFormat );
#if QT_VERSION < QT_VERSION_CHECK(5, 15, 0)
    mPrinter->setOrientation( QPrinter::Portrait );
    // paper size needs to be given in millimeters in order to be able to set a resolution to pass onto the map renderer
    QSizeF outputSize = mMapSettings.outputSize();
    mPrinter->setPaperSize( outputSize  * 25.4 / mMapSettings.outputDpi(), QPrinter::Millimeter );
    mPrinter->setPageMargins( 0, 0, 0, 0, QPrinter::Millimeter );
#else
    mPrinter->setPageOrientation( QPageLayout::Orientation::Portrait );
    // paper size needs to be given in millimeters in order to be able to set a resolution to pass onto the map renderer
    const QSizeF outputSize = mMapSettings.outputSize();
    const QPageSize pageSize( outputSize  * 25.4 / mMapSettings.outputDpi(), QPageSize::Unit::Millimeter );
    mPrinter->setPageSize( pageSize );
    mPrinter->setPageMargins( QMarginsF( 0, 0, 0, 0 ) );
#endif
    mPrinter->setResolution( mMapSettings.outputDpi() );

    if ( !mForceRaster )
    {
      mTempPainter.reset( new QPainter( mPrinter.get() ) );
      mDestPainter = mTempPainter.get();
    }
#else
    mError = ImageUnsupportedFormat;
#endif // ! QT_NO_PRINTER
  }

  if ( !mDestPainter )
  {
    // save rendered map to an image file
    mImage = QImage( mMapSettings.outputSize(), QImage::Format_ARGB32 );
    if ( mImage.isNull() )
    {
      mErrored = true;
      mError = ImageAllocationFail;
      return;
    }

    mImage.setDotsPerMeterX( 1000 * mMapSettings.outputDpi() / 25.4 );
    mImage.setDotsPerMeterY( 1000 * mMapSettings.outputDpi() / 25.4 );

    mTempPainter.reset( new QPainter( &mImage ) );
    mDestPainter = mTempPainter.get();
  }

  if ( !mDestPainter )
  {
    mErrored = true;
    return;
  }

  mJob.reset( new QgsMapRendererCustomPainterJob( mMapSettings, mDestPainter ) );
  static_cast< QgsMapRendererCustomPainterJob *>( mJob.get() )->prepare();
}
