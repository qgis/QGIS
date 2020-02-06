/***************************************************************************
    qgsadvanceddigitizingdockwidget.cpp  -  dock for CAD tools
    ----------------------
    begin                : October 2014
    copyright            : (C) Denis Rouzaud
    email                : denis.rouzaud@gmail.com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <QMenu>
#include <QEvent>
#include <QCoreApplication>

#include <cmath>

#include "qgsadvanceddigitizingdockwidget.h"
#include "qgsadvanceddigitizingfloater.h"
#include "qgsadvanceddigitizingcanvasitem.h"
#include "qgsapplication.h"
#include "qgscadutils.h"
#include "qgsexpression.h"
#include "qgslogger.h"
#include "qgsmapcanvas.h"
#include "qgsmaptoolcapture.h"
#include "qgsmaptooladvanceddigitizing.h"
#include "qgsmessagebaritem.h"
#include "qgslinestring.h"
#include "qgsfocuswatcher.h"
#include "qgssettings.h"
#include "qgssnappingutils.h"
#include "qgsproject.h"
#include "qgsmapmouseevent.h"
#include "qgsmessagelog.h"


QgsAdvancedDigitizingDockWidget::QgsAdvancedDigitizingDockWidget( QgsMapCanvas *canvas, QWidget *parent )
  : QgsDockWidget( parent )
  , mMapCanvas( canvas )
  , mSnapIndicator( qgis::make_unique< QgsSnapIndicator>( canvas ) )
  , mCommonAngleConstraint( QgsSettings().value( QStringLiteral( "/Cad/CommonAngle" ), 90 ).toDouble() )
{
  setupUi( this );

  mCadPaintItem = new QgsAdvancedDigitizingCanvasItem( canvas, this );

  mAngleConstraint.reset( new CadConstraint( mAngleLineEdit, mLockAngleButton, mRelativeAngleButton, mRepeatingLockAngleButton ) );
  mDistanceConstraint.reset( new CadConstraint( mDistanceLineEdit, mLockDistanceButton, nullptr, mRepeatingLockDistanceButton ) );
  mXConstraint.reset( new CadConstraint( mXLineEdit, mLockXButton, mRelativeXButton, mRepeatingLockXButton ) );
  mYConstraint.reset( new CadConstraint( mYLineEdit, mLockYButton, mRelativeYButton, mRepeatingLockYButton ) );
  mAdditionalConstraint = AdditionalConstraint::NoConstraint;

  mMapCanvas->installEventFilter( this );
  mAngleLineEdit->installEventFilter( this );
  mDistanceLineEdit->installEventFilter( this );
  mXLineEdit->installEventFilter( this );
  mYLineEdit->installEventFilter( this );

  // Connect the UI to the event filter to update constraints
  connect( mEnableAction, &QAction::triggered, this, &QgsAdvancedDigitizingDockWidget::activateCad );
  connect( mConstructionModeAction, &QAction::triggered, this, &QgsAdvancedDigitizingDockWidget::setConstructionMode );
  connect( mParallelAction, &QAction::triggered, this, &QgsAdvancedDigitizingDockWidget::additionalConstraintClicked );
  connect( mPerpendicularAction, &QAction::triggered, this, &QgsAdvancedDigitizingDockWidget::additionalConstraintClicked );
  connect( mLockAngleButton, &QAbstractButton::clicked, this, &QgsAdvancedDigitizingDockWidget::lockConstraint );
  connect( mLockDistanceButton, &QAbstractButton::clicked, this, &QgsAdvancedDigitizingDockWidget::lockConstraint );
  connect( mLockXButton, &QAbstractButton::clicked, this, &QgsAdvancedDigitizingDockWidget::lockConstraint );
  connect( mLockYButton, &QAbstractButton::clicked, this, &QgsAdvancedDigitizingDockWidget::lockConstraint );
  connect( mRelativeAngleButton, &QAbstractButton::clicked, this, &QgsAdvancedDigitizingDockWidget::setConstraintRelative );
  connect( mRelativeXButton, &QAbstractButton::clicked, this, &QgsAdvancedDigitizingDockWidget::setConstraintRelative );
  connect( mRelativeYButton, &QAbstractButton::clicked, this, &QgsAdvancedDigitizingDockWidget::setConstraintRelative );
  connect( mRepeatingLockDistanceButton, &QAbstractButton::clicked, this, &QgsAdvancedDigitizingDockWidget::setConstraintRepeatingLock );
  connect( mRepeatingLockAngleButton, &QAbstractButton::clicked, this, &QgsAdvancedDigitizingDockWidget::setConstraintRepeatingLock );
  connect( mRepeatingLockXButton, &QAbstractButton::clicked, this, &QgsAdvancedDigitizingDockWidget::setConstraintRepeatingLock );
  connect( mRepeatingLockYButton, &QAbstractButton::clicked, this, &QgsAdvancedDigitizingDockWidget::setConstraintRepeatingLock );
  connect( mAngleLineEdit, &QLineEdit::returnPressed, this, [ = ]() { lockConstraint(); } );
  connect( mDistanceLineEdit, &QLineEdit::returnPressed, this, [ = ]() { lockConstraint(); } );
  connect( mXLineEdit, &QLineEdit::returnPressed, this, [ = ]() { lockConstraint(); } );
  connect( mYLineEdit, &QLineEdit::returnPressed, this, [ = ]() { lockConstraint(); } );
  connect( mAngleLineEdit, &QLineEdit::textEdited, this, &QgsAdvancedDigitizingDockWidget::constraintTextEdited );
  connect( mDistanceLineEdit, &QLineEdit::textEdited, this, &QgsAdvancedDigitizingDockWidget::constraintTextEdited );
  connect( mXLineEdit, &QLineEdit::textEdited, this, &QgsAdvancedDigitizingDockWidget::constraintTextEdited );
  connect( mYLineEdit, &QLineEdit::textEdited, this, &QgsAdvancedDigitizingDockWidget::constraintTextEdited );
  //also watch for focus out events on these widgets
  QgsFocusWatcher *angleWatcher = new QgsFocusWatcher( mAngleLineEdit );
  connect( angleWatcher, &QgsFocusWatcher::focusOut, this, &QgsAdvancedDigitizingDockWidget::constraintFocusOut );
  QgsFocusWatcher *distanceWatcher = new QgsFocusWatcher( mDistanceLineEdit );
  connect( distanceWatcher, &QgsFocusWatcher::focusOut, this, &QgsAdvancedDigitizingDockWidget::constraintFocusOut );
  QgsFocusWatcher *xWatcher = new QgsFocusWatcher( mXLineEdit );
  connect( xWatcher, &QgsFocusWatcher::focusOut, this, &QgsAdvancedDigitizingDockWidget::constraintFocusOut );
  QgsFocusWatcher *yWatcher = new QgsFocusWatcher( mYLineEdit );
  connect( yWatcher, &QgsFocusWatcher::focusOut, this, &QgsAdvancedDigitizingDockWidget::constraintFocusOut );

  // config menu
  QMenu *menu = new QMenu( this );
  // common angles
  QActionGroup *angleButtonGroup = new QActionGroup( menu ); // actions are exclusive for common angles
  mCommonAngleActions = QMap<QAction *, double>();
  QList< QPair< double, QString > > commonAngles;
  QString menuText;
  QList<double> anglesDouble( { 0.0, 5.0, 10.0, 15.0, 18.0, 22.5, 30.0, 45.0, 90.0} );
  for ( QList<double>::const_iterator it = anglesDouble.constBegin(); it != anglesDouble.constEnd(); ++it )
  {
    if ( *it == 0 )
      menuText = tr( "Do Not Snap to Common Angles" );
    else
      menuText = QString( tr( "%1, %2, %3, %4°…" ) ).arg( *it, 0, 'f', 1 ).arg( *it * 2, 0, 'f', 1 ).arg( *it * 3, 0, 'f', 1 ).arg( *it * 4, 0, 'f', 1 );
    commonAngles << QPair<double, QString>( *it, menuText );
  }
  for ( QList< QPair<double, QString > >::const_iterator it = commonAngles.constBegin(); it != commonAngles.constEnd(); ++it )
  {
    QAction *action = new QAction( it->second, menu );
    action->setCheckable( true );
    action->setChecked( it->first == mCommonAngleConstraint );
    menu->addAction( action );
    angleButtonGroup->addAction( action );
    mCommonAngleActions.insert( action, it->first );
  }

  qobject_cast< QToolButton *>( mToolbar->widgetForAction( mSettingsAction ) )->setPopupMode( QToolButton::InstantPopup );
  mSettingsAction->setMenu( menu );
  connect( menu, &QMenu::triggered, this, &QgsAdvancedDigitizingDockWidget::settingsButtonTriggered );

  // set tooltips
  mConstructionModeAction->setToolTip( "<b>" + tr( "Construction mode" ) + "</b><br>(" + tr( "press c to toggle on/off" ) + ")" );
  mDistanceLineEdit->setToolTip( "<b>" + tr( "Distance" ) + "</b><br>(" + tr( "press d for quick access" ) + ")" );
  mLockDistanceButton->setToolTip( "<b>" + tr( "Lock distance" ) + "</b><br>(" + tr( "press Ctrl + d for quick access" ) + ")" );
  mRepeatingLockDistanceButton->setToolTip( "<b>" + tr( "Continuously lock distance" ) + "</b>" );

  mRelativeAngleButton->setToolTip( "<b>" + tr( "Toggles relative angle to previous segment" ) + "</b><br>(" + tr( "press Shift + a for quick access" ) + ")" );
  mAngleLineEdit->setToolTip( "<b>" + tr( "Angle" ) + "</b><br>(" + tr( "press a for quick access" ) + ")" );
  mLockAngleButton->setToolTip( "<b>" + tr( "Lock angle" ) + "</b><br>(" + tr( "press Ctrl + a for quick access" ) + ")" );
  mRepeatingLockAngleButton->setToolTip( "<b>" + tr( "Continuously lock angle" ) + "</b>" );

  mRelativeXButton->setToolTip( "<b>" + tr( "Toggles relative x to previous node" ) + "</b><br>(" + tr( "press Shift + x for quick access" ) + ")" );
  mXLineEdit->setToolTip( "<b>" + tr( "X coordinate" ) + "</b><br>(" + tr( "press x for quick access" ) + ")" );
  mLockXButton->setToolTip( "<b>" + tr( "Lock x coordinate" ) + "</b><br>(" + tr( "press Ctrl + x for quick access" ) + ")" );
  mRepeatingLockXButton->setToolTip( "<b>" + tr( "Continuously lock x coordinate" ) + "</b>" );

  mRelativeYButton->setToolTip( "<b>" + tr( "Toggles relative y to previous node" ) + "</b><br>(" + tr( "press Shift + y for quick access" ) + ")" );
  mYLineEdit->setToolTip( "<b>" + tr( "Y coordinate" ) + "</b><br>(" + tr( "press y for quick access" ) + ")" );
  mLockYButton->setToolTip( "<b>" + tr( "Lock y coordinate" ) + "</b><br>(" + tr( "press Ctrl + y for quick access" ) + ")" );
  mRepeatingLockYButton->setToolTip( "<b>" + tr( "Continuously lock y coordinate" ) + "</b>" );

  // Create the slots/signals
  connect( mXLineEdit, &QLineEdit::textChanged, this, &QgsAdvancedDigitizingDockWidget::valueXChanged );
  connect( mYLineEdit, &QLineEdit::textChanged, this, &QgsAdvancedDigitizingDockWidget::valueYChanged );
  connect( mDistanceLineEdit, &QLineEdit::textChanged, this, &QgsAdvancedDigitizingDockWidget::valueDistanceChanged );
  connect( mAngleLineEdit, &QLineEdit::textChanged, this, &QgsAdvancedDigitizingDockWidget::valueAngleChanged );

  // Create the floater
  mFloater = new QgsAdvancedDigitizingFloater( canvas, this );
  connect( mToggleFloaterAction, &QAction::triggered, mFloater, &QgsAdvancedDigitizingFloater::setActive );
  mToggleFloaterAction->setChecked( mFloater->active() );

  updateCapacity( true );
  connect( QgsProject::instance(), &QgsProject::snappingConfigChanged, this, [ = ] { updateCapacity( true ); } );

  disable();
}

void QgsAdvancedDigitizingDockWidget::setX( const QString &value, WidgetSetMode mode )
{
  mXLineEdit->setText( value );
  if ( mode == WidgetSetMode::ReturnPressed )
  {
    mXLineEdit->returnPressed();
  }
  else if ( mode == WidgetSetMode::FocusOut )
  {
    QEvent *e = new QEvent( QEvent::FocusOut );
    QCoreApplication::postEvent( mXLineEdit, e );
  }
  else if ( mode == WidgetSetMode::TextEdited )
  {
    mXLineEdit->textEdited( value );
  }
}
void QgsAdvancedDigitizingDockWidget::setY( const QString &value, WidgetSetMode mode )
{
  mYLineEdit->setText( value );
  if ( mode == WidgetSetMode::ReturnPressed )
  {
    mYLineEdit->returnPressed();
  }
  else if ( mode == WidgetSetMode::FocusOut )
  {
    QEvent *e = new QEvent( QEvent::FocusOut );
    QCoreApplication::postEvent( mYLineEdit, e );
  }
  else if ( mode == WidgetSetMode::TextEdited )
  {
    mYLineEdit->textEdited( value );
  }
}
void QgsAdvancedDigitizingDockWidget::setAngle( const QString &value, WidgetSetMode mode )
{
  mAngleLineEdit->setText( value );
  if ( mode == WidgetSetMode::ReturnPressed )
  {
    mAngleLineEdit->returnPressed();
  }
  else if ( mode == WidgetSetMode::FocusOut )
  {
    QEvent *e = new QEvent( QEvent::FocusOut );
    QCoreApplication::postEvent( mAngleLineEdit, e );
  }
  else if ( mode == WidgetSetMode::TextEdited )
  {
    mAngleLineEdit->textEdited( value );
  }
}
void QgsAdvancedDigitizingDockWidget::setDistance( const QString &value, WidgetSetMode mode )
{
  mDistanceLineEdit->setText( value );
  if ( mode == WidgetSetMode::ReturnPressed )
  {
    mDistanceLineEdit->returnPressed();
  }
  else if ( mode == WidgetSetMode::FocusOut )
  {
    QEvent *e = new QEvent( QEvent::FocusOut );
    QCoreApplication::postEvent( mDistanceLineEdit, e );
  }
  else if ( mode == WidgetSetMode::TextEdited )
  {
    mDistanceLineEdit->textEdited( value );
  }
}


void QgsAdvancedDigitizingDockWidget::setCadEnabled( bool enabled )
{
  mCadEnabled = enabled;
  mEnableAction->setChecked( enabled );
  mConstructionModeAction->setEnabled( enabled );
  mParallelAction->setEnabled( enabled );
  mPerpendicularAction->setEnabled( enabled );
  mSettingsAction->setEnabled( enabled );
  mInputWidgets->setEnabled( enabled );
  mToggleFloaterAction->setEnabled( enabled );

  clear();
  setConstructionMode( false );

  emit cadEnabledChanged( enabled );
}

void QgsAdvancedDigitizingDockWidget::activateCad( bool enabled )
{
  enabled &= mCurrentMapToolSupportsCad;

  mSessionActive = enabled;

  if ( enabled && !isVisible() )
  {
    show();
  }

  setCadEnabled( enabled );
}

void QgsAdvancedDigitizingDockWidget::additionalConstraintClicked( bool activated )
{
  if ( !activated )
  {
    lockAdditionalConstraint( AdditionalConstraint::NoConstraint );
  }
  if ( sender() == mParallelAction )
  {
    lockAdditionalConstraint( AdditionalConstraint::Parallel );
  }
  else if ( sender() == mPerpendicularAction )
  {
    lockAdditionalConstraint( AdditionalConstraint::Perpendicular );
  }
}

void QgsAdvancedDigitizingDockWidget::setConstraintRelative( bool activate )
{
  if ( sender() == mRelativeAngleButton )
  {
    mAngleConstraint->setRelative( activate );
    emit relativeAngleChanged( activate );
  }
  else if ( sender() == mRelativeXButton )
  {
    mXConstraint->setRelative( activate );
    emit relativeXChanged( activate );
  }
  else if ( sender() == mRelativeYButton )
  {
    mYConstraint->setRelative( activate );
    emit relativeYChanged( activate );
  }
}

void QgsAdvancedDigitizingDockWidget::setConstraintRepeatingLock( bool activate )
{
  if ( sender() == mRepeatingLockDistanceButton )
  {
    mDistanceConstraint->setRepeatingLock( activate );
  }
  else if ( sender() == mRepeatingLockAngleButton )
  {
    mAngleConstraint->setRepeatingLock( activate );
  }
  else if ( sender() == mRepeatingLockXButton )
  {
    mXConstraint->setRepeatingLock( activate );
  }
  else if ( sender() == mRepeatingLockYButton )
  {
    mYConstraint->setRepeatingLock( activate );
  }
}

void QgsAdvancedDigitizingDockWidget::setConstructionMode( bool enabled )
{
  mConstructionMode = enabled;
  mConstructionModeAction->setChecked( enabled );
}

void QgsAdvancedDigitizingDockWidget::settingsButtonTriggered( QAction *action )
{
  // common angles
  QMap<QAction *, double>::const_iterator ica = mCommonAngleActions.constFind( action );
  if ( ica != mCommonAngleActions.constEnd() )
  {
    ica.key()->setChecked( true );
    mCommonAngleConstraint = ica.value();
    QgsSettings().setValue( QStringLiteral( "/Cad/CommonAngle" ), ica.value() );
    return;
  }
}

void QgsAdvancedDigitizingDockWidget::releaseLocks( bool releaseRepeatingLocks )
{
  // release all locks except construction mode

  lockAdditionalConstraint( AdditionalConstraint::NoConstraint );

  if ( releaseRepeatingLocks || !mAngleConstraint->isRepeatingLock() )
  {
    mAngleConstraint->setLockMode( CadConstraint::NoLock );
    emit lockAngleChanged( false );
  }
  if ( releaseRepeatingLocks || !mDistanceConstraint->isRepeatingLock() )
  {
    mDistanceConstraint->setLockMode( CadConstraint::NoLock );
    emit lockDistanceChanged( false );
  }
  if ( releaseRepeatingLocks || !mXConstraint->isRepeatingLock() )
  {
    mXConstraint->setLockMode( CadConstraint::NoLock );
    emit lockXChanged( false );
  }
  if ( releaseRepeatingLocks || !mYConstraint->isRepeatingLock() )
  {
    mYConstraint->setLockMode( CadConstraint::NoLock );
    emit lockYChanged( false );
  }
}

#if 0
void QgsAdvancedDigitizingDockWidget::emit pointChanged()
{
  // run a fake map mouse event to update the paint item
  QPoint globalPos = mMapCanvas->cursor().pos();
  QPoint pos = mMapCanvas->mapFromGlobal( globalPos );
  QMouseEvent *e = new QMouseEvent( QEvent::MouseMove, pos, globalPos, Qt::NoButton, Qt::NoButton, Qt::NoModifier );
  mCurrentMapTool->canvasMoveEvent( e );
}
#endif

QgsAdvancedDigitizingDockWidget::CadConstraint *QgsAdvancedDigitizingDockWidget::objectToConstraint( const QObject *obj ) const
{
  CadConstraint *constraint = nullptr;
  if ( obj == mAngleLineEdit || obj == mLockAngleButton )
  {
    constraint = mAngleConstraint.get();
  }
  else if ( obj == mDistanceLineEdit || obj == mLockDistanceButton )
  {
    constraint = mDistanceConstraint.get();
  }
  else if ( obj == mXLineEdit  || obj == mLockXButton )
  {
    constraint = mXConstraint.get();
  }
  else if ( obj == mYLineEdit  || obj == mLockYButton )
  {
    constraint = mYConstraint.get();
  }
  return constraint;
}

double QgsAdvancedDigitizingDockWidget::parseUserInput( const QString &inputValue, bool &ok ) const
{
  ok = false;
  double value = qgsPermissiveToDouble( inputValue, ok );
  if ( ok )
  {
    return value;
  }
  else
  {
    // try to evaluate expression
    QgsExpression expr( inputValue );
    QVariant result = expr.evaluate();
    if ( expr.hasEvalError() )
      ok = false;
    else
      value = result.toDouble( &ok );
    return value;
  }
}

void QgsAdvancedDigitizingDockWidget::updateConstraintValue( CadConstraint *constraint, const QString &textValue, bool convertExpression )
{
  if ( !constraint || textValue.isEmpty() )
  {
    return;
  }

  if ( constraint->lockMode() == CadConstraint::NoLock )
    return;

  bool ok;
  double value = parseUserInput( textValue, ok );
  if ( !ok )
    return;

  constraint->setValue( value, convertExpression );
  // run a fake map mouse event to update the paint item
  emit pointChanged( mCadPointList.value( 0 ) );
}

void QgsAdvancedDigitizingDockWidget::lockConstraint( bool activate /* default true */ )
{
  CadConstraint *constraint = objectToConstraint( sender() );
  if ( !constraint )
  {
    return;
  }

  if ( activate )
  {
    QString textValue = constraint->lineEdit()->text();
    if ( !textValue.isEmpty() )
    {
      bool ok;
      double value = parseUserInput( textValue, ok );
      if ( ok )
      {
        constraint->setValue( value );
      }
      else
      {
        activate = false;
      }
    }
    else
    {
      activate = false;
    }
  }
  constraint->setLockMode( activate ? CadConstraint::HardLock : CadConstraint::NoLock );

  if ( constraint == mXConstraint.get() )
  {
    emit lockXChanged( activate );
  }
  else if ( constraint == mYConstraint.get() )
  {
    emit lockYChanged( activate );
  }
  else if ( constraint == mDistanceConstraint.get() )
  {
    emit lockDistanceChanged( activate );
  }
  else if ( constraint == mAngleConstraint.get() )
  {
    emit lockAngleChanged( activate );
  }

  if ( activate )
  {
    // deactivate perpendicular/parallel if angle has been activated
    if ( constraint == mAngleConstraint.get() )
    {
      lockAdditionalConstraint( AdditionalConstraint::NoConstraint );
    }

    // run a fake map mouse event to update the paint item
    emit pointChanged( mCadPointList.value( 0 ) );
  }
}

