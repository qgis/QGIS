/***************************************************************************
    qgsmaptoolidentify.cpp  -  map tool for identifying features
    ---------------------
    begin                : January 2006
    copyright            : (C) 2006 by Martin Dobias
    email                : wonder.sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
/* $Id$ */

#include "qgsmessageviewer.h"
#include "qgsmaptoolidentify.h"
#include "qgsmapcanvas.h"
#include "qgsvectordataprovider.h"
#include "qgsvectorlayer.h"
#include "qgsrasterlayer.h"
#include "qgsmaptopixel.h"
#include "qgsidentifyresults.h"
#include "qgsdistancearea.h"
#include "qgsfeature.h"
#include "qgsfeatureattribute.h"
#include "qgsattributedialog.h"
#include "qgscursors.h"
#include <QSettings>
#include <QMessageBox>
#include <QCursor>
#include <QPixmap>

QgsMapToolIdentify::QgsMapToolIdentify(QgsMapCanvas* canvas)
  : QgsMapTool(canvas),
    mResults(0),
    mViewer(0)
{
  // set cursor
  QPixmap myIdentifyQPixmap = QPixmap((const char **) identify_cursor);
  mCursor = QCursor(myIdentifyQPixmap, 1, 1);
}
    
QgsMapToolIdentify::~QgsMapToolIdentify()
{
  if (mResults)
  {
    mResults->done(0);
  }

  if (mViewer)
  {
    delete mViewer;
  }
}

void QgsMapToolIdentify::canvasMoveEvent(QMouseEvent * e)
{
}
  
void QgsMapToolIdentify::canvasPressEvent(QMouseEvent * e)
{
}

void QgsMapToolIdentify::canvasReleaseEvent(QMouseEvent * e)
{
  QgsMapLayer* layer = mCanvas->currentLayer();

  // call identify method for selected layer

  if (layer)
  {
    // In the special case of the WMS provider,
    // coordinates are sent back to the server as pixel coordinates
    // not the layer's native CRS.  So identify on screen coordinates!
    if (
        (layer->type() == QgsMapLayer::RASTER)
        &&
        (dynamic_cast<QgsRasterLayer*>(layer)->providerKey() == "wms")
       )
    {
      identifyRasterWmsLayer(dynamic_cast<QgsRasterLayer*>(layer), QgsPoint(e->x(), e->y()) );
    }
    else
    {
      // convert screen coordinates to map coordinates
      QgsPoint idPoint = mCanvas->getCoordinateTransform()->toMapCoordinates(e->x(), e->y());

      if (layer->type() == QgsMapLayer::VECTOR)
      {
        identifyVectorLayer(dynamic_cast<QgsVectorLayer*>(layer), idPoint);
      }
      else if (layer->type() == QgsMapLayer::RASTER)
      {
        identifyRasterLayer(dynamic_cast<QgsRasterLayer*>(layer), idPoint);
      }
      else
      {
#ifdef QGISDEBUG
        std::cout << "QgsMapToolIdentify::canvasReleaseEvent: unknown layer type!" << std::endl;
#endif
      }
    }

  }
  else
  {
    QMessageBox::warning(mCanvas,
      QObject::tr("No active layer"),
      QObject::tr("To identify features, you must choose an layer active by clicking on its name in the legend"));
  }

  
}


void QgsMapToolIdentify::identifyRasterLayer(QgsRasterLayer* layer, const QgsPoint& point)
{
  if (!layer)
    return;
  
  std::map<QString, QString> attributes;
  layer->identify(point, attributes);
  
  if(!mResults)
  {
    QgsAttributeAction aa;
    mResults = new QgsIdentifyResults(aa, mCanvas->window());
    mResults->setAttribute(Qt::WA_DeleteOnClose);
    // Be informed when the dialog box is closed so that we can stop using it. 
    connect(mResults, SIGNAL(accepted()), this, SLOT(resultsDialogGone()));
    connect(mResults, SIGNAL(rejected()), this, SLOT(resultsDialogGone()));
    mResults->restorePosition();
  }
  else
  {
    mResults->clear();
  }

  mResults->setTitle( layer->name() );
  mResults->setColumnText ( 0, QObject::tr("Band") );

  std::map<QString, QString>::iterator it;
  for (it = attributes.begin(); it != attributes.end(); it++)
  {
    mResults->addAttribute(it->first, it->second);
  }

  mResults->showAllAttributes();
  mResults->show();
}


