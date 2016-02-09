/***************************************************************************
                                 qgsmeasure.h
                               ------------------
        begin                : March 2005
        copyright            : (C) 2005 by Radim Blazek
        email                : blazek@itc.it
 ***************************************************************************/
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgisapp.h"
#include "qgsmeasuredialog.h"
#include "qgsmeasuretool.h"

#include "qgslogger.h"
#include "qgscontexthelp.h"
#include "qgsdistancearea.h"
#include "qgsmapcanvas.h"
#include "qgsmaprenderer.h"
#include "qgsproject.h"
#include "qgscoordinatereferencesystem.h"
#include "qgsunittypes.h"

#include <QCloseEvent>
#include <QLocale>
#include <QSettings>
#include <QPushButton>


QgsMeasureDialog::QgsMeasureDialog( QgsMeasureTool* tool, Qt::WindowFlags f )
    : QDialog( tool->canvas()->topLevelWidget(), f ), mTool( tool )
{
  setupUi( this );

  QPushButton *nb = new QPushButton( tr( "&New" ) );
  buttonBox->addButton( nb, QDialogButtonBox::ActionRole );
  connect( nb, SIGNAL( clicked() ), this, SLOT( restart() ) );

  // Add a configuration button
  QPushButton* cb = new QPushButton( tr( "&Configuration" ) );
  buttonBox->addButton( cb, QDialogButtonBox::ActionRole );
  connect( cb, SIGNAL( clicked() ), this, SLOT( openConfigTab() ) );

  mMeasureArea = tool->measureArea();
  mTotal = 0.;

  mUnitsCombo->addItem( QgsUnitTypes::toString( QGis::Meters ), QGis::Meters );
  mUnitsCombo->addItem( QgsUnitTypes::toString( QGis::Feet ), QGis::Feet );
  mUnitsCombo->addItem( QgsUnitTypes::toString( QGis::Degrees ), QGis::Degrees );
  mUnitsCombo->addItem( QgsUnitTypes::toString( QGis::NauticalMiles ), QGis::NauticalMiles );
  QSettings settings;
  QString units = settings.value( "/qgis/measure/displayunits", QgsUnitTypes::encodeUnit( QGis::Meters ) ).toString();
  mUnitsCombo->setCurrentIndex( mUnitsCombo->findData( QgsUnitTypes::decodeDistanceUnit( units ) ) );

  updateSettings();

  connect( mUnitsCombo, SIGNAL( currentIndexChanged( int ) ), this, SLOT( unitsChanged( int ) ) );
  connect( buttonBox, SIGNAL( rejected() ), this, SLOT( reject() ) );

  groupBox->setCollapsed( true );
}

void QgsMeasureDialog::openConfigTab()
{
  QgisApp::instance()->showOptionsDialog( this, "mOptionsPageMapTools" );
}

void QgsMeasureDialog::updateSettings()
{
  QSettings settings;

  mDecimalPlaces = settings.value( "/qgis/measure/decimalplaces", "3" ).toInt();
  mCanvasUnits = mTool->canvas()->mapUnits();
  // Configure QgsDistanceArea
  mDisplayUnits = static_cast< QGis::UnitType >( mUnitsCombo->itemData( mUnitsCombo->currentIndex() ).toInt() );
  mDa.setSourceCrs( mTool->canvas()->mapSettings().destinationCrs().srsid() );
  mDa.setEllipsoid( QgsProject::instance()->readEntry( "Measure", "/Ellipsoid", GEO_NONE ) );
  // Only use ellipsoidal calculation when project wide transformation is enabled.
  if ( mTool->canvas()->mapSettings().hasCrsTransformEnabled() )
  {
    mDa.setEllipsoidalMode( true );
  }
  else
  {
    mDa.setEllipsoidalMode( false );
  }

  QgsDebugMsg( "****************" );
  QgsDebugMsg( QString( "Ellipsoid ID : %1" ).arg( mDa.ellipsoid() ) );
  QgsDebugMsg( QString( "Ellipsoidal  : %1" ).arg( mDa.ellipsoidalEnabled() ? "true" : "false" ) );
  QgsDebugMsg( QString( "Decimalplaces: %1" ).arg( mDecimalPlaces ) );
  QgsDebugMsg( QString( "Display units: %1" ).arg( QgsUnitTypes::encodeUnit( mDisplayUnits ) ) );
  QgsDebugMsg( QString( "Canvas units : %1" ).arg( QgsUnitTypes::encodeUnit( mCanvasUnits ) ) );

  mTotal = 0;
  updateUi();
}

