/***************************************************************************
    qgsvaluemapwidgetfactory.cpp
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

#include "qgsvaluemapwidgetfactory.h"

#include "qgsvaluemapwidgetwrapper.h"
#include "qgsvaluemapsearchwidgetwrapper.h"
#include "qgsdefaultsearchwidgetwrapper.h"
#include "qgsvaluemapconfigdlg.h"

#include <QSettings>

QgsValueMapWidgetFactory::QgsValueMapWidgetFactory( const QString &name )
  : QgsEditorWidgetFactory( name )
{
}


QgsEditorWidgetWrapper *QgsValueMapWidgetFactory::create( QgsVectorLayer *vl, int fieldIdx, QWidget *editor, QWidget *parent ) const
{
  return new QgsValueMapWidgetWrapper( vl, fieldIdx, editor, parent );
}


QgsSearchWidgetWrapper *QgsValueMapWidgetFactory::createSearchWidget( QgsVectorLayer *vl, int fieldIdx, QWidget *parent ) const
{
  return new QgsValueMapSearchWidgetWrapper( vl, fieldIdx, parent );
}

QgsEditorConfigWidget *QgsValueMapWidgetFactory::configWidget( QgsVectorLayer *vl, int fieldIdx, QWidget *parent ) const
{
  return new QgsValueMapConfigDlg( vl, fieldIdx, parent );
}

QHash<const char *, int> QgsValueMapWidgetFactory::supportedWidgetTypes()
{
  QHash<const char *, int> map = QHash<const char *, int>();
  map.insert( QComboBox::staticMetaObject.className(), 10 );
  return map;
}
