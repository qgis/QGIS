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
#include "qgsfilterlineeditplugin.h"
#include "qgsmaplayercomboboxplugin.h"
#include "qgsprojectionselectionwidgetplugin.h"
#include "qgsrelationeditorwidgetplugin.h"
#include "qgsrelationreferencewidgetplugin.h"
#include "qgsscalerangewidgetplugin.h"
#include "qgsspinboxplugin.h"


QgisCustomWidgets::QgisCustomWidgets( QObject *parent )
    : QObject( parent )
{
  mWidgets.append( new QgsCollapsibleGroupBoxPlugin(this) );
  mWidgets.append( new QgsColorButtonPlugin(this) );
  mWidgets.append( new QgsColorButtonV2Plugin(this) );
  mWidgets.append( new QgsDataDefinedButtonPlugin(this) );
  mWidgets.append( new QgsDateTimeEditPlugin(this) );
  mWidgets.append( new QgsDoubleSpinBoxPlugin(this) );
  mWidgets.append( new QgsFieldComboBoxPlugin(this) );
  mWidgets.append( new QgsFieldExpressionWidgetPlugin(this) );
  mWidgets.append( new QgsFilterLineEditPlugin(this) );
  mWidgets.append( new QgsMapLayerComboBoxPlugin(this) );
  mWidgets.append( new QgsProjectionSelectionWidgetPlugin(this) );
  mWidgets.append( new QgsRelationEditorWidgetPlugin(this) );
  mWidgets.append( new QgsRelationReferenceWidgetPlugin(this) );
  mWidgets.append( new QgsScaleRangeWidgetPlugin(this) );
  mWidgets.append( new QgsSpinBoxPlugin(this) );
}

QList<QDesignerCustomWidgetInterface*> QgisCustomWidgets::customWidgets() const
{
  return mWidgets;
}

#if QT_VERSION < 0x050000
  Q_EXPORT_PLUGIN2( customwidgetsplugin, QgisCustomWidgets )
#endif
