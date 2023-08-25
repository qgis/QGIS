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
#include "qgsmessagebar.h"
#include "qgsmeasuredialog.h"
#include "qgsmeasuretool.h"
#include "qgsdistancearea.h"
#include "qgsmapcanvas.h"
#include "qgsproject.h"
#include "qgscoordinatereferencesystem.h"
#include "qgsunittypes.h"
#include "qgssettings.h"
#include "qgssettingsentryimpl.h"
#include "qgssettingstree.h"
#include "qgsgui.h"

#include <QClipboard>
#include <QCloseEvent>
#include <QLocale>
#include <QPushButton>


const QgsSettingsEntryBool *QgsMeasureDialog::settingClipboardHeader = new QgsSettingsEntryBool( QStringLiteral( "clipboard-header" ), QgsSettingsTree::sTreeMeasure, false, QObject::tr( "Whether the header should be copied to the clipboard along the coordinates, distances" ) );

const QgsSettingsEntryString *QgsMeasureDialog::settingClipboardSeparator = new QgsSettingsEntryString( QStringLiteral( "clipboard-separator" ), QgsSettingsTree::sTreeMeasure, QStringLiteral( "\t" ), QObject::tr( "Separator between the measure columns copied to the clipboard" ) );

const QgsSettingsEntryBool *QgsMeasureDialog::settingClipboardAlwaysUseDecimalPoint = new QgsSettingsEntryBool( QStringLiteral( "clipboard-use-decimal-point" ), QgsSettingsTree::sTreeMeasure, false, QObject::tr( "Whether to use the locale decimal separator or always use the decimal point. Needed to export data as csv with a locale that uses a comma as its decimal separator." ) );

QgsMeasureDialog::QgsMeasureDialog( QgsMeasureTool *tool, Qt::WindowFlags f )
  : QDialog( tool->canvas()->topLevelWidget(), f )
  , mMeasureArea( tool->measureArea() )
  , mTool( tool )
  , mCanvas( tool->canvas() )
{
  setupUi( this );
  QgsGui::enableAutoGeometryRestore( this );
  connect( buttonBox, &QDialogButtonBox::helpRequested, this, &QgsMeasureDialog::showHelp );

  // hide 3D related options
  editHorizontalTotal->hide();
  totalHorizontalDistanceLabel->hide();

  QPushButton *nb = new QPushButton( tr( "&New" ) );
  buttonBox->addButton( nb, QDialogButtonBox::ActionRole );
  connect( nb, &QAbstractButton::clicked, this, &QgsMeasureDialog::restart );

  // Add a configuration button
  QPushButton *cb = new QPushButton( tr( "&Configuration" ) );
  buttonBox->addButton( cb, QDialogButtonBox::ActionRole );
  connect( cb, &QAbstractButton::clicked, this, &QgsMeasureDialog::openConfigTab );

  if ( !mMeasureArea )
  {
    QAction *copyAction = new QAction( tr( "Copy" ), this );
    QPushButton *cpb = new QPushButton( tr( "Copy" ) );
    buttonBox->addButton( cpb, QDialogButtonBox::ActionRole );
    connect( cpb, &QAbstractButton::clicked, copyAction, &QAction::trigger );
    connect( copyAction, &QAction::triggered, this, &QgsMeasureDialog::copyMeasurements );

    // Add context menu in the table
    mTable->setContextMenuPolicy( Qt::ActionsContextMenu );
    mTable->addAction( copyAction );
  }

  repopulateComboBoxUnits( mMeasureArea );
  if ( mMeasureArea )
  {
    if ( mUseMapUnits )
      mUnitsCombo->setCurrentIndex( mUnitsCombo->findData( static_cast< int >( Qgis::AreaUnit::Unknown ) ) );
    else
      mUnitsCombo->setCurrentIndex( mUnitsCombo->findData( static_cast< int >( QgsProject::instance()->areaUnits() ) ) );
  }
  else
  {
    if ( mUseMapUnits )
      mUnitsCombo->setCurrentIndex( mUnitsCombo->findData( static_cast< int >( Qgis::DistanceUnit::Unknown ) ) );
    else
      mUnitsCombo->setCurrentIndex( mUnitsCombo->findData( static_cast< int >( QgsProject::instance()->distanceUnits() ) ) );
  }

  if ( !mCanvas->mapSettings().destinationCrs().isValid() )
  {
    mUnitsCombo->setEnabled( false );
    if ( mMeasureArea )
      mUnitsCombo->setCurrentIndex( mUnitsCombo->findData( static_cast< int >( Qgis::DistanceUnit::Unknown ) ) );
    else
      mUnitsCombo->setCurrentIndex( mUnitsCombo->findData( static_cast< int >( Qgis::AreaUnit::Unknown ) ) );
  }

  updateSettings();

  connect( mUnitsCombo, static_cast<void ( QComboBox::* )( int )>( &QComboBox::currentIndexChanged ), this, &QgsMeasureDialog::unitsChanged );
  connect( buttonBox, &QDialogButtonBox::rejected, this, &QgsMeasureDialog::reject );
  connect( mCanvas, &QgsMapCanvas::destinationCrsChanged, this, &QgsMeasureDialog::crsChanged );
  connect( mCartesian, &QRadioButton::toggled, this, &QgsMeasureDialog::projChanged );

  groupBox->setCollapsed( true );
}

