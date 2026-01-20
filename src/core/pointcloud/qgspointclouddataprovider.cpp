/***************************************************************************
                         qgspointclouddataprovider.cpp
                         -----------------------
    begin                : October 2020
    copyright            : (C) 2020 by Peter Petrik
    email                : zilolv at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgspointclouddataprovider.h"

#include <mutex>

#include "qgis.h"
#include "qgscopcpointcloudindex.h"
#include "qgsgeometry.h"
#include "qgsgeos.h"
#include "qgspointcloudindex.h"
#include "qgspointcloudrequest.h"
#include "qgspointcloudstatscalculator.h"
#include "qgsthreadingutils.h"

#include <QDebug>
#include <QtConcurrent/QtConcurrentMap>
#include <QtMath>

#include "moc_qgspointclouddataprovider.cpp"

QgsPointCloudDataProvider::QgsPointCloudDataProvider(
  const QString &uri,
  const QgsDataProvider::ProviderOptions &options,
  Qgis::DataProviderReadFlags flags )
  : QgsDataProvider( uri, options, flags )
{
}

QgsPointCloudDataProvider::~QgsPointCloudDataProvider() = default;

QgsPointCloudDataProvider::Capabilities QgsPointCloudDataProvider::capabilities() const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  return QgsPointCloudDataProvider::NoCapabilities;
}

bool QgsPointCloudDataProvider::hasValidIndex() const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  QgsPointCloudIndex lIndex = index();
  return lIndex.isValid();
}

QgsGeometry QgsPointCloudDataProvider::polygonBounds() const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  return QgsGeometry::fromRect( extent() );
}

QVariantMap QgsPointCloudDataProvider::originalMetadata() const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  return QVariantMap();
}

QgsPointCloudRenderer *QgsPointCloudDataProvider::createRenderer( const QVariantMap & ) const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  return nullptr;
}

QMap<int, QString> QgsPointCloudDataProvider::lasClassificationCodes()
{
  static QMap< int, QString > sCodes
  {
    {0, u"Created, Never Classified"_s},
    {1, u"Unclassified"_s},
    {2, u"Ground"_s},
    {3, u"Low Vegetation"_s},
    {4, u"Medium Vegetation"_s},
    {5, u"High Vegetation"_s},
    {6, u"Building"_s},
    {7, u"Low Point (Low Noise)"_s},
    {8, u"Reserved"_s},
    {9, u"Water"_s},
    {10, u"Rail"_s},
    {11, u"Road Surface"_s},
    {12, u"Reserved"_s},
    {13, u"Wire - Guard (Shield)"_s},
    {14, u"Wire - Conductor (Phase)"_s},
    {15, u"Transmission Tower"_s},
    {16, u"Wire-Structure Connector (Insulator)"_s},
    {17, u"Bridge Deck"_s},
    {18, u"High Noise"_s},
  };

  static std::once_flag initialized;
  std::call_once( initialized, []( )
  {
    for ( int i = 19; i <= 63; ++i )
      sCodes.insert( i, u"Reserved"_s );
    for ( int i = 64; i <= 255; ++i )
      sCodes.insert( i, u"User Definable"_s );
  } );

  return sCodes;
}

QMap<int, QString> QgsPointCloudDataProvider::translatedLasClassificationCodes()
{
  static QMap< int, QString > sCodes
  {
    {0, QObject::tr( "Created, Never Classified" )},
    {1, QObject::tr( "Unclassified" )},
    {2, QObject::tr( "Ground" )},
    {3, QObject::tr( "Low Vegetation" )},
    {4, QObject::tr( "Medium Vegetation" )},
    {5, QObject::tr( "High Vegetation" )},
    {6, QObject::tr( "Building" )},
    {7, QObject::tr( "Low Point (Noise)" )},
    {8, QObject::tr( "Reserved" )},
    {9, QObject::tr( "Water" )},
    {10, QObject::tr( "Rail" )},
    {11, QObject::tr( "Road Surface" )},
    {12, QObject::tr( "Reserved" )},
    {13, QObject::tr( "Wire - Guard (Shield)" )},
    {14, QObject::tr( "Wire - Conductor (Phase)" )},
    {15, QObject::tr( "Transmission Tower" )},
    {16, QObject::tr( "Wire-Structure Connector (Insulator)" )},
    {17, QObject::tr( "Bridge Deck" )},
    {18, QObject::tr( "High Noise" )},
  };

  static std::once_flag initialized;
  std::call_once( initialized, []( )
  {
    for ( int i = 19; i <= 63; ++i )
      sCodes.insert( i, QObject::tr( "Reserved" ) );
    for ( int i = 64; i <= 255; ++i )
      sCodes.insert( i, QObject::tr( "User Definable" ) );
  } );

  return sCodes;
}

QMap<int, QString> QgsPointCloudDataProvider::dataFormatIds()
{
  static const QMap< int, QString > sCodes
  {
    {0, u"No color or time stored"_s},
    {1, u"Time is stored"_s},
    {2, u"Color is stored"_s},
    {3, u"Color and time are stored"_s},
    {6, u"Time is stored"_s},
    {7, u"Time and color are stored)"_s},
    {8, u"Time, color and near infrared are stored"_s},
  };

  return sCodes;
}

QMap<int, QString> QgsPointCloudDataProvider::translatedDataFormatIds()
{
  static const QMap< int, QString > sCodes
  {
    {0, QObject::tr( "No color or time stored" )},
    {1, QObject::tr( "Time is stored" )},
    {2, QObject::tr( "Color is stored" )},
    {3, QObject::tr( "Color and time are stored" )},
    {6, QObject::tr( "Time is stored" )},
    {7, QObject::tr( "Time and color are stored)" )},
    {8, QObject::tr( "Time, color and near infrared are stored" )},
  };

  return sCodes;
}

QgsPointCloudStatistics QgsPointCloudDataProvider::metadataStatistics()
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  QgsPointCloudIndex pcIndex = index();
  if ( pcIndex )
  {
    return pcIndex.metadataStatistics();
  }
  return QgsPointCloudStatistics();
}

bool QgsPointCloudDataProvider::supportsSubsetString() const
{
  return true;
}

QString QgsPointCloudDataProvider::subsetStringDialect() const
{
  return tr( "QGIS expression" );
}

QString QgsPointCloudDataProvider::subsetStringHelpUrl() const
{
  // unfortunately we can't access QgsHelp here, that's a GUI class!
  return QString();
}

struct MapIndexedPointCloudNode
{
  typedef QVector<QMap<QString, QVariant>> result_type;

  MapIndexedPointCloudNode( QgsPointCloudRequest &request, const QgsVector3D &indexScale, const QgsVector3D &indexOffset,
                            const QgsGeometry &extentGeometry, const QgsDoubleRange &zRange, QgsPointCloudIndex index, int pointsLimit )
    : mRequest( request ), mIndexScale( indexScale ), mIndexOffset( indexOffset ), mExtentGeometry( extentGeometry ), mZRange( zRange ), mIndex( std::move( index ) ), mPointsLimit( pointsLimit )
  { }

  QVector<QVariantMap> operator()( QgsPointCloudNodeId n )
  {
    QVector<QVariantMap> acceptedPoints;
    std::unique_ptr<QgsPointCloudBlock> block( mIndex.nodeData( n, mRequest ) );

    if ( !block || pointsCount == mPointsLimit )
      return acceptedPoints;

    const char *ptr = block->data();
    const QgsPointCloudAttributeCollection blockAttributes = block->attributes();
    const std::size_t recordSize = blockAttributes.pointRecordSize();
    int xOffset = 0, yOffset = 0, zOffset = 0;
    const QgsPointCloudAttribute::DataType xType = blockAttributes.find( u"X"_s, xOffset )->type();
    const QgsPointCloudAttribute::DataType yType = blockAttributes.find( u"Y"_s, yOffset )->type();
    const QgsPointCloudAttribute::DataType zType = blockAttributes.find( u"Z"_s, zOffset )->type();
    auto extentEngine = std::make_unique< QgsGeos >( mExtentGeometry.constGet() );
    extentEngine->prepareGeometry();

    std::optional<bool> copcTimeFlag = std::nullopt;
    QVariantMap extraMetadata = mIndex.extraMetadata();
    if ( extraMetadata.contains( u"CopcGpsTimeFlag"_s ) )
      copcTimeFlag = extraMetadata[ u"CopcGpsTimeFlag"_s ].toBool();

    for ( int i = 0; i < block->pointCount() && pointsCount < mPointsLimit; ++i )
    {
      double x, y, z;
      QgsPointCloudAttribute::getPointXYZ( ptr, i, recordSize, xOffset, xType, yOffset, yType, zOffset, zType, block->scale(), block->offset(), x, y, z );

      if ( mZRange.contains( z ) && extentEngine->contains( x, y ) )
      {
        QVariantMap pointAttr = QgsPointCloudAttribute::getAttributeMap( ptr, i * recordSize, blockAttributes );
        pointAttr[ u"X"_s ] = x;
        pointAttr[ u"Y"_s ] = y;
        pointAttr[ u"Z"_s ] = z;


        if ( copcTimeFlag.has_value() )
        {
          const QDateTime gpsBaseTime = QDateTime::fromSecsSinceEpoch( 315964809, Qt::UTC );
          constexpr int numberOfSecsInWeek = 3600 * 24 * 7;
          // here we check the flag set in header to determine if we need to
          // parse the time as GPS week time or GPS adjusted standard time
          // however often times the flag is set wrong, so we determine if the value is bigger than the maximum amount of seconds in week then it has to be adjusted standard time
          if ( *copcTimeFlag || pointAttr[u"GpsTime"_s].toDouble() > numberOfSecsInWeek )
          {
            const QString utcTime = gpsBaseTime.addSecs( static_cast<qint64>( pointAttr[u"GpsTime"_s].toDouble() + 1e9 ) ).toString( Qt::ISODate );
            pointAttr[ u"GpsTime (raw)"_s] = pointAttr[u"GpsTime"_s];
            pointAttr[ u"GpsTime"_s] = utcTime;
          }
          else
          {
            const QString weekTime = gpsBaseTime.addSecs( pointAttr[u"GpsTime"_s].toLongLong() ).toString( "ddd hh:mm:ss" );
            pointAttr[ u"GpsTime (raw)"_s] = pointAttr[u"GpsTime"_s];
            pointAttr[ u"GpsTime"_s] = weekTime;
          }
        }
        pointsCount++;
        acceptedPoints.push_back( pointAttr );
      }
    }
    return acceptedPoints;
  }

  QgsPointCloudRequest &mRequest;
  QgsVector3D mIndexScale;
  QgsVector3D mIndexOffset;
  const QgsGeometry &mExtentGeometry;
  const QgsDoubleRange &mZRange;
  QgsPointCloudIndex mIndex;
  int mPointsLimit;
  int pointsCount = 0;
};

QVector<QVariantMap> QgsPointCloudDataProvider::identify(
  double maxError,
  const QgsGeometry &extentGeometry,
  const QgsDoubleRange &extentZRange, int pointsLimit )
{
  QVector<QVariantMap> acceptedPoints;

  // Try sub-indexes first
  for ( QgsPointCloudSubIndex &subidx : subIndexes() )
  {
    // Check if the sub-index is relevant and if it is loaded. We shouldn't
    // need to identify points in unloaded indices.
    QgsPointCloudIndex index = subidx.index();
    if ( !index
         || ( !subidx.zRange().overlaps( extentZRange ) )
         || !subidx.polygonBounds().intersects( extentGeometry ) )
      continue;
    acceptedPoints.append( identify( index, maxError, extentGeometry, extentZRange, pointsLimit ) );
  }

  // Then look at main index
  QgsPointCloudIndex mainIndex = index();
  acceptedPoints.append( identify( mainIndex, maxError, extentGeometry, extentZRange, pointsLimit ) );

  return acceptedPoints;
}

QVector<QVariantMap> QgsPointCloudDataProvider::identify(
  QgsPointCloudIndex &index, double maxError,
  const QgsGeometry &extentGeometry,
  const QgsDoubleRange &extentZRange, int pointsLimit )
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  QVector<QVariantMap> acceptedPoints;

  if ( !index.isValid() )
    return acceptedPoints;

  const QgsPointCloudNode root = index.getNode( index.root() );
  const QVector<QgsPointCloudNodeId> nodes = traverseTree( index, root, maxError, root.error(), extentGeometry, extentZRange );

  const QgsPointCloudAttributeCollection attributeCollection = index.attributes();
  QgsPointCloudRequest request;
  request.setAttributes( attributeCollection );

  acceptedPoints = QtConcurrent::blockingMappedReduced( nodes,
                   MapIndexedPointCloudNode( request, index.scale(), index.offset(), extentGeometry, extentZRange, index, pointsLimit ),
                   qOverload<const QVector<QMap<QString, QVariant>>&>( &QVector<QMap<QString, QVariant>>::append ),
                   QtConcurrent::UnorderedReduce );

  return acceptedPoints;
}

QVector<QgsPointCloudNodeId> QgsPointCloudDataProvider::traverseTree(
  const QgsPointCloudIndex &pc,
  QgsPointCloudNode node,
  double maxError,
  double nodeError,
  const QgsGeometry &extentGeometry,
  const QgsDoubleRange &extentZRange )
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  QVector<QgsPointCloudNodeId> nodes;

  const QgsBox3D nodeBounds = node.bounds();
  const QgsDoubleRange nodeZRange( nodeBounds.zMinimum(), nodeBounds.zMaximum() );
  if ( !extentZRange.overlaps( nodeZRange ) )
    return nodes;

  if ( !extentGeometry.intersects( nodeBounds.toRectangle() ) )
    return nodes;

  nodes.append( node.id() );

  const double childrenError = nodeError / 2.0;
  if ( childrenError < maxError )
    return nodes;

  for ( const QgsPointCloudNodeId &nn : node.children() )
  {
    const QgsPointCloudNode childNode = pc.getNode( nn );
    if ( extentGeometry.intersects( childNode.bounds().toRectangle() ) )
      nodes += traverseTree( pc, childNode, maxError, childrenError, extentGeometry, extentZRange );
  }

  return nodes;
}

bool QgsPointCloudDataProvider::setSubsetString( const QString &subset, bool updateFeatureCount )
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  Q_UNUSED( updateFeatureCount )
  QgsPointCloudIndex i = index();
  if ( !i )
    return false;

  if ( !i.setSubsetString( subset ) )
    return false;
  mSubsetString = subset;
  emit dataChanged();
  return true;
}

QString QgsPointCloudDataProvider::subsetString() const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  return mSubsetString;
}

