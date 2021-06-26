/***************************************************************************
                         qgspdaldataprovider.cpp
                         -----------------------
  Date                 : November 2020
  Copyright            : (C) 2020 by Peter Petrik
  Email                : zilolv at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   *
 *  
 *        *
 *                                     *
 *                                                                         *
 ***************************************************************************/

#include "qgis.h"
#include "qgspdalprovider.h"
#include "qgspdaldataitems.h"
#include "qgsruntimeprofiler.h"
#include "qgsapplication.h"
#include "qgslogger.h"
#include "qgsmessagelog.h"
#include "qgsjsonutils.h"

#include <pdal/io/LasReader.hpp>
#include <pdal/io/LasHeader.hpp>
#include <pdal/Options.hpp>

#define PROVIDER_KEY QStringLiteral( "pdal" )
#define PROVIDER_DESCRIPTION QStringLiteral( "PDAL point cloud data provider" )

QgsPdalProvider::QgsPdalProvider(
  const QString &uri,
  const QgsDataProvider::ProviderOptions &options,
  QgsDataProvider::ReadFlags flags )
  : QgsPointCloudDataProvider( uri, options, flags )
{
  std::unique_ptr< QgsScopedRuntimeProfile > profile;
  if ( QgsApplication::profiler()->groupIsActive( QStringLiteral( "projectload" ) ) )
    profile = qgis::make_unique< QgsScopedRuntimeProfile >( tr( "Open data source" ), QStringLiteral( "projectload" ) );

  mIsValid = load( uri );
}

QgsPdalProvider::~QgsPdalProvider() = default;

QgsCoordinateReferenceSystem QgsPdalProvider::crs() const
{
  return mCrs;
}

QgsRectangle QgsPdalProvider::extent() const
{
  return mExtent;
}

QgsPointCloudAttributeCollection QgsPdalProvider::attributes() const
{
  // TODO
  return QgsPointCloudAttributeCollection();
}

int QgsPdalProvider::pointCount() const
{
  return mPointCount;
}

QVariantMap QgsPdalProvider::originalMetadata() const
{
  return mOriginalMetadata;
}

bool QgsPdalProvider::isValid() const
{
  return mIsValid;
}

QString QgsPdalProvider::name() const
{
  return QStringLiteral( "pdal" );
}

QString QgsPdalProvider::description() const
{
  return QStringLiteral( "Point Clouds PDAL" );
}

QgsPointCloudIndex *QgsPdalProvider::index() const
{
  // TODO automatically generate EPT index
  return nullptr;
}

bool QgsPdalProvider::load( const QString &uri )
{
  try
  {
    pdal::Option las_opt( "filename", uri.toStdString() );
    pdal::Options las_opts;
    las_opts.add( las_opt );
    pdal::LasReader las_reader;
    las_reader.setOptions( las_opts );
    pdal::PointTable table;
    las_reader.prepare( table );
    pdal::LasHeader las_header = las_reader.header();

    const std::string tableMetadata = pdal::Utils::toJSON( table.metadata() );
    const QVariantMap readerMetadata = QgsJsonUtils::parseJson( tableMetadata ).toMap().value( QStringLiteral( "root" ) ).toMap();
    // source metadata is only value present here!
    if ( !readerMetadata.empty() )
      mOriginalMetadata = readerMetadata.constBegin().value().toMap();

    // extent
    /*
    double scale_x = las_header.scaleX();
    double scale_y = las_header.scaleY();
    double scale_z = las_header.scaleZ();

    double offset_x = las_header.offsetX();
    double offset_y = las_header.offsetY();
    double offset_z = las_header.offsetZ();
    */

    double xmin = las_header.minX();
    double xmax = las_header.maxX();
    double ymin = las_header.minY();
    double ymax = las_header.maxY();
    mExtent = QgsRectangle( xmin, ymin, xmax, ymax );

    mPointCount = las_header.pointCount();

    // projection
    QString wkt = QString::fromStdString( las_reader.getSpatialReference().getWKT() );
    mCrs = QgsCoordinateReferenceSystem::fromWkt( wkt );
    return true;
  }
  catch ( pdal::pdal_error &error )
  {
    QgsDebugMsg( QStringLiteral( "Error loading PDAL data source %1" ).arg( error.what() ) );
    QgsMessageLog::logMessage( tr( "Data source is invalid (%1)" ).arg( error.what() ), QStringLiteral( "PDAL" ) );
    return false;
  }
}

QgsPdalProviderMetadata::QgsPdalProviderMetadata():
  QgsProviderMetadata( PROVIDER_KEY, PROVIDER_DESCRIPTION )
{
}

QgsPdalProvider *QgsPdalProviderMetadata::createProvider( const QString &uri, const QgsDataProvider::ProviderOptions &options, QgsDataProvider::ReadFlags flags )
{
  return new QgsPdalProvider( uri, options, flags );
}

QgsProviderMetadata::ProviderMetadataCapabilities QgsPdalProviderMetadata::capabilities() const
{
  return ProviderMetadataCapability::LayerTypesForUri
         | ProviderMetadataCapability::PriorityForUri;
}

QList<QgsDataItemProvider *> QgsPdalProviderMetadata::dataItemProviders() const
{
  QList< QgsDataItemProvider * > providers;
  providers << new QgsPdalDataItemProvider;
  return providers;
}

QVariantMap QgsPdalProviderMetadata::decodeUri( const QString &uri ) const
{
  const QString path = uri;
  QVariantMap uriComponents;
  uriComponents.insert( QStringLiteral( "path" ), path );
  return uriComponents;
}

int QgsPdalProviderMetadata::priorityForUri( const QString &uri ) const
{
  const QVariantMap parts = decodeUri( uri );
  QFileInfo fi( parts.value( QStringLiteral( "path" ) ).toString() );
  if ( fi.suffix().compare( QLatin1String( "las" ), Qt::CaseInsensitive ) == 0 || fi.suffix().compare( QLatin1String( "laz" ), Qt::CaseInsensitive ) == 0 )
    return 100;

  return 0;
}

QList<QgsMapLayerType> QgsPdalProviderMetadata::validLayerTypesForUri( const QString &uri ) const
{
  const QVariantMap parts = decodeUri( uri );
  QFileInfo fi( parts.value( QStringLiteral( "path" ) ).toString() );
  if ( fi.suffix().compare( QLatin1String( "las" ), Qt::CaseInsensitive ) == 0 || fi.suffix().compare( QLatin1String( "laz" ), Qt::CaseInsensitive ) == 0 )
    return QList<QgsMapLayerType>() << QgsMapLayerType::PointCloudLayer;

  return QList<QgsMapLayerType>();
}

QString QgsPdalProviderMetadata::filters( QgsProviderMetadata::FilterType type )
{
  switch ( type )
  {
    case QgsProviderMetadata::FilterType::FilterVector:
    case QgsProviderMetadata::FilterType::FilterRaster:
    case QgsProviderMetadata::FilterType::FilterMesh:
    case QgsProviderMetadata::FilterType::FilterMeshDataset:
      return QString();

    case QgsProviderMetadata::FilterType::FilterPointCloud:
      // TODO get the available/supported filters from PDAL library
      return QObject::tr( "PDAL Point Clouds" ) + QStringLiteral( " (*.laz *.las)" );
  }
  return QString();
}

QString QgsPdalProviderMetadata::encodeUri( const QVariantMap &parts ) const
{
  const QString path = parts.value( QStringLiteral( "path" ) ).toString();
  return path;
}

QGISEXTERN QgsProviderMetadata *providerMetadataFactory()
{
  return new QgsPdalProviderMetadata();
}