void QgsAdvancedDigitizingDockWidget::constraintTextEdited( const QString &textValue )
{
  CadConstraint *constraint = objectToConstraint( sender() );
  if ( !constraint )
  {
    return;
  }

  updateConstraintValue( constraint, textValue, false );
}

void QgsAdvancedDigitizingDockWidget::constraintFocusOut()
{
  QLineEdit *lineEdit = qobject_cast< QLineEdit * >( sender()->parent() );
  if ( !lineEdit )
    return;

  CadConstraint *constraint = objectToConstraint( lineEdit );
  if ( !constraint )
  {
    return;
  }

  updateConstraintValue( constraint, lineEdit->text(), true );
}

void QgsAdvancedDigitizingDockWidget::lockAdditionalConstraint( AdditionalConstraint constraint )
{
  mAdditionalConstraint = constraint;
  mPerpendicularAction->setChecked( constraint == AdditionalConstraint::Perpendicular );
  mParallelAction->setChecked( constraint == AdditionalConstraint::Parallel );
}

void QgsAdvancedDigitizingDockWidget::updateCapacity( bool updateUIwithoutChange )
{
  CadCapacities newCapacities = nullptr;
  // first point is the mouse point (it doesn't count)
  if ( mCadPointList.count() > 1 )
  {
    newCapacities |= AbsoluteAngle | RelativeCoordinates;
  }
  if ( mCadPointList.count() > 2 )
  {
    newCapacities |= RelativeAngle;
  }
  if ( !updateUIwithoutChange && newCapacities == mCapacities )
  {
    return;
  }

  bool snappingEnabled = QgsProject::instance()->snappingConfig().enabled();

  // update the UI according to new capacities
  // still keep the old to compare

  bool relativeAngle = mCadEnabled && newCapacities.testFlag( RelativeAngle );
  bool absoluteAngle = mCadEnabled && newCapacities.testFlag( AbsoluteAngle );
  bool relativeCoordinates = mCadEnabled && newCapacities.testFlag( RelativeCoordinates );

  mPerpendicularAction->setEnabled( absoluteAngle && snappingEnabled );
  mParallelAction->setEnabled( absoluteAngle && snappingEnabled );

  //update tooltips on buttons
  if ( !snappingEnabled )
  {
    mPerpendicularAction->setToolTip( tr( "Snapping must be enabled to utilize perpendicular mode" ) );
    mParallelAction->setToolTip( tr( "Snapping must be enabled to utilize parallel mode" ) );
  }
  else
  {
    mPerpendicularAction->setToolTip( "<b>" + tr( "Perpendicular" ) + "</b><br>(" + tr( "press p to switch between perpendicular, parallel and normal mode" ) + ")" );
    mParallelAction->setToolTip( "<b>" + tr( "Parallel" ) + "</b><br>(" + tr( "press p to switch between perpendicular, parallel and normal mode" ) + ")" );
  }


  if ( !absoluteAngle )
  {
    lockAdditionalConstraint( AdditionalConstraint::NoConstraint );
  }

  // absolute angle = azimuth, relative = from previous line
  mLockAngleButton->setEnabled( absoluteAngle );
  mRelativeAngleButton->setEnabled( relativeAngle );
  mAngleLineEdit->setEnabled( absoluteAngle );
  emit enabledChangedAngle( absoluteAngle );
  if ( !absoluteAngle )
  {
    mAngleConstraint->setLockMode( CadConstraint::NoLock );
  }
  if ( !relativeAngle )
  {
    mAngleConstraint->setRelative( false );
    emit relativeAngleChanged( false );
  }
  else if ( relativeAngle && !mCapacities.testFlag( RelativeAngle ) )
  {
    // set angle mode to relative if can do and wasn't available before
    mAngleConstraint->setRelative( true );
    emit relativeAngleChanged( true );
  }

  // distance is always relative
  mLockDistanceButton->setEnabled( relativeCoordinates );
  mDistanceLineEdit->setEnabled( relativeCoordinates );
  emit enabledChangedDistance( relativeCoordinates );
  if ( !relativeCoordinates )
  {
    mDistanceConstraint->setLockMode( CadConstraint::NoLock );
  }

  mRelativeXButton->setEnabled( relativeCoordinates );
  mRelativeYButton->setEnabled( relativeCoordinates );

  // update capacities
  mCapacities = newCapacities;
}


