/***************************************************************************
    qgsvaluerelationwidgetfactory.cpp
     --------------------------------------
    Date                 : 5.1.2014
    Copyright            : (C) 2014 Matthias Kuhn
    Email                : matthias at opengis dot ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsvaluerelationwidgetfactory.h"

#include "qgsfeatureiterator.h"
#include "qgslogger.h"
#include "qgsvaluerelationconfigdlg.h"
#include "qgsvaluerelationsearchwidgetwrapper.h"
#include "qgsvectorlayer.h"
#include "qgsvaluerelationwidgetwrapper.h"

#include <QSettings>

QgsValueRelationWidgetFactory::QgsValueRelationWidgetFactory( const QString &name )
  : QgsEditorWidgetFactory( name )
{
}

QgsEditorWidgetWrapper *QgsValueRelationWidgetFactory::create( QgsVectorLayer *vl, int fieldIdx, QWidget *editor, QWidget *parent ) const
{
  return new QgsValueRelationWidgetWrapper( vl, fieldIdx, editor, parent );
}

QgsSearchWidgetWrapper *QgsValueRelationWidgetFactory::createSearchWidget( QgsVectorLayer *vl, int fieldIdx, QWidget *parent ) const
{
  return new QgsValueRelationSearchWidgetWrapper( vl, fieldIdx, parent );
}

QgsEditorConfigWidget *QgsValueRelationWidgetFactory::configWidget( QgsVectorLayer *vl, int fieldIdx, QWidget *parent ) const
{
  return new QgsValueRelationConfigDlg( vl, fieldIdx, parent );
}
