/***************************************************************************
                          qgselevationprofiletoolmeasure.cpp
                          ---------------
    begin                : April 2022
    copyright            : (C) 2022 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "qgselevationprofiletoolmeasure.h"
#include "qgselevationprofilecanvas.h"
#include "qgsapplication.h"
#include "qgsplotmouseevent.h"
#include "qgsguiutils.h"
#include "qgsclipper.h"
#include "qgsgui.h"
#include "qgsproject.h"

#include <QGraphicsLineItem>
#include <QGridLayout>
#include <QLabel>

//
// QgsProfileMeasureResultsDialog
//

QgsProfileMeasureResultsDialog::QgsProfileMeasureResultsDialog()
  : QDialog( nullptr, Qt::Tool )
{
  setWindowTitle( tr( "Profile Distance" ) );
  setObjectName( QStringLiteral( "QgsProfileMeasureResultsDialog" ) );

  QGridLayout *grid = new QGridLayout();
  grid->addWidget( new QLabel( tr( "Total Length" ) ), 0, 0 );
  mTotalLabel = new QLabel();
  mTotalLabel->setTextInteractionFlags( Qt::TextBrowserInteraction );
  grid->addWidget( mTotalLabel, 0, 1 );
  grid->addWidget( new QLabel( tr( "Δ Distance" ) ), 1, 0 );
  mDistanceLabel = new QLabel();
  mDistanceLabel->setTextInteractionFlags( Qt::TextBrowserInteraction );
  grid->addWidget( mDistanceLabel, 1, 1 );
  grid->addWidget( new QLabel( tr( "Δ Elevation" ) ), 2, 0 );
  mElevationLabel = new QLabel();
  mElevationLabel->setTextInteractionFlags( Qt::TextBrowserInteraction );
  grid->addWidget( mElevationLabel, 2, 1 );

  setLayout( grid );

  QgsGui::enableAutoGeometryRestore( this );
  installEventFilter( this );
}

void QgsProfileMeasureResultsDialog::setCrs( const QgsCoordinateReferenceSystem &crs )
{
  mCrs = crs;
}

bool QgsProfileMeasureResultsDialog::eventFilter( QObject *object, QEvent *event )
{
  if ( object == this && ( event->type() == QEvent::Close ||
                           event->type() == QEvent::Destroy ||
                           event->type() == QEvent::Hide ) )
  {
    emit closed();
  }
  return QDialog::eventFilter( object, event );
}

void QgsProfileMeasureResultsDialog::setMeasures( double total, double distance, double elevation )
{
  // these values are inherently dimensionless for now. We need proper support for vertical CRS
  // before we can determine what units the elevation are in!
  mTotalLabel->setText( QLocale().toString( total ) );
  mElevationLabel->setText( QLocale().toString( elevation ) );

  // the distance delta HAS units!
  const QgsSettings settings;
  const bool baseUnit = settings.value( QStringLiteral( "qgis/measure/keepbaseunit" ), true ).toBool();

  QgsUnitTypes::DistanceUnit distanceUnit = mCrs.mapUnits();
  const QgsUnitTypes::DistanceUnit projectUnit = QgsProject::instance()->distanceUnits();

  if ( projectUnit != distanceUnit
       && QgsUnitTypes::unitType( distanceUnit ) == QgsUnitTypes::DistanceUnitType::Standard
       && QgsUnitTypes::unitType( projectUnit ) == QgsUnitTypes::DistanceUnitType::Standard )
  {
    // convert distance to desired project units. We can only do this if neither the crs units nor the project units are geographic!
    distance *= QgsUnitTypes::fromUnitToUnitFactor( distanceUnit, projectUnit );
    distanceUnit = projectUnit;
  }

  int decimals = settings.value( QStringLiteral( "qgis/measure/decimalplaces" ), 3 ).toInt();
  if ( distanceUnit == QgsUnitTypes::DistanceDegrees && distance < 1 )
  {
    // special handling for degrees - because we can't use smaller units (eg m->mm), we need to make sure there's
    // enough decimal places to show a usable measurement value
    const int minPlaces = std::round( std::log10( 1.0 / distance ) ) + 1;
    decimals = std::max( decimals, minPlaces );
  }

  mDistanceLabel->setText( QgsUnitTypes::formatDistance( distance, decimals, distanceUnit, baseUnit ) );
}

void QgsProfileMeasureResultsDialog::clear()
{
  mTotalLabel->clear();
  mDistanceLabel->clear();
  mElevationLabel->clear();
}

//
// QgsElevationProfileToolMeasure
//

QgsElevationProfileToolMeasure::QgsElevationProfileToolMeasure( QgsElevationProfileCanvas *canvas )
  : QgsPlotTool( canvas, QObject::tr( "Measure Tool" ) )
  , mElevationCanvas( canvas )
{
  setCursor( Qt::CrossCursor );

  mRubberBand = new QGraphicsLineItem();
  mRubberBand->setZValue( 1000 );

  QPen pen;
  pen.setWidthF( QgsGuiUtils::scaleIconSize( 2 ) );
  pen.setCosmetic( true );
  QgsSettings settings;
  const int red = settings.value( QStringLiteral( "qgis/default_measure_color_red" ), 222 ).toInt();
  const int green = settings.value( QStringLiteral( "qgis/default_measure_color_green" ), 155 ).toInt();
  const int blue = settings.value( QStringLiteral( "qgis/default_measure_color_blue" ), 67 ).toInt();
  pen.setColor( QColor( red, green, blue, 100 ) );
  mRubberBand->setPen( pen );

  mRubberBand->hide();
  mElevationCanvas->scene()->addItem( mRubberBand );

  connect( mElevationCanvas, &QgsElevationProfileCanvas::plotAreaChanged, this, &QgsElevationProfileToolMeasure::plotAreaChanged );

  mDialog = new QgsProfileMeasureResultsDialog();

  connect( this, &QgsElevationProfileToolMeasure::cleared, mDialog, &QDialog::hide );
  connect( this, &QgsElevationProfileToolMeasure::measureChanged, mDialog, [ = ]( double totalDistance, double deltaCurve, double deltaElevation )
  {
    mDialog->setCrs( mElevationCanvas->crs() );
    mDialog->setMeasures( totalDistance, deltaCurve, deltaElevation );
    mDialog->show();
  } );
  connect( mDialog, &QgsProfileMeasureResultsDialog::closed, this, [ = ]
  {
    mMeasureInProgress = false;
    mRubberBand->hide();
    emit cleared();
  } );
}

QgsElevationProfileToolMeasure::~QgsElevationProfileToolMeasure()
{
  mDialog->deleteLater();
  mDialog = nullptr;
}

void QgsElevationProfileToolMeasure::plotAreaChanged()
{
  if ( mRubberBand->isVisible() )
  {
    updateRubberBand();
  }
}

void QgsElevationProfileToolMeasure::plotMoveEvent( QgsPlotMouseEvent *event )
{
  event->ignore();
  if ( !mMeasureInProgress )
    return;

  const QRectF plotArea = mElevationCanvas->plotArea();
  const QPointF snappedPoint = event->snappedPoint().toQPointF();
  mEndPoint = mElevationCanvas->canvasPointToPlotPoint( constrainPointToRect( snappedPoint, plotArea ) );

  updateRubberBand();

  const double deltaCurveDistance = mEndPoint.distance() - mStartPoint.distance();
  const double deltaElevation = mEndPoint.elevation() - mStartPoint.elevation();
  const double totalDistance = std::sqrt( std::pow( deltaCurveDistance, 2 ) + std::pow( deltaElevation, 2 ) );
  emit measureChanged( totalDistance, deltaCurveDistance, deltaElevation );
}

void QgsElevationProfileToolMeasure::plotPressEvent( QgsPlotMouseEvent *event )
{
  if ( event->button() != Qt::LeftButton )
  {
    event->ignore();
    return;
  }

  if ( mMeasureInProgress )
  {
    mMeasureInProgress = false;
    const QPointF snappedPoint = event->snappedPoint().toQPointF();
    const QRectF plotArea = mElevationCanvas->plotArea();
    mEndPoint = mElevationCanvas->canvasPointToPlotPoint( constrainPointToRect( snappedPoint, plotArea ) );
    updateRubberBand();
  }
  else
  {
    const QRectF plotArea = mElevationCanvas->plotArea();
    if ( !plotArea.contains( event->pos() ) )
    {
      event->ignore();
      return;
    }

    const QPointF snappedPoint = event->snappedPoint().toQPointF();

    mStartPoint = mElevationCanvas->canvasPointToPlotPoint( snappedPoint );
    mEndPoint = mStartPoint;
    updateRubberBand();
    mRubberBand->show();

    mMeasureInProgress = true;
    emit measureChanged( 0, 0, 0 );
  }
}

void QgsElevationProfileToolMeasure::plotReleaseEvent( QgsPlotMouseEvent *event )
{
  if ( event->button() == Qt::RightButton && mRubberBand->isVisible() )
  {
    event->ignore();
    mMeasureInProgress = false;
    mRubberBand->hide();
    emit cleared();
  }
  else if ( event->button() != Qt::LeftButton )
  {
    event->ignore();
  }
}

void QgsElevationProfileToolMeasure::updateRubberBand()
{
  const QgsDoubleRange distanceRange = mElevationCanvas->visibleDistanceRange();
  const QgsDoubleRange elevationRange = mElevationCanvas->visibleElevationRange();

  double distance1 = mStartPoint.distance();
  double elevation1 = mStartPoint.elevation();
  double distance2 = mEndPoint.distance();
  double elevation2 = mEndPoint.elevation();
  QgsClipper::clipLineSegment( distanceRange.lower(), distanceRange.upper(), elevationRange.lower(), elevationRange.upper(),
                               distance1, elevation1, distance2, elevation2 );

  const QgsPointXY p1 = mElevationCanvas->plotPointToCanvasPoint( QgsProfilePoint( distance1, elevation1 ) );
  const QgsPointXY p2 = mElevationCanvas->plotPointToCanvasPoint( QgsProfilePoint( distance2, elevation2 ) );

  mRubberBand->setLine( QLineF( p1.x(), p1.y(), p2.x(), p2.y() ) );
}
