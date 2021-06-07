/***************************************************************************
      qgsogrguiprovider.h  - GUI for QGIS Data provider for GDAL rasters
                             -------------------
    begin                : June, 2019
    copyright            : (C) 2019 by Peter Petrik
    email                : zilolv at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   *
 *  
 *        *
 *                                     *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSOGRGUIPROVIDER_H
#define QGSOGRGUIPROVIDER_H

#include "qgsproviderguimetadata.h"
#include "qgsogrsourceselect.h"
#include "qgis_sip.h"

///@cond PRIVATE
#define SIP_NO_FILE

class QgsOgrGuiProviderMetadata: public QgsProviderGuiMetadata
{
  public:
    QgsOgrGuiProviderMetadata();
    QList<QgsSourceSelectProvider *> sourceSelectProviders() override;
    QList<QgsDataItemGuiProvider *> dataItemGuiProviders() override;
    QList<QgsProjectStorageGuiProvider *> projectStorageGuiProviders() override;
};

///@endcond
#endif
