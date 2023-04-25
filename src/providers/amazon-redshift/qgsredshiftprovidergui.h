/***************************************************************************
   qgsredshiftprovidergui.h
   --------------------------------------
   Date      : 16.02.2021
   Copyright : (C) 2021 Amazon Inc. or its affiliates
   Author    : Marcel Bezdrighin
 ***************************************************************************/

/***************************************************************************
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 ***************************************************************************/
#ifndef QGSREDSHIFTPROVIDERGUI_H
#define QGSREDSHIFTPROVIDERGUI_H

#include <QList>
#include <QMainWindow>

#include "qgsproviderguimetadata.h"

class QgsRedshiftProviderGuiMetadata : public QgsProviderGuiMetadata
{
  public:
    QgsRedshiftProviderGuiMetadata();

    QList<QgsSourceSelectProvider *> sourceSelectProviders() override;
    QList<QgsDataItemGuiProvider *> dataItemGuiProviders() override;
    QList<QgsProjectStorageGuiProvider *> projectStorageGuiProviders() override;
    void registerGui( QMainWindow *mainWindow ) override;
};

#endif // QGSREDSHIFTPROVIDERGUI_H
