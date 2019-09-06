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
#include "qgsmapcanvas.h"
#include "qgisapp.h"
#include "qgs3dmapsettings.h"


Qgs3DMeasureDialog::Qgs3DMeasureDialog( Qgs3DMapToolMeasureLine *tool, Qt::WindowFlags f )
  : QDialog( tool->canvas()->topLevelWidget(), f )
  , mTool( tool )
{
  setupUi( this );

  // New button
  QPushButton *newButton = new QPushButton( tr( "&New" ) );
  buttonBox->addButton( newButton, QDialogButtonBox::ActionRole );
  connect( newButton, &QAbstractButton::clicked, this, &Qgs3DMeasureDialog::restart );

  // Remove/Hide unused features/options from 2D line measurement
  // Only support for Cartesian
  mCartesian->setChecked( true );
  // Hide ellipsoidal and Cartesian radio button (not needed)
  mCartesian->hide();
  mEllipsoidal->hide();
  groupBox->hide();

  // Initialize unit combo box
  // Add a configuration button
  QPushButton *cb = new QPushButton( tr( "&Configuration" ) );
  buttonBox->addButton( cb, QDialogButtonBox::ActionRole );
  connect( cb, &QAbstractButton::clicked, this, &Qgs3DMeasureDialog::openConfigTab );

  repopulateComboBoxUnits();

  // Remove Help button until we have proper documentation for it
  buttonBox->removeButton( buttonBox->button( QDialogButtonBox::Help ) );

  connect( buttonBox, &QDialogButtonBox::rejected, this, &Qgs3DMeasureDialog::reject );
  connect( mUnitsCombo, static_cast<void ( QComboBox::* )( int )>( &QComboBox::currentIndexChanged ), this, &Qgs3DMeasureDialog::unitsChanged );
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
}

void Qgs3DMeasureDialog::addPoint()
{
  int numPoints = mTool->points().size();
  if ( numPoints > 1 )
  {
    if ( !mTool->done() )
    {
      // Add new entry in the table
      addMeasurement( lastDistance(), lastZDistance(), lastHorisontalDistance() );
      mTotal += lastDistance();
      editTotal->setText( formatDistance( convertLength( mTotal, mDisplayedDistanceUnit ) ) );
    }
  }
  else
  {
    // Update total with new displayed unit
    editTotal->setText( formatDistance( convertLength( mTotal, mDisplayedDistanceUnit ) ) );
  }
}

double Qgs3DMeasureDialog::lastDistance()
{
  QgsPoint lastPoint = mTool->points().rbegin()[0];
  QgsPoint secondLastPoint = mTool->points().rbegin()[1];
  return lastPoint.distance3D( secondLastPoint );
}

double Qgs3DMeasureDialog::lastZDistance()
{
  QgsPoint lastPoint = mTool->points().rbegin()[0];
  QgsPoint secondLastPoint = mTool->points().rbegin()[1];
  return lastPoint.z() - secondLastPoint.z();
}

double Qgs3DMeasureDialog::lastHorisontalDistance()
{
  QgsPoint lastPoint = mTool->points().rbegin()[0];
  QgsPoint secondLastPoint = mTool->points().rbegin()[1];
  return lastPoint.distance( secondLastPoint );
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
    QgsLineString measureLine( mTool->points() );
    mTotal = measureLine.length3D();
    // Update total with new displayed unit
    editTotal->setText( formatDistance( convertLength( mTotal, mDisplayedDistanceUnit ) ) );
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
  resetTable();
}

void Qgs3DMeasureDialog::closeEvent( QCloseEvent *e )
{
  reject();
  e->accept();
}

void Qgs3DMeasureDialog::updateSettings()
{
  QgsSettings settings;

  mDecimalPlaces = settings.value( QStringLiteral( "qgis/measure/decimalplaces" ), "3" ).toInt();
  mMapDistanceUnit = mTool->canvas()->map()->crs().mapUnits();
  mDisplayedDistanceUnit = QgsUnitTypes::decodeDistanceUnit(
                             settings.value( QStringLiteral( "qgis/measure/displayunits" ),
                                 QgsUnitTypes::encodeUnit( QgsUnitTypes::DistanceUnknownUnit ) ).toString() );
  setupTableHeader();
  mUnitsCombo->setCurrentIndex( mUnitsCombo->findData( mDisplayedDistanceUnit ) );
}

