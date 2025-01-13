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
#include "qgselevationmap.h"
#include "qgssymbollayerutils.h"
#include "qgspointcloudlayer.h"
#include "qgspointcloudindex.h"
#include "qgslogger.h"
#include "qgscircle.h"
#include "qgsunittypes.h"
#include "qgsvirtualpointcloudprovider.h"

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

QgsPointCloudRenderer::QgsPointCloudRenderer()
{
  QgsTextFormat textFormat = QgsStyle::defaultStyle()->defaultTextFormat();
  QgsTextBufferSettings settings;
  settings.setEnabled( true );
  settings.setSize( 1 );
  textFormat.setBuffer( settings );
  mLabelTextFormat = textFormat;
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

  mDefaultPainterPenWidth = context.renderContext().convertToPainterUnits( pointSize(), pointSizeUnit(), pointSizeMapUnitScale() );

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

Qgis::RenderUnit QgsPointCloudRenderer::maximumScreenErrorUnit() const
{
  return mMaximumScreenErrorUnit;
}

void QgsPointCloudRenderer::setMaximumScreenErrorUnit( Qgis::RenderUnit unit )
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

void QgsPointCloudRenderer::drawPointToElevationMap( double x, double y, double z, QgsPointCloudRenderContext &context ) const
{
  drawPointToElevationMap( x, y, z, mDefaultPainterPenWidth, context );
}

void QgsPointCloudRenderer::drawPointToElevationMap( double x, double y, double z, int width, QgsPointCloudRenderContext &context ) const
{
  const QPointF originalXY( x, y );
  context.renderContext().mapToPixel().transformInPlace( x, y );
  QPainter *elevationPainter = context.renderContext().elevationMap()->painter();

  QBrush brush( QgsElevationMap::encodeElevation( z ) );
  switch ( mPointSymbol )
  {
    case Qgis::PointCloudSymbol::Square:
      elevationPainter->fillRect( QRectF( x - width * 0.5,
                                          y - width * 0.5,
                                          width, width ), brush );
      break;

    case Qgis::PointCloudSymbol::Circle:
      elevationPainter->setBrush( brush );
      elevationPainter->setPen( Qt::NoPen );
      elevationPainter->drawEllipse( QRectF( x - width * 0.5,
                                             y - width * 0.5,
                                             width, width ) );
      break;
  };
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

  destination->setRenderAsTriangles( mRenderAsTriangles );
  destination->setHorizontalTriangleFilter( mHorizontalTriangleFilter );
  destination->setHorizontalTriangleFilterThreshold( mHorizontalTriangleFilterThreshold );
  destination->setHorizontalTriangleFilterUnit( mHorizontalTriangleFilterUnit );

  destination->setShowLabels( mShowLabels );
  destination->setLabelTextFormat( mLabelTextFormat );
  destination->setZoomOutBehavior( mZoomOutBehavior );
}

void QgsPointCloudRenderer::restoreCommonProperties( const QDomElement &element, const QgsReadWriteContext &context )
{
  mPointSize = element.attribute( QStringLiteral( "pointSize" ), QStringLiteral( "1" ) ).toDouble();
  mPointSizeUnit = QgsUnitTypes::decodeRenderUnit( element.attribute( QStringLiteral( "pointSizeUnit" ), QStringLiteral( "MM" ) ) );
  mPointSizeMapUnitScale = QgsSymbolLayerUtils::decodeMapUnitScale( element.attribute( QStringLiteral( "pointSizeMapUnitScale" ), QString() ) );

  mMaximumScreenError = element.attribute( QStringLiteral( "maximumScreenError" ), QStringLiteral( "0.3" ) ).toDouble();
  mMaximumScreenErrorUnit = QgsUnitTypes::decodeRenderUnit( element.attribute( QStringLiteral( "maximumScreenErrorUnit" ), QStringLiteral( "MM" ) ) );
  mPointSymbol = qgsEnumKeyToValue( element.attribute( QStringLiteral( "pointSymbol" ) ), Qgis::PointCloudSymbol::Square );
  mDrawOrder2d = qgsEnumKeyToValue( element.attribute( QStringLiteral( "drawOrder2d" ) ), Qgis::PointCloudDrawOrder::Default );

  mRenderAsTriangles = element.attribute( QStringLiteral( "renderAsTriangles" ), QStringLiteral( "0" ) ).toInt();
  mHorizontalTriangleFilter = element.attribute( QStringLiteral( "horizontalTriangleFilter" ), QStringLiteral( "0" ) ).toInt();
  mHorizontalTriangleFilterThreshold = element.attribute( QStringLiteral( "horizontalTriangleFilterThreshold" ), QStringLiteral( "5" ) ).toDouble();
  mHorizontalTriangleFilterUnit = QgsUnitTypes::decodeRenderUnit( element.attribute( QStringLiteral( "horizontalTriangleFilterUnit" ), QStringLiteral( "MM" ) ) );

  mShowLabels = element.attribute( QStringLiteral( "showLabels" ), QStringLiteral( "0" ) ).toInt();
  if ( !element.firstChildElement( QStringLiteral( "text-style" ) ).isNull() )
  {
    mLabelTextFormat = QgsTextFormat();
    mLabelTextFormat.readXml( element.firstChildElement( QStringLiteral( "text-style" ) ), context );
  }
  mZoomOutBehavior = qgsEnumKeyToValue( element.attribute( QStringLiteral( "zoomOutBehavior" ) ), Qgis::PointCloudZoomOutRenderBehavior::RenderExtents );
}

void QgsPointCloudRenderer::saveCommonProperties( QDomElement &element, const QgsReadWriteContext &context ) const
{
  element.setAttribute( QStringLiteral( "pointSize" ), qgsDoubleToString( mPointSize ) );
  element.setAttribute( QStringLiteral( "pointSizeUnit" ), QgsUnitTypes::encodeUnit( mPointSizeUnit ) );
  element.setAttribute( QStringLiteral( "pointSizeMapUnitScale" ), QgsSymbolLayerUtils::encodeMapUnitScale( mPointSizeMapUnitScale ) );

  element.setAttribute( QStringLiteral( "maximumScreenError" ), qgsDoubleToString( mMaximumScreenError ) );
  element.setAttribute( QStringLiteral( "maximumScreenErrorUnit" ), QgsUnitTypes::encodeUnit( mMaximumScreenErrorUnit ) );
  element.setAttribute( QStringLiteral( "pointSymbol" ), qgsEnumValueToKey( mPointSymbol ) );
  element.setAttribute( QStringLiteral( "drawOrder2d" ), qgsEnumValueToKey( mDrawOrder2d ) );

  element.setAttribute( QStringLiteral( "renderAsTriangles" ), QString::number( static_cast< int >( mRenderAsTriangles ) ) );
  element.setAttribute( QStringLiteral( "horizontalTriangleFilter" ), QString::number( static_cast< int >( mHorizontalTriangleFilter ) ) );
  element.setAttribute( QStringLiteral( "horizontalTriangleFilterThreshold" ), qgsDoubleToString( mHorizontalTriangleFilterThreshold ) );
  element.setAttribute( QStringLiteral( "horizontalTriangleFilterUnit" ), QgsUnitTypes::encodeUnit( mHorizontalTriangleFilterUnit ) );

  if ( mShowLabels )
    element.setAttribute( QStringLiteral( "showLabels" ), QStringLiteral( "1" ) );
  if ( mLabelTextFormat.isValid() )
  {
    QDomDocument doc = element.ownerDocument();
    element.appendChild( mLabelTextFormat.writeXml( doc, context ) );
  }
  element.setAttribute( QStringLiteral( "zoomOutBehavior" ), qgsEnumValueToKey( mZoomOutBehavior ) );
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

  const double maxErrorPixels = renderContext.convertToPainterUnits( maximumScreenError(), maximumScreenErrorUnit() );// in pixels

  const QgsRectangle layerExtentLayerCoords = layer->dataProvider()->extent();
  QgsRectangle layerExtentMapCoords = layerExtentLayerCoords;
  if ( !renderContext.coordinateTransform().isShortCircuited() )
  {
    try
    {
      QgsCoordinateTransform extentTransform = renderContext.coordinateTransform();
      extentTransform.setBallparkTransformsAreAppropriate( true );
      layerExtentMapCoords = extentTransform.transformBoundingBox( layerExtentLayerCoords );
    }
    catch ( QgsCsException & )
    {
      QgsDebugError( QStringLiteral( "Could not transform node extent to map CRS" ) );
    }
  }

  const double mapUnitsPerPixel = renderContext.mapToPixel().mapUnitsPerPixel();
  if ( ( mapUnitsPerPixel < 0.0 ) || ( maxErrorPixels < 0.0 ) )
  {
    QgsDebugError( QStringLiteral( "invalid screen error" ) );
    return selectedPoints;
  }

  const double maxErrorInMapCoordinates = maxErrorPixels * mapUnitsPerPixel;
  const double maxErrorInLayerCoordinates = maxErrorInMapCoordinates * layerExtentLayerCoords.width() / layerExtentMapCoords.width();

  QgsGeometry selectionGeometry = geometry;
  if ( geometry.type() == Qgis::GeometryType::Point )
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
    QgsDebugError( QStringLiteral( "Could not transform geometry to layer CRS" ) );
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
