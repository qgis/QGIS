/***************************************************************************
  qgsprojectio.cpp - Save/Restore QGIS project
  --------------------------------------
Date                 : 19-Oct-2003
Copyright            : (C) 2003 by Gary E.Sherman
email                : sherman at mrcc.com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
/* qgsprojectio.cpp,v 1.45 2004/07/11 06:01:09 mhugent Exp */
#include <iostream>
#include <fstream>
#include <qfiledialog.h>
#include <qfileinfo.h>
#include <qdom.h>
#include <qmessagebox.h>
#include <qcolor.h>
#include <qapplication.h>
#include <qcursor.h>

#include "qgsmaplayer.h"
#include "qvariant.h"
#include "qgsvectorlayer.h"
#include "qgsrasterlayer.h"
#include "qgsmapcanvas.h"
#include "qgsrect.h"
#include "qgsprojectio.h"
#include "qgssinglesymrenderer.h"
#include "qgssimarenderer.h"
#include "qgsgraduatedmarenderer.h"
#include "qgsgraduatedsymrenderer.h"
#include "qgscontinuouscolrenderer.h"
#include "qgssymbologyutils.h"
#include "qgssisydialog.h"
#include "qgssimadialog.h"
#include "qgsgrasydialog.h"
#include "qgscontcoldialog.h"
#include "qgsdlgvectorlayerproperties.h"
#include "qgisapp.h"
#include "qgsmarkersymbol.h"
#include "qgsmaplayerregistry.h"
#include <map>

    QgsProjectIo::QgsProjectIo(int _action, QgsMapCanvas * theMapCanvas)
:  action(_action)
{
 mMapCanvas=theMapCanvas;
}


QgsProjectIo::~QgsProjectIo()
{}


QString QgsProjectIo::baseName()
{
  QFileInfo fi(fullPath);
  return fi.baseName(true);
}

bool QgsProjectIo::write(QgsRect theRect)
{
  if (fullPath.isEmpty())
  {
    selectFileName();
  }
  //QMessageBox::information(0,"Full Path",fullPath);
  int okToSave = 0;
  if (QFile::exists(fullPath) && (action == SAVEAS))
  {
    okToSave =
        QMessageBox::warning(0, QObject::tr("Overwrite File?"),
                QObject::tr("%1 exists.%2Do you want to overwrite it?").arg(fullPath).arg("\n"), QObject::tr("Yes"),
                QObject::tr("No"));
  }
  if (okToSave == 0)
  {
    // write the project information to the selected file
    writeXML(theRect);
    return true;
  } else
  {
    return false;
  }
}

