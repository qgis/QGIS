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
#include "qgsdistancearea.h"
#include "qgsmapcanvas.h"
#include "qgsproject.h"
#include "qgscoordinatereferencesystem.h"
#include "qgsunittypes.h"
#include "qgssettings.h"

#include <QCloseEvent>
#include <QLocale>
#include <QPushButton>


QgsMeasureDialog::QgsMeasureDialog( QgsMeasureTool *tool, Qt::WindowFlags f )
  : QDialog( tool->canvas()->topLevelWidget(), f )
  , mMeasureArea( tool->measureArea() )
  , mTool( tool )
  , mCanvas( tool->canvas() )
{
  setupUi( this );
  connect( buttonBox, &QDialogButtonBox::helpRequested, this, &QgsMeasureDialog::showHelp );

  QPushButton *nb = new QPushButton( tr( "&New" ) );
  buttonBox->addButton( nb, QDialogButtonBox::ActionRole );
  connect( nb, &QAbstractButton::clicked, this, &QgsMeasureDialog::restart );

  // Add a configuration button
  QPushButton *cb = new QPushButton( tr( "&Configuration" ) );
  buttonBox->addButton( cb, QDialogButtonBox::ActionRole );
  connect( cb, &QAbstractButton::clicked, this, &QgsMeasureDialog::openConfigTab );

  repopulateComboBoxUnits( mMeasureArea );
  if ( mMeasureArea )
    mUnitsCombo->setCurrentIndex( mUnitsCombo->findData( QgsProject::instance()->areaUnits() ) );
  else
    mUnitsCombo->setCurrentIndex( mUnitsCombo->findData( QgsProject::instance()->distanceUnits() ) );

  if ( !mCanvas->mapSettings().destinationCrs().isValid() )
  {
    mUnitsCombo->setEnabled( false );
    if ( mMeasureArea )
      mUnitsCombo->setCurrentIndex( mUnitsCombo->findData( QgsUnitTypes::DistanceUnknownUnit ) );
    else
      mUnitsCombo->setCurrentIndex( mUnitsCombo->findData( QgsUnitTypes::AreaUnknownUnit ) );
  }

  updateSettings();

  connect( mUnitsCombo, static_cast<void ( QComboBox::* )( int )>( &QComboBox::currentIndexChanged ), this, &QgsMeasureDialog::unitsChanged );
  connect( buttonBox, &QDialogButtonBox::rejected, this, &QgsMeasureDialog::reject );
  connect( mCanvas, &QgsMapCanvas::destinationCrsChanged, this, &QgsMeasureDialog::crsChanged );

  groupBox->setCollapsed( true );
}

void QgsMeasureDialog::openConfigTab()
{
  QgisApp::instance()->showOptionsDialog( this, QStringLiteral( "mOptionsPageMapTools" ) );
}

void QgsMeasureDialog::crsChanged()
{
  if ( !mCanvas->mapSettings().destinationCrs().isValid() )
  {
    mUnitsCombo->setEnabled( false );
    if ( mMeasureArea )
      mUnitsCombo->setCurrentIndex( mUnitsCombo->findData( QgsUnitTypes::DistanceUnknownUnit ) );
    else
      mUnitsCombo->setCurrentIndex( mUnitsCombo->findData( QgsUnitTypes::AreaUnknownUnit ) );
  }
  else
  {
    mUnitsCombo->setEnabled( true );
  }
  updateUi();
}

void QgsMeasureDialog::updateSettings()
{
  QgsSettings settings;

  mDecimalPlaces = settings.value( QStringLiteral( "qgis/measure/decimalplaces" ), "3" ).toInt();
  mCanvasUnits = mCanvas->mapUnits();
  // Configure QgsDistanceArea
  mDistanceUnits = QgsProject::instance()->distanceUnits();
  mAreaUnits = QgsProject::instance()->areaUnits();
  mDa.setSourceCrs( mCanvas->mapSettings().destinationCrs(), QgsProject::instance()->transformContext() );
  mDa.setEllipsoid( QgsProject::instance()->ellipsoid() );

  mTable->clear();
  mTotal = 0;
  updateUi();
}

