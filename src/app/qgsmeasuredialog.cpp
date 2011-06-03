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
/* $Id$ */

#include "qgsmeasuredialog.h"
#include "qgsmeasuretool.h"

#include "qgslogger.h"
#include "qgscontexthelp.h"
#include "qgsdistancearea.h"
#include "qgsmapcanvas.h"
#include "qgsmaprenderer.h"
#include "qgscoordinatereferencesystem.h"

#include <QCloseEvent>
#include <QLocale>
#include <QSettings>
#include <QPushButton>


QgsMeasureDialog::QgsMeasureDialog( QgsMeasureTool* tool, Qt::WFlags f )
    : QDialog( tool->canvas()->topLevelWidget(), f ), mTool( tool )
{
  setupUi( this );

  QPushButton *nb = new QPushButton( tr( "&New" ) );
  buttonBox->addButton( nb, QDialogButtonBox::ActionRole );
  connect( nb, SIGNAL( clicked() ), this, SLOT( restart() ) );

  mMeasureArea = tool->measureArea();
  mTotal = 0.;

  // Set one cell row where to update current distance
  // If measuring area, the table doesn't get shown
  QTreeWidgetItem* item = new QTreeWidgetItem( QStringList( QString::number( 0, 'f', 1 ) ) );
  item->setTextAlignment( 0, Qt::AlignRight );
  mTable->addTopLevelItem( item );

  //mTable->setHeaderLabels(QStringList() << tr("Segments (in meters)") << tr("Total") << tr("Azimuth") );

  QSettings settings;
  int s = settings.value( "/qgis/measure/projectionEnabled", "2" ).toInt();
  if ( s == 2 )
    mcbProjectionEnabled->setCheckState( Qt::Checked );
  else
    mcbProjectionEnabled->setCheckState( Qt::Unchecked );

  connect( mcbProjectionEnabled, SIGNAL( stateChanged( int ) ),
           this, SLOT( changeProjectionEnabledState() ) );

  updateUi();
}


void QgsMeasureDialog::restart()
{
  mTool->restart();

  // Set one cell row where to update current distance
  // If measuring area, the table doesn't get shown
  mTable->clear();
  QTreeWidgetItem* item = new QTreeWidgetItem( QStringList( QString::number( 0, 'f', 1 ) ) );
  item->setTextAlignment( 0, Qt::AlignRight );
  mTable->addTopLevelItem( item );
  mTotal = 0.;

  updateUi();
}


void QgsMeasureDialog::mousePress( QgsPoint &point )
{
  if ( mTool->points().size() == 0 )
  {
    addPoint( point );
    show();
  }
  raise();

  mouseMove( point );
}

void QgsMeasureDialog::mouseMove( QgsPoint &point )
{
  QSettings settings;
  int decimalPlaces = settings.value( "/qgis/measure/decimalplaces", "3" ).toInt();

  // show current distance/area while moving the point
  // by creating a temporary copy of point array
  // and adding moving point at the end
  if ( mMeasureArea && mTool->points().size() > 1 )
  {
    QList<QgsPoint> tmpPoints = mTool->points();
    tmpPoints.append( point );
    double area = mDa.measurePolygon( tmpPoints );
    editTotal->setText( formatArea( area, decimalPlaces ) );
  }
  else if ( !mMeasureArea && mTool->points().size() > 0 )
  {
    QgsPoint p1( mTool->points().last() ), p2( point );

    double d = mDa.measureLine( p1, p2 );
    editTotal->setText( formatDistance( mTotal + d, decimalPlaces ) );
    QGis::UnitType myDisplayUnits;
    // Ignore units
    convertMeasurement( d, myDisplayUnits, false );
    QTreeWidgetItem *item = mTable->topLevelItem( mTable->topLevelItemCount() - 1 );
    item->setText( 0, QLocale::system().toString( d, 'f', decimalPlaces ) );
  }
}

