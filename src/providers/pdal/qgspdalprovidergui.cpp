/***************************************************************************
  qgspdalprovidergui.cpp
  --------------------------------------
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

#include "qgsapplication.h"
#include "qgsproviderguimetadata.h"

#include "qgspdalprovider.h"


class QgsPdalProviderGuiMetadata : public QgsProviderGuiMetadata
{
  public:
    QgsPdalProviderGuiMetadata()
      : QgsProviderGuiMetadata( QStringLiteral( "pdal" ) )
    {
    }
};


QGISEXTERN QgsProviderGuiMetadata *providerGuiMetadataFactory()
{
  return new QgsPdalProviderGuiMetadata();
}
