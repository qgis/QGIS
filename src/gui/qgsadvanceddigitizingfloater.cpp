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
#include "moc_qgsadvanceddigitizingfloater.cpp"
#include "qgsmapcanvas.h"
#include "qgssettings.h"
#include "qgsfocuswatcher.h"
#include "qgsunittypes.h"

QgsAdvancedDigitizingFloater::QgsAdvancedDigitizingFloater( QgsMapCanvas *canvas, QgsAdvancedDigitizingDockWidget *cadDockWidget )
  : QWidget( canvas->viewport() )
  , mMapCanvas( canvas )
  , mCadDockWidget( cadDockWidget )
{
  setupUi( this );

  setAttribute( Qt::WA_TransparentForMouseEvents );
  adjustSize();

  setActive( QgsSettings().value( QStringLiteral( "/Cad/Floater" ), false ).toBool() );

  hideIfDisabled();

  enabledCommonAngleSnapping( cadDockWidget->commonAngleConstraint() );

  // This is required to be able to track mouse move events
  mMapCanvas->viewport()->installEventFilter( this );
  mMapCanvas->viewport()->setMouseTracking( true );

  // We reuse cadDockWidget's eventFilter for the CAD specific shortcuts
  mAngleLineEdit->installEventFilter( cadDockWidget );
  mDistanceLineEdit->installEventFilter( cadDockWidget );
  mXLineEdit->installEventFilter( cadDockWidget );
  mYLineEdit->installEventFilter( cadDockWidget );
  mZLineEdit->installEventFilter( cadDockWidget );
  mMLineEdit->installEventFilter( cadDockWidget );

  // Connect all cadDockWidget's signals to update the widget's display
  connect( cadDockWidget, &QgsAdvancedDigitizingDockWidget::cadEnabledChanged, this, &QgsAdvancedDigitizingFloater::hideIfDisabled );

  connect( cadDockWidget, &QgsAdvancedDigitizingDockWidget::valueXChanged, this, &QgsAdvancedDigitizingFloater::changeX );
  connect( cadDockWidget, &QgsAdvancedDigitizingDockWidget::valueYChanged, this, &QgsAdvancedDigitizingFloater::changeY );
  connect( cadDockWidget, &QgsAdvancedDigitizingDockWidget::valueZChanged, this, &QgsAdvancedDigitizingFloater::changeZ );
  connect( cadDockWidget, &QgsAdvancedDigitizingDockWidget::valueMChanged, this, &QgsAdvancedDigitizingFloater::changeM );
  connect( cadDockWidget, &QgsAdvancedDigitizingDockWidget::valueAngleChanged, this, &QgsAdvancedDigitizingFloater::changeAngle );
  connect( cadDockWidget, &QgsAdvancedDigitizingDockWidget::valueBearingChanged, this, &QgsAdvancedDigitizingFloater::changeBearing );
  connect( cadDockWidget, &QgsAdvancedDigitizingDockWidget::valueCommonAngleSnappingChanged, this, &QgsAdvancedDigitizingFloater::changeCommonAngleSnapping );
  connect( cadDockWidget, &QgsAdvancedDigitizingDockWidget::commonAngleSnappingShowInFloaterChanged, this, &QgsAdvancedDigitizingFloater::enabledCommonAngleSnapping );
  connect( cadDockWidget, &QgsAdvancedDigitizingDockWidget::valueDistanceChanged, this, &QgsAdvancedDigitizingFloater::changeDistance );

  connect( cadDockWidget, &QgsAdvancedDigitizingDockWidget::lockXChanged, this, &QgsAdvancedDigitizingFloater::changeLockX );
  connect( cadDockWidget, &QgsAdvancedDigitizingDockWidget::lockYChanged, this, &QgsAdvancedDigitizingFloater::changeLockY );
  connect( cadDockWidget, &QgsAdvancedDigitizingDockWidget::lockZChanged, this, &QgsAdvancedDigitizingFloater::changeLockZ );
  connect( cadDockWidget, &QgsAdvancedDigitizingDockWidget::lockMChanged, this, &QgsAdvancedDigitizingFloater::changeLockM );
  connect( cadDockWidget, &QgsAdvancedDigitizingDockWidget::lockAngleChanged, this, &QgsAdvancedDigitizingFloater::changeLockAngle );
  connect( cadDockWidget, &QgsAdvancedDigitizingDockWidget::lockDistanceChanged, this, &QgsAdvancedDigitizingFloater::changeLockDistance );

  connect( cadDockWidget, &QgsAdvancedDigitizingDockWidget::relativeXChanged, this, &QgsAdvancedDigitizingFloater::changeRelativeX );
  connect( cadDockWidget, &QgsAdvancedDigitizingDockWidget::relativeYChanged, this, &QgsAdvancedDigitizingFloater::changeRelativeY );
  connect( cadDockWidget, &QgsAdvancedDigitizingDockWidget::relativeZChanged, this, &QgsAdvancedDigitizingFloater::changeRelativeZ );
  connect( cadDockWidget, &QgsAdvancedDigitizingDockWidget::relativeMChanged, this, &QgsAdvancedDigitizingFloater::changeRelativeM );
  connect( cadDockWidget, &QgsAdvancedDigitizingDockWidget::relativeAngleChanged, this, &QgsAdvancedDigitizingFloater::changeRelativeAngle );
  // distance is always relative

  connect( cadDockWidget, &QgsAdvancedDigitizingDockWidget::focusOnXRequested, this, &QgsAdvancedDigitizingFloater::focusOnX );
  connect( cadDockWidget, &QgsAdvancedDigitizingDockWidget::focusOnYRequested, this, &QgsAdvancedDigitizingFloater::focusOnY );
  connect( cadDockWidget, &QgsAdvancedDigitizingDockWidget::focusOnZRequested, this, &QgsAdvancedDigitizingFloater::focusOnZ );
  connect( cadDockWidget, &QgsAdvancedDigitizingDockWidget::focusOnMRequested, this, &QgsAdvancedDigitizingFloater::focusOnM );
  connect( cadDockWidget, &QgsAdvancedDigitizingDockWidget::focusOnAngleRequested, this, &QgsAdvancedDigitizingFloater::focusOnAngle );
  connect( cadDockWidget, &QgsAdvancedDigitizingDockWidget::focusOnDistanceRequested, this, &QgsAdvancedDigitizingFloater::focusOnDistance );

  connect( cadDockWidget, &QgsAdvancedDigitizingDockWidget::enabledChangedX, this, &QgsAdvancedDigitizingFloater::enabledChangedX );
  connect( cadDockWidget, &QgsAdvancedDigitizingDockWidget::enabledChangedY, this, &QgsAdvancedDigitizingFloater::enabledChangedY );
  connect( cadDockWidget, &QgsAdvancedDigitizingDockWidget::enabledChangedZ, this, &QgsAdvancedDigitizingFloater::enabledChangedZ );
  connect( cadDockWidget, &QgsAdvancedDigitizingDockWidget::enabledChangedM, this, &QgsAdvancedDigitizingFloater::enabledChangedM );
  connect( cadDockWidget, &QgsAdvancedDigitizingDockWidget::enabledChangedAngle, this, &QgsAdvancedDigitizingFloater::enabledChangedAngle );
  // Bearing capability is the same an angle, connect to the same signal:
  connect( cadDockWidget, &QgsAdvancedDigitizingDockWidget::enabledChangedAngle, this, &QgsAdvancedDigitizingFloater::enabledChangedBearing );
  // Common angle snapping capbility is also linked to angle, connect to the same signal:
  connect( cadDockWidget, &QgsAdvancedDigitizingDockWidget::enabledChangedAngle, this, &QgsAdvancedDigitizingFloater::enabledCommonAngleSnapping );
  connect( cadDockWidget, &QgsAdvancedDigitizingDockWidget::enabledChangedDistance, this, &QgsAdvancedDigitizingFloater::enabledChangedDistance );

  // Connect our line edits signals to update cadDockWidget's state (implementation copied from QgsAdvancedDigitizingDockWidget)
  connect( mXLineEdit, &QLineEdit::returnPressed, cadDockWidget, [=]() { cadDockWidget->setX( mXLineEdit->text(), QgsAdvancedDigitizingDockWidget::WidgetSetMode::ReturnPressed ); } );
  connect( mYLineEdit, &QLineEdit::returnPressed, cadDockWidget, [=]() { cadDockWidget->setY( mYLineEdit->text(), QgsAdvancedDigitizingDockWidget::WidgetSetMode::ReturnPressed ); } );
  connect( mZLineEdit, &QLineEdit::returnPressed, cadDockWidget, [=]() { cadDockWidget->setZ( mZLineEdit->text(), QgsAdvancedDigitizingDockWidget::WidgetSetMode::ReturnPressed ); } );
  connect( mMLineEdit, &QLineEdit::returnPressed, cadDockWidget, [=]() { cadDockWidget->setM( mMLineEdit->text(), QgsAdvancedDigitizingDockWidget::WidgetSetMode::ReturnPressed ); } );
  connect( mAngleLineEdit, &QLineEdit::returnPressed, cadDockWidget, [=]() {
    cadDockWidget->setAngle( mAngleLineEdit->text(), QgsAdvancedDigitizingDockWidget::WidgetSetMode::ReturnPressed );
    const QString cleanedInputValue { QgsAdvancedDigitizingDockWidget::CadConstraint::removeSuffix( mAngleLineEdit->text(), Qgis::CadConstraintType::Angle ) };
    whileBlocking( mAngleLineEdit )->setText( cleanedInputValue );
  } );
  connect( mDistanceLineEdit, &QLineEdit::returnPressed, cadDockWidget, [=]() {
    cadDockWidget->setDistance( mDistanceLineEdit->text(), QgsAdvancedDigitizingDockWidget::WidgetSetMode::ReturnPressed );
    const QString cleanedInputValue { QgsAdvancedDigitizingDockWidget::CadConstraint::removeSuffix( mDistanceLineEdit->text(), Qgis::CadConstraintType::Distance ) };
    whileBlocking( mDistanceLineEdit )->setText( cleanedInputValue );
  } );

  connect( mXLineEdit, &QLineEdit::textEdited, cadDockWidget, [=]() { cadDockWidget->setX( mXLineEdit->text(), QgsAdvancedDigitizingDockWidget::WidgetSetMode::TextEdited ); } );
  connect( mYLineEdit, &QLineEdit::textEdited, cadDockWidget, [=]() { cadDockWidget->setY( mYLineEdit->text(), QgsAdvancedDigitizingDockWidget::WidgetSetMode::TextEdited ); } );
  connect( mZLineEdit, &QLineEdit::textEdited, cadDockWidget, [=]() { cadDockWidget->setZ( mZLineEdit->text(), QgsAdvancedDigitizingDockWidget::WidgetSetMode::TextEdited ); } );
  connect( mMLineEdit, &QLineEdit::textEdited, cadDockWidget, [=]() { cadDockWidget->setM( mMLineEdit->text(), QgsAdvancedDigitizingDockWidget::WidgetSetMode::TextEdited ); } );
  connect( mAngleLineEdit, &QLineEdit::textEdited, cadDockWidget, [=]() { cadDockWidget->setAngle( mAngleLineEdit->text(), QgsAdvancedDigitizingDockWidget::WidgetSetMode::TextEdited ); } );
  connect( mDistanceLineEdit, &QLineEdit::textEdited, cadDockWidget, [=]() { cadDockWidget->setDistance( mDistanceLineEdit->text(), QgsAdvancedDigitizingDockWidget::WidgetSetMode::TextEdited ); } );

  QgsFocusWatcher *xWatcher = new QgsFocusWatcher( mXLineEdit );
  connect( xWatcher, &QgsFocusWatcher::focusOut, cadDockWidget, [=]() { cadDockWidget->setX( mXLineEdit->text(), QgsAdvancedDigitizingDockWidget::WidgetSetMode::FocusOut ); } );
  QgsFocusWatcher *yWatcher = new QgsFocusWatcher( mYLineEdit );
  connect( yWatcher, &QgsFocusWatcher::focusOut, cadDockWidget, [=]() { cadDockWidget->setY( mYLineEdit->text(), QgsAdvancedDigitizingDockWidget::WidgetSetMode::FocusOut ); } );
  QgsFocusWatcher *zWatcher = new QgsFocusWatcher( mZLineEdit );
  connect( zWatcher, &QgsFocusWatcher::focusOut, cadDockWidget, [=]() { cadDockWidget->setZ( mZLineEdit->text(), QgsAdvancedDigitizingDockWidget::WidgetSetMode::FocusOut ); } );
  QgsFocusWatcher *mWatcher = new QgsFocusWatcher( mYLineEdit );
  connect( mWatcher, &QgsFocusWatcher::focusOut, cadDockWidget, [=]() { cadDockWidget->setM( mMLineEdit->text(), QgsAdvancedDigitizingDockWidget::WidgetSetMode::FocusOut ); } );
  QgsFocusWatcher *angleWatcher = new QgsFocusWatcher( mAngleLineEdit );
  connect( angleWatcher, &QgsFocusWatcher::focusOut, cadDockWidget, [=]() {
    cadDockWidget->setAngle( mAngleLineEdit->text(), QgsAdvancedDigitizingDockWidget::WidgetSetMode::FocusOut );
    whileBlocking( mAngleLineEdit )->setText( mCadDockWidget->constraintAngle()->displayValue() );
  } );
  connect( angleWatcher, &QgsFocusWatcher::focusIn, this, [=]() {
    const QString cleanedInputValue { QgsAdvancedDigitizingDockWidget::CadConstraint::removeSuffix( mAngleLineEdit->text(), Qgis::CadConstraintType::Angle ) };
    whileBlocking( mAngleLineEdit )->setText( cleanedInputValue );
  } );
  QgsFocusWatcher *distanceWatcher = new QgsFocusWatcher( mDistanceLineEdit );
  connect( distanceWatcher, &QgsFocusWatcher::focusOut, cadDockWidget, [=]() {
    cadDockWidget->setDistance( mDistanceLineEdit->text(), QgsAdvancedDigitizingDockWidget::WidgetSetMode::FocusOut );
    whileBlocking( mDistanceLineEdit )->setText( mCadDockWidget->constraintDistance()->displayValue() );
  } );
  connect( distanceWatcher, &QgsFocusWatcher::focusIn, this, [=]() {
    const QString cleanedInputValue { QgsAdvancedDigitizingDockWidget::CadConstraint::removeSuffix( mDistanceLineEdit->text(), Qgis::CadConstraintType::Distance ) };
    whileBlocking( mDistanceLineEdit )->setText( cleanedInputValue );
  } );
  changeCommonAngleSnapping( mCadDockWidget->commonAngleConstraint() );
}