void QgsMeasureDialog::unitsChanged( int index )
{
  mDisplayUnits = static_cast< QGis::UnitType >( mUnitsCombo->itemData( index ).toInt() );
  mTable->clear();
  mTotal = 0.;
  updateUi();
}

void QgsMeasureDialog::restart()
{
  mTool->restart();

  mTable->clear();
  mTotal = 0.;
  updateUi();
}


void QgsMeasureDialog::mouseMove( QgsPoint &point )
{
  mLastMousePoint = point;
  // show current distance/area while moving the point
  // by creating a temporary copy of point array
  // and adding moving point at the end
  if ( mMeasureArea && mTool->points().size() >= 2 )
  {
    QList<QgsPoint> tmpPoints = mTool->points();
    tmpPoints.append( point );
    double area = mDa.measurePolygon( tmpPoints );
    editTotal->setText( formatArea( area ) );
  }
  else if ( !mMeasureArea && mTool->points().size() >= 1 )
  {
    QgsPoint p1( mTool->points().last() ), p2( point );
    double d = mDa.measureLine( p1, p2 );

    editTotal->setText( formatDistance( mTotal + d ) );

    d = convertLength( d, mDisplayUnits );

    // Set moving
    QTreeWidgetItem *item = mTable->topLevelItem( mTable->topLevelItemCount() - 1 );
    if ( item )
    {
      item->setText( 0, QLocale::system().toString( d, 'f', mDecimalPlaces ) );
      QgsDebugMsg( QString( "Final result is %1" ).arg( item->text( 0 ) ) );
    }
  }
}

void QgsMeasureDialog::addPoint( QgsPoint &p )
{
  Q_UNUSED( p );

  int numPoints = mTool->points().size();
  if ( mMeasureArea && numPoints > 2 )
  {
    double area = mDa.measurePolygon( mTool->points() );
    editTotal->setText( formatArea( area ) );
  }
  else if ( !mMeasureArea && numPoints >= 1 )
  {
    if ( !mTool->done() )
    {
      QTreeWidgetItem * item = new QTreeWidgetItem( QStringList( QLocale::system().toString( 0.0, 'f', mDecimalPlaces ) ) );
      item->setTextAlignment( 0, Qt::AlignRight );
      mTable->addTopLevelItem( item );
      mTable->scrollToItem( item );
    }
    if ( numPoints > 1 )
    {
      mTotal = mDa.measureLine( mTool->points() );
      editTotal->setText( formatDistance( mTotal ) );
    }
  }
  QgsDebugMsg( "Exiting" );
}

void QgsMeasureDialog::removeLastPoint()
{
  int numPoints = mTool->points().size();
  if ( mMeasureArea )
  {
    if ( numPoints > 1 )
    {
      QList<QgsPoint> tmpPoints = mTool->points();
      tmpPoints.append( mLastMousePoint );
      double area = mDa.measurePolygon( tmpPoints );
      editTotal->setText( formatArea( area ) );
    }
    else
    {
      editTotal->setText( formatArea( 0 ) );
    }
  }
  else if ( !mMeasureArea && numPoints >= 1 )
  {
    //remove final row
    delete mTable->takeTopLevelItem( mTable->topLevelItemCount() - 1 );

    QgsPoint p1( mTool->points().last() );
    double d = mDa.measureLine( p1, mLastMousePoint );

    mTotal = mDa.measureLine( mTool->points() );
    editTotal->setText( formatDistance( mTotal + d ) );

    d = convertLength( d, mDisplayUnits );

    QTreeWidgetItem *item = mTable->topLevelItem( mTable->topLevelItemCount() - 1 );
    item->setText( 0, QLocale::system().toString( d, 'f', mDecimalPlaces ) );
  }
}

void QgsMeasureDialog::closeEvent( QCloseEvent *e )
{
  reject();
  e->accept();
}

