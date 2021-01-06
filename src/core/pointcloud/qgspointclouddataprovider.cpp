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

#include "qgis.h"
#include "qgspointclouddataprovider.h"
#include "qgspointcloudindex.h"
#include "qgsgeometry.h"
#include "qgspointcloudrequest.h"
#include "qgsgeometryengine.h"
#include <mutex>
#include <QDebug>
#include <QtMath>

#include <QtConcurrent/QtConcurrentMap>

QgsPointCloudDataProvider::QgsPointCloudDataProvider(
  const QString &uri,
  const QgsDataProvider::ProviderOptions &options,
  QgsDataProvider::ReadFlags flags )
  : QgsDataProvider( uri, options, flags )
{
}

QgsPointCloudDataProvider::~QgsPointCloudDataProvider() = default;

QgsPointCloudDataProvider::Capabilities QgsPointCloudDataProvider::capabilities() const
{
  return QgsPointCloudDataProvider::NoCapabilities;
}

bool QgsPointCloudDataProvider::hasValidIndex() const
{
  return index() && index()->isValid();
}

QgsGeometry QgsPointCloudDataProvider::polygonBounds() const
{
  return QgsGeometry::fromRect( extent() );
}

QVariantMap QgsPointCloudDataProvider::originalMetadata() const
{
  return QVariantMap();
}

QgsPointCloudRenderer *QgsPointCloudDataProvider::createRenderer( const QVariantMap & ) const
{
  return nullptr;
}

