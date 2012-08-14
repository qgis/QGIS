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
#include "qgsscaleutils.h"
#include "qgsgenericprojectionselector.h"
#include "qgsstylev2.h"
#include "qgssymbolv2.h"
#include "qgsstylev2managerdialog.h"
#include "qgsvectorcolorrampv2.h"
#include "qgssymbolv2selectordialog.h"

//qt includes
#include <QColorDialog>
#include <QInputDialog>
#include <QFileDialog>
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

  mProjectSrsId = myRenderer->destinationCrs().srsid();
  QgsDebugMsg( "Read project CRSID: " + QString::number( mProjectSrsId ) );
  projectionSelector->setSelectedCrsId( mProjectSrsId );
  projectionSelector->setEnabled( myProjectionEnabled );

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

  QString format = QgsProject::instance()->readEntry( "PositionPrecision", "/DegreeFormat", "D" );
  if ( format == "DM" )
    radDM->setChecked( true );
  else if ( format == "DMS" )
    radDMS->setChecked( true );
  else
    radD->setChecked( true );

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

  //get project scales
  QStringList myScales = QgsProject::instance()->readListEntry( "Scales", "/ScalesList" );
  if ( !myScales.isEmpty() )
  {
    QStringList::const_iterator scaleIt = myScales.constBegin();
    for ( ; scaleIt != myScales.constEnd(); ++scaleIt )
    {
      QListWidgetItem* newItem = new QListWidgetItem( lstScales );
      newItem->setText( *scaleIt );
      newItem->setFlags( Qt::ItemIsEditable | Qt::ItemIsEnabled | Qt::ItemIsSelectable );
      lstScales->addItem( newItem );
    }
  }

  grpProjectScales->setChecked( QgsProject::instance()->readBoolEntry( "Scales", "/useProjectScales" ) );

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

  grpOWSServiceCapabilities->setChecked( QgsProject::instance()->readBoolEntry( "WMSServiceCapabilities", "/", false ) );
  mWMSTitle->setText( QgsProject::instance()->readEntry( "WMSServiceTitle", "/" ) );
  mWMSContactOrganization->setText( QgsProject::instance()->readEntry( "WMSContactOrganization", "/", "" ) );
  mWMSContactPerson->setText( QgsProject::instance()->readEntry( "WMSContactPerson", "/", "" ) );
  mWMSContactMail->setText( QgsProject::instance()->readEntry( "WMSContactMail", "/", "" ) );
  mWMSContactPhone->setText( QgsProject::instance()->readEntry( "WMSContactPhone", "/", "" ) );
  mWMSAbstract->setPlainText( QgsProject::instance()->readEntry( "WMSServiceAbstract", "/", "" ) );
  mWMSOnlineResourceLineEdit->setText( QgsProject::instance()->readEntry( "WMSOnlineResource", "/", "" ) );
  mWMSUrlLineEdit->setText( QgsProject::instance()->readEntry( "WMSUrl", "/", "" ) );

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
      foreach ( QString value, values )
      {
        list << QString( "EPSG:%1" ).arg( value );
      }

      mWMSList->addItems( list );
    }
  }

  grpWMSList->setChecked( mWMSList->count() > 0 );

  bool addWktGeometry = QgsProject::instance()->readBoolEntry( "WMSAddWktGeometry", "/" );
  mAddWktGeometryCheckBox->setChecked( addWktGeometry );

  //WMS maxWidth / maxHeight
  mMaxWidthLineEdit->setValidator( new QIntValidator( mMaxWidthLineEdit ) );
  int maxWidth = QgsProject::instance()->readNumEntry( "WMSMaxWidth", "/", -1 );
  if ( maxWidth != -1 )
  {
    mMaxWidthLineEdit->setText( QString::number( maxWidth ) );
  }
  mMaxHeightLineEdit->setValidator( new QIntValidator( mMaxHeightLineEdit ) );
  int maxHeight = QgsProject::instance()->readNumEntry( "WMSMaxHeight", "/", -1 );
  if ( maxHeight != -1 )
  {
    mMaxHeightLineEdit->setText( QString::number( maxHeight ) );
  }

  QStringList wfsLayerIdList = QgsProject::instance()->readListEntry( "WFSLayers", "/" );

  twWFSLayers->setColumnCount( 2 );
  twWFSLayers->horizontalHeader()->setVisible( true );
  twWFSLayers->setRowCount( mapLayers.size() );

  i = 0;
  int j = 0;
  for ( QMap<QString, QgsMapLayer*>::const_iterator it = mapLayers.constBegin(); it != mapLayers.constEnd(); it++, i++ )
  {
    currentLayer = it.value();
    if ( currentLayer->type() == QgsMapLayer::VectorLayer )
    {

      QTableWidgetItem *twi = new QTableWidgetItem( QString::number( j ) );
      twWFSLayers->setVerticalHeaderItem( j, twi );

      twi = new QTableWidgetItem( currentLayer->name() );
      twi->setData( Qt::UserRole, it.key() );
      twi->setFlags( twi->flags() & ~Qt::ItemIsEditable );
      twWFSLayers->setItem( j, 0, twi );

      QCheckBox *cb = new QCheckBox();
      cb->setChecked( wfsLayerIdList.contains( currentLayer->id() ) );
      twWFSLayers->setCellWidget( j, 1, cb );
      j++;

    }
  }
  twWFSLayers->setRowCount( j );
  twWFSLayers->verticalHeader()->setResizeMode( QHeaderView::ResizeToContents );

  // Default Styles
  mStyle = QgsStyleV2::defaultStyle();
  populateStyles();

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

  radMeters->setChecked( unit == QGis::Meters );
  radFeet->setChecked( unit == QGis::Feet );
  radDegrees->setChecked( unit == QGis::Degrees );

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
  if ( radDegrees->isChecked() )
  {
    mapUnit = QGis::Degrees;
  }
  else if ( radFeet->isChecked() )
  {
    mapUnit = QGis::Feet;
  }
  else
  {
    mapUnit = QGis::Meters;
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
  QgsProject::instance()->writeEntry( "PositionPrecision", "/DegreeFormat",
                                      QString( radDM->isChecked() ? "DM" : radDMS->isChecked() ? "DMS" : "D" ) );

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

  //save project scales
  QStringList myScales;
  for ( int i = 0; i < lstScales->count(); ++i )
  {
    myScales.append( lstScales->item( i )->text() );
  }

  if ( !myScales.isEmpty() )
  {
    QgsProject::instance()->writeEntry( "Scales", "/ScalesList", myScales );
    QgsProject::instance()->writeEntry( "Scales", "/useProjectScales", grpProjectScales->isChecked() );
  }
  else
  {
    QgsProject::instance()->removeEntry( "Scales", "/" );
  }

  //use global or project scales depending on checkbox state
  if ( grpProjectScales->isChecked() )
  {
    emit scalesChanged( myScales );
  }
  else
  {
    emit scalesChanged();
  }

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

  QgsProject::instance()->writeEntry( "WMSServiceCapabilities", "/", grpOWSServiceCapabilities->isChecked() );
  QgsProject::instance()->writeEntry( "WMSServiceTitle", "/", mWMSTitle->text() );
  QgsProject::instance()->writeEntry( "WMSContactOrganization", "/", mWMSContactOrganization->text() );
  QgsProject::instance()->writeEntry( "WMSContactPerson", "/", mWMSContactPerson->text() );
  QgsProject::instance()->writeEntry( "WMSContactMail", "/", mWMSContactMail->text() );
  QgsProject::instance()->writeEntry( "WMSContactPhone", "/", mWMSContactPhone->text() );
  QgsProject::instance()->writeEntry( "WMSServiceAbstract", "/", mWMSAbstract->toPlainText() );
  QgsProject::instance()->writeEntry( "WMSOnlineResource", "/", mWMSOnlineResourceLineEdit->text() );
  QgsProject::instance()->writeEntry( "WMSUrl", "/", mWMSUrlLineEdit->text() );

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

  QString maxWidthText = mMaxWidthLineEdit->text();
  if ( maxWidthText.isEmpty() )
  {
    QgsProject::instance()->removeEntry( "WMSMaxWidth", "/" );
  }
  else
  {
    QgsProject::instance()->writeEntry( "WMSMaxWidth", "/", maxWidthText.toInt() );
  }
  QString maxHeightText = mMaxHeightLineEdit->text();
  if ( maxHeightText.isEmpty() )
  {
    QgsProject::instance()->removeEntry( "WMSMaxHeight", "/" );
  }
  else
  {
    QgsProject::instance()->writeEntry( "WMSMaxHeight", "/", maxHeightText.toInt() );
  }

  QStringList wfsLayerList;
  for ( int i = 0; i < twWFSLayers->rowCount(); i++ )
  {
    QCheckBox *cb = qobject_cast<QCheckBox *>( twWFSLayers->cellWidget( i, 1 ) );
    if ( cb && cb->isChecked() )
    {
      QString id = twWFSLayers->item( i, 0 )->data( Qt::UserRole ).toString();
      wfsLayerList << id;
    }
  }
  QgsProject::instance()->writeEntry( "WFSLayers", "/", wfsLayerList );

  // Default Styles
  QgsProject::instance()->writeEntry( "DefaultStyles", "/Marker", cboStyleMarker->currentText() );
  QgsProject::instance()->writeEntry( "DefaultStyles", "/Line", cboStyleLine->currentText() );
  QgsProject::instance()->writeEntry( "DefaultStyles", "/Fill", cboStyleFill->currentText() );
  QgsProject::instance()->writeEntry( "DefaultStyles", "/ColorRamp", cboStyleColorRamp->currentText() );
  QgsProject::instance()->writeEntry( "DefaultStyles", "/AlphaInt", 255 - mTransparencySlider->value() );
  QgsProject::instance()->writeEntry( "DefaultStyles", "/RandomColors", cbxStyleRandomColors->isChecked() );

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
  projectionSelector->setEnabled( state == Qt::Checked );

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

    radMeters->setChecked( units == QGis::Meters );
    radFeet->setChecked( units == QGis::Feet );
    radDegrees->setChecked( units == QGis::Degrees );
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
  foreach ( QListWidgetItem *item, mWMSList->selectedItems() )
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

void QgsProjectProperties::on_pbnAddScale_clicked()
{
  int myScale = QInputDialog::getInt(
                  this,
                  tr( "Enter scale" ),
                  tr( "Scale denominator" ),
                  -1,
                  1
                );

  if ( myScale != -1 )
  {
    QListWidgetItem* newItem = new QListWidgetItem( lstScales );
    newItem->setText( QString( "1:%1" ).arg( myScale ) );
    newItem->setFlags( Qt::ItemIsEditable | Qt::ItemIsEnabled | Qt::ItemIsSelectable );
    lstScales->addItem( newItem );
    lstScales->setCurrentItem( newItem );
  }
}

void QgsProjectProperties::on_pbnRemoveScale_clicked()
{
  int currentRow = lstScales->currentRow();
  QListWidgetItem* itemToRemove = lstScales->takeItem( currentRow );
  delete itemToRemove;
}

void QgsProjectProperties::on_pbnImportScales_clicked()
{
  QString fileName = QFileDialog::getOpenFileName( this, tr( "Load scales" ), ".",
                     tr( "XML files (*.xml *.XML)" ) );
  if ( fileName.isEmpty() )
  {
    return;
  }

  QString msg;
  QStringList myScales;
  if ( !QgsScaleUtils::loadScaleList( fileName, myScales, msg ) )
  {
    QgsDebugMsg( msg );
  }

  QStringList::const_iterator scaleIt = myScales.constBegin();
  for ( ; scaleIt != myScales.constEnd(); ++scaleIt )
  {
    QListWidgetItem* newItem = new QListWidgetItem( lstScales );
    newItem->setText( *scaleIt );
    newItem->setFlags( Qt::ItemIsEditable | Qt::ItemIsEnabled | Qt::ItemIsSelectable );
    lstScales->addItem( newItem );
  }
}

void QgsProjectProperties::on_pbnExportScales_clicked()
{
  QString fileName = QFileDialog::getSaveFileName( this, tr( "Save scales" ), ".",
                     tr( "XML files (*.xml *.XML)" ) );
  if ( fileName.isEmpty() )
  {
    return;
  }

  // ensure the user never ommited the extension from the file name
  if ( !fileName.toLower().endsWith( ".xml" ) )
  {
    fileName += ".xml";
  }

  QStringList myScales;
  for ( int i = 0; i < lstScales->count(); ++i )
  {
    myScales.append( lstScales->item( i )->text() );
  }

  QString msg;
  if ( !QgsScaleUtils::saveScaleList( fileName, myScales, msg ) )
  {
    QgsDebugMsg( msg );
  }
}

void QgsProjectProperties::populateStyles()
{
  // Styles - taken from qgsstylev2managerdialog

  // use QComboBox and QString lists for shorter code
  QStringList prefList;
  QList<QComboBox*> cboList;
  cboList << cboStyleMarker;
  prefList << QgsProject::instance()->readEntry( "DefaultStyles", "/Marker", "" );
  cboList << cboStyleLine;
  prefList << QgsProject::instance()->readEntry( "DefaultStyles", "/Line", "" );
  cboList << cboStyleFill;
  prefList << QgsProject::instance()->readEntry( "DefaultStyles", "/Fill", "" );
  cboList << cboStyleColorRamp;
  prefList << QgsProject::instance()->readEntry( "DefaultStyles", "/ColorRamp", "" );
  for ( int i = 0; i < cboList.count(); i++ )
  {
    cboList[i]->clear();
    cboList[i]->addItem( "" );
  }

  // populate symbols
  QStringList symbolNames = mStyle->symbolNames();
  for ( int i = 0; i < symbolNames.count(); ++i )
  {
    QString name = symbolNames[i];
    QgsSymbolV2* symbol = mStyle->symbol( name );
    QComboBox* cbo = 0;
    switch ( symbol->type() )
    {
      case QgsSymbolV2::Marker :
        cbo = cboStyleMarker;
        break;
      case QgsSymbolV2::Line :
        cbo = cboStyleLine;
        break;
      case QgsSymbolV2::Fill :
        cbo = cboStyleFill;
        break;
    }
    if ( cbo )
    {
      QIcon icon = QgsSymbolLayerV2Utils::symbolPreviewIcon( symbol, cbo->iconSize() );
      cbo->addItem( icon, name );
    }
    delete symbol;
  }

  // populate color ramps
  QStringList colorRamps = mStyle->colorRampNames();
  for ( int i = 0; i < colorRamps.count(); ++i )
  {
    QString name = colorRamps[i];
    QgsVectorColorRampV2* ramp = mStyle->colorRamp( name );
    QIcon icon = QgsSymbolLayerV2Utils::colorRampPreviewIcon( ramp, cboStyleColorRamp->iconSize() );
    cboStyleColorRamp->addItem( icon, name );
    delete ramp;
  }

  // set current index if found
  for ( int i = 0; i < cboList.count(); i++ )
  {
    int index = cboList[i]->findText( prefList[i], Qt::MatchCaseSensitive );
    if ( index >= 0 )
      cboList[i]->setCurrentIndex( index );
  }

  // random colors
  cbxStyleRandomColors->setChecked( QgsProject::instance()->readBoolEntry( "DefaultStyles", "/RandomColors", true ) );

  // alpha transparency
  int transparencyInt = 255 - QgsProject::instance()->readNumEntry( "DefaultStyles", "/AlphaInt", 255 );
  mTransparencySlider->setValue( transparencyInt );
  on_mTransparencySlider_valueChanged( transparencyInt );
}

void QgsProjectProperties::on_pbtnStyleManager_clicked()
{
  QgsStyleV2ManagerDialog dlg( mStyle, this );
  dlg.exec();
  populateStyles();
}

void QgsProjectProperties::on_pbtnStyleMarker_clicked()
{
  editSymbol( cboStyleMarker );
}

void QgsProjectProperties::on_pbtnStyleLine_clicked()
{
  editSymbol( cboStyleLine );
}

void QgsProjectProperties::on_pbtnStyleFill_clicked()
{
  editSymbol( cboStyleFill );
}

void QgsProjectProperties::on_pbtnStyleColorRamp_clicked()
{
  // TODO for now just open style manager
  // code in QgsStyleV2ManagerDialog::editColorRamp()
  on_pbtnStyleManager_clicked();
}

void QgsProjectProperties::on_mTransparencySlider_valueChanged( int value )
{
  double alpha = 1 - ( value / 255.0 );
  double transparencyPercent = ( 1 - alpha ) * 100;
  mTransparencyLabel->setText( tr( "Transparency %1%" ).arg(( int ) transparencyPercent ) );
}

void QgsProjectProperties::editSymbol( QComboBox* cbo )
{
  QString symbolName = cbo->currentText();
  if ( symbolName == "" )
  {
    QMessageBox::information( this, "", tr( "Select a valid symbol" ) );
    return;
  }
  QgsSymbolV2* symbol = mStyle->symbol( symbolName );
  if ( ! symbol )
  {
    QMessageBox::warning( this, "", tr( "Invalid symbol : " ) + symbolName );
    return;
  }

  // let the user edit the symbol and update list when done
  QgsSymbolV2SelectorDialog dlg( symbol, mStyle, 0, this );
  if ( dlg.exec() == 0 )
  {
    delete symbol;
    return;
  }

  // by adding symbol to style with the same name the old effectively gets overwritten
  mStyle->addSymbol( symbolName, symbol );

  // update icon
  QIcon icon = QgsSymbolLayerV2Utils::symbolPreviewIcon( symbol, cbo->iconSize() );
  cbo->setItemIcon( cbo->currentIndex(), icon );
}


