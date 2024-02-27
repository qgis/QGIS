/***************************************************************************
    qgssensorthingsguiprovider.h
     --------------------------------------
    Date                 : December 2023
    Copyright            : (C) 2023 by Nyall Dawson
    Email                : nyall dot dawson at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


#ifndef QGSSENSORTHINGSGUIPROVIDER_H
#define QGSSENSORTHINGSGUIPROVIDER_H

#include "qgsproviderguimetadata.h"
#include "qgssourceselectprovider.h"
#include "qgsprovidersourcewidgetprovider.h"

///@cond PRIVATE
#define SIP_NO_FILE

class QgsSensorThingsSourceSelectProvider : public QgsSourceSelectProvider
{
  public:

    QString providerKey() const final;
    QString text() const final;
    QIcon icon() const final;
    int ordering() const final;
    QgsAbstractDataSourceWidget *createDataSourceWidget( QWidget *parent = nullptr, Qt::WindowFlags fl = Qt::Widget, QgsProviderRegistry::WidgetMode widgetMode = QgsProviderRegistry::WidgetMode::Embedded ) const final;
};

class QgsSensorThingsSourceWidgetProvider : public QgsProviderSourceWidgetProvider
{
  public:
    QgsSensorThingsSourceWidgetProvider();
    QString providerKey() const override;
    bool canHandleLayer( QgsMapLayer *layer ) const override;
    QgsProviderSourceWidget *createWidget( QgsMapLayer *layer, QWidget *parent = nullptr ) override;
};

class QgsSensorThingsProviderGuiMetadata: public QgsProviderGuiMetadata
{
  public:
    QgsSensorThingsProviderGuiMetadata();
    QList<QgsSourceSelectProvider *> sourceSelectProviders() override;
    QList<QgsProviderSourceWidgetProvider *> sourceWidgetProviders() override;
    QList<QgsDataItemGuiProvider *> dataItemGuiProviders() override;
    QList<QgsSubsetStringEditorProvider *> subsetStringEditorProviders() override;

};

///@endcond
#endif // QGSSENSORTHINGSGUIPROVIDER_H
