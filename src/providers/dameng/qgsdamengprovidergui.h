/***************************************************************************
    qgsdamengprovidergui.h
    ------------------------
    begin                : 2025/01/14
    copyright            : ( C ) 2025 by Haiyang Zhao
    email                : zhaohaiyang@dameng.com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   ( at your option ) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSDAMENGPROVIDERGUI_H
#define QGSDAMENGPROVIDERGUI_H

#include <QList>
#include <QMainWindow>
#include <memory>

#include "qgsproviderguimetadata.h"

class QgsDamengProviderGuiMetadata : public QgsProviderGuiMetadata
{
public:
  QgsDamengProviderGuiMetadata();

  QList<QgsSourceSelectProvider*> sourceSelectProviders() override;
  QList<QgsDataItemGuiProvider*> dataItemGuiProviders() override;
  QList<QgsProjectStorageGuiProvider*> projectStorageGuiProviders() override;


};

#endif // QGSDAMENGPROVIDERGUI_H
