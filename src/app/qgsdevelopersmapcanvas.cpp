/***************************************************************************
                          qgsdevelopersmapcanvas.cpp
                             -------------------
    begin                : November 2025
    copyright            : (C) 2025 by Mathieu Pellerin
    email                : mathieu at opengis dot ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "moc_qgsdevelopersmapcanvas.cpp"
#include "qgsdevelopersmapcanvas.h"
#include "qgsapplication.h"
#include "qgsmaplayerstylemanager.h"
#include "qgsmapmouseevent.h"
#include "qgsfeaturerequest.h"

#include <QCursor>
#include <QFrame>
#include <QVBoxLayout>

QgsDevelopersMapTool::QgsDevelopersMapTool( QgsMapCanvas *canvas, QgsVectorLayer *layer )
  : QgsMapToolPan( canvas )
  , mDevelopersMapLayer( layer )
{
  mDevelopersMapFloatingPanel = std::make_unique<QgsDevelopersMapFloatingPanel>( canvas );
  mDevelopersMapFloatingPanel->setAnchorWidget( canvas );
  mDevelopersMapFloatingPanel->setAnchorWidgetPoint( QgsFloatingWidget::BottomRight );
  mDevelopersMapFloatingPanel->setAnchorPoint( QgsFloatingWidget::BottomRight );
  mDevelopersMapFloatingPanel->hide();
}

void QgsDevelopersMapTool::canvasMoveEvent( QgsMapMouseEvent *e )
{
  if ( !isPinching() && !( e->buttons() & Qt::LeftButton ) )
  {
    const QgsMapSettings mapSettings = mCanvas->mapSettings();
    const QgsRenderContext context = QgsRenderContext::fromMapSettings( mapSettings );
    double searchRadius = context.convertToMapUnits( 3.5, Qgis::RenderUnit::Millimeters );

    const QgsPointXY point = mCanvas->getCoordinateTransform()->toMapCoordinates( e->x(), e->y() );
    QgsRectangle rect = toLayerCoordinates( mDevelopersMapLayer, QgsRectangle( point.x() - searchRadius, point.y() - searchRadius, point.x() + searchRadius, point.y() + searchRadius ) );
    QgsFeatureRequest featureRequest;
    featureRequest.setFilterRect( rect );
    featureRequest.setNoAttributes();
    featureRequest.setFlags( Qgis::Qgis::FeatureRequestFlag::NoGeometry );
    QgsFeatureIterator fit = mDevelopersMapLayer->getFeatures( featureRequest );
    QgsFeature f;
    if ( fit.nextFeature( f ) )
    {
      setCursor( QCursor( Qt::PointingHandCursor ) );
    }
    else
    {
      setCursor( QCursor( Qt::ArrowCursor ) );
    }
  }
  else
  {
    QgsMapToolPan::canvasMoveEvent( e );
  }
}

void QgsDevelopersMapTool::canvasReleaseEvent( QgsMapMouseEvent *e )
{
  if ( !isPinching() && !isDragging() )
  {
    const QgsMapSettings mapSettings = mCanvas->mapSettings();
    const QgsRenderContext context = QgsRenderContext::fromMapSettings( mapSettings );
    double searchRadius = context.convertToMapUnits( 3.5, Qgis::RenderUnit::Millimeters );

    const QgsPointXY point = mCanvas->getCoordinateTransform()->toMapCoordinates( e->x(), e->y() );
    QgsRectangle rect = toLayerCoordinates( mDevelopersMapLayer, QgsRectangle( point.x() - searchRadius, point.y() - searchRadius, point.x() + searchRadius, point.y() + searchRadius ) );
    QgsFeatureRequest featureRequest;
    featureRequest.setFilterRect( rect );
    featureRequest.setFlags( Qgis::Qgis::FeatureRequestFlag::NoGeometry );
    QgsFeatureIterator fit = mDevelopersMapLayer->getFeatures( featureRequest );
    QgsFeature f;
    int featureCount = 0;
    while ( fit.nextFeature( f ) )
    {
      if ( ++featureCount > 1 )
      {
        break;
      }
    }

    if ( featureCount == 1 )
    {
      QString details = QStringLiteral( "**%1**" ).arg( f.attribute( QStringLiteral( "Name" ) ).toString() );
      QString gitNickname = f.attribute( QStringLiteral( "GIT Nickname" ) ).toString();
      if ( !gitNickname.isEmpty() )
      {
        details += QStringLiteral( " / [@%1](https://github.com/%1/)" ).arg( gitNickname );
      }
      if ( f.attribute( QStringLiteral( "Committer" ) ).toBool() )
      {
        details += QStringLiteral( "\n\n%1" ).arg( tr( "Committer" ) );
      }

      mDevelopersMapFloatingPanel->setText( details );
      mDevelopersMapFloatingPanel->show();
    }
    else
    {
      if ( featureCount > 1 )
      {
        mCanvas->zoomWithCenter( e->x(), e->y(), true );
      }
      mDevelopersMapFloatingPanel->hide();
    }
  }
  else
  {
    QgsMapToolPan::canvasReleaseEvent( e );
  }
}


QgsDevelopersMapFloatingPanel::QgsDevelopersMapFloatingPanel( QWidget *parent )
  : QgsFloatingWidget( parent )
{
  QVBoxLayout *layout = new QVBoxLayout( this );
  mLabel = new QLabel( this );
  mLabel->setAutoFillBackground( true );
  mLabel->setFrameShape( QFrame::StyledPanel );
  mLabel->setFrameShadow( QFrame::Plain );
  mLabel->setMargin( 10 );
  mLabel->setTextFormat( Qt::MarkdownText );
  mLabel->setOpenExternalLinks( true );
  mLabel->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Expanding );
  layout->addWidget( mLabel );
  layout->setSizeConstraint( QLayout::SetFixedSize );

  setLayout( layout );
  setSizePolicy( QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding );
}

void QgsDevelopersMapFloatingPanel::setText( const QString &text )
{
  mLabel->setText( text );
}

QgsDevelopersMapCanvas::QgsDevelopersMapCanvas( QWidget *parent )
  : QgsMapCanvas( parent )
{
  mDevelopersMapBaseLayer = std::make_unique<QgsRasterLayer>( QStringLiteral( "type=xyz&tilePixelRatio=1&url=https://tile.openstreetmap.org/%7Bz%7D/%7Bx%7D/%7By%7D.png&zmax=19&zmin=0&crs=EPSG3857" ), QStringLiteral( "OpenStreetMap" ), QLatin1String( "wms" ) );
  mDevelopersMapLayer = std::make_unique<QgsVectorLayer>( QgsApplication::pkgDataPath() + QStringLiteral( "/resources/data/contributors.json" ), tr( "Contributors" ), QLatin1String( "ogr" ) );
  bool ok = false;
  qDebug() << ( QgsApplication::pkgDataPath() + QStringLiteral( "/resources/data/contributors_map.qml" ) );
  mDevelopersMapLayer->loadNamedStyle( QgsApplication::pkgDataPath() + QStringLiteral( "/resources/data/contributors_map.qml" ), ok );

  QgsCoordinateTransform transform( mDevelopersMapLayer->crs(), mDevelopersMapBaseLayer->crs(), QgsProject::instance()->transformContext() );
  QgsRectangle extent = mDevelopersMapLayer->extent();
  try
  {
    extent = transform.transform( extent );
  }
  catch ( const QgsException &e )
  {
    Q_UNUSED( e )
    extent = mDevelopersMapBaseLayer->extent();
  }
  extent.scale( 1.05 );

  mapSettings().setLayers( QList<QgsMapLayer *>() << mDevelopersMapLayer.get() << mDevelopersMapBaseLayer.get() );
  mapSettings().setDestinationCrs( mDevelopersMapBaseLayer->crs() );
  mapSettings().setExtent( extent );

  setCanvasColor( palette().color( QPalette::Window ) );
  refresh();

  mDevelopersMapTool = std::make_unique<QgsDevelopersMapTool>( this, mDevelopersMapLayer.get() );
  setMapTool( mDevelopersMapTool.get() );
}
