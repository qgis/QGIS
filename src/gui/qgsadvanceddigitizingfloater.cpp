/***************************************************************************
  qgsadvanceddigitizingfloater.cpp  -  floater for CAD tools
  ----------------------
  begin                : May 2019
  copyright            : (C) Olivier Dalang
  email                : olivier.dalang@gmail.com
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

#include "qgsadvanceddigitizingfloater.h"
#include "qgsmessagelog.h"
#include "qgsmapcanvas.h"
#include "qgssettings.h"
#include "qgsfocuswatcher.h"

QgsAdvancedDigitizingFloater::QgsAdvancedDigitizingFloater( QgsMapCanvas *canvas, QgsAdvancedDigitizingDockWidget *cadDockWidget )
  : QWidget( canvas->viewport() ), mMapCanvas( canvas ), mCadDockWidget( cadDockWidget )
{
  setupUi( this );

  setAttribute( Qt::WA_TransparentForMouseEvents );
  adjustSize();

  setActive( QgsSettings().value( QStringLiteral( "/Cad/Floater" ), false ).toBool() );

  hideIfDisabled();

  // This is required to be able to track mouse move events
  mMapCanvas->viewport()->installEventFilter( this );
  mMapCanvas->viewport()->setMouseTracking( true );

  // We reuse cadDockWidget's eventFilter for the CAD specific shortcuts
  mAngleLineEdit->installEventFilter( cadDockWidget );
  mDistanceLineEdit->installEventFilter( cadDockWidget );
  mXLineEdit->installEventFilter( cadDockWidget );
  mYLineEdit->installEventFilter( cadDockWidget );

  // Connect all cadDockWidget's signals to update the widget's display
  connect( cadDockWidget, &QgsAdvancedDigitizingDockWidget::cadEnabledChanged, this, &QgsAdvancedDigitizingFloater::hideIfDisabled );

  connect( cadDockWidget, &QgsAdvancedDigitizingDockWidget::valueXChanged, this, &QgsAdvancedDigitizingFloater::changeX );
  connect( cadDockWidget, &QgsAdvancedDigitizingDockWidget::valueYChanged, this, &QgsAdvancedDigitizingFloater::changeY );
  connect( cadDockWidget, &QgsAdvancedDigitizingDockWidget::valueAngleChanged, this, &QgsAdvancedDigitizingFloater::changeAngle );
  connect( cadDockWidget, &QgsAdvancedDigitizingDockWidget::valueDistanceChanged, this, &QgsAdvancedDigitizingFloater::changeDistance );

  connect( cadDockWidget, &QgsAdvancedDigitizingDockWidget::lockXChanged, this, &QgsAdvancedDigitizingFloater::changeLockX );
  connect( cadDockWidget, &QgsAdvancedDigitizingDockWidget::lockYChanged, this, &QgsAdvancedDigitizingFloater::changeLockY );
  connect( cadDockWidget, &QgsAdvancedDigitizingDockWidget::lockAngleChanged, this, &QgsAdvancedDigitizingFloater::changeLockAngle );
  connect( cadDockWidget, &QgsAdvancedDigitizingDockWidget::lockDistanceChanged, this, &QgsAdvancedDigitizingFloater::changeLockDistance );

  connect( cadDockWidget, &QgsAdvancedDigitizingDockWidget::relativeXChanged, this, &QgsAdvancedDigitizingFloater::changeRelativeX );
  connect( cadDockWidget, &QgsAdvancedDigitizingDockWidget::relativeYChanged, this, &QgsAdvancedDigitizingFloater::changeRelativeY );
  connect( cadDockWidget, &QgsAdvancedDigitizingDockWidget::relativeAngleChanged, this, &QgsAdvancedDigitizingFloater::changeRelativeAngle );
  // distance is always relative

  connect( cadDockWidget, &QgsAdvancedDigitizingDockWidget::focusOnXRequested, this, &QgsAdvancedDigitizingFloater::focusOnX );
  connect( cadDockWidget, &QgsAdvancedDigitizingDockWidget::focusOnYRequested, this, &QgsAdvancedDigitizingFloater::focusOnY );
  connect( cadDockWidget, &QgsAdvancedDigitizingDockWidget::focusOnAngleRequested, this, &QgsAdvancedDigitizingFloater::focusOnAngle );
  connect( cadDockWidget, &QgsAdvancedDigitizingDockWidget::focusOnDistanceRequested, this, &QgsAdvancedDigitizingFloater::focusOnDistance );

  connect( cadDockWidget, &QgsAdvancedDigitizingDockWidget::enabledChangedX, this, &QgsAdvancedDigitizingFloater::enabledChangedX );
  connect( cadDockWidget, &QgsAdvancedDigitizingDockWidget::enabledChangedY, this, &QgsAdvancedDigitizingFloater::enabledChangedY );
  connect( cadDockWidget, &QgsAdvancedDigitizingDockWidget::enabledChangedAngle, this, &QgsAdvancedDigitizingFloater::enabledChangedAngle );
  connect( cadDockWidget, &QgsAdvancedDigitizingDockWidget::enabledChangedDistance, this, &QgsAdvancedDigitizingFloater::enabledChangedDistance );

  // Connect our line edits signals to update cadDockWidget's state (implementation copied from QgsAdvancedDigitizingDockWidget)
  connect( mXLineEdit, &QLineEdit::returnPressed, cadDockWidget, [ = ]() { cadDockWidget->setX( mXLineEdit->text(), QgsAdvancedDigitizingDockWidget::WidgetSetMode::ReturnPressed ); } );
  connect( mYLineEdit, &QLineEdit::returnPressed, cadDockWidget, [ = ]() { cadDockWidget->setY( mYLineEdit->text(), QgsAdvancedDigitizingDockWidget::WidgetSetMode::ReturnPressed ); } );
  connect( mAngleLineEdit, &QLineEdit::returnPressed, cadDockWidget, [ = ]() { cadDockWidget->setAngle( mAngleLineEdit->text(), QgsAdvancedDigitizingDockWidget::WidgetSetMode::ReturnPressed ); } );
  connect( mDistanceLineEdit, &QLineEdit::returnPressed, cadDockWidget, [ = ]() { cadDockWidget->setDistance( mDistanceLineEdit->text(), QgsAdvancedDigitizingDockWidget::WidgetSetMode::ReturnPressed ); } );

  connect( mXLineEdit, &QLineEdit::textEdited, cadDockWidget, [ = ]() { cadDockWidget->setX( mXLineEdit->text(), QgsAdvancedDigitizingDockWidget::WidgetSetMode::TextEdited ); } );
  connect( mYLineEdit, &QLineEdit::textEdited, cadDockWidget, [ = ]() { cadDockWidget->setY( mYLineEdit->text(), QgsAdvancedDigitizingDockWidget::WidgetSetMode::TextEdited ); } );
  connect( mAngleLineEdit, &QLineEdit::textEdited, cadDockWidget, [ = ]() { cadDockWidget->setAngle( mAngleLineEdit->text(), QgsAdvancedDigitizingDockWidget::WidgetSetMode::TextEdited ); } );
  connect( mDistanceLineEdit, &QLineEdit::textEdited, cadDockWidget, [ = ]() { cadDockWidget->setDistance( mDistanceLineEdit->text(), QgsAdvancedDigitizingDockWidget::WidgetSetMode::TextEdited ); } );

  QgsFocusWatcher *xWatcher = new QgsFocusWatcher( mXLineEdit );
  connect( xWatcher, &QgsFocusWatcher::focusOut, cadDockWidget, [ = ]() { cadDockWidget->setX( mXLineEdit->text(), QgsAdvancedDigitizingDockWidget::WidgetSetMode::FocusOut ); } );
  QgsFocusWatcher *yWatcher = new QgsFocusWatcher( mYLineEdit );
  connect( yWatcher, &QgsFocusWatcher::focusOut, cadDockWidget, [ = ]() { cadDockWidget->setY( mYLineEdit->text(), QgsAdvancedDigitizingDockWidget::WidgetSetMode::FocusOut ); } );
  QgsFocusWatcher *angleWatcher = new QgsFocusWatcher( mAngleLineEdit );
  connect( angleWatcher, &QgsFocusWatcher::focusOut, cadDockWidget, [ = ]() { cadDockWidget->setAngle( mAngleLineEdit->text(), QgsAdvancedDigitizingDockWidget::WidgetSetMode::FocusOut ); } );
  QgsFocusWatcher *distanceWatcher = new QgsFocusWatcher( mDistanceLineEdit );
  connect( distanceWatcher, &QgsFocusWatcher::focusOut, cadDockWidget, [ = ]() { cadDockWidget->setDistance( mDistanceLineEdit->text(), QgsAdvancedDigitizingDockWidget::WidgetSetMode::FocusOut ); } );

}

bool QgsAdvancedDigitizingFloater::eventFilter( QObject *obj, QEvent *event )
{
  if ( mCadDockWidget->cadEnabled() && mActive )
  {
    if ( event->type() == QEvent::MouseMove )
    {
      // We update the position when mouse moves
      QMouseEvent *mouseEvent = dynamic_cast<QMouseEvent *>( event );
      updatePos( mouseEvent->pos() );
    }
    else if ( event->type() == QEvent::Enter )
    {
      // We show the widget when mouse enters
      QEnterEvent  *enterEvent = dynamic_cast<QEnterEvent *>( event );
      updatePos( enterEvent->pos() );
      setVisible( true );
    }
    else if ( event->type() == QEvent::Leave )
    {
      // We hide the widget when mouse leaves
      setVisible( false );
    }
  }
  return QWidget::eventFilter( obj, event );
}

bool QgsAdvancedDigitizingFloater::active()
{
  return mActive;
}

void QgsAdvancedDigitizingFloater::setActive( bool active )
{
  QgsSettings().setValue( QStringLiteral( "/Cad/Floater" ), active );

  mActive = active;

  hideIfDisabled();
}

void QgsAdvancedDigitizingFloater::updatePos( const QPoint &pos )
{
  // We hardcode a small delta between the mouse position and the widget's position
  move( pos + QPoint( 15, 5 ) );
}

void QgsAdvancedDigitizingFloater::hideIfDisabled()
{
  if ( ! mCadDockWidget->cadEnabled() || ! mActive )
  {
    setVisible( false );
  }
}

void QgsAdvancedDigitizingFloater::changeX( const QString &text )
{
  mXLineEdit->setText( text );
}

void QgsAdvancedDigitizingFloater::changeY( const QString &text )
{
  mYLineEdit->setText( text );
}

void QgsAdvancedDigitizingFloater::changeDistance( const QString &text )
{
  mDistanceLineEdit->setText( text );
}

void QgsAdvancedDigitizingFloater::changeAngle( const QString &text )
{
  mAngleLineEdit->setText( text );
}

void QgsAdvancedDigitizingFloater::changeLockX( bool locked )
{
  if ( !locked )
  {
    mXLineEdit->setStyleSheet( QString() );
    mXLabel->setStyleSheet( QString() );
  }
  else
  {
    mXLineEdit->setStyleSheet( QStringLiteral( "font-weight: bold" ) );
    mXLabel->setStyleSheet( QStringLiteral( "font-weight: bold" ) );
  }
}

void QgsAdvancedDigitizingFloater::changeLockY( bool locked )
{
  if ( !locked )
  {
    mYLineEdit->setStyleSheet( QString() );
    mYLabel->setStyleSheet( QString() );
  }
  else
  {
    mYLineEdit->setStyleSheet( QStringLiteral( "font-weight: bold" ) );
    mYLabel->setStyleSheet( QStringLiteral( "font-weight: bold" ) );
  }
}

void QgsAdvancedDigitizingFloater::changeLockDistance( bool locked )
{
  if ( !locked )
  {
    mDistanceLineEdit->setStyleSheet( QString() );
    mDistanceLabel->setStyleSheet( QString() );
  }
  else
  {
    mDistanceLineEdit->setStyleSheet( QStringLiteral( "font-weight: bold" ) );
    mDistanceLabel->setStyleSheet( QStringLiteral( "font-weight: bold" ) );
  }
}

void QgsAdvancedDigitizingFloater::changeLockAngle( bool locked )
{
  if ( !locked )
  {
    mAngleLineEdit->setStyleSheet( QString() );
    mAngleLabel->setStyleSheet( QString() );
  }
  else
  {
    mAngleLineEdit->setStyleSheet( QStringLiteral( "font-weight: bold" ) );
    mAngleLabel->setStyleSheet( QStringLiteral( "font-weight: bold" ) );
  }
}


void QgsAdvancedDigitizingFloater::changeRelativeX( bool relative )
{
  if ( !relative )
  {
    mXLabel->setText( "x" );
  }
  else
  {
    mXLabel->setText( "Δx" );
  }
  adjustSize();
}

void QgsAdvancedDigitizingFloater::changeRelativeY( bool relative )
{
  if ( !relative )
  {
    mYLabel->setText( "y" );
  }
  else
  {
    mYLabel->setText( "Δy" );
  }
  adjustSize();
}

// distance is always relative

void QgsAdvancedDigitizingFloater::changeRelativeAngle( bool relative )
{
  if ( !relative )
  {
    mAngleLabel->setText( "a" );
  }
  else
  {
    mAngleLabel->setText( "Δa" );
  }
  adjustSize();
}

void QgsAdvancedDigitizingFloater::focusOnX()
{
  if ( mActive )
  {
    mXLineEdit->setFocus();
    mXLineEdit->selectAll();
  }
}

void QgsAdvancedDigitizingFloater::focusOnY()
{
  if ( mActive )
  {
    mYLineEdit->setFocus();
    mYLineEdit->selectAll();
  }
}

void QgsAdvancedDigitizingFloater::focusOnDistance()
{
  if ( mActive )
  {
    mDistanceLineEdit->setFocus();
    mDistanceLineEdit->selectAll();
  }
}

void QgsAdvancedDigitizingFloater::focusOnAngle()
{
  if ( mActive )
  {
    mAngleLineEdit->setFocus();
    mAngleLineEdit->selectAll();
  }
}


void QgsAdvancedDigitizingFloater::enabledChangedX( bool enabled )
{
  mXLineEdit->setVisible( enabled );
  mXLabel->setVisible( enabled );
  adjustSize();
}

void QgsAdvancedDigitizingFloater::enabledChangedY( bool enabled )
{
  mYLineEdit->setVisible( enabled );
  mYLabel->setVisible( enabled );
  adjustSize();
}

void QgsAdvancedDigitizingFloater::enabledChangedDistance( bool enabled )
{
  mDistanceLineEdit->setVisible( enabled );
  mDistanceLabel->setVisible( enabled );
  adjustSize();
}

void QgsAdvancedDigitizingFloater::enabledChangedAngle( bool enabled )
{
  mAngleLineEdit->setVisible( enabled );
  mAngleLabel->setVisible( enabled );
  adjustSize();
}
