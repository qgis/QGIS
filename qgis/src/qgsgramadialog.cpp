/***************************************************************************
                          qgsgramadialog.cpp  -  description
                             -------------------
    begin                : April 2004
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
/* $Id */

#include "qgsgramadialog.h"
#include "qgsgramaextensionwidget.h"
#include "qgsmarkersymbol.h"
#include "qgsvectorlayer.h"
#include "qgsdataprovider.h"
#include "qgsgraduatedmarenderer.h"
#include "qgsdlgvectorlayerproperties.h"
#include <qcombobox.h>
#include <qlayout.h>
#include <qpushbutton.h>
#include <qspinbox.h>

QgsGraMaDialog::QgsGraMaDialog(QgsVectorLayer* layer): QgsGraMaDialogBase(), ext(0), mVectorLayer(layer)
{
#ifdef QGISDEBUG
    qWarning("constructor QgsGraSyDialog");
#endif

    setOrientation(Qt::Vertical);
    setSizeGripEnabled(true);

    //find out the numerical fields of mVectorLayer
    QgsDataProvider *provider;
    if (provider = mVectorLayer->getDataProvider())
    {
	std::vector < QgsField > &fields = provider->fields();
	int fieldnumber = 0;
	QString str;
	
	for (std::vector < QgsField >::iterator it = fields.begin(); it != fields.end(); ++it)
        {
	    QString type = (*it).type();
	    if (type != "String" && type != "varchar" && type != "geometry")
            {
		str = (*it).name();
		str = str.left(1).upper() + str.right(str.length() - 1);  //make the first letter uppercase
		mClassificationComboBox->insertItem(str);
		mFieldMap.insert(std::make_pair(str, fieldnumber));
            }
	    fieldnumber++;
        }
    } 
    else
    {
	qWarning("Warning, data provider is null in QgsGraSyDialog::QgsGraSyDialog(...)");
	return;
    }

    mModeComboBox->insertItem("Empty");
    mModeComboBox->insertItem("Equal Interval");

    //restore the correct settings
    QgsGraduatedMaRenderer *renderer;
    
    //initial settings, use the buffer of the propertiesDialog if possible. If this is not possible, use the renderer of the vectorlayer directly
    if (mVectorLayer->propertiesDialog())
    {
	renderer = dynamic_cast < QgsGraduatedMaRenderer * >(layer->propertiesDialog()->getBufferRenderer());
    } 
    else
    {
	renderer = dynamic_cast < QgsGraduatedMaRenderer * >(layer->renderer());
    }

    if (renderer)
    {
	std::list < QgsRangeRenderItem * >list = renderer->items();
	ext = new QgsGraMaExtensionWidget(this, renderer->classificationField(), QgsGraSyDialog::EMPTY, list.size(), mVectorLayer);
    }

    //numberofclassesspinbox->setValue(list.size());
    QgsGraMaDialogBaseLayout->addMultiCellWidget(ext, 5, 5, 0, 3);
    ext->show();

    //do the necessary signal/slot connections
    QObject::connect(mNumberOfClassesSpinbox, SIGNAL(valueChanged(int)), this, SLOT(adjustNumberOfClasses()));
    QObject::connect(mClassificationComboBox, SIGNAL(activated(int)), this, SLOT(adjustNumberOfClasses()));
    QObject::connect(mModeComboBox, SIGNAL(activated(int)), this, SLOT(adjustNumberOfClasses()));
}

QgsGraMaDialog::QgsGraMaDialog()
{
#ifdef QGISDEBUG
    qWarning("constructor QgsGraSyDialog");
#endif
}
    
QgsGraMaDialog::~QgsGraMaDialog()
{
    if (ext)
    {
	ext->hide();
	delete ext;
    }
#ifdef QGISDEBUG
    qWarning("destructor QgsGraSyDialog");
#endif
}

