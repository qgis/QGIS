/***************************************************************************
    qgsrangewidgetfactory.cpp
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

#include "qgsrangewidgetfactory.h"
#include "qgsrangeconfigdlg.h"
#include "qgsrangewidgetwrapper.h"
#include "qgsvectorlayer.h"
#include <QDial>

QgsRangeWidgetFactory::QgsRangeWidgetFactory( const QString &name )
  : QgsEditorWidgetFactory( name )
{
}

QgsEditorWidgetWrapper *QgsRangeWidgetFactory::create( QgsVectorLayer *vl, int fieldIdx, QWidget *editor, QWidget *parent ) const
{
  return new QgsRangeWidgetWrapper( vl, fieldIdx, editor, parent );
}

QgsEditorConfigWidget *QgsRangeWidgetFactory::configWidget( QgsVectorLayer *vl, int fieldIdx, QWidget *parent ) const
{
  return new QgsRangeConfigDlg( vl, fieldIdx, parent );
}

unsigned int QgsRangeWidgetFactory::fieldScore( const QgsVectorLayer *vl, int fieldIdx ) const
{
  const QgsField field = vl->fields().at( fieldIdx );
  if ( field.type() == QMetaType::Type::Int )
    return 20;
  if ( field.type() == QMetaType::Type::Double )
    return 5; // low priority because the fixed number of decimal places may alter the original data
  if ( field.isNumeric() )
    return 5; // widgets used support only signed 32bits (int) and double
  return 0;
}

QHash<const char *, int> QgsRangeWidgetFactory::supportedWidgetTypes()
{
  QHash<const char *, int> map = QHash<const char *, int>();
  map.insert( QSlider::staticMetaObject.className(), 10 );
  map.insert( QDial::staticMetaObject.className(), 10 );
  map.insert( QSpinBox::staticMetaObject.className(), 10 );
  map.insert( QDoubleSpinBox::staticMetaObject.className(), 10 );
  return map;
}
