/***************************************************************************
                          qgslayerproperties.cpp  -  description
                             -------------------
    begin                : Sun Aug 11 2002
    copyright            : (C) 2002 by Gary E.Sherman
    email                : sherman at mrcc dot com
       Romans 3:23=>Romans 6:23=>Romans 5:8=>Romans 10:9,10=>Romans 12
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
#include <qframe.h>
#include <qcolordialog.h>
#include <qpushbutton.h>
#include <qlineedit.h>
#include <qspinbox.h>
#include <qstring.h>
#include <qlabel.h>
#include "qgsmaplayer.h"
#include "qgssymbol.h"
#include "qgslayerproperties.h"

QgsLayerProperties::QgsLayerProperties(QgsMapLayer * lyr):layer(lyr)
{
  // populate the property sheet based on the layer properties
  // general info
  QString source = lyr->source();
  source = source.left(source.find("password"));
  lblSource->setText(source);
  txtDisplayName->setText(lyr->name());
  //symbology
  sym = layer->symbol();
  newSym = *sym;

  btnSetColor->setPaletteBackgroundColor(sym->color());

  btnSetFillColor->setPaletteBackgroundColor(sym->fillColor());
  spinLineWidth->setValue(sym->lineWidth());
  setCaption("Layer Properties - " + lyr->name());
  // if this is a line layer, hide the fill properties
  if (lyr->featureType() == QGis::WKBMultiLineString || lyr->featureType() == QGis::WKBLineString)
    {
      lblFillColor->hide();
      btnSetFillColor->hide();

    }
}

QgsLayerProperties::~QgsLayerProperties()
{
}
void QgsLayerProperties::selectFillColor()
{

  QColor fc = QColorDialog::getColor(newSym.fillColor(), this);
  if (fc.isValid())
    {

      btnSetFillColor->setPaletteBackgroundColor(fc);
      newSym.setFillColor(fc);
    }
}
void QgsLayerProperties::selectOutlineColor()
{
  QColor oc = QColorDialog::getColor(newSym.color(), this);
  if (oc.isValid())
    {

      btnSetColor->setPaletteBackgroundColor(oc);
      newSym.setColor(oc);
    }
}

QString QgsLayerProperties::displayName()
{
  return txtDisplayName->text();
}

void QgsLayerProperties::setLineWidth(int w)
{
  newSym.setLineWidth(w);
}

QgsSymbol *QgsLayerProperties::getSymbol()
{
  return new QgsSymbol(newSym);
}