static QgsCadUtils::AlignMapPointConstraint _constraint( QgsAdvancedDigitizingDockWidget::CadConstraint *c )
{
  QgsCadUtils::AlignMapPointConstraint constr;
  constr.locked = c->lockMode() == QgsAdvancedDigitizingDockWidget::CadConstraint::HardLock;
  constr.relative = c->relative();
  constr.value = c->value();
  return constr;
}

bool QgsAdvancedDigitizingDockWidget::applyConstraints( QgsMapMouseEvent *e )
{
  QgsCadUtils::AlignMapPointContext context;
  context.snappingUtils = mMapCanvas->snappingUtils();
  context.mapUnitsPerPixel = mMapCanvas->mapUnitsPerPixel();
  context.xConstraint = _constraint( mXConstraint.get() );
  context.yConstraint = _constraint( mYConstraint.get() );
  context.distanceConstraint = _constraint( mDistanceConstraint.get() );
  context.angleConstraint = _constraint( mAngleConstraint.get() );
  context.cadPointList = mCadPointList;

  context.commonAngleConstraint.locked = true;
  context.commonAngleConstraint.relative = context.angleConstraint.relative;
  context.commonAngleConstraint.value = mCommonAngleConstraint;

  QgsCadUtils::AlignMapPointOutput output = QgsCadUtils::alignMapPoint( e->originalMapPoint(), context );

  bool res = output.valid;
  QgsPointXY point = output.finalMapPoint;
  mSnappedSegment.clear();
  if ( output.edgeMatch.hasEdge() )
  {
    QgsPointXY edgePt0, edgePt1;
    output.edgeMatch.edgePoints( edgePt0, edgePt1 );
    mSnappedSegment << edgePt0 << edgePt1;
  }
  if ( mAngleConstraint->lockMode() != CadConstraint::HardLock )
  {
    if ( output.softLockCommonAngle != -1 )
    {
      mAngleConstraint->setLockMode( CadConstraint::SoftLock );
      mAngleConstraint->setValue( output.softLockCommonAngle );
    }
    else
    {
      mAngleConstraint->setLockMode( CadConstraint::NoLock );
    }
  }

  // set the point coordinates in the map event
  e->setMapPoint( point );

  mSnapMatch = context.snappingUtils->snapToMap( point, nullptr, true );

  if ( mSnapMatch.isValid() )
  {
    mSnapIndicator->setMatch( mSnapMatch );
    mSnapIndicator->setVisible( true );
  }
  else
  {
    mSnapIndicator->setVisible( false );
  }

  /*
   * Constraints are applied in 2D, they are always called when using the tool
   * but they do not take into account if when you snap on a vertex it has
   * a Z value.
   * To get the value we use the snapPoint method. However, we only apply it
   * when the snapped point corresponds to the constrained point or on an edge
   * if the topological editing is activated.
   */
  if ( ( mSnapMatch.hasVertex() && ( point == mSnapMatch.point() ) ) || ( mSnapMatch.hasEdge() && QgsProject::instance()->topologicalEditing() ) )
  {
    e->snapPoint();
  }
  // update the point list
  updateCurrentPoint( point );

  updateUnlockedConstraintValues( point );

  if ( res )
  {
    emit popWarning();
  }
  else
  {
    emit pushWarning( tr( "Some constraints are incompatible. Resulting point might be incorrect." ) );
  }

  return res;
}


