/***************************************************************************
  qgsabstractmaterialsettings.h
  --------------------------------------
  Date                 : July 2020
  Copyright            : (C) 2020 by Nyall Dawson
  Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSABSTRACTMATERIALSETTINGS_H
#define QGSABSTRACTMATERIALSETTINGS_H

#include "qgis_core.h"
#include "qgis_sip.h"
#include "qgsabstract3dasset.h"
#include "qgspropertycollection.h"

#include <QColor>
#include <QString>

using namespace Qt::StringLiterals;

class QDomElement;
class QgsReadWriteContext;
class QgsLineMaterial;
class QgsExpressionContext;

/**
 * \ingroup core
 * \brief Abstract base class for material settings.
 *
 * \warning This is not considered stable API, and may change in future QGIS releases. It is
 * exposed to the Python bindings as a tech preview only.
 *
 */
class CORE_EXPORT QgsAbstractMaterialSettings : public QgsAbstract3DAsset
{
  public:
    Qgis::Asset3DType assetType() const override;

    /**
     * Returns the unique type name for the material.
     */
    virtual QString type() const = 0;

    /**
     * Returns TRUE if the material requires texture coordinates to be generated
     * during triangulation.
     *
     * \since QGIS 4.2
     */
    virtual bool requiresTextureCoordinates() const;

    /**
     * Returns TRUE if the material requires tangents generated
     * during triangulation.
     *
     * \since QGIS 4.2
     */
    virtual bool requiresTangents() const;

    // *INDENT-OFF*
    //! Data definable properties.
    enum class Property SIP_MONKEYPATCH_SCOPEENUM_UNNEST( QgsAbstractMaterialSettings, Property ) : int
    {
      Diffuse, //!< Diffuse color
      Ambient, //!< Ambient color (phong material)
      Warm,    //!< Warm color (gooch material)
      Cool,    //!< Cool color (gooch material)
      Specular //!< Specular color
    };
    // *INDENT-ON*

    /**
     * Sets the material property collection, used for data defined overrides.
     * \since QGIS 4.2
     */
    void setDataDefinedProperties( const QgsPropertyCollection &collection );

    /**
    * Returns the symbol material property collection, used for data defined overrides.
    * \since QGIS 4.2
    */
    QgsPropertyCollection dataDefinedProperties() const;

    /**
    * Returns a reference to the material properties definition, used for data defined overrides.
    * \since QGIS 4.2
    */
    const QgsPropertiesDefinition &propertyDefinitions() const;
    virtual QgsAbstractMaterialSettings *clone() const override = 0 SIP_FACTORY;

    void readXml( const QDomElement &element, const QgsReadWriteContext &context ) override;
    void writeXml( QDomElement &element, const QgsReadWriteContext &context ) const override;

  private:
    QgsPropertyCollection mDataDefinedProperties;
    static QgsPropertiesDefinition sPropertyDefinitions;
    void initPropertyDefinitions() const;
};


#endif // QGSABSTRACTMATERIALSETTINGS_H
