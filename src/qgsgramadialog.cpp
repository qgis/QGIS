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
#include "qgslegenditem.h"
#include <qcombobox.h>
#include <qlayout.h>
#include <qpainter.h>
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

    std::list < QgsRangeRenderItem * >list;
    
    if (renderer)
    {
	list = renderer->items();
	ext = new QgsGraMaExtensionWidget(this, renderer->classificationField(), QgsGraSyDialog::EMPTY, list.size(), mVectorLayer);
    }

    mClassificationComboBox->setCurrentItem(renderer->classificationField());
    
    //set the right colors and texts to the widgets
    int number = 0;
    for (std::list < QgsRangeRenderItem * >::iterator it = list.begin(); it != list.end(); ++it)
    {
	((QLineEdit *) (ext->getWidget(0, number)))->setText((*it)->value());
	((QLineEdit *) ext->getWidget(1, number))->setText((*it)->upper_value());
	((QLineEdit *) ext->getWidget(2, number))->setText((*it)->label());
	((QPushButton *) ext->getWidget(3, number))->setName(((QgsMarkerSymbol*)((*it)->getSymbol()))->picture());
	((QLineEdit *) ext->getWidget(4, number))->setText(QString::number(((QgsMarkerSymbol*)((*it)->getSymbol()))->scaleFactor(),'f',2));
	number++;
    }
    

    mNumberOfClassesSpinbox->setValue(list.size());
    QgsGraMaDialogBaseLayout->addMultiCellWidget(ext, 5, 5, 0, 3);
    ext->show();

    //do the necessary signal/slot connections
    QObject::connect(mNumberOfClassesSpinbox, SIGNAL(valueChanged(int)), this, SLOT(adjustNumberOfClasses()));
    QObject::connect(mClassificationComboBox, SIGNAL(activated(int)), this, SLOT(adjustClassification()));
    QObject::connect(mModeComboBox, SIGNAL(activated(int)), this, SLOT(adjustClassification()));
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
    if (ext)
    {
	ext->adjustMarkers();

	if (mClassificationComboBox->currentText().isEmpty())  //don't do anything, it there is no classification field
        {
	    return;
        }

	//font tor the legend text
	QFont f("arial", 10, QFont::Normal);
	QFontMetrics fm(f);
	
	//spaces
	int topspace = 5;
	int bottomspace = 5;
	int leftspace = 10;       //space between left side of the pixmap and the text/graphics
	int rightspace = 5;       //space between text/graphics and right side of the pixmap
	int wordspace = 5;        //space between graphics/word
	int markerheight;    //height of a marker (is different for every row)
	int markerwidth;     //width of the broadest marker
	int lowerupperwidth; // widht of the broadest lower-upper pair
	int rowspace = 5;         //spaces between rows of symbols
	int rowheight = fm.height();  //height of a text row
	int classesheight; //height of the classes section
	
        //find out the width of the widest label and the widest lower - upper pair
	int labelwidth=0;
	lowerupperwidth=0;

	for (int i = 0; i < mNumberOfClassesSpinbox->value(); i++)
        {
	    int currentlabelwidth=fm.width(((QLineEdit *) (ext->getWidget(2, i)))->text());
	    if(currentlabelwidth>labelwidth)
            {
		labelwidth=currentlabelwidth;
            }
	    
	    int currentluwidth=fm.width(((QLineEdit *) (ext->getWidget(0, i)))->text() + " - " + ((QLineEdit *) (ext->getWidget(1, i)))->text());
	    if(currentluwidth>lowerupperwidth)
	    {
		lowerupperwidth=currentluwidth;
	    }
        }

	//find out the width of the broadest marker and the total height of the class section
	markerwidth=0;
	classesheight=rowspace*(mNumberOfClassesSpinbox->value()-1);
	for (int i = 0; i < mNumberOfClassesSpinbox->value(); i++)
        {
	    QPicture p;
	    p.load(((QPushButton*)(ext->getWidget(3,i)))->name(),"svg");
	    int width=(int)(p.boundingRect().width()*((QLineEdit*)(ext->getWidget(4,i)))->text().toDouble());
	    if(width>markerwidth)
	    {
		markerwidth=width;
		qWarning("markerwidth: "+QString::number(markerwidth));
	    }
	    int height= (int)(p.boundingRect().height()*((QLineEdit*)(ext->getWidget(4,i)))->text().toDouble());
	    height = (height>rowheight) ? height : rowheight;
	    qWarning("height: " + QString::number(height));
	    classesheight+=height;
	}

	//create the pixmap for the render item
	QPixmap *pix = mVectorLayer->legendPixmap();
	QString name;
	if (mVectorLayer->propertiesDialog())
        {
	    name = mVectorLayer->propertiesDialog()->displayName();
	} 
	else
        {
	    name = "";
        }

	//query the name and the maximum upper value to estimate the necessary width of the pixmap
	int pixwidth = leftspace + rightspace + markerwidth + 2 * wordspace + labelwidth + lowerupperwidth; //width of the pixmap with symbol and values
	//consider 240 pixel for labels
	int namewidth = leftspace + fm.width(name) + rightspace;
	int width = (pixwidth > namewidth) ? pixwidth : namewidth;
	pix->resize(width, topspace + 2 * fm.height() + bottomspace + classesheight);
	pix->fill();

	QPainter p(pix);
	p.setFont(f);
	//draw the layer name and the name of the classification field into the pixmap
	p.drawText(leftspace, topspace + fm.height(), name);
	p.drawText(leftspace, topspace + 2 * fm.height(), mClassificationComboBox->currentText());
	
	QgsGraduatedMaRenderer *renderer = dynamic_cast < QgsGraduatedMaRenderer * >(mVectorLayer->renderer());
	
	if (!renderer)
        {
	    qWarning("Warning, typecast failed in QgsGraSyDialog::apply()");
	    return;
        }

	renderer->removeItems();
      
	int offset = topspace + 2 * fm.height();
	for (int i = 0; i < mNumberOfClassesSpinbox->value(); i++)
        {
	    QgsMarkerSymbol* sy = new QgsMarkerSymbol();
	    sy->setPicture(((QPushButton*)(ext->getWidget(3,i)))->name());
	    qWarning("SVG file: " + QString::fromAscii(((QPushButton*)(ext->getWidget(3,i)))->name()));
	    sy->setScaleFactor(((QLineEdit*)(ext->getWidget(4,i)))->text().toDouble());
	    
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
	    
	    if (lbcontainsletter == false && ubcontainsletter == false && lower_bound.length() > 0 && upper_bound.length() > 0) //only add the item if the value bounds do not contain letters and are not null strings
            {
		QgsRangeRenderItem *item = new QgsRangeRenderItem(sy, lower_bound, upper_bound, label);
		qWarning("lower_bound: " +lower_bound);
		qWarning("upper_bound: " +upper_bound);
		qWarning("label: " +label);
		renderer->addItem(item);
		
                //add the symbol to the picture
		QString legendstring = lower_bound + " - " + upper_bound;
		//todo: paint the picture
		QPicture pic;
		pic.load(((QPushButton*)(ext->getWidget(3,i)))->name(),"svg");
		double scalefactor=((QLineEdit*)(ext->getWidget(4,i)))->text().toDouble();
		int actrowheight=(int)(pic.boundingRect().height()*scalefactor);
		actrowheight= (actrowheight > rowheight) ? actrowheight : rowheight;
		qWarning("offset: "+QString::number(offset));
		qWarning("row: "+QString::number(i));
		qWarning("scalefactor: "+QString::number(scalefactor));
		qWarning("actrowheight: "+QString::number(actrowheight));
		qWarning("rowheight: " +QString::number(rowheight));
		p.scale(scalefactor,scalefactor);
		p.drawPicture((int)(leftspace/scalefactor),(int)(offset/scalefactor),pic);
		p.resetXForm();
		p.setPen(Qt::black);
		p.drawText(leftspace+markerwidth + wordspace, offset + actrowheight, legendstring);
		p.drawText(leftspace+markerwidth+2*wordspace+lowerupperwidth, offset + actrowheight, label);
		offset+=(rowspace+actrowheight);
	    }
	}
	
	renderer->setClassificationField(ext->classfield());

	if (mVectorLayer->legendItem())
        {
	    mVectorLayer->legendItem()->setPixmap(0, (*pix));
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

void QgsGraMaDialog::adjustClassification()
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
	if (mModeComboBox->currentText() == "Empty")
	{
	    ext->setClassification(QgsGraSyDialog::EMPTY,field);
	}
	else if(mModeComboBox->currentText() == "Equal Interval")
	{
	    ext->setClassification(QgsGraSyDialog::EQUAL_INTERVAL,field);
	}
    }
}
