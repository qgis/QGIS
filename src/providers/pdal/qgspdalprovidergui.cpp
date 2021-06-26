/***************************************************************************
  qgspdalprovidergui.cpp
  --------------------------------------
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

#include "qgsapplication.h"
#include "qgsproviderguimetadata.h"

#include "qgspdalprovider.h"


class QgsPdalProviderGuiMetadata: public QgsProviderGuiMetadata
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
