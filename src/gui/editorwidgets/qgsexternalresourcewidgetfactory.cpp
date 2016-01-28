/***************************************************************************
   qgsexternalresourcewidgetfactory.cpp

 ---------------------
 begin                : 16.12.2015
 copyright            : (C) 2015 by Denis Rouzaud
 email                : denis.rouzaud@gmail.com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsexternalresourcewidgetfactory.h"
#include "qgsfilewidget.h"

QgsExternalResourceWidgetFactory::QgsExternalResourceWidgetFactory( const QString& name )
    : QgsEditorWidgetFactory( name )
{
}

QgsEditorWidgetWrapper* QgsExternalResourceWidgetFactory::create( QgsVectorLayer* vl, int fieldIdx, QWidget* editor, QWidget* parent ) const
{
  return new QgsExternalResourceWidgetWrapper( vl, fieldIdx, editor, parent );
}

QgsEditorConfigWidget* QgsExternalResourceWidgetFactory::configWidget( QgsVectorLayer* vl, int fieldIdx, QWidget* parent ) const
{
  return new QgsExternalResourceConfigDlg( vl, fieldIdx, parent );
}

void QgsExternalResourceWidgetFactory::writeConfig( const QgsEditorWidgetConfig& config, QDomElement& configElement, QDomDocument& doc, const QgsVectorLayer* layer, int fieldIdx )
{
  Q_UNUSED( doc )
  Q_UNUSED( layer )
  Q_UNUSED( fieldIdx )

  configElement.setAttribute( "FileWidget", config.value( "FileWidget", true ).toBool() );
  configElement.setAttribute( "FileWidgetButton", config.value( "FileWidgetButton", true ).toBool() );


  // Non mandatory options are not saved into project file (to save some space).
  if ( config.contains( "UseLink" ) )
    configElement.setAttribute( "UseLink", config.value( "UseLink" ).toBool() );

  if ( config.contains( "FullUrl" ) )
    configElement.setAttribute( "FullUrl", config.value( "FullUrl" ).toBool() );

  if ( config.contains( "DefaultRoot" ) )
    configElement.setAttribute( "DefaultRoot", config.value( "DefaultRoot" ).toString() );

  if ( config.contains( "RelativeStorage" ) )
    configElement.setAttribute( "RelativeStorage" , config.value( "RelativeStorage" ).toString() );

  if ( config.contains( "DocumentViewer" ) )
    configElement.setAttribute( "DocumentViewer", config.value( "DocumentViewer" ).toInt() );

  if ( config.contains( "DocumentViewerWidth" ) )
    configElement.setAttribute( "DocumentViewerWidth", config.value( "DocumentViewerWidth" ).toInt() );

  if ( config.contains( "DocumentViewerHeight" ) )
    configElement.setAttribute( "DocumentViewerHeight", config.value( "DocumentViewerHeight" ).toInt() );

  if ( config.contains( "FileWidgetFilter" ) )
    configElement.setAttribute( "FileWidgetFilter", config.value( "FileWidgetFilter" ).toString() );

  configElement.setAttribute( "StorageMode", config.value( "StorageMode" ).toString() );
}

QgsEditorWidgetConfig QgsExternalResourceWidgetFactory::readConfig( const QDomElement& configElement, QgsVectorLayer* layer, int fieldIdx )
{
  Q_UNUSED( layer )
  Q_UNUSED( fieldIdx )

  QgsEditorWidgetConfig cfg;

  if ( configElement.hasAttribute( "FileWidgetButton" ) )
    cfg.insert( "FileWidgetButton", configElement.attribute( "FileWidgetButton" ) == "1" );

  if ( configElement.hasAttribute( "FileWidget" ) )
    cfg.insert( "FileWidget", configElement.attribute( "FileWidget" ) == "1" );

  if ( configElement.hasAttribute( "UseLink" ) )
    cfg.insert( "UseLink", configElement.attribute( "UseLink" ) == "1" );

  if ( configElement.hasAttribute( "FullUrl" ) )
    cfg.insert( "FullUrl", configElement.attribute( "FullUrl" ) == "1" );

  if ( configElement.hasAttribute( "DefaultRoot" ) )
    cfg.insert( "DefaultRoot", configElement.attribute( "DefaultRoot" ) );

  if ( configElement.hasAttribute( "RelativeStorage" ) )
  {
    if (( configElement.attribute( "RelativeStorage" ).toInt() == QgsFileWidget::RelativeDefaultPath && configElement.hasAttribute( "DefaultRoot" ) ) ||
        configElement.attribute( "RelativeStorage" ).toInt() == QgsFileWidget::RelativeProject )
      cfg.insert( "RelativeStorage" , configElement.attribute( "RelativeStorage" ).toInt() );
  }

  if ( configElement.hasAttribute( "DocumentViewer" ) )
    cfg.insert( "DocumentViewer", configElement.attribute( "DocumentViewer" ) );

  if ( configElement.hasAttribute( "DocumentViewerWidth" ) )
    cfg.insert( "DocumentViewerWidth", configElement.attribute( "DocumentViewerWidth" ) );

  if ( configElement.hasAttribute( "DocumentViewerHeight" ) )
    cfg.insert( "DocumentViewerHeight", configElement.attribute( "DocumentViewerHeight" ) );

  if ( configElement.hasAttribute( "FileWidgetFilter" ) )
    cfg.insert( "FileWidgetFilter", configElement.attribute( "FileWidgetFilter" ) );


  cfg.insert( "StorageMode", configElement.attribute( "StorageMode", "Files" ) );

  return cfg;
}

bool QgsExternalResourceWidgetFactory::isFieldSupported( QgsVectorLayer* vl, int fieldIdx )
{
  if ( vl->fields().at( fieldIdx ).type() == QVariant::String )
    return true;

  return false;
}
