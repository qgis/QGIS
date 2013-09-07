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
#include "qgsapplication.h"
#include "qgsdistancearea.h"
#include "qgisapp.h"
#include "qgscomposer.h"
#include "qgscontexthelp.h"
#include "qgscoordinatetransform.h"
#include "qgslogger.h"
#include "qgsmapcanvas.h"
#include "qgsmaplayer.h"
#include "qgsmaplayerregistry.h"
#include "qgsmaprenderer.h"
#include "qgsproject.h"
#include "qgsprojectlayergroupdialog.h"
#include "qgssnappingdialog.h"
#include "qgsrasterlayer.h"
#include "qgsvectorlayer.h"
#include "qgsvectordataprovider.h"
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

const char * QgsProjectProperties::GEO_NONE_DESC = QT_TRANSLATE_NOOP( "QgsOptions", "None / Planimetric" );

//stdc++ includes

QgsProjectProperties::QgsProjectProperties( QgsMapCanvas* mapCanvas, QWidget *parent, Qt::WFlags fl )
    : QgsOptionsDialogBase( "ProjectProperties", parent, fl )
    , mMapCanvas( mapCanvas )
    , mEllipsoidList()
    , mEllipsoidIndex( 0 )

{
  setupUi( this );
  // QgsOptionsDialogBase handles saving/restoring of geometry, splitter and current tab states,
  // switching vertical tabs between icon/text to icon-only modes (splitter collapsed to left),
  // and connecting QDialogButtonBox's accepted/rejected signals to dialog's accept/reject slots
  initOptionsBase( false );

  connect( buttonBox->button( QDialogButtonBox::Apply ), SIGNAL( clicked() ), this, SLOT( apply() ) );
  connect( this, SIGNAL( accepted() ), this, SLOT( apply() ) );
  connect( projectionSelector, SIGNAL( sridSelected( QString ) ), this, SLOT( setMapUnitsToCurrentProjection() ) );

  connect( cmbEllipsoid, SIGNAL( currentIndexChanged( int ) ), this, SLOT( updateEllipsoidUI( int ) ) );

  connect( radMeters, SIGNAL( toggled( bool ) ), btnGrpDegreeDisplay, SLOT( setDisabled( bool ) ) );
  connect( radFeet, SIGNAL( toggled( bool ) ), btnGrpDegreeDisplay, SLOT( setDisabled( bool ) ) );
  connect( radDegrees, SIGNAL( toggled( bool ) ), btnGrpDegreeDisplay, SLOT( setEnabled( bool ) ) );

  connect( radAutomatic, SIGNAL( toggled( bool ) ), mPrecisionFrame, SLOT( setDisabled( bool ) ) );
  connect( radManual, SIGNAL( toggled( bool ) ), mPrecisionFrame, SLOT( setEnabled( bool ) ) );

  ///////////////////////////////////////////////////////////
  // Properties stored in map canvas's QgsMapRenderer
  // these ones are propagated to QgsProject by a signal

  QgsMapRenderer* myRenderer = mMapCanvas->mapRenderer();
  QGis::UnitType myUnit = myRenderer->mapUnits();
  setMapUnits( myUnit );

  // we need to initialize it, since the on_cbxProjectionEnabled_toggled()
  // slot triggered by setChecked() might use it.
  mProjectSrsId = myRenderer->destinationCrs().srsid();

  QgsDebugMsg( "Read project CRSID: " + QString::number( mProjectSrsId ) );
  projectionSelector->setSelectedCrsId( mProjectSrsId );

  // see end of constructor for updating of projection selector

  ///////////////////////////////////////////////////////////
  // Properties stored in QgsProject

  title( QgsProject::instance()->title() );

  // get the manner in which the number of decimal places in the mouse
  // position display is set (manual or automatic)
  bool automaticPrecision = QgsProject::instance()->readBoolEntry( "PositionPrecision", "/Automatic", true );
  if ( automaticPrecision )
  {
    radAutomatic->setChecked( true );
    mPrecisionFrame->setEnabled( false );
  }
  else
  {
    radManual->setChecked( true );
    mPrecisionFrame->setEnabled( true );
  }

  cbxAbsolutePath->setCurrentIndex( QgsProject::instance()->readBoolEntry( "Paths", "/Absolute", true ) ? 0 : 1 );

  // populate combo box with ellipsoids

  QgsDebugMsg( "Setting upp ellipsoid" );

  populateEllipsoidList();

  // Reading ellipsoid from setttings
  QStringList mySplitEllipsoid = QgsProject::instance()->readEntry( "Measure", "/Ellipsoid", GEO_NONE ).split( ':' );

  int myIndex = 0;
  for ( int i = 0; i < mEllipsoidList.length(); i++ )
  {
    if ( mEllipsoidList[ i ].acronym.startsWith( mySplitEllipsoid[ 0 ] ) )
    {
      myIndex = i;
      break;
    }
  }

  // Update paramaters if present.
  if ( mySplitEllipsoid.length() >= 3 )
  {
    mEllipsoidList[ myIndex ].semiMajor =  mySplitEllipsoid[ 1 ].toDouble();
    mEllipsoidList[ myIndex ].semiMinor =  mySplitEllipsoid[ 2 ].toDouble();
  }

  updateEllipsoidUI( myIndex );


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
  pbnSelectionColor->setColorDialogTitle( tr( "Selection color" ) );
  pbnSelectionColor->setColorDialogOptions( QColorDialog::ShowAlphaChannel );

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
  mWMSFees->setText( QgsProject::instance()->readEntry( "WMSFees", "/", "" ) );
  mWMSAccessConstraints->setText( QgsProject::instance()->readEntry( "WMSAccessConstraints", "/", "" ) );
  mWMSKeywordList->setText( QgsProject::instance()->readListEntry( "WMSKeywordList", "/" ).join( "," ) );

  bool ok;
  QStringList values;

  mWMSExtMinX->setValidator( new QDoubleValidator( mWMSExtMinX ) );
  mWMSExtMinY->setValidator( new QDoubleValidator( mWMSExtMinY ) );
  mWMSExtMaxX->setValidator( new QDoubleValidator( mWMSExtMaxX ) );
  mWMSExtMaxY->setValidator( new QDoubleValidator( mWMSExtMaxY ) );

  values = QgsProject::instance()->readListEntry( "WMSExtent", "/", QStringList(), &ok );
  grpWMSExt->setChecked( ok && values.size() == 4 );
  if ( grpWMSExt->isChecked() )
  {
    mWMSExtMinX->setText( values[0] );
    mWMSExtMinY->setText( values[1] );
    mWMSExtMaxX->setText( values[2] );
    mWMSExtMaxY->setText( values[3] );
  }

  values = QgsProject::instance()->readListEntry( "WMSCrsList", "/", QStringList(), &ok );
  grpWMSList->setChecked( ok && values.size() > 0 );
  if ( grpWMSList->isChecked() )
  {
    mWMSList->addItems( values );
  }
  else
  {
    values = QgsProject::instance()->readListEntry( "WMSEpsgList", "/", QStringList(), &ok );
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

  //composer restriction for WMS
  values = QgsProject::instance()->readListEntry( "WMSRestrictedComposers", "/", QStringList(), &ok );
  mWMSComposerGroupBox->setChecked( ok );
  if ( ok )
  {
    mComposerListWidget->addItems( values );
  }

  //layer restriction for WMS
  values = QgsProject::instance()->readListEntry( "WMSRestrictedLayers", "/", QStringList(), &ok );
  mLayerRestrictionsGroupBox->setChecked( ok );
  if ( ok )
  {
    mLayerRestrictionsListWidget->addItems( values );
  }

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

  mWFSUrlLineEdit->setText( QgsProject::instance()->readEntry( "WFSUrl", "/", "" ) );
  QStringList wfsLayerIdList = QgsProject::instance()->readListEntry( "WFSLayers", "/" );
  QStringList wfstUpdateLayerIdList = QgsProject::instance()->readListEntry( "WFSTLayers", "Update" );
  QStringList wfstInsertLayerIdList = QgsProject::instance()->readListEntry( "WFSTLayers", "Insert" );
  QStringList wfstDeleteLayerIdList = QgsProject::instance()->readListEntry( "WFSTLayers", "Delete" );

  QSignalMapper *smPublied = new QSignalMapper( this );
  connect( smPublied, SIGNAL( mapped( int ) ), this, SLOT( cbxWFSPubliedStateChanged( int ) ) );
  QSignalMapper *smUpdate = new QSignalMapper( this );
  connect( smUpdate, SIGNAL( mapped( int ) ), this, SLOT( cbxWFSUpdateStateChanged( int ) ) );
  QSignalMapper *smInsert = new QSignalMapper( this );
  connect( smInsert, SIGNAL( mapped( int ) ), this, SLOT( cbxWFSInsertStateChanged( int ) ) );
  QSignalMapper *smDelete = new QSignalMapper( this );
  connect( smDelete, SIGNAL( mapped( int ) ), this, SLOT( cbxWFSDeleteStateChanged( int ) ) );

  twWFSLayers->setColumnCount( 5 );
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

      QCheckBox* cbp = new QCheckBox();
      cbp->setChecked( wfsLayerIdList.contains( currentLayer->id() ) );
      twWFSLayers->setCellWidget( j, 1, cbp );

      smPublied->setMapping( cbp, j );
      connect( cbp, SIGNAL( stateChanged( int ) ), smPublied, SLOT( map() ) );

      QgsVectorLayer* vlayer = qobject_cast<QgsVectorLayer*>( currentLayer );
      QgsVectorDataProvider* provider = vlayer->dataProvider();
      if (( provider->capabilities() & QgsVectorDataProvider::ChangeAttributeValues ) && ( provider->capabilities() & QgsVectorDataProvider::ChangeGeometries ) )
      {
        QCheckBox* cbu = new QCheckBox();
        cbu->setChecked( wfstUpdateLayerIdList.contains( currentLayer->id() ) );
        twWFSLayers->setCellWidget( j, 2, cbu );

        smUpdate->setMapping( cbu, j );
        connect( cbu, SIGNAL( stateChanged( int ) ), smUpdate, SLOT( map() ) );
      }
      if (( provider->capabilities() & QgsVectorDataProvider::AddFeatures ) )
      {
        QCheckBox* cbi = new QCheckBox();
        cbi->setChecked( wfstInsertLayerIdList.contains( currentLayer->id() ) );
        twWFSLayers->setCellWidget( j, 3, cbi );

        smInsert->setMapping( cbi, j );
        connect( cbi, SIGNAL( stateChanged( int ) ), smInsert, SLOT( map() ) );
      }
      if (( provider->capabilities() & QgsVectorDataProvider::DeleteFeatures ) )
      {
        QCheckBox* cbd = new QCheckBox();
        cbd->setChecked( wfstDeleteLayerIdList.contains( currentLayer->id() ) );
        twWFSLayers->setCellWidget( j, 4, cbd );

        smDelete->setMapping( cbd, j );
        connect( cbd, SIGNAL( stateChanged( int ) ), smDelete, SLOT( map() ) );
      }

      j++;
    }
  }
  twWFSLayers->setRowCount( j );
  twWFSLayers->verticalHeader()->setResizeMode( QHeaderView::ResizeToContents );

  // Default Styles
  mStyle = QgsStyleV2::defaultStyle();
  populateStyles();

  // Project macros
  QString pythonMacros = QgsProject::instance()->readEntry( "Macros", "/pythonCode", QString::null );
  grpPythonMacros->setChecked( !pythonMacros.isEmpty() );
  if ( !pythonMacros.isEmpty() )
  {
    ptePythonMacros->setPlainText( pythonMacros );
  }
  else
  {
    resetPythonMacros();
  }

  // Update projection selector (after mLayerSrsId is set)
  bool myProjectionEnabled = myRenderer->hasCrsTransformEnabled();
  bool onFlyChecked = cbxProjectionEnabled->isChecked();
  cbxProjectionEnabled->setChecked( myProjectionEnabled );

  if ( onFlyChecked == myProjectionEnabled )
  {
    // ensure selector is updated if cbxProjectionEnabled->toggled signal not sent
    on_cbxProjectionEnabled_toggled( myProjectionEnabled );
  }

  restoreOptionsBaseUi();
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

  if ( mEllipsoidList[ mEllipsoidIndex ].acronym.startsWith( "PARAMETER" ) )
  {
    double major = mEllipsoidList[ mEllipsoidIndex ].semiMajor;
    double minor = mEllipsoidList[ mEllipsoidIndex ].semiMinor;
    // If the user fields have changed, use them instead.
    if ( leSemiMajor->isModified() || leSemiMinor->isModified() )
    {
      QgsDebugMsg( "Using paramteric major/minor" );
      major = QLocale::system().toDouble( leSemiMajor->text() );
      minor = QLocale::system().toDouble( leSemiMinor->text() );
    }
    QgsProject::instance()->writeEntry( "Measure", "/Ellipsoid", QString( "PARAMETER:%1:%2" )
                                        .arg( major, 0, 'g', 17 )
                                        .arg( minor, 0, 'g', 17 ) );
  }
  else
  {
    QgsProject::instance()->writeEntry( "Measure", "/Ellipsoid", mEllipsoidList[ mEllipsoidIndex ].acronym );
  }

  //set the color for selections
  QColor myColor = pbnSelectionColor->color();
  QgsProject::instance()->writeEntry( "Gui", "/SelectionColorRedPart", myColor.red() );
  QgsProject::instance()->writeEntry( "Gui", "/SelectionColorGreenPart", myColor.green() );
  QgsProject::instance()->writeEntry( "Gui", "/SelectionColorBluePart", myColor.blue() );
  QgsProject::instance()->writeEntry( "Gui", "/SelectionColorAlphaPart", myColor.alpha() );

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
  QgsProject::instance()->writeEntry( "WMSFees", "/", mWMSFees->text() );
  QgsProject::instance()->writeEntry( "WMSAccessConstraints", "/", mWMSAccessConstraints->text() );
  //WMS keyword list
  QStringList keywordStringList = mWMSKeywordList->text().split( "," );
  if ( keywordStringList.size() > 0 )
  {
    QgsProject::instance()->writeEntry( "WMSKeywordList", "/", mWMSKeywordList->text().split( "," ) );
  }
  else
  {
    QgsProject::instance()->removeEntry( "WMSKeywordList", "/" );
  }

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

  //WMS composer restrictions
  if ( mWMSComposerGroupBox->isChecked() )
  {
    QStringList composerTitles;
    for ( int i = 0; i < mComposerListWidget->count(); ++i )
    {
      composerTitles << mComposerListWidget->item( i )->text();
    }
    QgsProject::instance()->writeEntry( "WMSRestrictedComposers", "/", composerTitles );
  }
  else
  {
    QgsProject::instance()->removeEntry( "WMSRestrictedComposers", "/" );
  }

  //WMS layer restrictions
  if ( mLayerRestrictionsGroupBox->isChecked() )
  {
    QStringList layerNames;
    for ( int i = 0; i < mLayerRestrictionsListWidget->count(); ++i )
    {
      layerNames << mLayerRestrictionsListWidget->item( i )->text();
    }
    QgsProject::instance()->writeEntry( "WMSRestrictedLayers", "/", layerNames );
  }
  else
  {
    QgsProject::instance()->removeEntry( "WMSRestrictedLayers", "/" );
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

  QgsProject::instance()->writeEntry( "WFSUrl", "/", mWFSUrlLineEdit->text() );
  QStringList wfsLayerList;
  QStringList wfstUpdateLayerList;
  QStringList wfstInsertLayerList;
  QStringList wfstDeleteLayerList;
  for ( int i = 0; i < twWFSLayers->rowCount(); i++ )
  {
    QString id = twWFSLayers->item( i, 0 )->data( Qt::UserRole ).toString();
    QCheckBox* cb;
    cb = qobject_cast<QCheckBox *>( twWFSLayers->cellWidget( i, 1 ) );
    if ( cb && cb->isChecked() )
    {
      wfsLayerList << id;
    }
    cb = qobject_cast<QCheckBox *>( twWFSLayers->cellWidget( i, 2 ) );
    if ( cb && cb->isChecked() )
    {
      wfstUpdateLayerList << id;
    }
    cb = qobject_cast<QCheckBox *>( twWFSLayers->cellWidget( i, 3 ) );
    if ( cb && cb->isChecked() )
    {
      wfstInsertLayerList << id;
    }
    cb = qobject_cast<QCheckBox *>( twWFSLayers->cellWidget( i, 4 ) );
    if ( cb && cb->isChecked() )
    {
      wfstDeleteLayerList << id;
    }
  }
  QgsProject::instance()->writeEntry( "WFSLayers", "/", wfsLayerList );
  QgsProject::instance()->writeEntry( "WFSTLayers", "Update", wfstUpdateLayerList );
  QgsProject::instance()->writeEntry( "WFSTLayers", "Insert", wfstInsertLayerList );
  QgsProject::instance()->writeEntry( "WFSTLayers", "Delete", wfstDeleteLayerList );

  // Default Styles
  QgsProject::instance()->writeEntry( "DefaultStyles", "/Marker", cboStyleMarker->currentText() );
  QgsProject::instance()->writeEntry( "DefaultStyles", "/Line", cboStyleLine->currentText() );
  QgsProject::instance()->writeEntry( "DefaultStyles", "/Fill", cboStyleFill->currentText() );
  QgsProject::instance()->writeEntry( "DefaultStyles", "/ColorRamp", cboStyleColorRamp->currentText() );
  QgsProject::instance()->writeEntry( "DefaultStyles", "/AlphaInt", ( int )( 255 - ( mTransparencySlider->value() * 2.55 ) ) );
  QgsProject::instance()->writeEntry( "DefaultStyles", "/RandomColors", cbxStyleRandomColors->isChecked() );

  // store project macros
  QString pythonMacros = ptePythonMacros->toPlainText();
  if ( !grpPythonMacros->isChecked() || pythonMacros.isEmpty() )
  {
    pythonMacros = QString::null;
    resetPythonMacros();
  }
  QgsProject::instance()->writeEntry( "Macros", "/pythonCode", pythonMacros );

  //todo XXX set canvas color
  emit refresh();
}

