/***************************************************************************
    qgsideoptions.cpp
    -------------------------
    begin                : June 2024
    copyright            : (C) 2024 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsideoptions.h"
#include "moc_qgsideoptions.cpp"
#include "qgsapplication.h"
#include "qgssettings.h"
#include "qgis.h"

#include <QDesktopServices>
#include <QUrl>

//
// QgsIdeOptionsWidget
//

QgsIdeOptionsWidget::QgsIdeOptionsWidget( QWidget *parent )
  : QgsOptionsPageWidget( parent )
{
  setupUi( this );

  layout()->setContentsMargins( 0, 0, 0, 0 );

  connect( mGenerateTokenButton, &QPushButton::clicked, this, &QgsIdeOptionsWidget::generateGitHubToken );

  QgsSettings settings;
  mTokenLineEdit->setText( settings.value( "pythonConsole/accessTokenGithub", QString() ).toString() );
}

QgsIdeOptionsWidget::~QgsIdeOptionsWidget() = default;

QString QgsIdeOptionsWidget::helpKey() const
{
  return QStringLiteral( "introduction/qgis_configuration.html#code-editor-options" );
}

void QgsIdeOptionsWidget::apply()
{
  QgsSettings settings;
  settings.setValue( "pythonConsole/accessTokenGithub", mTokenLineEdit->text() );
}

void QgsIdeOptionsWidget::generateGitHubToken()
{
  QDesktopServices::openUrl( QUrl(
    QStringLiteral( "https://github.com/settings/tokens/new?description=%1&scopes=gist" ).arg( tr( "QGIS Code Editor" ) )
  ) );
}

//
// QgsIdeOptionsFactory
//

QgsIdeOptionsFactory::QgsIdeOptionsFactory()
  : QgsOptionsWidgetFactory( tr( "IDE" ), QIcon(), QStringLiteral( "ide" ) )
{
}

QIcon QgsIdeOptionsFactory::icon() const
{
  return QgsApplication::getThemeIcon( QStringLiteral( "/mIconCodeEditor.svg" ) );
}

QgsOptionsPageWidget *QgsIdeOptionsFactory::createWidget( QWidget *parent ) const
{
  return new QgsIdeOptionsWidget( parent );
}

QString QgsIdeOptionsFactory::pagePositionHint() const
{
  return QString();
}
