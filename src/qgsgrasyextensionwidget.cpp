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

QgsGraSyExtensionWidget::QgsGraSyExtensionWidget(QWidget * parent, int classfield, QgsGraSyDialog::mode mode, int nofclasses, QgsVectorLayer * vlayer):QWidget(parent), m_classfield(classfield), m_gridlayout(new QGridLayout(this, 1, 8)), m_mode(mode), m_numberofclasses(nofclasses),
m_vectorlayer
(vlayer)
{
#ifdef DEBUG
  qWarning("constructor QgsGraSyExtensionWidget");
#endif

  m_gridlayout->setSpacing(10);

  //fill the title line into the grid layout
  QLabel *lvaluelabel = new QLabel(tr("Lower"), this);
  m_gridlayout->addWidget(lvaluelabel, 0, 0);
  QLabel *uvaluelabel = new QLabel(tr("Upper"), this);
  m_gridlayout->addWidget(uvaluelabel, 0, 1);
  QLabel *labellabel = new QLabel(tr("Label"), this);
  m_gridlayout->addWidget(labellabel, 0, 2);
  QLabel *outlinecolorlabel = new QLabel(tr("Outline\nColor"), this);
  m_gridlayout->addWidget(outlinecolorlabel, 0, 3);
  QLabel *outlinestylelabel = new QLabel(tr("Outline\nStyle"), this);
  m_gridlayout->addWidget(outlinestylelabel, 0, 4);
  QLabel *outlinewidthlabel = new QLabel(tr("Outline\nWidth"), this);
  m_gridlayout->addWidget(outlinewidthlabel, 0, 5);
  QLabel *fillcolorlabel = new QLabel(tr("Fill\nColor"), this);
  m_gridlayout->addWidget(fillcolorlabel, 0, 6);
  QLabel *fillpatternlabel = new QLabel(tr("Fill\nPattern"), this);
  m_gridlayout->addWidget(fillpatternlabel, 0, 7);

  //fint the minimum and maximum of the classification variable
  double minimum, maximum;

  QgsDataProvider *provider = m_vectorlayer->getDataProvider();
  if (provider)
    {
      if (m_mode == QgsGraSyDialog::EQUAL_INTERVAL)
        {
          minimum = provider->minValue(m_classfield).toDouble();
          maximum = provider->maxValue(m_classfield).toDouble();
      } else                    //don't waste performance if m_mode is QgsGraSyDialog::EMPTY
        {
          minimum = 0;
          maximum = 0;
        }
  } else
    {
      qWarning("Warning, provider is null in QgsGraSyExtensionWidget::QgsGraSyExtensionWidget(...)");
      return;
    }

  m_widgetvector.resize(m_numberofclasses * 8);
  QGis::VectorType m_type = m_vectorlayer->vectorType();

  //create the required number of rows
  for (int i = 1; i <= m_numberofclasses; i++)
    {

      QLineEdit *ltextfield = new QLineEdit(this);
      ltextfield->setMaximumWidth(60);
      m_gridlayout->addWidget(ltextfield, i, 0);
      m_widgetvector[8 * (i - 1)] = ltextfield;
      ltextfield->setAlignment(Qt::AlignLeft);

      QLineEdit *utextfield = new QLineEdit(this);
      utextfield->setMaximumWidth(60);
      m_gridlayout->addWidget(utextfield, i, 1);
      m_widgetvector[8 * (i - 1) + 1] = utextfield;
      ltextfield->setAlignment(Qt::AlignLeft);

      QLineEdit *labeltextfield = new QLineEdit(this);
      labeltextfield->setMaximumWidth(90);
      m_gridlayout->addWidget(labeltextfield, i, 2);
      m_widgetvector[8 * (i - 1) + 2] = labeltextfield;
      ltextfield->setAlignment(Qt::AlignLeft);

      QPushButton *outlinecolorbutton = new QPushButton(this);
      outlinecolorbutton->setMaximumWidth(20);
      outlinecolorbutton->setPaletteBackgroundColor(QColor(0, 0, 0));
      m_gridlayout->addWidget(outlinecolorbutton, i, 3);
      m_widgetvector[8 * (i - 1) + 3] = outlinecolorbutton;
      QObject::connect(outlinecolorbutton, SIGNAL(clicked()), this, SLOT(selectColor()));

      QPushButton *outlinestylebutton = new QPushButton(this);
      outlinestylebutton->setMaximumWidth(20);
      outlinestylebutton->setMaximumHeight(20);
      outlinestylebutton->setName("SolidLine");
      outlinestylebutton->setPixmap(QgsSymbologyUtils::char2LinePixmap("SolidLine"));
      m_gridlayout->addWidget(outlinestylebutton, i, 4);
      m_widgetvector[8 * (i - 1) + 4] = outlinestylebutton;
      QObject::connect(outlinestylebutton, SIGNAL(clicked()), this, SLOT(selectOutlineStyle()));

      QSpinBox *outlinewidthspinbox = new QSpinBox(this);
      outlinewidthspinbox->setMaximumWidth(60);
      outlinewidthspinbox->setValue(1);
      m_gridlayout->addWidget(outlinewidthspinbox, i, 5);
      m_widgetvector[8 * (i - 1) + 5] = outlinewidthspinbox;



      QPushButton *fillcolorbutton = new QPushButton(this);
      fillcolorbutton->setMaximumWidth(20);
      fillcolorbutton->setMaximumHeight(20);
      m_gridlayout->addWidget(fillcolorbutton, i, 6);
      m_widgetvector[8 * (i - 1) + 6] = fillcolorbutton;
      QObject::connect(fillcolorbutton, SIGNAL(clicked()), this, SLOT(selectColor()));

      QPushButton *fillpatternbutton = new QPushButton(this);
      fillpatternbutton->setMaximumWidth(20);
      fillpatternbutton->setMaximumHeight(20);
      fillpatternbutton->setName("SolidPattern");
      fillpatternbutton->setPixmap(QgsSymbologyUtils::char2PatternPixmap("SolidPattern"));
      m_gridlayout->addWidget(fillpatternbutton, i, 7);
      m_widgetvector[8 * (i - 1) + 7] = fillpatternbutton;
      QObject::connect(fillpatternbutton, SIGNAL(clicked()), this, SLOT(selectFillPattern()));




      //apply a nice color range from blue to red as default
      if (i == 1)
        {
          if (m_type == QGis::Line)
            {
              outlinecolorbutton->setPaletteBackgroundColor(QColor(0, 0, 255));
          } else                //point or polygon
            {
              fillcolorbutton->setPaletteBackgroundColor(QColor(0, 0, 255));
            }
      } else
        {
          if (m_type == QGis::Line)
            {
              outlinecolorbutton->
                setPaletteBackgroundColor(QColor(255 / m_numberofclasses * i, 0, 255 - (255 / m_numberofclasses * i)));
          } else                //point or polygon
            {
              fillcolorbutton->setPaletteBackgroundColor(QColor(255 / m_numberofclasses * i, 0, 255 - (255 / m_numberofclasses * i)));
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
      if (m_mode == QgsGraSyDialog::EQUAL_INTERVAL)
        {
          ltextfield->setText(QString::number(minimum + (maximum - minimum) / m_numberofclasses * (i - 1), 'f', 2));
          utextfield->setText(QString::number(minimum + (maximum - minimum) / m_numberofclasses * i, 'f', 2));
        }
    }
}

QgsGraSyExtensionWidget::QgsGraSyExtensionWidget()
{
#ifdef DEBUG
  qWarning("constructor QgsGraSyExtensionWidget");
#endif
}

QgsGraSyExtensionWidget::~QgsGraSyExtensionWidget()
{
#ifdef DEBUG
  qWarning("destructor QgsGraSyExtensionWidget");
#endif
}

QWidget *QgsGraSyExtensionWidget::getWidget(int column, int row)
{
  return m_widgetvector[8 * row + column];
}

void QgsGraSyExtensionWidget::selectColor()
{
  ((QPushButton *) sender())->setPaletteBackgroundColor(QColorDialog::getColor());
  m_vectorlayer->propertiesDialog()->raise();
  m_vectorlayer->rendererDialog()->raise();
  raise();
}

void QgsGraSyExtensionWidget::selectFillPattern()
{
  QgsPatternDialog patterndialog;
  if (patterndialog.exec() == QDialog::Accepted)
    {
      ((QPushButton *) sender())->setName(QgsSymbologyUtils::brushStyle2Char(patterndialog.pattern()));
      ((QPushButton *) sender())->setPixmap(QgsSymbologyUtils::brushStyle2Pixmap(patterndialog.pattern()));
    }
  m_vectorlayer->propertiesDialog()->raise();
  m_vectorlayer->rendererDialog()->raise();
  raise();
}

void QgsGraSyExtensionWidget::selectOutlineStyle()
{
  QgsLineStyleDialog linestyledialog;
  if (linestyledialog.exec() == QDialog::Accepted)
    {
      ((QPushButton *) sender())->setName(QgsSymbologyUtils::penStyle2Char(linestyledialog.style()));
      ((QPushButton *) sender())->setPixmap(QgsSymbologyUtils::penStyle2Pixmap(linestyledialog.style()));
    }
  m_vectorlayer->propertiesDialog()->raise();
  m_vectorlayer->rendererDialog()->raise();
  raise();
}