void QgsAdvancedDigitizingDockWidget::updateUnlockedConstraintValues( const QgsPointXY &point )
{
  bool previousPointExist, penulPointExist;
  QgsPointXY previousPt = previousPoint( &previousPointExist );
  QgsPointXY penultimatePt = penultimatePoint( &penulPointExist );

  // --- angle
  if ( !mAngleConstraint->isLocked() && previousPointExist )
  {
    double angle = 0.0;
    if ( penulPointExist && mAngleConstraint->relative() )
    {
      // previous angle
      angle = std::atan2( previousPt.y() - penultimatePt.y(),
                          previousPt.x() - penultimatePt.x() );
    }
    angle = ( std::atan2( point.y() - previousPt.y(),
                          point.x() - previousPt.x()
                        ) - angle ) * 180 / M_PI;
    // modulus
    angle = std::fmod( angle, 360.0 );
    mAngleConstraint->setValue( angle );
  }
  // --- distance
  if ( !mDistanceConstraint->isLocked() && previousPointExist )
  {
    mDistanceConstraint->setValue( std::sqrt( previousPt.sqrDist( point ) ) );
  }
  // --- X
  if ( !mXConstraint->isLocked() )
  {
    if ( previousPointExist && mXConstraint->relative() )
    {
      mXConstraint->setValue( point.x() - previousPt.x() );
    }
    else
    {
      mXConstraint->setValue( point.x() );
    }
  }
  // --- Y
  if ( !mYConstraint->isLocked() )
  {
    if ( previousPointExist && mYConstraint->relative() )
    {
      mYConstraint->setValue( point.y() - previousPt.y() );
    }
    else
    {
      mYConstraint->setValue( point.y() );
    }
  }
}


