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

#include <QSettings>
#include <QMenu>

#include "math.h"

#include "qgsadvanceddigitizingdockwidget.h"
#include "qgsadvanceddigitizingcanvasitem.h"
#include "qgsapplication.h"
#include "qgsexpression.h"
#include "qgslogger.h"
#include "qgsmapcanvas.h"
#include "qgsmaptoolcapture.h"
#include "qgsmaptooladvanceddigitizing.h"
#include "qgsmessagebaritem.h"
#include "qgspoint.h"
#include "qgslinestringv2.h"
#include "qgsfocuswatcher.h"

struct EdgesOnlyFilter : public QgsPointLocator::MatchFilter
{
  bool acceptMatch( const QgsPointLocator::Match& m ) override { return m.hasEdge(); }
};

bool QgsAdvancedDigitizingDockWidget::lineCircleIntersection( const QgsPoint& center, const double radius, const QList<QgsPoint>& segment, QgsPoint& intersection )
{
  Q_ASSERT( segment.count() == 2 );

  // formula taken from http://mathworld.wolfram.com/Circle-LineIntersection.html

  const double x1 = segment[0].x() - center.x();
  const double y1 = segment[0].y() - center.y();
  const double x2 = segment[1].x() - center.x();
  const double y2 = segment[1].y() - center.y();
  const double dx = x2 - x1;
  const double dy = y2 - y1;

  const double dr = sqrt( pow( dx, 2 ) + pow( dy, 2 ) );
  const double d = x1 * y2 - x2 * y1;

  const double disc = pow( radius, 2 ) * pow( dr, 2 ) - pow( d, 2 );

  if ( disc < 0 )
  {
    //no intersection or tangeant
    return false;
  }
  else
  {
    // two solutions
    const int sgnDy = dy < 0 ? -1 : 1;

    const double ax = center.x() + ( d * dy + sgnDy * dx * sqrt( pow( radius, 2 ) * pow( dr, 2 ) - pow( d, 2 ) ) ) / ( pow( dr, 2 ) );
    const double ay = center.y() + ( -d * dx + qAbs( dy ) * sqrt( pow( radius, 2 ) * pow( dr, 2 ) - pow( d, 2 ) ) ) / ( pow( dr, 2 ) );
    const QgsPoint p1( ax, ay );

    const double bx = center.x() + ( d * dy - sgnDy * dx * sqrt( pow( radius, 2 ) * pow( dr, 2 ) - pow( d, 2 ) ) ) / ( pow( dr, 2 ) );
    const double by = center.y() + ( -d * dx - qAbs( dy ) * sqrt( pow( radius, 2 ) * pow( dr, 2 ) - pow( d, 2 ) ) ) / ( pow( dr, 2 ) );
    const QgsPoint p2( bx, by );

    // snap to nearest intersection

    if ( intersection.sqrDist( p1 ) < intersection.sqrDist( p2 ) )
    {
      intersection.set( p1.x(), p1.y() );
    }
    else
    {
      intersection.set( p2.x(), p2.y() );
    }
    return true;
  }
}


