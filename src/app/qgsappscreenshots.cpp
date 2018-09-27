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

#include "qgsappscreenshots.h"

#include "qgsvectorlayerproperties.h"
#include "qgsvectorlayer.h"
#include "qgsproject.h"
#include "qgsmessagelog.h"
#include "qgisapp.h"

QgsAppScreenShots::QgsAppScreenShots( const QString &saveDirectory )
  : mSaveDirectory( saveDirectory )
{
  QString layerDef = QStringLiteral( "Point?crs=epsg:4326&field=pk:integer&field=my_text:string&field=my_integer:integer&field=my_double:double&key=pk" );
  mVectorLayer = new QgsVectorLayer( layerDef, QStringLiteral( "Layer" ), QStringLiteral( "memory" ) );
  QgsProject::instance()->addMapLayer( mVectorLayer );
}

void QgsAppScreenShots::saveScreenshot( const QString &name, QWidget *widget, GrabMode mode )
{
  QPixmap pix;
  int x = 0;
  int y = 0;
  int w = -1;
  int h = -1;

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
      const QRect geom = widget->frameGeometry();
      QPoint tl = geom.topLeft();
      x = tl.x();
      y = tl.y();
      w = geom.width();
      h = geom.height();
    }
  }
  if ( !widget || mode != GrabWidget )
  {
    WId wid = widget ? widget->winId() : 0;
    pix = scr->grabWindow( wid, x, y, w, h );
  }

  const QString &fileName = mSaveDirectory + "/" + name + ".png";
  pix.save( fileName );
  QMetaEnum metaEnum = QMetaEnum::fromType<GrabMode>();
  QgsMessageLog::logMessage( QString( "Screenshot saved: %1 (%2)" ).arg( fileName, metaEnum.key( mode ) ) );
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

void QgsAppScreenShots::takeScreenshots( Categories categories )
{
  if ( !categories || categories.testFlag( VectorLayerProperties ) )
    takeVectorLayerProperties();
}


// ----------------------
// !!!!! SCREENSHOTS !!!!

void QgsAppScreenShots::takeVectorLayerProperties()
{
  QString rootName = QLatin1String( "vectorlayerproperties_" );
  QgsVectorLayerProperties *dlg = new QgsVectorLayerProperties( mVectorLayer, QgisApp::instance() );
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
    saveScreenshot( rootName + name, dlg );
  }
  // ------------------
  // style menu clicked
  dlg->mOptionsListWidget->setCurrentRow( 0 );
  dlg->adjustSize();
  moveWidgetTo( dlg, Qt::BottomLeftCorner );
  QCoreApplication::processEvents();
  dlg->mBtnStyle->click();
  QCoreApplication::processEvents();
  saveScreenshot( rootName + "style_menu", dlg );
  QCoreApplication::processEvents();
  dlg->mBtnStyle->menu()->hide();
  QCoreApplication::processEvents();

  // exit properly
  dlg->close();
  dlg->deleteLater();
}

