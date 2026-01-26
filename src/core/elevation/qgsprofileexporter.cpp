/***************************************************************************
                         qgsprofileexporter.cpp
                         ---------------
    begin                : May 2023
    copyright            : (C) 2023 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "qgsprofileexporter.h"

#include "qgsabstractprofilegenerator.h"
#include "qgsabstractprofilesource.h"
#include "qgsdxfexport.h"
#include "qgsmemoryproviderutils.h"
#include "qgsprofilerenderer.h"
#include "qgsvectorfilewriter.h"
#include "qgsvectorlayer.h"

#include <QFileInfo>
#include <QThread>

#include "moc_qgsprofileexporter.cpp"

QgsProfileExporter::QgsProfileExporter( const QList<QgsAbstractProfileSource *> &sources, const QgsProfileRequest &request, Qgis::ProfileExportType type )
  : mType( type )
  , mRequest( request )
{
  for ( QgsAbstractProfileSource *source : sources )
  {
    if ( source )
    {
      if ( std::unique_ptr< QgsAbstractProfileGenerator > generator{ source->createProfileGenerator( mRequest ) } )
        mGenerators.emplace_back( std::move( generator ) );
    }
  }
}

QgsProfileExporter::~QgsProfileExporter() = default;

void QgsProfileExporter::run( QgsFeedback *feedback )
{
  if ( mGenerators.empty() )
    return;

  QgsProfilePlotRenderer renderer( std::move( mGenerators ), mRequest );
  renderer.startGeneration();
  renderer.waitForFinished();

  mFeatures = renderer.asFeatures( mType, feedback );
}

QList< QgsVectorLayer *> QgsProfileExporter::toLayers()
{
  if ( mFeatures.empty() )
    return {};

  // collect all features with the same geometry types together
  QHash< quint32, QVector< QgsAbstractProfileResults::Feature > > featuresByGeometryType;
  for ( const QgsAbstractProfileResults::Feature &feature : std::as_const( mFeatures ) )
  {
    featuresByGeometryType[static_cast< quint32 >( feature.geometry.wkbType() )].append( feature );
  }

  // generate a new memory provider layer for each geometry type
  QList< QgsVectorLayer * > res;
  for ( auto wkbTypeIt = featuresByGeometryType.constBegin(); wkbTypeIt != featuresByGeometryType.constEnd(); ++wkbTypeIt )
  {
    // first collate a master list of fields for this geometry type
    QgsFields outputFields;
    outputFields.append( QgsField( u"layer"_s, QMetaType::Type::QString ) );

    for ( const QgsAbstractProfileResults::Feature &feature : std::as_const( wkbTypeIt.value() ) )
    {
      for ( auto attributeIt = feature.attributes.constBegin(); attributeIt != feature.attributes.constEnd(); ++attributeIt )
      {
        const int existingFieldIndex = outputFields.lookupField( attributeIt.key() );
        if ( existingFieldIndex < 0 )
        {
          outputFields.append( QgsField( attributeIt.key(), static_cast<QMetaType::Type>( attributeIt.value().userType() ) ) );
        }
        else
        {
          if ( outputFields.at( existingFieldIndex ).type() != QMetaType::Type::QString && outputFields.at( existingFieldIndex ).type() != attributeIt.value().userType() )
          {
            // attribute type mismatch across fields, just promote to string types to be flexible
            outputFields[ existingFieldIndex ].setType( QMetaType::Type::QString );
          }
        }
      }
    }

    // note -- 2d profiles have no CRS associated, the coordinate values are not location based!
    std::unique_ptr< QgsVectorLayer > outputLayer( QgsMemoryProviderUtils::createMemoryLayer(
          u"profile"_s,
          outputFields,
          static_cast< Qgis::WkbType >( wkbTypeIt.key() ),
          mType == Qgis::ProfileExportType::Profile2D ? QgsCoordinateReferenceSystem() : mRequest.crs(),
          false ) );

    QList< QgsFeature > featuresToAdd;
    featuresToAdd.reserve( wkbTypeIt.value().size() );
    for ( const QgsAbstractProfileResults::Feature &feature : std::as_const( wkbTypeIt.value() ) )
    {
      QgsFeature out( outputFields );
      out.setAttribute( 0, feature.layerIdentifier );
      out.setGeometry( feature.geometry );
      for ( auto attributeIt = feature.attributes.constBegin(); attributeIt != feature.attributes.constEnd(); ++attributeIt )
      {
        const int outputFieldIndex = outputFields.lookupField( attributeIt.key() );
        const QgsField &targetField = outputFields.at( outputFieldIndex );
        QVariant value = attributeIt.value();
        targetField.convertCompatible( value );
        out.setAttribute( outputFieldIndex, value );
      }
      featuresToAdd << out;
    }

    if ( !outputLayer->dataProvider()->addFeatures( featuresToAdd, QgsFeatureSink::FastInsert ) )
    {
      QgsDebugError( u"Error exporting feature: %1"_s.arg( outputLayer->dataProvider()->lastError() ) );
    }
    res << outputLayer.release();
  }
  return res;
}

//
// QgsProfileExporterTask
//

QgsProfileExporterTask::QgsProfileExporterTask( const QList<QgsAbstractProfileSource *> &sources,
    const QgsProfileRequest &request,
    Qgis::ProfileExportType type,
    const QString &destination,
    const QgsCoordinateTransformContext &transformContext
                                              )
  : QgsTask( tr( "Exporting elevation profile" ), QgsTask::CanCancel )
  , mDestination( destination )
  , mTransformContext( transformContext )
{
  mExporter = std::make_unique< QgsProfileExporter >( sources, request, type );
}

bool QgsProfileExporterTask::run()
{
  mFeedback = std::make_unique< QgsFeedback >();

  mExporter->run( mFeedback.get() );

  mLayers = mExporter->toLayers();

  if ( mFeedback->isCanceled() )
  {
    mResult = ExportResult::Canceled;
    return true;
  }

  if ( !mDestination.isEmpty() && !mLayers.empty() )
  {
    const QFileInfo destinationFileInfo( mDestination );
    const QString fileExtension = destinationFileInfo.completeSuffix();
    const QString driverName = QgsVectorFileWriter::driverForExtension( fileExtension );

    if ( driverName == "DXF"_L1 )
    {
      // DXF gets special handling -- we use the inbuilt QgsDxfExport class
      QgsDxfExport dxf;
      QList< QgsDxfExport::DxfLayer > dxfLayers;
      for ( QgsVectorLayer *layer : std::as_const( mLayers ) )
      {
        QgsDxfExport::DxfLayer dxfLayer( layer );
        dxfLayers.append( dxfLayer );
        if ( layer->crs().isValid() )
          dxf.setDestinationCrs( layer->crs() );
      }
      dxf.addLayers( dxfLayers );
      QFile dxfFile( mDestination );
      switch ( dxf.writeToFile( &dxfFile, u"UTF-8"_s ) )
      {
        case QgsDxfExport::ExportResult::Success:
          mResult = ExportResult::Success;
          mCreatedFiles.append( mDestination );
          break;

        case QgsDxfExport::ExportResult::InvalidDeviceError:
        case QgsDxfExport::ExportResult::DeviceNotWritableError:
          mResult = ExportResult::DeviceError;
          break;

        case QgsDxfExport::ExportResult::EmptyExtentError:
          mResult = ExportResult::DxfExportFailed;
          break;
      }
    }
    else
    {
      // use vector file writer
      const bool outputFormatIsMultiLayer = QgsVectorFileWriter::supportedFormatExtensions( QgsVectorFileWriter::SupportsMultipleLayers ).contains( fileExtension );

      int layerCount = 1;
      for ( QgsVectorLayer *layer : std::as_const( mLayers ) )
      {
        QString thisLayerFilename;
        QgsVectorFileWriter::SaveVectorOptions options;
        if ( outputFormatIsMultiLayer )
        {
          thisLayerFilename = mDestination;
          options.actionOnExistingFile = layerCount == 1 ? QgsVectorFileWriter::ActionOnExistingFile::CreateOrOverwriteFile
                                         : QgsVectorFileWriter::ActionOnExistingFile::CreateOrOverwriteLayer;
          if ( mLayers.size() > 1 )
            options.layerName = u"profile_%1"_s.arg( layerCount );
        }
        else
        {
          options.actionOnExistingFile = QgsVectorFileWriter::ActionOnExistingFile::CreateOrOverwriteFile;
          if ( mLayers.size() > 1 )
          {
            thisLayerFilename = u"%1/%2_%3.%4"_s.arg( destinationFileInfo.path(), destinationFileInfo.baseName() ).arg( layerCount ).arg( fileExtension );
          }
          else
          {
            thisLayerFilename = mDestination;
          }
        }
        options.driverName = driverName;
        options.feedback = mFeedback.get();
        options.fileEncoding = u"UTF-8"_s;
        QString newFileName;
        QgsVectorFileWriter::WriterError result = QgsVectorFileWriter::writeAsVectorFormatV3(
              layer,
              thisLayerFilename,
              mTransformContext,
              options,
              &mError,
              &newFileName
            );
        switch ( result )
        {
          case QgsVectorFileWriter::NoError:
            mResult = ExportResult::Success;
            if ( !mCreatedFiles.contains( newFileName ) )
              mCreatedFiles.append( newFileName );
            break;

          case QgsVectorFileWriter::ErrDriverNotFound:
          case QgsVectorFileWriter::ErrCreateDataSource:
          case QgsVectorFileWriter::ErrCreateLayer:
            mResult = ExportResult::DeviceError;
            break;

          case QgsVectorFileWriter::ErrAttributeTypeUnsupported:
          case QgsVectorFileWriter::ErrAttributeCreationFailed:
          case QgsVectorFileWriter::ErrProjection:
          case QgsVectorFileWriter::ErrFeatureWriteFailed:
          case QgsVectorFileWriter::ErrInvalidLayer:
          case QgsVectorFileWriter::ErrSavingMetadata:
            mResult = ExportResult::LayerExportFailed;
            break;


          case QgsVectorFileWriter::Canceled:
            mResult = ExportResult::Canceled;
            break;
        }

        if ( mResult != ExportResult::Success )
          break;
        layerCount += 1;
      }
    }
  }
  else if ( mLayers.empty() )
  {
    mResult = ExportResult::Empty;
  }

  for ( QgsVectorLayer *layer : std::as_const( mLayers ) )
  {
    layer->moveToThread( nullptr );
  }

  mExporter.reset();
  return true;
}

void QgsProfileExporterTask::cancel()
{
  if ( mFeedback )
    mFeedback->cancel();

  QgsTask::cancel();
}

QList<QgsVectorLayer *> QgsProfileExporterTask::takeLayers()
{
  QList<QgsVectorLayer *> res;
  res.reserve( mLayers.size() );
  for ( QgsVectorLayer *layer : std::as_const( mLayers ) )
  {
    layer->moveToThread( QThread::currentThread() );
    res.append( layer );
  }
  mLayers.clear();
  return res;
}

QgsProfileExporterTask::ExportResult QgsProfileExporterTask::result() const
{
  return mResult;
}
