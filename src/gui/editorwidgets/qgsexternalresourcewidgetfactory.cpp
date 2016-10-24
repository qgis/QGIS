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
#include "qgsexternalresourcewidgetwrapper.h"
#include "qgsexternalresourceconfigdlg.h"

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

  configElement.setAttribute( QStringLiteral( "FileWidget" ), config.value( QStringLiteral( "FileWidget" ), true ).toBool() );
  configElement.setAttribute( QStringLiteral( "FileWidgetButton" ), config.value( QStringLiteral( "FileWidgetButton" ), true ).toBool() );


  // Non mandatory options are not saved into project file (to save some space).
  if ( config.contains( QStringLiteral( "UseLink" ) ) )
    configElement.setAttribute( QStringLiteral( "UseLink" ), config.value( QStringLiteral( "UseLink" ) ).toBool() );

  if ( config.contains( QStringLiteral( "FullUrl" ) ) )
    configElement.setAttribute( QStringLiteral( "FullUrl" ), config.value( QStringLiteral( "FullUrl" ) ).toBool() );

  if ( config.contains( QStringLiteral( "DefaultRoot" ) ) )
    configElement.setAttribute( QStringLiteral( "DefaultRoot" ), config.value( QStringLiteral( "DefaultRoot" ) ).toString() );

  if ( config.contains( QStringLiteral( "RelativeStorage" ) ) )
    configElement.setAttribute( QStringLiteral( "RelativeStorage" ) , config.value( QStringLiteral( "RelativeStorage" ) ).toString() );

  if ( config.contains( QStringLiteral( "DocumentViewer" ) ) )
    configElement.setAttribute( QStringLiteral( "DocumentViewer" ), config.value( QStringLiteral( "DocumentViewer" ) ).toInt() );

  if ( config.contains( QStringLiteral( "DocumentViewerWidth" ) ) )
    configElement.setAttribute( QStringLiteral( "DocumentViewerWidth" ), config.value( QStringLiteral( "DocumentViewerWidth" ) ).toInt() );

  if ( config.contains( QStringLiteral( "DocumentViewerHeight" ) ) )
    configElement.setAttribute( QStringLiteral( "DocumentViewerHeight" ), config.value( QStringLiteral( "DocumentViewerHeight" ) ).toInt() );

  if ( config.contains( QStringLiteral( "FileWidgetFilter" ) ) )
    configElement.setAttribute( QStringLiteral( "FileWidgetFilter" ), config.value( QStringLiteral( "FileWidgetFilter" ) ).toString() );

  configElement.setAttribute( QStringLiteral( "StorageMode" ), config.value( QStringLiteral( "StorageMode" ) ).toString() );
}

QgsEditorWidgetConfig QgsExternalResourceWidgetFactory::readConfig( const QDomElement& configElement, QgsVectorLayer* layer, int fieldIdx )
{
  Q_UNUSED( layer )
  Q_UNUSED( fieldIdx )

  QgsEditorWidgetConfig cfg;

  if ( configElement.hasAttribute( QStringLiteral( "FileWidgetButton" ) ) )
    cfg.insert( QStringLiteral( "FileWidgetButton" ), configElement.attribute( QStringLiteral( "FileWidgetButton" ) ) == QLatin1String( "1" ) );

  if ( configElement.hasAttribute( QStringLiteral( "FileWidget" ) ) )
    cfg.insert( QStringLiteral( "FileWidget" ), configElement.attribute( QStringLiteral( "FileWidget" ) ) == QLatin1String( "1" ) );

  if ( configElement.hasAttribute( QStringLiteral( "UseLink" ) ) )
    cfg.insert( QStringLiteral( "UseLink" ), configElement.attribute( QStringLiteral( "UseLink" ) ) == QLatin1String( "1" ) );

  if ( configElement.hasAttribute( QStringLiteral( "FullUrl" ) ) )
    cfg.insert( QStringLiteral( "FullUrl" ), configElement.attribute( QStringLiteral( "FullUrl" ) ) == QLatin1String( "1" ) );

  if ( configElement.hasAttribute( QStringLiteral( "DefaultRoot" ) ) )
    cfg.insert( QStringLiteral( "DefaultRoot" ), configElement.attribute( QStringLiteral( "DefaultRoot" ) ) );

  if ( configElement.hasAttribute( QStringLiteral( "RelativeStorage" ) ) )
  {
    if (( configElement.attribute( QStringLiteral( "RelativeStorage" ) ).toInt() == QgsFileWidget::RelativeDefaultPath && configElement.hasAttribute( QStringLiteral( "DefaultRoot" ) ) ) ||
        configElement.attribute( QStringLiteral( "RelativeStorage" ) ).toInt() == QgsFileWidget::RelativeProject )
      cfg.insert( QStringLiteral( "RelativeStorage" ) , configElement.attribute( QStringLiteral( "RelativeStorage" ) ).toInt() );
  }

  if ( configElement.hasAttribute( QStringLiteral( "DocumentViewer" ) ) )
    cfg.insert( QStringLiteral( "DocumentViewer" ), configElement.attribute( QStringLiteral( "DocumentViewer" ) ) );

  if ( configElement.hasAttribute( QStringLiteral( "DocumentViewerWidth" ) ) )
    cfg.insert( QStringLiteral( "DocumentViewerWidth" ), configElement.attribute( QStringLiteral( "DocumentViewerWidth" ) ) );

  if ( configElement.hasAttribute( QStringLiteral( "DocumentViewerHeight" ) ) )
    cfg.insert( QStringLiteral( "DocumentViewerHeight" ), configElement.attribute( QStringLiteral( "DocumentViewerHeight" ) ) );

  if ( configElement.hasAttribute( QStringLiteral( "FileWidgetFilter" ) ) )
    cfg.insert( QStringLiteral( "FileWidgetFilter" ), configElement.attribute( QStringLiteral( "FileWidgetFilter" ) ) );


  cfg.insert( QStringLiteral( "StorageMode" ), configElement.attribute( QStringLiteral( "StorageMode" ), QStringLiteral( "Files" ) ) );

  return cfg;
}

unsigned int QgsExternalResourceWidgetFactory::fieldScore( const QgsVectorLayer* vl, int fieldIdx ) const
{
  if ( vl->fields().at( fieldIdx ).type() == QVariant::String )
    return 5;

  return 0;
}