bool QgsAdvancedDigitizingFloater::eventFilter( QObject *obj, QEvent *event )
{
  if ( mCadDockWidget && mCadDockWidget->cadEnabled() && mActive )
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
      QEnterEvent *enterEvent = dynamic_cast<QEnterEvent *>( event );
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

bool QgsAdvancedDigitizingFloater::itemVisibility( const FloaterItem &item ) const
{
  return mItemsVisibility.testFlag( item );
}

void QgsAdvancedDigitizingFloater::setActive( bool active )
{
  QgsSettings().setValue( QStringLiteral( "/Cad/Floater" ), active );

  mActive = active;

  hideIfDisabled();
}

void QgsAdvancedDigitizingFloater::setItemVisibility( const QgsAdvancedDigitizingFloater::FloaterItem &item, bool visible )
{
  const QMetaEnum enumData { QMetaEnum::fromType<QgsAdvancedDigitizingFloater::FloaterItem>() };
  QgsSettings().setValue( QStringLiteral( "/Cad/%1ShowInFloater" ).arg( enumData.valueToKey( static_cast<int>( item ) ) ), visible );
  mItemsVisibility.setFlag( item, visible );
  switch ( item )
  {
    case FloaterItem::XCoordinate:
      enabledChangedX( visible );
      break;
    case FloaterItem::YCoordinate:
      enabledChangedY( visible );
      break;
    case FloaterItem::MCoordinate:
      enabledChangedM( visible );
      break;
    case FloaterItem::ZCoordinate:
      enabledChangedZ( visible );
      break;
    case FloaterItem::Angle:
      enabledChangedAngle( visible );
      break;
    case FloaterItem::Distance:
      enabledChangedDistance( visible );
      break;
    case FloaterItem::CommonAngleSnapping:
      enabledCommonAngleSnapping( visible );
      break;
    case FloaterItem::Bearing:
      enabledChangedBearing( visible );
      break;
  }
}

void QgsAdvancedDigitizingFloater::updatePos( const QPoint &pos )
{
  // We hardcode a small delta between the mouse position and the widget's position
  move( pos + QPoint( 15, 5 ) );
}

void QgsAdvancedDigitizingFloater::hideIfDisabled()
{
  if ( !mCadDockWidget || !mCadDockWidget->cadEnabled() || !mActive )
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

void QgsAdvancedDigitizingFloater::changeZ( const QString &text )
{
  mZLineEdit->setText( text );
}

void QgsAdvancedDigitizingFloater::changeM( const QString &text )
{
  mMLineEdit->setText( text );
}

void QgsAdvancedDigitizingFloater::changeCommonAngleSnapping( double angle )
{
  mCommonAngleSnappingLineEdit->setText( qgsDoubleNear( angle, 0.0 ) ? tr( "disabled" ) : QLocale().toString( angle ).append( tr( " °" ) ) );
}

void QgsAdvancedDigitizingFloater::changeDistance( const QString &text )
{
  mDistanceLineEdit->setText( text );
}

void QgsAdvancedDigitizingFloater::changeAngle( const QString &text )
{
  mAngleLineEdit->setText( text );
}

void QgsAdvancedDigitizingFloater::changeBearing( const QString &text )
{
  mBearingLineEdit->setText( text );
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

void QgsAdvancedDigitizingFloater::changeLockZ( bool locked )
{
  if ( !locked )
  {
    mZLineEdit->setStyleSheet( QString() );
    mZLabel->setStyleSheet( QString() );
  }
  else
  {
    mZLineEdit->setStyleSheet( QStringLiteral( "font-weight: bold" ) );
    mZLabel->setStyleSheet( QStringLiteral( "font-weight: bold" ) );
  }
}

void QgsAdvancedDigitizingFloater::changeLockM( bool locked )
{
  if ( !locked )
  {
    mMLineEdit->setStyleSheet( QString() );
    mMLabel->setStyleSheet( QString() );
  }
  else
  {
    mMLineEdit->setStyleSheet( QStringLiteral( "font-weight: bold" ) );
    mMLabel->setStyleSheet( QStringLiteral( "font-weight: bold" ) );
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

void QgsAdvancedDigitizingFloater::changeRelativeZ( bool relative )
{
  if ( !relative )
  {
    mZLabel->setText( "z" );
  }
  else
  {
    mZLabel->setText( "Δz" );
  }
  adjustSize();
}

void QgsAdvancedDigitizingFloater::changeRelativeM( bool relative )
{
  if ( !relative )
  {
    mMLabel->setText( "m" );
  }
  else
  {
    mMLabel->setText( "Δm" );
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

void QgsAdvancedDigitizingFloater::focusOnZ()
{
  if ( mActive )
  {
    mZLineEdit->setFocus();
    mZLineEdit->selectAll();
  }
}

void QgsAdvancedDigitizingFloater::focusOnM()
{
  if ( mActive )
  {
    mMLineEdit->setFocus();
    mMLineEdit->selectAll();
  }
}

void QgsAdvancedDigitizingFloater::focusOnDistance()
{
  if ( mActive )
  {
    const QString cleanedInputValue { QgsAdvancedDigitizingDockWidget::CadConstraint::removeSuffix( mDistanceLineEdit->text(), Qgis::CadConstraintType::Distance ) };
    mDistanceLineEdit->setText( cleanedInputValue );
    mDistanceLineEdit->setFocus();
    mDistanceLineEdit->selectAll();
  }
}

void QgsAdvancedDigitizingFloater::focusOnAngle()
{
  if ( mActive )
  {
    const QString cleanedInputValue { QgsAdvancedDigitizingDockWidget::CadConstraint::removeSuffix( mAngleLineEdit->text(), Qgis::CadConstraintType::Angle ) };
    mAngleLineEdit->setText( cleanedInputValue );
    mAngleLineEdit->setFocus();
    mAngleLineEdit->selectAll();
  }
}


void QgsAdvancedDigitizingFloater::enabledChangedX( bool enabled )
{
  mXLineEdit->setVisible( enabled && itemVisibility( FloaterItem::XCoordinate ) );
  mXLabel->setVisible( enabled && itemVisibility( FloaterItem::XCoordinate ) );
  adjustSize();
}

void QgsAdvancedDigitizingFloater::enabledChangedY( bool enabled )
{
  mYLineEdit->setVisible( enabled && itemVisibility( FloaterItem::YCoordinate ) );
  mYLabel->setVisible( enabled && itemVisibility( FloaterItem::YCoordinate ) );
  adjustSize();
}

void QgsAdvancedDigitizingFloater::enabledChangedZ( bool enabled )
{
  mZLineEdit->setVisible( enabled && itemVisibility( FloaterItem::ZCoordinate ) );
  mZLabel->setVisible( enabled && itemVisibility( FloaterItem::ZCoordinate ) );
  adjustSize();
}

void QgsAdvancedDigitizingFloater::enabledChangedM( bool enabled )
{
  mMLineEdit->setVisible( enabled && itemVisibility( FloaterItem::MCoordinate ) );
  mMLabel->setVisible( enabled && itemVisibility( FloaterItem::MCoordinate ) );
  adjustSize();
}

void QgsAdvancedDigitizingFloater::enabledChangedDistance( bool enabled )
{
  mDistanceLineEdit->setVisible( enabled && itemVisibility( FloaterItem::Distance ) );
  mDistanceLabel->setVisible( enabled && itemVisibility( FloaterItem::Distance ) );
  adjustSize();
}

void QgsAdvancedDigitizingFloater::enabledCommonAngleSnapping( bool enabled )
{
  mCommonAngleSnappingLineEdit->setVisible( enabled && itemVisibility( FloaterItem::CommonAngleSnapping ) );
  mCommonAngleSnappingLabel->setVisible( enabled && itemVisibility( FloaterItem::CommonAngleSnapping ) );
  adjustSize();
}

void QgsAdvancedDigitizingFloater::enabledChangedAngle( bool enabled )
{
  mAngleLineEdit->setVisible( enabled && itemVisibility( FloaterItem::Angle ) );
  mAngleLabel->setVisible( enabled && itemVisibility( FloaterItem::Angle ) );
  adjustSize();
}

void QgsAdvancedDigitizingFloater::enabledChangedBearing( bool enabled )
{
  mBearingLineEdit->setVisible( enabled && itemVisibility( FloaterItem::Bearing ) );
  mBearingLabel->setVisible( enabled && itemVisibility( FloaterItem::Bearing ) );
  adjustSize();
}