void QgsMeasureDialog::restorePosition()
{
  QSettings settings;
  restoreGeometry( settings.value( "/Windows/Measure/geometry" ).toByteArray() );
  int wh;
  if ( mMeasureArea )
    wh = settings.value( "/Windows/Measure/hNoTable", 70 ).toInt();
  else
    wh = settings.value( "/Windows/Measure/h", 200 ).toInt();
  resize( width(), wh );
  updateUi();
}

void QgsMeasureDialog::saveWindowLocation()
{
  QSettings settings;
  settings.setValue( "/Windows/Measure/geometry", saveGeometry() );
  const QString &key = mMeasureArea ? "/Windows/Measure/hNoTable" : "/Windows/Measure/h";
  settings.setValue( key, height() );
}

QString QgsMeasureDialog::formatDistance( double distance, bool convertUnits )
{
  QSettings settings;
  bool baseUnit = settings.value( "/qgis/measure/keepbaseunit", false ).toBool();

  if ( convertUnits )
    distance = convertLength( distance, mDisplayUnits );
  return QgsDistanceArea::textUnit( distance, mDecimalPlaces, mDisplayUnits, false, baseUnit );
}

QString QgsMeasureDialog::formatArea( double area )
{
  QSettings settings;
  bool baseUnit = settings.value( "/qgis/measure/keepbaseunit", false ).toBool();

  QGis::UnitType newDisplayUnits;
  convertMeasurement( area, newDisplayUnits, true );
  return QgsDistanceArea::textUnit( area, mDecimalPlaces, newDisplayUnits, true, baseUnit );
}

