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
#include "qgscontexthelp.h"
#include "qgscoordinatetransform.h"
#include "qgslogger.h"
#include "qgsmapcanvas.h"
#include "qgsmaplayer.h"
#include "qgsmaplayerregistry.h"
#include "qgsmaprender.h"
#include "qgsproject.h"
#include "qgsrenderer.h"
#include "qgssnappingdialog.h"

//qt includes
#include <QColorDialog>

//stdc++ includes
#include <iostream>


QgsProjectProperties::QgsProjectProperties(QgsMapCanvas* mapCanvas, QWidget *parent, Qt::WFlags fl)
  : QDialog(parent, fl), mMapCanvas(mapCanvas)
{
  setupUi(this);
  connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
  connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
  connect(buttonBox->button(QDialogButtonBox::Apply), SIGNAL(clicked()), this, SLOT(apply()));
  connect(this, SIGNAL(accepted()), this, SLOT(apply()));

  ///////////////////////////////////////////////////////////
  // Properties stored in map canvas's QgsMapRender
  // these ones are propagated to QgsProject by a signal
  
  QgsMapRender* myRender = mMapCanvas->mapRender();
  QGis::units myUnit = myRender->mapUnits();
  setMapUnits(myUnit);

  //see if the user wants on the fly projection enabled
  bool myProjectionEnabled = myRender->projectionsEnabled();
  cbxProjectionEnabled->setChecked(myProjectionEnabled);
  
  // set the default wkt to WGS 84
  long mySRSID = myRender->destinationSrs().srsid();
  QgsDebugMsg("Read project SRSID: " + QString::number(mySRSID));
  projectionSelector->setSelectedSRSID(mySRSID);

  ///////////////////////////////////////////////////////////
  // Properties stored in QgsProject
  
  title(QgsProject::instance()->title());

  // get the manner in which the number of decimal places in the mouse
  // position display is set (manual or automatic)
  bool automaticPrecision = QgsProject::instance()->readBoolEntry("PositionPrecision","/Automatic");
  if (automaticPrecision)
    radAutomatic->setChecked(true);
  else
    radManual->setChecked(true);

  int dp = QgsProject::instance()->readNumEntry("PositionPrecision", "/DecimalPlaces");
  spinBoxDP->setValue(dp);

  //get the colour selections and set the button colour accordingly
  int myRedInt = QgsProject::instance()->readNumEntry("Gui","/SelectionColorRedPart",255);
  int myGreenInt = QgsProject::instance()->readNumEntry("Gui","/SelectionColorGreenPart",255);
  int myBlueInt = QgsProject::instance()->readNumEntry("Gui","/SelectionColorBluePart",0);
  QColor myColour = QColor(myRedInt,myGreenInt,myBlueInt);
  pbnSelectionColour->setColor(myColour);

  //get the colour for map canvas background and set button colour accordingly (default white)
  myRedInt = QgsProject::instance()->readNumEntry("Gui","/CanvasColorRedPart",255);
  myGreenInt = QgsProject::instance()->readNumEntry("Gui","/CanvasColorGreenPart",255);
  myBlueInt = QgsProject::instance()->readNumEntry("Gui","/CanvasColorBluePart",255);
  myColour = QColor(myRedInt,myGreenInt,myBlueInt);
  pbnCanvasColor->setColor(myColour);

  //read the digitizing settings
  int topologicalEditing = QgsProject::instance()->readNumEntry("Digitizing", "/TopologicalEditing", 0);
  if(topologicalEditing != 0)
    {
      mEnableTopologicalEditingCheckBox->setCheckState(Qt::Checked);
    }
  else
    {
      mEnableTopologicalEditingCheckBox->setCheckState(Qt::Unchecked);
    }

  int avoidPolygonIntersections = QgsProject::instance()->readNumEntry("Digitizing", "/AvoidPolygonIntersections", 0);
  if(avoidPolygonIntersections != 0)
    {
      mAvoidIntersectionsCheckBox->setCheckState(Qt::Checked);
    }
  else
    {
      mAvoidIntersectionsCheckBox->setCheckState(Qt::Unchecked);
    }

  bool ok;
  QStringList layerIdList = QgsProject::instance()->readListEntry("Digitizing", "/LayerSnappingList", &ok);
  QStringList enabledList = QgsProject::instance()->readListEntry("Digitizing", "/LayerSnappingEnabledList", &ok);
  QStringList toleranceList = QgsProject::instance()->readListEntry("Digitizing", "/LayerSnappingToleranceList", &ok); 
  QStringList snapToList = QgsProject::instance()->readListEntry("Digitizing", "/LayerSnapToList", &ok); 

  QStringList::const_iterator idIter = layerIdList.constBegin();
  QStringList::const_iterator enabledIter = enabledList.constBegin();
  QStringList::const_iterator tolIter = toleranceList.constBegin();
  QStringList::const_iterator snapToIter = snapToList.constBegin();

  QgsMapLayer* currentLayer = 0;

  //create the new layer entries
  for(; idIter != layerIdList.constEnd(); ++idIter, ++enabledIter, ++tolIter, ++snapToIter)
    {
      currentLayer = QgsMapLayerRegistry::instance()->mapLayer(*idIter);
      if(currentLayer)
	{
	  LayerEntry newEntry;
	  newEntry.layerName = currentLayer->name();
	  if( (*enabledIter) == "enabled")
	    {
	      newEntry.checked = true;
	    }
	  else
	    {
	      newEntry.checked = false;
	    }
	  if( (*snapToIter) == "to_vertex")
	    {
	      newEntry.snapTo = 0;
	    }
	  else if( (*snapToIter) == "to_segment")
	    {
	      newEntry.snapTo = 1;
	    }
	  else //to vertex and segment
	    {
	      newEntry.snapTo = 2;
	    }
	  newEntry.tolerance = tolIter->toDouble();
	  mSnappingLayerSettings.insert(*idIter, newEntry);
	}
    }
  
}

