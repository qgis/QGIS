

#include <QWindow>
#include <QScreen>
#include <QImageWriter>

#include "qgsappscreenshots.h"

#include "qgsvectorlayerproperties.h"
#include "qgsvectorlayer.h"
#include "qgsproject.h"
#include "qgsmessagelog.h"

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

  QScreen *screen = QGuiApplication::primaryScreen();
  if ( widget )
  {
    const QWindow *window = widget->windowHandle();
    if ( window )
    {
      screen = window->screen();
    }
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
    pix = screen->grabWindow( 0, x, y, w, h );
  }

  const QString &fileName = mSaveDirectory + "/" + name + ".png";
  pix.save( fileName );
  QMetaEnum metaEnum = QMetaEnum::fromType<GrabMode>();
  QgsMessageLog::logMessage( QString( "Screenshot saved: %1 (%2)" ).arg( fileName, metaEnum.key( mode ) ) );
}

void QgsAppScreenShots::takeScreenshots( Categories categories )
{
  if ( !categories || categories.testFlag( VectorLayerProperties ) )
    takeVectorLayerProperties();
}

void QgsAppScreenShots::takeVectorLayerProperties()
{
  QgsVectorLayerProperties *dlg = new QgsVectorLayerProperties( mVectorLayer );
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
    saveScreenshot( name, dlg );
  }
  // ------------------
  // style menu clicked
  dlg->mOptionsListWidget->setCurrentRow( 0 );
  QCoreApplication::processEvents();
  dlg->mBtnStyle->click();
  saveScreenshot( "style", dlg );

  // exit properly
  dlg->close();
  dlg->deleteLater();
}

