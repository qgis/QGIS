/***************************************************************************
                              qgscomposition.cpp
                             -------------------
    begin                : January 2005
    copyright            : (C) 2005 by Radim Blazek
    email                : blazek@itc.it
 ***************************************************************************/
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgscomposition.h"
#include "qgscomposeritem.h"
#include "qgscomposermap.h"
#include <QDomDocument>
#include <QDomElement>
#include <QGraphicsRectItem>

QgsComposition::QgsComposition(QgsMapRenderer* mapRenderer): QGraphicsScene(0), mMapRenderer(mapRenderer), mPlotStyle(QgsComposition::Preview), mPaperItem(0)
{
  setBackgroundBrush(Qt::gray);

  //set paper item
  mPaperItem = new QGraphicsRectItem( 0, 0, 297, 210, 0); //default size A4
  mPaperItem->setBrush(Qt::white);
  addItem(mPaperItem);
  mPaperItem->setZValue(0);
  mPrintoutResolution = 300; //hardcoded default
}

QgsComposition::QgsComposition(): QGraphicsScene(0), mMapRenderer(0), mPlotStyle(QgsComposition::Preview), mPaperItem(0)
{

}

QgsComposition::~QgsComposition()
{
  delete mPaperItem;
}

void QgsComposition::setPaperSize(double width, double height)
{
  if(mPaperItem)
    {
      mPaperItem->setRect(QRectF(0, 0, width, height));
    }
}

double QgsComposition::paperHeight() const
{
  return mPaperItem->rect().height();
}

double QgsComposition::paperWidth() const
{
  return mPaperItem->rect().width();
}

QgsComposerItem* QgsComposition::composerItemAt(const QPointF & position)
{
  QList<QGraphicsItem *> itemList = items(position);
  QList<QGraphicsItem *>::iterator itemIt = itemList.begin();

  for(; itemIt != itemList.end(); ++itemIt)
    {
      QgsComposerItem* composerItem = dynamic_cast<QgsComposerItem*>(*itemIt);
      if(composerItem)
	{
	  return composerItem;
	}
    }
  return 0;
}

QList<QgsComposerItem*> QgsComposition::selectedComposerItems()
{
  QList<QgsComposerItem*> composerItemList;

  QList<QGraphicsItem *> graphicsItemList = selectedItems();
  QList<QGraphicsItem *>::iterator itemIter = graphicsItemList.begin();
  
  for(; itemIter != graphicsItemList.end(); ++itemIter)
    {
      QgsComposerItem* composerItem = dynamic_cast<QgsComposerItem*>(*itemIter);
      if(composerItem)
	{
	  composerItemList.push_back(composerItem);
	}
    }

  return composerItemList;
}

QList<const QgsComposerMap*> QgsComposition::composerMapItems() const
{
  QList<const QgsComposerMap*> resultList;

  QList<QGraphicsItem *> itemList = items();
  QList<QGraphicsItem *>::iterator itemIt = itemList.begin();
  for(; itemIt != itemList.end(); ++itemIt)
    {
      const QgsComposerMap* composerMap = dynamic_cast<const QgsComposerMap*>(*itemIt);
      if(composerMap)
	{
	  resultList.push_back(composerMap);
	}
    }

  return resultList;
}

const QgsComposerMap* QgsComposition::getComposerMapById(int id) const
{
  QList<const QgsComposerMap*> resultList;

  QList<QGraphicsItem *> itemList = items();
  QList<QGraphicsItem *>::iterator itemIt = itemList.begin();
  for(; itemIt != itemList.end(); ++itemIt)
    {
      const QgsComposerMap* composerMap = dynamic_cast<const QgsComposerMap*>(*itemIt);
      if(composerMap)
	{
	  if(composerMap->id() == id)
	    {
	      return composerMap;
	    }
	}
    }

  return 0;
}

int QgsComposition::pixelFontSize(double pointSize) const
{
  //in QgsComposition, one unit = one mm
  double sizeMM = pointSize * 0.3527;
  return sizeMM;
}

double QgsComposition::pointFontSize(int pixelSize) const
{
  double sizePoint = pixelSize / 0.3527;
  return sizePoint;
}

bool QgsComposition::writeXML(QDomElement& composerElem, QDomDocument& doc)
{
  if(composerElem.isNull())
    {
      return false;
    }

  QDomElement compositionElem = doc.createElement("Composition");
  if(mPaperItem)
    {
      compositionElem.setAttribute("paperWidth", mPaperItem->rect().width());
      compositionElem.setAttribute("paperHeight", mPaperItem->rect().height());
    }
  composerElem.appendChild(compositionElem);

  return true;
}

bool QgsComposition::readXML(const QDomElement& compositionElem, const QDomDocument& doc)
{
  if(compositionElem.isNull())
    {
      return false;
    }

  //create paper item
  bool widthConversionOk, heightConversionOk;
  double paperWidth = compositionElem.attribute("paperWidth").toDouble(&widthConversionOk);
  double paperHeight = compositionElem.attribute("paperHeight").toDouble(&heightConversionOk);

  if(widthConversionOk && heightConversionOk)
    {
      delete mPaperItem;
      mPaperItem = new QGraphicsRectItem( 0, 0, paperWidth, paperHeight, 0);
      mPaperItem->setBrush(Qt::white);
      addItem(mPaperItem);
      mPaperItem->setZValue(0);
    }

  return true;
}
