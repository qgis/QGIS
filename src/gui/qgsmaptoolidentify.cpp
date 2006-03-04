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
  : QgsMapTool(canvas), mResults(NULL)
{
  // set cursor
  QPixmap myIdentifyQPixmap = QPixmap((const char **) identify_cursor);
  mCursor = QCursor(myIdentifyQPixmap, 1, 1);
}
    
QgsMapToolIdentify::~QgsMapToolIdentify()
{
  delete mResults;
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
      std::cout << "identify: unknown layer type!" << std::endl;
#endif
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



void QgsMapToolIdentify::identifyVectorLayer(QgsVectorLayer* layer, const QgsPoint& point)
{
  if (!layer)
    return;
  
  // load identify radius from settings
  QSettings settings;
  int identifyValue = settings.readNumEntry("/qgis/map/identifyRadius", QGis::DEFAULT_IDENTIFY_RADIUS);

  // create the search rectangle
  double searchRadius = mCanvas->extent().width() * (identifyValue/1000.0);
    
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

      Q3ListViewItem *featureNode = mResults->addNode("foo");
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

      // measure distance or area
      if (layer->vectorType() == QGis::Line)
      {
        double dist = calc.measure(fet->geometry());
        QString str = QString::number(dist/1000, 'f', 3);
        str += " km";
        mResults->addAttribute(featureNode, ".Length", str);
      }
      else if (layer->vectorType() == QGis::Polygon)
      {
        double area = calc.measure(fet->geometry());
        QString str = QString::number(area/1000000, 'f', 3);
        str += " km2";
        mResults->addAttribute(featureNode, ".Area", str);
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
    if (featureCount == 0)
    {
      mResults->setTitle(layer->name() + " - " + QObject::tr("No features found") );
      mResults->setMessage ( QObject::tr("No features found"), QObject::tr("No features were found in the active layer at the point you clicked") );
    }
    
    QApplication::restoreOverrideCursor();
    
    mResults->show();
  } 
  else
  {
    // Edit attributes 
    // TODO: what to do if more features were selected? - nearest?
    changed_attr_map& changedAttributes = layer->changedAttributes();
    
    QApplication::setOverrideCursor(Qt::WaitCursor);

    if ( (fet = dataProvider->getNextFeature(true)) )
    {
      // Was already changed?
      changed_attr_map::iterator it = changedAttributes.find(fet->featureId());

      std::vector < QgsFeatureAttribute > old;
      if ( it != changedAttributes.end() )
      {
        std::map<QString,QString> oldattr = (*it).second;
        for( std::map<QString,QString>::iterator ait = oldattr.begin(); ait!=oldattr.end(); ++ait )
        {
          old.push_back ( QgsFeatureAttribute ( (*ait).first, (*ait).second ) );
        }
      }
      else
      {
        old = fet->attributeMap();
      }
      
      QApplication::restoreOverrideCursor();
      
      QgsAttributeDialog ad( &old );

      if ( ad.exec()==QDialog::Accepted )
      {
        std::map<QString,QString> attr;
        
        // Do this only once rather than each pass through the loop
        int oldSize = old.size();
        for(register int i= 0; i < oldSize; ++i)
        {
          attr.insert ( std::make_pair( old[i].fieldName(), ad.value(i) ) );
        }
        
        // Remove old if exists
        it = changedAttributes.find(fet->featureId());

        if ( it != changedAttributes.end() )
        { // found
          changedAttributes.erase ( it );
        }

        changedAttributes.insert ( std::make_pair( fet->featureId(), attr ) );
        layer->setModified();
      }
    }
    else
      QApplication::restoreOverrideCursor();
  }
  dataProvider->reset();
}
