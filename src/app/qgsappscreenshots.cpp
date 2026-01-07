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


#include "qgsappscreenshots.h"

#include "options/qgsoptions.h"
#include "qgisapp.h"
#include "qgs25drendererwidget.h"
#include "qgsapplication.h"
#include "qgsguiutils.h"
#include "qgsmessagelog.h"
#include "qgsproject.h"
#include "qgsrasterlayer.h"
#include "qgsrasterlayerproperties.h"
#include "qgsrendererpropertiesdialog.h"
#include "qgsvectorlayer.h"
#include "qgsvectorlayerjoininfo.h"
#include "qgsvectorlayerproperties.h"

#include <QIcon>
#include <QImage>
#include <QImageWriter>
#include <QMenu>
#include <QScreen>
#include <QWindow>

#include "moc_qgsappscreenshots.cpp"

QgsAppScreenShots::QgsAppScreenShots( const QString &saveDirectory )
  : mSaveDirectory( saveDirectory )
{
  QString layerDef = u"Point?crs=epsg:4326&field=pk:integer&field=my_text:string&field=fk_polygon:integer&field=my_double:double&key=pk"_s;
  const QgsVectorLayer::LayerOptions options { QgsProject::instance()->transformContext() };
  mLineLayer = new QgsVectorLayer( layerDef, u"Line Layer"_s, u"memory"_s, options );
  layerDef = u"Polygon?crs=epsg:2056&field=pk:integer&field=my_text:string&field=my_integer:integer&field=height:double&key=pk"_s;
  mPolygonLayer = new QgsVectorLayer( layerDef, u"Polygon Layer"_s, u"memory"_s, options );

  const QString dataPath( TEST_DATA_DIR ); //defined in CmakeLists.txt
  mRasterLayer = new QgsRasterLayer( dataPath + "/raster/with_color_table.tif", u"raster"_s, u"gdal"_s );
  Q_ASSERT( mRasterLayer->isValid() );

  // add join
  QgsVectorLayerJoinInfo join;
  join.setTargetFieldName( u"fk_polygon"_s );
  join.setJoinLayer( mPolygonLayer );
  join.setJoinFieldName( u"pk"_s );
  join.setUsingMemoryCache( true );
  join.setEditable( true );
  join.setCascadedDelete( true );
  mLineLayer->addJoin( join );

  // add layers to project
  QgsProject::instance()->addMapLayers( QList<QgsMapLayer *>() << mLineLayer << mPolygonLayer << mRasterLayer );
}

QPixmap QgsAppScreenShots::takeScreenshot( QWidget *widget, GrabMode mode, QRect crop, bool gradient )
{
  QPixmap pixmap;
  QRect geom;

  QScreen *scr = screen( widget );
  if ( widget )
  {
    widget->raise();
    if ( mode == GrabWidget )
    {
      pixmap = widget->grab();
    }
    else if ( mode == GrabWidgetAndFrame )
    {
      geom = widget->frameGeometry();
    }
  }
  if ( !widget || mode != GrabWidget )
  {
    const WId wid = widget ? widget->winId() : 0;
    pixmap = scr->grabWindow( wid );
    if ( !geom.isEmpty() )
    {
      const qreal dpr = scr->devicePixelRatio();
      pixmap = pixmap.copy( static_cast<int>( geom.x() * dpr ), static_cast<int>( geom.y() * dpr ), static_cast<int>( geom.width() * dpr ), static_cast<int>( geom.height() * dpr ) );
    }
  }

  pixmap.setDevicePixelRatio( scr->devicePixelRatio() );

  if ( !crop.isNull() )
  {
    const qreal dpr = scr->devicePixelRatio();
    if ( crop.height() == 0 )
      crop.setHeight( static_cast<int>( pixmap.height() / dpr ) );
    if ( crop.width() == 0 )
      crop.setWidth( static_cast<int>( pixmap.width() / dpr ) );
  }

  if ( !crop.isEmpty() )
  {
    const qreal dpr = scr->devicePixelRatio();
    crop = QRect( static_cast<int>( crop.x() * dpr ), static_cast<int>( crop.y() * dpr ), static_cast<int>( crop.width() * dpr ), static_cast<int>( crop.height() * dpr ) );
    pixmap = pixmap.copy( crop );
  }

  if ( gradient )
  {
    QImage img = pixmap.toImage();
    QLinearGradient linearGrad( QPointF( 0, pixmap.height() - mGradientSize ), QPointF( 0, pixmap.height() - mGradientSize / 10 ) );
    linearGrad.setColorAt( 0, Qt::transparent );
    linearGrad.setColorAt( 1, Qt::white );

    // create image and fill it with gradient
    QPainter painter( &img );
    painter.fillRect( img.rect(), linearGrad );
    pixmap = QPixmap::fromImage( img );
  }

  return pixmap;
}

