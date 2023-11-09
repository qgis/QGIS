/***************************************************************************
  qgsvirtuallayerprovidergui.h
  --------------------------------------
  Date                 : Octpner 2021
  Copyright            : (C) 2021 by Mathieu Pellerin
  Email                : nirvn dot asia at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsproviderguimetadata.h"

class QgsVirtualLayerProviderGuiMetadata final: public QgsProviderGuiMetadata
{
  public:
    QgsVirtualLayerProviderGuiMetadata();
    QList<QgsSourceSelectProvider *> sourceSelectProviders() override;
};
