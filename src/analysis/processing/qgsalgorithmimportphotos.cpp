/***************************************************************************
                         qgsalgorithmimportphotos.cpp
                         ------------------
    begin                : March 2018
    copyright            : (C) 2018 by Nyall Dawson
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

#include "qgsalgorithmimportphotos.h"

#include "qgsogrutils.h"
#include "qgsvectorlayer.h"

#include <QDirIterator>
#include <QFileInfo>
#include <QRegularExpression>

///@cond PRIVATE

QString QgsImportPhotosAlgorithm::name() const
{
  return u"importphotos"_s;
}

QString QgsImportPhotosAlgorithm::displayName() const
{
  return QObject::tr( "Import geotagged photos" );
}

QStringList QgsImportPhotosAlgorithm::tags() const
{
  return QObject::tr( "exif,metadata,gps,jpeg,jpg" ).split( ',' );
}

QString QgsImportPhotosAlgorithm::group() const
{
  return QObject::tr( "Vector creation" );
}

QString QgsImportPhotosAlgorithm::groupId() const
{
  return u"vectorcreation"_s;
}

void QgsImportPhotosAlgorithm::initAlgorithm( const QVariantMap & )
{
  addParameter( new QgsProcessingParameterFile( u"FOLDER"_s, QObject::tr( "Input folder" ), Qgis::ProcessingFileParameterBehavior::Folder ) );
  addParameter( new QgsProcessingParameterBoolean( u"RECURSIVE"_s, QObject::tr( "Scan recursively" ), false ) );

  auto output = std::make_unique<QgsProcessingParameterFeatureSink>( u"OUTPUT"_s, QObject::tr( "Photos" ), Qgis::ProcessingSourceType::VectorPoint, QVariant(), true );
  output->setCreateByDefault( true );
  addParameter( output.release() );

  auto invalid = std::make_unique<QgsProcessingParameterFeatureSink>( u"INVALID"_s, QObject::tr( "Invalid photos table" ), Qgis::ProcessingSourceType::Vector, QVariant(), true );
  invalid->setCreateByDefault( false );
  addParameter( invalid.release() );
}

QString QgsImportPhotosAlgorithm::shortHelpString() const
{
  return QObject::tr( "This algorithm creates a point layer corresponding to the geotagged locations from JPEG or HEIF/HEIC images from a source folder. Optionally the folder can be recursively scanned.\n\n"
                      "The point layer will contain a single PointZ feature per input file from which the geotags could be read. Any altitude information from the geotags will be used "
                      "to set the point's Z value.\n\n"
                      "Optionally, a table of unreadable or non-geotagged photos can also be created." );
}

QString QgsImportPhotosAlgorithm::shortDescription() const
{
  return QObject::tr( "Creates a point layer corresponding to the geotagged locations from JPEG or HEIF/HEIC images from a source folder." );
}

QgsImportPhotosAlgorithm *QgsImportPhotosAlgorithm::createInstance() const
{
  return new QgsImportPhotosAlgorithm();
}

QVariant QgsImportPhotosAlgorithm::parseMetadataValue( const QString &value )
{
  const thread_local QRegularExpression numRx( u"^\\s*\\(\\s*([-\\.\\d]+)\\s*\\)\\s*$"_s );
  const QRegularExpressionMatch numMatch = numRx.match( value );
  if ( numMatch.hasMatch() )
  {
    return numMatch.captured( 1 ).toDouble();
  }
  return value;
}

bool QgsImportPhotosAlgorithm::extractGeoTagFromMetadata( const QVariantMap &metadata, QgsPointXY &tag )
{
  double x = 0.0;
  if ( metadata.contains( u"EXIF_GPSLongitude"_s ) )
  {
    bool ok = false;
    x = metadata.value( u"EXIF_GPSLongitude"_s ).toDouble( &ok );
    if ( !ok )
      return false;

    if ( QStringView { metadata.value( u"EXIF_GPSLongitudeRef"_s ).toString() }.right( 1 ).compare( 'W'_L1, Qt::CaseInsensitive ) == 0
         || metadata.value( u"EXIF_GPSLongitudeRef"_s ).toDouble() < 0 )
    {
      x = -x;
    }
  }
  else
  {
    return false;
  }

  double y = 0.0;
  if ( metadata.contains( u"EXIF_GPSLatitude"_s ) )
  {
    bool ok = false;
    y = metadata.value( u"EXIF_GPSLatitude"_s ).toDouble( &ok );
    if ( !ok )
      return false;

    if ( QStringView { metadata.value( u"EXIF_GPSLatitudeRef"_s ).toString() }.right( 1 ).compare( 'S'_L1, Qt::CaseInsensitive ) == 0
         || metadata.value( u"EXIF_GPSLatitudeRef"_s ).toDouble() < 0 )
    {
      y = -y;
    }
  }
  else
  {
    return false;
  }

  tag = QgsPointXY( x, y );
  return true;
}

QVariant QgsImportPhotosAlgorithm::extractAltitudeFromMetadata( const QVariantMap &metadata )
{
  QVariant altitude;
  if ( metadata.contains( u"EXIF_GPSAltitude"_s ) )
  {
    double alt = metadata.value( u"EXIF_GPSAltitude"_s ).toDouble();
    if ( metadata.contains( u"EXIF_GPSAltitudeRef"_s ) && ( ( metadata.value( u"EXIF_GPSAltitudeRef"_s ).userType() == QMetaType::Type::QString && metadata.value( u"EXIF_GPSAltitudeRef"_s ).toString().right( 1 ) == "1"_L1 ) || metadata.value( u"EXIF_GPSAltitudeRef"_s ).toDouble() < 0 ) )
      alt = -alt;
    altitude = alt;
  }
  return altitude;
}

QVariant QgsImportPhotosAlgorithm::extractDirectionFromMetadata( const QVariantMap &metadata )
{
  QVariant direction;
  if ( metadata.contains( u"EXIF_GPSImgDirection"_s ) )
  {
    direction = metadata.value( u"EXIF_GPSImgDirection"_s ).toDouble();
  }
  return direction;
}

QVariant QgsImportPhotosAlgorithm::extractOrientationFromMetadata( const QVariantMap &metadata )
{
  QVariant orientation;
  if ( metadata.contains( u"EXIF_Orientation"_s ) )
  {
    switch ( metadata.value( u"EXIF_Orientation"_s ).toInt() )
    {
      case 1:
        orientation = 0;
        break;
      case 2:
        orientation = 0;
        break;
      case 3:
        orientation = 180;
        break;
      case 4:
        orientation = 180;
        break;
      case 5:
        orientation = 90;
        break;
      case 6:
        orientation = 90;
        break;
      case 7:
        orientation = 270;
        break;
      case 8:
        orientation = 270;
        break;
    }
  }
  return orientation;
}

QVariant QgsImportPhotosAlgorithm::extractTimestampFromMetadata( const QVariantMap &metadata )
{
  QVariant ts;
  if ( metadata.contains( u"EXIF_DateTimeOriginal"_s ) )
  {
    ts = metadata.value( u"EXIF_DateTimeOriginal"_s );
  }
  else if ( metadata.contains( u"EXIF_DateTimeDigitized"_s ) )
  {
    ts = metadata.value( u"EXIF_DateTimeDigitized"_s );
  }
  else if ( metadata.contains( u"EXIF_DateTime"_s ) )
  {
    ts = metadata.value( u"EXIF_DateTime"_s );
  }

  if ( !ts.isValid() )
    return ts;

  const thread_local QRegularExpression dsRegEx( u"(\\d+):(\\d+):(\\d+)\\s+(\\d+):(\\d+):(\\d+)"_s );
  const QRegularExpressionMatch dsMatch = dsRegEx.match( ts.toString() );
  if ( dsMatch.hasMatch() )
  {
    const int year = dsMatch.captured( 1 ).toInt();
    const int month = dsMatch.captured( 2 ).toInt();
    const int day = dsMatch.captured( 3 ).toInt();
    const int hour = dsMatch.captured( 4 ).toInt();
    const int min = dsMatch.captured( 5 ).toInt();
    const int sec = dsMatch.captured( 6 ).toInt();
    return QDateTime( QDate( year, month, day ), QTime( hour, min, sec ) );
  }
  else
  {
    return QVariant();
  }
}

QVariant QgsImportPhotosAlgorithm::parseCoord( const QString &string )
{
  const thread_local QRegularExpression coordRx( u"^\\s*\\(\\s*([-\\.\\d]+)\\s*\\)\\s*\\(\\s*([-\\.\\d]+)\\s*\\)\\s*\\(\\s*([-\\.\\d]+)\\s*\\)\\s*$"_s );
  const QRegularExpressionMatch coordMatch = coordRx.match( string );
  if ( coordMatch.hasMatch() )
  {
    const double hours = coordMatch.captured( 1 ).toDouble();
    const double minutes = coordMatch.captured( 2 ).toDouble();
    const double seconds = coordMatch.captured( 3 ).toDouble();
    return hours + minutes / 60.0 + seconds / 3600.0;
  }
  else
  {
    return QVariant();
  }
}

QVariantMap QgsImportPhotosAlgorithm::parseMetadataList( const QStringList &input )
{
  QVariantMap results;
  const thread_local QRegularExpression splitRx( u"(.*?)=(.*)"_s );
  for ( const QString &item : input )
  {
    const QRegularExpressionMatch match = splitRx.match( item );
    if ( !match.hasMatch() )
      continue;

    const QString tag = match.captured( 1 );
    QVariant value = parseMetadataValue( match.captured( 2 ) );

    if ( tag == "EXIF_GPSLatitude"_L1 || tag == "EXIF_GPSLongitude"_L1 )
      value = parseCoord( value.toString() );
    results.insert( tag, value );
  }
  return results;
}


class SetEditorWidgetForPhotoAttributePostProcessor : public QgsProcessingLayerPostProcessorInterface
{
  public:
    void postProcessLayer( QgsMapLayer *layer, QgsProcessingContext &, QgsProcessingFeedback * ) override
    {
      if ( QgsVectorLayer *vl = qobject_cast<QgsVectorLayer *>( layer ) )
      {
        QVariantMap config;
        // photo field shows picture viewer
        config.insert( u"DocumentViewer"_s, 1 );
        config.insert( u"FileWidget"_s, true );
        config.insert( u"UseLink"_s, true );
        config.insert( u"FullUrl"_s, true );
        vl->setEditorWidgetSetup( vl->fields().lookupField( u"photo"_s ), QgsEditorWidgetSetup( u"ExternalResource"_s, config ) );

        config.clear();
        // path field is a directory link
        config.insert( u"FileWidgetButton"_s, true );
        config.insert( u"StorageMode"_s, 1 );
        config.insert( u"UseLink"_s, true );
        config.insert( u"FullUrl"_s, true );
        vl->setEditorWidgetSetup( vl->fields().lookupField( u"directory"_s ), QgsEditorWidgetSetup( u"ExternalResource"_s, config ) );
      }
    }
};

QVariantMap QgsImportPhotosAlgorithm::processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  const QString folder = parameterAsFile( parameters, u"FOLDER"_s, context );

  const QDir importDir( folder );
  if ( !importDir.exists() )
  {
    throw QgsProcessingException( QObject::tr( "Directory %1 does not exist!" ).arg( folder ) );
  }

  const bool recurse = parameterAsBoolean( parameters, u"RECURSIVE"_s, context );

  QgsFields outFields;
  outFields.append( QgsField( u"photo"_s, QMetaType::Type::QString ) );
  outFields.append( QgsField( u"filename"_s, QMetaType::Type::QString ) );
  outFields.append( QgsField( u"directory"_s, QMetaType::Type::QString ) );
  outFields.append( QgsField( u"altitude"_s, QMetaType::Type::Double ) );
  outFields.append( QgsField( u"direction"_s, QMetaType::Type::Double ) );
  outFields.append( QgsField( u"rotation"_s, QMetaType::Type::Int ) );
  outFields.append( QgsField( u"longitude"_s, QMetaType::Type::QString ) );
  outFields.append( QgsField( u"latitude"_s, QMetaType::Type::QString ) );
  outFields.append( QgsField( u"timestamp"_s, QMetaType::Type::QDateTime ) );
  QString outputDest;
  std::unique_ptr<QgsFeatureSink> outputSink( parameterAsSink( parameters, u"OUTPUT"_s, context, outputDest, outFields, Qgis::WkbType::PointZ, QgsCoordinateReferenceSystem( u"EPSG:4326"_s ) ) );

  QgsFields invalidFields;
  invalidFields.append( QgsField( u"photo"_s, QMetaType::Type::QString ) );
  invalidFields.append( QgsField( u"filename"_s, QMetaType::Type::QString ) );
  invalidFields.append( QgsField( u"directory"_s, QMetaType::Type::QString ) );
  invalidFields.append( QgsField( u"readable"_s, QMetaType::Type::Bool ) );
  QString invalidDest;
  std::unique_ptr<QgsFeatureSink> invalidSink( parameterAsSink( parameters, u"INVALID"_s, context, invalidDest, invalidFields ) );

  const QStringList nameFilters { "*.jpeg", "*.jpg", "*.heic" };
  QStringList files;

  if ( !recurse )
  {
    const QFileInfoList fileInfoList = importDir.entryInfoList( nameFilters, QDir::NoDotAndDotDot | QDir::Files );
    for ( auto infoIt = fileInfoList.constBegin(); infoIt != fileInfoList.constEnd(); ++infoIt )
    {
      files.append( infoIt->absoluteFilePath() );
    }
  }
  else
  {
    QDirIterator it( folder, nameFilters, QDir::NoDotAndDotDot | QDir::Files, QDirIterator::Subdirectories );
    while ( it.hasNext() )
    {
      it.next();
      files.append( it.filePath() );
    }
  }

  auto saveInvalidFile = [&invalidSink, &parameters]( QgsAttributes &attributes, bool readable ) {
    if ( invalidSink )
    {
      QgsFeature f;
      attributes.append( readable );
      f.setAttributes( attributes );
      if ( !invalidSink->addFeature( f, QgsFeatureSink::FastInsert ) )
        throw QgsProcessingException( writeFeatureError( invalidSink.get(), parameters, u"INVALID"_s ) );
    }
  };

  const double step = files.count() > 0 ? 100.0 / files.count() : 1;
  int i = 0;
  for ( const QString &file : files )
  {
    i++;
    if ( feedback->isCanceled() )
    {
      break;
    }

    feedback->setProgress( i * step );

    const QFileInfo fi( file );
    QgsAttributes attributes;
    attributes << QDir::toNativeSeparators( file )
               << fi.completeBaseName()
               << QDir::toNativeSeparators( fi.absolutePath() );

    const gdal::dataset_unique_ptr hDS( GDALOpen( file.toUtf8().constData(), GA_ReadOnly ) );
    if ( !hDS )
    {
      feedback->reportError( QObject::tr( "Could not open %1" ).arg( QDir::toNativeSeparators( file ) ) );
      saveInvalidFile( attributes, false );
      continue;
    }

    char **GDALmetadata = GDALGetMetadata( hDS.get(), nullptr );
    if ( !GDALmetadata )
    {
      GDALmetadata = GDALGetMetadata( hDS.get(), "EXIF" );
    }
    if ( !GDALmetadata )
    {
      feedback->reportError( QObject::tr( "No metadata found in %1" ).arg( QDir::toNativeSeparators( file ) ) );
      saveInvalidFile( attributes, true );
    }
    else
    {
      if ( !outputSink )
        continue;

      QgsFeature f;
      const QVariantMap metadata = parseMetadataList( QgsOgrUtils::cStringListToQStringList( GDALmetadata ) );

      QgsPointXY tag;
      if ( !extractGeoTagFromMetadata( metadata, tag ) )
      {
        // no geotag
        feedback->reportError( QObject::tr( "Could not retrieve geotag for %1" ).arg( QDir::toNativeSeparators( file ) ) );
        saveInvalidFile( attributes, true );
        continue;
      }

      const QVariant altitude = extractAltitudeFromMetadata( metadata );
      const QgsGeometry p = QgsGeometry( new QgsPoint( tag.x(), tag.y(), altitude.toDouble(), 0, Qgis::WkbType::PointZ ) );
      f.setGeometry( p );

      attributes
        << altitude
        << extractDirectionFromMetadata( metadata )
        << extractOrientationFromMetadata( metadata )
        << tag.x()
        << tag.y()
        << extractTimestampFromMetadata( metadata );
      f.setAttributes( attributes );
      if ( !outputSink->addFeature( f, QgsFeatureSink::FastInsert ) )
        throw QgsProcessingException( writeFeatureError( outputSink.get(), parameters, u"OUTPUT"_s ) );
    }
  }

  QVariantMap outputs;
  if ( outputSink )
  {
    outputSink->finalize();
    outputs.insert( u"OUTPUT"_s, outputDest );

    if ( context.willLoadLayerOnCompletion( outputDest ) )
    {
      context.layerToLoadOnCompletionDetails( outputDest ).setPostProcessor( new SetEditorWidgetForPhotoAttributePostProcessor() );
    }
  }

  if ( invalidSink )
  {
    invalidSink->finalize();
    outputs.insert( u"INVALID"_s, invalidDest );
  }
  return outputs;
}

///@endcond
