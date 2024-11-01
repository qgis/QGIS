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
#include "moc_qgiscustomwidgets.cpp"
#include "qgsauthconfigselectplugin.h"
#include "qgscheckablecomboboxplugin.h"
#include "qgscollapsiblegroupboxplugin.h"
#include "qgscolorbuttonplugin.h"
#include "qgsdatetimeeditplugin.h"
#include "qgsdockwidgetplugin.h"
#include "qgsdoublespinboxplugin.h"
#include "qgsexpressionbuilderwidgetplugin.h"
#include "qgsextentgroupboxplugin.h"
#include "qgsexternalresourcewidgetplugin.h"
#include "qgsfeaturelistcomboboxplugin.h"
#include "qgsfeaturepickerwidgetplugin.h"
#include "qgsfieldcomboboxplugin.h"
#include "qgsfieldexpressionwidgetplugin.h"
#include "qgsfilewidgetplugin.h"
#include "qgsfilterlineeditplugin.h"
#include "qgsfontbuttonplugin.h"
#include "qgsmaplayercomboboxplugin.h"
#include "qgsopacitywidgetplugin.h"
#include "qgspasswordlineeditplugin.h"
#include "qgsprojectionselectionwidgetplugin.h"
#include "qgspropertyoverridebuttonplugin.h"
#include "qgsrasterbandcomboboxplugin.h"
#include "qgsrelationeditorwidgetplugin.h"
#include "qgsrelationreferencewidgetplugin.h"
#include "qgsscalerangewidgetplugin.h"
#include "qgsscalewidgetplugin.h"
//#include "qgsscrollareawidgetplugin.h"
#include "qgsspinboxplugin.h"
#include "qgssymbolbuttonplugin.h"

QgisCustomWidgets::QgisCustomWidgets( QObject *parent )
  : QObject( parent )
{
  mWidgets.append( new QgsAuthConfigSelectPlugin( this ) );
  mWidgets.append( new QgsCheckableComboBoxPlugin( this ) );
  mWidgets.append( new QgsCollapsibleGroupBoxPlugin( this ) );
  mWidgets.append( new QgsColorButtonPlugin( this ) );
  mWidgets.append( new QgsDateTimeEditPlugin( this ) );
  mWidgets.append( new QgsDockWidgetPlugin( this ) );
  mWidgets.append( new QgsDoubleSpinBoxPlugin( this ) );
  mWidgets.append( new QgsExpressionBuilderWidgetPlugin( this ) );
  mWidgets.append( new QgsExtentGroupBoxPlugin( this ) );
  mWidgets.append( new QgsExternalResourceWidgetPlugin( this ) );
  mWidgets.append( new QgsFeatureListComboBoxPlugin( this ) );
  mWidgets.append( new QgsFeaturePickerWidgetPlugin( this ) );
  mWidgets.append( new QgsFieldComboBoxPlugin( this ) );
  mWidgets.append( new QgsFieldExpressionWidgetPlugin( this ) );
  mWidgets.append( new QgsFileWidgetPlugin( this ) );
  mWidgets.append( new QgsFilterLineEditPlugin( this ) );
  mWidgets.append( new QgsFontButtonPlugin( this ) );
  mWidgets.append( new QgsMapLayerComboBoxPlugin( this ) );
  mWidgets.append( new QgsOpacityWidgetPlugin( this ) );
  mWidgets.append( new QgsPasswordLineEditPlugin( this ) );
  mWidgets.append( new QgsProjectionSelectionWidgetPlugin( this ) );
  mWidgets.append( new QgsPropertyOverrideButtonPlugin( this ) );
  mWidgets.append( new QgsRasterBandComboBoxPlugin( this ) );
  mWidgets.append( new QgsRelationEditorWidgetPlugin( this ) );
  mWidgets.append( new QgsRelationReferenceWidgetPlugin( this ) );
  mWidgets.append( new QgsScaleRangeWidgetPlugin( this ) );
  mWidgets.append( new QgsScaleWidgetPlugin( this ) );
  //  mWidgets.append( new QgsScrollAreaWidgetPlugin( this ) ); // this is causing troubles at the moment
  mWidgets.append( new QgsSpinBoxPlugin( this ) );
  mWidgets.append( new QgsSymbolButtonPlugin( this ) );
}

QList<QDesignerCustomWidgetInterface *> QgisCustomWidgets::customWidgets() const
{
  return mWidgets;
}
