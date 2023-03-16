/***************************************************************************
  qgsarcgisvectortileservicedataprovider.h
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

#ifndef QGSARCGISVECTORTILESERVICEDATAPROVIDER_H
#define QGSARCGISVECTORTILESERVICEDATAPROVIDER_H

#include "qgis_core.h"
#include "qgis_sip.h"
#include "qgsxyzvectortiledataprovider.h"

#define SIP_NO_FILE

///@cond PRIVATE

class CORE_EXPORT QgsArcGisVectorTileServiceDataProvider : public QgsXyzVectorTileDataProvider
{
    Q_OBJECT

  public:
    QgsArcGisVectorTileServiceDataProvider( const QString &uri,
                                            const QString &sourcePath,
                                            const QgsDataProvider::ProviderOptions &providerOptions,
                                            QgsDataProvider::ReadFlags flags );
    QgsVectorTileDataProvider *clone() const override;
    QString sourcePath() const override;
    bool isValid() const override;

  private:

    QString mSourcePath;
};

///@endcond

#endif // QGSARCGISVECTORTILESERVICEDATAPROVIDER_H
