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

#include "qgsmeasuredialog.h"
#include "qgsmeasuretool.h"

#include "qgslogger.h"
#include "qgscontexthelp.h"
#include "qgsdistancearea.h"
#include "qgsmapcanvas.h"
#include "qgsmaprenderer.h"
#include "qgsproject.h"
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

  updateSettings();
}

void QgsMeasureDialog::updateSettings()
{
  QSettings settings;

  mDecimalPlaces = settings.value( "/qgis/measure/decimalplaces", "3" ).toInt();
  mCanvasUnits = mTool->canvas()->mapUnits();
  mDisplayUnits = QGis::fromLiteral( settings.value( "/qgis/measure/displayunits", QGis::toLiteral( QGis::Meters ) ).toString() );
  // Configure QgsDistanceArea
  mDa.setSourceCrs( mTool->canvas()->mapRenderer()->destinationCrs().srsid() );
  mDa.setEllipsoid( QgsProject::instance()->readEntry( "Measure", "/Ellipsoid", GEO_NONE ) );
  // Only use ellipsoidal calculation when project wide transformation is enabled.
  if ( mTool->canvas()->mapRenderer()->hasCrsTransformEnabled() )
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
  QgsDebugMsg( QString( "Display units: %1" ).arg( QGis::toLiteral( mDisplayUnits ) ) );
  QgsDebugMsg( QString( "Canvas units : %1" ).arg( QGis::toLiteral( mCanvasUnits ) ) );

  // clear interface
  mTable->clear();

  mTotal = 0;
  updateUi();

}

void QgsMeasureDialog::restart()
{
  mTool->restart();

  mTable->clear();
  mTotal = 0.;
  updateUi();
}


void QgsMeasureDialog::mousePress( QgsPoint &point )
{
  show();
  raise();
  if ( ! mTool->done() )
  {
    mouseMove( point );
  }
}

void QgsMeasureDialog::mouseMove( QgsPoint &point )
{
  Q_UNUSED( point );

  // show current distance/area while moving the point
  // by creating a temporary copy of point array
  // and adding moving point at the end
  if ( mMeasureArea && mTool->points().size() > 2 )
  {
    double area = mDa.measurePolygon( mTool->points() );
    editTotal->setText( formatArea( area ) );
  }
  else if ( !mMeasureArea && mTool->points().size() > 1 )
  {
    int last = mTool->points().size() - 1;
    QgsPoint p1 = mTool->points()[last];
    QgsPoint p2 = mTool->points()[last-1];
    double d = mDa.measureLine( p1, p2 );

    mTotal = mDa.measureLine( mTool->points() );
    editTotal->setText( formatDistance( mTotal ) );

    QGis::UnitType displayUnits;
    // Meters or feet?
    convertMeasurement( d, displayUnits, false );

    // Set moving
    QTreeWidgetItem *item = mTable->topLevelItem( mTable->topLevelItemCount() - 1 );
    item->setText( 0, QLocale::system().toString( d, 'f', mDecimalPlaces ) );
    QgsDebugMsg( QString( "Final result is %1" ).arg( item->text( 0 ) ) );
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
  else if ( !mMeasureArea && numPoints > 1 )
  {
    QTreeWidgetItem * item = new QTreeWidgetItem( QStringList( QLocale::system().toString( 0.0, 'f', mDecimalPlaces ) ) );
    item->setTextAlignment( 0, Qt::AlignRight );
    mTable->addTopLevelItem( item );
    mTable->scrollToItem( item );
  }
  QgsDebugMsg( "Exiting" );
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

QString QgsMeasureDialog::formatDistance( double distance )
{
  QSettings settings;
  bool baseUnit = settings.value( "/qgis/measure/keepbaseunit", false ).toBool();

  QGis::UnitType newDisplayUnits;
  convertMeasurement( distance, newDisplayUnits, false );
  return QgsDistanceArea::textUnit( distance, mDecimalPlaces, newDisplayUnits, false, baseUnit );
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
  if ( ! mTool->canvas()->hasCrsTransformEnabled() )
  {
    toolTip += "<br> * " + tr( "Project CRS transformation is turned off." ) + " ";
    toolTip += tr( "Canvas units setting is taken from project properties setting (%1)." ).arg( QGis::tr( mCanvasUnits ) );
    toolTip += "<br> * " + tr( "Ellipsoidal calculation is not possible, as project CRS is undefined." );
  }
  else
  {
    if ( mDa.ellipsoidalEnabled() )
    {
      toolTip += "<br> * " + tr( "Project CRS transformation is turned on and ellipsoidal calculation is selected." ) + " ";
      toolTip += "<br> * " + tr( "The coordinates are transformed to the chosen ellipsoid (%1), and the result is in meters" ).arg( mDa.ellipsoid() );
    }
    else
    {
      toolTip += "<br> * " + tr( "Project CRS transformation is turned on but ellipsoidal calculation is not selected." );
      toolTip += "<br> * " + tr( "The canvas units setting is taken from the project CRS (%1)." ).arg( QGis::tr( mCanvasUnits ) );
    }
  }

  if (( mCanvasUnits == QGis::Meters && mDisplayUnits == QGis::Feet ) || ( mCanvasUnits == QGis::Feet && mDisplayUnits == QGis::Meters ) )
  {
    toolTip += "<br> * " + tr( "Finally, the value is converted from %1 to %2." ).arg( QGis::tr( mCanvasUnits ) ).arg( QGis::tr( mDisplayUnits ) );
  }

  editTotal->setToolTip( toolTip );
  mTable->setToolTip( toolTip );

  QGis::UnitType newDisplayUnits;
  double dummy = 1.0;
  convertMeasurement( dummy, newDisplayUnits, true );
  mTable->setHeaderLabels( QStringList( tr( "Segments [%1]" ).arg( QGis::tr( newDisplayUnits ) ) ) );

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

    for ( it = mTool->points().constBegin(); it != mTool->points().constEnd(); ++it )
    {
      p2 = *it;
      if ( !b )
      {
        double d  = mDa.measureLine( p1, p2 );
        QGis::UnitType dummyUnits;
        convertMeasurement( d, dummyUnits, false );

        QTreeWidgetItem *item = new QTreeWidgetItem( QStringList( QLocale::system().toString( d , 'f', mDecimalPlaces ) ) );
        item->setTextAlignment( 0, Qt::AlignRight );
        mTable->addTopLevelItem( item );
        mTable->scrollToItem( item );
      }
      p1 = p2;
      b = false;
    }
    mTotal = mDa.measureLine( mTool->points() );
    mTable->show(); // Show the table with items
    editTotal->setText( formatDistance( mTotal ) );
  }
}

void QgsMeasureDialog::convertMeasurement( double &measure, QGis::UnitType &u, bool isArea )
{
  // Helper for converting between meters and feet
  // The parameter &u is out only...

  // Get the canvas units
  QGis::UnitType myUnits = mCanvasUnits;

  QgsDebugMsg( QString( "Preferred display units are %1" ).arg( QGis::toLiteral( mDisplayUnits ) ) );

  mDa.convertMeasurement( measure, myUnits, mDisplayUnits, isArea );
  u = myUnits;
}

