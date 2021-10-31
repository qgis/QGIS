/***************************************************************************
  qgswfsprovidergui.cpp
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

#include "qgswfsprovider.h"
#include "qgswfssourceselect.h"
#include "qgssourceselectprovider.h"
#include "qgsproviderguimetadata.h"


class QgsWfsProviderGuiMetadata: public QgsProviderGuiMetadata
{
  public:
    QgsWfsProviderGuiMetadata();

    QList<QgsSourceSelectProvider *> sourceSelectProviders() override;
    QList<QgsDataItemGuiProvider *> dataItemGuiProviders() override;
    QList<QgsSubsetStringEditorProvider *> subsetStringEditorProviders() override;
};