QList<QgsPointXY> QgsAdvancedDigitizingDockWidget::snapSegmentToAllLayers( const QgsPointXY &originalMapPoint, bool *snapped ) const
{
  QList<QgsPointXY> segment;
  QgsPointXY pt1, pt2;
  QgsPointLocator::Match match;

  QgsSnappingUtils *snappingUtils = mMapCanvas->snappingUtils();

  QgsSnappingConfig canvasConfig = snappingUtils->config();
  QgsSnappingConfig localConfig = snappingUtils->config();

  localConfig.setMode( QgsSnappingConfig::AllLayers );
  localConfig.setType( QgsSnappingConfig::Segment );
  snappingUtils->setConfig( localConfig );

  match = snappingUtils->snapToMap( originalMapPoint, nullptr, true );

  snappingUtils->setConfig( canvasConfig );

  if ( match.isValid() && match.hasEdge() )
  {
    match.edgePoints( pt1, pt2 );
    segment << pt1 << pt2;
  }

  if ( snapped )
  {
    *snapped = segment.count() == 2;
  }

  return segment;
}

bool QgsAdvancedDigitizingDockWidget::alignToSegment( QgsMapMouseEvent *e, CadConstraint::LockMode lockMode )
{
  if ( mAdditionalConstraint == AdditionalConstraint::NoConstraint )
  {
    return false;
  }

  bool previousPointExist, penulPointExist, snappedSegmentExist;
  QgsPointXY previousPt = previousPoint( &previousPointExist );
  QgsPointXY penultimatePt = penultimatePoint( &penulPointExist );
  mSnappedSegment = snapSegmentToAllLayers( e->originalMapPoint(), &snappedSegmentExist );

  if ( !previousPointExist || !snappedSegmentExist )
  {
    return false;
  }

  double angle = std::atan2( mSnappedSegment[0].y() - mSnappedSegment[1].y(), mSnappedSegment[0].x() - mSnappedSegment[1].x() );

  if ( mAngleConstraint->relative() && penulPointExist )
  {
    angle -= std::atan2( previousPt.y() - penultimatePt.y(), previousPt.x() - penultimatePt.x() );
  }

  if ( mAdditionalConstraint == AdditionalConstraint::Perpendicular )
  {
    angle += M_PI_2;
  }

  angle *= 180 / M_PI;

  mAngleConstraint->setValue( angle );
  mAngleConstraint->setLockMode( lockMode );
  if ( lockMode == CadConstraint::HardLock )
  {
    mAdditionalConstraint = AdditionalConstraint::NoConstraint;
  }

  return true;
}