void QgsAppScreenShots::takeScreenshot( const QString &name, const QString &folder, QWidget *widget, QgsAppScreenShots::GrabMode mode )
{
  QPixmap pixmap = takeScreenshot( widget, mode );
  saveScreenshot( pixmap, name, folder );
}

void QgsAppScreenShots::saveScreenshot( QPixmap &pixmap, const QString &name, const QString &folder )
{
  const QDir topDirectory( mSaveDirectory );
  if ( !topDirectory.exists() )
  {
    QgsMessageLog::logMessage( u"Directory does not exist: %1"_s.arg( mSaveDirectory ), QString(), Qgis::MessageLevel::Critical );
    return;
  }

  const QDir directory( topDirectory.absolutePath() + "/" + folder );
  const QString fileName = directory.absolutePath() + "/" + name + ".png";
  if ( !directory.exists() )
  {
    if ( !topDirectory.mkpath( folder ) )
    {
      QgsMessageLog::logMessage( u"Could not create directory %1 in %2"_s.arg( folder, mSaveDirectory ), QString(), Qgis::MessageLevel::Critical );
      return;
    }
  }
  if ( pixmap.save( fileName ) )
    QgsMessageLog::logMessage( u"Screenshot saved: %1"_s.arg( fileName ) );
  else
    QgsMessageLog::logMessage( u"Failed to save screenshot: %1"_s.arg( fileName ), QString(), Qgis::MessageLevel::Critical );
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
  if ( !categories || categories.testFlag( GlobalOptions ) )
    takeGlobalOptions();

  if ( !categories || categories.testFlag( VectorLayerProperties ) )
  {
    takeVectorLayerProperties25DSymbol();
    takeVectorLayerProperties();
  }

  if ( !categories || categories.testFlag( RasterLayerProperties ) )
    takeRasterLayerProperties();
}

void QgsAppScreenShots::setGradientSize( int size )
{
  mGradientSize = size;
}


// ----------------------
// !!!!! SCREENSHOTS !!!!

void QgsAppScreenShots::takeVectorLayerProperties()
{
  const QString folder = u"working_with_vector/img/auto_generated/vector_layer_properties"_s;
  QgsVectorLayerProperties *dlg = new QgsVectorLayerProperties( QgisApp::instance()->mapCanvas(), QgisApp::instance()->visibleMessageBar(), mLineLayer, QgisApp::instance() );
  dlg->show();
  dlg->mJoinTreeWidget->expandAll(); // expand join tree
  // ----------------
  // do all the pages
  for ( int row = 0; row < dlg->mOptionsListWidget->count(); ++row )
  {
    dlg->mOptionsListWidget->setCurrentRow( row );
    dlg->adjustSize();
    QCoreApplication::processEvents();
    QString name = dlg->mOptionsListWidget->item( row )[0].text().toLower();
    name.replace( " "_L1, "_"_L1 ).replace( "&"_L1, "and"_L1 );
    takeScreenshot( name, folder, dlg );
  }
  // ------------------
  // style menu clicked
  dlg->mOptionsListWidget->setCurrentRow( 0 );
  dlg->adjustSize();
  moveWidgetTo( dlg, Qt::BottomLeftCorner );
  QCoreApplication::processEvents();
  dlg->mBtnStyle->click();
  QCoreApplication::processEvents();
  takeScreenshot( u"style_menu"_s, folder, dlg );
  QCoreApplication::processEvents();
  dlg->mBtnStyle->menu()->hide();
  QCoreApplication::processEvents();

  // exit properly
  dlg->close();
  dlg->deleteLater();
}

//---------------

