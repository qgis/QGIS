/***************************************************************************
                            qgsprojectproperties.cpp
       Set various project properties (inherits qgsprojectpropertiesbase)
                              -------------------
  begin                : May 18, 2004
  copyright            : (C) 2004 by Gary E.Sherman
  email                : sherman at mrcc.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsprojectproperties.h"

//qgis includes
#include "qgscontexthelp.h"
#include "qgscoordinatetransform.h"
#include "qgslogger.h"
#include "qgsmapcanvas.h"
#include "qgsmaplayer.h"
#include "qgsmaplayerregistry.h"
#include "qgsmaprenderer.h"
#include "qgsproject.h"
#include "qgsrenderer.h"
#include "qgssnappingdialog.h"
#include "qgsrasterlayer.h"
#include "qgsgenericprojectionselector.h"

//qt includes
#include <QColorDialog>
#include <QHeaderView>  // Qt 4.4
#include <QMessageBox>

//stdc++ includes


QgsProjectProperties::QgsProjectProperties( QgsMapCanvas* mapCanvas, QWidget *parent, Qt::WFlags fl )
    : QDialog( parent, fl )
    , mMapCanvas( mapCanvas )
{
  setupUi( this );
  connect( buttonBox, SIGNAL( accepted() ), this, SLOT( accept() ) );
  connect( buttonBox, SIGNAL( rejected() ), this, SLOT( reject() ) );
  connect( buttonBox->button( QDialogButtonBox::Apply ), SIGNAL( clicked() ), this, SLOT( apply() ) );
  connect( this, SIGNAL( accepted() ), this, SLOT( apply() ) );
  connect( projectionSelector, SIGNAL( sridSelected( QString ) ), this, SLOT( setMapUnitsToCurrentProjection() ) );

  ///////////////////////////////////////////////////////////
  // Properties stored in map canvas's QgsMapRenderer
  // these ones are propagated to QgsProject by a signal

  QgsMapRenderer* myRenderer = mMapCanvas->mapRenderer();
  QGis::UnitType myUnit = myRenderer->mapUnits();
  setMapUnits( myUnit );

  //see if the user wants on the fly projection enabled
  bool myProjectionEnabled = myRenderer->hasCrsTransformEnabled();
  cbxProjectionEnabled->setChecked( myProjectionEnabled );
  btnGrpMapUnits->setEnabled( !myProjectionEnabled );

  mProjectSrsId = myRenderer->destinationCrs().srsid();
  QgsDebugMsg( "Read project CRSID: " + QString::number( mProjectSrsId ) );
  projectionSelector->setSelectedCrsId( mProjectSrsId );

  ///////////////////////////////////////////////////////////
  // Properties stored in QgsProject

  title( QgsProject::instance()->title() );

  // get the manner in which the number of decimal places in the mouse
  // position display is set (manual or automatic)
  bool automaticPrecision = QgsProject::instance()->readBoolEntry( "PositionPrecision", "/Automatic" );
  if ( automaticPrecision )
  {
    radAutomatic->setChecked( true );
    spinBoxDP->setDisabled( true );
    labelDP->setDisabled( true );
  }
  else
  {
    radManual->setChecked( true );
  }

  cbxAbsolutePath->setCurrentIndex( QgsProject::instance()->readBoolEntry( "Paths", "/Absolute", true ) ? 0 : 1 );

  int dp = QgsProject::instance()->readNumEntry( "PositionPrecision", "/DecimalPlaces" );
  spinBoxDP->setValue( dp );

  //get the color selections and set the button color accordingly
  int myRedInt = QgsProject::instance()->readNumEntry( "Gui", "/SelectionColorRedPart", 255 );
  int myGreenInt = QgsProject::instance()->readNumEntry( "Gui", "/SelectionColorGreenPart", 255 );
  int myBlueInt = QgsProject::instance()->readNumEntry( "Gui", "/SelectionColorBluePart", 0 );
  int myAlphaInt = QgsProject::instance()->readNumEntry( "Gui", "/SelectionColorAlphaPart", 255 );
  QColor myColor = QColor( myRedInt, myGreenInt, myBlueInt, myAlphaInt );
  pbnSelectionColor->setColor( myColor );

  //get the color for map canvas background and set button color accordingly (default white)
  myRedInt = QgsProject::instance()->readNumEntry( "Gui", "/CanvasColorRedPart", 255 );
  myGreenInt = QgsProject::instance()->readNumEntry( "Gui", "/CanvasColorGreenPart", 255 );
  myBlueInt = QgsProject::instance()->readNumEntry( "Gui", "/CanvasColorBluePart", 255 );
  myColor = QColor( myRedInt, myGreenInt, myBlueInt );
  pbnCanvasColor->setColor( myColor );

  QgsMapLayer* currentLayer = 0;

  QStringList noIdentifyLayerIdList = QgsProject::instance()->readListEntry( "Identify", "/disabledLayers" );

  const QMap<QString, QgsMapLayer*> &mapLayers = QgsMapLayerRegistry::instance()->mapLayers();

  if ( mMapCanvas->currentLayer() )
  {
    mLayerSrsId = mMapCanvas->currentLayer()->crs().srsid();
  }
  else if ( mapLayers.size() > 0 )
  {
    mLayerSrsId = mapLayers.begin().value()->crs().srsid();
  }
  else
  {
    mLayerSrsId = mProjectSrsId;
  }

  twIdentifyLayers->setColumnCount( 3 );
  twIdentifyLayers->horizontalHeader()->setVisible( true );
  twIdentifyLayers->setHorizontalHeaderItem( 0, new QTableWidgetItem( tr( "Layer" ) ) );
  twIdentifyLayers->setHorizontalHeaderItem( 1, new QTableWidgetItem( tr( "Type" ) ) );
  twIdentifyLayers->setHorizontalHeaderItem( 2, new QTableWidgetItem( tr( "Identifiable" ) ) );
  twIdentifyLayers->setRowCount( mapLayers.size() );
  twIdentifyLayers->verticalHeader()->setResizeMode( QHeaderView::ResizeToContents );

  int i = 0;
  for ( QMap<QString, QgsMapLayer*>::const_iterator it = mapLayers.constBegin(); it != mapLayers.constEnd(); it++, i++ )
  {
    currentLayer = it.value();

    QTableWidgetItem *twi = new QTableWidgetItem( QString::number( i ) );
    twIdentifyLayers->setVerticalHeaderItem( i, twi );

    twi = new QTableWidgetItem( currentLayer->name() );
    twi->setData( Qt::UserRole, it.key() );
    twi->setFlags( twi->flags() & ~Qt::ItemIsEditable );
    twIdentifyLayers->setItem( i, 0, twi );

    QString type;
    if ( currentLayer->type() == QgsMapLayer::VectorLayer )
    {
      type = tr( "Vector" );
    }
    else if ( currentLayer->type() == QgsMapLayer::RasterLayer )
    {
      QgsRasterLayer *rl = qobject_cast<QgsRasterLayer *>( currentLayer );

      if ( rl && rl->providerType() == "wms" )
      {
        type = tr( "WMS" );
      }
      else
      {
        type = tr( "Raster" );
      }
    }

    twi = new QTableWidgetItem( type );
    twi->setFlags( twi->flags() & ~Qt::ItemIsEditable );
    twIdentifyLayers->setItem( i, 1, twi );

    QCheckBox *cb = new QCheckBox();
    cb->setChecked( !noIdentifyLayerIdList.contains( currentLayer->id() ) );
    twIdentifyLayers->setCellWidget( i, 2, cb );
  }

  grpWMSServiceCapabilities->setChecked( QgsProject::instance()->readBoolEntry( "WMSServiceCapabilities", "/", false ) );
  mWMSTitle->setText( QgsProject::instance()->readEntry( "WMSServiceTitle", "/" ) );
  mWMSContactOrganization->setText( QgsProject::instance()->readEntry( "WMSContactOrganization", "/", "" ) );
  mWMSContactPerson->setText( QgsProject::instance()->readEntry( "WMSContactPerson", "/", "" ) );
  mWMSContactMail->setText( QgsProject::instance()->readEntry( "WMSContactMail", "/", "" ) );
  mWMSContactPhone->setText( QgsProject::instance()->readEntry( "WMSContactPhone", "/", "" ) );
  mWMSAbstract->setPlainText( QgsProject::instance()->readEntry( "WMSServiceAbstract", "/", "" ) );
  mWMSOnlineResourceLineEdit->setText( QgsProject::instance()->readEntry( "WMSOnlineResource", "/", "" ) );

  bool ok;
  QStringList values;

  mWMSExtMinX->setValidator( new QDoubleValidator( mWMSExtMinX ) );
  mWMSExtMinY->setValidator( new QDoubleValidator( mWMSExtMinY ) );
  mWMSExtMaxX->setValidator( new QDoubleValidator( mWMSExtMaxX ) );
  mWMSExtMaxY->setValidator( new QDoubleValidator( mWMSExtMaxY ) );

  values = QgsProject::instance()->readListEntry( "WMSExtent", "/", &ok );
  grpWMSExt->setChecked( ok && values.size() == 4 );
  if ( grpWMSExt->isChecked() )
  {
    mWMSExtMinX->setText( values[0] );
    mWMSExtMinY->setText( values[1] );
    mWMSExtMaxX->setText( values[2] );
    mWMSExtMaxY->setText( values[3] );
  }

  values = QgsProject::instance()->readListEntry( "WMSCrsList", "/", &ok );
  grpWMSList->setChecked( ok && values.size() > 0 );
  if ( grpWMSList->isChecked() )
  {
    mWMSList->addItems( values );
  }
  else
  {
    values = QgsProject::instance()->readListEntry( "WMSEpsgList", "/", &ok );
    grpWMSList->setChecked( ok && values.size() > 0 );
    if ( grpWMSList->isChecked() )
    {
      QStringList list;
      foreach( QString value, values )
      {
        list << QString( "EPSG:%1" ).arg( value );
      }

      mWMSList->addItems( list );
    }
  }

  grpWMSList->setChecked( mWMSList->count() > 0 );

  bool addWktGeometry = QgsProject::instance()->readBoolEntry( "WMSAddWktGeometry", "/" );
  mAddWktGeometryCheckBox->setChecked( addWktGeometry );

  restoreState();
}

QgsProjectProperties::~QgsProjectProperties()
{
  saveState();
}



// return the map units
QGis::UnitType QgsProjectProperties::mapUnits() const
{
  return mMapCanvas->mapRenderer()->mapUnits();
}


void QgsProjectProperties::setMapUnits( QGis::UnitType unit )
{
  // select the button
  if ( unit == QGis::UnknownUnit )
  {
    unit = QGis::Meters;
  }
  if ( unit == QGis::Meters )
  {
    radMeters->setChecked( true );
  }
  else if ( unit == QGis::Feet )
  {
    radFeet->setChecked( true );
  }
  else if ( unit == QGis::DegreesMinutesSeconds )
  {
    radDMS->setChecked( true );
  }
  else
  {
    radDecimalDegrees->setChecked( true );
  }
  mMapCanvas->mapRenderer()->setMapUnits( unit );
}


QString QgsProjectProperties::title() const
{
  return titleEdit->text();
} //  QgsProjectPropertires::title() const


void QgsProjectProperties::title( QString const & title )
{
  titleEdit->setText( title );
  QgsProject::instance()->title( title );
} // QgsProjectProperties::title( QString const & title )



//when user clicks apply button
void QgsProjectProperties::apply()
{
  // Set the map units
  // Note. Qt 3.2.3 and greater have a function selectedId() that
  // can be used instead of the two part technique here
  QGis::UnitType mapUnit;
  if ( radMeters->isChecked() )
  {
    mapUnit = QGis::Meters;
  }
  else if ( radFeet->isChecked() )
  {
    mapUnit = QGis::Feet;
  }
  else if ( radDMS->isChecked() )
  {
    mapUnit = QGis::DegreesMinutesSeconds;
  }
  else
  {
    mapUnit = QGis::Degrees;
  }

  QgsMapRenderer* myRenderer = mMapCanvas->mapRenderer();
  myRenderer->setMapUnits( mapUnit );
  myRenderer->setProjectionsEnabled( cbxProjectionEnabled->isChecked() );

  // Only change the projection if there is a node in the tree
  // selected that has an srid. This prevents error if the user
  // selects a top-level node rather than an actual coordinate
  // system
  long myCRSID = projectionSelector->selectedCrsId();
  if ( myCRSID )
  {
    QgsCoordinateReferenceSystem srs( myCRSID, QgsCoordinateReferenceSystem::InternalCrsId );
    myRenderer->setDestinationCrs( srs );
    QgsDebugMsg( QString( "Selected CRS " ) + srs.description() );
    // write the currently selected projections _proj string_ to project settings
    QgsDebugMsg( QString( "SpatialRefSys/ProjectCRSProj4String: %1" ).arg( projectionSelector->selectedProj4String() ) );
    QgsProject::instance()->writeEntry( "SpatialRefSys", "/ProjectCRSProj4String", projectionSelector->selectedProj4String() );

    // Set the map units to the projected coordinates if we are projecting
    if ( isProjected() )
    {
      // If we couldn't get the map units, default to the value in the
      // projectproperties dialog box (set above)
      if ( srs.mapUnits() != QGis::UnknownUnit )
        myRenderer->setMapUnits( srs.mapUnits() );
    }
  }

  // Set the project title
  QgsProject::instance()->title( title() );

  // set the mouse display precision method and the
  // number of decimal places for the manual option
  // Note. Qt 3.2.3 and greater have a function selectedId() that
  // can be used instead of the two part technique here
  QgsProject::instance()->writeEntry( "PositionPrecision", "/Automatic", radAutomatic->isChecked() );
  QgsProject::instance()->writeEntry( "PositionPrecision", "/DecimalPlaces", spinBoxDP->value() );
  // Announce that we may have a new display precision setting
  emit displayPrecisionChanged();

  QgsProject::instance()->writeEntry( "Paths", "/Absolute", cbxAbsolutePath->currentIndex() == 0 );

  //set the color for selections
  QColor myColor = pbnSelectionColor->color();
  QgsProject::instance()->writeEntry( "Gui", "/SelectionColorRedPart", myColor.red() );
  QgsProject::instance()->writeEntry( "Gui", "/SelectionColorGreenPart", myColor.green() );
  QgsProject::instance()->writeEntry( "Gui", "/SelectionColorBluePart", myColor.blue() );
  QgsProject::instance()->writeEntry( "Gui", "/SelectionColorAlphaPart", myColor.alpha() );
  QgsRenderer::setSelectionColor( myColor );

  //set the color for canvas
  myColor = pbnCanvasColor->color();
  QgsProject::instance()->writeEntry( "Gui", "/CanvasColorRedPart", myColor.red() );
  QgsProject::instance()->writeEntry( "Gui", "/CanvasColorGreenPart", myColor.green() );
  QgsProject::instance()->writeEntry( "Gui", "/CanvasColorBluePart", myColor.blue() );

  QStringList noIdentifyLayerList;
  for ( int i = 0; i < twIdentifyLayers->rowCount(); i++ )
  {
    QCheckBox *cb = qobject_cast<QCheckBox *>( twIdentifyLayers->cellWidget( i, 2 ) );
    if ( cb && !cb->isChecked() )
    {
      QString id = twIdentifyLayers->item( i, 0 )->data( Qt::UserRole ).toString();
      noIdentifyLayerList << id;
    }
  }

  QgsProject::instance()->writeEntry( "Identify", "/disabledLayers", noIdentifyLayerList );

  QgsProject::instance()->writeEntry( "WMSServiceCapabilities", "/", grpWMSServiceCapabilities->isChecked() );
  QgsProject::instance()->writeEntry( "WMSServiceTitle", "/", mWMSTitle->text() );
  QgsProject::instance()->writeEntry( "WMSContactOrganization", "/", mWMSContactOrganization->text() );
  QgsProject::instance()->writeEntry( "WMSContactPerson", "/", mWMSContactPerson->text() );
  QgsProject::instance()->writeEntry( "WMSContactMail", "/", mWMSContactMail->text() );
  QgsProject::instance()->writeEntry( "WMSContactPhone", "/", mWMSContactPhone->text() );
  QgsProject::instance()->writeEntry( "WMSServiceAbstract", "/", mWMSAbstract->toPlainText() );
  QgsProject::instance()->writeEntry( "WMSOnlineResource", "/", mWMSOnlineResourceLineEdit->text() );

  if ( grpWMSExt->isChecked() )
  {
    QgsProject::instance()->writeEntry( "WMSExtent", "/",
                                        QStringList()
                                        << mWMSExtMinX->text()
                                        << mWMSExtMinY->text()
                                        << mWMSExtMaxX->text()
                                        << mWMSExtMaxY->text() );
  }
  else
  {
    QgsProject::instance()->removeEntry( "WMSExtent", "/" );
  }

  if ( grpWMSList->isChecked() && mWMSList->count() == 0 )
  {
    QMessageBox::information( this, tr( "Coordinate System Restriction" ), tr( "No coordinate systems selected. Disabling restriction." ) );
    grpWMSList->setChecked( false );
  }

  QgsProject::instance()->removeEntry( "WMSEpsgList", "/" );

  if ( grpWMSList->isChecked() )
  {
    QStringList crslist;
    for ( int i = 0; i < mWMSList->count(); i++ )
    {
      crslist << mWMSList->item( i )->text();
    }

    QgsProject::instance()->writeEntry( "WMSCrsList", "/", crslist );
  }
  else
  {
    QgsProject::instance()->removeEntry( "WMSCrsList", "/" );
  }

  QgsProject::instance()->writeEntry( "WMSAddWktGeometry", "/", mAddWktGeometryCheckBox->isChecked() );

  //todo XXX set canvas color
  emit refresh();
}

bool QgsProjectProperties::isProjected()
{
  return cbxProjectionEnabled->isChecked();
}

void QgsProjectProperties::showProjectionsTab()
{
  tabWidget->setCurrentIndex( 1 );
}

void QgsProjectProperties::on_pbnSelectionColor_clicked()
{
#if QT_VERSION >= 0x040500
  QColor color = QColorDialog::getColor( pbnSelectionColor->color(), 0, tr( "Selection color" ), QColorDialog::ShowAlphaChannel );
#else
  QColor color = QColorDialog::getColor( pbnSelectionColor->color() );
#endif

  if ( color.isValid() )
  {
    pbnSelectionColor->setColor( color );
  }
}

void QgsProjectProperties::on_pbnCanvasColor_clicked()
{
  QColor color = QColorDialog::getColor( pbnCanvasColor->color(), this );
  if ( color.isValid() )
  {
    pbnCanvasColor->setColor( color );
  }
}

void QgsProjectProperties::on_cbxProjectionEnabled_stateChanged( int state )
{
  btnGrpMapUnits->setEnabled( state == Qt::Unchecked );

  if ( state != Qt::Checked )
  {
    mProjectSrsId = projectionSelector->selectedCrsId();
    projectionSelector->setSelectedCrsId( mLayerSrsId );
  }
  else
  {
    mLayerSrsId = projectionSelector->selectedCrsId();
    projectionSelector->setSelectedCrsId( mProjectSrsId );
  }
}

void QgsProjectProperties::setMapUnitsToCurrentProjection()
{
  long myCRSID = projectionSelector->selectedCrsId();
  if ( myCRSID )
  {
    QgsCoordinateReferenceSystem srs( myCRSID, QgsCoordinateReferenceSystem::InternalCrsId );
    //set radio button to crs map unit type
    QGis::UnitType units = srs.mapUnits();
    switch ( units )
    {
      case QGis::Meters:
        radMeters->setChecked( true );
        break;
      case QGis::Feet:
        radFeet->setChecked( true );
        break;
      case QGis::Degrees:
        radDecimalDegrees->setChecked( true );
        break;
      case QGis::DegreesMinutesSeconds:
        radDMS->setChecked( true );
        break;
      default:
        break;
    }
  }
}

/*!
 * Function to save dialog window state
 */