QgsProjectProperties::~QgsProjectProperties()
{}



// return the map units
QGis::units QgsProjectProperties::mapUnits() const
{
  return mMapCanvas->mapRender()->mapUnits();
}


void QgsProjectProperties::setMapUnits(QGis::units unit)
{
  // select the button
  if (unit == QGis::UNKNOWN)
  {
    unit = QGis::METERS;
  }
  if (unit==QGis::METERS)
  {
    radMeters->setChecked(true);
  }
  else if(unit==QGis::FEET)
  {
    radFeet->setChecked(true);
  }
  else
  {
    radDecimalDegrees->setChecked(true);
  }
  mMapCanvas->mapRender()->setMapUnits(unit);
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
  QGis::units mapUnit;
  if (radMeters->isChecked())
  {
    mapUnit=QGis::METERS;
  }
  else if(radFeet->isChecked())
  {
    mapUnit=QGis::FEET;
  }
  else
  {
    mapUnit=QGis::DEGREES;
  }

  QgsMapRender* myRender = mMapCanvas->mapRender();
      
  myRender->setMapUnits(mapUnit);

  myRender->setProjectionsEnabled(cbxProjectionEnabled->isChecked());

  // Only change the projection if there is a node in the tree
  // selected that has an srid. This prevents error if the user
  // selects a top-level node rather than an actual coordinate
  // system
  long mySRSID = projectionSelector->getCurrentSRSID();
  if (mySRSID)
  {
    QgsSpatialRefSys srs(mySRSID, QgsSpatialRefSys::QGIS_SRSID);
    myRender->setDestinationSrs(srs);
    
    // write the currently selected projections _proj string_ to project settings
    std::cout << "SpatialRefSys/ProjectSRSProj4String: " <<  projectionSelector->getCurrentProj4String().toLocal8Bit().data() << std::endl;
    QgsProject::instance()->writeEntry("SpatialRefSys","/ProjectSRSProj4String",projectionSelector->getCurrentProj4String());

    // Set the map units to the projected coordinates if we are projecting
    if (isProjected())
    {
      // If we couldn't get the map units, default to the value in the
      // projectproperties dialog box (set above)
      if (srs.mapUnits() != QGis::UNKNOWN)
        myRender->setMapUnits(srs.mapUnits());
    }
  }

  // Set the project title
  QgsProject::instance()->title( title() );

  // set the mouse display precision method and the
  // number of decimal places for the manual option
  // Note. Qt 3.2.3 and greater have a function selectedId() that
  // can be used instead of the two part technique here
  if (radAutomatic->isChecked())
    QgsProject::instance()->writeEntry("PositionPrecision","/Automatic", true);
  else
    QgsProject::instance()->writeEntry("PositionPrecision","/Automatic", false);
  QgsProject::instance()->writeEntry("PositionPrecision","/DecimalPlaces", spinBoxDP->value());
  // Announce that we may have a new display precision setting
  emit displayPrecisionChanged();

  //set the colour for selections
  QColor myColour = pbnSelectionColour->color();
  QgsProject::instance()->writeEntry("Gui","/SelectionColorRedPart",myColour.red());
  QgsProject::instance()->writeEntry("Gui","/SelectionColorGreenPart",myColour.green());
  QgsProject::instance()->writeEntry("Gui","/SelectionColorBluePart",myColour.blue()); 
  QgsRenderer::setSelectionColor(myColour);

  //set the colour for canvas
  myColour = pbnCanvasColor->color();
  QgsProject::instance()->writeEntry("Gui","/CanvasColorRedPart",myColour.red());
  QgsProject::instance()->writeEntry("Gui","/CanvasColorGreenPart",myColour.green());
  QgsProject::instance()->writeEntry("Gui","/CanvasColorBluePart",myColour.blue()); 

  //write the digitizing settings
  int topologicalEditingEnabled = (mEnableTopologicalEditingCheckBox->checkState() == Qt::Checked) ? 1:0;
  QgsProject::instance()->writeEntry("Digitizing", "/TopologicalEditing", topologicalEditingEnabled);
  int avoidPolygonIntersectionsEnabled = (mAvoidIntersectionsCheckBox->checkState() == Qt::Checked) ? 1:0;
  QgsProject::instance()->writeEntry("Digitizing", "/AvoidPolygonIntersections", avoidPolygonIntersectionsEnabled);

  QMap<QString, LayerEntry>::const_iterator layerEntryIt;

  //store the layer snapping settings as string lists
  QStringList layerIdList;
  QStringList snapToList;
  QStringList toleranceList;
  QStringList enabledList;

  for(layerEntryIt = mSnappingLayerSettings.constBegin(); layerEntryIt != mSnappingLayerSettings.constEnd(); ++layerEntryIt)
    {
      layerIdList << layerEntryIt.key();
      toleranceList << QString::number(layerEntryIt->tolerance, 'f');
      if(layerEntryIt->checked)
	{
	  enabledList << "enabled";
	}
      else
	{
	  enabledList << "disabled";
	}
      if(layerEntryIt->snapTo == 0)
	{
	  snapToList << "to_vertex";
	}
      else if(layerEntryIt->snapTo == 1)
	{
	  snapToList << "to_segment";
	}
      else //to vertex and segment
	{
	  snapToList << "to_vertex_and_segment";
	}
    }

  if(mSnappingLayerSettings.size() > 0)
    {
      QgsProject::instance()->writeEntry("Digitizing", "/LayerSnappingList", layerIdList);
      QgsProject::instance()->writeEntry("Digitizing", "/LayerSnapToList", snapToList);
      QgsProject::instance()->writeEntry("Digitizing", "/LayerSnappingToleranceList", toleranceList);
      QgsProject::instance()->writeEntry("Digitizing", "/LayerSnappingEnabledList", enabledList);
    }

  //todo XXX set canvas colour
  emit refresh();
}

bool QgsProjectProperties::isProjected()
{
  return cbxProjectionEnabled->isChecked();
}

void QgsProjectProperties::showProjectionsTab()
{
  tabWidget->setCurrentPage(1);
}

void QgsProjectProperties::on_pbnSelectionColour_clicked()
{
  QColor color = QColorDialog::getColor( pbnSelectionColour->color(), this );
  if (color.isValid())
  {
    pbnSelectionColour->setColor(color);
  }
}

void QgsProjectProperties::on_pbnCanvasColor_clicked()
{
  QColor color = QColorDialog::getColor( pbnCanvasColor->color(), this );
  if (color.isValid())
  {
    pbnCanvasColor->setColor(color);
  }
}

void QgsProjectProperties::on_buttonBox_helpRequested()
{
  std::cout << "running help" << std::endl; 
  QgsContextHelp::run(context_id);
}

void QgsProjectProperties::on_mSnappingOptionsPushButton_clicked()
{
  QgsSnappingDialog d(mMapCanvas, mSnappingLayerSettings);
  if(d.exec() == QDialog::Accepted)
    {
      //retrieve the new layer snapping settings from the dialog
      d.layerSettings(mSnappingLayerSettings);
    }
}
