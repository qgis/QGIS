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

    /**
     * Returns the project's elevation range, which indicates the upper and lower
     * elevation limits associated with the project.
     *
     * \note This is a manual, use-set property, and does not necessarily
     * coincide with the elevation ranges for individual layers in the project.
     *
     * \see setElevationRange()
     * \see elevationRangeChanged()
     *
     * \since QGIS 3.38
     */
    QgsDoubleRange elevationRange() const { return mElevationRange; }

    /**
     * Returns the fixed size for elevation range filtering in the project, used when interactively filtering by elevation.
     *
     * Returns -1 if no fixed elevation range size is desired.
     *
     * A fixed size forces the selected elevation range to have a matching difference between
     * the upper and lower elevation.
     *
     * \see setElevationFilterRangeSize()
     *
     * \since QGIS 3.38
     */
    double elevationFilterRangeSize() const { return mElevationFilterRangeSize; }

    /**
     * Returns TRUE if the elevation range filter slider should be inverted for this project.
     *
     * \see setInvertElevationFilter()
     *
     * \since QGIS 3.38
     */
    bool invertElevationFilter() const { return mInvertElevationFilter; }

  public slots:

    /**
     * Sets the project's elevation \a range, which indicates the upper and lower
     * elevation limits associated with the project.
     *
     * \note This is a manual, use-set property, and does not necessarily
     * coincide with the elevation ranges for individual layers in the project.
     *
     * \see elevationRange()
     * \see elevationRangeChanged()
     *
     * \since QGIS 3.38
     */
    void setElevationRange( const QgsDoubleRange &range );

    /**
     * Sets the fixed size for elevation range filtering in the project, used when interactively filtering by elevation.
     *
     * Set to -1 if no fixed elevation range size is desired.
     *
     * A fixed size forces the selected elevation range to have a matching difference between
     * the upper and lower elevation.
     *
     * \see elevationFilterRangeSize()
     *
     * \since QGIS 3.38
     */
    void setElevationFilterRangeSize( double size );

    /**
     * Sets whether the elevation range filter slider should be inverted for this project.
     *
     * \see invertElevationFilter()
     *
     * \since QGIS 3.38
     */
    void setInvertElevationFilter( bool invert );

  signals:

    /**
     * Emitted when the elevation properties change.
     */
    void changed();

    /**
    * Emitted when the project's elevation \a is changed.
    *
     * \note This is a manual, use-set property, and does not necessarily
     * coincide with the elevation ranges for individual layers in the project.
     *
     * \see elevationRange()
     * \see setElevationRange()
     *
     * \since QGIS 3.38
    */
    void elevationRangeChanged( const QgsDoubleRange &range );

  private:
    std::unique_ptr<QgsAbstractTerrainProvider> mTerrainProvider;
    QgsDoubleRange mElevationRange;
    double mElevationFilterRangeSize = -1;
    bool mInvertElevationFilter = false;
};


#endif // QGSPROJECTELEVATIONPROPERTIES_H
