/***************************************************************************
     mapcoordsdialog.cpp
     --------------------------------------
    Date                 : 2005
    Copyright            : (C) 2005 by Lars Luthman
    Email                : larsl at users dot sourceforge dot net
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include <QValidator>
#include <QPushButton>

#include "qgsmapcanvas.h"
#include "qgsgeorefvalidators.h"
#include "qgsmapcoordsdialog.h"
#include "qgssettings.h"
#include "qgsmapmouseevent.h"
#include "qgsgui.h"
#include "qgsapplication.h"

QgsMapCoordsDialog::QgsMapCoordsDialog( QgsMapCanvas *qgisCanvas, const QgsPointXY &pixelCoords, QWidget *parent )
  : QDialog( parent, Qt::Dialog )
  , mQgisCanvas( qgisCanvas )
  , mPixelCoords( pixelCoords )
{
  setupUi( this );
  QgsGui::enableAutoGeometryRestore( this );

  connect( buttonBox, &QDialogButtonBox::accepted, this, &QgsMapCoordsDialog::buttonBox_accepted );

  setAttribute( Qt::WA_DeleteOnClose );

  mPointFromCanvasPushButton = new QPushButton( QgsApplication::getThemeIcon( "georeferencer/mPushButtonPencil.png" ), tr( "From Map Canvas" ) );
  mPointFromCanvasPushButton->setCheckable( true );
  buttonBox->addButton( mPointFromCanvasPushButton, QDialogButtonBox::ActionRole );
  mPointFromCanvasPushButton->setFocus();

  // User can input either DD or DMS coords (from QGIS mapcanvas we take DD coords)
  QgsDMSAndDDValidator *validator = new QgsDMSAndDDValidator( this );
  leXCoord->setValidator( validator );
  leYCoord->setValidator( validator );

  mToolEmitPoint = new QgsGeorefMapToolEmitPoint( qgisCanvas );
  mToolEmitPoint->setButton( mPointFromCanvasPushButton );

  QgsSettings settings;
  mMinimizeWindowCheckBox->setChecked( settings.value( QStringLiteral( "/Plugin-GeoReferencer/Config/Minimize" ), QStringLiteral( "1" ) ).toBool() );

  connect( mPointFromCanvasPushButton, &QAbstractButton::clicked, this, &QgsMapCoordsDialog::setToolEmitPoint );

  connect( mToolEmitPoint, &QgsGeorefMapToolEmitPoint::canvasClicked,
           this, &QgsMapCoordsDialog::maybeSetXY );
  connect( mToolEmitPoint, &QgsGeorefMapToolEmitPoint::mouseReleased, this, &QgsMapCoordsDialog::setPrevTool );

  connect( leXCoord, &QLineEdit::textChanged, this, &QgsMapCoordsDialog::updateOK );
  connect( leYCoord, &QLineEdit::textChanged, this, &QgsMapCoordsDialog::updateOK );
  updateOK();
}

QgsMapCoordsDialog::~QgsMapCoordsDialog()
{
  delete mToolEmitPoint;

  QgsSettings settings;
  settings.setValue( QStringLiteral( "/Plugin-GeoReferencer/Config/Minimize" ), mMinimizeWindowCheckBox->isChecked() );
}

void QgsMapCoordsDialog::updateOK()
{
  bool enable = ( leXCoord->text().size() != 0 && leYCoord->text().size() != 0 );
  QPushButton *okPushButton = buttonBox->button( QDialogButtonBox::Ok );
  okPushButton->setEnabled( enable );
}

void QgsMapCoordsDialog::setPrevTool()
{
  mQgisCanvas->setMapTool( mPrevMapTool );
}

void QgsMapCoordsDialog::buttonBox_accepted()
{
  bool ok;
  double x = leXCoord->text().toDouble( &ok );
  if ( !ok )
    x = dmsToDD( leXCoord->text() );

  double y = leYCoord->text().toDouble( &ok );
  if ( !ok )
    y = dmsToDD( leYCoord->text() );

  emit pointAdded( mPixelCoords, QgsPointXY( x, y ) );
  close();
}

void QgsMapCoordsDialog::maybeSetXY( const QgsPointXY &xy, Qt::MouseButton button )
{
  // Only LeftButton should set point
  if ( Qt::LeftButton == button )
  {
    QgsPointXY mapCoordPoint = xy;

    leXCoord->clear();
    leYCoord->clear();
    leXCoord->setText( qgsDoubleToString( mapCoordPoint.x() ) );
    leYCoord->setText( qgsDoubleToString( mapCoordPoint.y() ) );
  }

  parentWidget()->showNormal();
  parentWidget()->activateWindow();
  parentWidget()->raise();

  mPointFromCanvasPushButton->setChecked( false );
  buttonBox->button( QDialogButtonBox::Ok )->setFocus();
  activateWindow();
  raise();
}

void QgsMapCoordsDialog::setToolEmitPoint( bool isEnable )
{
  if ( isEnable )
  {
    if ( mMinimizeWindowCheckBox->isChecked() )
    {
      parentWidget()->showMinimized();
    }

    Q_ASSERT( parentWidget()->parentWidget() );
    parentWidget()->parentWidget()->activateWindow();
    parentWidget()->parentWidget()->raise();

    mPrevMapTool = mQgisCanvas->mapTool();
    mQgisCanvas->setMapTool( mToolEmitPoint );
  }
  else
  {
    mQgisCanvas->setMapTool( mPrevMapTool );
  }
}

double QgsMapCoordsDialog::dmsToDD( const QString &dms )
{
  QStringList list = dms.split( ' ' );
  QString tmpStr = list.at( 0 );
  double res = std::fabs( tmpStr.toDouble() );

  tmpStr = list.value( 1 );
  if ( !tmpStr.isEmpty() )
    res += tmpStr.toDouble() / 60;

  tmpStr = list.value( 2 );
  if ( !tmpStr.isEmpty() )
    res += tmpStr.toDouble() / 3600;

  if ( dms.startsWith( '-' ) )
    return -res;
  else
    return res;
}

QgsGeorefMapToolEmitPoint::QgsGeorefMapToolEmitPoint( QgsMapCanvas *canvas )
  : QgsMapTool( canvas )
{
  mSnapIndicator.reset( new QgsSnapIndicator( canvas ) );
}

void QgsGeorefMapToolEmitPoint::canvasMoveEvent( QgsMapMouseEvent *e )
{
  mSnapIndicator->setMatch( mapPointMatch( e ) );
}

void QgsGeorefMapToolEmitPoint::canvasPressEvent( QgsMapMouseEvent *e )
{
  QgsPointLocator::Match m = mapPointMatch( e );
  emit canvasClicked( m.isValid() ? m.point() : toMapCoordinates( e->pos() ), e->button() );
}

void QgsGeorefMapToolEmitPoint::canvasReleaseEvent( QgsMapMouseEvent *e )
{
  QgsMapTool::canvasReleaseEvent( e );
  emit mouseReleased();
}

void QgsGeorefMapToolEmitPoint::deactivate()
{
  mSnapIndicator->setMatch( QgsPointLocator::Match() );

  QgsMapTool::deactivate();
}

QgsPointLocator::Match QgsGeorefMapToolEmitPoint::mapPointMatch( QMouseEvent *e )
{
  QgsPointXY pnt = toMapCoordinates( e->pos() );
  return canvas()->snappingUtils()->snapToMap( pnt );
}