void QgsMeasureDialog::updateUi()
{
  // Set tooltip to indicate how we calculate measurments
  QString toolTip = tr( "The calculations are based on:" );

  bool forceCartesian = false;
  bool convertToDisplayUnits = true;

  if ( mTool->canvas()->mapSettings().destinationCrs().mapUnits() == QGis::Degrees
       && mDisplayUnits == QGis::Degrees )
  {
    //both source and destination units are degrees
    toolTip += "<br> * " + tr( "Both project CRS (%1) and measured length are in degrees, so measurement is calculated using cartesian calculations in degrees." ).arg(
                 mTool->canvas()->mapSettings().destinationCrs().description() );
    forceCartesian = true;
    convertToDisplayUnits = false; //not required since we will be measuring in degrees
  }
  else
  {
    QGis::UnitType resultUnit = QGis::UnknownUnit;
    if ( ! mTool->canvas()->hasCrsTransformEnabled() )
    {
      resultUnit =  mTool->canvas()->mapSettings().destinationCrs().mapUnits();
      toolTip += "<br> * " + tr( "Project CRS transformation is turned off." ) + ' ';
      toolTip += tr( "Distance is calculated in %1, based on project CRS (%2)." ).arg( QgsUnitTypes::toString( resultUnit ),
                 mTool->canvas()->mapSettings().destinationCrs().description() );
      toolTip += "<br> * " + tr( "Ellipsoidal calculation is not possible with CRS transformation disabled." );
      setWindowTitle( tr( "Measure (OTF off)" ) );
    }
    else
    {
      if ( mDa.willUseEllipsoid() )
      {
        resultUnit = QGis::Meters;
        toolTip += "<br> * " + tr( "Project CRS transformation is turned on and ellipsoidal calculation is selected." ) + ' ';
        toolTip += "<br> * " + tr( "The coordinates are transformed to the chosen ellipsoid (%1), and the measurement is calculated in %2." ).arg( mDa.ellipsoid(),
                   QgsUnitTypes::toString( resultUnit ) );
      }
      else
      {
        resultUnit = mTool->canvas()->mapSettings().destinationCrs().mapUnits();
        toolTip += "<br> * " + tr( "Project CRS transformation is turned on but ellipsoidal calculation is not selected." ) + ' ';
        toolTip += tr( "Distance is calculated in %1, based on project CRS (%2)." ).arg( QgsUnitTypes::toString( resultUnit ),
                   mTool->canvas()->mapSettings().destinationCrs().description() );
      }
      setWindowTitle( tr( "Measure (OTF on)" ) );
    }

    if ( QgsUnitTypes::unitType( resultUnit ) == QgsUnitTypes::Geographic &&
         QgsUnitTypes::unitType( mDisplayUnits ) == QgsUnitTypes::Standard )
    {
      toolTip += "<br> * Distance is roughly converted to meters by using scale at equator (1 degree = 111319.49 meters).";
      resultUnit = QGis::Meters;
    }
    else if ( QgsUnitTypes::unitType( resultUnit ) == QgsUnitTypes::Standard &&
              QgsUnitTypes::unitType( mDisplayUnits ) == QgsUnitTypes::Geographic )
    {
      toolTip += "<br> * Distance is roughly converted to degrees by using scale at equator (1 degree = 111319.49 meters).";
      resultUnit = QGis::Degrees;
    }

    if ( resultUnit != mDisplayUnits )
    {
      if ( QgsUnitTypes::unitType( resultUnit ) == QgsUnitTypes::Standard &&
           QgsUnitTypes::unitType( mDisplayUnits ) == QgsUnitTypes::Standard )
      {
        // only shown if both conditions are true:
        // - the display unit is a standard distance measurement (eg feet)
        // - either the canvas units is also a standard distance OR we are using an ellipsoid (in which case the
        //   value will be in meters)
        toolTip += "<br> * " + tr( "The value is converted from %1 to %2." ).arg( QgsUnitTypes::toString( resultUnit ),
                   QgsUnitTypes::toString( mDisplayUnits ) );
      }
      else
      {
        //should not be possible!
      }
    }
  }

  editTotal->setToolTip( toolTip );
  mTable->setToolTip( toolTip );
  mNotesLabel->setText( toolTip );

  mUnitsCombo->setCurrentIndex( mUnitsCombo->findData( mDisplayUnits ) );
  mTable->setHeaderLabels( QStringList( tr( "Segments [%1]" ).arg( QgsUnitTypes::toString( mDisplayUnits ) ) ) );

  if ( mMeasureArea )
  {
    double area = 0.0;
    if ( mTool->points().size() > 1 )
    {
      area = mDa.measurePolygon( mTool->points() );
    }
    mTable->hide(); // Hide the table, only show summary.
    editTotal->setText( formatArea( area ) );
  }
  else
  {
    QList<QgsPoint>::const_iterator it;
    bool b = true; // first point

    QgsPoint p1, p2;
    mTotal = 0;
    for ( it = mTool->points().constBegin(); it != mTool->points().constEnd(); ++it )
    {
      p2 = *it;
      if ( !b )
      {
        double d = -1;
        if ( forceCartesian )
        {
          //cartesian calculation forced
          d = sqrt( p2.sqrDist( p1 ) );
          mTotal += d;
        }
        else
        {
          d = mDa.measureLine( p1, p2 );
          d = convertLength( d, mDisplayUnits );
        }

        QTreeWidgetItem *item = new QTreeWidgetItem( QStringList( QLocale::system().toString( d, 'f', mDecimalPlaces ) ) );
        item->setTextAlignment( 0, Qt::AlignRight );
        mTable->addTopLevelItem( item );
        mTable->scrollToItem( item );
      }
      p1 = p2;
      b = false;
    }

    if ( !forceCartesian )
      mTotal = mDa.measureLine( mTool->points() );
    mTable->show(); // Show the table with items
    editTotal->setText( formatDistance( mTotal, convertToDisplayUnits ) );
  }
}

void QgsMeasureDialog::convertMeasurement( double &measure, QGis::UnitType &u, bool isArea )
{
  // Helper for converting between units
  // The parameter &u is out only...

  // Get the canvas units
  QGis::UnitType myUnits = mCanvasUnits;

  QgsDebugMsg( QString( "Preferred display units are %1" ).arg( QgsUnitTypes::encodeUnit( mDisplayUnits ) ) );

  mDa.convertMeasurement( measure, myUnits, mDisplayUnits, isArea );
  u = myUnits;
}

double QgsMeasureDialog::convertLength( double length, QGis::UnitType toUnit )
{
  return mDa.convertLengthMeasurement( length, toUnit );
}


void QgsMeasureDialog::reject()
{
  saveWindowLocation();
  restart();
  QDialog::close();
}