void QgsProjectProperties::saveState()
{
  QSettings settings;
  settings.setValue( "/Windows/ProjectProperties/geometry", saveGeometry() );
  settings.setValue( "/Windows/ProjectProperties/tab", tabWidget->currentIndex() );
}

/*!
 * Function to restore dialog window state
 */
void QgsProjectProperties::restoreState()
{
  QSettings settings;
  restoreGeometry( settings.value( "/Windows/ProjectProperties/geometry" ).toByteArray() );
  tabWidget->setCurrentIndex( settings.value( "/Windows/ProjectProperties/tab" ).toInt() );
}

/*!
 * Set WMS default extent to current canvas extent
 */
void QgsProjectProperties::on_pbnWMSExtCanvas_clicked()
{
  QgsRectangle ext = mMapCanvas->extent();
  mWMSExtMinX->setText( QString::number( ext.xMinimum(), 'f', 8 ) );
  mWMSExtMinY->setText( QString::number( ext.yMinimum(), 'f', 8 ) );
  mWMSExtMaxX->setText( QString::number( ext.xMaximum(), 'f', 8 ) );
  mWMSExtMaxY->setText( QString::number( ext.yMaximum(), 'f', 8 ) );
}

void QgsProjectProperties::on_pbnWMSAddSRS_clicked()
{
  QgsGenericProjectionSelector *mySelector = new QgsGenericProjectionSelector( this );
  mySelector->setMessage();
  if ( mySelector->exec() )
  {
    QString authid = mySelector->selectedAuthId();

    QList<QListWidgetItem *> items = mWMSList->findItems( authid.mid( 5 ), Qt::MatchFixedString );
    if ( items.size() == 0 )
    {
      mWMSList->addItem( authid );
    }
    else
    {
      QMessageBox::information( this, tr( "Coordinate System Restriction" ), tr( "CRS %1 was already selected" ).arg( authid ) );
    }
  }

  delete mySelector;
}

void QgsProjectProperties::on_pbnWMSRemoveSRS_clicked()
{
  foreach( QListWidgetItem *item, mWMSList->selectedItems() )
  {
    delete item;
  }
}

void QgsProjectProperties::on_pbnWMSSetUsedSRS_clicked()
{
  if ( mWMSList->count() > 1 )
  {
    if ( QMessageBox::question( this,
                                tr( "Coordinate System Restrictions" ),
                                tr( "The current selection of coordinate systems will be lost.\nProceed?" ) ) == QMessageBox::No )
      return;
  }

  QSet<QString> crsList;

  if ( cbxProjectionEnabled->isChecked() )
  {
    QgsCoordinateReferenceSystem srs( projectionSelector->selectedCrsId(), QgsCoordinateReferenceSystem::InternalCrsId );
    crsList << srs.authid();
  }

  const QMap<QString, QgsMapLayer*> &mapLayers = QgsMapLayerRegistry::instance()->mapLayers();
  for ( QMap<QString, QgsMapLayer*>::const_iterator it = mapLayers.constBegin(); it != mapLayers.constEnd(); it++ )
  {
    crsList << it.value()->crs().authid();
  }

  mWMSList->clear();
  mWMSList->addItems( crsList.values() );
}
