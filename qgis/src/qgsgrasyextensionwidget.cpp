/***************************************************************************
  qgsgrasyextensionwidget.cpp  -  description
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
#include "qgsgrasyextensionwidget.h"
#include <qlabel.h>
#include <qlineedit.h>
#include <qspinbox.h>
#include <qpushbutton.h>
#include <qcolordialog.h>
#include "qgslinestyledialog.h"
#include "qgspatterndialog.h"
#include "qgssymbologyutils.h"
#include <iostream>
#include <cfloat>
#include <qlayout.h>
#include "qgsvectorlayer.h"
#include "qgsdlgvectorlayerproperties.h"
#include "qgsdataprovider.h"
#include "qgsfeature.h"

QgsGraSyExtensionWidget::QgsGraSyExtensionWidget(QWidget * parent, int classfield, QgsGraSyDialog::mode mode, int nofclasses, QgsVectorLayer * vlayer):QScrollView(parent), m_classfield(classfield), mGridLayout(new QGridLayout(viewport(), 1, 8)), mMode(mode), mNumberOfClasses(nofclasses), mVectorLayer(vlayer)
{
#ifdef QGISDEBUG
    qWarning("constructor QgsGraSyExtensionWidget");
#endif
 
    setResizePolicy(QScrollView::AutoOneFit);
    mGridLayout->setSpacing(5);

    //fill the title line into the grid layout
    QLabel *lvaluelabel = new QLabel(tr("Lower"),viewport());
    lvaluelabel->setMaximumHeight(50);
    mGridLayout->addWidget(lvaluelabel, 0, 0);
    QLabel *uvaluelabel = new QLabel(tr("Upper"),viewport());
    uvaluelabel->setMaximumHeight(50);
    mGridLayout->addWidget(uvaluelabel, 0, 1);
    QLabel *labellabel = new QLabel(tr("Label"),viewport());
    labellabel->setMaximumHeight(50);
    mGridLayout->addWidget(labellabel, 0, 2);
    QLabel *outlinecolorlabel = new QLabel(tr("Outline\nColor"),viewport());
    outlinecolorlabel->setMaximumHeight(50);
    mGridLayout->addWidget(outlinecolorlabel, 0, 3);
    QLabel *outlinestylelabel = new QLabel(tr("Outline\nStyle"),viewport());
    outlinestylelabel->setMaximumHeight(50);
    mGridLayout->addWidget(outlinestylelabel, 0, 4);
    QLabel *outlinewidthlabel = new QLabel(tr("Outline\nWidth"),viewport());
    outlinewidthlabel->setMaximumHeight(50);
    mGridLayout->addWidget(outlinewidthlabel, 0, 5);
    QLabel *fillcolorlabel = new QLabel(tr("Fill\nColor"),viewport());
    fillcolorlabel->setMaximumHeight(50);
    mGridLayout->addWidget(fillcolorlabel, 0, 6);
    QLabel *fillpatternlabel = new QLabel(tr("Fill\nPattern"),viewport());
    fillpatternlabel->setMaximumHeight(50);
    mGridLayout->addWidget(fillpatternlabel, 0, 7);

    //fint the minimum and maximum of the classification variable
    double minimum, maximum;

    QgsDataProvider *provider = mVectorLayer->getDataProvider();
    if (provider)
    {
	if (mMode == QgsGraSyDialog::EQUAL_INTERVAL)
	{
	    minimum = provider->minValue(m_classfield).toDouble();
	    maximum = provider->maxValue(m_classfield).toDouble();
	} 
	else                    //don't waste performance if mMode is QgsGraSyDialog::EMPTY
	{
	    minimum = 0;
	    maximum = 0;
	}
    } 
    else
    {
      qWarning("Warning, provider is null in QgsGraSyExtensionWidget::QgsGraSyExtensionWidget(...)");
      return;
    }

    m_widgetvector.resize(mNumberOfClasses * 8);
    QGis::VectorType m_type = mVectorLayer->vectorType();

    //create the required number of rows
    for (int i = 1; i <= mNumberOfClasses; i++)
    {

	QLineEdit *ltextfield = new QLineEdit(viewport());
	mGridLayout->addWidget(ltextfield, i, 0);
	m_widgetvector[8 * (i - 1)] = ltextfield;
	ltextfield->setAlignment(Qt::AlignLeft);

	QLineEdit *utextfield = new QLineEdit(viewport());
	mGridLayout->addWidget(utextfield, i, 1);
	m_widgetvector[8 * (i - 1) + 1] = utextfield;
	ltextfield->setAlignment(Qt::AlignLeft);

	QLineEdit *labeltextfield = new QLineEdit(viewport());
	mGridLayout->addWidget(labeltextfield, i, 2);
	m_widgetvector[8 * (i - 1) + 2] = labeltextfield;
	ltextfield->setAlignment(Qt::AlignLeft);

	QPushButton *outlinecolorbutton = new QPushButton(viewport());
	outlinecolorbutton->setMinimumWidth(20);
	outlinecolorbutton->setPaletteBackgroundColor(QColor(0, 0, 0));
	mGridLayout->addWidget(outlinecolorbutton, i, 3);
	m_widgetvector[8 * (i - 1) + 3] = outlinecolorbutton;
	QObject::connect(outlinecolorbutton, SIGNAL(clicked()), this, SLOT(selectColor()));

	QPushButton *outlinestylebutton = new QPushButton(viewport());
	outlinestylebutton->setMinimumWidth(20);
	outlinestylebutton->setName("SolidLine");
	outlinestylebutton->setPixmap(QgsSymbologyUtils::char2LinePixmap("SolidLine"));
	mGridLayout->addWidget(outlinestylebutton, i, 4);
	m_widgetvector[8 * (i - 1) + 4] = outlinestylebutton;
	QObject::connect(outlinestylebutton, SIGNAL(clicked()), this, SLOT(selectOutlineStyle()));

	QSpinBox *outlinewidthspinbox = new QSpinBox(viewport());
	outlinewidthspinbox->setMinValue(1);//set line width 1 as minimum to avoid confusion between line width 0 and no pen line style
	mGridLayout->addWidget(outlinewidthspinbox, i, 5);
	m_widgetvector[8 * (i - 1) + 5] = outlinewidthspinbox;

	QPushButton *fillcolorbutton = new QPushButton(viewport());
	fillcolorbutton->setMinimumWidth(20);
	mGridLayout->addWidget(fillcolorbutton, i, 6);
	m_widgetvector[8 * (i - 1) + 6] = fillcolorbutton;
	QObject::connect(fillcolorbutton, SIGNAL(clicked()), this, SLOT(selectColor()));

	QPushButton *fillpatternbutton = new QPushButton(viewport());
	fillpatternbutton->setMinimumWidth(20);
	fillpatternbutton->setName("SolidPattern");
	fillpatternbutton->setPixmap(QgsSymbologyUtils::char2PatternPixmap("SolidPattern"));
	mGridLayout->addWidget(fillpatternbutton, i, 7);
	m_widgetvector[8 * (i - 1) + 7] = fillpatternbutton;
	QObject::connect(fillpatternbutton, SIGNAL(clicked()), this, SLOT(selectFillPattern()));




	//apply a nice color range from blue to red as default
	if (i == 1)
        {
	    if (m_type == QGis::Line)
            {
		outlinecolorbutton->setPaletteBackgroundColor(QColor(0, 0, 255));
	    } 
	    else                //point or polygon
            {
		fillcolorbutton->setPaletteBackgroundColor(QColor(0, 0, 255));
            }
	} 
	else
        {
	    if (m_type == QGis::Line)
            {
		outlinecolorbutton->
		    setPaletteBackgroundColor(QColor(255 / mNumberOfClasses * i, 0, 255 - (255 / mNumberOfClasses * i)));
	    } 
	    else                //point or polygon
            {
		fillcolorbutton->setPaletteBackgroundColor(QColor(255 / mNumberOfClasses * i, 0, 255 - (255 / mNumberOfClasses * i)));
            }
        }

	//disable the outline fieldes if the shapetye is polygon and i>1
	if (m_type == QGis::Polygon && i > 1)
        {
	    outlinecolorbutton->unsetPalette();
	    outlinecolorbutton->setEnabled(false);
	    outlinestylebutton->setText("");
	    outlinestylebutton->setEnabled(false);
	    outlinewidthspinbox->setSpecialValueText(" ");
	    outlinewidthspinbox->setValue(0);
	    outlinewidthspinbox->setEnabled(false);
        }
	//disable the fill fields if the shapetype is line
	if (m_type == QGis::Line)
        {
	    fillcolorbutton->unsetPalette();
	    fillcolorbutton->setEnabled(false);
	    fillpatternbutton->setText("");
	    fillpatternbutton->setEnabled(false);
        }
	//disable the pattern field if the shapefye is a point or a line
	if (m_type == QGis::Point || m_type == QGis::Line)
        {
	    fillpatternbutton->setEnabled(false);
        }

	/*Set the default values of the lower and upper bounds according to the chosen mode */
	if (mMode == QgsGraSyDialog::EQUAL_INTERVAL)
        {
	    ltextfield->setText(QString::number(minimum + (maximum - minimum) / mNumberOfClasses * (i - 1), 'f', 2));
	    utextfield->setText(QString::number(minimum + (maximum - minimum) / mNumberOfClasses * i, 'f', 2));
        }
    }

    resizeContents(200,50*(mNumberOfClasses+1));
    updateContents();
  
}