void Qgs3DMeasureDialog::unitsChanged( int index )
{
  mDisplayedDistanceUnit = static_cast< QgsUnitTypes::DistanceUnit >( mUnitsCombo->itemData( index ).toInt() );

  setupTableHeader();

  // Reset table
  mTable->clear();

  // Repopulate the table based on new displayed unit
  QVector<QgsPoint>::const_iterator it;
  bool isFirstPoint = true; // first point
  QgsPoint p1, p2;
  QVector< QgsPoint > tmpPoints = mTool->points();
  for ( it = tmpPoints.constBegin(); it != tmpPoints.constEnd(); ++it )
  {
    p2 = *it;
    if ( !isFirstPoint )
    {
      double distance = p1.distance3D( p2 );
      double zDistance = p2.z() - p1.z();
      double horisontalDistance = p1.distance( p2 );
      addMeasurement( distance, zDistance, horisontalDistance );
    }
    p1 = p2;
    isFirstPoint = false;
  }
  // Update total with new displayed unit
  editTotal->setText( formatDistance( convertLength( mTotal, mDisplayedDistanceUnit ) ) );
}

double Qgs3DMeasureDialog::convertLength( double length, QgsUnitTypes::DistanceUnit toUnit ) const
{
  double factorUnits = QgsUnitTypes::fromUnitToUnitFactor( mMapDistanceUnit, toUnit );
  return length * factorUnits;
}

QString Qgs3DMeasureDialog::formatDistance( double distance ) const
{
  QgsSettings settings;
  bool baseUnit = settings.value( QStringLiteral( "qgis/measure/keepbaseunit" ), true ).toBool();
  return QgsUnitTypes::formatDistance( distance, mDecimalPlaces, mDisplayedDistanceUnit, baseUnit );
}

void Qgs3DMeasureDialog::showHelp()
{
  QgsHelp::openHelp( QStringLiteral( "introduction/general_tools.html#measuring" ) );
}

void Qgs3DMeasureDialog::openConfigTab()
{
  QgisApp::instance()->showOptionsDialog( this, QStringLiteral( "mOptionsPageMapTools" ) );
}

void Qgs3DMeasureDialog::setupTableHeader()
{
  // Set the table header to show displayed unit
  QStringList headers;
  headers << tr( "Horisontal Distance" )
          << tr( "Vertical Distance" )
          << tr( "3D Distance" );
  QTreeWidgetItem *headerItem = new QTreeWidgetItem( headers );
  headerItem->setTextAlignment( 0, Qt::AlignRight );
  headerItem->setTextAlignment( 1, Qt::AlignRight );
  headerItem->setTextAlignment( 2, Qt::AlignRight );
  mTable->setHeaderItem( headerItem );
  mTable->resizeColumnToContents( 0 );
  mTable->resizeColumnToContents( 1 );
  mTable->resizeColumnToContents( 2 );
}

void Qgs3DMeasureDialog::addMeasurement( double distance, double zDistance, double horisontalDistance )
{
  QStringList content;
  content << QLocale().toString( convertLength( horisontalDistance, mDisplayedDistanceUnit ), 'f', mDecimalPlaces )
          << QLocale().toString( convertLength( zDistance, mDisplayedDistanceUnit ), 'f', mDecimalPlaces )
          << QLocale().toString( convertLength( distance, mDisplayedDistanceUnit ), 'f', mDecimalPlaces );
  QTreeWidgetItem *item = new QTreeWidgetItem( content );
  item->setTextAlignment( 0, Qt::AlignRight );
  item->setTextAlignment( 1, Qt::AlignRight );
  item->setTextAlignment( 2, Qt::AlignRight );
  mTable->addTopLevelItem( item );
  mTable->scrollToItem( item );
}

void Qgs3DMeasureDialog::resetTable()
{
  mTable->clear();
  mTotal = 0.;
  // Update total with new displayed unit
  editTotal->setText( formatDistance( convertLength( mTotal, mDisplayedDistanceUnit ) ) );
}
