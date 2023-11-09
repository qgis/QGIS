/***************************************************************************
    qgsdatetimeeditfactory.cpp
     --------------------------------------
    Date                 : 03.2014
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

#include "qgsdatetimeeditfactory.h"
#include "qgsdatetimeeditconfig.h"
#include "qgsdatetimeeditwrapper.h"
#include "qgsdatetimesearchwidgetwrapper.h"
#include "qgsdatetimeedit.h"

#include <QSettings>

QgsDateTimeEditFactory::QgsDateTimeEditFactory( const QString &name )
  : QgsEditorWidgetFactory( name )
{
}

QgsEditorWidgetWrapper *QgsDateTimeEditFactory::create( QgsVectorLayer *vl, int fieldIdx, QWidget *editor, QWidget *parent ) const
{
  return new QgsDateTimeEditWrapper( vl, fieldIdx, editor, parent );
}

QgsSearchWidgetWrapper *QgsDateTimeEditFactory::createSearchWidget( QgsVectorLayer *vl, int fieldIdx, QWidget *parent ) const
{
  return new QgsDateTimeSearchWidgetWrapper( vl, fieldIdx, parent );
}

QgsEditorConfigWidget *QgsDateTimeEditFactory::configWidget( QgsVectorLayer *vl, int fieldIdx, QWidget *parent ) const
{
  return new QgsDateTimeEditConfig( vl, fieldIdx, parent );
}

QHash<const char *, int> QgsDateTimeEditFactory::supportedWidgetTypes()
{
  QHash<const char *, int> map = QHash<const char *, int>();
  map.insert( QDateTimeEdit::staticMetaObject.className(), 10 );
  map.insert( QgsDateTimeEdit::staticMetaObject.className(), 10 );
  return map;
}

unsigned int QgsDateTimeEditFactory::fieldScore( const QgsVectorLayer *vl, int fieldIdx ) const
{
  const QgsField field = vl->fields().field( fieldIdx );
  const QVariantMap config = field.editorWidgetSetup().config();
  if ( field.isDateOrTime() || config.contains( QStringLiteral( "field_format" ) ) )
  {
    return 20;
  }
  else
  {
    return 5;
  }
}