void QgsMeasureDialog::addPoint( QgsPoint &point )
{
  QSettings settings;
  int decimalPlaces = settings.value( "/qgis/measure/decimalplaces", "3" ).toInt();

  int numPoints = mTool->points().size();
  if ( mMeasureArea && numPoints > 2 )
  {
    double area = mDa.measurePolygon( mTool->points() );
    editTotal->setText( formatArea( area, decimalPlaces ) );
  }
  else if ( !mMeasureArea && numPoints > 1 )
  {
    int last = numPoints - 2;

    QgsPoint p1 = mTool->points()[last], p2 = mTool->points()[last+1];

    double d = mDa.measureLine( p1, p2 );

    mTotal += d;
    editTotal->setText( formatDistance( mTotal, decimalPlaces ) );

    QGis::UnitType myDisplayUnits;
    // Ignore units
    convertMeasurement( d, myDisplayUnits, false );

    QTreeWidgetItem *item = mTable->topLevelItem( mTable->topLevelItemCount() - 1 );
    item->setText( 0, QLocale::system().toString( d, 'f', decimalPlaces ) );

    item = new QTreeWidgetItem( QStringList( QLocale::system().toString( 0.0, 'f', decimalPlaces ) ) );
    item->setTextAlignment( 0, Qt::AlignRight );
    mTable->addTopLevelItem( item );
    mTable->scrollToItem( item );
  }
}

void QgsMeasureDialog::on_buttonBox_rejected( void )
{
  restart();
  QDialog::close();
}

