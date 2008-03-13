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

#include "qgsattributedialog.h"
#include "qgscursors.h"
#include "qgsdistancearea.h"
#include "qgsfeature.h"
#include "qgsfield.h"
#include "qgsgeometry.h"
#include "qgslogger.h"
#include "qgsidentifyresults.h"
#include "qgsmapcanvas.h"
#include "qgsmaptopixel.h"
#include "qgsmessageviewer.h"
#include "qgsmaptoolidentify.h"
#include "qgsrasterlayer.h"
#include "qgsrubberband.h"
#include "qgsspatialrefsys.h"
#include "qgsvectordataprovider.h"
#include "qgsvectorlayer.h"

#include <QSettings>
#include <QMessageBox>
#include <QCursor>
#include <QPixmap>

QgsMapToolIdentify::QgsMapToolIdentify(QgsMapCanvas* canvas)
  : QgsMapTool(canvas),
    mResults(0),
    mRubberBand(0)
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

  delete mRubberBand;
}

void QgsMapToolIdentify::canvasMoveEvent(QMouseEvent * e)
{
}
  
void QgsMapToolIdentify::canvasPressEvent(QMouseEvent * e)
{
}

void QgsMapToolIdentify::canvasReleaseEvent(QMouseEvent * e)
{
  if(!mCanvas || mCanvas->isDrawing())
    {
      return;
    }

  QgsMapLayer* layer = mCanvas->currentLayer();
  
  // delete rubber band if there was any
  delete mRubberBand;
  mRubberBand = 0;

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
      QObject::tr("To identify features, you must choose an active layer by clicking on its name in the legend"));
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

  mResults->addAttribute( tr("(clicked coordinate)"), point.stringRep() );

  mResults->showAllAttributes();
  mResults->show();
}


void QgsMapToolIdentify::identifyRasterWmsLayer(QgsRasterLayer* layer, const QgsPoint& point)
{
  if (!layer)
  {
    return;
  }

  //if WMS layer does not cover the view origin,
  //we need to map the view pixel coordinates
  //to WMS layer pixel coordinates
  QgsRect viewExtent = mCanvas->extent();
  double mupp = mCanvas->mupp();
  if(mupp == 0)
    {
      return;
    }
  double xMinView = viewExtent.xMin();
  double yMaxView = viewExtent.yMax();

  QgsRect layerExtent = layer->extent();
  double xMinLayer = layerExtent.xMin();
  double yMaxLayer = layerExtent.yMax();

  double i, j;

  if(xMinView < xMinLayer)
    {
      i = (int)(point.x() - (xMinLayer - xMinView) / mupp); 
    }
  else
    {
      i = point.x();
    }

  if(yMaxView > yMaxLayer)
    {
      j = (int)(point.y() - (yMaxView - yMaxLayer) / mupp);
    }
  else
    {
      j = point.y();
    }
  

  QString text = layer->identifyAsText(QgsPoint(i, j));
  
  if (text.isEmpty())
  {
    showError(layer);
    return;
  }

  QgsMessageViewer* viewer = new QgsMessageViewer();
  viewer->setCaption( layer->name() );
  viewer->setMessageAsPlainText( QString(tr("WMS identify result for %1\n%2")).arg(point.stringRep()).arg(text) );

  viewer->showMessage(); // deletes itself on close
}