bool QgsAdvancedDigitizingDockWidget::canvasKeyPressEventFilter( QKeyEvent *e )
{
  // event on map tool

  if ( !mCadEnabled )
    return false;

  switch ( e->key() )
  {
    case Qt::Key_Backspace:
    case Qt::Key_Delete:
    {
      removePreviousPoint();
      releaseLocks( false );
      break;
    }
    case Qt::Key_Escape:
    {
      releaseLocks();
      break;
    }
    default:
    {
      keyPressEvent( e );
      break;
    }
  }
  // for map tools, continues with key press in any case
  return false;
}

void QgsAdvancedDigitizingDockWidget::clear()
{
  clearPoints();
  releaseLocks();
}

void QgsAdvancedDigitizingDockWidget::keyPressEvent( QKeyEvent *e )
{
  // event on dock (this)

  if ( !mCadEnabled )
    return;

  switch ( e->key() )
  {
    case Qt::Key_Backspace:
    case Qt::Key_Delete:
    {
      removePreviousPoint();
      releaseLocks( false );
      break;
    }
    case Qt::Key_Escape:
    {
      releaseLocks();
      break;
    }
    default:
    {
      filterKeyPress( e );
      break;
    }
  }
}

void QgsAdvancedDigitizingDockWidget::setPoints( const QList<QgsPointXY> &points )
{
  clearPoints();
  const auto constPoints = points;
  for ( const QgsPointXY &pt : constPoints )
  {
    addPoint( pt );
  }
}

