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
#include "qgspointcloudlayer.h"
#include "qgspointcloudlayerelevationproperties.h"
#include "qgspointcloudrequest.h"
#include "qgslogger.h"
#include "qgscircle.h"
#include <mutex>

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

static void _pointXY( QgsPointCloudRenderContext &context, const char *ptr, int i, double &x, double &y )
{
  const qint32 ix = *reinterpret_cast< const qint32 * >( ptr + i * context.pointRecordSize() + context.xOffset() );
  const qint32 iy = *reinterpret_cast< const qint32 * >( ptr + i * context.pointRecordSize() + context.yOffset() );
  x = context.offset().x() + context.scale().x() * ix;
  y = context.offset().y() + context.scale().y() * iy;
}

/**
     * Retrieves the z value for the point at index \a i.
     */
static double _pointZ( QgsPointCloudRenderContext &context, const char *ptr, int i )
{
  const qint32 iz = *reinterpret_cast<const qint32 * >( ptr + i * context.pointRecordSize() + context.zOffset() );
  return context.offset().z() + context.scale().z() * iz;
}

struct MapIndexedPointCloudNode
{
  typedef QVector<QMap<QString, QVariant>> result_type;

  MapIndexedPointCloudNode( QgsPointCloudRequest &request, QgsPointCloudRenderContext &context,
                            QgsGeometry &extentGeometry, const QgsDoubleRange &zRange, QgsPointCloudIndex *index )
    : mRequest( request ), mContext( context ), mExtentGeometry( extentGeometry ), mZRange( zRange ), mIndex( index )
  {

  }

  QVector<QMap<QString, QVariant>> operator()( const IndexedPointCloudNode &n )
  {
    QVector<QMap<QString, QVariant>> acceptedPoints;
    std::unique_ptr<QgsPointCloudBlock> block( mIndex->nodeData( n, mRequest ) );

    if ( !block )
      return acceptedPoints;

    const char *ptr = block->data();
    QgsPointCloudAttributeCollection blockAttributes = block->attributes();
    const std::size_t recordSize = blockAttributes.pointRecordSize();
    mContext.setAttributes( block->attributes() );
    for ( int i = 0; i < block->pointCount(); ++i )
    {
      double x, y, z;
      _pointXY( mContext, ptr, i, x, y );
      z = _pointZ( mContext, ptr, i );
      QgsPointXY pointXY( x, y );

      if ( mExtentGeometry.contains( &pointXY ) && mZRange.contains( z ) )
      {
        QMap<QString, QVariant> pointAttr = mContext.attributeMap( ptr, i * recordSize, blockAttributes );
        pointAttr[ QStringLiteral( "X" ) ] = x;
        pointAttr[ QStringLiteral( "Y" ) ] = y;
        pointAttr[ QStringLiteral( "Z" ) ] = z;
        acceptedPoints.push_back( pointAttr );
      }
    }
    return acceptedPoints;
  };

  QgsPointCloudRequest &mRequest;
  QgsPointCloudRenderContext &mContext;
  QgsGeometry &mExtentGeometry;
  const QgsDoubleRange &mZRange;
  QgsPointCloudIndex *mIndex = nullptr;
};

QVector<QMap<QString, QVariant>> QgsPointCloudDataProvider::identify(
                                float maxErrorInMapCoords,
                                QgsGeometry extentGeometry,
                                const QgsDoubleRange extentZRange )
{
  QVector<QMap<QString, QVariant>> acceptedPoints;

  QgsPointCloudIndex *index = this->index();
  const IndexedPointCloudNode root = index->root();

  QgsRenderContext renderContext;

  QgsRectangle rootNodeExtentMapCoords = index->nodeMapExtent( root );
// TODO? reproject the root node extent
//  try
//  {
//    rootNodeExtentMapCoords = renderContext.coordinateTransform().transformBoundingBox( index->nodeMapExtent( root ) );
//  }
//  catch ( QgsCsException & )
//  {
//    QgsDebugMsg( QStringLiteral( "Could not transform node extent to map CRS" ) );
//  }
  const float rootErrorInMapCoordinates = rootNodeExtentMapCoords.width() / index->span();

  QVector<IndexedPointCloudNode> nodes = traverseTree( index, root, maxErrorInMapCoords, rootErrorInMapCoordinates, extentGeometry, extentZRange );

  QgsPointCloudAttributeCollection attributeCollection = index->attributes();
  QgsPointCloudRequest request;
  request.setAttributes( attributeCollection );

  QgsPointCloudRenderContext context( renderContext, index->scale(), index->offset(), 1.0, 0.0 );

  acceptedPoints = QtConcurrent::blockingMappedReduced( nodes,
                   MapIndexedPointCloudNode( request, context, extentGeometry, extentZRange, index ),
                   qgis::overload<const QVector<QMap<QString, QVariant>>&>::of( &QVector<QMap<QString, QVariant>>::append ),
                   QtConcurrent::UnorderedReduce );

  return acceptedPoints;
}

QVector<IndexedPointCloudNode> QgsPointCloudDataProvider::traverseTree(
  const QgsPointCloudIndex *pc,
  IndexedPointCloudNode n,
  float maxError,
  float nodeError,
  const QgsGeometry &extentGeometry,
  const QgsDoubleRange extentZRange )
{
  QVector<IndexedPointCloudNode> nodes;

  const QgsDoubleRange nodeZRange = pc->nodeZRange( n );
  if ( !extentZRange.overlaps( nodeZRange ) )
    return nodes;

  if ( !extentGeometry.intersects( pc->nodeMapExtent( n ) ) )
    return nodes;

  nodes.append( n );

  float childrenError = nodeError / 2.0f;
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
