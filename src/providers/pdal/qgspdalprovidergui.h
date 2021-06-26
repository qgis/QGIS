/***************************************************************************
  qgspdalprovidergui.h
  ------------------------
  Date                 : November 2020
  Copyright            : (C) 2020 by Peter Petrik
  Email                : zilolv at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   *
 *  
 *        *
 *                                     *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSPDALPROVIDERGUI_H
#define QGSPDALPROVIDERGUI_H

#include <QList>
#include <QMainWindow>

#include "qgsproviderguimetadata.h"

class QgsPdalProviderGuiMetadata: public QgsProviderGuiMetadata
{
  public:
    QgsPdalProviderGuiMetadata();

    QList<QgsSourceSelectProvider *> sourceSelectProviders() override;
    QList<QgsDataItemGuiProvider *> dataItemGuiProviders() override;
    QList<QgsProjectStorageGuiProvider *> projectStorageGuiProviders() override;
    void registerGui( QMainWindow *mainWindow ) override;
};

#endif // QGSPDALPROVIDERGUI_H
