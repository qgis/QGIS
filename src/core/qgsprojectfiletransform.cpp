/***************************************************************************
                          qgsprojectfiletransform.cpp  -  description
                             -------------------
    begin                : Sun 15 dec 2007
    copyright            : (C) 2007 by Magnus Homann
    email                : magnus at homann.se
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
 /* $Id: $ */


#include "qgsprojectfiletransform.h"
#include "qgsprojectversion.h"
#include "qgslogger.h"
#include <QTextStream>
#include <QDomDocument>

typedef QgsProjectVersion PFV;


QgsProjectFileTransform::transform QgsProjectFileTransform::transformers[] = {
  {PFV(0,8,1), PFV(0,9,0), &QgsProjectFileTransform::transform081to090},
  {PFV(0,9,0), PFV(0,9,1), &QgsProjectFileTransform::transform090to091},
  {PFV(0,9,1), PFV(0,9,2), &QgsProjectFileTransform::transformNull}
};

bool QgsProjectFileTransform::updateRevision(QgsProjectVersion newVersion)
{
  bool returnValue = false;

  if ( ! mDom.isNull() )
  {
    for (int i = 0; i < sizeof(transformers)/sizeof(transform); i++)
    {
      if ( transformers[i].from == mCurrentVersion)
      {
        // Run the transformer, and update the revision in every case
        (this->*(transformers[i].transformFunc))();
        mCurrentVersion = transformers[i].to;
        returnValue = true;
      }
    }
  }
  return returnValue;
};

void QgsProjectFileTransform::dump()
{
  QgsDebugMsg(QString("Current project file version is %1.%2.%3\n")
              .arg(mCurrentVersion.majorVersion())
              .arg(mCurrentVersion.minorVersion())
              .arg(mCurrentVersion.subVersion()));
#ifdef QGISDEBUG
  // Using QgsDebugMsg() didn't print the entire mDom...
  std::cout << mDom.toString(2).toLatin1().constData();
#endif
}

/*
 *  Transformers below!
 */

void QgsProjectFileTransform::transform081to090()
{
  QgsDebugMsg("Entering...");
  if ( ! mDom.isNull() )
  {
    // Start with inserting a mapcanvas element and populate it

    QDomElement mapCanvas; // A null element.

    // there should only be one <qgis>
    QDomNode qgis = mDom.firstChildElement("qgis");
    if ( ! qgis.isNull() )
    {
      QgsDebugMsg("Populating new mapcanvas");

      // Create a mapcanvas
      mapCanvas = mDom.createElement("mapcanvas");
      // Append mapcanvas to parent 'qgis'.
      qgis.appendChild(mapCanvas);
      // Re-parent units
      mapCanvas.appendChild(qgis.namedItem("units"));
      // Re-parent extent
      mapCanvas.appendChild(qgis.namedItem("extent"));

      // See if we can find if projection is on.
      
      QDomElement properties = qgis.firstChildElement("properties");
      QDomElement spatial = properties.firstChildElement("SpatialRefSys");
      QDomElement projectionsEnabled = spatial.firstChildElement("ProjectionsEnabled"); 
      // Type is 'int', and '1' if on.
      // Create an element
      QDomElement projection = mDom.createElement("projections");
      QgsDebugMsg(QString("Projection flag: ") + projectionsEnabled.text());
      // Set flag from ProjectionsEnabled
      projection.appendChild(mDom.createTextNode(projectionsEnabled.text()));
      // Set new element as child of <mapcanvas>
      mapCanvas.appendChild(projection);

    }


    // Transforming coordinate-transforms
    // Create a list of all map layers
    QDomNodeList mapLayers = mDom.elementsByTagName("maplayer");
    bool doneDestination = false;
    for (int i = 0; i < mapLayers.count(); i++)
    {
      QDomNode mapLayer = mapLayers.item(i);
      // Find the coordinatetransform
      QDomNode coordinateTransform = mapLayer.namedItem("coordinatetransform");
      // Find the sourcesrs
      QDomNode sourceSRS = coordinateTransform.namedItem("sourcesrs");
      // Rename to srs
      sourceSRS.toElement().setTagName("srs");
      // Re-parent to maplayer
      mapLayer.appendChild(sourceSRS);
      // Re-move coordinatetransform
      // Take the destination SRS of the first layer and use for mapcanvas projection
      if (! doneDestination)
      {
        // Use destination SRS from the last layer
        QDomNode destinationSRS = coordinateTransform.namedItem("destinationsrs");
        // Re-parent the destination SRS to the mapcanvas
        // If mapcanvas wasn't set, nothing will happen.
        mapCanvas.appendChild(destinationSRS);
        // Only do this once
        doneDestination = true;
      }
      mapLayer.removeChild(coordinateTransform);
      //QDomNode id = mapLayer.namedItem("id");
      //QgsDebugMsg(QString("Found maplayer ") + id.toElement().text());
      
    }

    // Set the flag 'visible' to match the status of 'checked'
    QDomNodeList legendLayerFiles = mDom.elementsByTagName("legendlayerfile");
    QgsDebugMsg(QString("Legend layer file entrie: ") + QString::number(legendLayerFiles.count())); 
    for (int i = 0; i < mapLayers.count(); i++)
    {
      // Get one maplayer element from list
      QDomElement mapLayer = mapLayers.item(i).toElement();
      // Find it's id.
      QString id = mapLayer.firstChildElement("id").text();
      QgsDebugMsg(QString("Handling layer " + id));
      // Now, look it up in legend
      for (int j = 0; j < legendLayerFiles.count(); j++)
      {
        QDomElement legendLayerFile = legendLayerFiles.item(j).toElement();
        if (id == legendLayerFile.attribute("layerid") )
        {
          // Found a the legend layer that matches the maplayer
          QgsDebugMsg("Found matching id");

          // Set visible flag from maplayer to legendlayer
          legendLayerFile.setAttribute("visible", mapLayer.attribute("visible"));

          // Set overview flag from maplayer to legendlayer
          legendLayerFile.setAttribute("inOverview", mapLayer.attribute("showInOverviewFlag"));
        }
      }
    }
  }
  return;

};

void QgsProjectFileTransform::transform090to091()
{
  QgsDebugMsg("entering");
  if ( ! mDom.isNull() )
  {
  }
  return;

};

