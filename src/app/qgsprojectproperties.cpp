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
/* $Id$ */

#include "qgsprojectproperties.h"

//qgis includes
#include "qgsavoidintersectionsdialog.h"
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

//qt includes
#include <QColorDialog>
#include <QHeaderView>  // Qt 4.4
#include "qgslogger.h"

//stdc++ includes


QgsProjectProperties::QgsProjectProperties( QgsMapCanvas* mapCanvas, QWidget *parent, Qt::WFlags fl )
    : QDialog( parent, fl ), mMapCanvas( mapCanvas )
{
  setupUi( this );
  connect( buttonBox, SIGNAL( accepted() ), this, SLOT( accept() ) );
  connect( buttonBox, SIGNAL( rejected() ), this, SLOT( reject() ) );
  connect( buttonBox->button( QDialogButtonBox::Apply ), SIGNAL( clicked() ), this, SLOT( apply() ) );
  connect( this, SIGNAL( accepted() ), this, SLOT( apply() ) );

  ///////////////////////////////////////////////////////////
  // Properties stored in map canvas's QgsMapRenderer
  // these ones are propagated to QgsProject by a signal

  QgsMapRenderer* myRender = mMapCanvas->mapRenderer();
  QGis::UnitType myUnit = myRender->mapUnits();
  setMapUnits( myUnit );

  //see if the user wants on the fly projection enabled
  bool myProjectionEnabled = myRender->hasCrsTransformEnabled();
  cbxProjectionEnabled->setChecked( myProjectionEnabled );
  btnGrpMapUnits->setEnabled( !myProjectionEnabled );

  long myCRSID = myRender->destinationSrs().srsid();
  QgsDebugMsg( "Read project CRSID: " + QString::number( myCRSID ) );
  projectionSelector->setSelectedCrsId( myCRSID );

  ///////////////////////////////////////////////////////////
  // Properties stored in QgsProject

  title( QgsProject::instance()->title() );

  // get the manner in which the number of decimal places in the mouse
  // position display is set (manual or automatic)
  bool automaticPrecision = QgsProject::instance()->readBoolEntry( "PositionPrecision", "/Automatic" );
  if ( automaticPrecision )
  {
    radAutomatic->setChecked( true );
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
  QColor myColor = QColor( myRedInt, myGreenInt, myBlueInt );
  pbnSelectionColor->setColor( myColor );

  //get the color for map canvas background and set button color accordingly (default white)
  myRedInt = QgsProject::instance()->readNumEntry( "Gui", "/CanvasColorRedPart", 255 );
  myGreenInt = QgsProject::instance()->readNumEntry( "Gui", "/CanvasColorGreenPart", 255 );
  myBlueInt = QgsProject::instance()->readNumEntry( "Gui", "/CanvasColorBluePart", 255 );
  myColor = QColor( myRedInt, myGreenInt, myBlueInt );
  pbnCanvasColor->setColor( myColor );

  //read the digitizing settings
  int topologicalEditing = QgsProject::instance()->readNumEntry( "Digitizing", "/TopologicalEditing", 0 );
  if ( topologicalEditing != 0 )
  {
    mEnableTopologicalEditingCheckBox->setCheckState( Qt::Checked );
  }
  else
  {
    mEnableTopologicalEditingCheckBox->setCheckState( Qt::Unchecked );
  }

  bool avoidIntersectionListOk;
  mAvoidIntersectionsSettings.clear();
  QStringList avoidIntersectionsList = QgsProject::instance()->readListEntry( "Digitizing", "/AvoidIntersectionsList", &avoidIntersectionListOk );
  if ( avoidIntersectionListOk )
  {
    QStringList::const_iterator avoidIt = avoidIntersectionsList.constBegin();
    for ( ; avoidIt != avoidIntersectionsList.constEnd(); ++avoidIt )
    {
      mAvoidIntersectionsSettings.insert( *avoidIt );
    }
  }

  bool layerIdListOk, enabledListOk, toleranceListOk, toleranceUnitListOk, snapToListOk;
  QStringList layerIdList = QgsProject::instance()->readListEntry( "Digitizing", "/LayerSnappingList", &layerIdListOk );
  QStringList enabledList = QgsProject::instance()->readListEntry( "Digitizing", "/LayerSnappingEnabledList", &enabledListOk );
  QStringList toleranceList = QgsProject::instance()->readListEntry( "Digitizing", "/LayerSnappingToleranceList", & toleranceListOk );
  QStringList toleranceUnitList = QgsProject::instance()->readListEntry( "Digitizing", "/LayerSnappingToleranceUnitList", & toleranceUnitListOk );
  QStringList snapToList = QgsProject::instance()->readListEntry( "Digitizing", "/LayerSnapToList", &snapToListOk );

  QStringList::const_iterator idIter = layerIdList.constBegin();
  QStringList::const_iterator enabledIter = enabledList.constBegin();
  QStringList::const_iterator tolIter = toleranceList.constBegin();
  QStringList::const_iterator tolUnitIter = toleranceUnitList.constBegin();
  QStringList::const_iterator snapToIter = snapToList.constBegin();

  QgsMapLayer* currentLayer = 0;

  //create the new layer entries
  for ( ; idIter != layerIdList.constEnd(); ++idIter, ++enabledIter, ++tolIter, ++tolUnitIter, ++snapToIter )
  {
    if ( layerIdListOk )
    {
      currentLayer = QgsMapLayerRegistry::instance()->mapLayer( *idIter );
    }
    else
    {
      break;
    }

    if ( currentLayer )
    {
      LayerEntry newEntry;
      newEntry.layerName = currentLayer->name();

      newEntry.checked = false;
      if ( enabledListOk && enabledIter != enabledList.constEnd() )
      {
        if (( *enabledIter ) == "enabled" )
        {
          newEntry.checked = true;
        }
      }

      //snap to vertex / segment / vertex and segment
      if ( snapToListOk && snapToIter != snapToList.constEnd() )
      {
        if (( *snapToIter ) == "to_vertex" )
        {
          newEntry.snapTo = 0;
        }
        else if (( *snapToIter ) == "to_segment" )
        {
          newEntry.snapTo = 1;
        }
        else //to vertex and segment
        {
          newEntry.snapTo = 2;
        }
      }
      else
      {
        newEntry.snapTo = 0;
      }

      //snap tolerance
      if ( toleranceListOk && tolIter != toleranceList.constEnd() )
      {
        newEntry.tolerance = tolIter->toDouble();
      }
      else
      {
        newEntry.tolerance = 0;
      }

      //snap tolerance unit
      if ( toleranceUnitListOk && tolUnitIter != toleranceUnitList.constEnd() )
      {
        newEntry.toleranceUnit = tolUnitIter->toInt();
      }
      else
      {
        newEntry.toleranceUnit = 0;
      }
      mSnappingLayerSettings.insert( *idIter, newEntry );
    }
  }

  QStringList noIdentifyLayerIdList = QgsProject::instance()->readListEntry( "Identify", "/disabledLayers" );

  const QMap<QString, QgsMapLayer*> &mapLayers = QgsMapLayerRegistry::instance()->mapLayers();

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
    twi->setData( Qt::UserRole, it.key() );
    twIdentifyLayers->setVerticalHeaderItem( i, twi );

    twi = new QTableWidgetItem( currentLayer->name() );
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

      if ( rl && rl->providerKey() == "wms" )
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
    cb->setChecked( !noIdentifyLayerIdList.contains( currentLayer->getLayerID() ) );
    twIdentifyLayers->setCellWidget( i, 2, cb );
  }

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

  QgsMapRenderer* myRender = mMapCanvas->mapRenderer();

  myRender->setMapUnits( mapUnit );

  myRender->setProjectionsEnabled( cbxProjectionEnabled->isChecked() );

  // Only change the projection if there is a node in the tree
  // selected that has an srid. This prevents error if the user
  // selects a top-level node rather than an actual coordinate
  // system
  long myCRSID = projectionSelector->selectedCrsId();
  if ( myCRSID )
  {
    QgsCoordinateReferenceSystem srs( myCRSID, QgsCoordinateReferenceSystem::InternalCrsId );
    myRender->setDestinationSrs( srs );
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
        myRender->setMapUnits( srs.mapUnits() );
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
  QgsRenderer::setSelectionColor( myColor );

  //set the color for canvas
  myColor = pbnCanvasColor->color();
  QgsProject::instance()->writeEntry( "Gui", "/CanvasColorRedPart", myColor.red() );
  QgsProject::instance()->writeEntry( "Gui", "/CanvasColorGreenPart", myColor.green() );
  QgsProject::instance()->writeEntry( "Gui", "/CanvasColorBluePart", myColor.blue() );

  //write the digitizing settings
  int topologicalEditingEnabled = ( mEnableTopologicalEditingCheckBox->checkState() == Qt::Checked ) ? 1 : 0;
  QgsProject::instance()->writeEntry( "Digitizing", "/TopologicalEditing", topologicalEditingEnabled );

  //store avoid intersection layers
  QStringList avoidIntersectionList;
  QSet<QString>::const_iterator avoidIt = mAvoidIntersectionsSettings.constBegin();
  for ( ; avoidIt != mAvoidIntersectionsSettings.constEnd(); ++avoidIt )
  {
    avoidIntersectionList.append( *avoidIt );
  }
  QgsProject::instance()->writeEntry( "Digitizing", "/AvoidIntersectionsList", avoidIntersectionList );


  QMap<QString, LayerEntry>::const_iterator layerEntryIt;

  //store the layer snapping settings as string lists
  QStringList layerIdList;
  QStringList snapToList;
  QStringList toleranceList;
  QStringList enabledList;
  QStringList toleranceUnitList;

  for ( layerEntryIt = mSnappingLayerSettings.constBegin(); layerEntryIt != mSnappingLayerSettings.constEnd(); ++layerEntryIt )
  {
    layerIdList << layerEntryIt.key();
    toleranceList << QString::number( layerEntryIt->tolerance, 'f' );
    toleranceUnitList << QString::number(( int )layerEntryIt->toleranceUnit );
    if ( layerEntryIt->checked )
    {
      enabledList << "enabled";
    }
    else
    {
      enabledList << "disabled";
    }
    if ( layerEntryIt->snapTo == 0 )
    {
      snapToList << "to_vertex";
    }
    else if ( layerEntryIt->snapTo == 1 )
    {
      snapToList << "to_segment";
    }
    else //to vertex and segment
    {
      snapToList << "to_vertex_and_segment";
    }
  }

  if ( mSnappingLayerSettings.size() > 0 )
  {
    QgsProject::instance()->writeEntry( "Digitizing", "/LayerSnappingList", layerIdList );
    QgsProject::instance()->writeEntry( "Digitizing", "/LayerSnapToList", snapToList );
    QgsProject::instance()->writeEntry( "Digitizing", "/LayerSnappingToleranceList", toleranceList );
    QgsProject::instance()->writeEntry( "Digitizing", "/LayerSnappingToleranceUnitList", toleranceUnitList );
    QgsProject::instance()->writeEntry( "Digitizing", "/LayerSnappingEnabledList", enabledList );
  }

  QStringList noIdentifyLayerList;
  for ( int i = 0; i < twIdentifyLayers->rowCount(); i++ )
  {
    QCheckBox *cb = qobject_cast<QCheckBox *>( twIdentifyLayers->cellWidget( i, 2 ) );
    if ( cb && !cb->isChecked() )
    {
      QString id = twIdentifyLayers->verticalHeaderItem( i )->data( Qt::UserRole ).toString();
      noIdentifyLayerList << id;
    }
  }

  QgsProject::instance()->writeEntry( "Identify", "/disabledLayers", noIdentifyLayerList );

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
  QColor color = QColorDialog::getColor( pbnSelectionColor->color(), this );
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

void QgsProjectProperties::on_mAvoidIntersectionsPushButton_clicked()
{
  QgsAvoidIntersectionsDialog d( mMapCanvas, mAvoidIntersectionsSettings );
  if ( d.exec() == QDialog::Accepted )
  {
    d.enabledLayers( mAvoidIntersectionsSettings );
  }
}

void QgsProjectProperties::on_mSnappingOptionsPushButton_clicked()
{
  QgsSnappingDialog d( mMapCanvas, mSnappingLayerSettings );
  if ( d.exec() == QDialog::Accepted )
  {
    //retrieve the new layer snapping settings from the dialog
    d.layerSettings( mSnappingLayerSettings );
  }
}

void QgsProjectProperties::on_cbxProjectionEnabled_stateChanged( int state )
{
  btnGrpMapUnits->setEnabled( state == Qt::Unchecked );
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
