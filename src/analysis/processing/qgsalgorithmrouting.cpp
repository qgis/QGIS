#include "qgsalgorithmrouting.h"
#include "qgsvectorlayer.h"
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <qgsblockingnetworkrequest.h>
#include <qurlquery.h>

///@cond PRIVATE

QString QgsRoutingAlgorithm::name() const
{
  return QStringLiteral( "routingdirections" );
}

QString QgsRoutingAlgorithm::displayName() const
{
  return QObject::tr( "Directions" );
}

QStringList QgsRoutingAlgorithm::tags() const
{
  return QObject::tr( "routing, openrouteservice" ).split( ',' );
}

QString QgsRoutingAlgorithm::group() const
{
  return QObject::tr( "Routing" );
}

QString QgsRoutingAlgorithm::groupId() const
{
  return QStringLiteral( "routing" );
}

QString QgsRoutingAlgorithm::shortHelpString() const
{
  return QObject::tr( "This algorithm computes a basic route between two points with the profile provided. You need to have a valid API key, sign up at <a href='https://openrouteservice.org/sign-up/'>https://openrouteservice.org/sign-up/</a>. Current <a href='https://openrouteservice.org/restrictions/'>restriction limits</a> for the openrouteservice API apply. For quota information see your Openrouteservice dashboard." );
}

QgsRoutingAlgorithm *QgsRoutingAlgorithm::createInstance() const
{
  return new QgsRoutingAlgorithm();
}

QStringList profiles = QStringList() << "driving-car"
                       << "driving-hgv"
                       << "cycling-regular"
                       << "cycling-road"
                       << "cycling-mountain"
                       << "cycling-electric"
                       << "foot-walking"
                       << "foot-hiking"
                       << "wheelchair";

void QgsRoutingAlgorithm::initAlgorithm( const QVariantMap & )
{

  addParameter( new QgsProcessingParameterString(
                  QStringLiteral( "API_KEY" ),
                  QObject::tr( "API Key" ),
                  QString()
                ) );

  addParameter( new QgsProcessingParameterPoint( QStringLiteral( "START_POINT" ), QObject::tr( "Start point" ) ) );

  addParameter( new QgsProcessingParameterPoint( QStringLiteral( "END_POINT" ), QObject::tr( "End point" ) ) );

  addParameter( new QgsProcessingParameterEnum(
                  QStringLiteral( "IN_PROFILE" ),
                  QObject::tr( "Travel profile" ),
                  profiles,
                  0,
                  "driving-car",
                  false
                ) );

  addParameter( new QgsProcessingParameterFeatureSink( QStringLiteral( "OUTPUT" ),
                QObject::tr( "Route" ), Qgis::ProcessingSourceType::VectorLine ) );
}


bool QgsRoutingAlgorithm::prepareAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback * )
{
  return true;
}


QVariantMap QgsRoutingAlgorithm::processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  QString mEndpoint = QStringLiteral( "https://api.openrouteservice.org/v2/directions/" );
  QString api_key( parameterAsString( parameters, QStringLiteral( "API_KEY" ), context ) );
  int profile_index( parameterAsInt( parameters, QStringLiteral( "IN_PROFILE" ), context ) );

  QgsPointXY startPoint = parameterAsPoint( parameters, QStringLiteral( "START_POINT" ), context );
  QgsPointXY endPoint = parameterAsPoint( parameters, QStringLiteral( "END_POINT" ), context );
  QgsCoordinateReferenceSystem startCrs = parameterAsPointCrs( parameters, QStringLiteral( "START_POINT" ), context );
  QgsCoordinateReferenceSystem endCrs = parameterAsPointCrs( parameters, QStringLiteral( "END_POINT" ), context );

  QgsCoordinateReferenceSystem crs4326( "EPSG:4326" );
  const QgsCoordinateTransform ct( startCrs, crs4326, context.transformContext() );

  if ( startCrs != crs4326 or endCrs != crs4326 )
  {
    try
    {
      startPoint = ct.transform( startPoint );
      endPoint = ct.transform( endPoint );
    }
    catch ( QgsCsException & )
    {
      throw QgsProcessingException( QObject::tr( "Could not transform start/end point to destination CRS" ) );
    }
  }

  QUrl res( mEndpoint );

  res.setPath( res.path() + profiles.at( profile_index ) );

  QUrlQuery query;
  query.addQueryItem( QStringLiteral( "api_key" ), api_key );
  query.addQueryItem( QStringLiteral( "start" ), QString( "%1,%2" ).arg( QString::number( startPoint.x(), 'f', 2 ) ).arg( QString::number( startPoint.y(), 'f', 2 ) ) );
  query.addQueryItem( QStringLiteral( "end" ), QString( "%1,%2" ).arg( QString::number( endPoint.x(), 'f', 2 ) ).arg( QString::number( endPoint.y(), 'f', 2 ) ) );
  res.setQuery( query );

  QNetworkRequest request( res );

  QgsBlockingNetworkRequest newReq;
  const QgsBlockingNetworkRequest::ErrorCode errorCode = newReq.get( request, false, feedback );

  QJsonParseError err;
  const QJsonDocument doc = QJsonDocument::fromJson( newReq.reply().content(), &err );

  QVector<QgsPoint> pathPoints;

  if ( err.error == QJsonParseError::NoError && !doc.isNull() )
  {
    QJsonObject rootObj = doc.object();
    if ( rootObj.contains( "error" ) )
    {
      QJsonObject errorObj = rootObj["error"].toObject();
      QString errorCodeOrs = errorObj["code"].toVariant().toString();
      QString errorMessageOrs = errorObj["message"].toString();
      throw QgsProcessingException(
        QObject::tr( "Error Code: %1\n Error Message: %2" )
        .arg( errorCodeOrs )
        .arg( errorMessageOrs )
      );
    }
    else
    {
      QJsonArray featuresArray = rootObj["features"].toArray();

      for ( const QJsonValue &featureValue : featuresArray )
      {
        QJsonObject featureObj = featureValue.toObject();

        QJsonObject geometryObj = featureObj["geometry"].toObject();

        QJsonArray coordinatesArray = geometryObj["coordinates"].toArray();

        for ( const QJsonValue &coordValue : coordinatesArray )
        {
          QJsonArray coordPair = coordValue.toArray();
          double lon = coordPair[0].toDouble();
          double lat = coordPair[1].toDouble();

          pathPoints.append( QgsPoint( lon, lat ) );
        }
      }
    }
  }
  else
  {
    throw QgsProcessingException(
      QObject::tr( "Failed to parse response JSON: %1" )
      .arg( err.errorString() )
    );
  }

  Qgis::WkbType wkbType = Qgis::WkbType::LineString;
  QgsFields outputFields = QgsFields();
  QString dest;

  std::unique_ptr< QgsFeatureSink > sink( parameterAsSink( parameters, QStringLiteral( "OUTPUT" ), context, dest, outputFields, wkbType, crs4326 ) );
  if ( !sink )
    throw QgsProcessingException( invalidSinkError( parameters, QStringLiteral( "OUTPUT" ) ) );

  QgsFeature outputFeature;
  QgsAttributes attrs;

  outputFeature.setGeometry( QgsGeometry::fromPolyline( pathPoints ) );
  outputFeature.setAttributes( attrs );

  if ( !sink->addFeature( outputFeature, QgsFeatureSink::FastInsert ) )
    throw QgsProcessingException( writeFeatureError( sink.get(), parameters, QStringLiteral( "OUTPUT" ) ) );

  QVariantMap outputs;
  outputs.insert( QStringLiteral( "OUTPUT" ), dest );
  return outputs;
}




///@endcond
