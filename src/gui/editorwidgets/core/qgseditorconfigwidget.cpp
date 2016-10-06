/***************************************************************************
    qgseditorconfigwidget.cpp
     --------------------------------------
    Date                 : 24.4.2013
    Copyright            : (C) 2013 Matthias Kuhn
    Email                : matthias at opengis dot ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgseditorconfigwidget.h"


QgsEditorConfigWidget::QgsEditorConfigWidget( QgsVectorLayer* vl, int fieldIdx, QWidget* parent )
    : QWidget( parent )
    , mLayer( vl )
    , mField( fieldIdx )

{
}

int QgsEditorConfigWidget::field()
{
  return mField;
}

QgsVectorLayer*QgsEditorConfigWidget::layer()
{
  return mLayer;
}