bool QgsAdvancedDigitizingDockWidget::eventFilter( QObject *obj, QEvent *event )
{
  if ( !cadEnabled() )
  {
    return QgsDockWidget::eventFilter( obj, event );
  }

  // event for line edits and map canvas
  // we have to catch both KeyPress events and ShortcutOverride events. This is because
  // the Ctrl+D and Ctrl+A shortcuts for locking distance/angle clash with the global
  // "remove layer" and "select all" shortcuts. Catching ShortcutOverride events allows
  // us to intercept these keystrokes before they are caught by the global shortcuts
  if ( event->type() == QEvent::ShortcutOverride || event->type() == QEvent::KeyPress )
  {
    if ( QKeyEvent *keyEvent = dynamic_cast<QKeyEvent *>( event ) )
    {
      return filterKeyPress( keyEvent );
    }
  }
  return QgsDockWidget::eventFilter( obj, event );
}

bool QgsAdvancedDigitizingDockWidget::filterKeyPress( QKeyEvent *e )
{
  // we need to be careful here -- because this method is called on both KeyPress events AND
  // ShortcutOverride events, we have to take care that we don't trigger the handling for BOTH
  // these event types for a single key press. I.e. pressing "A" may first call trigger a
  // ShortcutOverride event (sometimes, not always!) followed immediately by a KeyPress event.
  QEvent::Type type = e->type();
  switch ( e->key() )
  {
    case Qt::Key_X:
    {
      // modifier+x ONLY caught for ShortcutOverride events...
      if ( type == QEvent::ShortcutOverride && ( e->modifiers() == Qt::AltModifier || e->modifiers() == Qt::ControlModifier ) )
      {
        mXConstraint->toggleLocked();
        emit lockXChanged( mXConstraint->isLocked() );
        emit pointChanged( mCadPointList.value( 0 ) );
        e->accept();
      }
      else if ( type == QEvent::ShortcutOverride && e->modifiers() == Qt::ShiftModifier )
      {
        if ( mCapacities.testFlag( RelativeCoordinates ) )
        {
          mXConstraint->toggleRelative();
          emit relativeXChanged( mXConstraint->relative() );
          emit pointChanged( mCadPointList.value( 0 ) );
          e->accept();
        }
      }
      // .. but "X" alone ONLY caught for KeyPress events (see comment at start of function)
      else if ( type == QEvent::KeyPress )
      {
        mXLineEdit->setFocus();
        mXLineEdit->selectAll();
        emit focusOnXRequested();
        e->accept();
      }
      break;
    }
    case Qt::Key_Y:
    {
      // modifier+y ONLY caught for ShortcutOverride events...
      if ( type == QEvent::ShortcutOverride && ( e->modifiers() == Qt::AltModifier || e->modifiers() == Qt::ControlModifier ) )
      {
        mYConstraint->toggleLocked();
        emit lockYChanged( mYConstraint->isLocked() );
        emit pointChanged( mCadPointList.value( 0 ) );
        e->accept();
      }
      else if ( type == QEvent::ShortcutOverride &&  e->modifiers() == Qt::ShiftModifier )
      {
        if ( mCapacities.testFlag( RelativeCoordinates ) )
        {
          mYConstraint->toggleRelative();
          emit relativeYChanged( mYConstraint->relative() );
          emit pointChanged( mCadPointList.value( 0 ) );
          e->accept();
        }
      }
      // .. but "y" alone ONLY caught for KeyPress events (see comment at start of function)
      else if ( type == QEvent::KeyPress )
      {
        mYLineEdit->setFocus();
        mYLineEdit->selectAll();
        emit focusOnYRequested();
        e->accept();
      }
      break;
    }
    case Qt::Key_A:
    {
      // modifier+a ONLY caught for ShortcutOverride events...
      if ( type == QEvent::ShortcutOverride && ( e->modifiers() == Qt::AltModifier || e->modifiers() == Qt::ControlModifier ) )
      {
        if ( mCapacities.testFlag( AbsoluteAngle ) )
        {
          mAngleConstraint->toggleLocked();
          emit lockAngleChanged( mAngleConstraint->isLocked() );
          emit pointChanged( mCadPointList.value( 0 ) );
          e->accept();
        }
      }
      else if ( type == QEvent::ShortcutOverride && e->modifiers() == Qt::ShiftModifier )
      {
        if ( mCapacities.testFlag( RelativeAngle ) )
        {
          mAngleConstraint->toggleRelative();
          emit relativeAngleChanged( mAngleConstraint->relative() );
          emit pointChanged( mCadPointList.value( 0 ) );
          e->accept();
        }
      }
      // .. but "a" alone ONLY caught for KeyPress events (see comment at start of function)
      else if ( type == QEvent::KeyPress )
      {
        mAngleLineEdit->setFocus();
        mAngleLineEdit->selectAll();
        emit focusOnAngleRequested();
        e->accept();
      }
      break;
    }
    case Qt::Key_D:
    {
      // modifier+d ONLY caught for ShortcutOverride events...
      if ( type == QEvent::ShortcutOverride && ( e->modifiers() == Qt::AltModifier || e->modifiers() == Qt::ControlModifier ) )
      {
        if ( mCapacities.testFlag( RelativeCoordinates ) )
        {
          mDistanceConstraint->toggleLocked();
          emit lockDistanceChanged( mDistanceConstraint->isLocked() );
          emit pointChanged( mCadPointList.value( 0 ) );
          e->accept();
        }
      }
      // .. but "d" alone ONLY caught for KeyPress events (see comment at start of function)
      else if ( type == QEvent::KeyPress )
      {
        mDistanceLineEdit->setFocus();
        mDistanceLineEdit->selectAll();
        emit focusOnDistanceRequested();
        e->accept();
      }
      break;
    }
    case Qt::Key_C:
    {
      if ( type == QEvent::KeyPress )
      {
        setConstructionMode( !mConstructionMode );
        e->accept();
      }
      break;
    }
    case Qt::Key_P:
    {
      if ( type == QEvent::KeyPress )
      {
        bool parallel = mParallelAction->isChecked();
        bool perpendicular = mPerpendicularAction->isChecked();

        if ( !parallel && !perpendicular )
        {
          lockAdditionalConstraint( AdditionalConstraint::Perpendicular );
        }
        else if ( perpendicular )
        {
          lockAdditionalConstraint( AdditionalConstraint::Parallel );
        }
        else
        {
          lockAdditionalConstraint( AdditionalConstraint::NoConstraint );
        }
        e->accept();
      }
      break;
    }
    default:
    {
      return false; // continues
    }
  }
  return e->isAccepted();
}

