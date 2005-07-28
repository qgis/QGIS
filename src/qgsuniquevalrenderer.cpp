/***************************************************************************
                         qgsuniquevalrenderer.cpp  -  description
                             -------------------
    begin                : July 2004
    copyright            : (C) 2004 by Marco Hugentobler
    email                : marco.hugentobler@autoform.ch
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
#include "qgsdlgvectorlayerproperties.h"
#include "qgsuniquevalrenderer.h"
#include "qgsuvaldialog.h"
#include "qgsfeatureattribute.h"
#include "qgsfeature.h"
#include "qgsvectorlayer.h"
#include "qgssymbol.h"
#include "qgssymbologyutils.h"
#include "qgsuvaldialog.h"
#include "qgssvgcache.h"
#include <qdom.h>
#include <qpainter.h>
#include <qpixmap.h>
#include <qpicture.h>
#include <vector>

QgsUniqueValRenderer::QgsUniqueValRenderer(QGis::VectorType type): mClassificationField(0)
{
    mVectorType = type;

//call superclass method to set up selection colour
    initialiseSelectionColor();

}

QgsUniqueValRenderer::QgsUniqueValRenderer(const QgsUniqueValRenderer& other)
{
    mVectorType = other.mVectorType;
    mClassificationField = other.mClassificationField;
    for(std::map<QString, QgsSymbol*>::iterator it=mSymbols.begin(); it!=mSymbols.end(); ++it)
    {
	QgsSymbol* s = new QgsSymbol(*(it->second));
	insertValue(it->first, s);
    }
}

QgsUniqueValRenderer& QgsUniqueValRenderer::operator=(const QgsUniqueValRenderer& other)
{
    if(this != &other)
    {
	mVectorType = other.mVectorType;
	mClassificationField = other.mClassificationField;
	clearValues();
	for(std::map<QString, QgsSymbol*>::iterator it=mSymbols.begin(); it!=mSymbols.end(); ++it)
	{
	    QgsSymbol* s = new QgsSymbol(*(it->second));
	    insertValue(it->first, s);
	}
    }
}

QgsUniqueValRenderer::~QgsUniqueValRenderer()
{
    for(std::map<QString,QgsSymbol*>::iterator it=mSymbols.begin();it!=mSymbols.end();++it)
    {
	delete it->second;
    }
}

const std::list<QgsSymbol*> QgsUniqueValRenderer::symbols() const
{
    std::list <QgsSymbol*> symbollist;
    for(std::map<QString, QgsSymbol*>::const_iterator it = mSymbols.begin(); it!=mSymbols.end(); ++it)
    {
	symbollist.push_back(it->second);
    }
    return symbollist;
}

void QgsUniqueValRenderer::insertValue(QString name, QgsSymbol* symbol)
{
    mSymbols.insert(std::make_pair(name, symbol));
}

void QgsUniqueValRenderer::setClassificationField(int field)
{
    mClassificationField=field;
}

int QgsUniqueValRenderer::classificationField()
{
    return mClassificationField;
}
    
void QgsUniqueValRenderer::renderFeature(QPainter* p, QgsFeature* f,QPicture* pic, 
	double* scalefactor, bool selected, int oversampling, double widthScale)
{
    std::vector < QgsFeatureAttribute > vec = f->attributeMap();
    QString value = vec[0].fieldValue();
    std::map<QString,QgsSymbol*>::iterator it=mSymbols.find(value);
    if(it!=mSymbols.end())
    {
	QgsSymbol* symbol = it->second;

	// Point 
	if ( pic && mVectorType == QGis::Point ) {
	    *pic = symbol->getPointSymbolAsPicture( oversampling, widthScale,
		                                             selected, mSelectionColor );
	    
	    if ( scalefactor ) *scalefactor = 1;
	} 

        // Line, polygon
 	if ( mVectorType != QGis::Point )
	{
	    if( !selected ) 
	    {
		QPen pen=symbol->pen();
		pen.setWidth ( (int) (widthScale * pen.width()) );
		p->setPen(pen);
		p->setBrush(symbol->brush());
	    }
	    else
	    {
		QPen pen=symbol->pen();
		pen.setWidth ( (int) (widthScale * pen.width()) );
		pen.setColor(mSelectionColor);
		QBrush brush=symbol->brush();
		brush.setColor(mSelectionColor);
		p->setPen(pen);
		p->setBrush(brush);
	    }
	}
    }
    else
    {
#ifdef QGISDEBUG
	qWarning("Warning, no render item found in QgsUniqueValRenderer::renderFeature");
#endif
    }
    
}

void QgsUniqueValRenderer::readXML(const QDomNode& rnode, QgsVectorLayer& vl)
{
    mVectorType = vl.vectorType();
    QDomNode classnode = rnode.namedItem("classificationfield");
    int classificationfield = classnode.toElement().text().toInt();
    this->setClassificationField(classificationfield);

    QDomNode symbolnode = rnode.namedItem("symbol");
    while (!symbolnode.isNull())
    {
	QgsSymbol* msy = new QgsSymbol();
	msy->readXML ( symbolnode );
	this->insertValue(msy->lowerValue(),msy);
	symbolnode = symbolnode.nextSibling();
    }

    vl.setRenderer(this);
}

void QgsUniqueValRenderer::clearValues()
{
    for(std::map<QString,QgsSymbol*>::iterator it=mSymbols.begin();it!=mSymbols.end();++it)
    {
	delete it->second;
    }
    mSymbols.clear();
}

QString QgsUniqueValRenderer::name() const
{
    return "Unique Value";
}

std::list<int> QgsUniqueValRenderer::classificationAttributes() const
{
    std::list<int> list;
    list.push_back(mClassificationField);
    return list;
}

bool QgsUniqueValRenderer::writeXML( QDomNode & layer_node, QDomDocument & document ) const
{
    bool returnval=true;
    QDomElement uniquevalue=document.createElement("uniquevalue");
    layer_node.appendChild(uniquevalue);
    QDomElement classificationfield=document.createElement("classificationfield");
    QDomText classificationfieldtxt=document.createTextNode(QString::number(mClassificationField));
    classificationfield.appendChild(classificationfieldtxt);
    uniquevalue.appendChild(classificationfield);
    for(std::map<QString,QgsSymbol*>::const_iterator it=mSymbols.begin();it!=mSymbols.end();++it)
    {
	if(!(it->second)->writeXML(uniquevalue,document))
	{
	    returnval=false;  
	}
    }
    return returnval;
}

QgsRenderer* QgsUniqueValRenderer::clone() const
{
    QgsUniqueValRenderer* r = new QgsUniqueValRenderer(*this);
    return r;
}
