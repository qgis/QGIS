/***************************************************************************
  qgsdelimitedtextprovidergui.cpp
  --------------------------------------
  Date                 : June 2019
  Copyright            : (C) 2019 by Martin Dobias
  Email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsdelimitedtextprovidergui.h"

#include "qgsapplication.h"
#include "qgsproviderguimetadata.h"
#include "qgssourceselectprovider.h"

#include "qgsdelimitedtextprovider.h"
#include "qgsdelimitedtextsourceselect.h"

//! Provider for delimited text source select
class QgsDelimitedTextSourceSelectProvider : public QgsSourceSelectProvider
{
  public:

    QString providerKey() const override { return QStringLiteral( "delimitedtext" ); }
    QString text() const override { return QObject::tr( "Delimited Text" ); }
    int ordering() const override { return QgsSourceSelectProvider::OrderLocalProvider + 30; }
    QIcon icon() const override { return QgsApplication::getThemeIcon( QStringLiteral( "/mActionAddDelimitedTextLayer.svg" ) ); }
    QgsAbstractDataSourceWidget *createDataSourceWidget( QWidget *parent = nullptr, Qt::WindowFlags fl = Qt::Widget, QgsProviderRegistry::WidgetMode widgetMode = QgsProviderRegistry::WidgetMode::Embedded ) const override
    {
      return new QgsDelimitedTextSourceSelect( parent, fl, widgetMode );
    }
};


QgsDelimitedTextProviderGuiMetadata::QgsDelimitedTextProviderGuiMetadata()
  : QgsProviderGuiMetadata( QgsDelimitedTextProvider::TEXT_PROVIDER_KEY )
{
}

QList<QgsSourceSelectProvider *> QgsDelimitedTextProviderGuiMetadata::sourceSelectProviders()
{
  QList<QgsSourceSelectProvider *> providers;
  providers << new QgsDelimitedTextSourceSelectProvider;
  return providers;
}

#ifndef HAVE_STATIC_PROVIDERS
QGISEXTERN QgsProviderGuiMetadata *providerGuiMetadataFactory()
{
  return new QgsDelimitedTextProviderGuiMetadata();
}
#endif
