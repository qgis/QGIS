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
#include "qgsmaptooledit.h"
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
#include "qgsmeshlayer.h"

#include <QActionGroup>


QgsAdvancedDigitizingDockWidget::QgsAdvancedDigitizingDockWidget( QgsMapCanvas *canvas, QWidget *parent )
  : QgsDockWidget( parent )
  , mMapCanvas( canvas )
  , mSnapIndicator( std::make_unique< QgsSnapIndicator>( canvas ) )
  , mCommonAngleConstraint( QgsSettings().value( QStringLiteral( "/Cad/CommonAngle" ), 0.0 ).toDouble() )
{
  setupUi( this );

  mCadPaintItem = new QgsAdvancedDigitizingCanvasItem( canvas, this );

  mAngleConstraint.reset( new CadConstraint( mAngleLineEdit, mLockAngleButton, mRelativeAngleButton, mRepeatingLockAngleButton ) );
  mDistanceConstraint.reset( new CadConstraint( mDistanceLineEdit, mLockDistanceButton, nullptr, mRepeatingLockDistanceButton ) );
  mXConstraint.reset( new CadConstraint( mXLineEdit, mLockXButton, mRelativeXButton, mRepeatingLockXButton ) );
  mYConstraint.reset( new CadConstraint( mYLineEdit, mLockYButton, mRelativeYButton, mRepeatingLockYButton ) );
  mZConstraint.reset( new CadConstraint( mZLineEdit, mLockZButton, mRelativeZButton, mRepeatingLockZButton ) );
  mMConstraint.reset( new CadConstraint( mMLineEdit, mLockMButton, mRelativeMButton, mRepeatingLockMButton ) );
  mAdditionalConstraint = AdditionalConstraint::NoConstraint;

  mMapCanvas->installEventFilter( this );
  mAngleLineEdit->installEventFilter( this );
  mDistanceLineEdit->installEventFilter( this );
  mXLineEdit->installEventFilter( this );
  mYLineEdit->installEventFilter( this );
  mZLineEdit->installEventFilter( this );
  mMLineEdit->installEventFilter( this );

  // Connect the UI to the event filter to update constraints
  connect( mEnableAction, &QAction::triggered, this, &QgsAdvancedDigitizingDockWidget::activateCad );
  connect( mConstructionModeAction, &QAction::triggered, this, &QgsAdvancedDigitizingDockWidget::setConstructionMode );
  connect( mParallelAction, &QAction::triggered, this, &QgsAdvancedDigitizingDockWidget::additionalConstraintClicked );
  connect( mPerpendicularAction, &QAction::triggered, this, &QgsAdvancedDigitizingDockWidget::additionalConstraintClicked );
  connect( mLockAngleButton, &QAbstractButton::clicked, this, &QgsAdvancedDigitizingDockWidget::lockConstraint );
  connect( mLockDistanceButton, &QAbstractButton::clicked, this, &QgsAdvancedDigitizingDockWidget::lockConstraint );
  connect( mLockXButton, &QAbstractButton::clicked, this, &QgsAdvancedDigitizingDockWidget::lockConstraint );
  connect( mLockYButton, &QAbstractButton::clicked, this, &QgsAdvancedDigitizingDockWidget::lockConstraint );
  connect( mLockZButton, &QAbstractButton::clicked, this, &QgsAdvancedDigitizingDockWidget::lockConstraint );
  connect( mLockMButton, &QAbstractButton::clicked, this, &QgsAdvancedDigitizingDockWidget::lockConstraint );
  connect( mRelativeAngleButton, &QAbstractButton::clicked, this, &QgsAdvancedDigitizingDockWidget::setConstraintRelative );
  connect( mRelativeXButton, &QAbstractButton::clicked, this, &QgsAdvancedDigitizingDockWidget::setConstraintRelative );
  connect( mRelativeYButton, &QAbstractButton::clicked, this, &QgsAdvancedDigitizingDockWidget::setConstraintRelative );
  connect( mRelativeZButton, &QAbstractButton::clicked, this, &QgsAdvancedDigitizingDockWidget::setConstraintRelative );
  connect( mRelativeMButton, &QAbstractButton::clicked, this, &QgsAdvancedDigitizingDockWidget::setConstraintRelative );
  connect( mRepeatingLockDistanceButton, &QAbstractButton::clicked, this, &QgsAdvancedDigitizingDockWidget::setConstraintRepeatingLock );
  connect( mRepeatingLockAngleButton, &QAbstractButton::clicked, this, &QgsAdvancedDigitizingDockWidget::setConstraintRepeatingLock );
  connect( mRepeatingLockXButton, &QAbstractButton::clicked, this, &QgsAdvancedDigitizingDockWidget::setConstraintRepeatingLock );
  connect( mRepeatingLockYButton, &QAbstractButton::clicked, this, &QgsAdvancedDigitizingDockWidget::setConstraintRepeatingLock );
  connect( mRepeatingLockZButton, &QAbstractButton::clicked, this, &QgsAdvancedDigitizingDockWidget::setConstraintRepeatingLock );
  connect( mRepeatingLockMButton, &QAbstractButton::clicked, this, &QgsAdvancedDigitizingDockWidget::setConstraintRepeatingLock );
  connect( mAngleLineEdit, &QLineEdit::returnPressed, this, [ = ]() { lockConstraint(); } );
  connect( mDistanceLineEdit, &QLineEdit::returnPressed, this, [ = ]() { lockConstraint(); } );
  connect( mXLineEdit, &QLineEdit::returnPressed, this, [ = ]() { lockConstraint(); } );
  connect( mYLineEdit, &QLineEdit::returnPressed, this, [ = ]() { lockConstraint(); } );
  connect( mZLineEdit, &QLineEdit::returnPressed, this, [ = ]() { lockConstraint(); } );
  connect( mMLineEdit, &QLineEdit::returnPressed, this, [ = ]() { lockConstraint(); } );
  connect( mAngleLineEdit, &QLineEdit::textEdited, this, &QgsAdvancedDigitizingDockWidget::constraintTextEdited );
  connect( mDistanceLineEdit, &QLineEdit::textEdited, this, &QgsAdvancedDigitizingDockWidget::constraintTextEdited );
  connect( mXLineEdit, &QLineEdit::textEdited, this, &QgsAdvancedDigitizingDockWidget::constraintTextEdited );
  connect( mYLineEdit, &QLineEdit::textEdited, this, &QgsAdvancedDigitizingDockWidget::constraintTextEdited );
  connect( mZLineEdit, &QLineEdit::textEdited, this, &QgsAdvancedDigitizingDockWidget::constraintTextEdited );
  connect( mMLineEdit, &QLineEdit::textEdited, this, &QgsAdvancedDigitizingDockWidget::constraintTextEdited );
  //also watch for focus out events on these widgets
  QgsFocusWatcher *angleWatcher = new QgsFocusWatcher( mAngleLineEdit );
  connect( angleWatcher, &QgsFocusWatcher::focusOut, this, &QgsAdvancedDigitizingDockWidget::constraintFocusOut );
  QgsFocusWatcher *distanceWatcher = new QgsFocusWatcher( mDistanceLineEdit );
  connect( distanceWatcher, &QgsFocusWatcher::focusOut, this, &QgsAdvancedDigitizingDockWidget::constraintFocusOut );
  QgsFocusWatcher *xWatcher = new QgsFocusWatcher( mXLineEdit );
  connect( xWatcher, &QgsFocusWatcher::focusOut, this, &QgsAdvancedDigitizingDockWidget::constraintFocusOut );
  QgsFocusWatcher *yWatcher = new QgsFocusWatcher( mYLineEdit );
  connect( yWatcher, &QgsFocusWatcher::focusOut, this, &QgsAdvancedDigitizingDockWidget::constraintFocusOut );
  QgsFocusWatcher *zWatcher = new QgsFocusWatcher( mZLineEdit );
  connect( zWatcher, &QgsFocusWatcher::focusOut, this, &QgsAdvancedDigitizingDockWidget::constraintFocusOut );
  QgsFocusWatcher *mWatcher = new QgsFocusWatcher( mMLineEdit );
  connect( mWatcher, &QgsFocusWatcher::focusOut, this, &QgsAdvancedDigitizingDockWidget::constraintFocusOut );

  // config menu
  QMenu *menu = new QMenu( this );
  // common angles
  QActionGroup *angleButtonGroup = new QActionGroup( menu ); // actions are exclusive for common angles
  mCommonAngleActions = QMap<QAction *, double>();
  QList< QPair< double, QString > > commonAngles;
  QString menuText;
  const QList<double> anglesDouble( { 0.0, 5.0, 10.0, 15.0, 18.0, 22.5, 30.0, 45.0, 90.0} );
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
  mSettingsAction->setCheckable( true );
  mSettingsAction->setToolTip( tr( "Snap to common angles" ) );
  mSettingsAction->setChecked( mCommonAngleConstraint != 0 );
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

  mRelativeZButton->setToolTip( "<b>" + tr( "Toggles relative z to previous node" ) + "</b><br>(" + tr( "press Shift + z for quick access" ) + ")" );
  mZLineEdit->setToolTip( "<b>" + tr( "Z coordinate" ) + "</b><br>(" + tr( "press z for quick access" ) + ")" );
  mLockZButton->setToolTip( "<b>" + tr( "Lock z coordinate" ) + "</b><br>(" + tr( "press Ctrl + z for quick access" ) + ")" );
  mRepeatingLockZButton->setToolTip( "<b>" + tr( "Continuously lock z coordinate" ) + "</b>" );

  mRelativeMButton->setToolTip( "<b>" + tr( "Toggles relative m to previous node" ) + "</b><br>(" + tr( "press Shift + m for quick access" ) + ")" );
  mMLineEdit->setToolTip( "<b>" + tr( "M coordinate" ) + "</b><br>(" + tr( "press m for quick access" ) + ")" );
  mLockMButton->setToolTip( "<b>" + tr( "Lock m coordinate" ) + "</b><br>(" + tr( "press Ctrl + m for quick access" ) + ")" );
  mRepeatingLockMButton->setToolTip( "<b>" + tr( "Continuously lock m coordinate" ) + "</b>" );

  // Create the slots/signals
  connect( mXLineEdit, &QLineEdit::textChanged, this, &QgsAdvancedDigitizingDockWidget::valueXChanged );
  connect( mYLineEdit, &QLineEdit::textChanged, this, &QgsAdvancedDigitizingDockWidget::valueYChanged );
  connect( mZLineEdit, &QLineEdit::textChanged, this, &QgsAdvancedDigitizingDockWidget::valueZChanged );
  connect( mMLineEdit, &QLineEdit::textChanged, this, &QgsAdvancedDigitizingDockWidget::valueMChanged );
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
void QgsAdvancedDigitizingDockWidget::setZ( const QString &value, WidgetSetMode mode )
{
  mZLineEdit->setText( value );
  if ( mode == WidgetSetMode::ReturnPressed )
  {
    mZLineEdit->returnPressed();
  }
  else if ( mode == WidgetSetMode::FocusOut )
  {
    QEvent *e = new QEvent( QEvent::FocusOut );
    QCoreApplication::postEvent( mZLineEdit, e );
  }
  else if ( mode == WidgetSetMode::TextEdited )
  {
    mZLineEdit->textEdited( value );
  }
}
void QgsAdvancedDigitizingDockWidget::setM( const QString &value, WidgetSetMode mode )
{
  mMLineEdit->setText( value );
  if ( mode == WidgetSetMode::ReturnPressed )
  {
    mMLineEdit->returnPressed();
  }
  else if ( mode == WidgetSetMode::FocusOut )
  {
    QEvent *e = new QEvent( QEvent::FocusOut );
    QCoreApplication::postEvent( mMLineEdit, e );
  }
  else if ( mode == WidgetSetMode::TextEdited )
  {
    mMLineEdit->textEdited( value );
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

  switchZM();
  emit cadEnabledChanged( enabled );
}


void QgsAdvancedDigitizingDockWidget::switchZM( )
{
  bool enableZ = false;
  bool enableM = false;

  if ( QgsMapLayer *layer = targetLayer() )
  {
    switch ( layer->type() )
    {
      case QgsMapLayerType::VectorLayer:
      {
        QgsVectorLayer *vlayer = qobject_cast<QgsVectorLayer *>( layer );
        const QgsWkbTypes::Type type = vlayer->wkbType();
        enableZ = QgsWkbTypes::hasZ( type );
        enableM = QgsWkbTypes::hasM( type );
        break;
      }

      case QgsMapLayerType::MeshLayer:
      {
        QgsMeshLayer *mlayer = qobject_cast<QgsMeshLayer *>( layer );
        enableZ = mlayer->isEditable();
        break;
      }

      case QgsMapLayerType::RasterLayer:
      case QgsMapLayerType::PluginLayer:
      case QgsMapLayerType::VectorTileLayer:
      case QgsMapLayerType::AnnotationLayer:
      case QgsMapLayerType::PointCloudLayer:
      case QgsMapLayerType::GroupLayer:
        break;
    }
  }

  setEnabledZ( enableZ );
  setEnabledM( enableM );
}

void QgsAdvancedDigitizingDockWidget::setEnabledZ( bool enable )
{
  mRelativeZButton->setEnabled( enable );
  mZLabel->setEnabled( enable );
  mZLineEdit->setEnabled( enable );
  if ( mZLineEdit->isEnabled() )
    mZLineEdit->setText( QLocale().toString( QgsMapToolEdit::defaultZValue(), 'f', 6 ) );
  else
    mZLineEdit->clear();
  mLockZButton->setEnabled( enable );
  emit enabledChangedZ( enable );
}

void QgsAdvancedDigitizingDockWidget::setEnabledM( bool enable )
{
  mRelativeMButton->setEnabled( enable );
  mMLabel->setEnabled( enable );
  mMLineEdit->setEnabled( enable );
  if ( mMLineEdit->isEnabled() )
    mMLineEdit->setText( QLocale().toString( QgsMapToolEdit::defaultMValue(), 'f', 6 ) );
  else
    mMLineEdit->clear();
  mLockMButton->setEnabled( enable );
  emit enabledChangedM( enable );
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
  else if ( sender() == mParallelAction )
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
  else if ( sender() == mRelativeZButton )
  {
    mZConstraint->setRelative( activate );
    emit relativeZChanged( activate );
  }
  else if ( sender() == mRelativeMButton )
  {
    mMConstraint->setRelative( activate );
    emit relativeMChanged( activate );
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
  else if ( sender() == mRepeatingLockZButton )
  {
    mZConstraint->setRepeatingLock( activate );
  }
  else if ( sender() == mRepeatingLockMButton )
  {
    mMConstraint->setRepeatingLock( activate );
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
  const QMap<QAction *, double>::const_iterator ica = mCommonAngleActions.constFind( action );
  if ( ica != mCommonAngleActions.constEnd() )
  {
    ica.key()->setChecked( true );
    mCommonAngleConstraint = ica.value();
    QgsSettings().setValue( QStringLiteral( "/Cad/CommonAngle" ), ica.value() );
    mSettingsAction->setChecked( mCommonAngleConstraint != 0 );
    return;
  }
}

QgsMapLayer *QgsAdvancedDigitizingDockWidget::targetLayer() const
{
  if ( QgsMapToolAdvancedDigitizing *advancedTool = qobject_cast< QgsMapToolAdvancedDigitizing * >( mMapCanvas->mapTool() ) )
  {
    return advancedTool->layer();
  }
  else
  {
    return mMapCanvas->currentLayer();
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
  if ( releaseRepeatingLocks || !mZConstraint->isRepeatingLock() )
  {
    mZConstraint->setLockMode( CadConstraint::NoLock );
    emit lockZChanged( false );
  }
  if ( releaseRepeatingLocks || !mMConstraint->isRepeatingLock() )
  {
    mMConstraint->setLockMode( CadConstraint::NoLock );
    emit lockMChanged( false );
  }

  if ( !mCadPointList.empty() )
  {
    if ( !mXConstraint->isLocked() && !mXConstraint->relative() )
    {
      mXConstraint->setValue( mCadPointList.constLast().x(), true );
    }
    if ( !mYConstraint->isLocked() && !mYConstraint->relative() )
    {
      mYConstraint->setValue( mCadPointList.constLast().y(), true );
    }
    if ( !mZConstraint->isLocked() && !mZConstraint->relative() )
    {
      mZConstraint->setValue( mCadPointList.constLast().z(), true );
    }
    if ( !mMConstraint->isLocked() && !mMConstraint->relative() )
    {
      mMConstraint->setValue( mCadPointList.constLast().m(), true );
    }
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
  else if ( obj == mZLineEdit  || obj == mLockZButton )
  {
    constraint = mZConstraint.get();
  }
  else if ( obj == mMLineEdit  || obj == mLockMButton )
  {
    constraint = mMConstraint.get();
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
    const QVariant result = expr.evaluate();
    if ( expr.hasEvalError() )
    {
      ok = false;
      QString inputValueC { inputValue };

      // First: try removing group separator
      if ( inputValue.contains( QLocale().groupSeparator() ) )
      {
        inputValueC.remove( QLocale().groupSeparator() );
        QgsExpression exprC( inputValueC );
        const QVariant resultC = exprC.evaluate();
        if ( ! exprC.hasEvalError() )
        {
          value = resultC.toDouble( &ok );
        }
      }

      // Second: be nice with non-dot locales
      if ( !ok && QLocale().decimalPoint() != QChar( '.' ) && inputValueC.contains( QLocale().decimalPoint() ) )
      {
        QgsExpression exprC( inputValueC .replace( QLocale().decimalPoint(), QChar( '.' ) ) );
        const QVariant resultC = exprC.evaluate();
        if ( ! exprC.hasEvalError() )
        {
          value = resultC.toDouble( &ok );
        }
      }
    }
    else
    {
      value = result.toDouble( &ok );
    }
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
  const double value = parseUserInput( textValue, ok );
  if ( !ok )
    return;

  constraint->setValue( value, convertExpression );
  // run a fake map mouse event to update the paint item
  emit pointChangedV2( mCadPointList.value( 0 ) );
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
    const QString textValue = constraint->lineEdit()->text();
    if ( !textValue.isEmpty() )
    {
      bool ok;
      const double value = parseUserInput( textValue, ok );
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
  else if ( constraint == mZConstraint.get() )
  {
    emit lockZChanged( activate );
  }
  else if ( constraint == mMConstraint.get() )
  {
    emit lockMChanged( activate );
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
    emit pointChangedV2( mCadPointList.value( 0 ) );
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
  CadCapacities newCapacities = CadCapacities();
  const bool isGeographic = mMapCanvas->mapSettings().destinationCrs().isGeographic();

  // first point is the mouse point (it doesn't count)
  if ( mCadPointList.count() > 1 )
  {
    newCapacities |=  RelativeCoordinates;
    if ( !isGeographic )
    {
      newCapacities |= AbsoluteAngle;
      newCapacities |= Distance;
    }
  }
  if ( mCadPointList.count() > 2 )
  {
    if ( !isGeographic )
      newCapacities |= RelativeAngle;
  }
  if ( !updateUIwithoutChange && newCapacities == mCapacities )
  {
    return;
  }

  const bool snappingEnabled = QgsProject::instance()->snappingConfig().enabled();

  // update the UI according to new capacities
  // still keep the old to compare

  const bool distance =  mCadEnabled && newCapacities.testFlag( Distance );
  const bool relativeAngle = mCadEnabled && newCapacities.testFlag( RelativeAngle );
  const bool absoluteAngle = mCadEnabled && newCapacities.testFlag( AbsoluteAngle );
  const bool relativeCoordinates = mCadEnabled && newCapacities.testFlag( RelativeCoordinates );

  mPerpendicularAction->setEnabled( distance && absoluteAngle && snappingEnabled );
  mParallelAction->setEnabled( distance && absoluteAngle && snappingEnabled );

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
  mLockDistanceButton->setEnabled( distance && relativeCoordinates );
  mDistanceLineEdit->setEnabled( distance && relativeCoordinates );
  emit enabledChangedDistance( distance && relativeCoordinates );
  if ( !( distance && relativeCoordinates ) )
  {
    mDistanceConstraint->setLockMode( CadConstraint::NoLock );
  }

  mRelativeXButton->setEnabled( relativeCoordinates );
  mRelativeYButton->setEnabled( relativeCoordinates );
  mRelativeZButton->setEnabled( relativeCoordinates );
  mRelativeMButton->setEnabled( relativeCoordinates );

  // update capacities
  mCapacities = newCapacities;
  mCadPaintItem->updatePosition();
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
  context.zConstraint = _constraint( mZConstraint.get() );
  context.mConstraint = _constraint( mMConstraint.get() );
  context.distanceConstraint = _constraint( mDistanceConstraint.get() );
  context.angleConstraint = _constraint( mAngleConstraint.get() );
  context.setCadPoints( mCadPointList );

  context.commonAngleConstraint.locked = !mMapCanvas->mapSettings().destinationCrs().isGeographic();
  context.commonAngleConstraint.relative = context.angleConstraint.relative;
  context.commonAngleConstraint.value = mCommonAngleConstraint;

  const QgsCadUtils::AlignMapPointOutput output = QgsCadUtils::alignMapPoint( e->originalMapPoint(), context );

  const bool res = output.valid;
  QgsPoint point = pointXYToPoint( output.finalMapPoint );
  mSnappedSegment.clear();
  if ( output.snapMatch.hasEdge() )
  {
    QgsPointXY edgePt0, edgePt1;
    output.snapMatch.edgePoints( edgePt0, edgePt1 );
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

  if ( output.snapMatch.isValid() )
  {
    mSnapIndicator->setMatch( output.snapMatch );
    mSnapIndicator->setVisible( true );
  }
  else
  {
    mSnapIndicator->setVisible( false );
  }

  /*
   * Ensure that Z and M are passed
   * It will be dropped as needed later.
   */
  point.setZ( QgsMapToolEdit::defaultZValue() );
  point.setM( QgsMapToolEdit::defaultMValue() );

  /*
   * Constraints are applied in 2D, they are always called when using the tool
   * but they do not take into account if when you snap on a vertex it has
   * a Z value.
   * To get the value we use the snapPoint method. However, we only apply it
   * when the snapped point corresponds to the constrained point or on an edge
   * if the topological editing is activated. Also, we don't apply it if
   * the point is not linked to a layer.
   */
  e->setMapPoint( point );
  mSnapMatch = context.snappingUtils->snapToMap( point, nullptr, true );
  if ( mSnapMatch.layer() )
  {
    if ( ( ( mSnapMatch.hasVertex() || mSnapMatch.hasLineEndpoint() ) && ( point == mSnapMatch.point() ) )
         || ( mSnapMatch.hasEdge() && QgsProject::instance()->topologicalEditing() ) )
    {
      e->snapPoint();
      point = mSnapMatch.interpolatedPoint( mMapCanvas->mapSettings().destinationCrs() );
    }
  }

  /*
   * And if M or Z lock button is activated get the value of the input.
   */
  if ( mLockZButton->isChecked() )
  {
    point.setZ( QLocale().toDouble( mZLineEdit->text() ) );
  }
  if ( mLockMButton->isChecked() )
  {
    point.setM( QLocale().toDouble( mMLineEdit->text() ) );
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


void QgsAdvancedDigitizingDockWidget::updateUnlockedConstraintValues( const QgsPoint &point )
{
  bool previousPointExist, penulPointExist;
  const QgsPoint previousPt = previousPointV2( &previousPointExist );
  const QgsPoint penultimatePt = penultimatePointV2( &penulPointExist );

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
    mDistanceConstraint->setValue( std::sqrt( previousPt.distanceSquared( point ) ) );
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
  // --- Z
  if ( !mZConstraint->isLocked() )
  {
    if ( previousPointExist && mZConstraint->relative() )
    {
      mZConstraint->setValue( point.z() - previousPt.z() );
    }
    else
    {
      mZConstraint->setValue( point.z() );
    }
  }
  // --- M
  if ( !mMConstraint->isLocked() )
  {
    if ( previousPointExist && mMConstraint->relative() )
    {
      mMConstraint->setValue( point.m() - previousPt.m() );
    }
    else
    {
      mMConstraint->setValue( point.m() );
    }
  }
}


QList<QgsPointXY> QgsAdvancedDigitizingDockWidget::snapSegmentToAllLayers( const QgsPointXY &originalMapPoint, bool *snapped ) const
{
  QList<QgsPointXY> segment;
  QgsPointXY pt1, pt2;
  QgsPointLocator::Match match;

  QgsSnappingUtils *snappingUtils = mMapCanvas->snappingUtils();

  const QgsSnappingConfig canvasConfig = snappingUtils->config();
  QgsSnappingConfig localConfig = snappingUtils->config();

  localConfig.setMode( Qgis::SnappingMode::AllLayers );
  localConfig.setTypeFlag( Qgis::SnappingType::Segment );
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
  const QgsPoint previousPt = previousPointV2( &previousPointExist );
  const QgsPoint penultimatePt = penultimatePointV2( &penulPointExist );
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
  const QEvent::Type type = e->type();
  switch ( e->key() )
  {
    case Qt::Key_X:
    {
      // modifier+x ONLY caught for ShortcutOverride events...
      if ( type == QEvent::ShortcutOverride && ( e->modifiers() == Qt::AltModifier || e->modifiers() == Qt::ControlModifier ) )
      {
        mXConstraint->toggleLocked();
        emit lockXChanged( mXConstraint->isLocked() );
        emit pointChangedV2( mCadPointList.value( 0 ) );
        e->accept();
      }
      else if ( type == QEvent::ShortcutOverride && e->modifiers() == Qt::ShiftModifier )
      {
        if ( mCapacities.testFlag( RelativeCoordinates ) )
        {
          mXConstraint->toggleRelative();
          emit relativeXChanged( mXConstraint->relative() );
          emit pointChangedV2( mCadPointList.value( 0 ) );
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
        emit pointChangedV2( mCadPointList.value( 0 ) );
        e->accept();
      }
      else if ( type == QEvent::ShortcutOverride &&  e->modifiers() == Qt::ShiftModifier )
      {
        if ( mCapacities.testFlag( RelativeCoordinates ) )
        {
          mYConstraint->toggleRelative();
          emit relativeYChanged( mYConstraint->relative() );
          emit pointChangedV2( mCadPointList.value( 0 ) );
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
    case Qt::Key_Z:
    {
      // modifier+z ONLY caught for ShortcutOverride events...
      if ( type == QEvent::ShortcutOverride && e->modifiers() == Qt::AltModifier )
      {
        mZConstraint->toggleLocked();
        emit lockZChanged( mZConstraint->isLocked() );
        emit pointChangedV2( mCadPointList.value( 0 ) );
        e->accept();
      }
      else if ( type == QEvent::ShortcutOverride &&  e->modifiers() == Qt::ShiftModifier )
      {
        if ( mCapacities.testFlag( RelativeCoordinates ) )
        {
          mZConstraint->toggleRelative();
          emit relativeZChanged( mZConstraint->relative() );
          emit pointChangedV2( mCadPointList.value( 0 ) );
          e->accept();
        }
      }
      // .. but "z" alone ONLY caught for KeyPress events (see comment at start of function)
      else if ( type == QEvent::KeyPress )
      {
        mZLineEdit->setFocus();
        mZLineEdit->selectAll();
        emit focusOnZRequested();
        e->accept();
      }
      break;
    }
    case Qt::Key_M:
    {
      // modifier+m ONLY caught for ShortcutOverride events...
      if ( type == QEvent::ShortcutOverride && ( e->modifiers() == Qt::AltModifier || e->modifiers() == Qt::ControlModifier ) )
      {
        mMConstraint->toggleLocked();
        emit lockMChanged( mMConstraint->isLocked() );
        emit pointChangedV2( mCadPointList.value( 0 ) );
        e->accept();
      }
      else if ( type == QEvent::ShortcutOverride &&  e->modifiers() == Qt::ShiftModifier )
      {
        if ( mCapacities.testFlag( RelativeCoordinates ) )
        {
          mMConstraint->toggleRelative();
          emit relativeMChanged( mMConstraint->relative() );
          emit pointChangedV2( mCadPointList.value( 0 ) );
          e->accept();
        }
      }
      // .. but "m" alone ONLY caught for KeyPress events (see comment at start of function)
      else if ( type == QEvent::KeyPress )
      {
        mMLineEdit->setFocus();
        mMLineEdit->selectAll();
        emit focusOnMRequested();
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
          emit pointChangedV2( mCadPointList.value( 0 ) );
          e->accept();
        }
      }
      else if ( type == QEvent::ShortcutOverride && e->modifiers() == Qt::ShiftModifier )
      {
        if ( mCapacities.testFlag( RelativeAngle ) )
        {
          mAngleConstraint->toggleRelative();
          emit relativeAngleChanged( mAngleConstraint->relative() );
          emit pointChangedV2( mCadPointList.value( 0 ) );
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
        if ( mCapacities.testFlag( RelativeCoordinates ) && mCapacities.testFlag( Distance ) )
        {
          mDistanceConstraint->toggleLocked();
          emit lockDistanceChanged( mDistanceConstraint->isLocked() );
          emit pointChangedV2( mCadPointList.value( 0 ) );
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
        const bool parallel = mParallelAction->isChecked();
        const bool perpendicular = mPerpendicularAction->isChecked();

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

        // run a fake map mouse event to update the paint item
        emit pointChangedV2( mCadPointList.value( 0 ) );
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
  // most of theses lines can be moved to updateCapacity
  connect( mMapCanvas, &QgsMapCanvas::destinationCrsChanged, this, &QgsAdvancedDigitizingDockWidget::enable, Qt::UniqueConnection );
  if ( mMapCanvas->mapSettings().destinationCrs().isGeographic() )
  {
    mAngleLineEdit->setToolTip( tr( "Angle constraint cannot be used on geographic coordinates. Change the coordinates system in the project properties." ) );
    mDistanceLineEdit->setToolTip( tr( "Distance constraint cannot be used on geographic coordinates. Change the coordinates system in the project properties." ) );

    mLabelX->setText( tr( "Long" ) );
    mLabelY->setText( tr( "Lat" ) );

    mXConstraint->setPrecision( 8 );
    mYConstraint->setPrecision( 8 );
  }
  else
  {
    mAngleLineEdit->setToolTip( "<b>" + tr( "Angle" ) + "</b><br>(" + tr( "press a for quick access" ) + ")" );
    mAngleLineEdit->setToolTip( QString() );

    mDistanceLineEdit->setToolTip( "<b>" + tr( "Distance" ) + "</b><br>(" + tr( "press d for quick access" ) + ")" );

    mLabelX->setText( tr( "x" ) );
    mLabelY->setText( tr( "y" ) );

    mXConstraint->setPrecision( 6 );
    mYConstraint->setPrecision( 6 );
  }

  updateCapacity();

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
  QgsPoint pt = pointXYToPoint( point );
  if ( !pointsCount() )
  {
    mCadPointList << pt;
  }
  else
  {
    mCadPointList.insert( 0, pt );
  }

  updateCapacity();
  updateCadPaintItem();
}

void QgsAdvancedDigitizingDockWidget::removePreviousPoint()
{
  if ( !pointsCount() )
    return;

  const int i = pointsCount() > 1 ? 1 : 0;
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

void QgsAdvancedDigitizingDockWidget::updateCurrentPoint( const QgsPoint &point )
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
  if ( updateWidget && mLineEdit->isEnabled() )
    mLineEdit->setText( QLocale().toString( value, 'f', mPrecision ) );
}

void QgsAdvancedDigitizingDockWidget::CadConstraint::toggleLocked()
{
  setLockMode( mLockMode == HardLock ? NoLock : HardLock );
}

void QgsAdvancedDigitizingDockWidget::CadConstraint::toggleRelative()
{
  setRelative( !mRelative );
}

void QgsAdvancedDigitizingDockWidget::CadConstraint::setPrecision( int precision )
{
  mPrecision = precision;
  if ( mLineEdit->isEnabled() )
    mLineEdit->setText( QLocale().toString( mValue, 'f', mPrecision ) );
}

QgsPoint QgsAdvancedDigitizingDockWidget::currentPointV2( bool *exist ) const
{
  if ( exist )
    *exist = pointsCount() > 0;
  if ( pointsCount() > 0 )
    return mCadPointList.value( 0 );
  else
    return QgsPoint();
}

QgsPoint QgsAdvancedDigitizingDockWidget::currentPointLayerCoordinates( QgsMapLayer *layer ) const
{
  if ( pointsCount() > 0 && layer )
  {
    QgsPoint res = mCadPointList.value( 0 );
    const QgsPointXY layerCoordinates = mMapCanvas->mapSettings().mapToLayerCoordinates( layer, res );
    res.setX( layerCoordinates.x() );
    res.setY( layerCoordinates.y() );
    return res;
  }
  return QgsPoint();
}

QgsPoint QgsAdvancedDigitizingDockWidget::previousPointV2( bool *exist ) const
{
  if ( exist )
    *exist = pointsCount() > 1;
  if ( pointsCount() > 1 )
    return mCadPointList.value( 1 );
  else
    return QgsPoint();
}

QgsPoint QgsAdvancedDigitizingDockWidget::penultimatePointV2( bool *exist ) const
{
  if ( exist )
    *exist = pointsCount() > 2;
  if ( pointsCount() > 2 )
    return mCadPointList.value( 2 );
  else
    return QgsPoint();
}

QgsPoint QgsAdvancedDigitizingDockWidget::pointXYToPoint( const QgsPointXY &point ) const
{
  return QgsPoint( point.x(), point.y(), getLineZ(), getLineM() );
}

double QgsAdvancedDigitizingDockWidget::getLineZ( ) const
{
  return mZLineEdit->isEnabled() ? QLocale().toDouble( mZLineEdit->text() ) : std::numeric_limits<double>::quiet_NaN();
}

double QgsAdvancedDigitizingDockWidget::getLineM( ) const
{
  return mMLineEdit->isEnabled() ? QLocale().toDouble( mMLineEdit->text() ) : std::numeric_limits<double>::quiet_NaN();
}