QgsGraSyExtensionWidget::QgsGraSyExtensionWidget(): QScrollView(0)
{
#ifdef QGISDEBUG
    qWarning("constructor QgsGraSyExtensionWidget");
#endif
}

QgsGraSyExtensionWidget::~QgsGraSyExtensionWidget()
{
#ifdef QGISDEBUG
    qWarning("destructor QgsGraSyExtensionWidget");
#endif
}

QWidget *QgsGraSyExtensionWidget::getWidget(int column, int row)
{
    return m_widgetvector[8 * row + column];
}

void QgsGraSyExtensionWidget::selectColor()
{
    ((QPushButton *) sender())->setPaletteBackgroundColor(QColorDialog::getColor(QColor(black),this));
    setActiveWindow();
}

void QgsGraSyExtensionWidget::selectFillPattern()
{
    QgsPatternDialog patterndialog;
    if (patterndialog.exec() == QDialog::Accepted)
    {
	((QPushButton *) sender())->setName(QgsSymbologyUtils::brushStyle2Char(patterndialog.pattern()));
	((QPushButton *) sender())->setPixmap(QgsSymbologyUtils::brushStyle2Pixmap(patterndialog.pattern()));
    }
    setActiveWindow();
}

void QgsGraSyExtensionWidget::selectOutlineStyle()
{
    QgsLineStyleDialog linestyledialog;
    if (linestyledialog.exec() == QDialog::Accepted)
    {
	((QPushButton *) sender())->setName(QgsSymbologyUtils::penStyle2Char(linestyledialog.style()));
	((QPushButton *) sender())->setPixmap(QgsSymbologyUtils::penStyle2Pixmap(linestyledialog.style()));
    }
    setActiveWindow();
}

void QgsGraSyExtensionWidget::resizeEvent(QResizeEvent* e)
{
    setContentsPos (0,0);
    QScrollView::resizeEvent(e);
}
