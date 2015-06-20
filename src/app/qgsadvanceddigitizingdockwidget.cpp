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

#include "math.h"

#include "qgisapp.h"
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
    : QDockWidget( parent )
    , mMapCanvas( canvas )
    , mMapToolList( QList<QgsMapToolAdvancedDigitizing*>() )
    , mCurrentMapTool( 0 )
    , mCadEnabled( false )
    , mConstructionMode( false )
    , mSnappingMode(( QgsMapMouseEvent::SnappingMode ) QSettings().value( "/Cad/SnappingMode", ( int )QgsMapMouseEvent::SnapProjectConfig ).toInt() )
    , mCommonAngleConstraint( QSettings().value( "/Cad/CommonAngle", 90 ).toInt() )
    , mCadPointList( QList<QgsPoint>() )
    , mSnappedToVertex( false )
    , mSnappedSegment( QList<QgsPoint>() )
    , mErrorMessage( 0 )
{
  setupUi( this );

  mCadPaintItem = new QgsAdvancedDigitizingCanvasItem( canvas, this ) ;

  mAngleConstraint = new CadConstraint( mAngleLineEdit, mLockAngleButton, mRelativeAngleButton );
  mDistanceConstraint = new CadConstraint( mDistanceLineEdit, mLockDistanceButton ) ;
  mXConstraint = new CadConstraint( mXLineEdit, mLockXButton, mRelativeXButton );
  mYConstraint = new CadConstraint( mYLineEdit, mLockYButton, mRelativeYButton ) ;
  mAdditionalConstraint = NoConstraint ;

  mAngleLineEdit->installEventFilter( this );
  mDistanceLineEdit->installEventFilter( this );
  mXLineEdit->installEventFilter( this );
  mYLineEdit->installEventFilter( this );

  // this action is also used in the advanced digitizing tool bar
  mEnableAction = new QAction( this );
  mEnableAction->setText( tr( "Enable advanced digitizing tools" ) );
  mEnableAction->setIcon( QgsApplication::getThemeIcon( "/cadtools/cad.png" ) );
  mEnableAction->setCheckable( true );
  mEnabledButton->addAction( mEnableAction );
  mEnabledButton->setDefaultAction( mEnableAction );

  // enable/disable on map tool change
  connect( canvas, SIGNAL( mapToolSet( QgsMapTool* ) ), this, SLOT( mapToolChanged( QgsMapTool* ) ) );

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
  connect( mAngleLineEdit, SIGNAL( returnPressed() ), this, SLOT( lockConstraint() ) );
  connect( mDistanceLineEdit, SIGNAL( returnPressed() ), this, SLOT( lockConstraint() ) );
  connect( mXLineEdit, SIGNAL( returnPressed() ), this, SLOT( lockConstraint() ) );
  connect( mYLineEdit, SIGNAL( returnPressed() ), this, SLOT( lockConstraint() ) );

  mapToolChanged( NULL );

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
}

QgsAdvancedDigitizingDockWidget::~QgsAdvancedDigitizingDockWidget()
{
  delete mErrorMessage;
}

void QgsAdvancedDigitizingDockWidget::hideEvent( QHideEvent* )
{
  // disable CAD but do not unset map event filter
  // so it will be reactivated whenever the map tool is show again
  setCadEnabled( false );
}

void QgsAdvancedDigitizingDockWidget::mapToolChanged( QgsMapTool* tool )
{
  QgsMapToolAdvancedDigitizing* toolMap = dynamic_cast<QgsMapToolAdvancedDigitizing*>( tool );
  mCurrentMapTool = 0;
  QString lblText;
  if ( !tool )
  {
    lblText = tr( "No map tool set" );
  }
  else if ( !toolMap || !toolMap->cadAllowed() )
  {
    lblText = tr( "CAD tools are not enabled for the current map tool" );
    QString toolName = tool->toolName();
    if ( !toolName.isEmpty() )
    {
      lblText.append( QString( " (%1)" ).arg( toolName ) );
    }
  }
  else if ( mMapCanvas->mapSettings().destinationCrs().geographicFlag() )
  {
    lblText = tr( "CAD tools can not be used on geographic coordinates. Change the coordinates system in the project properties." );
  }
  else
  {
    mCurrentMapTool = toolMap;
  }

  if ( mCurrentMapTool )
  {
    mEnableAction->setEnabled( true );
    mErrorLabel->hide();
    mCadWidget->show();
    setMaximumSize( 5000, 220 );

    // restore previous status
    const bool enabled = QSettings().value( "/Cad/SessionActive", false ).toBool();
    if ( enabled && !isVisible() )
    {
      show();
    }
    setCadEnabled( enabled );
  }
  else
  {
    mEnableAction->setEnabled( false );
    mErrorLabel->setText( lblText );
    mErrorLabel->show();
    mCadWidget->hide();
    setMaximumSize( 5000, 80 );

    setCadEnabled( false );
  }
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
  enabled &= mCurrentMapTool != 0;

  if ( mErrorMessage )
  {
    QgisApp::instance()->messageBar()->popWidget( mErrorMessage );
    mErrorMessage = 0;
  }
  QSettings().setValue( "/Cad/SessionActive", enabled );

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
  triggerMouseMoveEvent();
}

