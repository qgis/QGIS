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


QgsGraSyDialog::QgsGraSyDialog(QgsVectorLayer* layer): QgsGraSyDialogBase(), ext(0), m_vectorlayer(layer)
{
    QObject::connect(numberofclassesspinbox,SIGNAL(valueChanged(int)),this,SLOT(adjustNumberOfClasses()));
    QObject::connect(classificationComboBox,SIGNAL(activated(int)),this,SLOT(adjustNumberOfClasses()));
    QObject::connect(modeComboBox,SIGNAL(activated(int)),this,SLOT(adjustNumberOfClasses()));
    QObject::connect(closebutton,SIGNAL(clicked()),this,SLOT(hide()));
    QObject::connect(applybutton,SIGNAL(clicked()),this,SLOT(apply()));
    setOrientation(Qt::Vertical);

    //Set the initial display name
    displaynamefield->setText(m_vectorlayer->name());

    //find out the numerical fields of m_vectorlayer
    QgsDataProvider* provider;
    if(provider=m_vectorlayer->getDataProvider())
    {
	std::vector<QgsField>& fields=provider->fields();
	int fieldnumber=0;
	QString str;

	for(std::vector<QgsField>::iterator it=fields.begin();it!=fields.end();++it)
	{
	    QString type=(*it).getType();
	    if(type!="String"&&type!="varchar"&&type!="geometry")
	    {
		str=(*it).getName();
		classificationComboBox->insertItem(str);
		m_fieldmap.insert(std::make_pair(str,fieldnumber));	
	    }
	    fieldnumber++;
	}
    }
    else
    {
	qWarning("Warning, data provider is null in QgsGraSyDialog::QgsGraSyDialog(...)");
	return;
    }
    
    modeComboBox->insertItem("empty");
    modeComboBox->insertItem("equal interval");

    setSizeGripEnabled(true);
}

QgsGraSyDialog::QgsGraSyDialog()
{

}

QgsGraSyDialog::~QgsGraSyDialog()
{
    if(ext)
    {
	ext->hide();
	delete ext;
    }
}

void QgsGraSyDialog::adjustNumberOfClasses()
{
    showExtension(false);

    hide();

    //find out the number of the classification field
    QString fieldstring=classificationComboBox->currentText();

    if(fieldstring.isEmpty())//don't do anything, it there is no classification field
    {
	show();
	return;
    }

    std::map<QString,int>::iterator iter=m_fieldmap.find(fieldstring);
    int field=iter->second;

    //create a new extension dialog
    if (modeComboBox->currentText()=="empty")
    {
	ext=new QgsGraSyExtensionWidget(this,field,QgsGraSyDialog::EMPTY,numberofclassesspinbox->value(),m_vectorlayer);
    }
    else if(modeComboBox->currentText()=="equal interval")
    {
	ext=new QgsGraSyExtensionWidget(this,field,QgsGraSyDialog::EQUAL_INTERVAL,numberofclassesspinbox->value(),m_vectorlayer);
    }
    show();

    setExtension(ext);
    showExtension(true);
    
}

