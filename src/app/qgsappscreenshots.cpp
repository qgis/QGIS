/***************************************************************************
  qgsappscreenshots.cpp
  --------------------------------------
  Date                 : September 2018
  Copyright            : (C) 2018 by Denis Rouzaud
  Email                : denis@opengis.ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


#include <QMenu>
#include <QWindow>
#include <QScreen>
#include <QImageWriter>
#include <QIcon>
#include <QImage>

#include "qgsappscreenshots.h"

#include "qgsvectorlayerproperties.h"
#include "qgsvectorlayer.h"
#include "qgsproject.h"
#include "qgsmessagelog.h"
#include "qgisapp.h"
#include "qgsrendererpropertiesdialog.h"
#include "qgs25drendererwidget.h"
#include "qgsapplication.h"


QgsAppScreenShots::QgsAppScreenShots( const QString &saveDirectory )
  : mSaveDirectory( saveDirectory )
{
  QString layerDef = QStringLiteral( "Point?crs=epsg:4326&field=pk:integer&field=my_text:string&field=my_integer:integer&field=my_double:double&key=pk" );
  mLineLayer = new QgsVectorLayer( layerDef, QStringLiteral( "Line Layer" ), QStringLiteral( "memory" ) );
  layerDef = QStringLiteral( "Polygon?crs=epsg:2056&field=pk:integer&field=my_text:string&field=my_integer:integer&field=height:double&key=pk" );
  mPolygonLayer = new QgsVectorLayer( layerDef, QStringLiteral( "Polygon Layer" ), QStringLiteral( "memory" ) );

  QgsProject::instance()->addMapLayers( QList<QgsMapLayer *>()
                                        << mLineLayer
                                        << mPolygonLayer );
}

QPixmap QgsAppScreenShots::takeScreenshot( QWidget *widget, GrabMode mode )
{
  QPixmap pix;
  QRect geom;

  QScreen *scr = screen( widget );
  if ( widget )
  {
    widget->raise();
    if ( mode == GrabWidget )
    {
      pix = widget->grab();
    }
    else if ( mode == GrabWidgetAndFrame )
    {
      geom = widget->frameGeometry();
    }
  }
  if ( !widget || mode != GrabWidget )
  {
    WId wid = widget ? widget->winId() : 0;
    pix = scr->grabWindow( wid );
    if ( !geom.isEmpty() )
    {
      qreal dpr = scr->devicePixelRatio();
      pix = pix.copy( static_cast<int>( geom.x() * dpr ),
                      static_cast<int>( geom.y() * dpr ),
                      static_cast<int>( geom.width() * dpr ),
                      static_cast<int>( geom.height() * dpr ) );
    }
  }
  return pix;
}

void QgsAppScreenShots::takeScreenshot( const QString &name, QWidget *widget, QgsAppScreenShots::GrabMode mode )
{
  QPixmap pixmap = takeScreenshot( widget, mode );
  saveScreenshot( pixmap, name );
}

void QgsAppScreenShots::saveScreenshot( QPixmap &pixmap, const QString &name, QRect crop, bool gradient )
{
  if ( !crop.isNull() )
  {
    if ( crop.height() == 0 )
      crop.setHeight( pixmap.height() );
    if ( crop.width() == 0 )
      crop.setWidth( pixmap.width() );
  }
  if ( !crop.isEmpty() )
    pixmap = pixmap.copy( crop );


  if ( gradient )
  {
    QImage img = pixmap.toImage();
    QLinearGradient linearGrad( QPointF( 0, pixmap.height() - 200 ), QPointF( 0, pixmap.height() - 20 ) );
    linearGrad.setColorAt( 0, Qt::transparent );
    linearGrad.setColorAt( 1, Qt::white );

    // create image and fill it with gradient
    QImage image( pixmap.width(), pixmap.height(), QImage::Format_ARGB32 );
    QPainter painter( &img );
    painter.fillRect( img.rect(), linearGrad );
    pixmap = QPixmap::fromImage( img );
  }

  const QString &fileName = mSaveDirectory + "/" + name + ".png";
  pixmap.save( fileName );
  QgsMessageLog::logMessage( QString( "Screenshot saved: %1" ).arg( fileName ) );
}

void QgsAppScreenShots::moveWidgetTo( QWidget *widget, Qt::Corner corner, Reference reference )
{
  QRect screenGeom;
  switch ( reference )
  {
    case Screen:
      screenGeom = screen( widget )->geometry();
      break;
    case Widget:
    case QgisApp:
      // TODO
      return;
  }

  switch ( corner )
  {
    case Qt::BottomLeftCorner:
      widget->move( 0, screenGeom.height() - widget->frameGeometry().height() );
      break;
    case Qt::BottomRightCorner:
    case Qt::TopRightCorner:
    case Qt::TopLeftCorner:
      // TODO
      return;
  }
}

QScreen *QgsAppScreenShots::screen( QWidget *widget )
{
  QScreen *screen = QGuiApplication::primaryScreen();
  if ( widget )
  {
    const QWindow *window = widget->windowHandle();
    if ( window )
    {
      screen = window->screen();
    }
  }
  return screen;
}

void QgsAppScreenShots::takePicturesOf( Categories categories )
{
  if ( !categories || categories.testFlag( Symbol25D ) )
    take25dSymbol();

  if ( !categories || categories.testFlag( VectorLayerProperties ) )
    takeVectorLayerProperties();
}


// ----------------------
// !!!!! SCREENSHOTS !!!!

void QgsAppScreenShots::takeVectorLayerProperties()
{
  QString rootName = QLatin1String( "vectorlayerproperties_" );
  QgsVectorLayerProperties *dlg = new QgsVectorLayerProperties( mLineLayer, QgisApp::instance() );
  dlg->show();
  // ----------------
  // do all the pages
  for ( int row = 0; row < dlg->mOptionsListWidget->count(); ++row )
  {
    dlg->mOptionsListWidget->setCurrentRow( row );
    dlg->adjustSize();
    QCoreApplication::processEvents();
    QString name = dlg->mOptionsListWidget->item( row )[0].text().toLower();
    name.replace( " ", "_" );
    takeScreenshot( rootName + name, dlg );
  }
  // ------------------
  // style menu clicked
  dlg->mOptionsListWidget->setCurrentRow( 0 );
  dlg->adjustSize();
  moveWidgetTo( dlg, Qt::BottomLeftCorner );
  QCoreApplication::processEvents();
  dlg->mBtnStyle->click();
  QCoreApplication::processEvents();
  takeScreenshot( rootName + "style_menu", dlg );
  QCoreApplication::processEvents();
  dlg->mBtnStyle->menu()->hide();
  QCoreApplication::processEvents();

  // exit properly
  dlg->close();
  dlg->deleteLater();
}

void QgsAppScreenShots::take25dSymbol()
{
  QString rootName = QLatin1String( "vectorlayerproperties_" );
  QgsVectorLayerProperties *dlg = new QgsVectorLayerProperties( mPolygonLayer, QgisApp::instance() );
  dlg->show();
  dlg->mOptionsListWidget->setCurrentRow( 2 );
  Q_ASSERT( dlg->mOptionsListWidget->currentItem()->icon().pixmap( 24, 24 ).toImage()
            == QgsApplication::getThemeIcon( QStringLiteral( "/propertyicons/symbology.svg" ) ).pixmap( 24, 24 ).toImage() );
  int idx = dlg->mRendererDialog->cboRenderers->findData( QLatin1String( "25dRenderer" ) );
  Q_ASSERT( idx >= 0 );
  dlg->mRendererDialog->cboRenderers->setCurrentIndex( idx );
  QCoreApplication::processEvents();
  Qgs25DRendererWidget *w = dynamic_cast<Qgs25DRendererWidget *>( dlg->mRendererDialog->mActiveWidget );
  w->mHeightWidget->setField( QLatin1String( "height" ) );
  Q_ASSERT( w->mHeightWidget->expression() == QLatin1String( "\"height\"" ) );
  QCoreApplication::processEvents();
  dlg->adjustSize();
  QCoreApplication::processEvents();
  QPixmap pixmap = takeScreenshot( dlg );
  saveScreenshot( pixmap, rootName + QLatin1String( "25dsymbol" ), QRect( 0, 0, 0, 800 ), true );

// exit properly
  dlg->close();
  dlg->deleteLater();
}

