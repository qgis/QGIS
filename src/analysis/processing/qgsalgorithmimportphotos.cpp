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
  return QStringLiteral( "importphotos" );
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
  return QStringLiteral( "vectorcreation" );
}

void QgsImportPhotosAlgorithm::initAlgorithm( const QVariantMap & )
{
  addParameter( new QgsProcessingParameterFile( QStringLiteral( "FOLDER" ), QObject::tr( "Input folder" ), QgsProcessingParameterFile::Folder ) );
  addParameter( new QgsProcessingParameterBoolean( QStringLiteral( "RECURSIVE" ), QObject::tr( "Scan recursively" ), false ) );

  std::unique_ptr< QgsProcessingParameterFeatureSink > output = std::make_unique< QgsProcessingParameterFeatureSink >( QStringLiteral( "OUTPUT" ), QObject::tr( "Photos" ), QgsProcessing::TypeVectorPoint, QVariant(), true );
  output->setCreateByDefault( true );
  addParameter( output.release() );

  std::unique_ptr< QgsProcessingParameterFeatureSink > invalid = std::make_unique< QgsProcessingParameterFeatureSink >( QStringLiteral( "INVALID" ), QObject::tr( "Invalid photos table" ), QgsProcessing::TypeVector, QVariant(), true );
  invalid->setCreateByDefault( false );
  addParameter( invalid.release() );
}

QString QgsImportPhotosAlgorithm::shortHelpString() const
{
  return QObject::tr( "Creates a point layer corresponding to the geotagged locations from JPEG images from a source folder. Optionally the folder can be recursively scanned.\n\n"
                      "The point layer will contain a single PointZ feature per input file from which the geotags could be read. Any altitude information from the geotags will be used "
                      "to set the point's Z value.\n\n"
                      "Optionally, a table of unreadable or non-geotagged photos can also be created." );
}

QgsImportPhotosAlgorithm *QgsImportPhotosAlgorithm::createInstance() const
{
  return new QgsImportPhotosAlgorithm();
}