void QgsAdvancedDigitizingDockWidget::enable()
{
  connect( mMapCanvas, &QgsMapCanvas::destinationCrsChanged, this, &QgsAdvancedDigitizingDockWidget::enable, Qt::UniqueConnection );
  if ( mMapCanvas->mapSettings().destinationCrs().isGeographic() )
  {
    mErrorLabel->setText( tr( "CAD tools can not be used on geographic coordinates. Change the coordinates system in the project properties." ) );
    mErrorLabel->show();
    mEnableAction->setEnabled( false );
    setCadEnabled( false );
  }
  else
  {
    mEnableAction->setEnabled( true );
    mErrorLabel->hide();
    mCadWidget->show();

    mCurrentMapToolSupportsCad = true;

    if ( mSessionActive && !isVisible() )
    {
      show();
    }
    setCadEnabled( mSessionActive );
  }
}

void QgsAdvancedDigitizingDockWidget::disable()
{
  disconnect( mMapCanvas, &QgsMapCanvas::destinationCrsChanged, this, &QgsAdvancedDigitizingDockWidget::enable );

  mEnableAction->setEnabled( false );
  mErrorLabel->setText( tr( "CAD tools are not enabled for the current map tool" ) );
  mErrorLabel->show();
  mCadWidget->hide();

  mCurrentMapToolSupportsCad = false;

  setCadEnabled( false );
}

void QgsAdvancedDigitizingDockWidget::updateCadPaintItem()
{
  mCadPaintItem->update();
}

void QgsAdvancedDigitizingDockWidget::addPoint( const QgsPointXY &point )
{
  if ( !pointsCount() )
  {
    mCadPointList << point;
  }
  else
  {
    mCadPointList.insert( 0, point );
  }

  updateCapacity();
  updateCadPaintItem();
}

void QgsAdvancedDigitizingDockWidget::removePreviousPoint()
{
  if ( !pointsCount() )
    return;

  int i = pointsCount() > 1 ? 1 : 0;
  mCadPointList.removeAt( i );
  updateCapacity();
  updateCadPaintItem();
}

void QgsAdvancedDigitizingDockWidget::clearPoints()
{
  mCadPointList.clear();
  mSnappedSegment.clear();

  updateCapacity();
  updateCadPaintItem();
}

void QgsAdvancedDigitizingDockWidget::updateCurrentPoint( const QgsPointXY &point )
{
  if ( !pointsCount() )
  {
    mCadPointList << point;
    updateCapacity();
  }
  else
  {
    mCadPointList[0] = point;
  }
  updateCadPaintItem();
}


void QgsAdvancedDigitizingDockWidget::CadConstraint::setLockMode( LockMode mode )
{
  mLockMode = mode;
  mLockerButton->setChecked( mode == HardLock );
  if ( mRepeatingLockButton )
  {
    if ( mode == HardLock )
    {
      mRepeatingLockButton->setEnabled( true );
    }
    else
    {
      mRepeatingLockButton->setChecked( false );
      mRepeatingLockButton->setEnabled( false );
    }
  }

  if ( mode == NoLock )
  {
    mLineEdit->clear();
  }

}

void QgsAdvancedDigitizingDockWidget::CadConstraint::setRepeatingLock( bool repeating )
{
  mRepeatingLock = repeating;
  if ( mRepeatingLockButton )
    mRepeatingLockButton->setChecked( repeating );
}

void QgsAdvancedDigitizingDockWidget::CadConstraint::setRelative( bool relative )
{
  mRelative = relative;
  if ( mRelativeButton )
  {
    mRelativeButton->setChecked( relative );
  }
}

void QgsAdvancedDigitizingDockWidget::CadConstraint::setValue( double value, bool updateWidget )
{
  mValue = value;
  if ( updateWidget )
    mLineEdit->setText( QLocale().toString( value, 'f', 6 ) );
}

void QgsAdvancedDigitizingDockWidget::CadConstraint::toggleLocked()
{
  setLockMode( mLockMode == HardLock ? NoLock : HardLock );
}

void QgsAdvancedDigitizingDockWidget::CadConstraint::toggleRelative()
{
  setRelative( !mRelative );
}

QgsPointXY QgsAdvancedDigitizingDockWidget::currentPoint( bool *exist ) const
{
  if ( exist )
    *exist = pointsCount() > 0;
  if ( pointsCount() > 0 )
    return mCadPointList.value( 0 );
  else
    return QgsPointXY();
}

QgsPointXY QgsAdvancedDigitizingDockWidget::previousPoint( bool *exist ) const
{
  if ( exist )
    *exist = pointsCount() > 1;
  if ( pointsCount() > 1 )
    return mCadPointList.value( 1 );
  else
    return QgsPointXY();
}

QgsPointXY QgsAdvancedDigitizingDockWidget::penultimatePoint( bool *exist ) const
{
  if ( exist )
    *exist = pointsCount() > 2;
  if ( pointsCount() > 2 )
    return mCadPointList.value( 2 );
  else
    return QgsPointXY();
}
