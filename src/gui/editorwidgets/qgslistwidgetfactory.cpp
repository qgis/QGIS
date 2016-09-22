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
#include "qgsdummyconfigdlg.h"
#include "qgsfields.h"
#include "qgsvectorlayer.h"
#include "qgseditorwidgetregistry.h"

#include <QVariant>
#include <QSettings>

QgsListWidgetFactory::QgsListWidgetFactory( const QString& name ):
    QgsEditorWidgetFactory( name )
{
}

QgsEditorWidgetWrapper* QgsListWidgetFactory::create( QgsVectorLayer *vl, int fieldIdx, QWidget *editor, QWidget *parent ) const
{
  return new QgsListWidgetWrapper( vl, fieldIdx, editor, parent );
}

QgsEditorConfigWidget* QgsListWidgetFactory::configWidget( QgsVectorLayer *vl, int fieldIdx, QWidget *parent ) const
{
  Q_UNUSED( vl );
  Q_UNUSED( fieldIdx );
  Q_UNUSED( parent );
  return new QgsDummyConfigDlg( vl, fieldIdx, parent, QObject::tr( "List field" ) );
}

QgsEditorWidgetConfig QgsListWidgetFactory::readConfig( const QDomElement &configElement, QgsVectorLayer *layer, int fieldIdx )
{
  Q_UNUSED( configElement );
  Q_UNUSED( layer );
  Q_UNUSED( fieldIdx );
  return QgsEditorWidgetConfig();
}

void QgsListWidgetFactory::writeConfig( const QgsEditorWidgetConfig& config, QDomElement& configElement, QDomDocument& doc, const QgsVectorLayer* layer, int fieldIdx )
{
  Q_UNUSED( config );
  Q_UNUSED( configElement );
  Q_UNUSED( doc );
  Q_UNUSED( layer );
  Q_UNUSED( fieldIdx );
}

QString QgsListWidgetFactory::representValue( QgsVectorLayer* vl, int fieldIdx, const QgsEditorWidgetConfig& config, const QVariant& cache, const QVariant& value ) const
{
  Q_UNUSED( vl );
  Q_UNUSED( fieldIdx );
  Q_UNUSED( config );
  Q_UNUSED( cache );

  if ( value.isNull() )
  {
    QSettings settings;
    return settings.value( "qgis/nullValue", "NULL" ).toString();
  }

  QString result;
  const QVariantList list = value.toList();
  for ( QVariantList::const_iterator i = list.constBegin(); i != list.constEnd(); ++i )
  {
    if ( !result.isEmpty() ) result.append( ", " );
    result.append( i->toString() );
  }
  return result;
}

Qt::AlignmentFlag QgsListWidgetFactory::alignmentFlag( QgsVectorLayer *vl, int fieldIdx, const QgsEditorWidgetConfig &config ) const
{
  Q_UNUSED( vl );
  Q_UNUSED( fieldIdx );
  Q_UNUSED( config );

  return Qt::AlignLeft;
}

unsigned int QgsListWidgetFactory::fieldScore( const QgsVectorLayer* vl, int fieldIdx ) const
{
  const QgsField field = vl->fields().field( fieldIdx );
  return ( field.type() == QVariant::List || field.type() == QVariant::StringList ) && field.subType() != QVariant::Invalid ? 20 : 0;
}