void QgsAppScreenShots::takeVectorLayerProperties25DSymbol()
{
  const QString folder = u"working_with_vector/img/auto_generated/vector_layer_properties/"_s;
  QgsVectorLayerProperties *dlg = new QgsVectorLayerProperties( QgisApp::instance()->mapCanvas(), QgisApp::instance()->visibleMessageBar(), mPolygonLayer, QgisApp::instance() );
  dlg->show();
  dlg->mOptionsListWidget->setCurrentRow( 2 );
  Q_ASSERT( dlg->mOptionsListWidget->currentItem()->icon().pixmap( 24, 24 ).toImage() == QgsApplication::getThemeIcon( u"/propertyicons/symbology.svg"_s ).pixmap( 24, 24 ).toImage() );
  const int idx = dlg->mRendererDialog->cboRenderers->findData( "25dRenderer"_L1 );
  Q_ASSERT( idx >= 0 );
  dlg->mRendererDialog->cboRenderers->setCurrentIndex( idx );
  QCoreApplication::processEvents();
  Qgs25DRendererWidget *w = qobject_cast<Qgs25DRendererWidget *>( dlg->mRendererDialog->mActiveWidget );
  w->mHeightWidget->setField( u"height"_s );
  Q_ASSERT( w->mHeightWidget->expression() == "\"height\""_L1 );
  QCoreApplication::processEvents();
  dlg->adjustSize();
  QCoreApplication::processEvents();
  const int cropHeight = w->mAdvancedConfigurationBox->mapTo( dlg, w->mAdvancedConfigurationBox->frameGeometry().bottomLeft() ).y();
  QPixmap pixmap = takeScreenshot( dlg, GrabWidgetAndFrame, QRect( 0, 0, 0, cropHeight ), true );
  saveScreenshot( pixmap, u"25dsymbol"_s, folder );

  // exit properly
  dlg->close();
  dlg->deleteLater();
}

//---------------

void QgsAppScreenShots::takeGlobalOptions()
{
  const QString folder = u"introduction/img/auto_generated/global_options/"_s;
  QgsOptions *dlg = QgisApp::instance()->createOptionsDialog();
  dlg->setMinimumHeight( 600 );
  dlg->show();
  QCoreApplication::processEvents();

  for ( int page = 0; page < dlg->mOptionsStackedWidget->count(); ++page )
  {
    dlg->mOptionsStackedWidget->setCurrentIndex( page );
    dlg->adjustSize();
    QCoreApplication::processEvents();
    QString name = dlg->mOptTreeView->currentIndex().data( Qt::DisplayRole ).toString().toLower();
    name.replace( " "_L1, "_"_L1 ).replace( "&"_L1, "and"_L1 );
    takeScreenshot( name, folder, dlg );
  }
  // -----------------
  // advanced settings
  dlg->mOptionsStackedWidget->setCurrentIndex( dlg->mOptionsStackedWidget->count() - 1 );
  QCoreApplication::processEvents();
  Q_ASSERT( dlg->mOptTreeView->currentIndex().data( Qt::DecorationRole ).value<QIcon>().pixmap( 24, 24 ).toImage() == QgsApplication::getThemeIcon( u"/mIconWarning.svg"_s ).pixmap( 24, 24 ).toImage() );
  QWidget *editor = dlg->findChild<QWidget *>( u"mAdvancedSettingsEditor"_s );
  if ( editor )
    editor->show();
  QCoreApplication::processEvents();
  QCoreApplication::processEvents(); // seems a second call is needed, the tabble might not be fully displayed otherwise
  takeScreenshot( u"advanced_with_settings_shown"_s, folder, dlg );

  // exit properly
  dlg->close();
  dlg->deleteLater();
}

//---------------

void QgsAppScreenShots::takeRasterLayerProperties()
{
  const QString folder = u"working_with_raster/img/auto_generated/raster_layer_properties"_s;
  QgsRasterLayerProperties *dlg = new QgsRasterLayerProperties( mRasterLayer, QgisApp::instance()->mapCanvas() );
  dlg->show();
  // ----------------
  // do all the pages
  for ( int row = 0; row < dlg->mOptionsListWidget->count(); ++row )
  {
    dlg->mOptionsListWidget->setCurrentRow( row );
    dlg->adjustSize();
    QCoreApplication::processEvents();
    QString name = dlg->mOptionsListWidget->item( row )[0].text().toLower();
    name.replace( " "_L1, "_"_L1 ).replace( "&"_L1, "and"_L1 );
    takeScreenshot( name, folder, dlg );
  }
  // exit properly
  dlg->close();
  dlg->deleteLater();
}
