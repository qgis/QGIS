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
class CORE_EXPORT QgsAbstractMaterialSettings SIP_ABSTRACT
{
#ifdef SIP_RUN
    SIP_CONVERT_TO_SUBCLASS_CODE
    if ( sipCpp->type() == "gooch"_L1 )
    {
      sipType = sipType_QgsGoochMaterialSettings;
    }
    else if ( sipCpp->type() == "phong"_L1 )
    {
      sipType = sipType_QgsPhongMaterialSettings;
    }
    else if ( sipCpp->type() == "phongtextured" )
    {
      sipType = sipType_QgsPhongTexturedMaterialSettings;
    }
    else if ( sipCpp->type() == "simpleline" )
    {
      sipType = sipType_QgsSimpleLineMaterialSettings;
    }
    else if ( sipCpp->type() == "null" )
    {
      sipType = sipType_QgsNullMaterialSettings;
    }
    else if ( sipCpp->type() == "metalrough" )
    {
      sipType = sipType_QgsMetalRoughMaterialSettings;
    }
    else if ( sipCpp->type() == "metalroughtextured" )
    {
      sipType = sipType_QgsMetalRoughTexturedMaterialSettings;
    }
    else
    {
      sipType = 0;
    }
  SIP_END
#endif

  public:
    virtual ~QgsAbstractMaterialSettings() = default;

    /**
     * Returns the unique type name for the material.
     */
    virtual QString type() const = 0;

    /**
     * Clones the material settings.
     *
     * Caller takes ownership of the returned object.
     */
    virtual QgsAbstractMaterialSettings *clone() const = 0 SIP_FACTORY;

    //! Reads settings from a DOM \a element
    virtual void readXml( const QDomElement &element, const QgsReadWriteContext & );

    //! Writes settings to a DOM \a element
    virtual void writeXml( QDomElement &element, const QgsReadWriteContext & ) const;

    /**
     * Returns TRUE if this settings exactly matches an \a other settings.
     *
     * \since QGIS 3.42
     */
    virtual bool equals( const QgsAbstractMaterialSettings *other ) const = 0;

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

    /**
     * Returns an approximate color representing the blended material color.
     *
     * \since QGIS 4.2
     */
    virtual QColor averageColor() const = 0;

    /**
     * Decomposes a base color into the material's color components, and sets the material's colors accordingly.
     *
     * \param baseColor The color to decompose
     *
     * \since QGIS 4.2
     */
    virtual void setColorsFromBase( const QColor &baseColor ) = 0;

  private:
    QgsPropertyCollection mDataDefinedProperties;
    static QgsPropertiesDefinition sPropertyDefinitions;
    void initPropertyDefinitions() const;
};


#endif // QGSABSTRACTMATERIALSETTINGS_H