QgsAdvancedDigitizingDockWidget::QgsAdvancedDigitizingDockWidget( QgsMapCanvas* canvas, QWidget *parent )
    : QgsDockWidget( parent )
    , mMapCanvas( canvas )
    , mCurrentMapToolSupportsCad( false )
    , mCadEnabled( false )
    , mConstructionMode( false )
    , mSnappingMode(( QgsMapMouseEvent::SnappingMode ) QSettings().value( "/Cad/SnappingMode", QgsMapMouseEvent::SnapProjectConfig ).toInt() )
    , mCommonAngleConstraint( QSettings().value( "/Cad/CommonAngle", 90 ).toInt() )
    , mSnappedToVertex( false )
    , mSessionActive( false )
    , mErrorMessage( nullptr )
{
  setupUi( this );

  mCadPaintItem = new QgsAdvancedDigitizingCanvasItem( canvas, this ) ;

  mAngleConstraint.reset( new CadConstraint( mAngleLineEdit, mLockAngleButton, mRelativeAngleButton, mRepeatingLockAngleButton ) );
  mDistanceConstraint.reset( new CadConstraint( mDistanceLineEdit, mLockDistanceButton, nullptr, mRepeatingLockDistanceButton ) );
  mXConstraint.reset( new CadConstraint( mXLineEdit, mLockXButton, mRelativeXButton, mRepeatingLockXButton ) );
  mYConstraint.reset( new CadConstraint( mYLineEdit, mLockYButton, mRelativeYButton, mRepeatingLockYButton ) );
  mAdditionalConstraint = NoConstraint ;

  mMapCanvas->installEventFilter( this );
  mAngleLineEdit->installEventFilter( this );
  mDistanceLineEdit->installEventFilter( this );
  mXLineEdit->installEventFilter( this );
  mYLineEdit->installEventFilter( this );

  // this action is also used in the advanced digitizing tool bar
  mEnableAction = new QAction( this );
  mEnableAction->setText( tr( "Enable advanced digitizing tools" ) );
  mEnableAction->setIcon( QgsApplication::getThemeIcon( "/cadtools/cad.svg" ) );
  mEnableAction->setCheckable( true );
  mEnabledButton->addAction( mEnableAction );
  mEnabledButton->setDefaultAction( mEnableAction );

  // Connect the UI to the event filter to update constraints
  connect( mEnableAction, SIGNAL( triggered( bool ) ), this, SLOT( activateCad( bool ) ) );
  connect( mConstructionModeButton, SIGNAL( clicked( bool ) ), this, SLOT( setConstructionMode( bool ) ) );
  connect( mParallelButton, SIGNAL( clicked( bool ) ), this, SLOT( addtionalConstraintClicked( bool ) ) );
  connect( mPerpendicularButton, SIGNAL( clicked( bool ) ), this, SLOT( addtionalConstraintClicked( bool ) ) );
  connect( mLockAngleButton, SIGNAL( clicked( bool ) ), this, SLOT( lockConstraint( bool ) ) );
  connect( mLockDistanceButton, SIGNAL( clicked( bool ) ), this, SLOT( lockConstraint( bool ) ) );
  connect( mLockXButton, SIGNAL( clicked( bool ) ), this, SLOT( lockConstraint( bool ) ) );
  connect( mLockYButton, SIGNAL( clicked( bool ) ), this, SLOT( lockConstraint( bool ) ) );
  connect( mRelativeAngleButton, SIGNAL( clicked( bool ) ), this, SLOT( setConstraintRelative( bool ) ) );
  connect( mRelativeXButton, SIGNAL( clicked( bool ) ), this, SLOT( setConstraintRelative( bool ) ) );
  connect( mRelativeYButton, SIGNAL( clicked( bool ) ), this, SLOT( setConstraintRelative( bool ) ) );
  connect( mRepeatingLockDistanceButton, SIGNAL( clicked( bool ) ), this, SLOT( setConstraintRepeatingLock( bool ) ) );
  connect( mRepeatingLockAngleButton, SIGNAL( clicked( bool ) ), this, SLOT( setConstraintRepeatingLock( bool ) ) );
  connect( mRepeatingLockXButton, SIGNAL( clicked( bool ) ), this, SLOT( setConstraintRepeatingLock( bool ) ) );
  connect( mRepeatingLockYButton, SIGNAL( clicked( bool ) ), this, SLOT( setConstraintRepeatingLock( bool ) ) );
  connect( mAngleLineEdit, SIGNAL( returnPressed() ), this, SLOT( lockConstraint() ) );
  connect( mDistanceLineEdit, SIGNAL( returnPressed() ), this, SLOT( lockConstraint() ) );
  connect( mXLineEdit, SIGNAL( returnPressed() ), this, SLOT( lockConstraint() ) );
  connect( mYLineEdit, SIGNAL( returnPressed() ), this, SLOT( lockConstraint() ) );
  connect( mAngleLineEdit, SIGNAL( textEdited( QString ) ), this, SLOT( constraintTextEdited( QString ) ) );
  connect( mDistanceLineEdit, SIGNAL( textEdited( QString ) ), this, SLOT( constraintTextEdited( QString ) ) );
  connect( mXLineEdit, SIGNAL( textEdited( QString ) ), this, SLOT( constraintTextEdited( QString ) ) );
  connect( mYLineEdit, SIGNAL( textEdited( QString ) ), this, SLOT( constraintTextEdited( QString ) ) );
  //also watch for focus out events on these widgets
  QgsFocusWatcher* angleWatcher = new QgsFocusWatcher( mAngleLineEdit );
  connect( angleWatcher, SIGNAL( focusOut() ), this, SLOT( constraintFocusOut() ) );
  QgsFocusWatcher* distanceWatcher = new QgsFocusWatcher( mDistanceLineEdit );
  connect( distanceWatcher, SIGNAL( focusOut() ), this, SLOT( constraintFocusOut() ) );
  QgsFocusWatcher* xWatcher = new QgsFocusWatcher( mXLineEdit );
  connect( xWatcher, SIGNAL( focusOut() ), this, SLOT( constraintFocusOut() ) );
  QgsFocusWatcher* yWatcher = new QgsFocusWatcher( mYLineEdit );
  connect( yWatcher, SIGNAL( focusOut() ), this, SLOT( constraintFocusOut() ) );

  // config menu
  QMenu *menu = new QMenu( this );
  // common angles
  QActionGroup* angleButtonGroup = new QActionGroup( menu ); // actions are exclusive for common angles
  mCommonAngleActions = QMap<QAction*, int>();
  QList< QPair< int, QString > > commonAngles;
  commonAngles << QPair<int, QString>( 0, tr( "Do not snap to common angles" ) );
  commonAngles << QPair<int, QString>( 30, tr( "Snap to 30%1 angles" ).arg( QString::fromUtf8( "°" ) ) );
  commonAngles << QPair<int, QString>( 45, tr( "Snap to 45%1 angles" ).arg( QString::fromUtf8( "°" ) ) );
  commonAngles << QPair<int, QString>( 90, tr( "Snap to 90%1 angles" ).arg( QString::fromUtf8( "°" ) ) );
  for ( QList< QPair< int, QString > >::const_iterator it = commonAngles.begin(); it != commonAngles.end(); ++it )
  {
    QAction* action = new QAction( it->second, menu );
    action->setCheckable( true );
    action->setChecked( it->first == mCommonAngleConstraint );
    menu->addAction( action );
    angleButtonGroup->addAction( action );
    mCommonAngleActions.insert( action, it->first );
  }
  // snapping on layers
  menu->addSeparator();
  QActionGroup* snapButtonGroup = new QActionGroup( menu ); // actions are exclusive for snapping modes
  mSnappingActions = QMap<QAction*, QgsMapMouseEvent::SnappingMode>();
  QList< QPair< QgsMapMouseEvent::SnappingMode, QString > > snappingModes;
  snappingModes << QPair<QgsMapMouseEvent::SnappingMode, QString>( QgsMapMouseEvent::NoSnapping, tr( "Do not snap to vertices or segment" ) );
  snappingModes << QPair<QgsMapMouseEvent::SnappingMode, QString>( QgsMapMouseEvent::SnapProjectConfig, tr( "Snap according to project configuration" ) );
  snappingModes << QPair<QgsMapMouseEvent::SnappingMode, QString>( QgsMapMouseEvent::SnapAllLayers, tr( "Snap to all layers" ) );
  for ( QList< QPair< QgsMapMouseEvent::SnappingMode, QString > >::const_iterator it = snappingModes.begin(); it != snappingModes.end(); ++it )
  {
    QAction* action = new QAction( it->second, menu );
    action->setCheckable( true );
    action->setChecked( it->first == mSnappingMode );
    menu->addAction( action );
    snapButtonGroup->addAction( action );
    mSnappingActions.insert( action, it->first );
  }

  mSettingsButton->setMenu( menu );
  connect( mSettingsButton, SIGNAL( triggered( QAction* ) ), this, SLOT( settingsButtonTriggered( QAction* ) ) );

  updateCapacity( true );
  disable();
}