void QgsMeasureDialog::closeEvent( QCloseEvent *e )
{
  saveWindowLocation();
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

QString QgsMeasureDialog::formatDistance( double distance, int decimalPlaces )
{
  QSettings settings;
  bool baseUnit = settings.value( "/qgis/measure/keepbaseunit", false ).toBool();

  QGis::UnitType myDisplayUnits;
  convertMeasurement( distance, myDisplayUnits, false );
  return QgsDistanceArea::textUnit( distance, decimalPlaces, myDisplayUnits, false, baseUnit );
}

QString QgsMeasureDialog::formatArea( double area, int decimalPlaces )
{
  QSettings settings;
  bool baseUnit = settings.value( "/qgis/measure/keepbaseunit", false ).toBool();

  QGis::UnitType myDisplayUnits;
  convertMeasurement( area, myDisplayUnits, true );
  return QgsDistanceArea::textUnit( area, decimalPlaces, myDisplayUnits, true, baseUnit );
}

void QgsMeasureDialog::updateUi()
{
  QSettings settings;
  int decimalPlaces = settings.value( "/qgis/measure/decimalplaces", "3" ).toInt();

  double dummy = 1.0;
  QGis::UnitType myDisplayUnits;
  // The dummy distance is ignored
  convertMeasurement( dummy, myDisplayUnits, false );

  switch ( myDisplayUnits )
  {
    case QGis::Meters:
      mTable->setHeaderLabels( QStringList( tr( "Segments (in meters)" ) ) );
      break;
    case QGis::Feet:
      mTable->setHeaderLabels( QStringList( tr( "Segments (in feet)" ) ) );
      break;
    case QGis::DegreesMinutesSeconds:
    case QGis::DegreesDecimalMinutes:
    case QGis::Degrees:
      mTable->setHeaderLabels( QStringList( tr( "Segments (in degrees)" ) ) );
      break;
    case QGis::UnknownUnit:
      mTable->setHeaderLabels( QStringList( tr( "Segments" ) ) );
  }

  if ( mMeasureArea )
  {
    mTable->hide();
    editTotal->setText( formatArea( 0, decimalPlaces ) );
  }
  else
  {
    mTable->show();
    editTotal->setText( formatDistance( 0, decimalPlaces ) );
  }

  configureDistanceArea();
}

void QgsMeasureDialog::convertMeasurement( double &measure, QGis::UnitType &u, bool isArea )
{
  // Helper for converting between meters and feet
  // The parameter &u is out only...

  QGis::UnitType myUnits = mTool->canvas()->mapUnits();
  if (( myUnits == QGis::Degrees || myUnits == QGis::Feet )  &&
      mcbProjectionEnabled->isChecked() )
  {
    // Measuring on an ellipsoid returns meters, and so does using projections???
    myUnits = QGis::Meters;
    QgsDebugMsg( "We're measuring on an ellipsoid or using projections, the system is returning meters" );
  }

  // Get the units for display
  QSettings settings;
  QString myDisplayUnitsTxt = settings.value( "/qgis/measure/displayunits", "meters" ).toString();

  // Only convert between meters and feet
  if ( myUnits == QGis::Meters && myDisplayUnitsTxt == "feet" )
  {
    QgsDebugMsg( QString( "Converting %1 meters" ).arg( QString::number( measure ) ) );
    measure /= 0.3048;
    if ( isArea )
    {
      measure /= 0.3048;
    }
    QgsDebugMsg( QString( "to %1 feet" ).arg( QString::number( measure ) ) );
    myUnits = QGis::Feet;
  }
  if ( myUnits == QGis::Feet && myDisplayUnitsTxt == "meters" )
  {
    QgsDebugMsg( QString( "Converting %1 feet" ).arg( QString::number( measure ) ) );
    measure *= 0.3048;
    if ( isArea )
    {
      measure *= 0.3048;
    }
    QgsDebugMsg( QString( "to %1 meters" ).arg( QString::number( measure ) ) );
    myUnits = QGis::Meters;
  }

  u = myUnits;
}

void QgsMeasureDialog::changeProjectionEnabledState()
{
  // store value
  QSettings settings;
  if ( mcbProjectionEnabled->isChecked() )
    settings.setValue( "/qgis/measure/projectionEnabled", 2 );
  else
    settings.setValue( "/qgis/measure/projectionEnabled", 0 );

  // clear interface
  mTable->clear();
  QTreeWidgetItem* item = new QTreeWidgetItem( QStringList( QString::number( 0, 'f', 1 ) ) );
  item->setTextAlignment( 0, Qt::AlignRight );
  mTable->addTopLevelItem( item );
  mTotal = 0;
  updateUi();

  int decimalPlaces = settings.value( "/qgis/measure/decimalplaces", "3" ).toInt();

  if ( mMeasureArea )
  {
    double area = 0.0;
    if ( mTool->points().size() > 1 )
    {
      area = mDa.measurePolygon( mTool->points() );
    }
    editTotal->setText( formatArea( area, decimalPlaces ) );
  }
  else
  {
    QList<QgsPoint>::const_iterator it;
    bool b = true; // first point

    QgsPoint p1, p2;

    for ( it = mTool->points().constBegin(); it != mTool->points().constEnd(); ++it )
    {
      p2 = *it;
      if ( !b )
      {
        double d  = mDa.measureLine( p1, p2 );
        mTotal += d;
        editTotal->setText( formatDistance( mTotal, decimalPlaces ) );
        QGis::UnitType myDisplayUnits;

        convertMeasurement( d, myDisplayUnits, false );

        QTreeWidgetItem *item = mTable->topLevelItem( mTable->topLevelItemCount() - 1 );
        item->setText( 0, QLocale::system().toString( d, 'f', decimalPlaces ) );
        item = new QTreeWidgetItem( QStringList( QLocale::system().toString( 0.0, 'f', decimalPlaces ) ) );
        item->setTextAlignment( 0, Qt::AlignRight );
        mTable->addTopLevelItem( item );
        mTable->scrollToItem( item );
      }
      p1 = p2;
      b = false;
    }
  }
}

void QgsMeasureDialog::configureDistanceArea()
{
  QSettings settings;
  QString ellipsoidId = settings.value( "/qgis/measure/ellipsoid", "WGS84" ).toString();
  mDa.setSourceCrs( mTool->canvas()->mapRenderer()->destinationCrs().srsid() );
  mDa.setEllipsoid( ellipsoidId );
  mDa.setProjectionsEnabled( mcbProjectionEnabled->isChecked() );
}