void QgsMapToolIdentify::identifyVectorLayer(QgsVectorLayer* layer, const QgsPoint& point)
{
  if (!layer)
    return;
  
  // load identify radius from settings
  QSettings settings;
  double identifyValue = settings.value("/Map/identifyRadius", QGis::DEFAULT_IDENTIFY_RADIUS).toDouble();
  QString ellipsoid = settings.readEntry("/qgis/measure/ellipsoid", "WGS84");

  // create the search rectangle
  double searchRadius = mCanvas->extent().width() * (identifyValue/100.0);
    
  QgsRect r;
  r.setXmin(point.x() - searchRadius);
  r.setXmax(point.x() + searchRadius);
  r.setYmin(point.y() - searchRadius);
  r.setYmax(point.y() + searchRadius);
  
  r = toLayerCoords(layer, r);

  int featureCount = 0;
  //QgsFeature feat;
  QgsAttributeAction& actions = *layer->actions();
  QString fieldIndex = layer->displayField();
  QgsVectorDataProvider* dataProvider = layer->getDataProvider();
  const QgsFieldMap& fields = dataProvider->fields();

  // init distance/area calculator
  QgsDistanceArea calc;
  calc.setProjectionsEnabled(mCanvas->projectionsEnabled()); // project?
  calc.setEllipsoid(ellipsoid);
  calc.setSourceSRS(layer->srs().srsid());
  
  // display features falling within the search radius
  if(!mResults)
    {
      mResults = new QgsIdentifyResults(actions, mCanvas->window());
      mResults->setAttribute(Qt::WA_DeleteOnClose);
      // Be informed when the dialog box is closed so that we can stop using it.
      connect(mResults, SIGNAL(accepted()), this, SLOT(resultsDialogGone()));
      connect(mResults, SIGNAL(rejected()), this, SLOT(resultsDialogGone()));
      connect(mResults, SIGNAL(selectedFeatureChanged(int)), this, SLOT(highlightFeature(int)));
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
  
  int lastFeatureId = 0;
  
  QList<QgsFeature> featureList;
  layer->featuresInRectangle(r, featureList, true, true);
  QList<QgsFeature>::iterator f_it = featureList.begin();
    
  for(; f_it != featureList.end(); ++f_it)
      {
	featureCount++;
	
	QTreeWidgetItem* featureNode = mResults->addNode("foo");
	featureNode->setData(0, Qt::UserRole, QVariant(f_it->featureId())); // save feature id
	lastFeatureId = f_it->featureId();
	featureNode->setText(0, fieldIndex);
	const QgsAttributeMap& attr = f_it->attributeMap();
	
	for (QgsAttributeMap::const_iterator it = attr.begin(); it != attr.end(); ++it)
	  {
	    //QgsDebugMsg(it->fieldName() + " == " + fieldIndex);
	    
	    if (fields[it.key()].name() == fieldIndex)
	      {
		featureNode->setText(1, it->toString());
	      }
	    mResults->addAttribute(featureNode, fields[it.key()].name(), it->isNull() ? "NULL" : it->toString());
	  }
	
	// Calculate derived attributes and insert:
	// measure distance or area depending on geometry type
	if (layer->vectorType() == QGis::Line)
	  {
	    double dist = calc.measure(f_it->geometry());
	    QString str = calc.textUnit(dist, 3, mCanvas->mapUnits(), false);
	    mResults->addDerivedAttribute(featureNode, QObject::tr("Length"), str);
	  }
	else if (layer->vectorType() == QGis::Polygon)
	  {
	    double area = calc.measure(f_it->geometry());
	    QString str = calc.textUnit(area, 3, mCanvas->mapUnits(), true);
	    mResults->addDerivedAttribute(featureNode, QObject::tr("Area"), str);
	  }
	
	// Add actions 
	QgsAttributeAction::aIter iter = actions.begin();
	for (register int i = 0; iter != actions.end(); ++iter, ++i)
	  {
	    mResults->addAction( featureNode, i, QObject::tr("action"), iter->name() );
	  }
	
      }
    
    QgsDebugMsg("Feature count on identify: " + QString::number(featureCount));
    
    //also test the not commited features //todo: eliminate copy past code
    
    mResults->setTitle(layer->name() + " - " + QString::number(featureCount) + QObject::tr(" features found"));
    if (featureCount == 1) 
      {
	mResults->showAllAttributes();
	mResults->setTitle(layer->name() + " - " + QObject::tr(" 1 feature found") );
	highlightFeature(lastFeatureId);
      }
    else if (featureCount == 0)
      {
	mResults->setTitle(layer->name() + " - " + QObject::tr("No features found") );
	mResults->setMessage ( QObject::tr("No features found"), QObject::tr("No features were found in the active layer at the point you clicked") );
      }
    else
      {
	QString title = layer->name();
	title += QString( tr("- %1 features found","Identify results window title",featureCount) ).arg(featureCount);
	mResults->setTitle(title);    
      }
    QApplication::restoreOverrideCursor();
    
    mResults->show();
}

#if 0 //MH: old state of the function
void QgsMapToolIdentify::identifyVectorLayer(QgsVectorLayer* layer, const QgsPoint& point)
{
  if (!layer)
    return;
  
  // load identify radius from settings
  QSettings settings;
  double identifyValue = settings.value("/Map/identifyRadius", QGis::DEFAULT_IDENTIFY_RADIUS).toDouble();
  QString ellipsoid = settings.readEntry("/qgis/measure/ellipsoid", "WGS84");

  // create the search rectangle
  double searchRadius = mCanvas->extent().width() * (identifyValue/100.0);
    
  QgsRect r;
  r.setXmin(point.x() - searchRadius);
  r.setXmax(point.x() + searchRadius);
  r.setYmin(point.y() - searchRadius);
  r.setYmax(point.y() + searchRadius);
  
  r = toLayerCoords(layer, r);

  int featureCount = 0;
  QgsFeature feat;
  QgsAttributeAction& actions = *layer->actions();
  QString fieldIndex = layer->displayField();
  QgsVectorDataProvider* dataProvider = layer->getDataProvider();
  QgsAttributeList allAttributes = dataProvider->allAttributesList();
  const QgsFieldMap& fields = dataProvider->fields();
  
  dataProvider->select(allAttributes, r, true, true);

  // init distance/area calculator
  QgsDistanceArea calc;
  calc.setProjectionsEnabled(mCanvas->projectionsEnabled()); // project?
  calc.setEllipsoid(ellipsoid);
  calc.setSourceSRS(layer->srs().srsid());
  
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
      connect(mResults, SIGNAL(selectedFeatureChanged(int)), this, SLOT(highlightFeature(int)));
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
    
    int lastFeatureId = 0;

    QTreeWidgetItem *click = mResults->addNode(tr("(clicked coordinate)"));
    click->setText(1, point.stringRep());

    while (dataProvider->getNextFeature(feat))
    {
      featureCount++;

      QTreeWidgetItem* featureNode = mResults->addNode("foo");
      featureNode->setData(0, Qt::UserRole, QVariant(feat.featureId())); // save feature id
      lastFeatureId = feat.featureId();
      featureNode->setText(0, fieldIndex);
      const QgsAttributeMap& attr = feat.attributeMap();
      
      for (QgsAttributeMap::const_iterator it = attr.begin(); it != attr.end(); ++it)
      {
        //QgsDebugMsg(it->fieldName() + " == " + fieldIndex);
        
        if (fields[it.key()].name() == fieldIndex)
        {
          featureNode->setText(1, it->toString());
        }
        mResults->addAttribute(featureNode, fields[it.key()].name(), it->toString());
      }

      // Calculate derived attributes and insert:
      // measure distance or area depending on geometry type
      if (layer->vectorType() == QGis::Line)
      {
        double dist = calc.measure(feat.geometry());
        QString str = calc.textUnit(dist, 3, mCanvas->mapUnits(), false);
        mResults->addDerivedAttribute(featureNode, QObject::tr("Length"), str);
      }
      else if (layer->vectorType() == QGis::Polygon)
      {
        double area = calc.measure(feat.geometry());
        QString str = calc.textUnit(area, 3, mCanvas->mapUnits(), true);
        mResults->addDerivedAttribute(featureNode, QObject::tr("Area"), str);
      }

      // Add actions 
      QgsAttributeAction::aIter iter = actions.begin();
      for (register int i = 0; iter != actions.end(); ++iter, ++i)
      {
        mResults->addAction( featureNode, i, QObject::tr("action"), iter->name() );
      }

    }

    QgsDebugMsg("Feature count on identify: " + QString::number(featureCount));

    //also test the not commited features //todo: eliminate copy past code

    mResults->setTitle(layer->name() + " - " + QString::number(featureCount) + QObject::tr(" features found"));
    if (featureCount == 1) 
    {
      mResults->showAllAttributes();
      mResults->setTitle(layer->name() + " - " + QObject::tr(" 1 feature found") );
      highlightFeature(lastFeatureId);
    }
    else if (featureCount == 0)
    {
      mResults->setTitle(layer->name() + " - " + QObject::tr("No features found") );
      mResults->setMessage ( QObject::tr("No features found"), QObject::tr("No features were found in the active layer at the point you clicked") );
    }
    else
    {
      QString title = layer->name();
      title += QString( tr("- %1 features found","Identify results window title",featureCount) ).arg(featureCount);
      mResults->setTitle(title);    
    }
    QApplication::restoreOverrideCursor();

    mResults->show();
  } 
  else // ( layer->isEditable() )
  {
    // Edit attributes 
    // TODO: what to do if more features were selected? - nearest?
    QgsChangedAttributesMap& changedAttributes = layer->changedAttributes();
    
    QApplication::setOverrideCursor(Qt::WaitCursor);

    if (dataProvider->getNextFeature(feat))
    {
      // these are the values to populate the dialog with
      // start off with list of committed attribute values
      QgsAttributeMap old = feat.attributeMap();

      // Test if this feature already changed since the last commit

      QgsChangedAttributesMap::iterator it = changedAttributes.find(feat.featureId());
      if ( it != changedAttributes.end() )
      {
        // Yes, this feature already has in-memory attribute changes

        // go through and apply the modified-but-not-committed values
        QgsAttributeMap oldattr = *it;
        int index=0;
        for (QgsAttributeMap::const_iterator oldit = old.begin(); oldit != old.end(); ++oldit)
        {
          QgsAttributeMap::iterator ait = oldattr.find( oldit.key() );
          if ( ait != oldattr.end() )
          {
            // replace the committed value with the
            // modified-but-not-committed value
            old[index] = *ait;
          }

          ++index;
        }
      }

      QApplication::restoreOverrideCursor();

      // Show the attribute value editing dialog
      QgsAttributeDialog ad( dataProvider->fields(), old );

      if (ad.exec() == QDialog::Accepted)
      {

        int i = 0;
        for (QgsAttributeMap::const_iterator oldit = old.begin(); oldit != old.end(); ++oldit, ++i)
        {
          // only apply changed values if they were edited by the user
          if (ad.isDirty(i))
          {
            QgsDebugMsg("found a changed attribute: " + QString::number(i) + " = " + ad.value(i));

            QgsAttributeMap& chattr = changedAttributes[ feat.featureId() ];
            chattr[i] = ad.value(i);

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
}
#endif 


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
  mv->exec(); // deletes itself on close
}

void QgsMapToolIdentify::resultsDialogGone()
{
  mResults = 0;
  
  delete mRubberBand;
  mRubberBand = 0;
}

void QgsMapToolIdentify::deactivate()
{
  if (mResults)
    mResults->done(0); // close the window
  QgsMapTool::deactivate();
}

void QgsMapToolIdentify::highlightFeature(int featureId)
{
  QgsVectorLayer* layer = dynamic_cast<QgsVectorLayer*>(mCanvas->currentLayer());
  if (!layer)
    return;
  
  delete mRubberBand;
  mRubberBand = 0;

  QgsFeature feat;
  if(layer->getFeatureAtId(featureId, feat, true, false) != 0)
    {
      return;
    }

  if(!feat.geometry())
    {
      return;
    }
      
  mRubberBand = new QgsRubberBand(mCanvas, feat.geometry()->vectorType() == QGis::Polygon);
  
  if (mRubberBand)
  {
    mRubberBand->setToGeometry(feat.geometry(), *layer);
    mRubberBand->setWidth(2);
    mRubberBand->setColor(Qt::red);
    mRubberBand->show();
  }
}
