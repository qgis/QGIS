/***************************************************************************
                         qgsintersection2circles.cpp
                         ----------------------
    begin                : October 2021
    copyright            : (C) 2021 by Antoine Facchini
    email                : antoine dot facchini at oslandia dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <QPushButton>

#include "qgisapp.h"
#include "qgsapplication.h"
#include "qgsfeatureaction.h"
#include "qgsmapcanvas.h"
#include "qgsmaptooledit.h"
#include "qgsmaptoolemitpoint.h"
#include "qgsmessagebar.h"
#include "qgsproject.h"
#include "qgsrubberband.h"
#include "qgssettings.h"
#include "qgssettingsregistrycore.h"
#include "qgsvectorlayer.h"
#include "qgsmapmouseevent.h"

#include "qgsintersection2circles.h"


QgsSnapPoint::QgsSnapPoint( QgsMapCanvas *canvas )
  : QgsMapToolCapture( canvas, QgisApp::instance()->cadDockWidget(), QgsSnapPoint::CapturePoint )
{}

void QgsSnapPoint::cadCanvasPressEvent( QgsMapMouseEvent *e )
{
  emit selectPoint( mapPoint( *e ), e->button() );
}



QgsIntersection2CirclesDialog::QgsIntersection2CirclesDialog( QgsMapCanvas *mapCanvas, QWidget *parent ) : QDialog( parent )
{
  setupUi( this );

  mMapCanvas = mapCanvas;
  mMapToolPoint = new QgsSnapPoint( mMapCanvas );

  mDefaultColor = QgsMapToolEdit::digitizingStrokeColor();
  mSelectedColor = QgsProject::instance()->selectionColor();

  initCircleParameters( mRubberCircle1, mRubberInter1, mBtnIntersection1,
                        mX1, mY1, mRadius1, mSelectCenter1, CircleNum1 );
  initCircleParameters( mRubberCircle2, mRubberInter2, mBtnIntersection2,
                        mX2, mY2, mRadius2, mSelectCenter2, CircleNum2 );

  connect( mButtonBox, &QDialogButtonBox::accepted, this, &QgsIntersection2CirclesDialog::onAccepted );
  mButtonBox->button( QDialogButtonBox::Ok )->setEnabled( false );
}

void QgsIntersection2CirclesDialog::initCircleParameters( QgsRubberBand *&rubberCircle, QgsRubberBand *&rubberInter,
    QRadioButton *btnIntersection, QgsDoubleSpinBox *x,
    QgsDoubleSpinBox *y, QgsDoubleSpinBox *radius,
    QToolButton *selectCenter, CircleNumber circleNum )
{
  rubberCircle = new QgsRubberBand( mMapCanvas );
  rubberCircle->setWidth( 2 );
  rubberCircle->setColor( mDefaultColor );

  rubberInter = new QgsRubberBand( mMapCanvas, QgsWkbTypes::PointGeometry );
  rubberInter->setIconSize( 10 );
  rubberInter->setWidth( 2 );
  rubberInter->setIcon( QgsRubberBand::ICON_CROSS );
  rubberInter->setColor( mDefaultColor );

  connect( btnIntersection, &QRadioButton::toggled,
  [ = ]() { selectIntersection( rubberInter, btnIntersection ); } );

  const double maxValue = std::numeric_limits<double>::max();
  const double minValue = -maxValue;

  x->setMinimum( minValue );
  x->setMaximum( maxValue );
  x->setClearValue( 0.0 );
  y->setMinimum( minValue );
  y->setMaximum( maxValue );
  y->setClearValue( 0.0 );
  radius->setMaximum( maxValue );
  radius->setClearValue( 0.0 );

  connect( selectCenter, &QToolButton::pressed,
  [ = ]() { toggleSelectCenter( circleNum ); } );

  connect( x, QOverload<double>::of( &QgsDoubleSpinBox::valueChanged ),
  [ = ]( double ) { propertiesChanged(); } );
  connect( y, QOverload<double>::of( &QgsDoubleSpinBox::valueChanged ),
  [ = ]( double ) { propertiesChanged(); } );
  connect( radius, QOverload<double>::of( &QgsDoubleSpinBox::valueChanged ),
  [ = ]( double ) { propertiesChanged(); } );
}

void QgsIntersection2CirclesDialog::show()
{
  mRubberCircle1->show();
  mRubberCircle2->show();

  mRubberInter1->show();
  mRubberInter2->show();

  QDialog::show();
}

void QgsIntersection2CirclesDialog::clearInformations()
{
  mX1->clear();
  mY1->clear();
  mX2->clear();
  mY2->clear();
  mRadius1->clear();
  mRadius2->clear();
  mIntersection1.clear();
  mIntersection2.clear();

  mBtnIntersection1->setAutoExclusive( false );
  mBtnIntersection1->setChecked( false );
  mBtnIntersection2->setChecked( false );
  mBtnIntersection1->setAutoExclusive( true );
}

void QgsIntersection2CirclesDialog::hideDrawings()
{
  mMapCanvas->unsetMapTool( mMapToolPoint );

  mRubberCircle1->reset();
  mRubberCircle1->hide();
  mRubberCircle2->reset();
  mRubberCircle2->hide();

  mRubberInter1->reset();
  mRubberInter1->hide();
  mRubberInter2->reset();
  mRubberInter2->hide();
}

void QgsIntersection2CirclesDialog::reject()
{
  clearInformations();
  hideDrawings();
  QDialog::reject();
}

void QgsIntersection2CirclesDialog::onAccepted()
{
  QgsVectorLayer *vlayer = qobject_cast<QgsVectorLayer *>( mMapCanvas->currentLayer() );
  if ( !vlayer || vlayer->geometryType() != QgsWkbTypes::PointGeometry || !vlayer->isEditable() )
  {
    QgisApp::instance()->messageBar()->pushMessage(
      QObject::tr( "No editable point layer" ),
      Qgis::MessageLevel::Warning );
    return;
  }

  QgsPoint intersection;
  if ( mBtnIntersection1->isEnabled() && mBtnIntersection1->isChecked() )
  {
    intersection = mIntersection1;
  }
  else if ( mBtnIntersection2->isEnabled() && mBtnIntersection2->isChecked() )
  {
    intersection = mIntersection2;
  }
  else
  {
    return;
  }

  const QgsCoordinateTransform ct = mMapCanvas->mapSettings().layerTransform( vlayer );
  if ( ct.isValid() )
  {
    intersection.transform( ct, Qgis::TransformDirection::Reverse );
  }

  QgsPoint *point = intersection.clone();

  if ( QgsWkbTypes::hasZ( vlayer->wkbType() ) )
    point->addZValue( QgsSettingsRegistryCore::settingsDigitizingDefaultZValue.value() );
  if ( QgsWkbTypes::hasM( vlayer->wkbType() ) )
    point->addMValue( QgsSettingsRegistryCore::settingsDigitizingDefaultMValue.value() );

  QgsFeature f;
  const QgsFields fields = vlayer->fields();
  f = QgsFeature();
  f.setGeometry( QgsGeometry( point ) );

  QgsFeatureAction action( tr( "Feature added" ), f, vlayer );
  action.addFeature();

  clearInformations();
  hideDrawings();
}

void QgsIntersection2CirclesDialog::toggleSelectCenter( CircleNumber circleNum )
{
  disconnect( mMapToolPoint, &QgsSnapPoint::selectPoint, 0, 0 );

  mMapCanvas->setMapTool( mMapToolPoint );

  connect( mMapToolPoint, &QgsSnapPoint::selectPoint,
           [ = ]( const QgsPoint & point, Qt::MouseButton button )
  {
    updateCenterPoint( circleNum, point, button );
  } );
}

void QgsIntersection2CirclesDialog::updateCenterPoint( CircleNumber circleNum, const QgsPoint &point, Qt::MouseButton button )
{
  if ( button != Qt::LeftButton )
  {
    mMapCanvas->unsetMapTool( mMapToolPoint );
    return;
  }

  switch ( circleNum )
  {
    case CircleNum1:
      mX1->setValue( point.x() );
      mY1->setValue( point.y() );
      break;
    case CircleNum2:
      mX2->setValue( point.x() );
      mY2->setValue( point.y() );
  }
}

void QgsIntersection2CirclesDialog::propertiesChanged()
{
  QgsPoint center( mX1->value(), mY1->value() );
  mCircle1 = QgsCircle( center, mRadius1->value() );

  center = QgsPoint( mX2->value(), mY2->value() );
  mCircle2 = QgsCircle( center, mRadius2->value() );

  const int numOfIntersections = mCircle1.intersections( mCircle2, mIntersection1, mIntersection2 );
  mRubberInter1->setToGeometry( QgsGeometry( mIntersection1.clone() ) );
  mRubberInter2->setToGeometry( QgsGeometry( mIntersection2.clone() ) );

  switch ( numOfIntersections )
  {
    case 0:
      mBtnIntersection1->setEnabled( false );
      mRubberInter1->hide();
      mBtnIntersection2->setEnabled( false );
      mRubberInter2->hide();

      mButtonBox->button( QDialogButtonBox::Ok )->setEnabled( false );
      break;

    case 1:
      mBtnIntersection1->setEnabled( true );
      mBtnIntersection1->setChecked( true ); // prevent the case where mBtnIntersection2 is already checked
      mBtnIntersection2->setEnabled( false );
      mRubberInter2->hide();

      mButtonBox->button( QDialogButtonBox::Ok )->setEnabled( true );
      break;

    case 2:
      mBtnIntersection1->setEnabled( true );
      mBtnIntersection2->setEnabled( true );

      mButtonBox->button( QDialogButtonBox::Ok )->setEnabled( true );
      break;

    default:
      break;
  }

  updateCircle();
  mRubberInter1->updatePosition();
  mRubberInter2->updatePosition();
}

void QgsIntersection2CirclesDialog::updateCircle()
{
  mRubberCircle1->setToGeometry( QgsGeometry( mCircle1.toCircularString( true )->clone() ) );
  mRubberCircle2->setToGeometry( QgsGeometry( mCircle2.toCircularString( true )->clone() ) );
}

void QgsIntersection2CirclesDialog::selectIntersection( QgsRubberBand *intersection, QRadioButton *button )
{
  const QColor color = button->isChecked() ? mSelectedColor : mDefaultColor;
  intersection->setColor( color );
  intersection->update();
}