bool QgsProjectProperties::isProjected()
{
  return cbxProjectionEnabled->isChecked();
}

void QgsProjectProperties::showProjectionsTab()
{
  mOptionsListWidget->setCurrentRow( 1 );
}

void QgsProjectProperties::on_cbxProjectionEnabled_toggled( bool onFlyEnabled )
{
  QString measureOnFlyState = tr( "Measure tool (CRS transformation: %1)" );
  QString unitsOnFlyState = tr( "Canvas units (CRS transformation: %1)" );
  if ( !onFlyEnabled )
  {
    // reset projection to default
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
    mProjectSrsId = mLayerSrsId;
    projectionSelector->setSelectedCrsId( mLayerSrsId );

    QgsCoordinateReferenceSystem srs( mLayerSrsId, QgsCoordinateReferenceSystem::InternalCrsId );
    //set radio button to crs map unit type
    QGis::UnitType units = srs.mapUnits();

    radMeters->setChecked( units == QGis::Meters );
    radFeet->setChecked( units == QGis::Feet );
    radDegrees->setChecked( units == QGis::Degrees );

    // unset ellipsoid
    mEllipsoidIndex = 0;

    btnGrpMeasureEllipsoid->setTitle( measureOnFlyState.arg( tr( "OFF" ) ) );
    btnGrpMapUnits->setTitle( unitsOnFlyState.arg( tr( "OFF" ) ) );
  }
  else
  {
    if ( !mLayerSrsId )
    {
      mLayerSrsId = projectionSelector->selectedCrsId();
    }
    projectionSelector->setSelectedCrsId( mProjectSrsId );

    btnGrpMeasureEllipsoid->setTitle( measureOnFlyState.arg( tr( "ON" ) ) );
    btnGrpMapUnits->setTitle( unitsOnFlyState.arg( tr( "ON" ) ) );
  }

  setMapUnitsToCurrentProjection();

  // Enable/Disable selector and update tool-tip
  updateEllipsoidUI( mEllipsoidIndex ); // maybe already done by setMapUnitsToCurrentProjection

}