void QgsMeasureDialog::projChanged()
{
  if ( mCartesian->isChecked() )
  {
    mDa.setEllipsoid( geoNone() );
  }
  else
  {
    mDa.setEllipsoid( QgsProject::instance()->ellipsoid() );
  }

  // Update the table and information displayed to the user
  updateUi();
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
      mUnitsCombo->setCurrentIndex( mUnitsCombo->findData( static_cast< int >( Qgis::DistanceUnit::Unknown ) ) );
    else
      mUnitsCombo->setCurrentIndex( mUnitsCombo->findData( static_cast< int >( Qgis::AreaUnit::Unknown ) ) );
  }
  else
  {
    mUnitsCombo->setEnabled( true );
  }

  updateUi();
}

void QgsMeasureDialog::updateSettings()
{
  const QgsSettings settings;

  mDecimalPlaces = settings.value( QStringLiteral( "qgis/measure/decimalplaces" ), 3 ).toInt();
  mCanvasUnits = mCanvas->mapUnits();

  // Configure QgsDistanceArea
  mDistanceUnits = QgsProject::instance()->distanceUnits();
  mMapDistanceUnits = QgsProject::instance()->crs().mapUnits();
  mAreaUnits = QgsProject::instance()->areaUnits();
  mDa.setSourceCrs( mCanvas->mapSettings().destinationCrs(), QgsProject::instance()->transformContext() );
  updateUnitsMembers();

  // Calling projChanged() will set the ellipsoid and clear then re-populate the table
  projChanged();

  if ( mCartesian->isChecked() || !mCanvas->mapSettings().destinationCrs().isValid() ||
       ( mCanvas->mapSettings().destinationCrs().mapUnits() == Qgis::DistanceUnit::Degrees
         && mDistanceUnits == Qgis::DistanceUnit::Degrees ) )
  {
    mDa.setEllipsoid( geoNone() );
  }
  else
  {
    mDa.setEllipsoid( QgsProject::instance()->ellipsoid() );
  }
}

void QgsMeasureDialog::unitsChanged( int index )
{
  if ( mMeasureArea )
  {
    mAreaUnits = static_cast< Qgis::AreaUnit >( mUnitsCombo->itemData( index ).toInt() );
  }
  else
  {
    mDistanceUnits = static_cast< Qgis::DistanceUnit >( mUnitsCombo->itemData( index ).toInt() );
  }

  updateUnitsMembers();
  updateUi();
}

void QgsMeasureDialog:: updateUnitsMembers()
{
  if ( mMeasureArea )
  {
    if ( mAreaUnits == Qgis::AreaUnit::Unknown )
    {
      mUseMapUnits = true;
      mAreaUnits = QgsUnitTypes::distanceToAreaUnit( mMapDistanceUnits );
    }
    else
    {
      mUseMapUnits = false;
    }
  }
  else
  {
    if ( mDistanceUnits == Qgis::DistanceUnit::Unknown )
    {
      mUseMapUnits = true;
      mDistanceUnits = mMapDistanceUnits;
    }
    else
    {
      mUseMapUnits = false;
    }
  }
}

