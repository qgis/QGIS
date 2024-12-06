/***************************************************************************
  qgsvectortileproviderguimetadata.h
  --------------------------------------
  Date                 : March 2020
  Copyright            : (C) 2020 by Martin Dobias
  Email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSVECTORTILEPROVIDERGUIMETADATA_H
#define QGSVECTORTILEPROVIDERGUIMETADATA_H

///@cond PRIVATE
#define SIP_NO_FILE

#include <QList>
#include <QMainWindow>

#include "qgsproviderguimetadata.h"

class QgsVectorTileProviderGuiMetadata : public QgsProviderGuiMetadata
{
  public:
    QgsVectorTileProviderGuiMetadata();

    QList<QgsDataItemGuiProvider *> dataItemGuiProviders() override;
    QList<QgsSourceSelectProvider *> sourceSelectProviders() override;
};

///@endcond

#endif // QGSVECTORTILEPROVIDERGUIMETADATA_H