void QgsGraMaDialog::apply()
{
    qWarning("entering QgsGraMaDialog::apply()");
    if (ext)
    {
	qWarning("1");
	if (mClassificationComboBox->currentText().isEmpty())  //don't do anything, it there is no classification field
        {
	    return;
        }
	
	QgsGraduatedMaRenderer *renderer = dynamic_cast < QgsGraduatedMaRenderer * >(mVectorLayer->renderer());
	
	if (!renderer)
        {
	    qWarning("Warning, typecast failed in QgsGraSyDialog::apply()");
	    return;
        }

	renderer->removeItems();
	qWarning("2");
	for (int i = 0; i < mNumberOfClassesSpinbox->value(); i++)
        {
	    QgsMarkerSymbol* sy = new QgsMarkerSymbol();
	    qWarning("2.1");
	    sy->setPicture(((QPushButton*)(ext->getWidget(3,i)))->name());
	    qWarning("2.2");
	    qWarning("SVG file: " + QString::fromAscii(((QPushButton*)(ext->getWidget(3,i)))->name()));
	    qWarning("2.3");
	    sy->setScaleFactor(((QLineEdit*)(ext->getWidget(4,i)))->text().toDouble());
	    qWarning("2.4");
	    QString lower_bound = ((QLineEdit *) (ext->getWidget(0, i)))->text();
	    QString upper_bound = ((QLineEdit *) (ext->getWidget(1, i)))->text();
	    QString label = ((QLineEdit *) (ext->getWidget(2, i)))->text();
	    
	    //test, if lower_bound is numeric or not (making a subclass of QString would be the proper solution)
	    bool lbcontainsletter = false;
	    for (uint j = 0; j < lower_bound.length(); j++)
            {
		if (lower_bound.ref(j).isLetter())
                {
		    lbcontainsletter = true;
                }
            }
	    
	    //test, if upper_bound is numeric or not (making a subclass of QString would be the proper solution)
	    bool ubcontainsletter = false;
	    for (uint j = 0; j < upper_bound.length(); j++)
            {
		if (upper_bound.ref(j).isLetter())
                {
		    ubcontainsletter = true;
                }
            }
	    qWarning("3");
	    if (lbcontainsletter == false && ubcontainsletter == false && lower_bound.length() > 0 && upper_bound.length() > 0) //only add the item if the value bounds do not contain letters and are not null strings
            {
		QgsRangeRenderItem *item = new QgsRangeRenderItem(sy, lower_bound, upper_bound, label);
		qWarning("lower_bound: " +lower_bound);
		qWarning("upper_bound: " +upper_bound);
		qWarning("label: " +label);
		renderer->addItem(item);
		//to do: add the symbol to the picture
	    }
	}
	qWarning("4");
	renderer->setClassificationField(ext->classfield());

	if (mVectorLayer->legendItem())
        {
	    //mVectorLayer->legendItem()->setPixmap(0, (*pix));
        }
	

	if (mVectorLayer->propertiesDialog())
        {
	    mVectorLayer->propertiesDialog()->setRendererDirty(false);
        }
	mVectorLayer->triggerRepaint();

    } 
    else                          //number of classes is 0
    {
	std::cout << "warning, number of classes is zero" << std::endl << std::flush;
    }
}

void QgsGraMaDialog::adjustNumberOfClasses()
{
    //find out the number of the classification field
    QString fieldstring = mClassificationComboBox->currentText();
    if (fieldstring.isEmpty())    //don't do anything, it there is no classification field
    {
	show();
	return;
    }
    
    std::map < QString, int >::iterator iter = mFieldMap.find(fieldstring);
    int field = iter->second;
    
    if(ext)
    {
	QgsGraMaDialogBaseLayout->remove(ext);
	delete ext;
    }

    //create a new extension dialog
    if (mModeComboBox->currentText() == "Empty")
    {
	ext = new QgsGraMaExtensionWidget(this, field, QgsGraSyDialog::EMPTY, mNumberOfClassesSpinbox->value(), mVectorLayer);
    } 
    else if (mModeComboBox->currentText() == "Equal Interval")
    {
	ext = new QgsGraMaExtensionWidget(this, field, QgsGraSyDialog::EQUAL_INTERVAL, mNumberOfClassesSpinbox->value(), mVectorLayer);
    }
    
    if (mNumberOfClassesSpinbox->value() == 0)
    {
	ext = 0;
	return;
    }
    
    QgsGraMaDialogBaseLayout->addMultiCellWidget(ext, 5, 5, 0, 3);
    ext->show();
}