void QgsGraSyDialog::apply() const
{
    if(ext)
    {
	if(classificationComboBox->currentText().isEmpty())//don't do anything, it there is no classification field
	{
	    return;
	}
	
//create the pixmap for the render item
	QPixmap* pix=m_vectorlayer->legendPixmap();
	QString name=displaynamefield->text();
	//query the name and the maximum upper value to estimate the necessary width of the pixmap (12 pixel width per letter seems to be appropriate)
	int pixwidth=90+((QLineEdit*)(ext->getWidget(1,numberofclassesspinbox->value()-1)))->text().length()*2*12;//width of the pixmap with symbol and values
	//consider 240 pixel for labels
	int pixpluswidth=pixwidth+240;
	int namewidth=45+name.length()*12;
	int width=(pixpluswidth>namewidth) ? pixpluswidth : namewidth;
	pix->resize(width,70+35*numberofclassesspinbox->value());
	pix->fill();
	QPainter p(pix);

	//draw the layer name and the name of the classification field into the pixmap
	p.drawText(45,35,name);
	m_vectorlayer->setlayerName(name);
	p.drawText(45,70,classificationComboBox->currentText());

	QgsGraduatedSymRenderer* renderer=dynamic_cast<QgsGraduatedSymRenderer*>(m_vectorlayer->renderer());
	if(!renderer)
	{
	    qWarning("Warning, typecast failed in QgsGraSyDialog::apply()");
	    return;
	}
	
	renderer->removeItems();

	for(int i=0;i<numberofclassesspinbox->value();i++)
	{
	    QgsSymbol sy(QColor(255,0,0));

	    if(m_vectorlayer->vectorType()==QGis::Polygon)
	    {
		sy.pen().setColor(((QPushButton*)(ext->getWidget(3,0)))->paletteBackgroundColor());
		sy.pen().setStyle(QgsSymbologyUtils::qString2PenStyle((((QPushButton*)(ext->getWidget(4,0)))->text())));
		sy.pen().setWidth(((QSpinBox*)(ext->getWidget(5,0)))->value());
	    }
	    else
	    {
		sy.pen().setColor(((QPushButton*)(ext->getWidget(3,i)))->paletteBackgroundColor());
		sy.pen().setStyle(QgsSymbologyUtils::qString2PenStyle((((QPushButton*)(ext->getWidget(4,i)))->text())));
		sy.pen().setWidth(((QSpinBox*)(ext->getWidget(5,i)))->value());
	    }

	    if(m_vectorlayer->vectorType()!=QGis::Line)
	    {
		sy.brush().setColor(((QPushButton*)(ext->getWidget(6,i)))->paletteBackgroundColor());
	    }

	    if(m_vectorlayer->vectorType()==QGis::Polygon)
	    {
		sy.brush().setStyle(QgsSymbologyUtils::qString2BrushStyle((((QPushButton*)(ext->getWidget(7,i)))->text())));
	    }
	    else if (m_vectorlayer->vectorType()==QGis::Point)
	    {
		sy.brush().setStyle(Qt::SolidPattern);
	    } 

	    QString lower_bound=((QLineEdit*)(ext->getWidget(0,i)))->text();
	    QString upper_bound=((QLineEdit*)(ext->getWidget(1,i)))->text();
	    QString label=((QLineEdit*)(ext->getWidget(2,i)))->text();

	    //test, if lower_bound is numeric or not (making a subclass of QString would be the proper solution) 
	    bool lbcontainsletter=false;
	    for(uint j=0;j<lower_bound.length();j++)
	    {
		if(lower_bound.ref(j).isLetter())
		{
		    lbcontainsletter=true;
		}
	    }

            //test, if upper_bound is numeric or not (making a subclass of QString would be the proper solution)
	    bool ubcontainsletter=false;
	    for(uint j=0;j<upper_bound.length();j++)
	    {
		if(upper_bound.ref(j).isLetter())
		{
		    ubcontainsletter=true;
		}
	    }

	    if(lbcontainsletter==false&&ubcontainsletter==false&&lower_bound.length()>0&&upper_bound.length()>0)//only add the item if the value bounds do not contain letters and are not null strings
	    {
		QgsRangeRenderItem* item = new QgsRangeRenderItem(sy, lower_bound, upper_bound, ((QLineEdit*)(ext->getWidget(2,i)))->text());
	
		renderer->addItem(item);
		//add the symbol to the picture
	    
		QString legendstring=lower_bound+" - "+upper_bound;
		p.setPen(sy.pen());
		p.setBrush(sy.brush());
		if(m_vectorlayer->vectorType()==QGis::Polygon)
		{
		    p.drawRect(10,70+5+30*i,30,20);//implement different painting for lines and points here
		}
		else if(m_vectorlayer->vectorType()==QGis::Line)
		{
		    p.drawLine(10,70+5+30*i+10,40,70+5+30*i+10);
		}
		else if(m_vectorlayer->vectorType()==QGis::Point)
		{
		    p.drawRect(25,70+5+30*i+10,5,5);
		}
		p.setPen(Qt::black);
		p.drawText(45,70+25+30*i,legendstring);
		p.drawText(pixwidth+10,70+25+30*i,label);
	    }
	}
	
	renderer->setClassificationField(ext->classfield());

	m_vectorlayer->triggerRepaint();
	m_vectorlayer->legendItem()->setPixmap(0,(*pix));
    }

    else//number of classes is 0
    {
	std::cout << "warning, number of classes is zero" << std::endl << std::flush;
    }

}
