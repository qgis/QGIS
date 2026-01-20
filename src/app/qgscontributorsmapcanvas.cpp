/***************************************************************************
                          qgscontributorsmapcanvas.cpp
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

#include "qgscontributorsmapcanvas.h"

#include "qgsapplication.h"
#include "qgsfeaturerequest.h"
#include "qgsmaplayerstylemanager.h"
#include "qgsmapmouseevent.h"

#include <QCursor>
#include <QFrame>
#include <QVBoxLayout>

#include "moc_qgscontributorsmapcanvas.cpp"

QgsContributorsMapTool::QgsContributorsMapTool( QgsMapCanvas *canvas, QgsVectorLayer *layer )
  : QgsMapToolPan( canvas )
  , mContributorsMapLayer( layer )
{
  mContributorsMapFloatingPanel = new QgsContributorsMapFloatingPanel( canvas );
  mContributorsMapFloatingPanel->setAnchorWidget( canvas );
  mContributorsMapFloatingPanel->setAnchorWidgetPoint( QgsFloatingWidget::BottomRight );
  mContributorsMapFloatingPanel->setAnchorPoint( QgsFloatingWidget::BottomRight );
  mContributorsMapFloatingPanel->hide();
}

void QgsContributorsMapTool::canvasMoveEvent( QgsMapMouseEvent *e )
{
  if ( !isPinching() && !( e->buttons() & Qt::LeftButton ) )
  {
    QgsFeatureRequest featureRequest;
    featureRequest.setFilterRect( filterRectForMouseEvent( e ) );
    featureRequest.setLimit( 1 );
    featureRequest.setNoAttributes();
    featureRequest.setFlags( Qgis::Qgis::FeatureRequestFlag::NoGeometry );
    QgsFeatureIterator fit = mContributorsMapLayer->getFeatures( featureRequest );
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

void QgsContributorsMapTool::canvasReleaseEvent( QgsMapMouseEvent *e )
{
  if ( !isPinching() && !isDragging() )
  {
    QgsFeatureRequest featureRequest;
    featureRequest.setFilterRect( filterRectForMouseEvent( e ) );
    featureRequest.setFlags( Qgis::Qgis::FeatureRequestFlag::NoGeometry );
    QgsFeatureIterator fit = mContributorsMapLayer->getFeatures( featureRequest );
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
      QString details = u"**%1**"_s.arg( f.attribute( u"Name"_s ).toString() );
      QString gitNickname = f.attribute( u"GIT Nickname"_s ).toString();
      if ( !gitNickname.isEmpty() )
      {
        details += u" / [@%1](https://github.com/%1/)"_s.arg( gitNickname );
      }
      if ( f.attribute( u"Committer"_s ).toBool() )
      {
        details += u"\n\n%1"_s.arg( tr( "Committer" ) );
      }

      mContributorsMapFloatingPanel->setText( details );
      mContributorsMapFloatingPanel->show();
    }
    else
    {
      if ( featureCount > 1 )
      {
        mCanvas->zoomWithCenter( e->x(), e->y(), true );
      }
      mContributorsMapFloatingPanel->hide();
    }
  }
  else
  {
    QgsMapToolPan::canvasReleaseEvent( e );
  }
}

QgsRectangle QgsContributorsMapTool::filterRectForMouseEvent( QgsMapMouseEvent *e )
{
  const QgsMapSettings mapSettings = mCanvas->mapSettings();
  const QgsRenderContext context = QgsRenderContext::fromMapSettings( mapSettings );
  double searchRadius = context.convertToMapUnits( 3.5, Qgis::RenderUnit::Millimeters );
  const QgsPointXY point = mCanvas->getCoordinateTransform()->toMapCoordinates( e->x(), e->y() );
  return toLayerCoordinates( mContributorsMapLayer, QgsRectangle( point.x() - searchRadius, point.y() - searchRadius, point.x() + searchRadius, point.y() + searchRadius ) );
}


QgsContributorsMapFloatingPanel::QgsContributorsMapFloatingPanel( QWidget *parent )
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

void QgsContributorsMapFloatingPanel::setText( const QString &text )
{
  mLabel->setText( text );
}

QgsContributorsMapCanvas::QgsContributorsMapCanvas( QWidget *parent )
  : QgsMapCanvas( parent )
{
  mContributorsMapBaseLayer = std::make_unique<QgsRasterLayer>( u"type=xyz&tilePixelRatio=1&url=https://tile.openstreetmap.org/%7Bz%7D/%7Bx%7D/%7By%7D.png&zmax=19&zmin=0&crs=EPSG3857"_s, u"OpenStreetMap"_s, "wms"_L1 );
  mContributorsMapLayer = std::make_unique<QgsVectorLayer>( QgsApplication::pkgDataPath() + u"/resources/data/contributors.json"_s, tr( "Contributors" ), "ogr"_L1 );
  bool ok = false;
  mContributorsMapLayer->loadNamedStyle( QgsApplication::pkgDataPath() + u"/resources/data/contributors_map.qml"_s, ok );

  QgsCoordinateTransform transform( mContributorsMapLayer->crs(), mContributorsMapBaseLayer->crs(), QgsProject::instance()->transformContext() );
  QgsRectangle extent = mContributorsMapLayer->extent();
  try
  {
    extent = transform.transformBoundingBox( extent );
  }
  catch ( const QgsException &e )
  {
    Q_UNUSED( e )
    extent = mContributorsMapBaseLayer->extent();
  }
  extent.scale( 1.05 );

  mapSettings().setFlag( Qgis::MapSettingsFlag::UseRenderingOptimization, true );
  mapSettings().setFlag( Qgis::MapSettingsFlag::RenderPartialOutput, true );
  mapSettings().setLayers( QList<QgsMapLayer *>() << mContributorsMapLayer.get() << mContributorsMapBaseLayer.get() );
  mapSettings().setDestinationCrs( mContributorsMapBaseLayer->crs() );
  mapSettings().setExtent( extent );

  setParallelRenderingEnabled( true );
  setPreviewJobsEnabled( true );
  setCanvasColor( palette().color( QPalette::Window ) );
  refresh();

  mContributorsMapTool = std::make_unique<QgsContributorsMapTool>( this, mContributorsMapLayer.get() );
  setMapTool( mContributorsMapTool.get() );
}
