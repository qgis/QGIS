/***************************************************************************
                          qgscontcoldialog.cpp 
 Continuous color renderer dialog
                             -------------------
    begin                : 2004-02-11
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

#include "qgscontcoldialog.h"

#include <cfloat>
#include <iostream>

#include <qcolordialog.h>
#include <qcombobox.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qpushbutton.h>
#include <qspinbox.h>
#include <qtoolbutton.h>
 
#include "qgscontinuouscolrenderer.h"
#include "qgsdlgvectorlayerproperties.h"
#include "qgsfield.h"
#include "qgslegenditem.h"
#include "qgsvectordataprovider.h"
#include "qgsvectorlayer.h"



QgsContColDialog::QgsContColDialog(QgsVectorLayer * layer)
    : QgsContColDialogBase(), mVectorLayer(layer)
{
#ifdef QGISDEBUG
    qWarning("constructor QgsContColDialog");
#endif

    QObject::connect(btnMinValue, SIGNAL(clicked()), this, SLOT(selectMinimumColor()));
    QObject::connect(btnMaxValue, SIGNAL(clicked()), this, SLOT(selectMaximumColor()));

    //find out the numerical fields of mVectorLayer
    QgsVectorDataProvider *provider;
    if (provider = dynamic_cast<QgsVectorDataProvider*>(mVectorLayer->getDataProvider()))
    {
	std::vector < QgsField > const & fields = provider->fields();
	int fieldnumber = 0;
	QString str;

	for (std::vector < QgsField >::const_iterator it = fields.begin(); it != fields.end(); ++it)
        {
	    QString type = (*it).type();
	    if (type != "String" && type != "varchar" && type != "geometry")
            {
		str = (*it).name();
		str = str.left(1).upper() + str.right(str.length() - 1);  //make the first letter uppercase
		classificationComboBox->insertItem(str);
		mFieldMap.insert(std::make_pair(str, fieldnumber));
            }
	    fieldnumber++;
        }
    } 
    else
    {
	qWarning("Warning, data provider is null in QgsContColDialog::QgsContColDialog(...)");
	return;
    }
    
    //restore the correct colors for minimum and maximum values
    
    const QgsContinuousColRenderer* renderer = dynamic_cast < const QgsContinuousColRenderer * >(layer->renderer());;
    
    if (renderer)
    {
	classificationComboBox->setCurrentItem(renderer->classificationField());
	const QgsSymbol* minsymbol = renderer->minimumSymbol();
	const QgsSymbol* maxsymbol = renderer->maximumSymbol();
	if (mVectorLayer->vectorType() == QGis::Line || mVectorLayer->vectorType() == QGis::Point)
        {
	    lblMinValue->setPaletteBackgroundColor(minsymbol->pen().color());
	    lblMaxValue->setPaletteBackgroundColor(maxsymbol->pen().color());
	} 
	else
        {
	    lblMinValue->setPaletteBackgroundColor(minsymbol->brush().color());
	    lblMaxValue->setPaletteBackgroundColor(maxsymbol->brush().color());
        }
	outlinewidthspinbox->setValue(minsymbol->pen().width());
	outlinewidthspinbox->setMinValue(1);
    }
}

QgsContColDialog::QgsContColDialog()
{
#ifdef QGISDEBUG
    qWarning("constructor QgsContColDialog");
#endif
}

QgsContColDialog::~QgsContColDialog()
{
#ifdef QGISDEBUG
    qWarning("destructor QgsContColDialog");
#endif
}

void QgsContColDialog::apply()
{
    QString fieldstring = classificationComboBox->currentText();
    if (fieldstring.isEmpty())    //don't do anything, it there is no classification field
    {
	return;
    }
    std::map < QString, int >::iterator iter = mFieldMap.find(fieldstring);
    int classfield = iter->second;
    
    //find the minimum and maximum for the classification variable
    double minimum, maximum;
    QgsVectorDataProvider *provider = dynamic_cast<QgsVectorDataProvider*>(mVectorLayer->getDataProvider());
    if (provider)
    {
	minimum = provider->minValue(classfield).toDouble();
	maximum = provider->maxValue(classfield).toDouble();
    } 
    else
    {
	qWarning("Warning, provider is null in QgsGraSyExtensionWidget::QgsGraSyExtensionWidget(...)");
	return;
    }


    //create the render items for minimum and maximum value
    QgsSymbol* minsymbol = new QgsSymbol(mVectorLayer->vectorType(), QString::number(minimum, 'f'), "", "");
    if (mVectorLayer->vectorType() == QGis::Line || mVectorLayer->vectorType() == QGis::Point)
    {
	minsymbol->setPen(QPen(lblMinValue->paletteBackgroundColor(),outlinewidthspinbox->value()));
    } 
    else
    {
	minsymbol->setBrush(QBrush(lblMinValue->paletteBackgroundColor()));
	minsymbol->setPen(QPen(QColor(0, 0, 0), outlinewidthspinbox->value()));
    }
    
    QgsSymbol* maxsymbol = new QgsSymbol(mVectorLayer->vectorType(), QString::number(maximum, 'f'), "", "");
    if (mVectorLayer->vectorType() == QGis::Line || mVectorLayer->vectorType() == QGis::Point)
    {
	maxsymbol->setPen(QPen(lblMaxValue->paletteBackgroundColor(),outlinewidthspinbox->value()));
    } 
    else
    {
	maxsymbol->setBrush(QBrush(lblMaxValue->paletteBackgroundColor()));
	maxsymbol->setPen(QPen(QColor(0, 0, 0), outlinewidthspinbox->value()));
    }
    
    //set the render items to the buffer renderer of the property dialog (if there is one)
    QgsContinuousColRenderer *renderer = dynamic_cast < QgsContinuousColRenderer * >(mVectorLayer->propertiesDialog()->getBufferRenderer());
    
    if(!renderer)
    {
	renderer = new QgsContinuousColRenderer(mVectorLayer->vectorType());
	mVectorLayer->setRenderer(renderer);
    }
    
    renderer->setMinimumSymbol(minsymbol);
    renderer->setMaximumSymbol(maxsymbol);
    renderer->setClassificationField(classfield);

    mVectorLayer->refreshLegend();
}

void QgsContColDialog::selectMinimumColor()
{
    QColor mincolor = QColorDialog::getColor(QColor(Qt::black), this);
    if(mincolor.isValid())
    {
	lblMinValue->setPaletteBackgroundColor(mincolor);
    }
    setActiveWindow();
}

void QgsContColDialog::selectMaximumColor()
{
    QColor maxcolor = QColorDialog::getColor(QColor(Qt::black), this);
    if(maxcolor.isValid())
    {
	lblMaxValue->setPaletteBackgroundColor(maxcolor);
    }
    setActiveWindow();
}