void QgsAdvancedDigitizingDockWidget::setConstructionMode( bool enabled )
{
  mConstructionMode = enabled;
  mConstructionModeButton->setChecked( enabled );
}

void QgsAdvancedDigitizingDockWidget::settingsButtonTriggered( QAction* action )
{
  // snapping
  QMap<QAction*, QgsMapMouseEvent::SnappingMode>::const_iterator isn = mSnappingActions.find( action );
  if ( isn != mSnappingActions.end() )
  {
    isn.key()->setChecked( true );
    mSnappingMode = isn.value();
    QSettings().setValue( "/Cad/SnappingMode", ( int )isn.value() );
    return;
  }

  // common angles
  QMap<QAction*, int>::const_iterator ica = mCommonAngleActions.find( action );
  if ( ica != mCommonAngleActions.end() )
  {
    ica.key()->setChecked( true );
    mCommonAngleConstraint = ica.value();
    QSettings().setValue( "/Cad/CommonAngle", ica.value() );
    return;
  }
}

void QgsAdvancedDigitizingDockWidget::releaseLocks()
{
  // release all locks except construction mode

  lockAdditionalConstraint( NoConstraint );

  mAngleConstraint->setLockMode( CadConstraint::NoLock );
  mDistanceConstraint->setLockMode( CadConstraint::NoLock );
  mXConstraint->setLockMode( CadConstraint::NoLock );
  mYConstraint->setLockMode( CadConstraint::NoLock );
}

void QgsAdvancedDigitizingDockWidget::triggerMouseMoveEvent()
{
  // run a fake map mouse event to update the paint item
  QPoint globalPos = mMapCanvas->cursor().pos();
  QPoint pos = mMapCanvas->mapFromGlobal( globalPos );
  QMouseEvent* e = new QMouseEvent( QEvent::MouseMove, pos, globalPos, Qt::NoButton, Qt::NoButton, Qt::NoModifier );
  mCurrentMapTool->canvasMoveEvent( e );
}

