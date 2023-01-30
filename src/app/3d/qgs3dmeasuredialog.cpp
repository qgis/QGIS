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

#include "qgs3dmeasuredialog.h"
#include "qgs3dmaptoolmeasureline.h"
#include "qgisapp.h"
#include "qgs3dmapcanvas.h"
#include "qgs3dmapsettings.h"
#include "qgshelp.h"

Qgs3DMeasureDialog::Qgs3DMeasureDialog( Qgs3DMapToolMeasureLine *tool, Qt::WindowFlags f )
  : QDialog( tool->canvas()->topLevelWidget(), f )
  , mTool( tool )
{
  setupUi( this );

  setWindowTitle( tr( " 3D Measurement Tool" ) );

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

  // Update text for 3D specific
  totalDistanceLabel->setText( tr( "Total 3D Distance" ) );

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
  const QgsSettings settings;
  restoreGeometry( settings.value( QStringLiteral( "Windows/3DMeasure/geometry" ) ).toByteArray() );
  const int wh = settings.value( QStringLiteral( "Windows/3DMeasure/h" ), 200 ).toInt();
  resize( width(), wh );
}

void Qgs3DMeasureDialog::addPoint()
{
  const int numPoints = mTool->points().size();
  if ( numPoints > 1 )
  {
    if ( !mTool->done() )
    {
      // Add new entry in the table
      addMeasurement( lastDistance(), lastVerticalDistance(), lastHorizontalDistance() );
      mTotal += lastDistance();
      mHorizontalTotal += lastHorizontalDistance();
      updateTotal();
    }
  }
  else
  {
    updateTotal();
  }
}

double Qgs3DMeasureDialog::lastDistance()
{
  const QgsPoint lastPoint = mTool->points().rbegin()[0];
  const QgsPoint secondLastPoint = mTool->points().rbegin()[1];
  return lastPoint.distance3D( secondLastPoint );
}

double Qgs3DMeasureDialog::lastVerticalDistance()
{
  const QgsPoint lastPoint = mTool->points().rbegin()[0];
  const QgsPoint secondLastPoint = mTool->points().rbegin()[1];
  return lastPoint.z() - secondLastPoint.z();
}

double Qgs3DMeasureDialog::lastHorizontalDistance()
{
  const QgsPoint lastPoint = mTool->points().rbegin()[0];
  const QgsPoint secondLastPoint = mTool->points().rbegin()[1];
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
  const int numPoints = mTool->points().size();
  if ( numPoints >= 1 )
  {
    // Remove final row
    delete mTable->takeTopLevelItem( mTable->topLevelItemCount() - 1 );
    // Update total distance
    const QgsLineString measureLine( mTool->points() );
    mTotal = measureLine.length3D();
    mHorizontalTotal = measureLine.length();
    updateTotal();
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
  const QgsSettings settings;

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
  updateTable();
  updateTotal();
}

double Qgs3DMeasureDialog::convertLength( double length, QgsUnitTypes::DistanceUnit toUnit ) const
{
  const double factorUnits = QgsUnitTypes::fromUnitToUnitFactor( mMapDistanceUnit, toUnit );
  return length * factorUnits;
}

QString Qgs3DMeasureDialog::formatDistance( double distance ) const
{
  const QgsSettings settings;
  const bool baseUnit = settings.value( QStringLiteral( "qgis/measure/keepbaseunit" ), true ).toBool();
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
  headers << tr( "Horizontal Distance" );
  headers << tr( "Vertical Distance" );
  headers << tr( "3D Distance" );

  QTreeWidgetItem *headerItem = new QTreeWidgetItem( headers );
  for ( int i = 0; i < headers.count(); ++i )
  {
    headerItem->setTextAlignment( i, Qt::AlignRight );
  }
  mTable->setHeaderItem( headerItem );
  for ( int i = 0; i < headers.count(); ++i )
  {
    mTable->resizeColumnToContents( i );
  }
}

void Qgs3DMeasureDialog::addMeasurement( double distance, double verticalDistance, double horizontalDistance )
{
  QStringList content;
  content << QLocale().toString( convertLength( horizontalDistance, mDisplayedDistanceUnit ), 'f', mDecimalPlaces );
  content << QLocale().toString( convertLength( verticalDistance, mDisplayedDistanceUnit ), 'f', mDecimalPlaces );
  content << QLocale().toString( convertLength( distance, mDisplayedDistanceUnit ), 'f', mDecimalPlaces );
  QTreeWidgetItem *item = new QTreeWidgetItem( content );
  for ( int i = 0; i < content.count(); ++i )
  {
    item->setTextAlignment( i, Qt::AlignRight );
  }
  mTable->addTopLevelItem( item );
  mTable->scrollToItem( item );
}

void Qgs3DMeasureDialog::updateTotal()
{
  // Update total with new displayed unit
  editTotal->setText( formatDistance( convertLength( mTotal, mDisplayedDistanceUnit ) ) );
  editHorizontalTotal->setText( formatDistance( convertLength( mHorizontalTotal, mDisplayedDistanceUnit ) ) );
}

void Qgs3DMeasureDialog::updateTable()
{
  setupTableHeader();

  // Reset table
  mTable->clear();

  // Repopulate the table based on new displayed unit
  QVector<QgsPoint>::const_iterator it;
  bool isFirstPoint = true; // first point
  QgsPoint p1, p2;
  const QVector< QgsPoint > tmpPoints = mTool->points();
  for ( it = tmpPoints.constBegin(); it != tmpPoints.constEnd(); ++it )
  {
    p2 = *it;
    if ( !isFirstPoint )
    {
      const double distance = p1.distance3D( p2 );
      const double verticalDistance = p2.z() - p1.z();
      const double horizontalDistance = p1.distance( p2 );
      addMeasurement( distance, verticalDistance, horizontalDistance );
    }
    p1 = p2;
    isFirstPoint = false;
  }
}

void Qgs3DMeasureDialog::resetTable()
{
  mTable->clear();
  mTotal = 0.;
  mHorizontalTotal = 0.;
  updateTotal();
}
