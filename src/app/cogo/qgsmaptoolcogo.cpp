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
#include "qgsgeometryrubberband.h"
#include "qgsvectorlayer.h"

#include <iostream>
#include <QPushButton>

QgsIntersection2CirclesDialog::QgsIntersection2CirclesDialog( QgsMapCanvas *mapCanva, QgsVectorLayer *vlayer, QWidget *parent ) : QDialog( parent )
{
  setupUi( this );

  mLayer = vlayer;

  mRubberCircle1 = new QgsGeometryRubberBand( mapCanva );
  mRubberCircle2 = new QgsGeometryRubberBand( mapCanva );

  mRubberCircle1->setStrokeWidth( 2 );
  mRubberCircle1->setIconType( QgsGeometryRubberBand::ICON_NONE );
  mRubberCircle2->setStrokeWidth( 2 );
  mRubberCircle2->setIconType( QgsGeometryRubberBand::ICON_NONE );

  connect( mButtonBox, &QDialogButtonBox::accepted, this, &QgsIntersection2CirclesDialog::onAccepted );
  mButtonBox->button( QDialogButtonBox::Ok )->setEnabled( false );

  // Circle 1
//  connect( mSelectCenter1, &QRadioButton::pressed, SIGNAL( selectCenter( CircleNum1 ) ) );

  connect( mX1, QOverload<double>::of( &QgsDoubleSpinBox::valueChanged ),
  [ = ]( double ) { propertiesChanged( CircleNum1 ); } );
  connect( mY1, QOverload<double>::of( &QgsDoubleSpinBox::valueChanged ),
  [ = ]( double ) { propertiesChanged( CircleNum1 ); } );
  connect( mRadius1, QOverload<double>::of( &QgsDoubleSpinBox::valueChanged ),
  [ = ]( double ) { propertiesChanged( CircleNum1 ); } );

  // Circle 2
//  connect( mSelectCenter2, &QRadioButton::pressed, this, SIGNAL( selectCenter( CircleNum2 ) ) );

  connect( mX2, QOverload<double>::of( &QgsDoubleSpinBox::valueChanged ),
  [ = ]( double ) { propertiesChanged( CircleNum2 ); } );
  connect( mY2, QOverload<double>::of( &QgsDoubleSpinBox::valueChanged ),
  [ = ]( double ) { propertiesChanged( CircleNum2 ); } );
  connect( mRadius2, QOverload<double>::of( &QgsDoubleSpinBox::valueChanged ),
  [ = ]( double ) { propertiesChanged( CircleNum2 ); } );
}

void QgsIntersection2CirclesDialog::onAccepted()
{
}

void QgsIntersection2CirclesDialog::propertiesChanged( CircleNumber circleNum )
{
  QgsPoint center( mX1->value(), mY1->value() );
  mCircle1 = QgsCircle( center, mRadius1->value() );

  center = QgsPoint( mX2->value(), mY2->value() );
  mCircle2 = QgsCircle( center, mRadius2->value() );

  updateCircle( circleNum );
}

void QgsIntersection2CirclesDialog::updateCircle( CircleNumber circleNum )
{
  mRubberCircle1->setGeometry( mCircle1.toCircularString( true )->clone() );
  mRubberCircle2->setGeometry( mCircle2.toCircularString( true )->clone() );
}
