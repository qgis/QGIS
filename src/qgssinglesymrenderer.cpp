/***************************************************************************
                         qgssinglesymrenderer.cpp  -  description
                             -------------------
    begin                : Oct 2003
    copyright            : (C) 2003 by Marco Hugentobler
    email                : mhugent@geo.unizh.ch
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
#include "qgis.h"
#include "qgssinglesymrenderer.h"
#include "qgsfeature.h"
#include "qgis.h"
#include "qgsvectorlayer.h"
#include "qstring.h"
#include "qgssisydialog.h"
#include "qgslegenditem.h"
#include "qgssymbologyutils.h"
#include "qgssvgcache.h"
#include <qdom.h>
#include <qpainter.h>
#include <qpicture.h>
#include <qpixmap.h>

QgsSingleSymRenderer::QgsSingleSymRenderer(): mItem(new QgsRenderItem())
{
  //call superclass method to set up selection colour
  initialiseSelectionColor();

}

QgsSingleSymRenderer::~QgsSingleSymRenderer()
{
  delete mItem;
}

void QgsSingleSymRenderer::addItem(QgsRenderItem* ri)
{
  delete mItem;
  mItem = ri;
}

void QgsSingleSymRenderer::renderFeature(QPainter * p, QgsFeature * f, QPicture* pic, 
	         double* scalefactor, bool selected, int oversampling, double widthScale)
{
	// Point 
	if ( pic && mVectorType == QGis::Point) {
	    *pic = mItem->getSymbol()->getPointSymbolAsPicture( oversampling, widthScale, 
					 selected, mSelectionColor );
	    
	    if ( scalefactor ) *scalefactor = 1;
	} 

        // Line, polygon
 	if ( mVectorType != QGis::Point )
	{
	    if( !selected ) 
	    {
		QPen pen=mItem->getSymbol()->pen();
		pen.setWidth ( (int) (widthScale * pen.width()) );
		p->setPen(pen);
		p->setBrush(mItem->getSymbol()->brush());
	    }
	    else
	    {
		QPen pen=mItem->getSymbol()->pen();
		pen.setWidth ( (int) (widthScale * pen.width()) );
		pen.setColor(mSelectionColor);
		QBrush brush=mItem->getSymbol()->brush();
		brush.setColor(mSelectionColor);
		p->setPen(pen);
		p->setBrush(brush);
	    }
	}
}

void QgsSingleSymRenderer::initializeSymbology(QgsVectorLayer * layer, QgsDlgVectorLayerProperties * pr)
{
  bool toproperties = false;    //if false: rendererDialog is associated with the vector layer and image is rendered, true: rendererDialog is associated with buffer dialog of vector layer properties and no image is rendered
  if (pr)
  {
    toproperties = true;
  }

  if (layer)
  {
    mVectorType = layer->vectorType();
    QgsSymbol* sy = new QgsSymbol();
    sy->brush().setStyle(Qt::SolidPattern);
    sy->pen().setStyle(Qt::SolidLine);
    sy->pen().setWidth(1);//set width 1 as default instead of width 0

    //random fill colors for points and polygons and pen colors for lines
    int red = 1 + (int) (255.0 * rand() / (RAND_MAX + 1.0));
    int green = 1 + (int) (255.0 * rand() / (RAND_MAX + 1.0));
    int blue = 1 + (int) (255.0 * rand() / (RAND_MAX + 1.0));

    //font tor the legend text
    QFont f("arial", 10, QFont::Normal);
    QFontMetrics fm(f);

    QPixmap *pixmap;
    if (toproperties)
    {
      pixmap = pr->getBufferPixmap();
    } 
    else
    {
      pixmap = layer->legendPixmap();
    }

    QString name = layer->name();
    int width = 40 + fm.width(layer->name());
    int height = (fm.height() + 10 > 35) ? fm.height() + 10 : 35;

    pixmap->resize(width, height);
    pixmap->fill();
    QPainter p(pixmap);
    p.setPen(sy->pen());

    if (layer->vectorType() == QGis::Line)
    {
      sy->pen().setColor(QColor(red, green, blue));
      //paint the pixmap for the legend
      p.setPen(sy->pen());
      p.drawLine(10, pixmap->height() - 25, 25, pixmap->height() - 10);
    } 
    else
    {
      sy->brush().setColor(QColor(red, green, blue));
      sy->pen().setColor(QColor(0, 0, 0));
      //paint the pixmap for the legend
      p.setPen(sy->pen());
      p.setBrush(sy->brush());
      if (layer->vectorType() == QGis::Point)
      {
        //p.drawRect(20, pixmap->height() - 17, 5, 5);
        QPixmap pm = sy->getPointSymbolAsPixmap();
        p.drawPixmap ( (int) (17-pm.width()/2), (int) ((pixmap->height()-pm.height())/2), pm );
      } 
      else                //polygon
      {
        p.drawRect(10, pixmap->height() - 25, 20, 15);
      }
    }

    p.setPen(Qt::black);
    p.setFont(f);
    p.drawText(35, pixmap->height() - 10, name);
    QgsRenderItem* ri = new QgsRenderItem(sy, "", "");
    addItem(ri);

    QgsSiSyDialog *dialog = new QgsSiSyDialog(layer);
    if (toproperties)
    {
        mVectorType = layer->vectorType();
	QgsSymbol* sy = new QgsSymbol();
	sy->brush().setStyle(Qt::SolidPattern);
	sy->pen().setStyle(Qt::SolidLine);
	sy->pen().setWidth(1);//set width 1 as default instead of width 0

	//random fill colors for points and polygons and pen colors for lines
	int red = 1 + (int) (255.0 * rand() / (RAND_MAX + 1.0));
	int green = 1 + (int) (255.0 * rand() / (RAND_MAX + 1.0));
	int blue = 1 + (int) (255.0 * rand() / (RAND_MAX + 1.0));

	//font tor the legend text
	QFont f("arial", 10, QFont::Normal);
	QFontMetrics fm(f);

	QPixmap *pixmap;
	if (toproperties)
        {
	    pixmap = pr->getBufferPixmap();
	} 
	else
        {
	    pixmap = layer->legendPixmap();
        }

	QString name = layer->name();
	int width = 40 + fm.width(layer->name());
	int height = (fm.height() + 10 > 35) ? fm.height() + 10 : 35;

	pixmap->resize(width, height);
	pixmap->fill();
	QPainter p(pixmap);
	p.setPen(sy->pen());
	
	if (layer->vectorType() == QGis::Line)
        {
	    sy->pen().setColor(QColor(red, green, blue));
	    //paint the pixmap for the legend
	    p.setPen(sy->pen());
	    p.drawLine(10, pixmap->height() - 25, 25, pixmap->height() - 10);
	} 
	else
        {
	    sy->brush().setColor(QColor(red, green, blue));
	    sy->pen().setColor(QColor(0, 0, 0));
	    //paint the pixmap for the legend
	    p.setPen(sy->pen());
	    p.setBrush(sy->brush());
	    if (layer->vectorType() == QGis::Point)
            {
		//p.drawRect(20, pixmap->height() - 17, 5, 5);
		QPixmap pm = sy->getPointSymbolAsPixmap();
		p.drawPixmap ( (int) (17-pm.width()/2), (int) ((pixmap->height()-pm.height())/2), pm );
	    } 
	    else                //polygon
            {
		p.drawRect(10, pixmap->height() - 25, 20, 15);
            }
	}

	p.setPen(Qt::black);
	p.setFont(f);
	p.drawText(35, pixmap->height() - 10, name);
	QgsRenderItem* ri = new QgsRenderItem(sy, "", "");
	addItem(ri);

	QgsSiSyDialog *dialog = new QgsSiSyDialog(layer);
	if (toproperties)
        {
	    pr->setBufferDialog(dialog);
	} 
	else
        {
	    layer->setRendererDialog(dialog);
	    QgsLegendItem *item;
	    layer->updateItemPixmap();
        }
    } 
    else
    {
      layer->setRendererDialog(dialog);
      QgsLegendItem *item;
      layer->updateItemPixmap();
    }
  } 
  else
  {
    qWarning("Warning, null pointer in QgsSingleSymRenderer::initializeSymbology()");
  }
}

void QgsSingleSymRenderer::readXML(const QDomNode& rnode, QgsVectorLayer& vl)
{
    mVectorType = vl.vectorType();
    QgsSymbol* sy = new QgsSymbol();

  QDomNode rinode = rnode.namedItem("renderitem");

    Q_ASSERT( ! rinode.isNull() );

    QDomNode vnode = rinode.namedItem("value");

    Q_ASSERT( ! rinode.isNull() );

    QDomElement velement = vnode.toElement();
    QString value = velement.text();

    QDomNode synode = rinode.namedItem("symbol");
    
    if ( synode.isNull() )
    {
        qDebug( "%s:%d in project file no symbol node in renderitem DOM" );
        // XXX abort?
    }
    else
    {
        sy->readXML ( synode );
    }

    QDomNode lnode = rinode.namedItem("label");

    Q_ASSERT( ! rinode.isNull() );

    QDomElement lnodee = lnode.toElement();
    QString label = lnodee.text();

    //create a renderer and add it to the vector layer
    QgsRenderItem* ri = new QgsRenderItem(sy, value, label);
    this->addItem(ri);
    vl.setRenderer(this);
    QgsSiSyDialog *sdialog = new QgsSiSyDialog(&vl);
    vl.setRendererDialog(sdialog);

    QgsDlgVectorLayerProperties *properties = new QgsDlgVectorLayerProperties(&vl);
    vl.setLayerProperties(properties);
    properties->setLegendType("Single Symbol");

    sdialog->apply();
}

bool QgsSingleSymRenderer::writeXML( QDomNode & layer_node, QDomDocument & document )
{
  bool returnval=false;
  QDomElement singlesymbol=document.createElement("singlesymbol");
  layer_node.appendChild(singlesymbol);
  if(mItem)
  {
    returnval=mItem->writeXML(singlesymbol,document);
  }
  return returnval;
}


std::list<int> QgsSingleSymRenderer::classificationAttributes()
{
  std::list<int> list;
  return list;//return an empty list
}

QString QgsSingleSymRenderer::name()
{
  return "Single Symbol";
}

const std::list<QgsRenderItem*> QgsSingleSymRenderer::items() const
{
  std::list<QgsRenderItem*> list;
  list.push_back(mItem);
  return list;
}