void QgsAdvancedDigitizingDockWidget::lockConstraint( bool activate /* default true */ )
{
  QObject* obj = sender();
  CadConstraint* constraint = NULL;
  if ( obj == mAngleLineEdit || obj == mLockAngleButton )
  {
    constraint = mAngleConstraint;
  }
  else if ( obj == mDistanceLineEdit || obj == mLockDistanceButton )
  {
    constraint = mDistanceConstraint;
  }
  else if ( obj == mXLineEdit  || obj == mLockXButton )
  {
    constraint = mXConstraint;
  }
  else if ( obj == mYLineEdit  || obj == mLockYButton )
  {
    constraint = mYConstraint;
  }
  if ( !constraint )
  {
    return;
  }

  if ( activate )
  {
    QString textValue = constraint->lineEdit()->text();
    bool ok;
    double value = textValue.toDouble( &ok );
    if ( !textValue.isEmpty() )
    {
      if ( ok )
      {
        constraint->setValue( value );
      }
      else
      {
        // try to evalute expression
        QgsExpression expr( textValue );
        QVariant result = expr.evaluate();
        value = result.toDouble( &ok );
        if ( expr.hasEvalError() || !ok )
        {
          activate = false;
        }
        else
        {
          constraint->setValue( value );
        }
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
    if ( constraint == mAngleConstraint )
    {
      lockAdditionalConstraint( NoConstraint );
    }

    // run a fake map mouse event to update the paint item
    triggerMouseMoveEvent();
  }
}

void QgsAdvancedDigitizingDockWidget::lockAdditionalConstraint( AdditionalConstraint constraint )
{
  mAdditionalConstraint = constraint;
  mPerpendicularButton->setChecked( constraint == Perpendicular );
  mParallelButton->setChecked( constraint == Parallel );
}

void QgsAdvancedDigitizingDockWidget::updateCapacity( bool updateUIwithoutChange )
{
  CadCapacities newCapacities = 0;
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

  QgsPoint point = e->mapPoint();
  mSnappedToVertex = e->isSnappedToVertex();
  mSnappedSegment = e->snapSegment();

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
      const double dx = mSnappedSegment[1].x() - mSnappedSegment[0].x();
      if ( dx == 0 )
      {
        point.setY( mSnappedSegment[0].y() );
      }
      else
      {
        const double dy = mSnappedSegment[1].y() - mSnappedSegment[0].y();
        point.setY( mSnappedSegment[0].y() + ( dy * ( point.x() - mSnappedSegment[0].x() ) ) / dx );
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
      const double dy = mSnappedSegment[1].y() - mSnappedSegment[0].y();
      if ( dy == 0 )
      {
        point.setX( mSnappedSegment[0].x() );
      }
      else
      {
        const double dx = mSnappedSegment[1].x() - mSnappedSegment[0].x();
        point.setX( mSnappedSegment[0].x() + ( dx * ( point.y() - mSnappedSegment[0].y() ) ) / dy );
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
      if ( cosa == 0 )
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
      if ( sina == 0 )
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
      const double x3 = mSnappedSegment[0].x();
      const double y3 = mSnappedSegment[0].y();
      const double x4 = mSnappedSegment[1].x();
      const double y4 = mSnappedSegment[1].y();

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
  e->setPoint( point );
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
  QList<QgsPoint> mSnappedSegment = e->snapSegment( &mSnappedSegmentExist, true );

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

bool QgsAdvancedDigitizingDockWidget::canvasPressEventFilter( QgsMapMouseEvent* e )
{
  Q_UNUSED( e );

  return mCadEnabled && mConstructionMode;
}

bool QgsAdvancedDigitizingDockWidget::canvasReleaseEventFilter( QgsMapMouseEvent* e )
{
  if ( !mCadEnabled )
    return false;

  if ( mErrorMessage )
  {
    QgisApp::instance()->messageBar()->popWidget( mErrorMessage );
    mErrorMessage = 0;
  }

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
    mCurrentMapTool->canvasMoveEvent( e );

    // Parallel or perpendicular mode and snapped to segment
    // this has emitted the lockAngle signal
    return true;
  }

  addPoint( e->mapPoint() );

  releaseLocks();

  if ( e->button() == Qt::LeftButton )
  {
    // stop digitizing if not intermediate point and if line or polygon
    if ( !mConstructionMode &&
         ( e->mapTool()->mode() == QgsMapToolCapture::CaptureNone ||
           e->mapTool()->mode() == QgsMapToolCapture::CapturePoint ) )
    {
      clearPoints();
    }
  }
  return mConstructionMode;
}

bool QgsAdvancedDigitizingDockWidget::canvasMoveEventFilter( QgsMapMouseEvent* e )
{
  if ( !mCadEnabled )
    return false;

  if ( !applyConstraints( e ) )
  {
    if ( !mErrorMessage )
    {
      // errors messages
      mErrorMessage = new QgsMessageBarItem( tr( "CAD tools" ),
                                             tr( "Some constraints are incompatible. Resulting point might be incorrect." ),
                                             QgsMessageBar::WARNING, 0 );

      QgisApp::instance()->messageBar()->pushItem( mErrorMessage );
    }
  }
  else if ( mErrorMessage )
  {
    QgisApp::instance()->messageBar()->popWidget( mErrorMessage );
    mErrorMessage = 0;
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
      releaseLocks();
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
      releaseLocks();
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
        triggerMouseMoveEvent();
      }
      else if ( e->modifiers() == Qt::ShiftModifier )
      {
        if ( mCapacities.testFlag( RelativeCoordinates ) )
        {
          mXConstraint->toggleRelative();
          triggerMouseMoveEvent();
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
        triggerMouseMoveEvent();
      }
      else if ( e->modifiers() == Qt::ShiftModifier )
      {
        if ( mCapacities.testFlag( RelativeCoordinates ) )
        {
          mYConstraint->toggleRelative();
          triggerMouseMoveEvent();
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
          triggerMouseMoveEvent();
        }
      }
      else if ( e->modifiers() == Qt::ShiftModifier )
      {
        if ( mCapacities.testFlag( RelativeAngle ) )
        {
          mAngleConstraint->toggleRelative();
          triggerMouseMoveEvent();
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
          triggerMouseMoveEvent();
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

QgsPoint QgsAdvancedDigitizingDockWidget::currentPoint( bool* exist ) const
{
  if ( exist )
    *exist = pointsCount() > 0;
  if ( pointsCount() > 0 )
    return mCadPointList.at( 0 );
  else
    return QgsPoint();
}

QgsPoint QgsAdvancedDigitizingDockWidget::previousPoint( bool* exist ) const
{
  if ( exist )
    *exist = pointsCount() > 1;
  if ( pointsCount() > 1 )
    return mCadPointList.at( 1 );
  else
    return QgsPoint();
}

QgsPoint QgsAdvancedDigitizingDockWidget::penultimatePoint( bool* exist ) const
{
  if ( exist )
    *exist = pointsCount() > 2;
  if ( pointsCount() > 2 )
    return mCadPointList.at( 2 );
  else
    return QgsPoint();
}

void QgsAdvancedDigitizingDockWidget::addPoint( QgsPoint point )
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

void QgsAdvancedDigitizingDockWidget::updateCurrentPoint( QgsPoint point )
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

  if ( mode == NoLock )
  {
    mLineEdit->clear();
  }
}

void QgsAdvancedDigitizingDockWidget::CadConstraint::setRelative( bool relative )
{
  mRelative = relative;
  if ( mRelativeButton )
  {
    mRelativeButton->setChecked( relative );
  }
}

void QgsAdvancedDigitizingDockWidget::CadConstraint::setValue( double value )
{
  mValue = value;
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