void QgsProjectProperties::cbxWFSPubliedStateChanged( int aIdx )
{
  QCheckBox* cb = qobject_cast<QCheckBox *>( twWFSLayers->cellWidget( aIdx, 1 ) );
  if ( cb && !cb->isChecked() )
  {
    QCheckBox* cbn = qobject_cast<QCheckBox *>( twWFSLayers->cellWidget( aIdx, 2 ) );
    if ( cbn )
      cbn->setChecked( false );
  }
}

void QgsProjectProperties::cbxWFSUpdateStateChanged( int aIdx )
{
  QCheckBox* cb = qobject_cast<QCheckBox *>( twWFSLayers->cellWidget( aIdx, 2 ) );
  if ( cb && cb->isChecked() )
  {
    QCheckBox* cbn = qobject_cast<QCheckBox *>( twWFSLayers->cellWidget( aIdx, 1 ) );
    if ( cbn )
      cbn->setChecked( true );
  }
  else if ( cb && !cb->isChecked() )
  {
    QCheckBox* cbn = qobject_cast<QCheckBox *>( twWFSLayers->cellWidget( aIdx, 3 ) );
    if ( cbn )
      cbn->setChecked( false );
  }
}

void QgsProjectProperties::cbxWFSInsertStateChanged( int aIdx )
{
  QCheckBox* cb = qobject_cast<QCheckBox *>( twWFSLayers->cellWidget( aIdx, 3 ) );
  if ( cb && cb->isChecked() )
  {
    QCheckBox* cbn = qobject_cast<QCheckBox *>( twWFSLayers->cellWidget( aIdx, 2 ) );
    if ( cbn )
      cbn->setChecked( true );
  }
  else if ( cb && !cb->isChecked() )
  {
    QCheckBox* cbn = qobject_cast<QCheckBox *>( twWFSLayers->cellWidget( aIdx, 4 ) );
    if ( cbn )
      cbn->setChecked( false );
  }
}

