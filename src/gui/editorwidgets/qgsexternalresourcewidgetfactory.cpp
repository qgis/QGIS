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

void QgsExternalResourceWidgetFactory::writeConfig( const QVariantMap& config, QDomElement& configElement, QDomDocument& doc, const QgsVectorLayer* layer, int fieldIdx )
{
  Q_UNUSED( doc )
  Q_UNUSED( layer )
  Q_UNUSED( fieldIdx )

  config2xml( config, configElement, QStringLiteral( "FileWidget" ) );
  config2xml( config, configElement, QStringLiteral( "FileWidgetButton" ) );
  config2xml( config, configElement, QStringLiteral( "UseLink" ) );
  config2xml( config, configElement, QStringLiteral( "FullUrl" ) );
  config2xml( config, configElement, QStringLiteral( "DefaultRoot" ) );
  config2xml( config, configElement, QStringLiteral( "RelativeStorage" ) );
  config2xml( config, configElement, QStringLiteral( "DocumentViewer" ) );
  config2xml( config, configElement, QStringLiteral( "DocumentViewerWidth" ) );
  config2xml( config, configElement, QStringLiteral( "DocumentViewerHeight" ) );
  config2xml( config, configElement, QStringLiteral( "FileWidgetFilter" ) );
  config2xml( config, configElement, QStringLiteral( "StorageMode" ) );
}

QVariantMap QgsExternalResourceWidgetFactory::readConfig( const QDomElement& configElement, QgsVectorLayer* layer, int fieldIdx )
{
  Q_UNUSED( layer )
  Q_UNUSED( fieldIdx )

  QVariantMap cfg;

  xml2config( configElement, cfg, QStringLiteral( "FileWidget" ) );
  xml2config( configElement, cfg, QStringLiteral( "FileWidgetButton" ) );
  xml2config( configElement, cfg, QStringLiteral( "UseLink" ) );
  xml2config( configElement, cfg, QStringLiteral( "FullUrl" ) );
  xml2config( configElement, cfg, QStringLiteral( "DefaultRoot" ) );
  if ( configElement.hasAttribute( QStringLiteral( "RelativeStorage" ) ) )
  {
    if (( configElement.attribute( QStringLiteral( "RelativeStorage" ) ).toInt() == QgsFileWidget::RelativeDefaultPath && configElement.hasAttribute( QStringLiteral( "DefaultRoot" ) ) ) ||
        configElement.attribute( QStringLiteral( "RelativeStorage" ) ).toInt() == QgsFileWidget::RelativeProject )
      xml2config( configElement, cfg, QStringLiteral( "RelativeStorage" ) );
  }
  xml2config( configElement, cfg, QStringLiteral( "DocumentViewer" ) );
  xml2config( configElement, cfg, QStringLiteral( "DocumentViewerWidth" ) );
  xml2config( configElement, cfg, QStringLiteral( "DocumentViewerHeight" ) );
  xml2config( configElement, cfg, QStringLiteral( "FileWidgetFilter" ) );
  xml2config( configElement, cfg, QStringLiteral( "StorageMode" ) );

  return cfg;
}

unsigned int QgsExternalResourceWidgetFactory::fieldScore( const QgsVectorLayer* vl, int fieldIdx ) const
{
  if ( vl->fields().at( fieldIdx ).type() == QVariant::String )
    return 5;

  return 0;
}