QMap<int, QString> QgsPointCloudDataProvider::lasClassificationCodes()
{
  static QMap< int, QString > sCodes
  {
    {0, QStringLiteral( "Created, Never Classified" )},
    {1, QStringLiteral( "Unclassified" )},
    {2, QStringLiteral( "Ground" )},
    {3, QStringLiteral( "Low Vegetation" )},
    {4, QStringLiteral( "Medium Vegetation" )},
    {5, QStringLiteral( "High Vegetation" )},
    {6, QStringLiteral( "Building" )},
    {7, QStringLiteral( "Low Point (Low Noise)" )},
    {8, QStringLiteral( "Reserved" )},
    {9, QStringLiteral( "Water" )},
    {10, QStringLiteral( "Rail" )},
    {11, QStringLiteral( "Road Surface" )},
    {12, QStringLiteral( "Reserved" )},
    {13, QStringLiteral( "Wire - Guard (Shield)" )},
    {14, QStringLiteral( "Wire - Conductor (Phase)" )},
    {15, QStringLiteral( "Transmission Tower" )},
    {16, QStringLiteral( "Wire-Structure Connector (Insulator)" )},
    {17, QStringLiteral( "Bridge Deck" )},
    {18, QStringLiteral( "High Noise" )},
  };

  static std::once_flag initialized;
  std::call_once( initialized, [ = ]( )
  {
    for ( int i = 19; i <= 63; ++i )
      sCodes.insert( i, QStringLiteral( "Reserved" ) );
    for ( int i = 64; i <= 255; ++i )
      sCodes.insert( i, QStringLiteral( "User Definable" ) );
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
  std::call_once( initialized, [ = ]( )
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
  static QMap< int, QString > sCodes
  {
    {0, QStringLiteral( "No color or time stored" )},
    {1, QStringLiteral( "Time is stored" )},
    {2, QStringLiteral( "Color is stored" )},
    {3, QStringLiteral( "Color and time are stored" )},
    {6, QStringLiteral( "Time is stored" )},
    {7, QStringLiteral( "Time and color are stored)" )},
    {8, QStringLiteral( "Time, color and near infrared are stored" )},
  };

  return sCodes;
}

QMap<int, QString> QgsPointCloudDataProvider::translatedDataFormatIds()
{
  static QMap< int, QString > sCodes
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

QVariant QgsPointCloudDataProvider::metadataStatistic( const QString &, QgsStatisticalSummary::Statistic ) const
{
  return QVariant();
}

QVariantList QgsPointCloudDataProvider::metadataClasses( const QString & ) const
{
  return QVariantList();
}

QVariant QgsPointCloudDataProvider::metadataClassStatistic( const QString &, const QVariant &, QgsStatisticalSummary::Statistic ) const
{
  return QVariant();
}

template <typename T>
void _attribute( const char *data, std::size_t offset, QgsPointCloudAttribute::DataType type, T &value )
{
  switch ( type )
  {
    case QgsPointCloudAttribute::Char:
      value = *( data + offset );
      break;

    case QgsPointCloudAttribute::Int32:
      value = *reinterpret_cast< const qint32 * >( data + offset );
      break;

    case QgsPointCloudAttribute::Short:
    {
      value = *reinterpret_cast< const short * >( data + offset );
    }
    break;

    case QgsPointCloudAttribute::UShort:
      value = *reinterpret_cast< const unsigned short * >( data + offset );
      break;

    case QgsPointCloudAttribute::Float:
      value = static_cast< T >( *reinterpret_cast< const float * >( data + offset ) );
      break;

    case QgsPointCloudAttribute::Double:
      value = *reinterpret_cast< const double * >( data + offset );
      break;
  }
}

/**
* Retrieves the x, y, z values for the point at index \a i.
*/
static void _pointXYZ( const char *ptr, int i, std::size_t pointRecordSize, int xOffset, QgsPointCloudAttribute::DataType xType,
                       int yOffset, QgsPointCloudAttribute::DataType yType,
                       int zOffset, QgsPointCloudAttribute::DataType zType,
                       const QgsVector3D &indexScale, const QgsVector3D &indexOffset, double &x, double &y, double &z )
{
  _attribute( ptr, i * pointRecordSize + xOffset, xType, x );
  x = indexOffset.x() + indexScale.x() * x;

  _attribute( ptr, i * pointRecordSize + yOffset, yType, y );
  y = indexOffset.y() + indexScale.y() * y;

  _attribute( ptr, i * pointRecordSize + zOffset, zType, z );
  z = indexOffset.z() + indexScale.z() * z;
}

/**
* Retrieves all the attributes of a point
*/
QVariantMap _attributeMap( const char *data, std::size_t recordOffset, const QgsPointCloudAttributeCollection &attributeCollection )
{
  QVariantMap map;
  const QVector<QgsPointCloudAttribute> attributes = attributeCollection.attributes();
  for ( const QgsPointCloudAttribute &attr : attributes )
  {
    QString attributeName = attr.name();
    int attributeOffset;
    attributeCollection.find( attributeName, attributeOffset );
    switch ( attr.type() )
    {
      case QgsPointCloudAttribute::Char:
      {
        const char value = *( data + recordOffset + attributeOffset );
        map[ attributeName ] = value;
      }
      break;

      case QgsPointCloudAttribute::Int32:
      {
        const qint32 value = *reinterpret_cast< const qint32 * >( data + recordOffset + attributeOffset );
        map[ attributeName ] = value;
      }
      break;

      case QgsPointCloudAttribute::Short:
      {
        const short value = *reinterpret_cast< const short * >( data + recordOffset + attributeOffset );
        map[ attributeName ] = value;
      }
      break;

      case QgsPointCloudAttribute::UShort:
      {
        const unsigned short value = *reinterpret_cast< const unsigned short * >( data + recordOffset + attributeOffset );
        map[ attributeName ] = value;
      }
      break;

      case QgsPointCloudAttribute::Float:
      {
        const float value = *reinterpret_cast< const float * >( data + recordOffset + attributeOffset );
        map[ attributeName ] = value;
      }
      break;

      case QgsPointCloudAttribute::Double:
      {
        const double value = *reinterpret_cast< const double * >( data + recordOffset + attributeOffset );
        map[ attributeName ] = value;
      }
      break;
    }
  }
  return map;
}

struct MapIndexedPointCloudNode
{
  typedef QVector<QMap<QString, QVariant>> result_type;

  MapIndexedPointCloudNode( QgsPointCloudRequest &request, const QgsVector3D &indexScale, const QgsVector3D &indexOffset,
                            const QgsGeometry &extentGeometry, const QgsDoubleRange &zRange, QgsPointCloudIndex *index, int pointsLimit )
    : mRequest( request ), mIndexScale( indexScale ), mIndexOffset( indexOffset ), mExtentGeometry( extentGeometry ), mZRange( zRange ), mIndex( index ), mPointsLimit( pointsLimit )
  { }

  QVector<QVariantMap> operator()( IndexedPointCloudNode n )
  {
    QVector<QVariantMap> acceptedPoints;
    std::unique_ptr<QgsPointCloudBlock> block( mIndex->nodeData( n, mRequest ) );

    if ( !block || pointsCount == mPointsLimit )
      return acceptedPoints;

    const char *ptr = block->data();
    QgsPointCloudAttributeCollection blockAttributes = block->attributes();
    const std::size_t recordSize = blockAttributes.pointRecordSize();
    int xOffset, yOffset, zOffset;
    const QgsPointCloudAttribute::DataType xType = blockAttributes.find( QStringLiteral( "X" ), xOffset )->type();
    const QgsPointCloudAttribute::DataType yType = blockAttributes.find( QStringLiteral( "Y" ), yOffset )->type();
    const QgsPointCloudAttribute::DataType zType = blockAttributes.find( QStringLiteral( "Z" ), zOffset )->type();
    std::unique_ptr< QgsGeometryEngine > extentEngine( QgsGeometry::createGeometryEngine( mExtentGeometry.constGet() ) );
    extentEngine->prepareGeometry();
    for ( int i = 0; i < block->pointCount() && pointsCount < mPointsLimit; ++i )
    {
      double x, y, z;
      _pointXYZ( ptr, i, recordSize, xOffset, xType, yOffset, yType, zOffset, zType, mIndexScale, mIndexOffset, x, y, z );
      QgsPoint pointXY( x, y );

      if ( mZRange.contains( z ) && extentEngine->contains( &pointXY ) )
      {
        QVariantMap pointAttr = _attributeMap( ptr, i * recordSize, blockAttributes );
        pointAttr[ QStringLiteral( "X" ) ] = x;
        pointAttr[ QStringLiteral( "Y" ) ] = y;
        pointAttr[ QStringLiteral( "Z" ) ] = z;
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
  QgsPointCloudIndex *mIndex = nullptr;
  int mPointsLimit;
  int pointsCount = 0;
};

QVector<QVariantMap> QgsPointCloudDataProvider::identify(
  double maxError,
  const QgsGeometry &extentGeometry,
  const QgsDoubleRange &extentZRange, int pointsLimit )
{
  QVector<QVariantMap> acceptedPoints;

  QgsPointCloudIndex *index = this->index();
  const IndexedPointCloudNode root = index->root();

  QgsRectangle rootNodeExtent = index->nodeMapExtent( root );
  const double rootError = rootNodeExtent.width() / index->span();

  QVector<IndexedPointCloudNode> nodes = traverseTree( index, root, maxError, rootError, extentGeometry, extentZRange );

  QgsPointCloudAttributeCollection attributeCollection = index->attributes();
  QgsPointCloudRequest request;
  request.setAttributes( attributeCollection );

  acceptedPoints = QtConcurrent::blockingMappedReduced( nodes,
                   MapIndexedPointCloudNode( request, index->scale(), index->offset(), extentGeometry, extentZRange, index, pointsLimit ),
                   qgis::overload<const QVector<QMap<QString, QVariant>>&>::of( &QVector<QMap<QString, QVariant>>::append ),
                   QtConcurrent::UnorderedReduce );

  return acceptedPoints;
}

QVector<IndexedPointCloudNode> QgsPointCloudDataProvider::traverseTree(
  const QgsPointCloudIndex *pc,
  IndexedPointCloudNode n,
  double maxError,
  double nodeError,
  const QgsGeometry &extentGeometry,
  const QgsDoubleRange &extentZRange )
{
  QVector<IndexedPointCloudNode> nodes;

  const QgsDoubleRange nodeZRange = pc->nodeZRange( n );
  if ( !extentZRange.overlaps( nodeZRange ) )
    return nodes;

  if ( !extentGeometry.intersects( pc->nodeMapExtent( n ) ) )
    return nodes;

  nodes.append( n );

  double childrenError = nodeError / 2.0;
  if ( childrenError < maxError )
    return nodes;

  const QList<IndexedPointCloudNode> children = pc->nodeChildren( n );
  for ( const IndexedPointCloudNode &nn : children )
  {
    if ( extentGeometry.intersects( pc->nodeMapExtent( nn ) ) )
      nodes += traverseTree( pc, nn, maxError, childrenError, extentGeometry, extentZRange );
  }

  return nodes;
}

QVector<QVariantMap> QgsPointCloudDataProvider::getPointsOnRay( const QVector3D &rayOrigin, const QVector3D &rayDirection, double maxScreenError, double cameraFov, int screenSizePx, double pointAngle )
{
  QVector<QVariantMap> points;
  QgsPointCloudIndex *index = this->index();
  IndexedPointCloudNode root = index->root();
  QgsRectangle rootNodeExtentMapCoords = index->nodeMapExtent( root );
  const float rootErrorInMapCoordinates = rootNodeExtentMapCoords.width() / index->span();

  QVector<IndexedPointCloudNode> nodes = getNodesIntersectingWithRay( index, root, maxScreenError, rootErrorInMapCoordinates, cameraFov, screenSizePx, rayOrigin, rayDirection );

  QgsPointCloudAttributeCollection attributeCollection = index->attributes();
  QgsPointCloudRequest request;
  request.setAttributes( attributeCollection );

  for ( IndexedPointCloudNode n : nodes )
  {
    std::unique_ptr<QgsPointCloudBlock> block( index->nodeData( n, request ) );

    if ( !block )
      continue;
    const char *ptr = block->data();
    QgsPointCloudAttributeCollection blockAttributes = block->attributes();
    const std::size_t recordSize = blockAttributes.pointRecordSize();
    int xOffset, yOffset, zOffset;
    blockAttributes.find( QStringLiteral( "X" ), xOffset );
    blockAttributes.find( QStringLiteral( "Y" ), yOffset );
    blockAttributes.find( QStringLiteral( "Z" ), zOffset );
    for ( int i = 0; i < block->pointCount(); ++i )
    {
      double x, y, z;
      _pointXY( ptr, i, recordSize, xOffset, yOffset, index->scale(), index->offset(), x, y );
      z = _pointZ( ptr, i, recordSize, zOffset, index->scale(), index->offset() );
      QVector3D point( x, y, z );
      // project point on ray
      QVector3D projectedPoint = rayOrigin + QgsVector3D::dotProduct( point - rayOrigin, rayDirection ) * rayDirection;
      // check whether point is in front of the ray
      bool isInFront = QgsVector3D::dotProduct( point - rayOrigin, rayDirection ) > 0.0;

      if ( !isInFront )
        continue;

      // calculate the angle between the point and the projected point
      QVector3D v1 = ( projectedPoint - rayOrigin ).normalized();
      QVector3D v2 = ( point - rayOrigin ).normalized();
      double angle = qRadiansToDegrees( std::acos( std::abs( QVector3D::dotProduct( v1, v2 ) ) ) );

      if ( angle > pointAngle )
        continue;

      QVariantMap pointAttr = _attributeMap( ptr, i * recordSize, blockAttributes );
      pointAttr[ QStringLiteral( "X" ) ] = x;
      pointAttr[ QStringLiteral( "Y" ) ] = y;
      pointAttr[ QStringLiteral( "Z" ) ] = z;
      points.push_back( pointAttr );
    }
  }

  return points;
}

bool __boxIntesects( const QgsBox3d &box, const QVector3D &rayOrigin, const QVector3D &rayDirection )
{
  double tminX = box.xMinimum() - rayOrigin.x(), tmaxX = box.xMaximum() - rayOrigin.x();
  double tminY = box.yMinimum() - rayOrigin.y(), tmaxY = box.yMaximum() - rayOrigin.y();
  double tminZ = box.zMinimum() - rayOrigin.z(), tmaxZ = box.zMaximum() - rayOrigin.z();
  if ( rayDirection.x() < 0 ) std::swap( tminX, tmaxX );
  if ( rayDirection.y() < 0 ) std::swap( tminY, tmaxY );
  if ( rayDirection.z() < 0 ) std::swap( tminZ, tmaxZ );
  if ( rayDirection.x() != 0 )
  {
    tminX /= rayDirection.x();
    tmaxX /= rayDirection.x();
  }
  else
  {
    tminX = std::numeric_limits<double>::lowest();
    tmaxX = std::numeric_limits<double>::max();
  }
  if ( rayDirection.y() != 0 )
  {
    tminY /= rayDirection.y();
    tmaxY /= rayDirection.y();
  }
  else
  {
    tminY = std::numeric_limits<double>::lowest();
    tmaxY = std::numeric_limits<double>::max();
  }
  if ( rayDirection.z() != 0 )
  {
    tminZ /= rayDirection.z();
    tmaxZ /= rayDirection.z();
  }
  else
  {
    tminZ = std::numeric_limits<double>::lowest();
    tmaxZ = std::numeric_limits<double>::max();
  }
  QgsDoubleRange tRange( std::max( std::max( tminX, tminY ), tminZ ), std::min( std::min( tmaxX, tmaxY ), tmaxZ ) );
  return !tRange.isEmpty();
}

QVector<IndexedPointCloudNode> QgsPointCloudDataProvider::getNodesIntersectingWithRay(
  const QgsPointCloudIndex *pc, IndexedPointCloudNode n,
  double maxError, double nodeError, double cameraFov, int screenSizePx,
  const QVector3D &rayOrigin, const QVector3D &rayDirection )
{
  QVector<IndexedPointCloudNode> nodes;

  QgsRectangle n2DExtent = pc->nodeMapExtent( n );
  QgsDoubleRange zRange = pc->nodeZRange( n );
  QgsBox3d box( n2DExtent.xMinimum(), n2DExtent.yMinimum(), zRange.lower(), n2DExtent.xMaximum(), n2DExtent.yMaximum(), zRange.upper() );

  // calculate screen space error:
  float distance = box.distanceFromPoint( rayOrigin.x(), rayOrigin.y(), rayOrigin.z() );
  float phi = nodeError * screenSizePx / ( 2 * distance * tan( cameraFov * M_PI / ( 2 * 180 ) ) );

  if ( !__boxIntesects( box, rayOrigin, rayDirection ) )
  {
    return nodes;
  }

  nodes.append( n );

  float childrenError = nodeError / 2.0f;
  if ( phi < maxError )
    return nodes;

  const QList<IndexedPointCloudNode> children = pc->nodeChildren( n );
  for ( const IndexedPointCloudNode &nn : children )
  {
    nodes += getNodesIntersectingWithRay( pc, nn, maxError, childrenError, cameraFov, screenSizePx, rayOrigin, rayDirection );
  }

  return nodes;
}
