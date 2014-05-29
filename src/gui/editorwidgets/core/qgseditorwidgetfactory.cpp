/***************************************************************************
    qgseditorwidgetfactory.cpp
     --------------------------------------
    Date                 : 21.4.2013
    Copyright            : (C) 2013 Matthias Kuhn
    Email                : matthias dot kuhn at gmx dot ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgseditorwidgetfactory.h"

#include <QSettings>

QgsEditorWidgetFactory::QgsEditorWidgetFactory( const QString& name )
    : mName( name )
{
}

QgsEditorWidgetFactory::~QgsEditorWidgetFactory()
{
}

QString QgsEditorWidgetFactory::name()
{
  return mName;
}

QgsEditorWidgetConfig QgsEditorWidgetFactory::readEditorConfig( const QDomElement& configElement, QgsVectorLayer* layer, int fieldIdx )
{
  return readConfig( configElement, layer, fieldIdx );
}

void QgsEditorWidgetFactory::writeConfig( const QgsEditorWidgetConfig& config, QDomElement& configElement, QDomDocument& doc, const QgsVectorLayer* layer, int fieldIdx )
{
  Q_UNUSED( config );
  Q_UNUSED( configElement );
  Q_UNUSED( doc );
  Q_UNUSED( layer );
  Q_UNUSED( fieldIdx );
}

QString QgsEditorWidgetFactory::representValue( QgsVectorLayer* vl, int fieldIdx, const QgsEditorWidgetConfig& config, const QVariant& cache, const QVariant& value ) const
{
  Q_UNUSED( vl )
  Q_UNUSED( fieldIdx )
  Q_UNUSED( config )
  Q_UNUSED( cache )
  Q_UNUSED( value )

  const QgsField &fld = vl->pendingFields().at( fieldIdx );
  return fld.displayString( value );
}

QVariant QgsEditorWidgetFactory::createCache( QgsVectorLayer* vl, int fieldIdx, const QgsEditorWidgetConfig& config )
{
  Q_UNUSED( vl )
  Q_UNUSED( fieldIdx )
  Q_UNUSED( config )

  return QVariant();
}

QgsEditorWidgetConfig QgsEditorWidgetFactory::readConfig( const QDomElement& configElement, QgsVectorLayer* layer, int fieldIdx )
{
  Q_UNUSED( configElement );
  Q_UNUSED( layer );
  Q_UNUSED( fieldIdx );

  return QgsEditorWidgetConfig();
}

bool QgsEditorWidgetFactory::isFieldSupported( QgsVectorLayer* vl, int fieldIdx )
{
  Q_UNUSED( vl )
  Q_UNUSED( fieldIdx )
  return true;
}