void QgsMeasureDialog::unitsChanged( int index )
{
  if ( mMeasureArea )
    mAreaUnits = static_cast< QgsUnitTypes::AreaUnit >( mUnitsCombo->itemData( index ).toInt() );
  else
    mDistanceUnits = static_cast< QgsUnitTypes::DistanceUnit >( mUnitsCombo->itemData( index ).toInt() );
  mTable->clear();
  mTotal = 0.;
  updateUi();

  if ( !mTool->done() )
  {
    // re-add temporary mouse cursor position
    addPoint();
    mouseMove( mLastMousePoint );
  }
}

void QgsMeasureDialog::restart()
{
  mTool->restart();

  mTable->clear();
  mTotal = 0.;
  updateUi();
}


void QgsMeasureDialog::mouseMove( const QgsPointXY &point )
{
  mLastMousePoint = point;
  // show current distance/area while moving the point
  // by creating a temporary copy of point array
  // and adding moving point at the end
  if ( mMeasureArea && mTool->points().size() >= 2 )
  {
    QVector<QgsPointXY> tmpPoints = mTool->points();
    tmpPoints.append( point );
    double area;
    if ( mCartesian->isChecked() )
      area = mDa.measurePolygon( tmpPoints, true );
    else
      area = mDa.measurePolygon( tmpPoints );
    editTotal->setText( formatArea( area ) );
  }
  else if ( !mMeasureArea && !mTool->points().empty() )
  {
    QVector< QgsPointXY > tmpPoints = mTool->points();
    QgsPointXY p1( tmpPoints.at( tmpPoints.size() - 1 ) ), p2( point );

    double d;
    if ( mCartesian->isChecked() )
      d = p1.distance( p2 );
    else
      d = mDa.measureLine( p1, p2 );

    editTotal->setText( formatDistance( mTotal + d ) );

    d = convertLength( d, mDistanceUnits );

    // Set moving
    QTreeWidgetItem *item = mTable->topLevelItem( mTable->topLevelItemCount() - 1 );
    if ( item )
    {
      item->setText( 0, QLocale().toString( d, 'f', mDecimalPlaces ) );
    }
  }
}

void QgsMeasureDialog::addPoint()
{
  int numPoints = mTool->points().size();
  if ( mMeasureArea && numPoints > 2 )
  {
    double area;
    if ( mCartesian->isChecked() )
      area = mDa.measurePolygon( mTool->points(), true );
    else
      area = mDa.measurePolygon( mTool->points() );
    editTotal->setText( formatArea( area ) );
  }
  else if ( !mMeasureArea && numPoints >= 1 )
  {
    if ( !mTool->done() )
    {
      QTreeWidgetItem *item = new QTreeWidgetItem( QStringList( QLocale().toString( 0.0, 'f', mDecimalPlaces ) ) );
      item->setTextAlignment( 0, Qt::AlignRight );
      mTable->addTopLevelItem( item );
      mTable->scrollToItem( item );
    }
    if ( numPoints > 1 )
    {
      if ( mCartesian->isChecked() )
        mTool->points().at( 0 ).distance( mTool->points().at( 1 ) );
      else
        mTotal = mDa.measureLine( mTool->points() );
      editTotal->setText( formatDistance( mTotal ) );
    }
  }
}