//returns the zOrder of the restored project
std::list<QString> QgsProjectIo::read(QString path)
{
  // Get the  registry (its a singleton so
  // it will be the same registry data as used by the map canvas)
  QgsMapLayerRegistry * myMapLayerRegistry = QgsMapLayerRegistry::instance();
  std::list<QString> myZOrder;
  myZOrder.clear();
  if(path.isNull())
  {
    path = selectFileName();
  }

  std::auto_ptr<QDomDocument> doc;

  if (!path.isEmpty())
  {
    doc = std::auto_ptr<QDomDocument>(new QDomDocument("qgisdocument"));
    QFile file(path);
    if (!file.open(IO_ReadOnly))
    {
      return myZOrder;
    }
    if (!doc->setContent(&file))
    {
      file.close();
      return myZOrder;
    }
    file.close();
    //enable the hourglass
    qWarning("opened document" + file.name());
    // get the extent
    QDomNodeList extents = doc->elementsByTagName("extent");
    QDomNode extentNode = extents.item(0);
    QDomNode xminNode = extentNode.namedItem("xmin");
    QDomNode yminNode = extentNode.namedItem("ymin");
    QDomNode xmaxNode = extentNode.namedItem("xmax");
    QDomNode ymaxNode = extentNode.namedItem("ymax");
    QDomElement exElement = xminNode.toElement();
    double xmin = exElement.text().toDouble();
    exElement = yminNode.toElement();
    double ymin = exElement.text().toDouble();
    exElement = xmaxNode.toElement();
    double xmax = exElement.text().toDouble();
    exElement = ymaxNode.toElement();
    double ymax = exElement.text().toDouble();
    QgsRect savedExtent(xmin, ymin, xmax, ymax);


    QDomNodeList nl = doc->elementsByTagName("maplayer");
    QString layerCount;
    layerCount = layerCount.setNum(nl.count());
    //QMessageBox::information(0, "Number of map layers", layerCount);
    QString wk;
    // process the map layer nodes
    for (int i = 0; i < nl.count(); i++)
    {
      QDomNode node = nl.item(i);
      QDomElement element = node.toElement();
      QString type = element.attribute("type");
      QString visible = element.attribute("visible");
      QString showInOverview = element.attribute("showInOverviewFlag");

      //QMessageBox::information(0,"Type of map layer", type);
      // process layer name
      QDomNode mnl = node.namedItem("layername");
      QTextStream ts(&wk, IO_WriteOnly);
      ts << mnl.nodeType();
      //QMessageBox::information(0,"Node Type", wk);
      QDomElement mne = mnl.toElement();
      //QMessageBox::information(0,"Layer Name", mne.text());
      QString layerName = mne.text();
      
      //process data source
      mnl = node.namedItem("datasource");
      mne = mnl.toElement();
      //QMessageBox::information(0,"Datasource Name", mne.text());
      QString dataSource = mne.text();

      //process zorder
      mnl = node.namedItem("zorder");
      mne = mnl.toElement();
      //QMessageBox::information(0,"Zorder", mne.text());

      // XXX I strongly suggest that much of this be pushed into the
      // XXX provider objects.  There just be just enough here to dispatch the
      // XXX read to a provider.  --MAC

      // add the layer to the maplayer

      QString myNewLayerId="";

      if (type == "vector")
      {
	//process provider key
	QDomNode pkeyNode = node.namedItem("provider");
	QString provider;
	if (pkeyNode.isNull())
	  provider = "";
	else
	{
	  QDomElement pkeyElt = pkeyNode.toElement();
	  provider = pkeyElt.text();
	}
	
        // determine type of vector layer
        if (provider != "")
	{
	  
	} else if ((dataSource.find("host=") > -1) && 
		   (dataSource.find("dbname=") > -1))
        {
          provider = "postgres";
        } else
        {
          provider = "ogr";
        }
        QgsVectorLayer *dbl = new QgsVectorLayer(dataSource, layerName, provider);

        Q_CHECK_PTR( dbl );

        if ( ! dbl )
        {
#ifdef QGISDEBUG
          std::cerr << __FILE__ << ":" << __LINE__
              << " unable to create vector layer for "
              << dataSource << "\n"; 
#endif                  
          return myZOrder;
        }

        if ( ! dbl->isValid() )
        {
#ifdef QGISDEBUG
          std::cerr << __FILE__ << ":" << __LINE__
              << " created vector layer for "
              << dataSource << "is invalid ... skipping\n"; 
#endif
          delete dbl;   // discard bogus layer

          // XXX naturally we could be smart and ask the user for the
          // XXX new location of the data, but for now we'll just
          // XXX ignore the missing data and move on.  Perhaps this
          // XXX will be revisited when the architecture is refactored.

          return myZOrder;
        }
        myNewLayerId=dbl->getLayerID();

        QDomNode singlenode = node.namedItem("singlesymbol");
        QDomNode graduatednode = node.namedItem("graduatedsymbol");
        QDomNode continuousnode = node.namedItem("continuoussymbol");
        QDomNode singlemarkernode = node.namedItem("singlemarker");
        QDomNode graduatedmarkernode = node.namedItem("graduatedmarker");

        QgsRenderer* renderer;

        if (!singlenode.isNull())
        {
          renderer = new QgsSingleSymRenderer();
          renderer->readXML(singlenode,*dbl);
        }
        else if (!graduatednode.isNull())
        {
          renderer = new QgsGraduatedSymRenderer();
          renderer->readXML(graduatednode,*dbl);
        }
        else if (!continuousnode.isNull())
        {
          renderer = new QgsContinuousColRenderer();
          renderer->readXML(continuousnode,*dbl);
        }
        else if(!singlemarkernode.isNull())
        {
          renderer = new QgsSiMaRenderer();
          renderer->readXML(singlemarkernode,*dbl);
        }
        else if(!graduatedmarkernode.isNull())
        {
          renderer = new QgsGraduatedMaRenderer();
          renderer->readXML(graduatedmarkernode,*dbl);
        }

        dbl->setVisible(visible == "1");
        if (showInOverview == "1")
        {
          dbl->toggleShowInOverview();
	}
        myMapLayerRegistry->addMapLayer(dbl);
        myZOrder.push_back(dbl->getLayerID());
      } 
      else if (type == "raster")
      {
        QgsRasterLayer *myRasterLayer = new QgsRasterLayer(dataSource, layerName);
        myNewLayerId=myRasterLayer->getLayerID();

        myRasterLayer->setVisible(visible == "1");
        if (showInOverview == "1")
        {
          myRasterLayer->toggleShowInOverview();
	}

        mnl = node.namedItem("rasterproperties");

        QDomNode snode = mnl.namedItem("showDebugOverlayFlag");
        QDomElement myElement = snode.toElement();
        QVariant myQVariant = (QVariant) myElement.attribute("boolean");
        myRasterLayer->setShowDebugOverlayFlag(myQVariant.toBool());

        snode = mnl.namedItem("drawingStyle");
        myElement = snode.toElement();
        myRasterLayer->setDrawingStyle(myElement.text());

        snode = mnl.namedItem("invertHistogramFlag");
        myElement = snode.toElement();
        myQVariant = (QVariant) myElement.attribute("boolean");
        myRasterLayer->setInvertHistogramFlag(myQVariant.toBool());

        snode = mnl.namedItem("stdDevsToPlotDouble");
        myElement = snode.toElement();
        myRasterLayer->setStdDevsToPlot(myElement.text().toDouble());

        snode = mnl.namedItem("transparencyLevelInt");
        myElement = snode.toElement();
        myRasterLayer->setTransparency(myElement.text().toInt());

        snode = mnl.namedItem("redBandNameQString");
        myElement = snode.toElement();
        myRasterLayer->setRedBandName(myElement.text());
        snode = mnl.namedItem("greenBandNameQString");
        myElement = snode.toElement();
        myRasterLayer->setGreenBandName(myElement.text());

        snode = mnl.namedItem("blueBandNameQString");
        myElement = snode.toElement();
        myRasterLayer->setBlueBandName(myElement.text());

        snode = mnl.namedItem("grayBandNameQString");
        myElement = snode.toElement();
        myRasterLayer->setGrayBandName(myElement.text());

        myMapLayerRegistry->addMapLayer(myRasterLayer);
        myZOrder.push_back(myRasterLayer->getLayerID());
      }
      mMapCanvas->setExtent(savedExtent);
    }

    QApplication::restoreOverrideCursor();
    return myZOrder;
  }
}

