/***************************************************************************
                         qgsprojectelevationproperties.h
                         ---------------
    begin                : February 2022
    copyright            : (C) 2022 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSPROJECTELEVATIONPROPERTIES_H
#define QGSPROJECTELEVATIONPROPERTIES_H

#include "qgis_core.h"
#include "qgsrange.h"
#include "qgsunittypes.h"
#include <memory>
#include <QObject>

class QDomElement;
class QgsReadWriteContext;
class QDomDocument;
class QgsAbstractTerrainProvider;
class QgsProject;

/**
 * \brief Contains elevation properties for a QgsProject.
 *
 * \ingroup core
 * \since QGIS 3.26
 */
class CORE_EXPORT QgsProjectElevationProperties : public QObject
{
    Q_OBJECT

  public:

    /**
     * Constructor for QgsProjectElevationProperties with the specified \a parent object.
     */
    QgsProjectElevationProperties( QObject *parent SIP_TRANSFERTHIS = nullptr );

    ~QgsProjectElevationProperties() override;

    /**
     * Resets the properties to a default state.
     */
    void reset();

    /**
     * Resolves reference to layers from stored layer ID.
     */
    void resolveReferences( const QgsProject *project );

    /**
     * Reads the property state from a DOM \a element.
     * \see writeXml()
     */
    bool readXml( const QDomElement &element, const QgsReadWriteContext &context );

    /**
     * Returns a DOM element representing the properties.
     * \see readXml()
     */
    QDomElement writeXml( QDomDocument &document, const QgsReadWriteContext &context ) const;

    /**
     * Returns the project's terrain provider.
     *
     * \see setTerrainProvider()
     */
    QgsAbstractTerrainProvider *terrainProvider();

    /**
     * Sets the project's terrain \a provider.
     *
     * Ownership of \a provider is transferred to this object.
     *
     * \see terrainProvider()
     */
    void setTerrainProvider( QgsAbstractTerrainProvider *provider SIP_TRANSFER );

  signals:

    /**
     * Emitted when the elevation properties change.
     */
    void changed();

  private:

    std::unique_ptr< QgsAbstractTerrainProvider > mTerrainProvider;

};


#endif // QGSPROJECTELEVATIONPROPERTIES_H