void QgsMapToolIdentify::identifyRasterWmsLayer(QgsRasterLayer* layer, const QgsPoint& point)
{
  if (!layer)
  {
    return;
  }

  QString text = layer->identifyAsText(point);

  if (text.isEmpty())
  {
    showError(layer);
    return;
  }

  if (!mViewer)
  {
    mViewer = new QgsMessageViewer();
  }

  mViewer->setCaption( layer->name() );
  mViewer->setMessageAsPlainText( text );

//  mViewer->exec();
  mViewer->show();
}


void QgsMapToolIdentify::identifyVectorLayer(QgsVectorLayer* layer, const QgsPoint& point)
{
  if (!layer)
    return;
  
  // load identify radius from settings
  QSettings settings;
  double identifyValue = settings.value("/Map/identifyRadius", QGis::DEFAULT_IDENTIFY_RADIUS).toDouble();

  // create the search rectangle
  double searchRadius = mCanvas->extent().width() * (identifyValue/100.0);
    
  QgsRect r;
  r.setXmin(point.x() - searchRadius);
  r.setXmax(point.x() + searchRadius);
  r.setYmin(point.y() - searchRadius);
  r.setYmax(point.y() + searchRadius);
  
  if (layer->projectionsEnabled())
  { 
    try
    {
      r = layer->coordinateTransform()->transform(r, QgsCoordinateTransform::INVERSE);
    }
    catch (QgsCsException &e)
    {
      qDebug("Inverse transform error in %s line %d:\n%s",
             __FILE__, __LINE__, e.what());
    }
  }
  
  int featureCount = 0;
  QgsFeature *fet;
  QgsAttributeAction& actions = *layer->actions();
  QString fieldIndex = layer->displayField();
  QgsVectorDataProvider* dataProvider = layer->getDataProvider();
  dataProvider->select(&r, true);

  QgsDistanceArea calc;
  calc.setSourceSRS(layer->coordinateTransform()->sourceSRS().srsid());
  
  if ( !layer->isEditable() )
  {
    // display features falling within the search radius
    if(!mResults)
    {
      mResults = new QgsIdentifyResults(actions, mCanvas->window());
      mResults->setAttribute(Qt::WA_DeleteOnClose);
      // Be informed when the dialog box is closed so that we can stop using it.
      connect(mResults, SIGNAL(accepted()), this, SLOT(resultsDialogGone()));
      connect(mResults, SIGNAL(rejected()), this, SLOT(resultsDialogGone()));
      // restore the identify window position and show it
      mResults->restorePosition();
    }
    else
    {
      mResults->raise();
      mResults->clear();
      mResults->setActions(actions);
    }

    QApplication::setOverrideCursor(Qt::WaitCursor);

    while ((fet = dataProvider->getNextFeature(true)))
    {
      featureCount++;

      QTreeWidgetItem* featureNode = mResults->addNode("foo");
      featureNode->setText(0, fieldIndex);
      std::vector < QgsFeatureAttribute > attr = fet->attributeMap();
      // Do this only once rather than each pass through the loop
      int attrSize = attr.size();
      for (register int i = 0; i < attrSize; i++)
      {
#ifdef QGISDEBUG
        std::cout << attr[i].fieldName().toLocal8Bit().data() << " == " << fieldIndex.toLocal8Bit().data() << std::endl;
#endif
        if (attr[i].fieldName().lower() == fieldIndex)
        {
          featureNode->setText(1, attr[i].fieldValue());
        }
        mResults->addAttribute(featureNode, attr[i].fieldName(), attr[i].fieldValue());
      }

      // Calculate derived attributes and insert:
      // measure distance or area depending on geometry type
      if (layer->vectorType() == QGis::Line)
      {
        double dist = calc.measure(fet->geometry());
        QString str = QString::number(dist/1000, 'f', 3);
        str += " km";
        mResults->addDerivedAttribute(featureNode, QObject::tr("Length"), str);
      }
      else if (layer->vectorType() == QGis::Polygon)
      {
        double area = calc.measure(fet->geometry());
        QString str = QString::number(area/1000000, 'f', 3);
        str += " km^2";
        mResults->addDerivedAttribute(featureNode, QObject::tr("Area"), str);
      }

      // Add actions 
      QgsAttributeAction::aIter iter = actions.begin();
      for (register int i = 0; iter != actions.end(); ++iter, ++i)
      {
        mResults->addAction( featureNode, i, QObject::tr("action"), iter->name() );
      }

      delete fet;
    }

#ifdef QGISDEBUG
    std::cout << "Feature count on identify: " << featureCount << std::endl;
#endif

    //also test the not commited features //todo: eliminate copy past code

    mResults->setTitle(layer->name() + " - " + QString::number(featureCount) + QObject::tr(" features found"));
    if (featureCount == 1) 
    {
      mResults->showAllAttributes();
      mResults->setTitle(layer->name() + " - " + QObject::tr(" 1 feature found") );
    }
    else if (featureCount == 0)
    {
      mResults->setTitle(layer->name() + " - " + QObject::tr("No features found") );
      mResults->setMessage ( QObject::tr("No features found"), QObject::tr("No features were found in the active layer at the point you clicked") );
    }
    else
    {
      QString title = layer->name();
      title += QString(" - %1").arg(featureCount);
      title += QObject::tr(" features found");
      mResults->setTitle(title);    
    }
    QApplication::restoreOverrideCursor();

    mResults->show();
  } 
  else // ( layer->isEditable() )
  {
    // Edit attributes 
    // TODO: what to do if more features were selected? - nearest?
    changed_attr_map& changedAttributes = layer->changedAttributes();
    
    QApplication::setOverrideCursor(Qt::WaitCursor);

    if ( (fet = dataProvider->getNextFeature(true)) )
    {
      // these are the values to populate the dialog with
      std::vector < QgsFeatureAttribute > old;

      // start off with list of committed attribute values
      old = fet->attributeMap();

      // Test if this feature already changed since the last commit

      changed_attr_map::iterator it = changedAttributes.find(fet->featureId());
      if ( it != changedAttributes.end() )
      {
        // Yes, this feature already has in-memory attribute changes

        // go through and apply the modified-but-not-committed values
        std::map<QString,QString> oldattr = (*it).second;
        int index=0;
        for ( std::vector<QgsFeatureAttribute>::const_iterator
                oldit  = old.begin();
                oldit != old.end();
              ++oldit)
        {
          std::map<QString,QString>::iterator ait =
            oldattr.find( (*oldit).fieldName() );
          if ( ait != oldattr.end() )
          {
            // replace the committed value with the
            // modified-but-not-committed value
            old[index] = QgsFeatureAttribute ( (*ait).first, (*ait).second );
          }

          ++index;
        }
      }

      QApplication::restoreOverrideCursor();

      // Show the attribute value editing dialog
      QgsAttributeDialog ad( &old );

      if (ad.exec() == QDialog::Accepted)
      {

        int oldSize = old.size();


        for (int i = 0; i < oldSize; ++i)
        {
          // only apply changed values if they were edited by the user
          if (ad.isDirty(i))
          {
#ifdef QGISDEBUG
        std::cout << "QgsMapToolIdentify::identifyVectorLayer: found an changed attribute: "
          << old[i].fieldName().toLocal8Bit().data()
          << " = "
          << ad.value(i).toLocal8Bit().data()
          << "." << std::endl;
#endif
            changedAttributes[ fet->featureId() ][ old[i].fieldName() ] = ad.value(i);

            // propagate "dirtyness" to the layer
            layer->setModified();
          }
        }

      }
    }
    else
    {
      QApplication::restoreOverrideCursor();
      QMessageBox::information(0, tr("No features found"), 
			       tr("<p>No features were found within the search radius. "
				  "Note that it is currently not possible to use the "
				  "identify tool on unsaved features.</p>"));
    }
  }
  dataProvider->reset();
}


void QgsMapToolIdentify::showError(QgsMapLayer * mapLayer)
{
//   QMessageBox::warning(
//     this,
//     mapLayer->errorCaptionString(),
//     tr("Could not draw") + " " + mapLayer->name() + " " + tr("because") + ":\n" +
//       mapLayer->errorString()
//   );

  QgsMessageViewer * mv = new QgsMessageViewer();
  mv->setCaption( mapLayer->errorCaptionString() );
  mv->setMessageAsPlainText(
    QObject::tr("Could not identify objects on") + " " + mapLayer->name() + " " + QObject::tr("because") + ":\n" +
    mapLayer->errorString()
  );
  mv->exec();
  delete mv;

}

void QgsMapToolIdentify::resultsDialogGone()
{
  mResults = 0;
}

void QgsMapToolIdentify::deactivate()
{
  if (mResults)
    mResults->done(0); // close the window
}
