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
#include "qgsdlgvectorlayerproperties.h"
#include "qgsdataprovider.h"
#include "qgsfield.h"
#include <qspinbox.h>

QgsContColDialog::QgsContColDialog(QgsVectorLayer * layer):QgsContColDialogBase(), m_vectorlayer(layer)
{
#ifdef QGISDEBUG
  qWarning("constructor QgsContColDialog");
#endif

  QObject::connect(mincolorbutton, SIGNAL(clicked()), this, SLOT(selectMinimumColor()));
  QObject::connect(maxcolorbutton, SIGNAL(clicked()), this, SLOT(selectMaximumColor()));

  //find out the numerical fields of m_vectorlayer
  QgsDataProvider *provider;
  if (provider = m_vectorlayer->getDataProvider())
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
              m_fieldmap.insert(std::make_pair(str, fieldnumber));
            }
          fieldnumber++;
        }
  } else
    {
      qWarning("Warning, data provider is null in QgsContColDialog::QgsContColDialog(...)");
      return;
    }

  //restore the correct colors for minimum and maximum values

  QgsContinuousColRenderer *renderer;

  if (m_vectorlayer->propertiesDialog())
    {
      renderer = dynamic_cast < QgsContinuousColRenderer * >(layer->propertiesDialog()->getBufferRenderer());
  } else
    {
      renderer = dynamic_cast < QgsContinuousColRenderer * >(layer->renderer());
    }

  if (renderer)
    {
      QgsRenderItem *minitem = renderer->minimumItem();
      QgsRenderItem *maxitem = renderer->maximumItem();
      if (m_vectorlayer->vectorType() == QGis::Line)
        {
          mincolorbutton->setPaletteBackgroundColor(minitem->getSymbol()->pen().color());
          maxcolorbutton->setPaletteBackgroundColor(maxitem->getSymbol()->pen().color());
      } else
        {
          mincolorbutton->setPaletteBackgroundColor(minitem->getSymbol()->brush().color());
          maxcolorbutton->setPaletteBackgroundColor(maxitem->getSymbol()->brush().color());
        }
      outlinewidthspinbox->setValue(minitem->getSymbol()->pen().width());
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
  std::map < QString, int >::iterator iter = m_fieldmap.find(fieldstring);
  int classfield = iter->second;

  //find the minimum and maximum for the classification variable
  double minimum, maximum;
  QgsDataProvider *provider = m_vectorlayer->getDataProvider();
  if (provider)
    {
      minimum = provider->minValue(classfield).toDouble();
      maximum = provider->maxValue(classfield).toDouble();
  } else
    {
      qWarning("Warning, provider is null in QgsGraSyExtensionWidget::QgsGraSyExtensionWidget(...)");
      return;
    }


  //create the render items for minimum and maximum value
  QgsSymbol minsymbol;
  if (m_vectorlayer->vectorType() == QGis::Line)
    {
      minsymbol.setPen(QPen(mincolorbutton->paletteBackgroundColor(),outlinewidthspinbox->value()));
    } 
  else
    {
      minsymbol.setBrush(QBrush(mincolorbutton->paletteBackgroundColor()));
      minsymbol.setPen(QPen(QColor(0, 0, 0), outlinewidthspinbox->value()));
    }
  QgsRenderItem *minimumitem = new QgsRenderItem(minsymbol, QString::number(minimum, 'f'), " ");


  QgsSymbol maxsymbol;
  if (m_vectorlayer->vectorType() == QGis::Line)
    {
      maxsymbol.setPen(QPen(maxcolorbutton->paletteBackgroundColor(),outlinewidthspinbox->value()));
    } 
  else
    {
      maxsymbol.setBrush(QBrush(maxcolorbutton->paletteBackgroundColor()));
      maxsymbol.setPen(QPen(QColor(0, 0, 0), outlinewidthspinbox->value()));
    }
  QgsRenderItem *maximumitem = new QgsRenderItem(maxsymbol, QString::number(maximum, 'f'), " ");

  //set the render items to the buffer renderer of the property dialog (if there is one)
  QgsContinuousColRenderer *renderer;
  if (m_vectorlayer->propertiesDialog())
    {
      renderer = dynamic_cast < QgsContinuousColRenderer * >(m_vectorlayer->propertiesDialog()->getBufferRenderer());
  } else
    {
      renderer = dynamic_cast < QgsContinuousColRenderer * >(m_vectorlayer->renderer());
    }

  if (renderer)
    {
      renderer->setMinimumItem(minimumitem);
      renderer->setMaximumItem(maximumitem);
      renderer->setClassificationField(classfield);
  } else
    {
      qWarning("Warning, typecast failed in QgsContColDialog::apply()");
      return;
    }

  //add a pixmap to the legend item

  //font tor the legend text
  //TODO Make the font a user option
  QFont f("times", 12, QFont::Normal);
  QFontMetrics fm(f);

  //spaces in pixel
  int topspace = 5;             //space between top of pixmap and first row
  int leftspace = 10;           //space between left side and text/graphics
  int rightspace = 5;           //space betwee text/graphics and right side
  int bottomspace = 5;          //space between last row and bottom of the pixmap
  int gradientwidth = 40;       //widht of the gradient
  int gradientheight = 100;     //height of the gradient
  int wordspace = 10;           //space between graphics/word

  //add a pixmap to the QgsLegendItem
  QPixmap *pix = m_vectorlayer->legendPixmap();
  //use the name and the maximum value to estimate the necessary width of the pixmap
  QString name;
  if (m_vectorlayer->propertiesDialog())
    {
      name = m_vectorlayer->propertiesDialog()->displayName();
  } else
    {
      name = "";
    }
  int namewidth = fm.width(name);
  int numberlength = gradientwidth + wordspace + fm.width(QString::number(maximum, 'f', 2));
  int pixwidth = (numberlength > namewidth) ? numberlength : namewidth;
  pix->resize(leftspace + pixwidth + rightspace, topspace + 2 * fm.height() + gradientheight + bottomspace);
  pix->fill();
  QPainter p(pix);

  p.setPen(QPen(QColor(0, 0, 0), 1));
  p.setFont(f);
  //draw the layer name and the name of the classification field into the pixmap
  p.drawText(leftspace, topspace + fm.height(), name);
  p.drawText(leftspace, topspace + fm.height() * 2, classificationComboBox->currentText());

  int rangeoffset = topspace + fm.height() * 2;

  //draw the color range line by line
  for (int i = 0; i < gradientheight; i++)
    {
      p.setPen(QColor(mincolorbutton->paletteBackgroundColor().red() + (maxcolorbutton->paletteBackgroundColor().red() - mincolorbutton->paletteBackgroundColor().red()) / gradientheight * i, mincolorbutton->paletteBackgroundColor().green() + (maxcolorbutton->paletteBackgroundColor().green() - mincolorbutton->paletteBackgroundColor().green()) / gradientheight * i, mincolorbutton->paletteBackgroundColor().blue() + (maxcolorbutton->paletteBackgroundColor().blue() - mincolorbutton->paletteBackgroundColor().blue()) / gradientheight * i)); //use the appropriate color
      p.drawLine(leftspace, rangeoffset + i, leftspace + gradientwidth, rangeoffset + i);
    }

  //draw the minimum and maximum values beside the color range
  p.setPen(QPen(QColor(0, 0, 0)));
  p.setFont(f);
  p.drawText(leftspace + gradientwidth + wordspace, rangeoffset + fm.height(), QString::number(minimum, 'f', 2));
  p.drawText(leftspace + gradientwidth + wordspace, rangeoffset + gradientheight, QString::number(maximum, 'f', 2));

  if (m_vectorlayer->legendItem())
    {
      m_vectorlayer->legendItem()->setPixmap(0, (*pix));
    }

  m_vectorlayer->setRenderer(renderer);
  m_vectorlayer->setRendererDialog(this);
  if (m_vectorlayer->propertiesDialog())
    {
      m_vectorlayer->propertiesDialog()->setRendererDirty(false);
    }

  m_vectorlayer->triggerRepaint();
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
