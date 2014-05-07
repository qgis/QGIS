/***************************************************************************
   qgscustomwidgets.cpp
    --------------------------------------
   Date                 : 25.04.2014
   Copyright            : (C) 2014 Denis Rouzaud
   Email                : denis.rouzaud@gmail.com
***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#include "qplugin.h"

#include "qgiscustomwidgets.h"

#include "qgscollapsiblegroupboxplugin.h"
#include "qgsfieldcomboboxplugin.h"
#include "qgsfieldexpressionwidgetplugin.h"
#include "qgsmaplayercomboboxplugin.h"
#include "qgsscalerangewidgetplugin.h"


QgisCustomWidgets::QgisCustomWidgets( QObject *parent )
    : QObject( parent )
{

  // !!!!!!!!!!!!!!!!!!!!!
  // do not forget to add the corresponding line in python/customwidgets.py
  // to make the custom widget available from python
  // !!!!!!!!!!!!!!!!!!!!!

  mWidgets.append( new QgsCollapsibleGroupBoxPlugin );
  mWidgets.append( new QgsFieldComboBoxPlugin );
  mWidgets.append( new QgsFieldExpressionWidgetPlugin );
  mWidgets.append( new QgsMapLayerComboBoxPlugin );
  mWidgets.append( new QgsScaleRangeWidgetPlugin );
}

QList<QDesignerCustomWidgetInterface*> QgisCustomWidgets::customWidgets() const
{
  return mWidgets;
}

Q_EXPORT_PLUGIN2( customwidgetsplugin, QgisCustomWidgets )