void QgsProjectProperties::cbxWFSDeleteStateChanged( int aIdx )
{
  QCheckBox* cb = qobject_cast<QCheckBox *>( twWFSLayers->cellWidget( aIdx, 4 ) );
  if ( cb && cb->isChecked() )
  {
    QCheckBox* cbn = qobject_cast<QCheckBox *>( twWFSLayers->cellWidget( aIdx, 3 ) );
    if ( cbn )
      cbn->setChecked( true );
  }
}

void QgsProjectProperties::setMapUnitsToCurrentProjection()
{
  long myCRSID = projectionSelector->selectedCrsId();
  if ( !isProjected() || !myCRSID )
    return;

  QgsCoordinateReferenceSystem srs( myCRSID, QgsCoordinateReferenceSystem::InternalCrsId );
  //set radio button to crs map unit type
  QGis::UnitType units = srs.mapUnits();

  radMeters->setChecked( units == QGis::Meters );
  radFeet->setChecked( units == QGis::Feet );
  radDegrees->setChecked( units == QGis::Degrees );

  // attempt to reset the projection ellipsoid according to the srs
  int i;
  for ( i = 0; i < mEllipsoidList.length() && mEllipsoidList[ i ].description != srs.description(); i++ )
    ;
  if ( i < mEllipsoidList.length() )
    updateEllipsoidUI( i );
}

