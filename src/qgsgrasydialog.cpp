/***************************************************************************
                         qgsgrasydialog.cpp  -  description
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
#include "qgsgrasydialog.h"
#include "qspinbox.h"
#include "qpushbutton.h"
#include <qcombobox.h>
#include "qgsgrasyextensionwidget.h"
#include "qgssymbologyutils.h"
#include "qgsrangerenderitem.h"
#include "qlineedit.h"
#include "qgsgraduatedsymrenderer.h"
#include "qgsvectorlayer.h"
#include "qgslegenditem.h"
#include "qgsvectorlayerproperties.h"
#include "qgsdataprovider.h"
#include "qgsfield.h"
#include "qscrollview.h"
#include <qlayout.h>

QgsGraSyDialog::QgsGraSyDialog(QgsVectorLayer * layer):QgsGraSyDialogBase(), ext(0), mVectorLayer(layer)
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
		classificationComboBox->insertItem(str);
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

    modeComboBox->insertItem("Empty");
    modeComboBox->insertItem("Equal Interval");
    
    //restore the correct settings
    QgsGraduatedSymRenderer *renderer;
    
    //initial settings, use the buffer of the propertiesDialog if possible. If this is not possible, use the renderer of the vectorlayer directly
    if (mVectorLayer->propertiesDialog())
    {
	renderer = dynamic_cast < QgsGraduatedSymRenderer * >(layer->propertiesDialog()->getBufferRenderer());
    } 
    else
    {
	renderer = dynamic_cast < QgsGraduatedSymRenderer * >(layer->renderer());
    }


    if (renderer)
    {
	std::list < QgsRangeRenderItem * >list = renderer->items();
	
	ext = new QgsGraSyExtensionWidget(this, renderer->classificationField(), QgsGraSyDialog::EMPTY, list.size(), mVectorLayer);
	
	classificationComboBox->setCurrentItem(renderer->classificationField());

	QGis::VectorType m_type = mVectorLayer->vectorType();

	//set the right colors and texts to the widgets
	int number = 0;
	for (std::list < QgsRangeRenderItem * >::iterator it = list.begin(); it != list.end(); ++it)
        {
	    ((QLineEdit *) (ext->getWidget(0, number)))->setText((*it)->value());
	    ((QLineEdit *) ext->getWidget(1, number))->setText((*it)->upper_value());
	    ((QLineEdit *) ext->getWidget(2, number))->setText((*it)->label());
	    if(m_type != QGis::Polygon || number < 1)
	    {
		((QPushButton *) ext->getWidget(3, number))->setPaletteBackgroundColor((*it)->getSymbol()->pen().color());
		((QPushButton *) ext->getWidget(4, number))->setPixmap(QgsSymbologyUtils::penStyle2Pixmap((*it)->getSymbol()->pen().style()));
		((QPushButton *) ext->getWidget(4, number))->setName(QgsSymbologyUtils::penStyle2Char((*it)->getSymbol()->pen().style()));
	    }
	    
	    ((QSpinBox *) ext->getWidget(5, number))->setValue((*it)->getSymbol()->pen().width());
	    if(m_type!=QGis::Line)
	    {
		((QPushButton *) ext->getWidget(6, number))->setPaletteBackgroundColor((*it)->getSymbol()->brush().color());
		((QPushButton *) ext->getWidget(7, number))->setName(QgsSymbologyUtils::brushStyle2Char((*it)->getSymbol()->brush().style()));
		((QPushButton *) ext->getWidget(7, number))->setPixmap(QgsSymbologyUtils::brushStyle2Pixmap((*it)->getSymbol()->brush().style()));
	    }
	    number++;
        }
	
	numberofclassesspinbox->setValue(list.size());
	
	if (numberofclassesspinbox->value() == 0)
        {
	    ext = 0;
	    return;
        }
	
	numberofclassesspinbox->setValue(list.size());
	QgsGraSyDialogBaseLayout->addMultiCellWidget(ext, 5, 5, 0, 3);
	ext->show();
	
    }
    
    //do the necessary signal/slot connections
    QObject::connect(numberofclassesspinbox, SIGNAL(valueChanged(int)), this, SLOT(adjustNumberOfClasses()));
    QObject::connect(classificationComboBox, SIGNAL(activated(int)), this, SLOT(adjustNumberOfClasses()));
    QObject::connect(modeComboBox, SIGNAL(activated(int)), this, SLOT(adjustNumberOfClasses()));
}

QgsGraSyDialog::QgsGraSyDialog()
{
#ifdef QGISDEBUG
    qWarning("constructor QgsGraSyDialog");
#endif
}

QgsGraSyDialog::~QgsGraSyDialog()
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

void QgsGraSyDialog::adjustNumberOfClasses()
{
    //find out the number of the classification field
    QString fieldstring = classificationComboBox->currentText();
    
    if (fieldstring.isEmpty())    //don't do anything, it there is no classification field
    {
	show();
	return;
    }
    
    std::map < QString, int >::iterator iter = mFieldMap.find(fieldstring);
    int field = iter->second;
    
    if(ext)
    {
	QgsGraSyDialogBaseLayout->remove(ext);
	delete ext;
    }
    
    //create a new extension dialog
    if (modeComboBox->currentText() == "Empty")
    {
	ext = new QgsGraSyExtensionWidget(this, field, QgsGraSyDialog::EMPTY, numberofclassesspinbox->value(), mVectorLayer);
    } 
    else if (modeComboBox->currentText() == "Equal Interval")
    {
	ext = new QgsGraSyExtensionWidget(this, field, QgsGraSyDialog::EQUAL_INTERVAL, numberofclassesspinbox->value(), mVectorLayer);
    }
    
    if (numberofclassesspinbox->value() == 0)
    {
	ext = 0;
	return;
    }
    
    QgsGraSyDialogBaseLayout->addMultiCellWidget(ext, 5, 5, 0, 3);
    ext->show();
    
}

void QgsGraSyDialog::apply()
{
    if (ext)
    {
	if (classificationComboBox->currentText().isEmpty())  //don't do anything, it there is no classification field
        {
	    return;
        }

	//font tor the legend text
	QFont f("times", 12, QFont::Normal);
	QFontMetrics fm(f);
	
	//spaces
	int topspace = 5;
	int bottomspace = 5;
	int leftspace = 10;       //space between left side of the pixmap and the text/graphics
	int rightspace = 5;       //space between text/graphics and right side of the pixmap
	int wordspace = 5;        //space between graphics/word
	int symbolheight = 15;    //height of an area where a symbol is painted
	int symbolwidth = 15;     //width of an area where a symbol is painted
	int rowspace = 5;         //spaces between rows of symbols
	int rowheight = (fm.height() > symbolheight) ? fm.height() : symbolheight;  //height of a row in the symbology part
	//find out the width of the widest label
	QString widestlabel = "";
	for (int i = 0; i < numberofclassesspinbox->value(); i++)
        {
	    QString string = ((QLineEdit *) (ext->getWidget(2, i)))->text();
	    if (string.length() > widestlabel.length())
            {
		widestlabel = string;
            }
        }
	int labelwidth = fm.width(widestlabel);
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
	//query the name and the maximum upper value to estimate the necessary width of the pixmap (12 pixel width per letter seems to be appropriate)
	int pixwidth = leftspace + rightspace + symbolwidth + 2 * wordspace + labelwidth + fm.width(((QLineEdit *) (ext->getWidget(1, numberofclassesspinbox->value() - 1)))->text() + " - " + ((QLineEdit *) (ext->getWidget(0, numberofclassesspinbox->value() - 1)))->text()); //width of the pixmap with symbol and values
	//consider 240 pixel for labels
	int namewidth = leftspace + fm.width(name) + rightspace;
	int width = (pixwidth > namewidth) ? pixwidth : namewidth;
	pix->resize(width, topspace + 2 * fm.height() + bottomspace + (rowheight + rowspace) * numberofclassesspinbox->value());
	pix->fill();
	QPainter p(pix);
	p.setFont(f);
	//draw the layer name and the name of the classification field into the pixmap
	p.drawText(leftspace, topspace + fm.height(), name);
	p.drawText(leftspace, topspace + 2 * fm.height(), classificationComboBox->currentText());
	
	
	QgsGraduatedSymRenderer *renderer = dynamic_cast < QgsGraduatedSymRenderer * >(mVectorLayer->renderer());
	
	if (!renderer)
        {
	    qWarning("Warning, typecast failed in QgsGraSyDialog::apply()");
	    return;
        }
	
	renderer->removeItems();
	
	int offset = topspace + 2 * fm.height();
	int rowincrement = rowheight + rowspace;
	for (int i = 0; i < numberofclassesspinbox->value(); i++)
        {
	    QgsSymbol sy(QColor(255, 0, 0));
	    
	    if (mVectorLayer->vectorType() == QGis::Polygon)
            {
		sy.pen().setColor(((QPushButton *) (ext->getWidget(3, 0)))->paletteBackgroundColor());
		sy.pen().setStyle(QgsSymbologyUtils::char2PenStyle((((QPushButton *) (ext->getWidget(4, 0)))->name())));
		sy.pen().setWidth(((QSpinBox *) (ext->getWidget(5, 0)))->value());
	    } 
	    else
            {
		sy.pen().setColor(((QPushButton *) (ext->getWidget(3, i)))->paletteBackgroundColor());
		sy.pen().setStyle(QgsSymbologyUtils::char2PenStyle((((QPushButton *) (ext->getWidget(4, i)))->name())));
		sy.pen().setWidth(((QSpinBox *) (ext->getWidget(5, i)))->value());
            }
	    
	    if (mVectorLayer->vectorType() != QGis::Line)
            {
		sy.brush().setColor(((QPushButton *) (ext->getWidget(6, i)))->paletteBackgroundColor());
            }
	    
	    if (mVectorLayer->vectorType() == QGis::Polygon)
            {
		sy.brush().setStyle(QgsSymbologyUtils::char2BrushStyle((((QPushButton *) (ext->getWidget(7, i)))->name())));
	    } 
	    else if (mVectorLayer->vectorType() == QGis::Point)
            {
		sy.brush().setStyle(Qt::SolidPattern);
            }
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
		QgsRangeRenderItem *item = new QgsRangeRenderItem(sy, lower_bound, upper_bound, ((QLineEdit *) (ext->getWidget(2, i)))->text());
		
		renderer->addItem(item);
		//add the symbol to the picture

		QString legendstring = lower_bound + " - " + upper_bound;
		p.setPen(sy.pen());
		p.setBrush(sy.brush());
		if (mVectorLayer->vectorType() == QGis::Polygon)
                {
		    p.drawRect(leftspace, offset + rowincrement * i + (rowheight - symbolheight), symbolwidth, symbolheight); //implement different painting for lines and points here
		} 
		else if (mVectorLayer->vectorType() == QGis::Line)
                {
		    p.drawLine(leftspace, offset + rowincrement * i + (rowheight - symbolheight), leftspace + symbolwidth,
                             offset + rowincrement * i + rowheight);
		} 
		else if (mVectorLayer->vectorType() == QGis::Point)
                {
		    p.drawRect(leftspace + symbolwidth / 2, offset + (int) (rowincrement * (i + 0.5)), 5, 5);
                }
		p.setPen(Qt::black);
		p.drawText(leftspace + symbolwidth + wordspace, offset + rowincrement * i + rowheight, legendstring);
		p.drawText(pixwidth - labelwidth - rightspace, offset + rowincrement * i + rowheight, label);
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
