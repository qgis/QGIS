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
#include "qgscolorbuttonplugin.h"
#include "qgscolorbuttonv2plugin.h"
#include "qgsdatadefinedbuttonplugin.h"
#include "qgsdatetimeeditplugin.h"
#include "qgsdoublespinboxplugin.h"
#include "qgsfieldcomboboxplugin.h"
#include "qgsfieldexpressionwidgetplugin.h"
#include "qgsmaplayercomboboxplugin.h"
#include "qgsrelationeditorwidgetplugin.h"
#include "qgsrelationreferencewidgetplugin.h"
#include "qgsscalerangewidgetplugin.h"
#include "qgsspinboxplugin.h"


QgisCustomWidgets::QgisCustomWidgets( QObject *parent )
    : QObject( parent )
{
  mWidgets.append( new QgsCollapsibleGroupBoxPlugin );
  mWidgets.append( new QgsColorButtonPlugin );
  mWidgets.append( new QgsColorButtonV2Plugin );
  mWidgets.append( new QgsDataDefinedButtonPlugin );
  mWidgets.append( new QgsDateTimeEditPlugin );
  mWidgets.append( new QgsDoubleSpinBoxPlugin );
  mWidgets.append( new QgsFieldComboBoxPlugin );
  mWidgets.append( new QgsFieldExpressionWidgetPlugin );
  mWidgets.append( new QgsMapLayerComboBoxPlugin );
  mWidgets.append( new QgsRelationEditorWidgetPlugin );
  mWidgets.append( new QgsRelationReferenceWidgetPlugin );
  mWidgets.append( new QgsScaleRangeWidgetPlugin );
  mWidgets.append( new QgsSpinBoxPlugin );
}

QList<QDesignerCustomWidgetInterface*> QgisCustomWidgets::customWidgets() const
{
  return mWidgets;
}

Q_EXPORT_PLUGIN2( customwidgetsplugin, QgisCustomWidgets )