void QgsMeasureDialog::removeLastPoint()
{
  int numPoints = mTool->points().size();
  if ( mMeasureArea )
  {
    if ( numPoints > 1 )
    {
      QVector<QgsPointXY> tmpPoints = mTool->points();
      tmpPoints.append( mLastMousePoint );
      double area;
      if ( mCartesian->isChecked() )
        area = mDa.measurePolygon( tmpPoints, true );
      else
        area = mDa.measurePolygon( tmpPoints );

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

    if ( mCartesian->isChecked() )
      mTotal = mTool->points().at( 0 ).distance( mTool->points().at( 1 ) );
    else
      mTotal = mDa.measureLine( mTool->points() );

    if ( !mTool->done() )
    {
      // need to add the distance for the temporary mouse cursor point
      QVector< QgsPointXY > tmpPoints = mTool->points();
      QgsPointXY p1( tmpPoints.at( tmpPoints.size() - 1 ) );
      double d;
      if ( mCartesian->isChecked() )
        d = p1.distance( mLastMousePoint );
      else
        d = mDa.measureLine( p1, mLastMousePoint );

      d = convertLength( d, mDistanceUnits );

      QTreeWidgetItem *item = mTable->topLevelItem( mTable->topLevelItemCount() - 1 );
      item->setText( 0, QLocale().toString( d, 'f', mDecimalPlaces ) );
      editTotal->setText( formatDistance( mTotal + d ) );
    }
    else
    {
      editTotal->setText( formatDistance( mTotal ) );
    }
  }
}

void QgsMeasureDialog::closeEvent( QCloseEvent *e )
{
  reject();
  e->accept();
}

void QgsMeasureDialog::restorePosition()
{
  QgsSettings settings;
  restoreGeometry( settings.value( QStringLiteral( "Windows/Measure/geometry" ) ).toByteArray() );
  int wh;
  if ( mMeasureArea )
    wh = settings.value( QStringLiteral( "Windows/Measure/hNoTable" ), 70 ).toInt();
  else
    wh = settings.value( QStringLiteral( "Windows/Measure/h" ), 200 ).toInt();
  resize( width(), wh );
  updateUi();
}

void QgsMeasureDialog::saveWindowLocation()
{
  QgsSettings settings;
  settings.setValue( QStringLiteral( "Windows/Measure/geometry" ), saveGeometry() );
  const QString &key = mMeasureArea ? "/Windows/Measure/hNoTable" : "/Windows/Measure/h";
  settings.setValue( key, height() );
}

QString QgsMeasureDialog::formatDistance( double distance, bool convertUnits ) const
{
  QgsSettings settings;
  bool baseUnit = settings.value( QStringLiteral( "qgis/measure/keepbaseunit" ), true ).toBool();

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

QString QgsMeasureDialog::formatArea( double area, bool convertUnits ) const
{
  if ( convertUnits )
    area = convertArea( area, mAreaUnits );

  return QgsDistanceArea::formatArea( area, mDecimalPlaces, mAreaUnits, true );
}

void QgsMeasureDialog::updateUi()
{
  // Set tooltip to indicate how we calculate measurments
  QString toolTip = tr( "The calculations are based on:" );

  bool forceCartesian = false;
  bool convertToDisplayUnits = true;

  if ( mMeasureArea )
  {
    if ( !mCanvas->mapSettings().destinationCrs().isValid() )
    {
      // no CRS => no units, newb!
      toolTip += "<br> * " + tr( "No map projection set, so area is calculated using Cartesian calculations." );
      toolTip += "<br> * " + tr( "Units are unknown." );
      forceCartesian = true;
      convertToDisplayUnits = false;
    }
    else if ( mCanvas->mapSettings().destinationCrs().mapUnits() == QgsUnitTypes::DistanceDegrees
              && ( mAreaUnits == QgsUnitTypes::AreaSquareDegrees || mAreaUnits == QgsUnitTypes::AreaUnknownUnit ) )
    {
      //both source and destination units are degrees
      toolTip += "<br> * " + tr( "Both project CRS (%1) and measured area are in degrees, so area is calculated using Cartesian calculations in square degrees." ).arg(
                   mCanvas->mapSettings().destinationCrs().description() );
      forceCartesian = true;
      convertToDisplayUnits = false; //not required since we will be measuring in degrees
    }
    else
    {
      QgsUnitTypes::AreaUnit resultUnit = QgsUnitTypes::AreaUnknownUnit;
      if ( mDa.willUseEllipsoid() )
      {
        resultUnit = QgsUnitTypes::AreaSquareMeters;
        toolTip += "<br> * " + tr( "Project ellipsoidal calculation is selected." ) + ' ';
        toolTip += "<br> * " + tr( "The coordinates are transformed to the chosen ellipsoid (%1), and the area is calculated in %2." ).arg( mDa.ellipsoid(),
                   QgsUnitTypes::toString( resultUnit ) );
      }
      else
      {
        resultUnit = QgsUnitTypes::distanceToAreaUnit( mCanvas->mapSettings().destinationCrs().mapUnits() );
        toolTip += "<br> * " + tr( "Project ellipsoidal calculation is not selected." ) + ' ';
        toolTip += tr( "Area is calculated in %1, based on project CRS (%2)." ).arg( QgsUnitTypes::toString( resultUnit ),
                   mCanvas->mapSettings().destinationCrs().description() );
      }
      setWindowTitle( tr( "Measure" ) );

      if ( QgsUnitTypes::unitType( resultUnit ) == QgsUnitTypes::Geographic &&
           QgsUnitTypes::unitType( mAreaUnits ) == QgsUnitTypes::Standard )
      {
        toolTip += QLatin1String( "<br> * Area is roughly converted to square meters by using scale at equator (1 degree = 111319.49 meters)." );
        resultUnit = QgsUnitTypes::AreaSquareMeters;
      }
      else if ( QgsUnitTypes::unitType( resultUnit ) == QgsUnitTypes::Standard &&
                QgsUnitTypes::unitType( mAreaUnits ) == QgsUnitTypes::Geographic )
      {
        toolTip += QLatin1String( "<br> * Area is roughly converted to square degrees by using scale at equator (1 degree = 111319.49 meters)." );
        resultUnit = QgsUnitTypes::AreaSquareDegrees;
      }

      if ( resultUnit != mAreaUnits )
      {
        if ( QgsUnitTypes::unitType( resultUnit ) == QgsUnitTypes::Standard &&
             QgsUnitTypes::unitType( mAreaUnits ) == QgsUnitTypes::Standard )
        {
          // only shown if both conditions are true:
          // - the display unit is a standard distance measurement (e.g., square feet)
          // - either the canvas units is also a standard distance OR we are using an ellipsoid (in which case the
          //   value will be in square meters)
          toolTip += "<br> * " + tr( "The value is converted from %1 to %2." ).arg( QgsUnitTypes::toString( resultUnit ),
                     QgsUnitTypes::toString( mAreaUnits ) );
        }
        else
        {
          //should not be possible!
        }
      }
    }
  }
  else
  {
    if ( !mCanvas->mapSettings().destinationCrs().isValid() )
    {
      // no CRS => no units, newb!
      toolTip += "<br> * " + tr( "No map projection set, so distance is calculated using Cartesian calculations." );
      toolTip += "<br> * " + tr( "Units are unknown." );
      forceCartesian = true;
      convertToDisplayUnits = false;
    }
    else if ( mCanvas->mapSettings().destinationCrs().mapUnits() == QgsUnitTypes::DistanceDegrees
              && mDistanceUnits == QgsUnitTypes::DistanceDegrees )
    {
      //both source and destination units are degrees
      toolTip += "<br> * " + tr( "Both project CRS (%1) and measured length are in degrees, so distance is calculated using Cartesian calculations in degrees." ).arg(
                   mCanvas->mapSettings().destinationCrs().description() );
      forceCartesian = true;
      convertToDisplayUnits = false; //not required since we will be measuring in degrees
    }
    else
    {
      QgsUnitTypes::DistanceUnit resultUnit = QgsUnitTypes::DistanceUnknownUnit;
      if ( mDa.willUseEllipsoid() )
      {
        resultUnit = QgsUnitTypes::DistanceMeters;
        toolTip += "<br> * " + tr( "Project ellipsoidal calculation is selected." ) + ' ';
        toolTip += "<br> * " + tr( "The coordinates are transformed to the chosen ellipsoid (%1), and the distance is calculated in %2." ).arg( mDa.ellipsoid(),
                   QgsUnitTypes::toString( resultUnit ) );
      }
      else
      {
        resultUnit = mCanvas->mapSettings().destinationCrs().mapUnits();
        toolTip += "<br> * " + tr( "Project ellipsoidal calculation is not selected." ) + ' ';
        toolTip += tr( "Distance is calculated in %1, based on project CRS (%2)." ).arg( QgsUnitTypes::toString( resultUnit ),
                   mCanvas->mapSettings().destinationCrs().description() );
      }
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
  }

  editTotal->setToolTip( toolTip );
  mTable->setToolTip( toolTip );
  mNotesLabel->setText( toolTip );

  if ( mMeasureArea )
  {
    mUnitsCombo->setCurrentIndex( mUnitsCombo->findData( mAreaUnits ) );
  }
  else
  {
    mUnitsCombo->setCurrentIndex( mUnitsCombo->findData( mDistanceUnits ) );
    if ( mDistanceUnits != QgsUnitTypes::DistanceUnknownUnit )
    {
      mTable->setHeaderLabels( QStringList( tr( "Segments [%1]" ).arg( QgsUnitTypes::toString( mDistanceUnits ) ) ) );
    }
    else
    {
      mTable->setHeaderLabels( QStringList( tr( "Segments" ) ) );
    }
  }

  if ( mMeasureArea )
  {
    double area = 0.0;
    if ( mTool->points().size() > 1 )
    {
      if ( mCartesian->isChecked() )
        area = mDa.measurePolygon( mTool->points(), true );
      else
        area = mDa.measurePolygon( mTool->points() );
    }
    mTable->hide(); // Hide the table, only show summary.
    editTotal->setText( formatArea( area ) );
  }
  else
  {
    QVector<QgsPointXY>::const_iterator it;
    bool b = true; // first point

    QgsPointXY p1, p2;
    mTotal = 0;
    QVector< QgsPointXY > tmpPoints = mTool->points();
    for ( it = tmpPoints.constBegin(); it != tmpPoints.constEnd(); ++it )
    {
      p2 = *it;
      if ( !b )
      {
        double d = -1;
        if ( forceCartesian )
        {
          //Cartesian calculation forced
          d = std::sqrt( p2.sqrDist( p1 ) );
          mTotal += d;
        }
        else
        {
          if ( mCartesian->isChecked() )
            d = p1.distance( p2 );
          else
            d = mDa.measureLine( p1, p2 );
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

    if ( !forceCartesian )
      mTotal = mDa.measureLine( mTool->points() );

    mTable->show(); // Show the table with items
    editTotal->setText( formatDistance( mTotal, convertToDisplayUnits ) );
  }
}

void QgsMeasureDialog::repopulateComboBoxUnits( bool isArea )
{
  mUnitsCombo->clear();
  if ( isArea )
  {
    mUnitsCombo->addItem( QgsUnitTypes::toString( QgsUnitTypes::AreaSquareMeters ), QgsUnitTypes::AreaSquareMeters );
    mUnitsCombo->addItem( QgsUnitTypes::toString( QgsUnitTypes::AreaSquareKilometers ), QgsUnitTypes::AreaSquareKilometers );
    mUnitsCombo->addItem( QgsUnitTypes::toString( QgsUnitTypes::AreaSquareFeet ), QgsUnitTypes::AreaSquareFeet );
    mUnitsCombo->addItem( QgsUnitTypes::toString( QgsUnitTypes::AreaSquareYards ), QgsUnitTypes::AreaSquareYards );
    mUnitsCombo->addItem( QgsUnitTypes::toString( QgsUnitTypes::AreaSquareMiles ), QgsUnitTypes::AreaSquareMiles );
    mUnitsCombo->addItem( QgsUnitTypes::toString( QgsUnitTypes::AreaHectares ), QgsUnitTypes::AreaHectares );
    mUnitsCombo->addItem( QgsUnitTypes::toString( QgsUnitTypes::AreaAcres ), QgsUnitTypes::AreaAcres );
    mUnitsCombo->addItem( QgsUnitTypes::toString( QgsUnitTypes::AreaSquareCentimeters ), QgsUnitTypes::AreaSquareCentimeters );
    mUnitsCombo->addItem( QgsUnitTypes::toString( QgsUnitTypes::AreaSquareMillimeters ), QgsUnitTypes::AreaSquareMillimeters );
    mUnitsCombo->addItem( QgsUnitTypes::toString( QgsUnitTypes::AreaSquareNauticalMiles ), QgsUnitTypes::AreaSquareNauticalMiles );
    mUnitsCombo->addItem( QgsUnitTypes::toString( QgsUnitTypes::AreaSquareDegrees ), QgsUnitTypes::AreaSquareDegrees );
    mUnitsCombo->addItem( tr( "map units" ), QgsUnitTypes::AreaUnknownUnit );
  }
  else
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
}

double QgsMeasureDialog::convertLength( double length, QgsUnitTypes::DistanceUnit toUnit ) const
{
  return mDa.convertLengthMeasurement( length, toUnit );
}

double QgsMeasureDialog::convertArea( double area, QgsUnitTypes::AreaUnit toUnit ) const
{
  return mDa.convertAreaMeasurement( area, toUnit );
}


void QgsMeasureDialog::reject()
{
  saveWindowLocation();
  restart();
  QDialog::close();
}

void QgsMeasureDialog::showHelp()
{
  QgsHelp::openHelp( QStringLiteral( "introduction/general_tools.html#measuring" ) );
}
