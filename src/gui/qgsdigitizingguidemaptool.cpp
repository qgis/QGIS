/***************************************************************************
    qgsdigitizingguidemaptool.h
    ----------------------
    begin                : August 2023
    copyright            : (C) Denis Rouzaud
    email                : denis@opengis.ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsdigitizingguidemaptool.h"

#include "qgsannotationlineitem.h"
#include "qgsannotationmarkeritem.h"
#include "qgsannotationpointtextitem.h"
#include "qgscircle.h"
#include "qgsdigitizingguidelayer.h"
#include "qgsgeometry.h"
#include "qgsguiutils.h"
#include "qgsmapcanvas.h"
#include "qgsmapmouseevent.h"
#include "qgsmultipoint.h"
#include "qgsproject.h"
#include "qgsrubberband.h"
#include "qgssettingsentryimpl.h"
#include "qgssettingsregistrycore.h"
#include "qgssnapindicator.h"
#include "qgssnappingutils.h"
#include "qgsfloatingwidget.h"

#include <QInputDialog>


QgsDigitizingGuideMapTool::QgsDigitizingGuideMapTool( QgsMapCanvas *canvas )
  : QgsMapTool( canvas )
{
  connect( canvas, &QgsMapCanvas::mapToolSet, this, [ = ]( QgsMapTool * tool, QgsMapTool * )
  {
    if ( tool != this )
      mPreviousTool = tool;
  } );
  mPreviousTool = canvas->mapTool();
}

void QgsDigitizingGuideMapTool::keyPressEvent( QKeyEvent *e )
{
  QgsMapTool::keyPressEvent( e );

  if ( e->key() == Qt::Key_Escape )
  {
    restorePreviousMapTool();
  }
}

void QgsDigitizingGuideMapTool::restorePreviousMapTool() const
{
  canvas()->setMapTool( mPreviousTool );
}


QgsDigitizingGuideMapToolDistanceToPoints::QgsDigitizingGuideMapToolDistanceToPoints( QgsMapCanvas *canvas )
  : QgsDigitizingGuideMapTool( canvas )
{
  mSnapIndicator.reset( new QgsSnapIndicator( canvas ) );
}


void QgsDigitizingGuideMapToolDistanceToPoints::deactivate()
{
  mDistances.clear();

  mSnapIndicator->setMatch( QgsPointLocator::Match() );
  mCircleRubberBand->reset( Qgis::GeometryType::Line );
  mCenterRubberBand->reset( Qgis::GeometryType::Point );

  QgsDigitizingGuideMapTool::deactivate();
}

void QgsDigitizingGuideMapToolDistanceToPoints::updateRubberband()
{
  if ( !mCircleRubberBand )
  {
    mCircleRubberBand.reset( new QgsRubberBand( mCanvas, Qgis::GeometryType::Line ) );
    QColor color = QgsSettingsRegistryCore::settingsDigitizingLineColor->value();
    mCircleRubberBand->setLineStyle( Qt::DotLine );
    mCircleRubberBand->setStrokeColor( color );
  }
  else
  {
    mCircleRubberBand->reset( Qgis::GeometryType::Line );
  }

  if ( !mCenterRubberBand )
  {
    mCenterRubberBand.reset( new QgsRubberBand( mCanvas, Qgis::GeometryType::Point ) );
    mCenterRubberBand->setIcon( QgsRubberBand::ICON_CROSS );
    //mCenterRubberBand->setWidth( QgsGuiUtils::scaleIconSize( 8 ) );
    mCenterRubberBand->setIconSize( QgsGuiUtils::scaleIconSize( 10 ) );
    mCenterRubberBand->setColor( QgsSettingsRegistryCore::settingsDigitizingLineColor->value() );
  }
  else
  {
    mCenterRubberBand->reset( Qgis::GeometryType::Point );
  }

  for ( const auto &point : std::as_const( mDistances ) )
  {
    mCenterRubberBand->addPoint( point.first );
    QgsCircle *c = new QgsCircle( QgsPoint( point.first ), point.second );
    QgsGeometry g = QgsGeometry( c->toCircularString() );
    mCircleRubberBand->addGeometry( g, QgsProject::instance()->digitizingGuideLayer()->crs() );
  }
}

QgsPoint QgsDigitizingGuideMapToolDistanceToPoints::intersectionSolution( const QgsMapMouseEvent *e ) const
{
  QgsPointXY p1, p2;
  QgsVertexId vertexId;
  QgsGeometryUtils::circleCircleIntersections( mDistances.first().first, mDistances.first().second, mDistances.constLast().first, mDistances.constLast().second, p1, p2 );
  QgsMultiPoint solutions( QVector<QgsPointXY>() << p1 << p2 );
  QgsPoint solution = QgsGeometryUtils::closestVertex( solutions, QgsPoint( e->mapPoint() ), vertexId );
  double distance = solution.distance( QgsPoint( e->mapPoint() ) );
  double tolerance = QgsTolerance::vertexSearchRadius( mCanvas->mapSettings() );
  if ( distance < tolerance )
  {
    return solution;
  }
  else
  {
    return QgsPoint();
  }
}


void QgsDigitizingGuideMapToolDistanceToPoints::canvasMoveEvent( QgsMapMouseEvent *e )
{
  e->snapPoint();

  if ( mDistances.count() < 2 )
  {
    mSnapIndicator->setMatch( e->mapPointMatch() );
  }
  else
  {
    QgsPoint solution = intersectionSolution( e );
    if ( !solution.isEmpty() )
    {
      QgsPointLocator::Match match( QgsPointLocator::Type::Vertex, nullptr, QgsFeatureId(), 0, solution );
      mSnapIndicator->setMatch( match );
    }
    else
    {
      mSnapIndicator->setMatch( QgsPointLocator::Match() );
    }
  }
}

void QgsDigitizingGuideMapToolDistanceToPoints::canvasReleaseEvent( QgsMapMouseEvent *e )
{
  if ( e->button() == Qt::MouseButton::RightButton )
  {
    mDistances.clear();
  }
  else
  {
    e->snapPoint();
    mSnapIndicator->setMatch( e->mapPointMatch() );

    if ( mDistances.count() < 2 )
    {
      QInputDialog *d = new QInputDialog;
      //d->setDialogTitle( tr( "Distance to point" ) );
      d->setLabelText( tr( "Distance to point [m]" ) );
      d->setInputMode( QInputDialog::DoubleInput );
      d->setDoubleDecimals( 3 );

      QgsPointXY pt = toLayerCoordinates( QgsProject::instance()->digitizingGuideLayer(), e->mapPoint() );
      mDistances.append( std::pair<QgsPointXY, double>( pt, 0 ) );

      connect( d, &QInputDialog::doubleValueChanged, this, [ = ]( double distance )
      {
        mDistances.last().second = distance;
        updateRubberband();
      } );

      if ( !d->exec() )
      {
        mDistances.removeLast();
      }
    }
    else
    {
      QgsPoint solution = intersectionSolution( e );
      if ( !solution.isEmpty() )
      {
        bool ok = false;
        QString title = QInputDialog::getText( nullptr, tr( "Add Point Giude" ), tr( "Guide Title" ),  QLineEdit::EchoMode::Normal, tr( "Distance to 2 points" ), &ok );
        if ( ok )
        {
          createPointDistanceToPointsGuide( solution, mDistances, title );
        }
        mDistances.clear();
        restorePreviousMapTool();
        return;
      }
    }
  }
  mSnapIndicator->setMatch( QgsPointLocator::Match() );
  updateRubberband();
}

void QgsDigitizingGuideMapToolDistanceToPoints::createPointDistanceToPointsGuide( const QgsPoint &guidePoint, const QList<std::pair<QgsPointXY, double> > &distances, const QString &title )
{
  QgsDigitizingGuideLayer *dl = QgsProject::instance()->digitizingGuideLayer();

  QList<QgsAnnotationItem *> details;
  for ( const auto &distance : distances )
  {
    details << dl->createDetailsPoint( QgsPoint( distance.first ) );

    QgsLineString *line = new QgsLineString( {guidePoint, distance.first} );
    details << dl->createDetailsLine( line );
    details << dl->createDetailsPointTextGuide( QString::number( distance.second, 'f', 3 ), line->centroid(), guidePoint.azimuth( QgsPoint( distance.first ) ) - 90 );
  }

  dl->addPointGuide( guidePoint, title, details );
}


QgsDigitizingGuideMapToolLineAbstract::QgsDigitizingGuideMapToolLineAbstract( const QString &defaultTitle, QgsMapCanvas *canvas )
  : QgsDigitizingGuideMapTool( canvas )
  , mDefaultTitle( defaultTitle )
{
  mSnapIndicator.reset( new QgsSnapIndicator( canvas ) );
}

void QgsDigitizingGuideMapToolLineAbstract::deactivate()
{
  deleteUserInputWidget();

  mSegment.reset();
  mSnapIndicator->setMatch( QgsPointLocator::Match() );
  mRubberBand->reset( Qgis::GeometryType::Line );

  QgsDigitizingGuideMapTool::deactivate();
}

void QgsDigitizingGuideMapToolLineAbstract::canvasMoveEvent( QgsMapMouseEvent *e )
{
  e->snapPoint();

  if ( !mSegment )
  {
    EdgesOnlyFilter filter;
    const QgsPointLocator::Match match = canvas()->snappingUtils()->snapToMap( e->snapPoint(), &filter );

    if ( match.isValid() )
    {
      mSnapIndicator->setMatch( match );
    }
    else
    {
      mSnapIndicator->setMatch( QgsPointLocator::Match() );
    }
  }
  else
  {
    QgsLineString *line = createLine( e->snapPoint() );
    updateRubberBand( line );
  }
}

void QgsDigitizingGuideMapToolLineAbstract::updateRubberBand( QgsLineString *line )
{
  if ( !mRubberBand )
  {
    mRubberBand.reset( new QgsRubberBand( mCanvas, Qgis::GeometryType::Line ) );
    QColor color = QgsSettingsRegistryCore::settingsDigitizingLineColor->value();
    mRubberBand->setLineStyle( Qt::DotLine );
    mRubberBand->setStrokeColor( color );
  }
  else
  {
    mRubberBand->reset( Qgis::GeometryType::Line );
  }
  mRubberBand->addGeometry( QgsGeometry( line ), QgsProject::instance()->digitizingGuideLayer()->crs() );
}

void QgsDigitizingGuideMapToolLineAbstract::canvasReleaseEvent( QgsMapMouseEvent *e )
{
  if ( !mSegment )
  {
    if ( e->button() == Qt::MouseButton::RightButton )
    {
      restorePreviousMapTool();
      return;
    }
    EdgesOnlyFilter filter;
    const QgsPointLocator::Match match = canvas()->snappingUtils()->snapToMap( e->mapPoint(), &filter );

    if ( match.isValid() )
    {
      QgsPointXY p1, p2;
      match.edgePoints( p1, p2 );
      mSegment = {p1, p2};

      createUserInputWidget();
    }
  }
  else
  {

    QgsLineString *line = createLine( e->mapPoint() );
    if ( line )
    {
      QgsDigitizingGuideLayer *dl = QgsProject::instance()->digitizingGuideLayer();
      dl->addLineGuide( line, mUserInputWidget->title() );
    }
    restorePreviousMapTool();
    return;
  }

  mSnapIndicator->setMatch( QgsPointLocator::Match() );
}

void QgsDigitizingGuideMapToolLineAbstract::createUserInputWidget()
{
  deleteUserInputWidget();

  mFloatingWidget = new QgsFloatingWidget( mCanvas );
  mFloatingWidget->setAnchorWidget( mCanvas );
  mFloatingWidget->setAnchorWidgetPoint( QgsFloatingWidget::AnchorPoint::TopRight );
  mFloatingWidget->setAnchorPoint( QgsFloatingWidget::AnchorPoint::TopRight );

  QFrame *f = new QFrame();
  f->setObjectName( QStringLiteral( "mUserInputContainer" ) );

  QPalette pal = mCanvas->window()->palette();
  pal.setBrush( mCanvas->window()->backgroundRole(), pal.window() );
  f->setPalette( pal );
  f->setAutoFillBackground( true );
  f->setFrameShape( QFrame::StyledPanel );
  f->setFrameShadow( QFrame::Plain );

  QBoxLayout *topLayout = new QBoxLayout( QBoxLayout::TopToBottom );
  topLayout->setContentsMargins( 0, 0, 0, 0 );
  topLayout->addWidget( f );
  mFloatingWidget->setLayout( topLayout );
  topLayout->setSizeConstraint( QLayout::SetFixedSize );

  mUserInputWidget = new QgsDigitizingGuideToolUserInputWidget( mDefaultTitle, mHasOffset, mFloatingWidget );
  mUserInputWidget->setFocus( Qt::TabFocusReason );

  QHBoxLayout *containerLayout = new QHBoxLayout();
  containerLayout->setContentsMargins( 0, 0, 0, 0 );
  containerLayout->setSizeConstraint( QLayout::SetFixedSize );
  containerLayout->addWidget( mUserInputWidget );
  f->setLayout( containerLayout );
  mFloatingWidget->setSizePolicy( QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding );
  mFloatingWidget->show();

  connect( mUserInputWidget, &QgsDigitizingGuideToolUserInputWidget::offsetChanged, this, &QgsDigitizingGuideMapToolLineAbstract::offsetChanged );
  connect( mUserInputWidget, &QgsDigitizingGuideToolUserInputWidget::editingFinished, this, &QgsDigitizingGuideMapToolLineAbstract::validateFromUserWidget );
  connect( mUserInputWidget, &QgsDigitizingGuideToolUserInputWidget::editingCanceled, this, &QgsDigitizingGuideMapToolLineAbstract::restorePreviousMapTool );
}

void QgsDigitizingGuideMapToolLineAbstract::deleteUserInputWidget()
{
  if ( mUserInputWidget )
  {
    disconnect( mUserInputWidget, &QgsDigitizingGuideToolUserInputWidget::offsetChanged, this, &QgsDigitizingGuideMapToolLineAbstract::offsetChanged );
    disconnect( mUserInputWidget, &QgsDigitizingGuideToolUserInputWidget::editingFinished, this, &QgsDigitizingGuideMapToolLineAbstract::validateFromUserWidget );
    disconnect( mUserInputWidget, &QgsDigitizingGuideToolUserInputWidget::editingCanceled, this, &QgsDigitizingGuideMapToolLineAbstract::restorePreviousMapTool );
    mUserInputWidget->releaseKeyboard();
  }
  mUserInputWidget = nullptr;
  if ( mFloatingWidget )
  {
    mFloatingWidget->deleteLater();
    mFloatingWidget = nullptr;
  }
}

void QgsDigitizingGuideMapToolLineAbstract::offsetChanged( double offset )
{
  QgsLineString *line = createLine( QgsPoint(), offset );
  updateRubberBand( line );
}

void QgsDigitizingGuideMapToolLineAbstract::validateFromUserWidget( double offset )
{
  QgsLineString *line = createLine( QgsPoint(), offset );
  if ( line )
  {
    QgsDigitizingGuideLayer *dl = QgsProject::instance()->digitizingGuideLayer();
    dl->addLineGuide( line, mUserInputWidget->title() );
  }
  restorePreviousMapTool();
}

QgsDigitizingGuideMapToolLineExtension::QgsDigitizingGuideMapToolLineExtension( QgsMapCanvas *canvas )
  : QgsDigitizingGuideMapToolLineAbstract( tr( "Line Extension" ), canvas )
{
}

QgsLineString *QgsDigitizingGuideMapToolLineExtension::createLine( const QgsPointXY &point, double offset )
{
  Q_UNUSED( offset )

  if ( !mSegment )
    return nullptr;

  double d1 = mSegment->first.sqrDist( point );
  double d2 = mSegment->second.sqrDist( point );

  const QgsPointXY &startPoint = d1 < d2 ? mSegment->first : mSegment->second;

  const QgsPointXY endPoint = QgsGeometryUtils::perpendicularSegment( QgsPoint( point ), QgsPoint( mSegment->first ), QgsPoint( mSegment->second ) ).endPoint();

  return new QgsLineString( {startPoint, endPoint} );
}


QgsDigitizingGuideMapToolLineParallel::QgsDigitizingGuideMapToolLineParallel( QgsMapCanvas *canvas )
  : QgsDigitizingGuideMapToolLineAbstract( tr( "Parallel" ), canvas )
{
  mHasOffset = true;
}

QgsLineString *QgsDigitizingGuideMapToolLineParallel::createLine( const QgsPointXY &point, double offset )
{
  if ( !mSegment )
    return nullptr;

  const QgsPointXY &s1 = mSegment->first;
  const QgsPointXY &s2 = mSegment->second;

  const double nx = s2.y() - s1.y();
  const double ny = -( s2.x() - s1.x() );
  const double t = ( point.x() * ny - point.y() * nx - s1.x() * ny + s1.y() * nx ) / ( ( s2.x() - s1.x() ) * ny - ( s2.y() - s1.y() ) * nx );

  QgsPoint p1;
  QgsPoint p2;

  if ( offset == 0 )
  {
    if ( t > 1 )
    {
      p1 = QgsPoint( s1 );
      p2 = QgsPoint( s1.x() + ( s2.x() - s1.x() ) * t, s1.y() + ( s2.y() - s1.y() ) * t );
    }
    else if ( t < 0 )
    {
      p1 = QgsPoint( s1.x() + ( s2.x() - s1.x() ) * t, s1.y() + ( s2.y() - s1.y() ) * t );
      p2 = QgsPoint( s2 );
    }
    else
    {
      p1 = QgsPoint( s1 );
      p2 = QgsPoint( s2 );
    }
    const QgsPoint pointPoint = QgsPoint( point );

    offset = QgsGeometryUtils::leftOfLine( pointPoint, p2, p1 ) * QgsGeometryUtils::distToInfiniteLine( pointPoint, p1, p2 );
  }
  else
  {
    p1 = QgsPoint( s1 );
    p2 = QgsPoint( s2 );
  }

  QgsPolylineXY newLine = QgsGeometry::fromPolylineXY( {p1, p2} ).offsetCurve( offset, 8, Qgis::JoinStyle::Miter, 2 ).asPolyline();

  if ( mUserInputWidget && !point.isEmpty() )
  {
    mUserInputWidget->setOffset( offset );
  }

  return new QgsLineString( {newLine.first(), newLine.last()} );
}

QgsDigitizingGuideMapToolLinePerpendicular::QgsDigitizingGuideMapToolLinePerpendicular( QgsMapCanvas *canvas )
  : QgsDigitizingGuideMapToolLineAbstract( tr( "Perpendicular" ), canvas )
{
}

QgsLineString *QgsDigitizingGuideMapToolLinePerpendicular::createLine( const QgsPointXY &point, double offset )
{
  Q_UNUSED( offset )

  if ( !mSegment )
    return nullptr;

  QgsLineString *line = new QgsLineString(QgsGeometryUtils::perpendicularSegment( QgsPoint( point ), QgsPoint( mSegment->first ), QgsPoint( mSegment->second ) ) );

  return line;
}

///@cond PRIVATE

QgsDigitizingGuideToolUserInputWidget::QgsDigitizingGuideToolUserInputWidget( const QString &title, bool offset, QWidget *parent )
  : QWidget( parent )
{
  setupUi( this );

  mTitleLineEdit->setText( title );
  mOffsetLabel->setVisible( offset );
  mOffsetSpinBox->setVisible( offset );

  if ( offset )
  {
    mOffsetSpinBox->setDecimals( 6 );
    mOffsetSpinBox->setMinimum( -1e8 );
    mOffsetSpinBox->setMaximum( 1e8 );
    mOffsetSpinBox->installEventFilter( this );
    connect( mOffsetSpinBox, static_cast < void ( QDoubleSpinBox::* )( double ) > ( &QDoubleSpinBox::valueChanged ), this, &QgsDigitizingGuideToolUserInputWidget::offsetChanged );
    setFocusProxy( mOffsetSpinBox );
  }
}

QString QgsDigitizingGuideToolUserInputWidget::title() const
{
  return mTitleLineEdit->text();
}

void QgsDigitizingGuideToolUserInputWidget::setOffset( double offset )
{
  whileBlocking( mOffsetSpinBox )->setValue( offset );
}

double QgsDigitizingGuideToolUserInputWidget::offset() const
{
  return mOffsetSpinBox->value();
}

bool QgsDigitizingGuideToolUserInputWidget::eventFilter( QObject *obj, QEvent *ev )
{
  if ( ( obj == mOffsetSpinBox || obj == mTitleLineEdit ) && ev->type() == QEvent::KeyPress )
  {
    QKeyEvent *event = static_cast<QKeyEvent *>( ev );
    if ( event->key() == Qt::Key_Escape )
    {
      emit editingCanceled();
      return true;
    }
    if ( obj == mOffsetSpinBox && ( event->key() == Qt::Key_Enter || event->key() == Qt::Key_Return ) )
    {
      emit editingFinished( offset() );
      return true;
    }
  }

  return false;
}

///@endcond PRIVATE
