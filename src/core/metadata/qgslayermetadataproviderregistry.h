/***************************************************************************
  qgslayermetadataproviderregistry.h - QgsLayerMetadataProviderRegistry

 ---------------------
 begin                : 17.8.2022
 copyright            : (C) 2022 by Alessandro Pasotti
 email                : elpaso at itopen dot it
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSLAYERMETADATAPROVIDERREGISTRY_H
#define QGSLAYERMETADATAPROVIDERREGISTRY_H

#include <QObject>

#include "qgis_core.h"
#include "qgis.h"

#include "qgslayermetadata.h"
#include "qgsabstractlayermetadataprovider.h"

class QgsFeedback;

#ifdef SIP_RUN
% ModuleHeaderCode
#include "qgsabstractlayermetadataprovider.h"
% End
#endif

/**
 * \ingroup core
 * \brief Registry of layer metadata provider backends.
 *
 * This is a singleton that should be accessed through QgsApplication::layerMetadataProviderRegistry().
 *
 * \see QgsAbstractLayerMetadataProvider
 * \since QGIS 3.28
 */
class CORE_EXPORT QgsLayerMetadataProviderRegistry : public QObject
{

    Q_OBJECT
  public:

    //! Creates the layer metadata provider registry, with an optional \a parent
    explicit QgsLayerMetadataProviderRegistry( QObject *parent = nullptr );

    //! Registers a layer metadata provider \a metadataProvider and takes ownership of it
    void registerLayerMetadataProvider( QgsAbstractLayerMetadataProvider *metadataProvider SIP_TRANSFER );

    //! Unregisters a layer metadata provider \a metadataProvider and destroys its instance
    void unregisterLayerMetadataProvider( QgsAbstractLayerMetadataProvider *metadataProvider );

    //! Returns the list of all registered layer metadata providers.
    QList<QgsAbstractLayerMetadataProvider *> layerMetadataProviders() const;

    //! Returns metadata provider implementation if the \a id matches one. Returns NULLPTR otherwise.
    QgsAbstractLayerMetadataProvider *layerMetadataProviderFromId( const QString &id );

    /**
     * Search for layers in all the registered layer metadata providers, optionally filtering by \a searchString
     * and \a geographicExtent, an optional \a feedback can be used to monitor and control the search process.
     */
    const QgsLayerMetadataSearchResults search( const QgsMetadataSearchContext &searchContext, const QString &searchString = QString(), const QgsRectangle &geographicExtent = QgsRectangle(), QgsFeedback *feedback = nullptr );

  private:

    QHash<QString,  QgsAbstractLayerMetadataProvider *> mMetadataProviders;

};

#endif // QGSLAYERMETADATAPROVIDERREGISTRY_H
