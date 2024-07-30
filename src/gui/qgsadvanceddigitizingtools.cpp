/***************************************************************************
     qgsadvanceddigitizingintersectiontools.cpp
     ----------------------
     begin                : May 2024
     copyright            : (C) Mathieu Pellerin
     email                : mathieu at opengis dot ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <QMouseEvent>
#include <QEnterEvent>
#include <QLocale>

#include "qgsadvanceddigitizingtools.h"
#include "qgsapplication.h"
#include "qgsdoublespinbox.h"
#include "qgsmapcanvas.h"

QgsAdvancedDigitizingTool::QgsAdvancedDigitizingTool( QgsMapCanvas *canvas, QgsAdvancedDigitizingDockWidget *cadDockWidget )
  : QObject( canvas ? canvas->viewport() : nullptr )
  , mMapCanvas( canvas )
  , mCadDockWidget( cadDockWidget )
{
}

QgsAdvancedDigitizingCirclesIntersectionTool::QgsAdvancedDigitizingCirclesIntersectionTool( QgsMapCanvas *canvas, QgsAdvancedDigitizingDockWidget *cadDockWidget )
  : QgsAdvancedDigitizingTool( canvas, cadDockWidget )
{
}

QgsAdvancedDigitizingCirclesIntersectionTool::~QgsAdvancedDigitizingCirclesIntersectionTool()
{
  if ( mToolWidget )
  {
    mToolWidget->deleteLater();
  }
}

QWidget *QgsAdvancedDigitizingCirclesIntersectionTool::createWidget()
{
  QWidget *toolWidget = new QWidget();

  QGridLayout *layout = new QGridLayout( toolWidget );
  layout->setContentsMargins( 0, 0, 0, 0 );
  toolWidget->setLayout( layout );

  QLabel *label = new QLabel( QStringLiteral( "Circle #1" ), toolWidget );
  layout->addWidget( label, 0, 0, 1, 3 );

  mCircle1Digitize = new QToolButton( toolWidget );
  mCircle1Digitize->setCheckable( true );
  mCircle1Digitize->setChecked( false );
  mCircle1Digitize->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/mActionMapIdentification.svg" ) ) );
  layout->addWidget( mCircle1Digitize, 1, 2, 2, 1 );

  label = new QLabel( QStringLiteral( "X" ), toolWidget );
  layout->addWidget( label, 1, 0 );

  mCircle1X = new QgsDoubleSpinBox( toolWidget );
  mCircle1X->setMinimum( std::numeric_limits<double>::min() );
  mCircle1X->setMaximum( std::numeric_limits<double>::max() );
  connect( mCircle1X, &QgsDoubleSpinBox::textEdited, this, [ = ]() { mCircle1Digitize->setChecked( false ); } );
  layout->addWidget( mCircle1X, 1, 1 );

  label = new QLabel( QStringLiteral( "Y" ), toolWidget );
  layout->addWidget( label, 2, 0 );

  mCircle1Y = new QgsDoubleSpinBox( toolWidget );
  mCircle1Y->setMinimum( std::numeric_limits<double>::min() );
  mCircle1Y->setMaximum( std::numeric_limits<double>::max() );
  connect( mCircle1Y, &QgsDoubleSpinBox::textEdited, this, [ = ]() { mCircle1Digitize->setChecked( false ); } );
  layout->addWidget( mCircle1Y, 2, 1 );

  label = new QLabel( QStringLiteral( "Distance" ), toolWidget );
  layout->addWidget( label, 3, 0 );

  mCircle1Distance = new QgsDoubleSpinBox( toolWidget );
  mCircle1Distance->setMinimum( 0 );
  mCircle1Distance->setMaximum( std::numeric_limits<double>::max() );
  connect( mCircle1Distance, &QgsDoubleSpinBox::returnPressed, this, [ = ]() { mCircle2Digitize->setChecked( true ); } );
  layout->addWidget( mCircle1Distance, 3, 1 );

  label = new QLabel( QStringLiteral( "Circle #2" ), toolWidget );
  layout->addWidget( label, 4, 0, 1, 3 );

  mCircle2Digitize = new QToolButton( toolWidget );
  mCircle2Digitize->setCheckable( true );
  mCircle2Digitize->setChecked( false );
  mCircle2Digitize->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/mActionMapIdentification.svg" ) ) );
  layout->addWidget( mCircle2Digitize, 5, 2, 2, 1 );

  label = new QLabel( QStringLiteral( "X" ), toolWidget );
  layout->addWidget( label, 5, 0 );

  mCircle2X = new QgsDoubleSpinBox( toolWidget );
  mCircle2X->setMinimum( std::numeric_limits<double>::min() );
  mCircle2X->setMaximum( std::numeric_limits<double>::max() );
  connect( mCircle2X, &QgsDoubleSpinBox::textEdited, this, [ = ]() { mCircle2Digitize->setChecked( false ); } );
  layout->addWidget( mCircle2X, 5, 1 );

  label = new QLabel( QStringLiteral( "Y" ), toolWidget );
  layout->addWidget( label, 6, 0 );

  mCircle2Y = new QgsDoubleSpinBox( toolWidget );
  mCircle2Y->setMinimum( std::numeric_limits<double>::min() );
  mCircle2Y->setMaximum( std::numeric_limits<double>::max() );
  connect( mCircle2Y, &QgsDoubleSpinBox::textEdited, this, [ = ]() { mCircle2Digitize->setChecked( false ); } );
  layout->addWidget( mCircle2Y, 6, 1 );

  label = new QLabel( QStringLiteral( "Distance" ), toolWidget );
  layout->addWidget( label, 7, 0 );

  mCircle2Distance = new QgsDoubleSpinBox( toolWidget );
  mCircle2Distance->setMinimum( 0 );
  mCircle2Distance->setMaximum( std::numeric_limits<double>::max() );
  layout->addWidget( mCircle2Distance, 7, 1 );

  connect( mCircle1X, static_cast < void ( QgsDoubleSpinBox::* )( double ) > ( &QgsDoubleSpinBox::valueChanged ), this, [ = ]( double ) { processParameters(); } );
  connect( mCircle1Y, static_cast < void ( QgsDoubleSpinBox::* )( double ) > ( &QgsDoubleSpinBox::valueChanged ), this, [ = ]( double ) { processParameters(); } );
  connect( mCircle1Distance, static_cast < void ( QgsDoubleSpinBox::* )( double ) > ( &QgsDoubleSpinBox::valueChanged ), this, [ = ]( double ) { processParameters(); } );
  connect( mCircle2X, static_cast < void ( QgsDoubleSpinBox::* )( double ) > ( &QgsDoubleSpinBox::valueChanged ), this, [ = ]( double ) { processParameters(); } );
  connect( mCircle2Y, static_cast < void ( QgsDoubleSpinBox::* )( double ) > ( &QgsDoubleSpinBox::valueChanged ), this, [ = ]( double ) { processParameters(); } );
  connect( mCircle2Distance, static_cast < void ( QgsDoubleSpinBox::* )( double ) > ( &QgsDoubleSpinBox::valueChanged ), this, [ = ]( double ) { processParameters(); } );

  mCircle1Digitize->setChecked( true );

  mToolWidget = toolWidget;
  return toolWidget;
}

void QgsAdvancedDigitizingCirclesIntersectionTool::processParameters()
{
  mP1 = QgsPointXY();
  mP2 = QgsPointXY();
  QgsGeometryUtils::circleCircleIntersections( QgsPointXY( mCircle1X->value(), mCircle1Y->value() ), mCircle1Distance->value(),
      QgsPointXY( mCircle2X->value(), mCircle2Y->value() ), mCircle2Distance->value(),
      mP1, mP2 );
  emit paintRequested();
}

void QgsAdvancedDigitizingCirclesIntersectionTool::canvasMoveEvent( QgsMapMouseEvent *event )
{
  if ( mCircle1Digitize->isChecked() )
  {
    mCircle1X->setValue( event->mapPoint().x() );
    mCircle1Y->setValue( event->mapPoint().y() );
  }
  else if ( mCircle2Digitize->isChecked() )
  {
    mCircle2X->setValue( event->mapPoint().x() );
    mCircle2Y->setValue( event->mapPoint().y() );
  }

  if ( !mP1.isEmpty() )
  {
    mP1Closest = QgsGeometryUtils::distance2D( QgsPoint( mP1 ), QgsPoint( event->mapPoint() ) ) < QgsGeometryUtils::distance2D( QgsPoint( mP2 ), QgsPoint( event->mapPoint() ) );
  }

  event->setAccepted( false );
}

void QgsAdvancedDigitizingCirclesIntersectionTool::canvasReleaseEvent( QgsMapMouseEvent *event )
{
  if ( mCircle1Digitize->isChecked() )
  {
    mCircle1X->setValue( event->mapPoint().x() );
    mCircle1Y->setValue( event->mapPoint().y() );
    mCircle1Digitize->setChecked( false );
    mCircle1Distance->setFocus();
    event->setAccepted( false );
    return;
  }
  else if ( mCircle2Digitize->isChecked() )
  {
    mCircle2X->setValue( event->mapPoint().x() );
    mCircle2Y->setValue( event->mapPoint().y() );
    mCircle2Digitize->setChecked( false );
    mCircle2Distance->setFocus();
    event->setAccepted( false );
    return;
  }

  if ( !mP1.isEmpty() )
  {
    mP1Closest = QgsGeometryUtils::distance2D( QgsPoint( mP1 ), QgsPoint( event->mapPoint() ) ) < QgsGeometryUtils::distance2D( QgsPoint( mP2 ), QgsPoint( event->mapPoint() ) );
    event->setMapPoint( mP1Closest ? mP1 : mP2 );
    if ( mToolWidget )
    {
      mToolWidget->deleteLater();
    }
    deleteLater();
    return;
  }

  event->setAccepted( false );
}

void QgsAdvancedDigitizingCirclesIntersectionTool::drawCircle( QPainter *painter, double x, double y, double distance )
{
  painter->setPen( QPen( QColor( 0, 127, 0, 200 ), 2 ) );
  painter->setBrush( Qt::NoBrush );

  mapCanvas()->getCoordinateTransform()->transformInPlace( x, y );
  painter->drawLine( QLineF( x - 3, y - 3, x + 3, y + 3 ) );
  painter->drawLine( QLineF( x - 3, y + 3, x + 3, y - 3 ) );

  painter->setPen( QPen( QColor( 0, 127, 0, 150 ), 1, Qt::DashLine ) );
  distance = distance / mapCanvas()->mapSettings().mapUnitsPerPixel();
  painter->drawEllipse( QRectF( x - distance, y - distance, distance * 2, distance * 2 ) );
}

void QgsAdvancedDigitizingCirclesIntersectionTool::drawCandidate( QPainter *painter, double x, double y, bool closest )
{
  if ( closest )
  {
    painter->setPen( QPen( QColor( 127, 0, 0, 222 ), 4 ) );
  }
  else
  {
    painter->setPen( QPen( QColor( 0, 127, 0, 222 ), 2 ) );
  }

  mapCanvas()->getCoordinateTransform()->transformInPlace( x, y );
  double size = closest ? 5 : 3;
  painter->drawRect( QRectF( x - size, y - size, size * 2, size * 2 ) );
}

void QgsAdvancedDigitizingCirclesIntersectionTool::paint( QPainter *painter )
{
  painter->save();

  drawCircle( painter, mCircle1X->value(), mCircle1Y->value(), mCircle1Distance->value() );
  drawCircle( painter, mCircle2X->value(), mCircle2Y->value(), mCircle2Distance->value() );

  if ( !mP1.isEmpty() )
  {
    if ( mP1Closest )
    {
      drawCandidate( painter, mP2.x(), mP2.y(), false );
      drawCandidate( painter, mP1.x(), mP1.y(), true );
    }
    else
    {
      drawCandidate( painter, mP1.x(), mP1.y(), false );
      drawCandidate( painter, mP2.x(), mP2.y(), true );
    }
  }

  painter->restore();
}
