/***************************************************************************
  qgsimageserverprovidergui.h
  --------------------------------------
  Date                 : April 2026
  Copyright            : (C) 2026 by Nyall Dawson
  Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsproviderguimetadata.h"

class QgsImageServerProviderGuiMetadata : public QgsProviderGuiMetadata
{
  public:
    QgsImageServerProviderGuiMetadata();

    QList<QgsProviderSourceWidgetProvider *> sourceWidgetProviders() override;
};