/*!
 * Function to save non-base dialog states
 */
void QgsProjectProperties::saveState()
{
}

/*!
 * Function to restore non-base dialog states
 */
void QgsProjectProperties::restoreState()
{
}

/*!
 * Set WMS default extent to current canvas extent
 */
void QgsProjectProperties::on_pbnWMSExtCanvas_clicked()
{
  QgsRectangle ext = mMapCanvas->extent();
  mWMSExtMinX->setText( qgsDoubleToString( ext.xMinimum() ) );
  mWMSExtMinY->setText( qgsDoubleToString( ext.yMinimum() ) );
  mWMSExtMaxX->setText( qgsDoubleToString( ext.xMaximum() ) );
  mWMSExtMaxY->setText( qgsDoubleToString( ext.yMaximum() ) );
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

void QgsProjectProperties::on_mAddWMSComposerButton_clicked()
{
  QSet<QgsComposer*> projectComposers = QgisApp::instance()->printComposers();
  QStringList composerTitles;
  QSet<QgsComposer*>::const_iterator cIt = projectComposers.constBegin();
  for ( ; cIt != projectComposers.constEnd(); ++cIt )
  {
    composerTitles << ( *cIt )->title();
  }

  bool ok;
  QString name = QInputDialog::getItem( this, tr( "Select print composer" ), tr( "Composer Title" ), composerTitles, 0, false, &ok );
  if ( ok )
  {
    if ( mComposerListWidget->findItems( name, Qt::MatchExactly ).size() < 1 )
    {
      mComposerListWidget->addItem( name );
    }
  }
}

void QgsProjectProperties::on_mRemoveWMSComposerButton_clicked()
{
  QListWidgetItem* currentItem = mComposerListWidget->currentItem();
  if ( currentItem )
  {
    delete mComposerListWidget->takeItem( mComposerListWidget->row( currentItem ) );
  }
}

void QgsProjectProperties::on_mAddLayerRestrictionButton_clicked()
{
  QgsProjectLayerGroupDialog d( this, QgsProject::instance()->fileName() );
  d.setWindowTitle( tr( "Select restricted layers and groups" ) );
  if ( d.exec() == QDialog::Accepted )
  {
    QStringList layerNames = d.selectedLayerNames();
    QStringList::const_iterator layerIt = layerNames.constBegin();
    for ( ; layerIt != layerNames.constEnd(); ++layerIt )
    {
      if ( mLayerRestrictionsListWidget->findItems( *layerIt, Qt::MatchExactly ).size() < 1 )
      {
        mLayerRestrictionsListWidget->addItem( *layerIt );
      }
    }

    QStringList groups = d.selectedGroups();
    QStringList::const_iterator groupIt = groups.constBegin();
    for ( ; groupIt != groups.constEnd(); ++groupIt )
    {
      if ( mLayerRestrictionsListWidget->findItems( *groupIt, Qt::MatchExactly ).size() < 1 )
      {
        mLayerRestrictionsListWidget->addItem( *groupIt );
      }
    }
  }
}

void QgsProjectProperties::on_mRemoveLayerRestrictionButton_clicked()
{
  QListWidgetItem* currentItem = mLayerRestrictionsListWidget->currentItem();
  if ( currentItem )
  {
    delete mLayerRestrictionsListWidget->takeItem( mLayerRestrictionsListWidget->row( currentItem ) );
  }
}

void QgsProjectProperties::on_pbnWFSLayersSelectAll_clicked()
{
  for ( int i = 0; i < twWFSLayers->rowCount(); i++ )
  {
    QCheckBox *cb = qobject_cast<QCheckBox *>( twWFSLayers->cellWidget( i, 1 ) );
    cb->setChecked( true );
  }
}

void QgsProjectProperties::on_pbnWFSLayersUnselectAll_clicked()
{
  for ( int i = 0; i < twWFSLayers->rowCount(); i++ )
  {
    QCheckBox *cb = qobject_cast<QCheckBox *>( twWFSLayers->cellWidget( i, 1 ) );
    cb->setChecked( false );
  }
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
  int transparencyInt = ( 255 - QgsProject::instance()->readNumEntry( "DefaultStyles", "/AlphaInt", 255 ) ) / 2.55;
  mTransparencySlider->setValue( transparencyInt );
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
  mTransparencySpinBox->blockSignals( true );
  mTransparencySpinBox->setValue( value );
  mTransparencySpinBox->blockSignals( false );
}

void QgsProjectProperties::on_mTransparencySpinBox_valueChanged( int value )
{
  mTransparencySlider->blockSignals( true );
  mTransparencySlider->setValue( value );
  mTransparencySlider->blockSignals( false );
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

void QgsProjectProperties::resetPythonMacros()
{
  grpPythonMacros->setChecked( false );
  ptePythonMacros->setPlainText( "def openProject():\n    pass\n\n" \
                                 "def saveProject():\n    pass\n\n" \
                                 "def closeProject():\n    pass\n" );
}

void QgsProjectProperties::populateEllipsoidList()
{
  //
  // Populate the ellipsoid list
  //
  sqlite3      *myDatabase;
  const char   *myTail;
  sqlite3_stmt *myPreparedStatement;
  int           myResult;
  EllipsoidDefs myItem, i;

  myItem.acronym = GEO_NONE;
  myItem.description =  tr( GEO_NONE_DESC );
  myItem.semiMajor = 0.0;
  myItem.semiMinor = 0.0;
  mEllipsoidList.append( myItem );

  myItem.acronym = QString( "PARAMETER:6370997:6370997" );
  myItem.description = tr( "Parameters:" );
  myItem.semiMajor = 6370997.0;
  myItem.semiMinor = 6370997.0;
  mEllipsoidList.append( myItem );

  //check the db is available
  myResult = sqlite3_open_v2( QgsApplication::srsDbFilePath().toUtf8().data(), &myDatabase, SQLITE_OPEN_READONLY, NULL );
  if ( myResult )
  {
    QgsDebugMsg( QString( "Can't open database: %1" ).arg( sqlite3_errmsg( myDatabase ) ) );
    // XXX This will likely never happen since on open, sqlite creates the
    //     database if it does not exist.
    Q_ASSERT( myResult == 0 );
  }

  // Set up the query to retrieve the projection information needed to populate the ELLIPSOID list
  QString mySql = "select acronym, name, radius, parameter2 from tbl_ellipsoid order by name";
  myResult = sqlite3_prepare( myDatabase, mySql.toUtf8(), mySql.toUtf8().length(), &myPreparedStatement, &myTail );
  // XXX Need to free memory from the error msg if one is set
  if ( myResult == SQLITE_OK )
  {
    while ( sqlite3_step( myPreparedStatement ) == SQLITE_ROW )
    {
      QString para1, para2;
      myItem.acronym = ( const char * )sqlite3_column_text( myPreparedStatement, 0 );
      myItem.description = ( const char * )sqlite3_column_text( myPreparedStatement, 1 );

      // Copied from QgsDistanecArea. Should perhaps be moved there somehow?
      // No error checking, this values are for show only, never used in calculations.

      // Fall-back values
      myItem.semiMajor = 0.0;
      myItem.semiMinor = 0.0;
      // Crash if no column?
      para1 = ( const char * )sqlite3_column_text( myPreparedStatement, 2 );
      para2 = ( const char * )sqlite3_column_text( myPreparedStatement, 3 );
      myItem.semiMajor = para1.mid( 2 ).toDouble();
      if ( para2.left( 2 ) == "b=" )
      {
        myItem.semiMinor = para2.mid( 2 ).toDouble();
      }
      else if ( para2.left( 3 ) == "rf=" )
      {
        double invFlattening = para2.mid( 3 ).toDouble();
        if ( invFlattening != 0.0 )
        {
          myItem.semiMinor = myItem.semiMajor - ( myItem.semiMajor / invFlattening );
        }
      }
      mEllipsoidList.append( myItem );
    }
  }

  // close the sqlite3 statement
  sqlite3_finalize( myPreparedStatement );
  sqlite3_close( myDatabase );

  // Add all items to selector

  foreach ( i, mEllipsoidList )
  {
    cmbEllipsoid->addItem( i.description );
  }
}

void QgsProjectProperties::updateEllipsoidUI( int newIndex )
{
  // Just return if the list isn't populated yet
  if ( mEllipsoidList.isEmpty() )
  {
    return;
  }
  // Called whenever settings change, adjusts the UI accordingly
  // Pre-select current ellipsoid

  // Check if CRS transformation is on, or else turn everything off
  double myMajor =  mEllipsoidList[ newIndex ].semiMajor;
  double myMinor =  mEllipsoidList[ newIndex ].semiMinor;

  // If user has modified the radii (only possible if parametric!), before
  // changing ellipsoid, save the modified coordinates
  if ( leSemiMajor->isModified() || leSemiMinor->isModified() )
  {
    QgsDebugMsg( "Saving major/minor" );
    mEllipsoidList[ mEllipsoidIndex ].semiMajor = QLocale::system().toDouble( leSemiMajor->text() );
    mEllipsoidList[ mEllipsoidIndex ].semiMinor = QLocale::system().toDouble( leSemiMinor->text() );
  }

  mEllipsoidIndex = newIndex;
  leSemiMajor->setEnabled( false );
  leSemiMinor->setEnabled( false );
  leSemiMajor->setText( "" );
  leSemiMinor->setText( "" );
  if ( cbxProjectionEnabled->isChecked() )
  {
    cmbEllipsoid->setEnabled( true );
    cmbEllipsoid->setToolTip( "" );
    if ( mEllipsoidList[ mEllipsoidIndex ].acronym.startsWith( "PARAMETER:" ) )
    {
      leSemiMajor->setEnabled( true );
      leSemiMinor->setEnabled( true );
    }
    else
    {
      leSemiMajor->setToolTip( tr( "Select %1 from pull-down menu to adjust radii" ).arg( tr( "Parameters:" ) ) );
      leSemiMinor->setToolTip( tr( "Select %1 from pull-down menu to adjust radii" ).arg( tr( "Parameters:" ) ) );
    }
    if ( mEllipsoidList[ mEllipsoidIndex ].acronym != GEO_NONE )
    {
      leSemiMajor->setText( QLocale::system().toString( myMajor, 'f', 3 ) );
      leSemiMinor->setText( QLocale::system().toString( myMinor, 'f', 3 ) );
    }
  }
  else
  {
    cmbEllipsoid->setEnabled( false );
    cmbEllipsoid->setToolTip( tr( "Can only use ellipsoidal calculations when CRS transformation is enabled" ) );
  }
  cmbEllipsoid->setCurrentIndex( mEllipsoidIndex ); // Not always necessary
}
