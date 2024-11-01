/***************************************************************************
  qgspdalprovidergui.h
  ------------------------
  Date                 : November 2020
  Copyright            : (C) 2020 by Peter Petrik
  Email                : zilolv at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSPDALPROVIDERGUI_H
#define QGSPDALPROVIDERGUI_H

#include <QList>
#include <QMainWindow>

#include "qgsproviderguimetadata.h"

class QgsPdalProviderGuiMetadata : public QgsProviderGuiMetadata
{
  public:
    QgsPdalProviderGuiMetadata();

    QList<QgsSourceSelectProvider *> sourceSelectProviders() override;
    QList<QgsProjectStorageGuiProvider *> projectStorageGuiProviders() override;
    void registerGui( QMainWindow *mainWindow ) override;
};

#endif // QGSPDALPROVIDERGUI_H
