/***************************************************************************
    qgsphotowidgetfactory.cpp
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

#include "qgsphotowidgetfactory.h"

#include "qgsphotowidgetwrapper.h"
#include "qgsphotoconfigdlg.h"

QgsPhotoWidgetFactory::QgsPhotoWidgetFactory( const QString& name )
    :  QgsEditorWidgetFactory( name )
{
}


QgsEditorWidgetWrapper* QgsPhotoWidgetFactory::create( QgsVectorLayer* vl, int fieldIdx, QWidget* editor, QWidget* parent ) const
{
  return new QgsPhotoWidgetWrapper( vl, fieldIdx, editor, parent );
}

QgsEditorConfigWidget* QgsPhotoWidgetFactory::configWidget( QgsVectorLayer* vl, int fieldIdx, QWidget* parent ) const
{
  return new QgsPhotoConfigDlg( vl, fieldIdx, parent );
}

QgsEditorWidgetConfig QgsPhotoWidgetFactory::readConfig( const QDomElement& configElement, QgsVectorLayer* layer, int fieldIdx )
{
  Q_UNUSED( layer )
  Q_UNUSED( fieldIdx )

  QgsEditorWidgetConfig cfg;

  cfg.insert( QStringLiteral( "Height" ), configElement.attribute( QStringLiteral( "Height" ), 0 ).toInt() );
  cfg.insert( QStringLiteral( "Width" ), configElement.attribute( QStringLiteral( "Width" ), 0 ).toInt() );

  return cfg;
}

void QgsPhotoWidgetFactory::writeConfig( const QgsEditorWidgetConfig& config, QDomElement& configElement, QDomDocument& doc, const QgsVectorLayer* layer, int fieldIdx )
{
  Q_UNUSED( doc )
  Q_UNUSED( layer )
  Q_UNUSED( fieldIdx )

  configElement.setAttribute( QStringLiteral( "Height" ), config.value( QStringLiteral( "Height" ), 0 ).toString() );
  configElement.setAttribute( QStringLiteral( "Width" ), config.value( QStringLiteral( "Width" ), 0 ).toString() );
}
