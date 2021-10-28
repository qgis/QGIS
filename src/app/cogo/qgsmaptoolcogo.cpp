/***************************************************************************
                         qgsmaptoolcogo.cpp
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

#include "qgsmaptoolcogo.h"
#include "qgis.h"
#include "qgsrubberband.h"
#include "qgsvectorlayer.h"
#include "qgsmaptoolemitpoint.h"
#include "qgsmapcanvas.h"
#include "qgsattributedialog.h"
#include "qgsattributeeditorcontext.h"
#include "qgsfeatureaction.h"

#include <iostream>
#include <QPushButton>

QgsIntersection2CirclesDialog::QgsIntersection2CirclesDialog( QgsMapCanvas *mapCanva, QgsVectorLayer *vlayer, QWidget *parent ) : QDialog( parent )
{
  setupUi( this );

  mLayer = vlayer;
  mMapCanva = mapCanva;

  mRubberCircle1 = new QgsRubberBand( mapCanva );
  mRubberCircle2 = new QgsRubberBand( mapCanva );

  mRubberCircle1->setWidth( 2 );
  mRubberCircle2->setWidth( 2 );
  mRubberCircle1->setColor( QColor( 0, 255, 0, 150 ) );
  mRubberCircle2->setColor( QColor( 0, 255, 0, 150 ) );

  mRubberInter1 = new QgsRubberBand( mapCanva, QgsWkbTypes::PointGeometry );
  mRubberInter2 = new QgsRubberBand( mapCanva, QgsWkbTypes::PointGeometry );

  mRubberInter1->setIconSize( 10 );
  mRubberInter2->setIconSize( 10 );
  mRubberInter1->setWidth( 2 );
  mRubberInter2->setWidth( 2 );
  mRubberInter1->setIcon( QgsRubberBand::ICON_CROSS );
  mRubberInter2->setIcon( QgsRubberBand::ICON_CROSS );
  mRubberInter1->setColor( QColor( 0, 255, 0, 150 ) );
  mRubberInter2->setColor( QColor( 0, 255, 0, 150 ) );

  connect( mBtnIntersection1, &QCheckBox::stateChanged,
  [ = ]() { selectIntersection( mRubberInter1, mBtnIntersection1 ); } );
  connect( mBtnIntersection2, &QCheckBox::stateChanged,
  [ = ]() { selectIntersection( mRubberInter2, mBtnIntersection2 ); } );

  connect( mButtonBox, &QDialogButtonBox::accepted, this, &QgsIntersection2CirclesDialog::onAccepted );
  mButtonBox->button( QDialogButtonBox::Ok )->setEnabled( false );

  const double maxValue = std::numeric_limits<double>::max();
  const double minValue = -maxValue;

  mX1->setMinimum( minValue );
  mX1->setMaximum( maxValue );
  mY1->setMinimum( minValue );
  mY1->setMaximum( maxValue );
  mRadius1->setMaximum( maxValue );

  mX2->setMinimum( minValue );
  mX2->setMaximum( maxValue );
  mY2->setMinimum( minValue );
  mY2->setMaximum( maxValue );
  mRadius2->setMaximum( maxValue );

  // Circle 1
  connect( mSelectCenter1, &QToolButton::pressed,
  [ = ]() { toggleSelectCenter( CircleNum1 ); } );

  connect( mX1, QOverload<double>::of( &QgsDoubleSpinBox::valueChanged ),
  [ = ]( double ) { propertiesChanged( CircleNum1 ); } );
  connect( mY1, QOverload<double>::of( &QgsDoubleSpinBox::valueChanged ),
  [ = ]( double ) { propertiesChanged( CircleNum1 ); } );
  connect( mRadius1, QOverload<double>::of( &QgsDoubleSpinBox::valueChanged ),
  [ = ]( double ) { propertiesChanged( CircleNum1 ); } );

  // Circle 2
  connect( mSelectCenter2, &QToolButton::pressed,
  [ = ]() { toggleSelectCenter( CircleNum2 ); } );

  connect( mX2, QOverload<double>::of( &QgsDoubleSpinBox::valueChanged ),
  [ = ]( double ) { propertiesChanged( CircleNum2 ); } );
  connect( mY2, QOverload<double>::of( &QgsDoubleSpinBox::valueChanged ),
  [ = ]( double ) { propertiesChanged( CircleNum2 ); } );
  connect( mRadius2, QOverload<double>::of( &QgsDoubleSpinBox::valueChanged ),
  [ = ]( double ) { propertiesChanged( CircleNum2 ); } );
}

void QgsIntersection2CirclesDialog::show()
{
  mRubberCircle1->show();
  mRubberCircle2->show();

  mRubberInter1->show();
  mRubberInter2->show();

  QDialog::show();
}

void QgsIntersection2CirclesDialog::reject()
{
  delete mMapToolPoint;
  mMapToolPoint = nullptr;

  mRubberCircle1->hide();
  mRubberCircle2->hide();

  mRubberInter1->hide();
  mRubberInter2->hide();

  QDialog::reject();
}

void QgsIntersection2CirclesDialog::onAccepted()
{
  QgsFeature f;
  QgsFields fields = mLayer->fields();

  if ( mBtnIntersection1->isEnabled() && mBtnIntersection1->isChecked() )
  {
    f = QgsFeature();
    f.setGeometry( QgsGeometry( mIntersection1.clone() ) );

    QgsFeatureAction action( tr( "Feature added" ), f, mLayer );
    action.addFeature();
  }

  if ( mBtnIntersection2->isEnabled() && mBtnIntersection2->isChecked() )
  {
    f = QgsFeature();
    f.setGeometry( QgsGeometry( mIntersection2.clone() ) );

    QgsFeatureAction action( tr( "Feature added" ), f, mLayer );
    action.addFeature();
  }
}

void QgsIntersection2CirclesDialog::toggleSelectCenter( CircleNumber circleNum )
{
  if ( mMapToolPoint )
  {
    delete mMapToolPoint;
    mMapToolPoint = nullptr;
  }

  mMapToolPoint = new QgsMapToolEmitPoint( mMapCanva );
  mMapCanva->setMapTool( mMapToolPoint );

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

  if ( circleNum == CircleNum1 )
  {
    mX1->setValue( point.x() );
    mY1->setValue( point.y() );
  }
  else
  {
    mX2->setValue( point.x() );
    mY2->setValue( point.y() );
  }
}

void QgsIntersection2CirclesDialog::propertiesChanged( CircleNumber circleNum )
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

  updateCircle( circleNum );
}

void QgsIntersection2CirclesDialog::updateCircle( CircleNumber circleNum )
{
  mRubberCircle1->setToGeometry( QgsGeometry( mCircle1.toCircularString( true )->clone() ) );
  mRubberCircle2->setToGeometry( QgsGeometry( mCircle2.toCircularString( true )->clone() ) );
}

void QgsIntersection2CirclesDialog::selectIntersection( QgsRubberBand *intersection, QCheckBox *button )
{
  const QColor color = button->isChecked() ? QColor( 0, 255, 0, 150 ) : QColor( Qt::blue );
  intersection->setColor( color );
  intersection->update();
}
