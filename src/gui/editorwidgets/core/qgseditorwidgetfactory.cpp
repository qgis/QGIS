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

QgsEditorWidgetConfig QgsEditorWidgetFactory::readConfig( const QDomElement& configElement, QgsVectorLayer* layer, int fieldIdx )
{
  Q_UNUSED( configElement );
  Q_UNUSED( layer );
  Q_UNUSED( fieldIdx );

  return QgsEditorWidgetConfig();
}

void QgsEditorWidgetFactory::writeConfig( const QgsEditorWidgetConfig& config, QDomElement& configElement, const QDomDocument& doc, const QgsVectorLayer* layer, int fieldIdx )
{
  Q_UNUSED( config );
  Q_UNUSED( configElement );
  Q_UNUSED( doc );
  Q_UNUSED( layer );
  Q_UNUSED( fieldIdx );
}

