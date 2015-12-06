/***************************************************************************
    qgsfilenamewidgetfactory.cpp
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

#include "qgsfilenamewidgetfactory.h"

QgsFileNameWidgetFactory::QgsFileNameWidgetFactory( const QString& name )
  : QgsEditorWidgetFactory( name )
{
}

QgsEditorWidgetWrapper* QgsFileNameWidgetFactory::create( QgsVectorLayer* vl, int fieldIdx, QWidget* editor, QWidget* parent ) const
{
  return new QgsFileNameWidgetWrapper( vl, fieldIdx, editor, parent );
}

QgsEditorConfigWidget* QgsFileNameWidgetFactory::configWidget( QgsVectorLayer* vl, int fieldIdx, QWidget* parent ) const
{
  return new QgsFileNameConfigDlg( vl, fieldIdx, parent );
}

void QgsFileNameWidgetFactory::writeConfig( const QgsEditorWidgetConfig& config, QDomElement& configElement, QDomDocument& doc, const QgsVectorLayer* layer, int fieldIdx )
{
  Q_UNUSED( doc )
  Q_UNUSED( layer )
  Q_UNUSED( fieldIdx )

  // Non mandatory options are not saved into project file (to save some space).
  if ( config.contains( "UseLink" ) )
    configElement.setAttribute( "UseLink", config.value( "UseLink" ).toBool() );

  if ( config.contains( "FullUrl" ) )
    configElement.setAttribute( "FullUrl", config.value( "FullUrl" ).toBool() );
  
  if ( config.contains( "DefaultRoot" ) )
    configElement.setAttribute( "DefaultRoot", config.value( "DefaultRoot" ).toString() );
  
  if ( config.contains( "RelativeStorage" ) )
    configElement.setAttribute( "RelativeStorage" , config.value( "RelativeStorage" ).toString() );
  
  configElement.setAttribute( "StorageMode", config.value( "StorageMode" ).toString() );
}

QgsEditorWidgetConfig QgsFileNameWidgetFactory::readConfig( const QDomElement& configElement, QgsVectorLayer* layer, int fieldIdx )
{
  Q_UNUSED( layer )
  Q_UNUSED( fieldIdx )

  QgsEditorWidgetConfig cfg;

  if ( configElement.hasAttribute( "UseLink" ) )
    cfg.insert( "UseLink", configElement.attribute( "UseLink" ) == "1" );
  
  if ( configElement.hasAttribute( "FullUrl" ) )
    cfg.insert( "FullUrl", configElement.attribute( "FullUrl" ) == "1" );

  if ( configElement.hasAttribute( "DefaultRoot" ) )
    cfg.insert( "DefaultRoot", configElement.attribute( "DefaultRoot" ) );

  if ( configElement.hasAttribute( "RelativeStorage" ) )
  {
    if ( ( configElement.attribute( "RelativeStorage" ) == "Default" && configElement.hasAttribute( "DefaultRoot" ) ) ||
	 configElement.attribute( "RelativeStorage" ) == "Project" )
      cfg.insert( "RelativeStorage" , configElement.attribute( "RelativeStorage" ) );
  }
  
  cfg.insert( "StorageMode", configElement.attribute( "StorageMode", "Files" ) );
						      
  return cfg;
}

bool QgsFileNameWidgetFactory::isFieldSupported( QgsVectorLayer* vl, int fieldIdx )
{
  if ( vl->fields().at( fieldIdx ).type() == QVariant::String )
      return true;
  
  return false;
}