void QgsAdvancedDigitizingDockWidget::hideEvent( QHideEvent* )
{
  // disable CAD but do not unset map event filter
  // so it will be reactivated whenever the map tool is show again
  setCadEnabled( false );
}

void QgsAdvancedDigitizingDockWidget::setCadEnabled( bool enabled )
{
  mCadEnabled = enabled;
  mEnableAction->setChecked( enabled );
  mCadButtons->setEnabled( enabled );
  mInputWidgets->setEnabled( enabled );

  clearPoints();
  releaseLocks();
  setConstructionMode( false );
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

void QgsAdvancedDigitizingDockWidget::addtionalConstraintClicked( bool activated )
{
  if ( !activated )
  {
    lockAdditionalConstraint( NoConstraint );
  }
  if ( sender() == mParallelButton )
  {
    lockAdditionalConstraint( Parallel );
  }
  else if ( sender() == mPerpendicularButton )
  {
    lockAdditionalConstraint( Perpendicular );
  }
}


void QgsAdvancedDigitizingDockWidget::setConstraintRelative( bool activate )
{
  if ( sender() == mRelativeAngleButton )
  {
    mAngleConstraint->setRelative( activate );
  }
  else if ( sender() == mRelativeXButton )
  {
    mXConstraint->setRelative( activate );
  }
  else if ( sender() == mRelativeYButton )
  {
    mYConstraint->setRelative( activate );
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
  mConstructionModeButton->setChecked( enabled );
}

void QgsAdvancedDigitizingDockWidget::settingsButtonTriggered( QAction* action )
{
  // snapping
  QMap<QAction*, QgsMapMouseEvent::SnappingMode>::const_iterator isn = mSnappingActions.constFind( action );
  if ( isn != mSnappingActions.constEnd() )
  {
    isn.key()->setChecked( true );
    mSnappingMode = isn.value();
    QSettings().setValue( "/Cad/SnappingMode", ( int )isn.value() );
    return;
  }

  // common angles
  QMap<QAction*, int>::const_iterator ica = mCommonAngleActions.constFind( action );
  if ( ica != mCommonAngleActions.constEnd() )
  {
    ica.key()->setChecked( true );
    mCommonAngleConstraint = ica.value();
    QSettings().setValue( "/Cad/CommonAngle", ica.value() );
    return;
  }
}

void QgsAdvancedDigitizingDockWidget::releaseLocks( bool releaseRepeatingLocks )
{
  // release all locks except construction mode

  lockAdditionalConstraint( NoConstraint );

  if ( releaseRepeatingLocks || !mAngleConstraint->isRepeatingLock() )
    mAngleConstraint->setLockMode( CadConstraint::NoLock );
  if ( releaseRepeatingLocks || !mDistanceConstraint->isRepeatingLock() )
    mDistanceConstraint->setLockMode( CadConstraint::NoLock );
  if ( releaseRepeatingLocks || !mXConstraint->isRepeatingLock() )
    mXConstraint->setLockMode( CadConstraint::NoLock );
  if ( releaseRepeatingLocks || !mYConstraint->isRepeatingLock() )
    mYConstraint->setLockMode( CadConstraint::NoLock );
}

#if 0
void QgsAdvancedDigitizingDockWidget::emit pointChanged()
{
  // run a fake map mouse event to update the paint item
  QPoint globalPos = mMapCanvas->cursor().pos();
  QPoint pos = mMapCanvas->mapFromGlobal( globalPos );
  QMouseEvent* e = new QMouseEvent( QEvent::MouseMove, pos, globalPos, Qt::NoButton, Qt::NoButton, Qt::NoModifier );
  mCurrentMapTool->canvasMoveEvent( e );
}
#endif

QgsAdvancedDigitizingDockWidget::CadConstraint* QgsAdvancedDigitizingDockWidget::objectToConstraint( const QObject* obj ) const
{
  CadConstraint* constraint = nullptr;
  if ( obj == mAngleLineEdit || obj == mLockAngleButton )
  {
    constraint = mAngleConstraint.data();
  }
  else if ( obj == mDistanceLineEdit || obj == mLockDistanceButton )
  {
    constraint = mDistanceConstraint.data();
  }
  else if ( obj == mXLineEdit  || obj == mLockXButton )
  {
    constraint = mXConstraint.data();
  }
  else if ( obj == mYLineEdit  || obj == mLockYButton )
  {
    constraint = mYConstraint.data();
  }
  return constraint;
}

double QgsAdvancedDigitizingDockWidget::parseUserInput( const QString& inputValue, bool& ok ) const
{
  ok = false;
  double value = inputValue.toDouble( &ok );
  if ( ok )
  {
    return value;
  }
  else
  {
    // try to evalute expression
    QgsExpression expr( inputValue );
    QVariant result = expr.evaluate();
    if ( expr.hasEvalError() )
      ok = false;
    else
      value = result.toDouble( &ok );
    return value;
  }
}

void QgsAdvancedDigitizingDockWidget::updateConstraintValue( CadConstraint* constraint, const QString& textValue, bool convertExpression )
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
  CadConstraint* constraint = objectToConstraint( sender() );
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

  if ( activate )
  {
    // deactivate perpendicular/parallel if angle has been activated
    if ( constraint == mAngleConstraint.data() )
    {
      lockAdditionalConstraint( NoConstraint );
    }

    // run a fake map mouse event to update the paint item
    emit pointChanged( mCadPointList.value( 0 ) );
  }
}

void QgsAdvancedDigitizingDockWidget::constraintTextEdited( const QString& textValue )
{
  CadConstraint* constraint = objectToConstraint( sender() );
  if ( !constraint )
  {
    return;
  }

  updateConstraintValue( constraint, textValue, false );
}

void QgsAdvancedDigitizingDockWidget::constraintFocusOut()
{
  QLineEdit* lineEdit = qobject_cast< QLineEdit* >( sender()->parent() );
  if ( !lineEdit )
    return;

  CadConstraint* constraint = objectToConstraint( lineEdit );
  if ( !constraint )
  {
    return;
  }

  updateConstraintValue( constraint, lineEdit->text(), true );
}

void QgsAdvancedDigitizingDockWidget::lockAdditionalConstraint( AdditionalConstraint constraint )
{
  mAdditionalConstraint = constraint;
  mPerpendicularButton->setChecked( constraint == Perpendicular );
  mParallelButton->setChecked( constraint == Parallel );
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

  // update the UI according to new capacities
  // still keep the old to compare

  bool relativeAngle = mCadEnabled && newCapacities.testFlag( RelativeAngle );
  bool absoluteAngle = mCadEnabled && newCapacities.testFlag( AbsoluteAngle );
  bool relativeCoordinates = mCadEnabled && newCapacities.testFlag( RelativeCoordinates );

  mPerpendicularButton->setEnabled( absoluteAngle );
  mParallelButton->setEnabled( absoluteAngle );
  if ( !absoluteAngle )
  {
    lockAdditionalConstraint( NoConstraint );
  }

  // absolute angle = azimuth, relative = from previous line
  mLockAngleButton->setEnabled( absoluteAngle );
  mRelativeAngleButton->setEnabled( relativeAngle );
  mAngleLineEdit->setEnabled( absoluteAngle );
  if ( !absoluteAngle )
  {
    mAngleConstraint->setLockMode( CadConstraint::NoLock );
  }
  if ( !relativeAngle )
  {
    mAngleConstraint->setRelative( false );
  }
  else if ( relativeAngle && !mCapacities.testFlag( RelativeAngle ) )
  {
    // set angle mode to relative if can do and wasn't available before
    mAngleConstraint->setRelative( true );
  }

  // distance is alway relative
  mLockDistanceButton->setEnabled( relativeCoordinates );
  mDistanceLineEdit->setEnabled( relativeCoordinates );
  if ( !relativeCoordinates )
  {
    mDistanceConstraint->setLockMode( CadConstraint::NoLock );
  }

  mRelativeXButton->setEnabled( relativeCoordinates );
  mRelativeYButton->setEnabled( relativeCoordinates );

  // update capacities
  mCapacities = newCapacities;
}


bool QgsAdvancedDigitizingDockWidget::applyConstraints( QgsMapMouseEvent* e )
{
  bool res = true;

  QgsDebugMsg( "Constraints (locked / relative / value" );
  QgsDebugMsg( QString( "Angle:    %1 %2 %3" ).arg( mAngleConstraint->isLocked() ).arg( mAngleConstraint->relative() ).arg( mAngleConstraint->value() ) );
  QgsDebugMsg( QString( "Distance: %1 %2 %3" ).arg( mDistanceConstraint->isLocked() ).arg( mDistanceConstraint->relative() ).arg( mDistanceConstraint->value() ) );
  QgsDebugMsg( QString( "X:        %1 %2 %3" ).arg( mXConstraint->isLocked() ).arg( mXConstraint->relative() ).arg( mXConstraint->value() ) );
  QgsDebugMsg( QString( "Y:        %1 %2 %3" ).arg( mYConstraint->isLocked() ).arg( mYConstraint->relative() ).arg( mYConstraint->value() ) );

  QgsPoint point = e->snapPoint( mSnappingMode );

  mSnappedSegment = e->snapSegment( mSnappingMode );

  bool previousPointExist, penulPointExist;
  QgsPoint previousPt = previousPoint( &previousPointExist );
  QgsPoint penultimatePt = penultimatePoint( &penulPointExist );

  // *****************************
  // ---- X Constrain
  if ( mXConstraint->isLocked() )
  {
    if ( !mXConstraint->relative() )
    {
      point.setX( mXConstraint->value() );
    }
    else if ( mCapacities.testFlag( RelativeCoordinates ) )
    {
      point.setX( previousPt.x() + mXConstraint->value() );
    }
    if ( !mSnappedSegment.isEmpty() && !mXConstraint->isLocked() )
    {
      // intersect with snapped segment line at X ccordinate
      const double dx = mSnappedSegment.at( 1 ).x() - mSnappedSegment.at( 0 ).x();
      if ( dx == 0 )
      {
        point.setY( mSnappedSegment.at( 0 ).y() );
      }
      else
      {
        const double dy = mSnappedSegment.at( 1 ).y() - mSnappedSegment.at( 0 ).y();
        point.setY( mSnappedSegment.at( 0 ).y() + ( dy * ( point.x() - mSnappedSegment.at( 0 ).x() ) ) / dx );
      }
    }
  }
  // *****************************
  // ---- Y Constrain
  if ( mYConstraint->isLocked() )
  {
    if ( !mYConstraint->relative() )
    {
      point.setY( mYConstraint->value() );
    }
    else if ( mCapacities.testFlag( RelativeCoordinates ) )
    {
      point.setY( previousPt.y() + mYConstraint->value() );
    }
    if ( !mSnappedSegment.isEmpty() && !mYConstraint->isLocked() )
    {
      // intersect with snapped segment line at Y ccordinate
      const double dy = mSnappedSegment.at( 1 ).y() - mSnappedSegment.at( 0 ).y();
      if ( dy == 0 )
      {
        point.setX( mSnappedSegment.at( 0 ).x() );
      }
      else
      {
        const double dx = mSnappedSegment.at( 1 ).x() - mSnappedSegment.at( 0 ).x();
        point.setX( mSnappedSegment.at( 0 ).x() + ( dx * ( point.y() - mSnappedSegment.at( 0 ).y() ) ) / dy );
      }
    }
  }
  // *****************************
  // ---- Angle constrain
  // input angles are in degrees
  if ( mAngleConstraint->lockMode() == CadConstraint::SoftLock )
  {
    // reset the lock
    mAngleConstraint->setLockMode( CadConstraint::NoLock );
  }
  if ( !mAngleConstraint->isLocked() && mCapacities.testFlag( AbsoluteAngle ) && mCommonAngleConstraint != 0 )
  {
    double commonAngle = mCommonAngleConstraint * M_PI / 180;
    // see if soft common angle constraint should be performed
    // only if not in HardLock mode
    double softAngle = qAtan2( point.y() - previousPt.y(),
                               point.x() - previousPt.x() );
    double deltaAngle = 0;
    if ( mAngleConstraint->relative() && mCapacities.testFlag( RelativeAngle ) )
    {
      // compute the angle relative to the last segment (0° is aligned with last segment)
      deltaAngle = qAtan2( previousPt.y() - penultimatePt.y(),
                           previousPt.x() - penultimatePt.x() );
      softAngle -= deltaAngle;
    }
    int quo = qRound( softAngle / commonAngle );
    if ( qAbs( softAngle - quo * commonAngle ) * 180.0 * M_1_PI <= SoftConstraintToleranceDegrees )
    {
      // also check the distance in pixel to the line, otherwise it's too sticky at long ranges
      softAngle = quo * commonAngle ;
      // http://mathworld.wolfram.com/Point-LineDistance2-Dimensional.html
      // use the direction vector (cos(a),sin(a)) from previous point. |x2-x1|=1 since sin2+cos2=1
      const double dist = qAbs( qCos( softAngle + deltaAngle ) * ( previousPt.y() - point.y() )
                                - qSin( softAngle + deltaAngle ) * ( previousPt.x() - point.x() ) );
      if ( dist / mMapCanvas->mapSettings().mapUnitsPerPixel() < SoftConstraintTolerancePixel )
      {
        mAngleConstraint->setLockMode( CadConstraint::SoftLock );
        mAngleConstraint->setValue( 180.0 / M_PI * softAngle );
      }
    }
  }
  if ( mAngleConstraint->isLocked() )
  {
    double angleValue = mAngleConstraint->value() * M_PI / 180;
    if ( mAngleConstraint->relative() && mCapacities.testFlag( RelativeAngle ) )
    {
      // compute the angle relative to the last segment (0° is aligned with last segment)
      angleValue += qAtan2( previousPt.y() - penultimatePt.y(),
                            previousPt.x() - penultimatePt.x() );
    }

    double cosa = qCos( angleValue );
    double sina = qSin( angleValue );
    double v = ( point.x() - previousPt.x() ) * cosa + ( point.y() - previousPt.y() ) * sina ;
    if ( mXConstraint->isLocked() && mYConstraint->isLocked() )
    {
      // do nothing if both X,Y are already locked
    }
    else if ( mXConstraint->isLocked() )
    {
      if ( qgsDoubleNear( cosa, 0.0 ) )
      {
        res = false;
      }
      else
      {
        double x = mXConstraint->value();
        if ( !mXConstraint->relative() )
        {
          x -= previousPt.x();
        }
        point.setY( previousPt.y() + x * sina / cosa );
      }
    }
    else if ( mYConstraint->isLocked() )
    {
      if ( qgsDoubleNear( sina, 0.0 ) )
      {
        res = false;
      }
      else
      {
        double y = mYConstraint->value();
        if ( !mYConstraint->relative() )
        {
          y -= previousPt.y();
        }
        point.setX( previousPt.x() + y * cosa / sina );
      }
    }
    else
    {
      point.setX( previousPt.x() + cosa * v );
      point.setY( previousPt.y() + sina * v );
    }

    if ( !mSnappedSegment.isEmpty() && !mDistanceConstraint->isLocked() )
    {
      // magnetize to the intersection of the snapped segment and the lockedAngle

      // line of previous point + locked angle
      const double x1 = previousPt.x();
      const double y1 = previousPt.y();
      const double x2 = previousPt.x() + cosa;
      const double y2 = previousPt.y() + sina;
      // line of snapped segment
      const double x3 = mSnappedSegment.at( 0 ).x();
      const double y3 = mSnappedSegment.at( 0 ).y();
      const double x4 = mSnappedSegment.at( 1 ).x();
      const double y4 = mSnappedSegment.at( 1 ).y();

      const double d = ( x1 - x2 ) * ( y3 - y4 ) - ( y1 - y2 ) * ( x3 - x4 );

      // do not compute intersection if lines are almost parallel
      // this threshold might be adapted
      if ( qAbs( d ) > 0.01 )
      {
        point.setX((( x3 - x4 )*( x1*y2 - y1*x2 ) - ( x1 - x2 )*( x3*y4 - y3*x4 ) ) / d );
        point.setY((( y3 - y4 )*( x1*y2 - y1*x2 ) - ( y1 - y2 )*( x3*y4 - y3*x4 ) ) / d );
      }
    }
  }
  // *****************************
  // ---- Distance constraint
  if ( mDistanceConstraint->isLocked() && previousPointExist )
  {
    if ( mXConstraint->isLocked() || mYConstraint->isLocked() )
    {
      // perform both to detect errors in constraints
      if ( mXConstraint->isLocked() )
      {
        const QList<QgsPoint> verticalSegment = QList<QgsPoint>()
                                                << QgsPoint( mXConstraint->value(), point.y() )
                                                << QgsPoint( mXConstraint->value(), point.y() + 1 );
        res &= lineCircleIntersection( previousPt, mDistanceConstraint->value(), verticalSegment, point );
      }
      if ( mYConstraint->isLocked() )
      {
        const QList<QgsPoint> horizontalSegment = QList<QgsPoint>()
            << QgsPoint( point.x(), mYConstraint->value() )
            << QgsPoint( point.x() + 1, mYConstraint->value() );
        res &= lineCircleIntersection( previousPt, mDistanceConstraint->value(), horizontalSegment, point );
      }
    }
    else
    {
      const double dist = sqrt( point.sqrDist( previousPt ) );
      if ( dist == 0 )
      {
        // handle case where mouse is over origin and distance constraint is enabled
        // take arbitrary horizontal line
        point.set( previousPt.x() + mDistanceConstraint->value(), previousPt.y() );
      }
      else
      {
        const double vP = mDistanceConstraint->value() / dist;
        point.set( previousPt.x() + ( point.x() - previousPt.x() ) * vP,
                   previousPt.y() + ( point.y() - previousPt.y() ) * vP );
      }

      if ( !mSnappedSegment.isEmpty() && !mAngleConstraint->isLocked() )
      {
        // we will magnietize to the intersection of that segment and the lockedDistance !
        res &= lineCircleIntersection( previousPt, mDistanceConstraint->value(), snappedSegment(), point );
      }
    }
  }

  // *****************************
  // ---- caluclate CAD values
  QgsDebugMsg( QString( "point:             %1 %2" ).arg( point.x() ).arg( point.y() ) );
  QgsDebugMsg( QString( "previous point:    %1 %2" ).arg( previousPt.x() ).arg( previousPt.y() ) );
  QgsDebugMsg( QString( "penultimate point: %1 %2" ).arg( penultimatePt.x() ).arg( penultimatePt.y() ) );
  //QgsDebugMsg( QString( "dx: %1 dy: %2" ).arg( point.x() - previousPt.x() ).arg( point.y() - previousPt.y() ) );
  //QgsDebugMsg( QString( "ddx: %1 ddy: %2" ).arg( previousPt.x() - penultimatePt.x() ).arg( previousPt.y() - penultimatePt.y() ) );

  // set the point coordinates in the map event
  e->setMapPoint( point );

  // update the point list
  updateCurrentPoint( point );

  // *****************************
  // ---- update the GUI with the values
  // --- angle
  if ( !mAngleConstraint->isLocked() && previousPointExist )
  {
    double angle = 0.0;
    if ( penulPointExist && mAngleConstraint->relative() )
    {
      // previous angle
      angle = qAtan2( previousPt.y() - penultimatePt.y(),
                      previousPt.x() - penultimatePt.x() );
    }
    angle = ( qAtan2( point.y() - previousPt.y(),
                      point.x() - previousPt.x()
                    ) - angle ) * 180 / M_PI;
    // modulus
    angle = fmod( angle, 360.0 );
    mAngleConstraint->setValue( angle );
  }
  // --- distance
  if ( !mDistanceConstraint->isLocked() && previousPointExist )
  {
    mDistanceConstraint->setValue( sqrt( previousPt.sqrDist( point ) ) );
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

  return res;
}


bool QgsAdvancedDigitizingDockWidget::alignToSegment( QgsMapMouseEvent* e, CadConstraint::LockMode lockMode )
{
  if ( mAdditionalConstraint == NoConstraint )
  {
    return false;
  }

  bool previousPointExist, penulPointExist, mSnappedSegmentExist;
  QgsPoint previousPt = previousPoint( &previousPointExist );
  QgsPoint penultimatePt = penultimatePoint( &penulPointExist );
  QList<QgsPoint> mSnappedSegment = e->snapSegment( mSnappingMode, &mSnappedSegmentExist, true );

  if ( !previousPointExist || !mSnappedSegmentExist )
  {
    return false;
  }

  double angle = qAtan2( mSnappedSegment[0].y() - mSnappedSegment[1].y(), mSnappedSegment[0].x() - mSnappedSegment[1].x() );

  if ( mAngleConstraint->relative() && penulPointExist )
  {
    angle -= qAtan2( previousPt.y() - penultimatePt.y(), previousPt.x() - penultimatePt.x() );
  }

  if ( mAdditionalConstraint == Perpendicular )
  {
    angle += M_PI_2;
  }

  angle *= 180 / M_PI;

  mAngleConstraint->setValue( angle );
  mAngleConstraint->setLockMode( lockMode );
  if ( lockMode == CadConstraint::HardLock )
  {
    mAdditionalConstraint = NoConstraint;
  }

  return true;
}

bool QgsAdvancedDigitizingDockWidget::canvasPressEvent( QgsMapMouseEvent* e )
{
  applyConstraints( e );
  return mCadEnabled && mConstructionMode;
}

bool QgsAdvancedDigitizingDockWidget::canvasReleaseEvent( QgsMapMouseEvent* e, bool captureSegment )
{
  if ( !mCadEnabled )
    return false;

  emit popWarning();

  if ( e->button() == Qt::RightButton )
  {
    clearPoints();
    releaseLocks();
    return false;
  }

  applyConstraints( e );

  if ( alignToSegment( e ) )
  {
    // launch a fake move event so rubber bands of map tools will be adapted with new constraints
    // emit pointChanged( e );

    // Parallel or perpendicular mode and snapped to segment
    // this has emitted the lockAngle signal
    return true;
  }

  addPoint( e->mapPoint() );

  releaseLocks( false );

  if ( e->button() == Qt::LeftButton )
  {
    // stop digitizing if not intermediate point and if line or polygon
    if ( !mConstructionMode && !captureSegment )
    {
      clearPoints();
    }
  }
  return mConstructionMode;
}

bool QgsAdvancedDigitizingDockWidget::canvasMoveEvent( QgsMapMouseEvent* e )
{
  if ( !mCadEnabled )
    return false;

  if ( !applyConstraints( e ) )
  {
    emit pushWarning( tr( "Some constraints are incompatible. Resulting point might be incorrect." ) );
  }
  else
  {
    popWarning();
  }

  // perpendicular/parallel constraint
  // do a soft lock when snapping to a segment
  alignToSegment( e, CadConstraint::SoftLock );
  mCadPaintItem->update();

  return false;
}

bool QgsAdvancedDigitizingDockWidget::canvasKeyPressEventFilter( QKeyEvent* e )
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

bool QgsAdvancedDigitizingDockWidget::eventFilter( QObject* obj, QEvent* event )
{
  // event for line edits
  Q_UNUSED( obj );
  if ( event->type() != QEvent::KeyPress )
  {
    return false;
  }
  QKeyEvent* keyEvent = dynamic_cast<QKeyEvent*>( event );
  if ( !keyEvent )
  {
    return false;
  }
  return filterKeyPress( keyEvent ) ;
}

bool QgsAdvancedDigitizingDockWidget::filterKeyPress( QKeyEvent* e )
{
  switch ( e->key() )
  {
    case Qt::Key_X:
    {
      if ( e->modifiers() == Qt::AltModifier || e->modifiers() == Qt::ControlModifier )
      {
        mXConstraint->toggleLocked();
        emit pointChanged( mCadPointList.value( 0 ) );
      }
      else if ( e->modifiers() == Qt::ShiftModifier )
      {
        if ( mCapacities.testFlag( RelativeCoordinates ) )
        {
          mXConstraint->toggleRelative();
          emit pointChanged( mCadPointList.value( 0 ) );
        }
      }
      else
      {
        mXLineEdit->setFocus();
        mXLineEdit->selectAll();
      }
      break;
    }
    case Qt::Key_Y:
    {
      if ( e->modifiers() == Qt::AltModifier || e->modifiers() == Qt::ControlModifier )
      {
        mYConstraint->toggleLocked();
        emit pointChanged( mCadPointList.value( 0 ) );
      }
      else if ( e->modifiers() == Qt::ShiftModifier )
      {
        if ( mCapacities.testFlag( RelativeCoordinates ) )
        {
          mYConstraint->toggleRelative();
          emit pointChanged( mCadPointList.value( 0 ) );
        }
      }
      else
      {
        mYLineEdit->setFocus();
        mYLineEdit->selectAll();
      }
      break;
    }
    case Qt::Key_A:
    {
      if ( e->modifiers() == Qt::AltModifier || e->modifiers() == Qt::ControlModifier )
      {
        if ( mCapacities.testFlag( AbsoluteAngle ) )
        {
          mAngleConstraint->toggleLocked();
          emit pointChanged( mCadPointList.value( 0 ) );
        }
      }
      else if ( e->modifiers() == Qt::ShiftModifier )
      {
        if ( mCapacities.testFlag( RelativeAngle ) )
        {
          mAngleConstraint->toggleRelative();
          emit pointChanged( mCadPointList.value( 0 ) );
        }
      }
      else
      {
        mAngleLineEdit->setFocus();
        mAngleLineEdit->selectAll();
      }
      break;
    }
    case Qt::Key_D:
    {
      if ( e->modifiers() == Qt::AltModifier || e->modifiers() == Qt::ControlModifier )
      {
        if ( mCapacities.testFlag( RelativeCoordinates ) )
        {
          mDistanceConstraint->toggleLocked();
          emit pointChanged( mCadPointList.value( 0 ) );
        }
      }
      else
      {
        mDistanceLineEdit->setFocus();
        mDistanceLineEdit->selectAll();
      }
      break;
    }
    case Qt::Key_C:
    {
      setConstructionMode( !mConstructionMode );
      break;
    }
    case Qt::Key_P:
    {
      bool parallel = mParallelButton->isChecked();
      bool perpendicular = mPerpendicularButton->isChecked();

      if ( !parallel && !perpendicular )
      {
        lockAdditionalConstraint( Perpendicular );
      }
      else if ( perpendicular )
      {
        lockAdditionalConstraint( Parallel );
      }
      else
      {
        lockAdditionalConstraint( NoConstraint );
      }
      break;
    }
    default:
    {
      return false; // continues
    }
  }
  return true; // stop the event
}

void QgsAdvancedDigitizingDockWidget::enable()
{
  if ( mMapCanvas->mapSettings().destinationCrs().geographicFlag() )
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
    setMaximumHeight( 220 );

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
  mEnableAction->setEnabled( false );
  mErrorLabel->setText( tr( "CAD tools are not enabled for the current map tool" ) );
  mErrorLabel->show();
  mCadWidget->hide();
  setMaximumHeight( 80 );

  mCurrentMapToolSupportsCad = false;

  setCadEnabled( false );
}

void QgsAdvancedDigitizingDockWidget::addPoint( const QgsPoint& point )
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
}

void QgsAdvancedDigitizingDockWidget::removePreviousPoint()
{
  if ( !pointsCount() )
    return;

  int i = pointsCount() > 1 ? 1 : 0;
  mCadPointList.removeAt( i );
  updateCapacity();
}

void QgsAdvancedDigitizingDockWidget::clearPoints()
{
  mCadPointList.clear();
  mSnappedSegment.clear();
  mSnappedToVertex = false;

  updateCapacity();
}

void QgsAdvancedDigitizingDockWidget::updateCurrentPoint( const QgsPoint& point )
{
  if ( !pointsCount() )
  {
    mCadPointList << point;
    updateCapacity();
  }
  else
  {
    mCadPointList[0] = point ;
  }
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
    mLineEdit->setText( QString::number( value, 'f' ) );
}

void QgsAdvancedDigitizingDockWidget::CadConstraint::toggleLocked()
{
  setLockMode( mLockMode == HardLock ? NoLock : HardLock );
}

void QgsAdvancedDigitizingDockWidget::CadConstraint::toggleRelative()
{
  setRelative( mRelative ? false : true );
}

QgsPoint QgsAdvancedDigitizingDockWidget::currentPoint( bool* exist ) const
{
  if ( exist )
    *exist = pointsCount() > 0;
  if ( pointsCount() > 0 )
    return mCadPointList.value( 0 );
  else
    return QgsPoint();
}

QgsPoint QgsAdvancedDigitizingDockWidget::previousPoint( bool* exist ) const
{
  if ( exist )
    *exist = pointsCount() > 1;
  if ( pointsCount() > 1 )
    return mCadPointList.value( 1 );
  else
    return QgsPoint();
}

QgsPoint QgsAdvancedDigitizingDockWidget::penultimatePoint( bool* exist ) const
{
  if ( exist )
    *exist = pointsCount() > 2;
  if ( pointsCount() > 2 )
    return mCadPointList.value( 2 );
  else
    return QgsPoint();
}