QVariant QgsImportPhotosAlgorithm::parseMetadataValue( const QString &value )
{
  const QRegularExpression numRx( QStringLiteral( "^\\s*\\(\\s*([-\\.\\d]+)\\s*\\)\\s*$" ) );
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
  if ( metadata.contains( QStringLiteral( "EXIF_GPSLongitude" ) ) )
  {
    bool ok = false;
    x = metadata.value( QStringLiteral( "EXIF_GPSLongitude" ) ).toDouble( &ok );
    if ( !ok )
      return false;

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    if ( metadata.value( QStringLiteral( "EXIF_GPSLongitudeRef" ) ).toString().rightRef( 1 ).compare( QLatin1String( "W" ), Qt::CaseInsensitive ) == 0
         || metadata.value( QStringLiteral( "EXIF_GPSLongitudeRef" ) ).toDouble() < 0 )
#else
    if ( QStringView { metadata.value( QStringLiteral( "EXIF_GPSLongitudeRef" ) ).toString() }.right( 1 ).compare( QLatin1String( "W" ), Qt::CaseInsensitive ) == 0
         || metadata.value( QStringLiteral( "EXIF_GPSLongitudeRef" ) ).toDouble() < 0 )
#endif
    {
      x = -x;
    }
  }
  else
  {
    return false;
  }

  double y = 0.0;
  if ( metadata.contains( QStringLiteral( "EXIF_GPSLatitude" ) ) )
  {
    bool ok = false;
    y = metadata.value( QStringLiteral( "EXIF_GPSLatitude" ) ).toDouble( &ok );
    if ( !ok )
      return false;

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    if ( metadata.value( QStringLiteral( "EXIF_GPSLatitudeRef" ) ).toString().rightRef( 1 ).compare( QLatin1String( "S" ), Qt::CaseInsensitive ) == 0
         || metadata.value( QStringLiteral( "EXIF_GPSLatitudeRef" ) ).toDouble() < 0 )
#else
    if ( QStringView { metadata.value( QStringLiteral( "EXIF_GPSLatitudeRef" ) ).toString() }.right( 1 ).compare( QLatin1String( "S" ), Qt::CaseInsensitive ) == 0
         || metadata.value( QStringLiteral( "EXIF_GPSLatitudeRef" ) ).toDouble() < 0 )
#endif
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
  if ( metadata.contains( QStringLiteral( "EXIF_GPSAltitude" ) ) )
  {
    double alt = metadata.value( QStringLiteral( "EXIF_GPSAltitude" ) ).toDouble();
    if ( metadata.contains( QStringLiteral( "EXIF_GPSAltitudeRef" ) ) &&
         ( ( metadata.value( QStringLiteral( "EXIF_GPSAltitudeRef" ) ).type() == QVariant::String && metadata.value( QStringLiteral( "EXIF_GPSAltitudeRef" ) ).toString().right( 1 ) == QLatin1String( "1" ) )
           || metadata.value( QStringLiteral( "EXIF_GPSAltitudeRef" ) ).toDouble() < 0 ) )
      alt = -alt;
    altitude = alt;
  }
  return altitude;
}

QVariant QgsImportPhotosAlgorithm::extractDirectionFromMetadata( const QVariantMap &metadata )
{
  QVariant direction;
  if ( metadata.contains( QStringLiteral( "EXIF_GPSImgDirection" ) ) )
  {
    direction = metadata.value( QStringLiteral( "EXIF_GPSImgDirection" ) ).toDouble();
  }
  return direction;
}

QVariant QgsImportPhotosAlgorithm::extractOrientationFromMetadata( const QVariantMap &metadata )
{
  QVariant orientation;
  if ( metadata.contains( QStringLiteral( "EXIF_Orientation" ) ) )
  {
    switch ( metadata.value( QStringLiteral( "EXIF_Orientation" ) ).toInt() )
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
  if ( metadata.contains( QStringLiteral( "EXIF_DateTimeOriginal" ) ) )
  {
    ts = metadata.value( QStringLiteral( "EXIF_DateTimeOriginal" ) );
  }
  else if ( metadata.contains( QStringLiteral( "EXIF_DateTimeDigitized" ) ) )
  {
    ts = metadata.value( QStringLiteral( "EXIF_DateTimeDigitized" ) );
  }
  else if ( metadata.contains( QStringLiteral( "EXIF_DateTime" ) ) )
  {
    ts = metadata.value( QStringLiteral( "EXIF_DateTime" ) );
  }

  if ( !ts.isValid() )
    return ts;

  const QRegularExpression dsRegEx( QStringLiteral( "(\\d+):(\\d+):(\\d+)\\s+(\\d+):(\\d+):(\\d+)" ) );
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
  const QRegularExpression coordRx( QStringLiteral( "^\\s*\\(\\s*([-\\.\\d]+)\\s*\\)\\s*\\(\\s*([-\\.\\d]+)\\s*\\)\\s*\\(\\s*([-\\.\\d]+)\\s*\\)\\s*$" ) );
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
  const QRegularExpression splitRx( QStringLiteral( "(.*?)=(.*)" ) );
  for ( const QString &item : input )
  {
    const QRegularExpressionMatch match = splitRx.match( item );
    if ( !match.hasMatch() )
      continue;

    const QString tag = match.captured( 1 );
    QVariant value = parseMetadataValue( match.captured( 2 ) );

    if ( tag == QLatin1String( "EXIF_GPSLatitude" ) || tag == QLatin1String( "EXIF_GPSLongitude" ) )
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
      if ( QgsVectorLayer *vl = qobject_cast< QgsVectorLayer * >( layer ) )
      {
        QVariantMap config;
        // photo field shows picture viewer
        config.insert( QStringLiteral( "DocumentViewer" ), 1 );
        config.insert( QStringLiteral( "FileWidget" ), true );
        config.insert( QStringLiteral( "UseLink" ), true );
        config.insert( QStringLiteral( "FullUrl" ), true );
        vl->setEditorWidgetSetup( vl->fields().lookupField( QStringLiteral( "photo" ) ), QgsEditorWidgetSetup( QStringLiteral( "ExternalResource" ), config ) );

        config.clear();
        // path field is a directory link
        config.insert( QStringLiteral( "FileWidgetButton" ), true );
        config.insert( QStringLiteral( "StorageMode" ), 1 );
        config.insert( QStringLiteral( "UseLink" ), true );
        config.insert( QStringLiteral( "FullUrl" ), true );
        vl->setEditorWidgetSetup( vl->fields().lookupField( QStringLiteral( "directory" ) ), QgsEditorWidgetSetup( QStringLiteral( "ExternalResource" ), config ) );
      }
    }
};

QVariantMap QgsImportPhotosAlgorithm::processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  const QString folder = parameterAsFile( parameters, QStringLiteral( "FOLDER" ), context );

  const QDir importDir( folder );
  if ( !importDir.exists() )
  {
    throw QgsProcessingException( QObject::tr( "Directory %1 does not exist!" ).arg( folder ) );
  }

  const bool recurse = parameterAsBoolean( parameters, QStringLiteral( "RECURSIVE" ), context );

  QgsFields outFields;
  outFields.append( QgsField( QStringLiteral( "photo" ), QVariant::String ) );
  outFields.append( QgsField( QStringLiteral( "filename" ), QVariant::String ) );
  outFields.append( QgsField( QStringLiteral( "directory" ), QVariant::String ) );
  outFields.append( QgsField( QStringLiteral( "altitude" ), QVariant::Double ) );
  outFields.append( QgsField( QStringLiteral( "direction" ), QVariant::Double ) );
  outFields.append( QgsField( QStringLiteral( "rotation" ), QVariant::Int ) );
  outFields.append( QgsField( QStringLiteral( "longitude" ), QVariant::String ) );
  outFields.append( QgsField( QStringLiteral( "latitude" ), QVariant::String ) );
  outFields.append( QgsField( QStringLiteral( "timestamp" ), QVariant::DateTime ) );
  QString outputDest;
  std::unique_ptr< QgsFeatureSink > outputSink( parameterAsSink( parameters, QStringLiteral( "OUTPUT" ), context, outputDest, outFields,
      QgsWkbTypes::PointZ, QgsCoordinateReferenceSystem( QStringLiteral( "EPSG:4326" ) ) ) );

  QgsFields invalidFields;
  invalidFields.append( QgsField( QStringLiteral( "photo" ), QVariant::String ) );
  invalidFields.append( QgsField( QStringLiteral( "filename" ), QVariant::String ) );
  invalidFields.append( QgsField( QStringLiteral( "directory" ), QVariant::String ) );
  invalidFields.append( QgsField( QStringLiteral( "readable" ), QVariant::Bool ) );
  QString invalidDest;
  std::unique_ptr< QgsFeatureSink > invalidSink( parameterAsSink( parameters, QStringLiteral( "INVALID" ), context, invalidDest, invalidFields ) );

  const QStringList nameFilters { "*.jpeg", "*.jpg" };
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

  auto saveInvalidFile = [&invalidSink, &parameters]( QgsAttributes & attributes, bool readable )
  {
    if ( invalidSink )
    {
      QgsFeature f;
      attributes.append( readable );
      f.setAttributes( attributes );
      if ( !invalidSink->addFeature( f, QgsFeatureSink::FastInsert ) )
        throw QgsProcessingException( writeFeatureError( invalidSink.get(), parameters, QStringLiteral( "INVALID" ) ) );
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

    if ( char **GDALmetadata = GDALGetMetadata( hDS.get(), nullptr ) )
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
      const QgsGeometry p = QgsGeometry( new QgsPoint( tag.x(), tag.y(), altitude.toDouble(), 0, QgsWkbTypes::PointZ ) );
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
        throw QgsProcessingException( writeFeatureError( outputSink.get(), parameters, QStringLiteral( "OUTPUT" ) ) );
    }
    else
    {
      feedback->reportError( QObject::tr( "No metadata found in %1" ).arg( QDir::toNativeSeparators( file ) ) );
      saveInvalidFile( attributes, true );
    }
  }

  QVariantMap outputs;
  if ( outputSink )
  {
    outputs.insert( QStringLiteral( "OUTPUT" ), outputDest );

    if ( context.willLoadLayerOnCompletion( outputDest ) )
    {
      context.layerToLoadOnCompletionDetails( outputDest ).setPostProcessor( new SetEditorWidgetForPhotoAttributePostProcessor() );
    }
  }

  if ( invalidSink )
    outputs.insert( QStringLiteral( "INVALID" ), invalidDest );
  return outputs;
}

///@endcond
