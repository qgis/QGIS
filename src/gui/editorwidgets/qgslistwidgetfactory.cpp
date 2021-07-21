/***************************************************************************
    qgslistwidgetfactory.cpp
     --------------------------------------
    Date                 : 09.2016
    Copyright            : (C) 2016 Patrick Valsecchi
    Email                : patrick.valsecchi@camptocamp.com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgslistwidgetfactory.h"
#include "qgslistwidgetwrapper.h"
#include "qgslistconfigdlg.h"
#include "qgsfields.h"
#include "qgsvectorlayer.h"
#include "qgseditorwidgetregistry.h"

#include <QVariant>
#include <QSettings>

QgsListWidgetFactory::QgsListWidgetFactory( const QString &name ):
  QgsEditorWidgetFactory( name )
{
}

QgsEditorWidgetWrapper *QgsListWidgetFactory::create( QgsVectorLayer *vl, int fieldIdx, QWidget *editor, QWidget *parent ) const
{
  return new QgsListWidgetWrapper( vl, fieldIdx, editor, parent );
}

QgsEditorConfigWidget *QgsListWidgetFactory::configWidget( QgsVectorLayer *vl, int fieldIdx, QWidget *parent ) const
{
  return new QgsListConfigDlg( vl, fieldIdx, parent );
}

unsigned int QgsListWidgetFactory::fieldScore( const QgsVectorLayer *vl, int fieldIdx ) const
{
  const QgsField field = vl->fields().field( fieldIdx );
  return ( field.type() == QVariant::List || field.type() == QVariant::StringList || field.type() == QVariant::Map ) && field.subType() != QVariant::Invalid ? 20 : 0;
}
