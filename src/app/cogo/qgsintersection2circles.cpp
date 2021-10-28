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

#include "qgsfeatureaction.h"
#include "qgsmapcanvas.h"
#include "qgsmaptoolemitpoint.h"
#include "qgsrubberband.h"
#include "qgsvectorlayer.h"

#include "qgsintersection2circles.h"


QgsIntersection2CirclesDialog::QgsIntersection2CirclesDialog( QgsMapCanvas *mapCanvas, QgsVectorLayer *vlayer, QWidget *parent ) : QDialog( parent )
{
  setupUi( this );

  mLayer = vlayer;
  mMapCanvas = mapCanvas;

  initCircleParameters( mRubberCircle1, mRubberInter1, mBtnIntersection1,
                        mX1, mY1, mRadius1, mSelectCenter1, CircleNum1 );
  initCircleParameters( mRubberCircle2, mRubberInter2, mBtnIntersection2,
                        mX2, mY2, mRadius2, mSelectCenter2, CircleNum2 );

  connect( mButtonBox, &QDialogButtonBox::accepted, this, &QgsIntersection2CirclesDialog::onAccepted );
  mButtonBox->button( QDialogButtonBox::Ok )->setEnabled( false );
}

void QgsIntersection2CirclesDialog::initCircleParameters( QgsRubberBand *&rubberCircle, QgsRubberBand *&rubberInter,
    QCheckBox *btnIntersection, QgsDoubleSpinBox *x,
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

  connect( btnIntersection, &QCheckBox::stateChanged,
  [ = ]() { selectIntersection( rubberInter, btnIntersection ); } );

  const double maxValue = std::numeric_limits<double>::max();
  const double minValue = -maxValue;

  x->setMinimum( minValue );
  x->setMaximum( maxValue );
  y->setMinimum( minValue );
  y->setMaximum( maxValue );
  radius->setMaximum( maxValue );

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

void QgsIntersection2CirclesDialog::hideDrawings()
{
  delete mMapToolPoint;
  mMapToolPoint = nullptr;

  mRubberCircle1->hide();
  mRubberCircle2->hide();

  mRubberInter1->hide();
  mRubberInter2->hide();
}

void QgsIntersection2CirclesDialog::reject()
{
  hideDrawings();
  QDialog::reject();
}

void QgsIntersection2CirclesDialog::onAccepted()
{
  auto addFeature = [ = ]( QgsPoint point )
  {
    QgsFeature f;
    const QgsFields fields = mLayer->fields();
    f = QgsFeature();
    f.setGeometry( QgsGeometry( point.clone() ) );

    QgsFeatureAction action( tr( "Feature added" ), f, mLayer );
    action.addFeature();
  };

  if ( mBtnIntersection1->isEnabled() && mBtnIntersection1->isChecked() )
  {
    addFeature( mIntersection1 );
  }

  if ( mBtnIntersection2->isEnabled() && mBtnIntersection2->isChecked() )
  {
    addFeature( mIntersection2 );
  }

  hideDrawings();
}

void QgsIntersection2CirclesDialog::toggleSelectCenter( CircleNumber circleNum )
{
  if ( mMapToolPoint )
  {
    delete mMapToolPoint;
    mMapToolPoint = nullptr;
  }

  mMapToolPoint = new QgsMapToolEmitPoint( mMapCanvas );
  mMapCanvas->setMapTool( mMapToolPoint );

  connect( mMapToolPoint, &QgsMapToolEmitPoint::canvasClicked,
           [ = ]( const QgsPointXY & point, Qt::MouseButton button )
  {
    updateCenterPoint( circleNum, point, button );
  } );
}

void QgsIntersection2CirclesDialog::updateCenterPoint( CircleNumber circleNum, const QgsPointXY &point, Qt::MouseButton button )
{
  if ( button != Qt::LeftButton )
  {
    delete mMapToolPoint;
    mMapToolPoint = nullptr;
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
}

void QgsIntersection2CirclesDialog::updateCircle()
{
  mRubberCircle1->setToGeometry( QgsGeometry( mCircle1.toCircularString( true )->clone() ) );
  mRubberCircle2->setToGeometry( QgsGeometry( mCircle2.toCircularString( true )->clone() ) );
}

void QgsIntersection2CirclesDialog::selectIntersection( QgsRubberBand *intersection, QCheckBox *button )
{
  const QColor color = button->isChecked() ? mSelectedColor : mDefaultColor;
  intersection->setColor( color );
  intersection->update();
}
