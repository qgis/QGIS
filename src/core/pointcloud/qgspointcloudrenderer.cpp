/***************************************************************************
                         qgspointcloudrenderer.cpp
                         --------------------
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

#include "qgspointcloudrenderer.h"
#include "qgspointcloudrendererregistry.h"
#include "qgsapplication.h"
#include "qgssymbollayerutils.h"
#include "qgspointcloudlayer.h"
#include "qgspointcloudindex.h"
#include "qgspointcloudlayerelevationproperties.h"
#include "qgslogger.h"
#include "qgscircle.h"
#include <QThread>
#include <QPointer>

QgsPointCloudRenderContext::QgsPointCloudRenderContext( QgsRenderContext &context, const QgsVector3D &scale, const QgsVector3D &offset, double zValueScale, double zValueFixedOffset, QgsFeedback *feedback )
  : mRenderContext( context )
  , mScale( scale )
  , mOffset( offset )
  , mZValueScale( zValueScale )
  , mZValueFixedOffset( zValueFixedOffset )
  , mFeedback( feedback )
{

}

long QgsPointCloudRenderContext::pointsRendered() const
{
  return mPointsRendered;
}

void QgsPointCloudRenderContext::incrementPointsRendered( long count )
{
  mPointsRendered += count;
}

void QgsPointCloudRenderContext::setAttributes( const QgsPointCloudAttributeCollection &attributes )
{
  mAttributes = attributes;
  mPointRecordSize = mAttributes.pointRecordSize();

  // fetch offset for x/y/z attributes
  attributes.find( QStringLiteral( "X" ), mXOffset );
  attributes.find( QStringLiteral( "Y" ), mYOffset );
  attributes.find( QStringLiteral( "Z" ), mZOffset );
}

QgsPointCloudRenderer *QgsPointCloudRenderer::load( QDomElement &element, const QgsReadWriteContext &context )
{
  if ( element.isNull() )
    return nullptr;

  // load renderer
  const QString rendererType = element.attribute( QStringLiteral( "type" ) );

  QgsPointCloudRendererAbstractMetadata *m = QgsApplication::pointCloudRendererRegistry()->rendererMetadata( rendererType );
  if ( !m )
    return nullptr;

  std::unique_ptr< QgsPointCloudRenderer > r( m->createRenderer( element, context ) );
  return r.release();
}

QSet<QString> QgsPointCloudRenderer::usedAttributes( const QgsPointCloudRenderContext & ) const
{
  return QSet< QString >();
}

std::unique_ptr<QgsPreparedPointCloudRendererData> QgsPointCloudRenderer::prepare()
{
  return nullptr;
}

void QgsPointCloudRenderer::startRender( QgsPointCloudRenderContext &context )
{
#ifdef QGISDEBUG
  if ( !mThread )
  {
    mThread = QThread::currentThread();
  }
  else
  {
    Q_ASSERT_X( mThread == QThread::currentThread(), "QgsPointCloudRenderer::startRender", "startRender called in a different thread - use a cloned renderer instead" );
  }
#endif

  mPainterPenWidth = context.renderContext().convertToPainterUnits( pointSize(), pointSizeUnit(), pointSizeMapUnitScale() );

  switch ( mPointSymbol )
  {
    case Qgis::PointCloudSymbol::Square:
      // for square point we always disable antialiasing -- it's not critical here and we benefit from the performance boost disabling it gives
      context.renderContext().painter()->setRenderHint( QPainter::Antialiasing, false );
      break;

    case Qgis::PointCloudSymbol::Circle:
      break;
  }
}

void QgsPointCloudRenderer::stopRender( QgsPointCloudRenderContext & )
{
#ifdef QGISDEBUG
  Q_ASSERT_X( mThread == QThread::currentThread(), "QgsPointCloudRenderer::stopRender", "stopRender called in a different thread - use a cloned renderer instead" );
#endif
}

bool QgsPointCloudRenderer::legendItemChecked( const QString & )
{
  return false;
}

void QgsPointCloudRenderer::checkLegendItem( const QString &, bool )
{

}

double QgsPointCloudRenderer::maximumScreenError() const
{
  return mMaximumScreenError;
}

void QgsPointCloudRenderer::setMaximumScreenError( double error )
{
  mMaximumScreenError = error;
}

QgsUnitTypes::RenderUnit QgsPointCloudRenderer::maximumScreenErrorUnit() const
{
  return mMaximumScreenErrorUnit;
}

void QgsPointCloudRenderer::setMaximumScreenErrorUnit( QgsUnitTypes::RenderUnit unit )
{
  mMaximumScreenErrorUnit = unit;
}

QList<QgsLayerTreeModelLegendNode *> QgsPointCloudRenderer::createLegendNodes( QgsLayerTreeLayer * )
{
  return QList<QgsLayerTreeModelLegendNode *>();
}

QStringList QgsPointCloudRenderer::legendRuleKeys() const
{
  return QStringList();
}

void QgsPointCloudRenderer::copyCommonProperties( QgsPointCloudRenderer *destination ) const
{
  destination->setPointSize( mPointSize );
  destination->setPointSizeUnit( mPointSizeUnit );
  destination->setPointSizeMapUnitScale( mPointSizeMapUnitScale );
  destination->setMaximumScreenError( mMaximumScreenError );
  destination->setMaximumScreenErrorUnit( mMaximumScreenErrorUnit );
  destination->setPointSymbol( mPointSymbol );
  destination->setDrawOrder2d( mDrawOrder2d );
}

void QgsPointCloudRenderer::restoreCommonProperties( const QDomElement &element, const QgsReadWriteContext & )
{
  mPointSize = element.attribute( QStringLiteral( "pointSize" ), QStringLiteral( "1" ) ).toDouble();
  mPointSizeUnit = QgsUnitTypes::decodeRenderUnit( element.attribute( QStringLiteral( "pointSizeUnit" ), QStringLiteral( "MM" ) ) );
  mPointSizeMapUnitScale = QgsSymbolLayerUtils::decodeMapUnitScale( element.attribute( QStringLiteral( "pointSizeMapUnitScale" ), QString() ) );

  mMaximumScreenError = element.attribute( QStringLiteral( "maximumScreenError" ), QStringLiteral( "0.3" ) ).toDouble();
  mMaximumScreenErrorUnit = QgsUnitTypes::decodeRenderUnit( element.attribute( QStringLiteral( "maximumScreenErrorUnit" ), QStringLiteral( "MM" ) ) );
  mPointSymbol = static_cast< Qgis::PointCloudSymbol >( element.attribute( QStringLiteral( "pointSymbol" ), QStringLiteral( "0" ) ).toInt() );
  mDrawOrder2d = static_cast< Qgis::PointCloudDrawOrder >( element.attribute( QStringLiteral( "drawOrder2d" ), QStringLiteral( "0" ) ).toInt() );
}

void QgsPointCloudRenderer::saveCommonProperties( QDomElement &element, const QgsReadWriteContext & ) const
{
  element.setAttribute( QStringLiteral( "pointSize" ), qgsDoubleToString( mPointSize ) );
  element.setAttribute( QStringLiteral( "pointSizeUnit" ), QgsUnitTypes::encodeUnit( mPointSizeUnit ) );
  element.setAttribute( QStringLiteral( "pointSizeMapUnitScale" ), QgsSymbolLayerUtils::encodeMapUnitScale( mPointSizeMapUnitScale ) );

  element.setAttribute( QStringLiteral( "maximumScreenError" ), qgsDoubleToString( mMaximumScreenError ) );
  element.setAttribute( QStringLiteral( "maximumScreenErrorUnit" ), QgsUnitTypes::encodeUnit( mMaximumScreenErrorUnit ) );
  element.setAttribute( QStringLiteral( "pointSymbol" ), QString::number( static_cast< int >( mPointSymbol ) ) );
  element.setAttribute( QStringLiteral( "drawOrder2d" ), QString::number( static_cast< int >( mDrawOrder2d ) ) );
}

Qgis::PointCloudSymbol QgsPointCloudRenderer::pointSymbol() const
{
  return mPointSymbol;
}

void QgsPointCloudRenderer::setPointSymbol( Qgis::PointCloudSymbol symbol )
{
  mPointSymbol = symbol;
}

Qgis::PointCloudDrawOrder QgsPointCloudRenderer::drawOrder2d() const
{
  return mDrawOrder2d;
}

void QgsPointCloudRenderer::setDrawOrder2d( Qgis::PointCloudDrawOrder order )
{
  mDrawOrder2d = order;
}

QVector<QVariantMap> QgsPointCloudRenderer::identify( QgsPointCloudLayer *layer, const QgsRenderContext &renderContext, const QgsGeometry &geometry, double toleranceForPointIdentification )
{
  QVector<QVariantMap> selectedPoints;

  QgsPointCloudIndex *index = layer->dataProvider()->index();
  const IndexedPointCloudNode root = index->root();

  const double maxErrorPixels = renderContext.convertToPainterUnits( maximumScreenError(), maximumScreenErrorUnit() );// in pixels

  const QgsRectangle rootNodeExtentLayerCoords = index->nodeMapExtent( root );
  QgsRectangle rootNodeExtentMapCoords;
  if ( !renderContext.coordinateTransform().isShortCircuited() )
  {
    try
    {
      QgsCoordinateTransform extentTransform = renderContext.coordinateTransform();
      extentTransform.setBallparkTransformsAreAppropriate( true );
      rootNodeExtentMapCoords = extentTransform.transformBoundingBox( rootNodeExtentLayerCoords );
    }
    catch ( QgsCsException & )
    {
      QgsDebugMsg( QStringLiteral( "Could not transform node extent to map CRS" ) );
      rootNodeExtentMapCoords = rootNodeExtentLayerCoords;
    }
  }
  else
  {
    rootNodeExtentMapCoords = rootNodeExtentLayerCoords;
  }

  const double rootErrorInMapCoordinates = rootNodeExtentMapCoords.width() / index->span();
  const double rootErrorInLayerCoordinates = rootNodeExtentLayerCoords.width() / index->span();

  const double mapUnitsPerPixel = renderContext.mapToPixel().mapUnitsPerPixel();
  if ( ( rootErrorInMapCoordinates < 0.0 ) || ( mapUnitsPerPixel < 0.0 ) || ( maxErrorPixels < 0.0 ) )
  {
    QgsDebugMsg( QStringLiteral( "invalid screen error" ) );
    return selectedPoints;
  }

  const double maxErrorInMapCoordinates = maxErrorPixels * mapUnitsPerPixel;
  const double maxErrorInLayerCoordinates = maxErrorInMapCoordinates * rootErrorInLayerCoordinates / rootErrorInMapCoordinates;

  QgsGeometry selectionGeometry = geometry;
  if ( geometry.type() == QgsWkbTypes::PointGeometry )
  {
    const double x = geometry.asPoint().x();
    const double y = geometry.asPoint().y();
    const double toleranceInPixels = toleranceForPointIdentification / renderContext.mapToPixel().mapUnitsPerPixel();
    const double pointSizePixels = renderContext.convertToPainterUnits( mPointSize, mPointSizeUnit, mPointSizeMapUnitScale );
    switch ( pointSymbol() )
    {
      case Qgis::PointCloudSymbol::Square:
      {
        const QgsPointXY deviceCoords = renderContext.mapToPixel().transform( QgsPointXY( x, y ) );
        const QgsPointXY point1( deviceCoords.x() - std::max( toleranceInPixels, pointSizePixels / 2.0 ), deviceCoords.y() - std::max( toleranceInPixels, pointSizePixels / 2.0 ) );
        const QgsPointXY point2( deviceCoords.x() + std::max( toleranceInPixels, pointSizePixels / 2.0 ), deviceCoords.y() + std::max( toleranceInPixels, pointSizePixels / 2.0 ) );
        const QgsPointXY point1MapCoords = renderContext.mapToPixel().toMapCoordinates( point1.x(), point1.y() );
        const QgsPointXY point2MapCoords = renderContext.mapToPixel().toMapCoordinates( point2.x(), point2.y() );
        const QgsRectangle pointRect( point1MapCoords, point2MapCoords );
        selectionGeometry = QgsGeometry::fromRect( pointRect );
        break;
      }
      case Qgis::PointCloudSymbol::Circle:
      {
        const QgsPoint centerMapCoords( x, y );
        const QgsPointXY deviceCoords = renderContext.mapToPixel().transform( centerMapCoords );
        const QgsPoint point1( deviceCoords.x(), deviceCoords.y() - std::max( toleranceInPixels, pointSizePixels / 2.0 ) );
        const QgsPoint point2( deviceCoords.x(), deviceCoords.y() + std::max( toleranceInPixels, pointSizePixels / 2.0 ) );
        const QgsPointXY point1MapCoords = renderContext.mapToPixel().toMapCoordinates( point1.x(), point1.y() );
        const QgsPointXY point2MapCoords = renderContext.mapToPixel().toMapCoordinates( point2.x(), point2.y() );
        const QgsCircle circle = QgsCircle::from2Points( QgsPoint( point1MapCoords ), QgsPoint( point2MapCoords ) );
        std::unique_ptr<QgsPolygon> polygon( circle.toPolygon( 6 ) );
        const QgsGeometry circleGeometry( std::move( polygon ) );
        selectionGeometry = circleGeometry;
        break;
      }
    }
  }

  // selection geometry must be in layer CRS for QgsPointCloudDataProvider::identify
  try
  {
    selectionGeometry.transform( renderContext.coordinateTransform(), Qgis::TransformDirection::Reverse );
  }
  catch ( QgsCsException & )
  {
    QgsDebugMsg( QStringLiteral( "Could not transform geometry to layer CRS" ) );
    return selectedPoints;
  }

  selectedPoints = layer->dataProvider()->identify( maxErrorInLayerCoordinates, selectionGeometry, renderContext.zRange() );

  selectedPoints.erase( std::remove_if( selectedPoints.begin(), selectedPoints.end(), [this]( const QMap<QString, QVariant> &point ) { return !this->willRenderPoint( point ); } ), selectedPoints.end() );

  return selectedPoints;
}

//
// QgsPreparedPointCloudRendererData
//
QgsPreparedPointCloudRendererData::~QgsPreparedPointCloudRendererData() = default;