void QgsMeasureDialog::restart()
{
  mTool->restart();
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
    const double area = mDa.measurePolygon( tmpPoints );
    editTotal->setText( formatArea( area ) );
  }
  else if ( !mMeasureArea && !mTool->points().empty() )
  {
    const QVector< QgsPointXY > tmpPoints = mTool->points();
    QgsPointXY p1( tmpPoints.at( tmpPoints.size() - 1 ) ), p2( point );
    double d = mDa.measureLine( p1, p2 );
    editTotal->setText( formatDistance( mTotal + d, mConvertToDisplayUnits ) );
    d = convertLength( d, mDistanceUnits );

    // Set moving
    QTreeWidgetItem *item = mTable->topLevelItem( mTable->topLevelItemCount() - 1 );
    if ( item )
    {
      item->setText( Columns::X, QLocale().toString( p2.x(), 'f', mDecimalPlacesCoordinates ) );
      item->setText( Columns::Y, QLocale().toString( p2.y(), 'f', mDecimalPlacesCoordinates ) );
      item->setText( Columns::Distance, QLocale().toString( d, 'f', mDecimalPlaces ) );
    }
  }
}

void QgsMeasureDialog::addPoint()
{
  const int numPoints = mTool->points().size();
  if ( mMeasureArea && numPoints > 2 )
  {
    const double area = mDa.measurePolygon( mTool->points() );
    editTotal->setText( formatArea( area ) );
  }
  else if ( !mMeasureArea && numPoints >= 1 )
  {
    if ( !mTool->done() )
    {
      if ( numPoints == 1 )
      {
        // First point, so we add a first item, with no computed distance and only the coordinates
        QTreeWidgetItem *item = new QTreeWidgetItem();
        QgsPointXY lastPoint = mTool->points().last();
        item->setText( Columns::X, QLocale().toString( lastPoint.x(), 'f', mDecimalPlacesCoordinates ) );
        item->setText( Columns::Y, QLocale().toString( lastPoint.y(), 'f', mDecimalPlacesCoordinates ) );
        mTable->addTopLevelItem( item );
      }
      QTreeWidgetItem *item = new QTreeWidgetItem();
      QgsPointXY lastPoint = mTool->points().last();
      item->setText( Columns::X, QLocale().toString( lastPoint.x(), 'f', mDecimalPlacesCoordinates ) );
      item->setText( Columns::Y, QLocale().toString( lastPoint.y(), 'f', mDecimalPlacesCoordinates ) );
      item->setText( Columns::Distance, QLocale().toString( 0.0, 'f', mDecimalPlaces ) );
      item->setTextAlignment( Columns::Distance, Qt::AlignRight );
      mTable->addTopLevelItem( item );
      mTable->scrollToItem( item );
    }
    if ( numPoints > 1 )
    {
      mTotal = mDa.measureLine( mTool->points() );
      editTotal->setText( formatDistance( mTotal, mConvertToDisplayUnits ) );
    }
  }
}

