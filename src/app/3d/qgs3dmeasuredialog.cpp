/***************************************************************************
  qgs3dmeasuredialog.cpp
  --------------------------------------
  Date                 : Jun 2019
  Copyright            : (C) 2019 by Ismail Sunni
  Email                : imajimatika at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <QCloseEvent>
#include <QPushButton>

#include "qgsproject.h"
#include "qgs3dmeasuredialog.h"
#include "qgs3dmaptoolmeasureline.h"
#include "qdebug.h"
#include "qgsmapcanvas.h"
#include "qgisapp.h"

Qgs3DMeasureDialog::Qgs3DMeasureDialog( Qgs3DMapToolMeasureLine *tool, Qt::WindowFlags f )
  : QDialog( tool->canvas()->topLevelWidget(), f )
  , mTool( tool )
{
  setupUi( this );

  // New button
  QPushButton *newButton = new QPushButton( tr( "&New" ) );
  buttonBox->addButton( newButton, QDialogButtonBox::ActionRole );
  connect( newButton, &QAbstractButton::clicked, this, &Qgs3DMeasureDialog::restart );

  // Only support for cartesian
  mCartesian->setChecked( true );

  // Hide ellipsoidal and cartesian radio button
  mCartesian->hide();
  mEllipsoidal->hide();

  // Initialize unit combo box
  repopulateComboBoxUnits();

  // Choose unit
  if ( mUseMapUnits )
    mUnitsCombo->setCurrentIndex( mUnitsCombo->findData( QgsUnitTypes::DistanceUnknownUnit ) );
  else
    mUnitsCombo->setCurrentIndex( mUnitsCombo->findData( QgsProject::instance()->distanceUnits() ) );

  QgsMapCanvas *canvas2D = QgisApp::instance()->mapCanvas();

  qInfo() << "3D Measure Dialog created";
  connect( buttonBox, &QDialogButtonBox::rejected, this, &Qgs3DMeasureDialog::reject );
  connect( mUnitsCombo, static_cast<void ( QComboBox::* )( int )>( &QComboBox::currentIndexChanged ), this, &Qgs3DMeasureDialog::unitsChanged );
  connect( buttonBox, &QDialogButtonBox::helpRequested, this, &Qgs3DMeasureDialog::showHelp );
  connect( canvas2D, &QgsMapCanvas::destinationCrsChanged, this, &Qgs3DMeasureDialog::crsChanged );
}

void Qgs3DMeasureDialog::saveWindowLocation()
{
  QgsSettings settings;
  settings.setValue( QStringLiteral( "Windows/3DMeasure/geometry" ), saveGeometry() );
  const QString &key = "/Windows/3DMeasure/h";
  settings.setValue( key, height() );
}

void Qgs3DMeasureDialog::restorePosition()
{
  QgsSettings settings;
  restoreGeometry( settings.value( QStringLiteral( "Windows/3DMeasure/geometry" ) ).toByteArray() );
  int wh = settings.value( QStringLiteral( "Windows/3DMeasure/h" ), 200 ).toInt();
  resize( width(), wh );
//    updateUi();
}

void Qgs3DMeasureDialog::addPoint()
{
  int numPoints = mTool->points().size();
  qInfo() << "Add point. Current num points: " << numPoints;
  if ( numPoints > 1 )
  {
    if ( !mTool->done() )
    {
      QTreeWidgetItem *item = new QTreeWidgetItem( QStringList( QLocale().toString( 0.0, 'f', mDecimalPlaces ) ) );
      item->setTextAlignment( 0, Qt::AlignRight );
      mTable->addTopLevelItem( item );
      mTable->scrollToItem( item );


      item->setText( 0, QString::number( lastDistance() ) );
      mTotal += lastDistance();
      editTotal->setText( QString::number( mTotal ) );
    }
  }
  else
  {
    editTotal->setText( QString::number( mTotal ) );
  }
}

double Qgs3DMeasureDialog::lastDistance()
{
  QgsPoint lastPoint = mTool->points().rbegin()[0];
  QgsPoint secondLastPoint = mTool->points().rbegin()[1];
  // Euclidean distance
  qInfo() << "Last line " << lastPoint.x() << lastPoint.y() << lastPoint.z();
  qInfo() << "Last line " << secondLastPoint.x() << secondLastPoint.y() << secondLastPoint.z();
  return qSqrt(
           ( lastPoint.x() - secondLastPoint.x() ) * ( lastPoint.x() - secondLastPoint.x() ) +
           ( lastPoint.y() - secondLastPoint.y() ) * ( lastPoint.y() - secondLastPoint.y() ) +
           ( lastPoint.z() - secondLastPoint.z() ) * ( lastPoint.z() - secondLastPoint.z() )
         );
}

void Qgs3DMeasureDialog::updateUi()
{
  // Set tooltip to indicate how we calculate measurements
  QString toolTip = tr( "The calculations are based on:" );

  mConvertToDisplayUnits = true;
  QgsMapCanvas *canvas2D = QgisApp::instance()->mapCanvas();

  if ( mCartesian->isChecked() || !canvas2D->mapSettings().destinationCrs().isValid() )
  {
    toolTip += "<br> * ";
    if ( mCartesian->isChecked() )
    {
      toolTip += tr( "Cartesian calculation selected" );
      mConvertToDisplayUnits = true;
    }
    else
    {
      toolTip += tr( "No map projection set." );
      toolTip += "<br> * " + tr( "Units are unknown." );
      mConvertToDisplayUnits = false;
    }
    mDa.setEllipsoid( GEO_NONE );
  }
  else if ( canvas2D->mapSettings().destinationCrs().mapUnits() == QgsUnitTypes::DistanceDegrees
            && mDistanceUnits == QgsUnitTypes::DistanceDegrees )
  {
    //both source and destination units are degrees
    toolTip += "<br> * " + tr( "Both project CRS (%1) and measured length are in degrees, so distance is calculated using Cartesian calculations in degrees." ).arg(
                 canvas2D->mapSettings().destinationCrs().description() );
    mDa.setEllipsoid( GEO_NONE );
    mConvertToDisplayUnits = false; //not required since we will be measuring in degrees
  }
  else
  {
    QgsUnitTypes::DistanceUnit resultUnit = QgsUnitTypes::DistanceUnknownUnit;
    resultUnit = canvas2D->mapSettings().destinationCrs().mapUnits();
    toolTip += "<br> * " + tr( "Project ellipsoidal calculation is not selected." ) + ' ';
    toolTip += tr( "Distance is calculated in %1, based on project CRS (%2)." ).arg( QgsUnitTypes::toString( resultUnit ),
               canvas2D->mapSettings().destinationCrs().description() );
    setWindowTitle( tr( "Measure" ) );

    if ( QgsUnitTypes::unitType( resultUnit ) == QgsUnitTypes::Geographic &&
         QgsUnitTypes::unitType( mDistanceUnits ) == QgsUnitTypes::Standard )
    {
      toolTip += QLatin1String( "<br> * Distance is roughly converted to meters by using scale at equator (1 degree = 111319.49 meters)." );
      resultUnit = QgsUnitTypes::DistanceMeters;
    }
    else if ( QgsUnitTypes::unitType( resultUnit ) == QgsUnitTypes::Standard &&
              QgsUnitTypes::unitType( mDistanceUnits ) == QgsUnitTypes::Geographic )
    {
      toolTip += QLatin1String( "<br> * Distance is roughly converted to degrees by using scale at equator (1 degree = 111319.49 meters)." );
      resultUnit = QgsUnitTypes::DistanceDegrees;
    }
    if ( resultUnit != mDistanceUnits )
    {
      if ( QgsUnitTypes::unitType( resultUnit ) == QgsUnitTypes::Standard &&
           QgsUnitTypes::unitType( mDistanceUnits ) == QgsUnitTypes::Standard )
      {
        // only shown if both conditions are true:
        // - the display unit is a standard distance measurement (e.g., feet)
        // - either the canvas units is also a standard distance OR we are using an ellipsoid (in which case the
        //   value will be in meters)
        toolTip += "<br> * " + tr( "The value is converted from %1 to %2." ).arg( QgsUnitTypes::toString( resultUnit ),
                   QgsUnitTypes::toString( mDistanceUnits ) );
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
  editTotal->setText( QString::number( mTotal ) );

  if ( mUseMapUnits )
  {
    mUnitsCombo->setCurrentIndex( mUnitsCombo->findData( QgsUnitTypes::DistanceUnknownUnit ) );
    mTable->setHeaderLabels( QStringList( tr( "Segments [%1]" ).arg( QgsUnitTypes::toString( mMapDistanceUnits ) ) ) );
  }
  else
  {
    mUnitsCombo->setCurrentIndex( mUnitsCombo->findData( mDistanceUnits ) );
    if ( mDistanceUnits != QgsUnitTypes::DistanceUnknownUnit )
      mTable->setHeaderLabels( QStringList( tr( "Segments [%1]" ).arg( QgsUnitTypes::toString( mDistanceUnits ) ) ) );
    else
      mTable->setHeaderLabels( QStringList( tr( "Segments" ) ) );
  }
  QVector<QgsPoint>::const_iterator it;
  bool b = true; // first point

  QgsPoint p1, p2;
  mTotal = 0;
  QVector< QgsPoint > tmpPoints = mTool->points();
  for ( it = tmpPoints.constBegin(); it != tmpPoints.constEnd(); ++it )
  {
    p2 = *it;
    if ( !b )
    {
      double d = -1;
      d = mDa.measureLine( p1, p2 );
      if ( mConvertToDisplayUnits )
      {
        if ( mDistanceUnits == QgsUnitTypes::DistanceUnknownUnit && mMapDistanceUnits != QgsUnitTypes::DistanceUnknownUnit )
          d = convertLength( d, mMapDistanceUnits );
        else
          d = convertLength( d, mDistanceUnits );
      }

      QTreeWidgetItem *item = new QTreeWidgetItem( QStringList( QLocale().toString( d, 'f', mDecimalPlaces ) ) );
      item->setTextAlignment( 0, Qt::AlignRight );
      mTable->addTopLevelItem( item );
      mTable->scrollToItem( item );
    }
    p1 = p2;
    b = false;
  }

  mTotal = mDa.measureLine3D( mTool->points() );
  mTable->show(); // Show the table with items
  editTotal->setText( formatDistance( mTotal, mConvertToDisplayUnits ) );
}

void Qgs3DMeasureDialog::repopulateComboBoxUnits()
{
  mUnitsCombo->addItem( QgsUnitTypes::toString( QgsUnitTypes::DistanceMeters ), QgsUnitTypes::DistanceMeters );
  mUnitsCombo->addItem( QgsUnitTypes::toString( QgsUnitTypes::DistanceKilometers ), QgsUnitTypes::DistanceKilometers );
  mUnitsCombo->addItem( QgsUnitTypes::toString( QgsUnitTypes::DistanceFeet ), QgsUnitTypes::DistanceFeet );
  mUnitsCombo->addItem( QgsUnitTypes::toString( QgsUnitTypes::DistanceYards ), QgsUnitTypes::DistanceYards );
  mUnitsCombo->addItem( QgsUnitTypes::toString( QgsUnitTypes::DistanceMiles ), QgsUnitTypes::DistanceMiles );
  mUnitsCombo->addItem( QgsUnitTypes::toString( QgsUnitTypes::DistanceNauticalMiles ), QgsUnitTypes::DistanceNauticalMiles );
  mUnitsCombo->addItem( QgsUnitTypes::toString( QgsUnitTypes::DistanceCentimeters ), QgsUnitTypes::DistanceCentimeters );
  mUnitsCombo->addItem( QgsUnitTypes::toString( QgsUnitTypes::DistanceMillimeters ), QgsUnitTypes::DistanceMillimeters );
  mUnitsCombo->addItem( QgsUnitTypes::toString( QgsUnitTypes::DistanceDegrees ), QgsUnitTypes::DistanceDegrees );
  mUnitsCombo->addItem( tr( "map units" ), QgsUnitTypes::DistanceUnknownUnit );
}

void Qgs3DMeasureDialog::removeLastPoint()
{
  int numPoints = mTool->points().size();
  if ( numPoints >= 1 )
  {
    // Remove final row
    delete mTable->takeTopLevelItem( mTable->topLevelItemCount() - 1 );
    // Update total distance
    mTotal = mDa.measureLine3D( mTool->points() );
    editTotal->setText( formatDistance( mTotal, mConvertToDisplayUnits ) );
  }
}

void Qgs3DMeasureDialog::reject()
{
  saveWindowLocation();
  restart();
  QDialog::close();
}

void Qgs3DMeasureDialog::restart()
{
  mTool->restart();

  mTable->clear();
  mTotal = 0.;
  updateUi();
}

void Qgs3DMeasureDialog::closeEvent( QCloseEvent *e )
{
  reject();
  e->accept();
}

void Qgs3DMeasureDialog::unitsChanged( int index )
{
  mDistanceUnits = static_cast< QgsUnitTypes::DistanceUnit >( mUnitsCombo->itemData( index ).toInt() );
  if ( mDistanceUnits == QgsUnitTypes::DistanceUnknownUnit )
  {
    mUseMapUnits = true;
    mDistanceUnits = mMapDistanceUnits;
  }
  else
  {
    mUseMapUnits = false;
  }
  mTable->clear();
  mTotal = 0.;
  updateUi();
}

void Qgs3DMeasureDialog::crsChanged()
{
  QgsMapCanvas *canvas2D = QgisApp::instance()->mapCanvas();
  if ( !canvas2D->mapSettings().destinationCrs().isValid() )
  {
    mUnitsCombo->setEnabled( false );

    mUnitsCombo->setCurrentIndex( mUnitsCombo->findData( QgsUnitTypes::DistanceUnknownUnit ) );
  }
  else
  {
    mUnitsCombo->setEnabled( true );
  }

  mTable->clear();
  mTotal = 0.;
  updateUi();
}

double Qgs3DMeasureDialog::convertLength( double length, QgsUnitTypes::DistanceUnit toUnit ) const
{
  return mDa.convertLengthMeasurement( length, toUnit );
}

QString Qgs3DMeasureDialog::formatDistance( double distance, bool convertUnits ) const
{
  QgsSettings settings;
  bool baseUnit = settings.value( QStringLiteral( "qgis/3Dmeasure/keepbaseunit" ), true ).toBool();

  if ( convertUnits )
    distance = convertLength( distance, mDistanceUnits );

  int decimals = mDecimalPlaces;
  if ( mDistanceUnits == QgsUnitTypes::DistanceDegrees  && distance < 1 )
  {
    // special handling for degrees - because we can't use smaller units (eg m->mm), we need to make sure there's
    // enough decimal places to show a usable measurement value
    int minPlaces = std::round( std::log10( 1.0 / distance ) ) + 1;
    decimals = std::max( decimals, minPlaces );
  }
  return QgsDistanceArea::formatDistance( distance, decimals, mDistanceUnits, baseUnit );
}

void Qgs3DMeasureDialog::showHelp()
{
  QgsHelp::openHelp( QStringLiteral( "introduction/general_tools.html#measuring" ) );
}
