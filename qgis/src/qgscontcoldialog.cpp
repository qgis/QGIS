#include "qgscontcoldialog.h"
#include "qgsvectorlayer.h"
#include "qpushbutton.h"
#include "qcolordialog.h"
#include <qcombobox.h>
#include <cfloat>
#include "qgscontinuouscolrenderer.h"
#include <iostream>
#include "qgslayerproperties.h"
#include "qgslegenditem.h"
#include <qlineedit.h>
#include "qgsvectorlayerproperties.h"
#include "qgsdataprovider.h"
#include "qgsfield.h"

QgsContColDialog::QgsContColDialog(QgsVectorLayer* layer): QgsContColDialogBase(), m_vectorlayer(layer)
{
    QObject::connect(closebutton,SIGNAL(clicked()),this,SLOT(hide()));
    QObject::connect(applybutton,SIGNAL(clicked()),this,SLOT(apply()));
    QObject::connect(mincolorbutton,SIGNAL(clicked()),this,SLOT(selectMinimumColor()));
    QObject::connect(maxcolorbutton,SIGNAL(clicked()),this,SLOT(selectMaximumColor()));

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
	qWarning("Warning, data provider is null in QgsContColDialog::QgsContColDialog(...)");
	return;
    }
}

QgsContColDialog::QgsContColDialog()
{
    
}

QgsContColDialog::~QgsContColDialog()
{

}

void QgsContColDialog::apply()
{
    QString fieldstring=classificationComboBox->currentText();
    if(fieldstring.isEmpty())//don't do anything, it there is no classification field
    {
	return;
    }
    std::map<QString,int>::iterator iter=m_fieldmap.find(fieldstring);
    int classfield=iter->second;

    //find the minimum and maximum for the classification variable
    double minimum,maximum;
    QgsDataProvider* provider = m_vectorlayer->getDataProvider();
    if(provider)
    {
	minimum=provider->minValue(classfield).toDouble();
	maximum=provider->maxValue(classfield).toDouble();
    }
    else
    {
	qWarning("Warning, provider is null in QgsGraSyExtensionWidget::QgsGraSyExtensionWidget(...)");
	return;
    }


    //create the render items for minimum and maximum value
    QgsSymbol minsymbol;
    if(m_vectorlayer->vectorType()==QGis::Line)
    {
	minsymbol.setPen(QPen(mincolorbutton->paletteBackgroundColor()));  
    }
    else
    {
	minsymbol.setBrush(QBrush(mincolorbutton->paletteBackgroundColor()));
	minsymbol.setPen(QPen(QColor(0,0,0),1));
    }
    QgsRenderItem* minimumitem=new QgsRenderItem(minsymbol,QString::number(minimum,'f')," ");

    QgsSymbol maxsymbol;
    if(m_vectorlayer->vectorType()==QGis::Line)
    {
	maxsymbol.setPen(QPen(maxcolorbutton->paletteBackgroundColor()));
    }
    else
    {
	maxsymbol.setBrush(QBrush(maxcolorbutton->paletteBackgroundColor()));
	maxsymbol.setPen(QPen(QColor(0,0,0),1));
    }
    QgsRenderItem* maximumitem=new QgsRenderItem(maxsymbol,QString::number(maximum,'f')," ");

    //set the render items to the render of m_vectorlayer
    QgsContinuousColRenderer* renderer=dynamic_cast<QgsContinuousColRenderer*>(m_vectorlayer->renderer());
    if(renderer)
    {
	renderer->setMinimumItem(minimumitem);
	renderer->setMaximumItem(maximumitem);
	renderer->setClassificationField(classfield);
    }
    else
    {
	qWarning("Warning, typecast failed in QgsContColDialog::apply()");
	return;
    }
	
    //add a pixmap to the QgsLegendItem
    QPixmap* pix=m_vectorlayer->legendPixmap();
    //use the name and the maximum value to estimate the necessary width of the pixmap (12 pixel width per letter seems to be appropriate)
    QString name=displaynamefield->text();
    int namewidth=45+name.length()*12;
    int numberlength=(int)(60+QString::number(maximum,'f',2).length()*12);
    int pixwidth=(numberlength > namewidth) ? numberlength : namewidth;
    pix->resize(pixwidth,200);
    pix->fill();
    QPainter p(pix);

    p.setPen(QPen(QColor(0,0,0),1));
    //draw the layer name and the name of the classification field into the pixmap
    p.drawText(45,35,name);
    m_vectorlayer->setlayerName(name);
    p.drawText(45,70,classificationComboBox->currentText());
	
    //draw the color range line by line
    for(int i=0;i<100;i++)
    {
	p.setPen(QColor(mincolorbutton->paletteBackgroundColor().red()+(maxcolorbutton->paletteBackgroundColor().red()-mincolorbutton->paletteBackgroundColor().red())/100*i,mincolorbutton->paletteBackgroundColor().green()+(maxcolorbutton->paletteBackgroundColor().green()-mincolorbutton->paletteBackgroundColor().green())/100*i,mincolorbutton->paletteBackgroundColor().blue()+(maxcolorbutton->paletteBackgroundColor().blue()-mincolorbutton->paletteBackgroundColor().blue())/100*i));//use the appropriate color
	p.drawLine(10,90+i,50,90+i);
    }

    //draw the minimum and maximum values beside the color range
    p.setPen(QPen(QColor(0,0,0),1));
    p.drawText(60,105,QString::number(minimum,'f',2));
    p.drawText(60,190,QString::number(maximum,'f',2));

    m_vectorlayer->triggerRepaint();
    m_vectorlayer->legendItem()->setPixmap(0,(*pix));
}

void QgsContColDialog::selectMinimumColor()
{
    mincolorbutton->setPaletteBackgroundColor(QColorDialog::getColor());
    m_vectorlayer->propertiesDialog()->raise();
    raise();
}

void QgsContColDialog::selectMaximumColor()
{
    maxcolorbutton->setPaletteBackgroundColor(QColorDialog::getColor());
    m_vectorlayer->propertiesDialog()->raise();
    raise();
}