void QgsMeasureDialog::removeLastPoint()
{
  const int numPoints = mTool->points().size();
  if ( mMeasureArea )
  {
    if ( numPoints > 1 )
    {
      QVector<QgsPointXY> tmpPoints = mTool->points();
      if ( !mTool->done() )
        tmpPoints.append( mLastMousePoint );
      const double area = mDa.measurePolygon( tmpPoints );
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

    mTotal = mDa.measureLine( mTool->points() );

    if ( !mTool->done() )
    {
      // need to add the distance for the temporary mouse cursor point
      const QVector< QgsPointXY > tmpPoints = mTool->points();
      const QgsPointXY p1( tmpPoints.at( tmpPoints.size() - 1 ) );
      double d = mDa.measureLine( p1, mLastMousePoint );

      d = convertLength( d, mDistanceUnits );

      QTreeWidgetItem *item = mTable->topLevelItem( mTable->topLevelItemCount() - 1 );
      item->setText( Columns::X, QLocale().toString( mLastMousePoint.x(), 'f', mDecimalPlacesCoordinates ) );
      item->setText( Columns::Y, QLocale().toString( mLastMousePoint.y(), 'f', mDecimalPlacesCoordinates ) );
      item->setText( Columns::Distance, QLocale().toString( d, 'f', mDecimalPlaces ) );
      editTotal->setText( formatDistance( mTotal + d ) );
    }
    else
    {
      editTotal->setText( formatDistance( mTotal, mConvertToDisplayUnits ) );
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
  const QgsSettings settings;
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
  const QString &key = mMeasureArea ? "/Windows/Measure/hNoTable" : "/Windows/Measure/h";
  settings.setValue( key, height() );
}

QString QgsMeasureDialog::formatDistance( double distance, bool convertUnits ) const
{
  const QgsSettings settings;
  const bool baseUnit = settings.value( QStringLiteral( "qgis/measure/keepbaseunit" ), true ).toBool();

  if ( convertUnits )
    distance = convertLength( distance, mDistanceUnits );

  int decimals = mDecimalPlaces;
  if ( mDistanceUnits == Qgis::DistanceUnit::Degrees  && distance < 1 )
  {
    // special handling for degrees - because we can't use smaller units (eg m->mm), we need to make sure there's
    // enough decimal places to show a usable measurement value
    const int minPlaces = std::round( std::log10( 1.0 / distance ) ) + 1;
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
  // Clear the table
  mTable->clear();
  mTotal = 0.;

  // Set tooltip to indicate how we calculate measurements
  QString toolTip = tr( "The calculations are based on:" );

  mDa.setEllipsoid( QgsProject::instance()->ellipsoid() );
  mConvertToDisplayUnits = true;

  const auto getEllipsoidFriendlyName = [this]()
  {
    // If mDa.ellipsoid is an acronym (e.g "EPSG:7030"), retrieve the user
    // friendly name ("WGS 84 (EPSG:7030)")
    QString ellipsoid = mDa.ellipsoid();
    if ( ellipsoid.contains( ':' ) )
    {
      const auto ellipsoidList = QgsEllipsoidUtils::definitions();
      for ( const auto &ellpsDefinition : ellipsoidList )
      {
        if ( ellpsDefinition.acronym == ellipsoid )
        {
          ellipsoid = ellpsDefinition.description;
          break;
        }
      }
    }
    return ellipsoid;
  };

  if ( mMeasureArea )
  {
    if ( mCartesian->isChecked() || !mCanvas->mapSettings().destinationCrs().isValid() )
    {
      toolTip += "<br> * ";
      if ( mCartesian->isChecked() )
      {
        toolTip += tr( "Cartesian calculation selected, so area is calculated using Cartesian calculations." );
        mConvertToDisplayUnits = true;
      }
      else
      {
        toolTip += tr( "No map projection set, so area is calculated using Cartesian calculations." );
        toolTip += "<br> * " + tr( "Units are unknown." );
        mConvertToDisplayUnits = false;
      }
      mDa.setEllipsoid( geoNone() );
    }
    else if ( mCanvas->mapSettings().destinationCrs().mapUnits() == Qgis::DistanceUnit::Degrees
              && ( mAreaUnits == Qgis::AreaUnit::SquareDegrees || mAreaUnits == Qgis::AreaUnit::Unknown ) )
    {
      //both source and destination units are degrees
      toolTip += "<br> * " + tr( "Both project CRS (%1) and measured area are in degrees, so area is calculated using Cartesian calculations in square degrees." ).arg(
                   mCanvas->mapSettings().destinationCrs().userFriendlyIdentifier() );
      mDa.setEllipsoid( geoNone() );
      mConvertToDisplayUnits = false; //not required since we will be measuring in degrees
    }
    else
    {
      Qgis::AreaUnit resultUnit = Qgis::AreaUnit::Unknown;
      if ( mDa.willUseEllipsoid() )
      {
        resultUnit = Qgis::AreaUnit::SquareMeters;
        toolTip += "<br> * " + tr( "Project ellipsoidal calculation is selected." ) + ' ';
        toolTip += "<br> * " + tr( "The coordinates are transformed to the chosen ellipsoid (%1), and the area is calculated in %2." ).arg( getEllipsoidFriendlyName(),
                   QgsUnitTypes::toString( resultUnit ) );
      }
      else
      {
        resultUnit = QgsUnitTypes::distanceToAreaUnit( mCanvas->mapSettings().destinationCrs().mapUnits() );
        toolTip += "<br> * " + tr( "Project ellipsoidal calculation is not selected." ) + ' ';
        toolTip += tr( "Area is calculated in %1, based on project CRS (%2)." ).arg( QgsUnitTypes::toString( resultUnit ),
                   mCanvas->mapSettings().destinationCrs().userFriendlyIdentifier() );
      }
      setWindowTitle( tr( "Measure" ) );

      if ( QgsUnitTypes::unitType( resultUnit ) == Qgis::DistanceUnitType::Geographic &&
           QgsUnitTypes::unitType( mAreaUnits ) == Qgis::DistanceUnitType::Standard )
      {
        toolTip += QLatin1String( "<br> * Area is roughly converted to square meters by using scale at equator (1 degree = 111319.49 meters)." );
        resultUnit = Qgis::AreaUnit::SquareMeters;
      }
      else if ( QgsUnitTypes::unitType( resultUnit ) == Qgis::DistanceUnitType::Standard &&
                QgsUnitTypes::unitType( mAreaUnits ) == Qgis::DistanceUnitType::Geographic )
      {
        toolTip += QLatin1String( "<br> * Area is roughly converted to square degrees by using scale at equator (1 degree = 111319.49 meters)." );
        resultUnit = Qgis::AreaUnit::SquareDegrees;
      }

      if ( resultUnit != mAreaUnits )
      {
        if ( QgsUnitTypes::unitType( resultUnit ) == Qgis::DistanceUnitType::Standard &&
             QgsUnitTypes::unitType( mAreaUnits ) == Qgis::DistanceUnitType::Standard )
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
    if ( mCartesian->isChecked() || !mCanvas->mapSettings().destinationCrs().isValid() )
    {
      toolTip += "<br> * ";
      if ( mCartesian->isChecked() )
      {
        toolTip += tr( "Cartesian calculation selected, so distance is calculated using Cartesian calculations." );
        mConvertToDisplayUnits = true;
      }
      else
      {
        toolTip += tr( "No map projection set, so distance is calculated using Cartesian calculations." );
        toolTip += "<br> * " + tr( "Units are unknown." );
        mConvertToDisplayUnits = false;
      }
      mDa.setEllipsoid( geoNone() );
    }
    else if ( mCanvas->mapSettings().destinationCrs().mapUnits() == Qgis::DistanceUnit::Degrees
              && mDistanceUnits == Qgis::DistanceUnit::Degrees )
    {
      //both source and destination units are degrees
      toolTip += "<br> * " + tr( "Both project CRS (%1) and measured length are in degrees, so distance is calculated using Cartesian calculations in degrees." ).arg(
                   mCanvas->mapSettings().destinationCrs().userFriendlyIdentifier() );
      mDa.setEllipsoid( geoNone() );
      mConvertToDisplayUnits = false; //not required since we will be measuring in degrees
    }
    else
    {
      Qgis::DistanceUnit resultUnit = Qgis::DistanceUnit::Unknown;
      if ( mDa.willUseEllipsoid() )
      {
        resultUnit = Qgis::DistanceUnit::Meters;
        toolTip += "<br> * " + tr( "Project ellipsoidal calculation is selected." ) + ' ';
        toolTip += "<br> * " + tr( "The coordinates are transformed to the chosen ellipsoid (%1), and the distance is calculated in %2." ).arg( getEllipsoidFriendlyName(),
                   QgsUnitTypes::toString( resultUnit ) );
      }
      else
      {
        resultUnit = mCanvas->mapSettings().destinationCrs().mapUnits();
        toolTip += "<br> * " + tr( "Project ellipsoidal calculation is not selected." ) + ' ';
        toolTip += tr( "Distance is calculated in %1, based on project CRS (%2)." ).arg( QgsUnitTypes::toString( resultUnit ),
                   mCanvas->mapSettings().destinationCrs().userFriendlyIdentifier() );
      }
      setWindowTitle( tr( "Measure" ) );

      if ( QgsUnitTypes::unitType( resultUnit ) == Qgis::DistanceUnitType::Geographic &&
           QgsUnitTypes::unitType( mDistanceUnits ) == Qgis::DistanceUnitType::Standard )
      {
        toolTip += QLatin1String( "<br> * Distance is roughly converted to meters by using scale at equator (1 degree = 111319.49 meters)." );
        resultUnit = Qgis::DistanceUnit::Meters;
      }
      else if ( QgsUnitTypes::unitType( resultUnit ) == Qgis::DistanceUnitType::Standard &&
                QgsUnitTypes::unitType( mDistanceUnits ) == Qgis::DistanceUnitType::Geographic )
      {
        toolTip += QLatin1String( "<br> * Distance is roughly converted to degrees by using scale at equator (1 degree = 111319.49 meters)." );
        resultUnit = Qgis::DistanceUnit::Degrees;
      }

      if ( resultUnit != mDistanceUnits )
      {
        if ( QgsUnitTypes::unitType( resultUnit ) == Qgis::DistanceUnitType::Standard &&
             QgsUnitTypes::unitType( mDistanceUnits ) == Qgis::DistanceUnitType::Standard )
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

  if ( mCanvas->mapSettings().destinationCrs().mapUnits() == Qgis::DistanceUnit::Degrees )
  {
    mDecimalPlacesCoordinates = 5;
  }
  else
  {
    mDecimalPlacesCoordinates = 3;
  }

  editTotal->setToolTip( toolTip );
  mTable->setToolTip( toolTip );
  mNotesLabel->setText( toolTip );

  if ( mMeasureArea )
  {
    if ( mUseMapUnits )
      mUnitsCombo->setCurrentIndex( mUnitsCombo->findData( static_cast<int>( Qgis::AreaUnit::Unknown ) ) );
    else
      mUnitsCombo->setCurrentIndex( mUnitsCombo->findData( static_cast< int >( mAreaUnits ) ) );
  }
  else
  {
    if ( mUseMapUnits )
    {
      mUnitsCombo->setCurrentIndex( mUnitsCombo->findData( static_cast< int >( Qgis::DistanceUnit::Unknown ) ) );
      mTable->headerItem()->setText( Columns::Distance, tr( "Segments [%1]" ).arg( QgsUnitTypes::toString( mMapDistanceUnits ) ) );
    }
    else
    {
      mUnitsCombo->setCurrentIndex( mUnitsCombo->findData( static_cast< int >( mDistanceUnits ) ) );
      if ( mDistanceUnits != Qgis::DistanceUnit::Unknown )
        mTable->headerItem()->setText( Columns::Distance,  tr( "Segments [%1]" ).arg( QgsUnitTypes::toString( mDistanceUnits ) ) );
      else
        mTable->headerItem()->setText( Columns::Distance,  tr( "Segments" ) );
    }
  }

  if ( mMeasureArea )
  {
    double area = 0.0;
    if ( mTool->points().size() > 1 )
    {
      area = mDa.measurePolygon( mTool->points() );
    }
    mTable->hide(); // Hide the table, only show summary
    mSpacer->changeSize( 40, 5, QSizePolicy::Fixed, QSizePolicy::Expanding );
    editTotal->setText( formatArea( area ) );
  }
  else
  {
    // Repopulate the table
    QVector<QgsPointXY>::const_iterator it;
    bool firstPoint = true;

    QgsPointXY previousPoint, point;
    mTotal = 0;
    const QVector< QgsPointXY > tmpPoints = mTool->points();
    for ( it = tmpPoints.constBegin(); it != tmpPoints.constEnd(); ++it )
    {
      point = *it;

      QTreeWidgetItem *item = new QTreeWidgetItem();
      item->setText( Columns::X, QLocale().toString( point.x(), 'f', mDecimalPlacesCoordinates ) );
      item->setText( Columns::Y, QLocale().toString( point.y(), 'f', mDecimalPlacesCoordinates ) );

      if ( !firstPoint )
      {
        double d = -1;
        d = mDa.measureLine( previousPoint, point );
        if ( mConvertToDisplayUnits )
        {
          if ( mDistanceUnits == Qgis::DistanceUnit::Unknown && mMapDistanceUnits != Qgis::DistanceUnit::Unknown )
            d = convertLength( d, mMapDistanceUnits );
          else
            d = convertLength( d, mDistanceUnits );
        }
        item->setText( Columns::Distance, QLocale().toString( d, 'f', mDecimalPlaces ) );
        item->setTextAlignment( Columns::Distance, Qt::AlignRight );
      }

      mTable->addTopLevelItem( item );
      mTable->scrollToItem( item );

      previousPoint = point;
      firstPoint = false;
    }

    mTotal = mDa.measureLine( mTool->points() );
    mTable->show(); // Show the table with items
    mSpacer->changeSize( 40, 5, QSizePolicy::Fixed, QSizePolicy::Maximum );
    editTotal->setText( formatDistance( mTotal, mConvertToDisplayUnits ) );

    if ( !mTool->done() )
    {
      // re-add temporary mouse cursor position
      addPoint();
      mouseMove( mLastMousePoint );
    }
  }
}

void QgsMeasureDialog::repopulateComboBoxUnits( bool isArea )
{
  mUnitsCombo->clear();
  if ( isArea )
  {
    mUnitsCombo->addItem( QgsUnitTypes::toString( Qgis::AreaUnit::SquareMeters ), static_cast< int >( Qgis::AreaUnit::SquareMeters ) );
    mUnitsCombo->addItem( QgsUnitTypes::toString( Qgis::AreaUnit::SquareKilometers ), static_cast< int >( Qgis::AreaUnit::SquareKilometers ) );
    mUnitsCombo->addItem( QgsUnitTypes::toString( Qgis::AreaUnit::SquareFeet ), static_cast< int >( Qgis::AreaUnit::SquareFeet ) );
    mUnitsCombo->addItem( QgsUnitTypes::toString( Qgis::AreaUnit::SquareYards ), static_cast< int >( Qgis::AreaUnit::SquareYards ) );
    mUnitsCombo->addItem( QgsUnitTypes::toString( Qgis::AreaUnit::SquareMiles ), static_cast< int >( Qgis::AreaUnit::SquareMiles ) );
    mUnitsCombo->addItem( QgsUnitTypes::toString( Qgis::AreaUnit::Hectares ), static_cast< int >( Qgis::AreaUnit::Hectares ) );
    mUnitsCombo->addItem( QgsUnitTypes::toString( Qgis::AreaUnit::Acres ), static_cast< int >( Qgis::AreaUnit::Acres ) );
    mUnitsCombo->addItem( QgsUnitTypes::toString( Qgis::AreaUnit::SquareCentimeters ), static_cast< int >( Qgis::AreaUnit::SquareCentimeters ) );
    mUnitsCombo->addItem( QgsUnitTypes::toString( Qgis::AreaUnit::SquareMillimeters ), static_cast< int >( Qgis::AreaUnit::SquareMillimeters ) );
    mUnitsCombo->addItem( QgsUnitTypes::toString( Qgis::AreaUnit::SquareNauticalMiles ), static_cast< int >( Qgis::AreaUnit::SquareNauticalMiles ) );
    mUnitsCombo->addItem( QgsUnitTypes::toString( Qgis::AreaUnit::SquareInches ), static_cast< int >( Qgis::AreaUnit::SquareInches ) );
    mUnitsCombo->addItem( QgsUnitTypes::toString( Qgis::AreaUnit::SquareDegrees ), static_cast< int >( Qgis::AreaUnit::SquareDegrees ) );
    mUnitsCombo->addItem( tr( "map units" ), static_cast< int >( Qgis::AreaUnit::Unknown ) );
  }
  else
  {
    mUnitsCombo->addItem( QgsUnitTypes::toString( Qgis::DistanceUnit::Meters ), static_cast< int >( Qgis::DistanceUnit::Meters ) );
    mUnitsCombo->addItem( QgsUnitTypes::toString( Qgis::DistanceUnit::Kilometers ), static_cast< int >( Qgis::DistanceUnit::Kilometers ) );
    mUnitsCombo->addItem( QgsUnitTypes::toString( Qgis::DistanceUnit::Feet ), static_cast< int >( Qgis::DistanceUnit::Feet ) );
    mUnitsCombo->addItem( QgsUnitTypes::toString( Qgis::DistanceUnit::Yards ), static_cast< int >( Qgis::DistanceUnit::Yards ) );
    mUnitsCombo->addItem( QgsUnitTypes::toString( Qgis::DistanceUnit::Miles ), static_cast< int >( Qgis::DistanceUnit::Miles ) );
    mUnitsCombo->addItem( QgsUnitTypes::toString( Qgis::DistanceUnit::NauticalMiles ), static_cast< int >( Qgis::DistanceUnit::NauticalMiles ) );
    mUnitsCombo->addItem( QgsUnitTypes::toString( Qgis::DistanceUnit::Centimeters ), static_cast< int >( Qgis::DistanceUnit::Centimeters ) );
    mUnitsCombo->addItem( QgsUnitTypes::toString( Qgis::DistanceUnit::Millimeters ), static_cast< int >( Qgis::DistanceUnit::Millimeters ) );
    mUnitsCombo->addItem( QgsUnitTypes::toString( Qgis::DistanceUnit::Inches ), static_cast< int >( Qgis::DistanceUnit::Inches ) );
    mUnitsCombo->addItem( QgsUnitTypes::toString( Qgis::DistanceUnit::Degrees ), static_cast< int >( Qgis::DistanceUnit::Degrees ) );
    mUnitsCombo->addItem( tr( "map units" ), static_cast< int >( Qgis::DistanceUnit::Unknown ) );
  }
}

double QgsMeasureDialog::convertLength( double length, Qgis::DistanceUnit toUnit ) const
{
  return mDa.convertLengthMeasurement( length, toUnit );
}

double QgsMeasureDialog::convertArea( double area, Qgis::AreaUnit toUnit ) const
{
  return mDa.convertAreaMeasurement( area, toUnit );
}

void QgsMeasureDialog::copyMeasurements()
{
  const bool includeHeader = settingClipboardHeader->value();
  const bool alwaysUseDecimalPoint = settingClipboardAlwaysUseDecimalPoint->value();

  // Get the separator
  QString separator = settingClipboardSeparator->value();

  // If the field separator is a comma and the locale uses a comma as decimal separator, change to a semicolon
  if ( separator == QLatin1String( "," ) && !alwaysUseDecimalPoint && QLocale().decimalPoint() == QLatin1String( "," ) )
    separator = QStringLiteral( ";" );

  if ( separator.isEmpty() )
    separator = QStringLiteral( "\t" );

  QClipboard *clipboard = QApplication::clipboard();
  QString text;
  QTreeWidgetItemIterator it( mTable );

  if ( includeHeader )
  {
    text += mTable->headerItem()->text( Columns::X ) + separator;
    text += mTable->headerItem()->text( Columns::Y ) + separator;
    text += mTable->headerItem()->text( Columns::Distance ) + QStringLiteral( "\n" );
  }


  auto replaceDecimalSeparator = [ alwaysUseDecimalPoint ]( const QString & value ) -> QString
  {
    QString result = value;
    if ( alwaysUseDecimalPoint && QLocale().decimalPoint() != QLatin1String( "." ) )
      result.replace( QLocale().decimalPoint(), QStringLiteral( "." ) );
    return result;
  };

  while ( *it )
  {
    text += replaceDecimalSeparator( ( *it )->text( Columns::X ) ) + separator;
    text += replaceDecimalSeparator( ( *it )->text( Columns::Y ) ) + separator;
    text += replaceDecimalSeparator( ( *it )->text( Columns::Distance ) ) + QStringLiteral( "\n" );
    it++;
  }

  clipboard->setText( text );

  // Display a message to the user
  QgisApp::instance()->messageBar()->pushInfo( tr( "Measure" ), tr( "Measurements copied to clipboard" ) );
}

void QgsMeasureDialog::reject()
{
  saveWindowLocation();
  restart();
  QDialog::close();
}

void QgsMeasureDialog::showHelp()
{
  QgsHelp::openHelp( QStringLiteral( "map_views/map_view.html#sec-measure" ) );
}