QString QgsProjectIo::selectFileName()
{
  if (action == SAVE && fullPath.isEmpty())
  {
    action = SAVEAS;
  }
  switch (action)
  {
      case OPEN:
          fullPath =
              QFileDialog::getOpenFileName("./", QObject::tr("QGis files (*.qgs)"), 0, 0,
                      QObject::tr("Choose a QGIS project file to open"));
          break;
      case SAVEAS:
          fullPath =
              QFileDialog::getSaveFileName("./", QObject::tr("QGis files (*.qgs)"), 0, 0, QObject::tr("Choose a filename to save"));
          break;
  }
  return fullPath;
}

void QgsProjectIo::setFileName(QString fn)
{
  fullPath = fn;
}

QString QgsProjectIo::fullPathName()
{
  return fullPath;
}



void QgsProjectIo::writeXML(QgsRect theExtent)
{
  std::ofstream xml(fullPath);
  if (!xml.fail())
  {
    xml << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>" << std::endl;
    xml << "<!DOCTYPE qgis SYSTEM \"http://mrcc.com/qgis.dtd\">" << std::endl;
    xml << "<qgis projectname=\"default project\">\n";
    xml << "<title>QGis Project File</title>\n";

    xml << "<extent>\n";

    xml << "\t<xmin>" << theExtent.xMin() << "</xmin>\n";
    xml << "\t<ymin>" << theExtent.yMin() << "</ymin>\n";
    xml << "\t<xmax>" << theExtent.xMax() << "</xmax>\n";
    xml << "\t<ymax>" << theExtent.yMax() << "</ymax>\n";
    xml << "</extent>\n";

    // get the layer registry so we can write the layer data to disk
    // the registry is a singleton so
    // it will be the same registry data as used by the map canvas
    QgsMapLayerRegistry * myMapLayerRegistry = QgsMapLayerRegistry::instance();
    std::list<QString> myZOrder = mMapCanvas->zOrders();
    xml << "<projectlayers layercount=\"" << myZOrder.size() << "\"> \n";
    int i=0;
    std::list<QString>::iterator myMaplayerIterator;
    for ( myMaplayerIterator = myZOrder.begin(); myMaplayerIterator != myZOrder.end(); ++myMaplayerIterator ) 
    {
      QgsMapLayer *lyr = myMapLayerRegistry->mapLayer(*myMaplayerIterator);
      bool isDatabase = false;
      xml << "\t<maplayer type=\"";
      switch (lyr->type())
      {
          case QgsMapLayer::VECTOR:
              xml << "vector";
              break;
          case QgsMapLayer::RASTER:
              xml << "raster";
              break;
          case QgsMapLayer::DATABASE:
              xml << "database";
              isDatabase = true;
              break;
      }
      //
      // layer visibility
      //
      xml << "\" visible=\"";
      if (lyr->visible())
      {
        xml << "1";
      } else
      {
        xml << "0";
      }
      //
      // layer is shown in overview?
      //
      xml << "\" showInOverviewFlag=\"";
      if (lyr->showInOverviewStatus())
      {
        xml << "1";
      } else
      {
        xml << "0";
      }
      xml << "\">\n";
      
      if (isDatabase)
      {
        // cast the layer to a qgsdatabaselayer
        // TODO fix this so database layers are properly saved/restored 
        // when name is changed in legend
        /* QgsDatabaseLayer *dblyr = (QgsDatabaseLayer *)lyr;
           xml << "\t\t<layername>" + dblyr->schemaName() << "." <<
           dblyr->geometryTableName() << "</layername>\n"; */
        xml << "\t\t<layername>" << lyr->name().ascii() << "</layername>\n";
      } else
      {
        xml << "\t\t<layername>" << lyr->name().ascii() << "</layername>\n";
      }
      xml << "\t\t<datasource>" << lyr->source().replace('&', "&amp;").ascii()
	  << "</datasource>\n";
      xml << "\t\t<zorder>" << i << "</zorder>\n";
      if (lyr->type() != QgsMapLayer::RASTER)
      {
        QgsVectorLayer *layer = dynamic_cast < QgsVectorLayer * >(lyr);
        if (!layer)
        {
          qWarning("Warning, cast failed in QgsProjectIo, line 451");
        }
	
	xml << "\t\t<provider>" << layer->providerType() << "</provider>\n";
	
        QgsRenderer* renderer;
        if(renderer=layer->renderer())
        {
          renderer->writeXML(xml);
        }

      } else                //raster layer properties
      {
        //cast the maplayer to rasterlayer
        QgsRasterLayer *myRasterLayer = (QgsRasterLayer *) lyr;
        //Raster flag to indicate whether debug infor overlay should be rendered onto the raster

        xml << "\t\t<rasterproperties>\n";
        xml << "\t\t\t<showDebugOverlayFlag boolean=\"";
        if (myRasterLayer->getShowDebugOverlayFlag())
        {
          xml << "true\"/>\n";
        } else
        {
          xml << "false\"/>\n";
        }

        // The drawing style for the layer
        xml << "\t\t\t<drawingStyle>" << myRasterLayer->getDrawingStyleAsQString().ascii() << "</drawingStyle>\n";
        //Raster : flag indicating whether the histogram should be inverted or not 
        xml << "\t\t\t<invertHistogramFlag boolean=\"";
        if (myRasterLayer->getInvertHistogramFlag())
        {
          xml << "true\"/>\n";
        } else
        {
          xml << "false\"/>\n";
        }
        //Raster : Number of stddev to plot (0) to ignore -->
        xml << "\t\t\t<stdDevsToPlotDouble>" << myRasterLayer->getStdDevsToPlot() << "</stdDevsToPlotDouble>\n";
        //Raster transparency for this layer should be 0-255 -->
        xml << "\t\t\t<transparencyLevelInt>" << myRasterLayer->getTransparency() << "</transparencyLevelInt>\n";
        //Raster : the band to be associated with the color red - usually red -->
        xml << "\t\t\t<redBandNameQString>" << myRasterLayer->getRedBandName().ascii() << "</redBandNameQString>\n";
        //Raster : the band to be associated with the color green - usually green -->
        xml << "\t\t\t<greenBandNameQString>" << myRasterLayer->getGreenBandName().ascii() << "</greenBandNameQString>\n";
        //Raster : the band to be associated with the color blue - usually blue -->
        xml << "\t\t\t<blueBandNameQString>" << myRasterLayer->getBlueBandName().ascii() << "</blueBandNameQString>\n";
        //Raster :  the band to be associated with the grayscale only ouput - usually gray  -->
        xml << "\t\t\t<grayBandNameQString>" << myRasterLayer->getGrayBandName().ascii() << "</grayBandNameQString>\n";
        xml << "\t\t</rasterproperties>\n";
      }
      xml << "\t</maplayer>\n";
      i++;
    }
    xml << "</projectlayers>\n";
    xml << "</qgis>\n";
    xml.close();
  } 
}
