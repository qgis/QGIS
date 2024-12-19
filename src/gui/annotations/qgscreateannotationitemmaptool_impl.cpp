/***************************************************************************
                             qgscreateannotationitemmaptool_impl.cpp
                             ------------------------
    Date                 : September 2021
    Copyright            : (C) 2021 Nyall Dawson
    Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgscreateannotationitemmaptool_impl.h"
#include "moc_qgscreateannotationitemmaptool_impl.cpp"
#include "qgsmapmouseevent.h"
#include "qgsannotationpointtextitem.h"
#include "qgsannotationmarkeritem.h"
#include "qgsannotationlineitem.h"
#include "qgsannotationpolygonitem.h"
#include "qgsannotationlinetextitem.h"
#include "qgsannotationrectangletextitem.h"
#include "qgsannotationpictureitem.h"
#include "qgsannotationlayer.h"
#include "qgsstyle.h"
#include "qgsmapcanvas.h"
#include "qgsmarkersymbol.h"
#include "qgslinesymbol.h"
#include "qgsfillsymbol.h"
#include "qgsadvanceddigitizingdockwidget.h"
#include "qgsapplication.h"
#include "qgsrecentstylehandler.h"
#include "qgscurvepolygon.h"
#include "qgsrubberband.h"
#include "qgssettingsregistrycore.h"
#include "qgssvgcache.h"
#include "qgsimagecache.h"

#include <QFileDialog>
#include <QImageReader>

///@cond PRIVATE

//
// QgsMapToolCaptureAnnotationItem
//

QgsMapToolCaptureAnnotationItem::QgsMapToolCaptureAnnotationItem( QgsMapCanvas *canvas, QgsAdvancedDigitizingDockWidget *cadDockWidget, CaptureMode mode )
  : QgsMapToolCapture( canvas, cadDockWidget, mode )
{
  mToolName = tr( "Annotation tool" );
}

QgsCreateAnnotationItemMapToolHandler *QgsMapToolCaptureAnnotationItem::handler() const
{
  return mHandler;
}

QgsMapTool *QgsMapToolCaptureAnnotationItem::mapTool()
{
  return this;
}

QgsMapLayer *QgsMapToolCaptureAnnotationItem::layer() const
{
  return mHandler->targetLayer();
}


QgsMapToolCapture::Capabilities QgsMapToolCaptureAnnotationItem::capabilities() const
{
  // no geometry validation!
  return SupportsCurves;
}

bool QgsMapToolCaptureAnnotationItem::supportsTechnique( Qgis::CaptureTechnique technique ) const
{
  switch ( technique )
  {
    case Qgis::CaptureTechnique::StraightSegments:
    case Qgis::CaptureTechnique::CircularString:
    case Qgis::CaptureTechnique::Streaming:
    case Qgis::CaptureTechnique::Shape:
      return true;
  }
  BUILTIN_UNREACHABLE
}


//
// QgsCreatePointTextItemMapTool
//

QgsCreatePointTextItemMapTool::QgsCreatePointTextItemMapTool( QgsMapCanvas *canvas, QgsAdvancedDigitizingDockWidget *cadDockWidget )
  : QgsMapToolAdvancedDigitizing( canvas, cadDockWidget )
  , mHandler( new QgsCreateAnnotationItemMapToolHandler( canvas, cadDockWidget ) )
{
  setUseSnappingIndicator( true );
}

QgsCreatePointTextItemMapTool::~QgsCreatePointTextItemMapTool() = default;

void QgsCreatePointTextItemMapTool::cadCanvasPressEvent( QgsMapMouseEvent *event )
{
  if ( event->button() != Qt::LeftButton )
    return;

  const QgsPointXY layerPoint = toLayerCoordinates( mHandler->targetLayer(), event->mapPoint() );

  std::unique_ptr<QgsAnnotationPointTextItem> createdItem = std::make_unique<QgsAnnotationPointTextItem>( tr( "Text" ), layerPoint );
  createdItem->setAlignment( Qt::AlignLeft );
  QgsTextFormat format = QgsStyle::defaultTextFormatForProject( QgsProject::instance(), QgsStyle::TextFormatContext::Labeling );
  // default to HTML formatting
  format.setAllowHtmlFormatting( true );
  createdItem->setFormat( format );
  // newly created point text items default to using symbology reference scale at the current map scale
  createdItem->setUseSymbologyReferenceScale( true );
  createdItem->setSymbologyReferenceScale( canvas()->scale() );
  mHandler->pushCreatedItem( createdItem.release() );
}

QgsCreateAnnotationItemMapToolHandler *QgsCreatePointTextItemMapTool::handler() const
{
  return mHandler;
}

QgsMapTool *QgsCreatePointTextItemMapTool::mapTool()
{
  return this;
}


//
// QgsCreateMarkerMapTool
//

QgsCreateMarkerItemMapTool::QgsCreateMarkerItemMapTool( QgsMapCanvas *canvas, QgsAdvancedDigitizingDockWidget *cadDockWidget )
  : QgsMapToolCaptureAnnotationItem( canvas, cadDockWidget, CapturePoint )
{
  mHandler = new QgsCreateAnnotationItemMapToolHandler( canvas, cadDockWidget, this );
}

void QgsCreateMarkerItemMapTool::cadCanvasReleaseEvent( QgsMapMouseEvent *event )
{
  if ( event->button() != Qt::LeftButton )
    return;

  const QgsPointXY layerPoint = toLayerCoordinates( mHandler->targetLayer(), event->mapPoint() );
  std::unique_ptr<QgsAnnotationMarkerItem> createdItem = std::make_unique<QgsAnnotationMarkerItem>( QgsPoint( layerPoint ) );

  std::unique_ptr<QgsMarkerSymbol> markerSymbol = QgsApplication::recentStyleHandler()->recentSymbol<QgsMarkerSymbol>( QStringLiteral( "marker_annotation_item" ) );
  if ( !markerSymbol )
    markerSymbol.reset( qgis::down_cast<QgsMarkerSymbol *>( QgsSymbol::defaultSymbol( Qgis::GeometryType::Point ) ) );
  createdItem->setSymbol( markerSymbol.release() );

  // set reference scale to match canvas scale, but don't enable it by default for marker items
  createdItem->setSymbologyReferenceScale( canvas()->scale() );

  mHandler->pushCreatedItem( createdItem.release() );

  stopCapturing();

  cadDockWidget()->clearPoints();
}

//
// QgsCreateLineMapTool
//

QgsCreateLineItemMapTool::QgsCreateLineItemMapTool( QgsMapCanvas *canvas, QgsAdvancedDigitizingDockWidget *cadDockWidget )
  : QgsMapToolCaptureAnnotationItem( canvas, cadDockWidget, CaptureLine )
{
  mHandler = new QgsCreateAnnotationItemMapToolHandler( canvas, cadDockWidget, this );
}

void QgsCreateLineItemMapTool::lineCaptured( const QgsCurve *line )
{
  if ( line->isEmpty() )
    return;

  // do it!
  std::unique_ptr<QgsAbstractGeometry> geometry( line->simplifiedTypeRef()->clone() );
  if ( qgsgeometry_cast<QgsCurve *>( geometry.get() ) )
  {
    std::unique_ptr<QgsAnnotationLineItem> createdItem = std::make_unique<QgsAnnotationLineItem>( qgsgeometry_cast<QgsCurve *>( geometry.release() ) );

    std::unique_ptr<QgsLineSymbol> lineSymbol = QgsApplication::recentStyleHandler()->recentSymbol<QgsLineSymbol>( QStringLiteral( "line_annotation_item" ) );
    if ( !lineSymbol )
      lineSymbol.reset( qgis::down_cast<QgsLineSymbol *>( QgsSymbol::defaultSymbol( Qgis::GeometryType::Line ) ) );
    createdItem->setSymbol( lineSymbol.release() );

    // set reference scale to match canvas scale, but don't enable it by default for marker items
    createdItem->setSymbologyReferenceScale( canvas()->scale() );

    mHandler->pushCreatedItem( createdItem.release() );
  }
}

//
// QgsCreatePolygonItemMapTool
//

QgsCreatePolygonItemMapTool::QgsCreatePolygonItemMapTool( QgsMapCanvas *canvas, QgsAdvancedDigitizingDockWidget *cadDockWidget )
  : QgsMapToolCaptureAnnotationItem( canvas, cadDockWidget, CapturePolygon )
{
  mHandler = new QgsCreateAnnotationItemMapToolHandler( canvas, cadDockWidget, this );
}

void QgsCreatePolygonItemMapTool::polygonCaptured( const QgsCurvePolygon *polygon )
{
  if ( polygon->isEmpty() )
    return;

  std::unique_ptr<QgsAbstractGeometry> geometry( polygon->exteriorRing()->simplifiedTypeRef()->clone() );
  if ( qgsgeometry_cast<QgsCurve *>( geometry.get() ) )
  {
    std::unique_ptr<QgsCurvePolygon> newPolygon = std::make_unique<QgsCurvePolygon>();
    newPolygon->setExteriorRing( qgsgeometry_cast<QgsCurve *>( geometry.release() ) );
    std::unique_ptr<QgsAnnotationPolygonItem> createdItem = std::make_unique<QgsAnnotationPolygonItem>( newPolygon.release() );

    std::unique_ptr<QgsFillSymbol> fillSymbol = QgsApplication::recentStyleHandler()->recentSymbol<QgsFillSymbol>( QStringLiteral( "polygon_annotation_item" ) );
    if ( !fillSymbol )
      fillSymbol.reset( qgis::down_cast<QgsFillSymbol *>( QgsSymbol::defaultSymbol( Qgis::GeometryType::Polygon ) ) );
    createdItem->setSymbol( fillSymbol.release() );

    // set reference scale to match canvas scale, but don't enable it by default for marker items
    createdItem->setSymbologyReferenceScale( canvas()->scale() );

    mHandler->pushCreatedItem( createdItem.release() );
  }
}


//
// QgsCreatePictureItemMapTool
//

const QgsSettingsEntryString *QgsCreatePictureItemMapTool::settingLastSourceFolder = new QgsSettingsEntryString( QStringLiteral( "last-source-folder" ), sTreePicture, QString(), QStringLiteral( "Last used folder for picture annotation source files" ) );

QgsCreatePictureItemMapTool::QgsCreatePictureItemMapTool( QgsMapCanvas *canvas, QgsAdvancedDigitizingDockWidget *cadDockWidget )
  : QgsMapToolAdvancedDigitizing( canvas, cadDockWidget )
  , mHandler( new QgsCreateAnnotationItemMapToolHandler( canvas, cadDockWidget, this ) )
{
  setUseSnappingIndicator( true );
}

void QgsCreatePictureItemMapTool::cadCanvasPressEvent( QgsMapMouseEvent *event )
{
  if ( event->button() == Qt::RightButton && mRubberBand )
  {
    mRubberBand.reset();
    cadDockWidget()->clearPoints();
    return;
  }

  if ( event->button() != Qt::LeftButton )
    return;

  if ( !mRubberBand )
  {
    mFirstPoint = event->snapPoint();
    mRect.setRect( mFirstPoint.x(), mFirstPoint.y(), mFirstPoint.x(), mFirstPoint.y() );

    mRubberBand.reset( new QgsRubberBand( mCanvas, Qgis::GeometryType::Polygon ) );
    mRubberBand->setWidth( digitizingStrokeWidth() );
    QColor color = digitizingStrokeColor();

    const double alphaScale = QgsSettingsRegistryCore::settingsDigitizingLineColorAlphaScale->value();
    color.setAlphaF( color.alphaF() * alphaScale );
    mRubberBand->setLineStyle( Qt::DotLine );
    mRubberBand->setStrokeColor( color );

    const QColor fillColor = digitizingFillColor();
    mRubberBand->setFillColor( fillColor );
  }
  else
  {
    mRubberBand.reset();

    QStringList formatsFilter;
    formatsFilter.append( QStringLiteral( "*.svg" ) );
    const QByteArrayList supportedFormats = QImageReader::supportedImageFormats();
    for ( const auto &format : supportedFormats )
    {
      formatsFilter.append( QString( QStringLiteral( "*.%1" ) ).arg( QString( format ) ) );
    }
    const QString dialogFilter = QStringLiteral( "%1 (%2);;%3 (*.*)" ).arg( tr( "Images" ), formatsFilter.join( QLatin1Char( ' ' ) ), tr( "All files" ) );
    const QString initialDir = settingLastSourceFolder->value();
    const QString imagePath = QFileDialog::getOpenFileName( nullptr, tr( "Add Picture Annotation" ), initialDir.isEmpty() ? QDir::homePath() : initialDir, dialogFilter );

    if ( imagePath.isEmpty() )
    {
      return; //canceled by the user
    }

    settingLastSourceFolder->setValue( QFileInfo( imagePath ).path() );

    const QgsPointXY point1 = toLayerCoordinates( mHandler->targetLayer(), mFirstPoint );
    const QgsPointXY point2 = toLayerCoordinates( mHandler->targetLayer(), event->snapPoint() );

    const QgsPointXY devicePoint1 = toCanvasCoordinates( mFirstPoint );
    const QgsPointXY devicePoint2 = toCanvasCoordinates( event->snapPoint() );
    const double initialWidthPixels = std::abs( devicePoint1.x() - devicePoint2.x() );
    const double initialHeightPixels = std::abs( devicePoint1.y() - devicePoint2.y() );

    const QFileInfo pathInfo( imagePath );
    Qgis::PictureFormat format = Qgis::PictureFormat::Unknown;

    QSizeF size;
    if ( pathInfo.suffix().compare( QLatin1String( "svg" ), Qt::CaseInsensitive ) == 0 )
    {
      format = Qgis::PictureFormat::SVG;
      size = QgsApplication::svgCache()->svgViewboxSize( imagePath, 100, QColor(), QColor(), 1, 1 );
    }
    else
    {
      format = Qgis::PictureFormat::Raster;
      size = QgsApplication::imageCache()->originalSize( imagePath );
    }

    cadDockWidget()->clearPoints();

    std::unique_ptr<QgsAnnotationPictureItem> createdItem = std::make_unique<QgsAnnotationPictureItem>( format, imagePath, QgsRectangle( point1, point2 ) );
    if ( size.isValid() )
    {
      const double pixelsToMm = mCanvas->mapSettings().outputDpi() / 25.4;
      if ( size.width() / size.height() > initialWidthPixels / initialHeightPixels )
      {
        createdItem->setFixedSize( QSizeF( initialWidthPixels / pixelsToMm, size.height() / size.width() * initialWidthPixels / pixelsToMm ) );
      }
      else
      {
        createdItem->setFixedSize( QSizeF( size.width() / size.height() * initialHeightPixels / pixelsToMm, initialHeightPixels / pixelsToMm ) );
      }
      createdItem->setFixedSizeUnit( Qgis::RenderUnit::Millimeters );
    }

    mHandler->pushCreatedItem( createdItem.release() );
  }
}

void QgsCreatePictureItemMapTool::cadCanvasMoveEvent( QgsMapMouseEvent *event )
{
  if ( !mRubberBand )
    return;

  const QgsPointXY mapPoint = event->snapPoint();
  mRect.setBottomRight( mapPoint.toQPointF() );

  mRubberBand->reset( Qgis::GeometryType::Polygon );
  mRubberBand->addPoint( mRect.bottomLeft(), false );
  mRubberBand->addPoint( mRect.bottomRight(), false );
  mRubberBand->addPoint( mRect.topRight(), false );
  mRubberBand->addPoint( mRect.topLeft(), true );
}

void QgsCreatePictureItemMapTool::keyPressEvent( QKeyEvent *event )
{
  if ( event->key() == Qt::Key_Escape )
  {
    if ( mRubberBand )
    {
      mRubberBand.reset();
      cadDockWidget()->clearPoints();
      event->ignore();
    }
  }
}

QgsCreateAnnotationItemMapToolHandler *QgsCreatePictureItemMapTool::handler() const
{
  return mHandler;
}

QgsMapTool *QgsCreatePictureItemMapTool::mapTool()
{
  return this;
}


//
// QgsCreateRectangleTextItemMapTool
//

QgsCreateRectangleTextItemMapTool::QgsCreateRectangleTextItemMapTool( QgsMapCanvas *canvas, QgsAdvancedDigitizingDockWidget *cadDockWidget )
  : QgsMapToolAdvancedDigitizing( canvas, cadDockWidget )
  , mHandler( new QgsCreateAnnotationItemMapToolHandler( canvas, cadDockWidget, this ) )
{
  setUseSnappingIndicator( true );
}

void QgsCreateRectangleTextItemMapTool::cadCanvasPressEvent( QgsMapMouseEvent *event )
{
  if ( event->button() == Qt::RightButton && mRubberBand )
  {
    mRubberBand.reset();
    cadDockWidget()->clearPoints();
    return;
  }

  if ( event->button() != Qt::LeftButton )
    return;

  if ( !mRubberBand )
  {
    mFirstPoint = event->snapPoint();
    mRect.setRect( mFirstPoint.x(), mFirstPoint.y(), mFirstPoint.x(), mFirstPoint.y() );

    mRubberBand.reset( new QgsRubberBand( mCanvas, Qgis::GeometryType::Polygon ) );
    mRubberBand->setWidth( digitizingStrokeWidth() );
    QColor color = digitizingStrokeColor();

    const double alphaScale = QgsSettingsRegistryCore::settingsDigitizingLineColorAlphaScale->value();
    color.setAlphaF( color.alphaF() * alphaScale );
    mRubberBand->setLineStyle( Qt::DotLine );
    mRubberBand->setStrokeColor( color );

    const QColor fillColor = digitizingFillColor();
    mRubberBand->setFillColor( fillColor );
  }
  else
  {
    mRubberBand.reset();

    const QgsPointXY point1 = toLayerCoordinates( mHandler->targetLayer(), mFirstPoint );
    const QgsPointXY point2 = toLayerCoordinates( mHandler->targetLayer(), event->snapPoint() );

    cadDockWidget()->clearPoints();

    std::unique_ptr<QgsAnnotationRectangleTextItem> createdItem = std::make_unique<QgsAnnotationRectangleTextItem>( tr( "Text" ), QgsRectangle( point1, point2 ) );

    QgsTextFormat format = QgsStyle::defaultTextFormatForProject( QgsProject::instance(), QgsStyle::TextFormatContext::Labeling );
    // default to HTML formatting
    format.setAllowHtmlFormatting( true );
    createdItem->setFormat( format );

    // newly created rect text items default to using symbology reference scale at the current map scale
    createdItem->setUseSymbologyReferenceScale( true );
    createdItem->setSymbologyReferenceScale( canvas()->scale() );
    mHandler->pushCreatedItem( createdItem.release() );
  }
}

void QgsCreateRectangleTextItemMapTool::cadCanvasMoveEvent( QgsMapMouseEvent *event )
{
  if ( !mRubberBand )
    return;

  const QgsPointXY mapPoint = event->snapPoint();
  mRect.setBottomRight( mapPoint.toQPointF() );

  mRubberBand->reset( Qgis::GeometryType::Polygon );
  mRubberBand->addPoint( mRect.bottomLeft(), false );
  mRubberBand->addPoint( mRect.bottomRight(), false );
  mRubberBand->addPoint( mRect.topRight(), false );
  mRubberBand->addPoint( mRect.topLeft(), true );
}

void QgsCreateRectangleTextItemMapTool::keyPressEvent( QKeyEvent *event )
{
  if ( event->key() == Qt::Key_Escape )
  {
    if ( mRubberBand )
    {
      mRubberBand.reset();
      cadDockWidget()->clearPoints();
      event->ignore();
    }
  }
}

QgsCreateAnnotationItemMapToolHandler *QgsCreateRectangleTextItemMapTool::handler() const
{
  return mHandler;
}

QgsMapTool *QgsCreateRectangleTextItemMapTool::mapTool()
{
  return this;
}


//
// QgsCreateLineTextItemMapTool
//

QgsCreateLineTextItemMapTool::QgsCreateLineTextItemMapTool( QgsMapCanvas *canvas, QgsAdvancedDigitizingDockWidget *cadDockWidget )
  : QgsMapToolCaptureAnnotationItem( canvas, cadDockWidget, CaptureLine )
{
  mHandler = new QgsCreateAnnotationItemMapToolHandler( canvas, cadDockWidget, this );
}

void QgsCreateLineTextItemMapTool::lineCaptured( const QgsCurve *line )
{
  if ( line->isEmpty() )
    return;

  // do it!
  std::unique_ptr<QgsAbstractGeometry> geometry( line->simplifiedTypeRef()->clone() );
  if ( qgsgeometry_cast<QgsCurve *>( geometry.get() ) )
  {
    std::unique_ptr<QgsAnnotationLineTextItem> createdItem = std::make_unique<QgsAnnotationLineTextItem>( tr( "Text" ), qgsgeometry_cast<QgsCurve *>( geometry.release() ) );

    std::unique_ptr<QgsLineSymbol> lineSymbol = QgsApplication::recentStyleHandler()->recentSymbol<QgsLineSymbol>( QStringLiteral( "line_annotation_item" ) );
    if ( !lineSymbol )
      lineSymbol.reset( qgis::down_cast<QgsLineSymbol *>( QgsSymbol::defaultSymbol( Qgis::GeometryType::Line ) ) );

    QgsTextFormat format = QgsStyle::defaultTextFormatForProject( QgsProject::instance(), QgsStyle::TextFormatContext::Labeling );
    // default to HTML formatting
    format.setAllowHtmlFormatting( true );
    createdItem->setFormat( format );

    // newly created point text items default to using symbology reference scale at the current map scale
    createdItem->setUseSymbologyReferenceScale( true );
    createdItem->setSymbologyReferenceScale( canvas()->scale() );
    mHandler->pushCreatedItem( createdItem.release() );
  }
}

///@endcond PRIVATE
