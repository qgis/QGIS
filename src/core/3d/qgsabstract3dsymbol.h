/***************************************************************************
  qgsabstract3dsymbol.h
  --------------------------------------
  Date                 : July 2017
  Copyright            : (C) 2017 by Martin Dobias
  Email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSABSTRACT3DSYMBOL_H
#define QGSABSTRACT3DSYMBOL_H

#include "qgis_core.h"
#include "qgis_sip.h"

#include "qgspropertycollection.h"

class QDomElement;
class QString;

class QgsReadWriteContext;
class Qgs3DSceneExporter;

namespace Qt3DCore { class QEntity; } SIP_SKIP


/**
 * \ingroup 3d
 * \brief Abstract base class for 3D symbols that are used by VectorLayer3DRenderer objects.
 *
 * 3D symbol objects define appearance of GIS data.
 *
 * \warning This is not considered stable API, and may change in future QGIS releases. It is
 * exposed to the Python bindings as a tech preview only.
 *
 * \note Prior to QGIS 3.16 this was available through the QGIS 3D library.
 *
 * \since QGIS 3.0
 */
class CORE_EXPORT QgsAbstract3DSymbol
{
  public:
    virtual ~QgsAbstract3DSymbol() = default;

    //! Returns identifier of symbol type. Each 3D symbol implementation should return a different type.
    virtual QString type() const = 0;
    //! Returns a new instance of the symbol with the same settings
    virtual QgsAbstract3DSymbol *clone() const = 0 SIP_FACTORY;

    //! Writes symbol configuration to the given DOM element
    virtual void writeXml( QDomElement &elem, const QgsReadWriteContext &context ) const = 0;
    //! Reads symbol configuration from the given DOM element
    virtual void readXml( const QDomElement &elem, const QgsReadWriteContext &context ) = 0;

    /**
     * Returns the list of the vector layer geometry types which are compatible with this symbol.
     *
     * \since QGIS 3.16
     */
    virtual QList< QgsWkbTypes::GeometryType > compatibleGeometryTypes() const;

    //! Data definable properties.
    enum Property
    {
      PropertyHeight = 0,       //!< Height (altitude)
      PropertyExtrusionHeight,  //!< Extrusion height (zero means no extrusion)
    };

    //! Returns the symbol layer property definitions.
    static const QgsPropertiesDefinition &propertyDefinitions();

    //! Returns a reference to the symbol layer's property collection, used for data defined overrides.
    QgsPropertyCollection &dataDefinedProperties() { return mDataDefinedProperties; }

    //! Returns a reference to the symbol layer's property collection, used for data defined overrides.
    const QgsPropertyCollection &dataDefinedProperties() const { return mDataDefinedProperties; } SIP_SKIP

    //! Sets the symbol layer's property collection, used for data defined overrides.
    void setDataDefinedProperties( const QgsPropertyCollection &collection ) { mDataDefinedProperties = collection; }

    /**
     * Exports the geometries contained within the hierarchy of entity.
     * Returns whether any objects were exported
     * If this function is not overloaded we don't try to export anything
     */
    virtual bool exportGeometries( Qgs3DSceneExporter *exporter, Qt3DCore::QEntity *entity, const QString &objectNamePrefix ) const SIP_SKIP;

    /**
     * Sets default properties for the symbol based on a layer's configuration.
     *
     * \since QGIS 3.26
     */
    virtual void setDefaultPropertiesFromLayer( const QgsVectorLayer *layer );

  protected:

    /**
     * Copies base class settings from this object to a \a destination object.
     */
    virtual void copyBaseSettings( QgsAbstract3DSymbol *destination ) const;
    QgsPropertyCollection mDataDefinedProperties;

  private:
    static void initPropertyDefinitions();

    //! Property definitions
    static QgsPropertiesDefinition sPropertyDefinitions;
};


#endif // QGSABSTRACT3DSYMBOL_H
