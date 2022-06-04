/***************************************************************************
                         qgsmaplayerelevationproperties.h
                         ---------------
    begin                : November 2020
    copyright            : (C) 2020 by Nyall Dawson
    email                : nyall dot dawson dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


#ifndef QGSMAPLAYERELEVATIONPROPERTIES_H
#define QGSMAPLAYERELEVATIONPROPERTIES_H

#include "qgis_core.h"
#include "qgis_sip.h"
#include "qgsreadwritecontext.h"
#include "qgsrange.h"
#include "qgspropertycollection.h"

#include <QObject>
#include <QDomElement>

/**
 * \class QgsMapLayerElevationProperties
 * \ingroup core
 * \brief Base class for storage of map layer elevation properties.
 *
 * QgsMapLayerElevationProperties exposes user-configurable settings for controlling
 * how an individual QgsMapLayer behaves with relation to z values or elevations.
 *
 * \since QGIS 3.18
 */
class CORE_EXPORT QgsMapLayerElevationProperties : public QObject
{
#ifdef SIP_RUN
#include "qgspointcloudlayerelevationproperties.h"
#include "qgsrasterlayerelevationproperties.h"
#include "qgsvectorlayerelevationproperties.h"
#include "qgsmeshlayerelevationproperties.h"
#endif

    Q_OBJECT

#ifdef SIP_RUN
    SIP_CONVERT_TO_SUBCLASS_CODE
    if ( qobject_cast<QgsPointCloudLayerElevationProperties *>( sipCpp ) )
    {
      sipType = sipType_QgsPointCloudLayerElevationProperties;
    }
    else if ( qobject_cast<QgsVectorLayerElevationProperties *>( sipCpp ) )
    {
      sipType = sipType_QgsVectorLayerElevationProperties;
    }
    else if ( qobject_cast<QgsRasterLayerElevationProperties *>( sipCpp ) )
    {
      sipType = sipType_QgsRasterLayerElevationProperties;
    }
    else if ( qobject_cast<QgsMeshLayerElevationProperties *>( sipCpp ) )
    {
      sipType = sipType_QgsMeshLayerElevationProperties;
    }
    else
    {
      sipType = 0;
    }
    SIP_END
#endif

  public:

    /**
     * Data definable properties.
     * \since QGIS 3.26
     */
    enum Property
    {
      ZOffset, //! Z offset
      ExtrusionHeight, //!< Extrusion height
    };

    /**
     * Flags attached to the elevation property.
     */
    enum Flag
    {
      FlagDontInvalidateCachedRendersWhenRangeChanges = 1  //!< Any cached rendering will not be invalidated when z range context is modified.
    };
    Q_DECLARE_FLAGS( Flags, Flag )

    /**
     * Constructor for QgsMapLayerElevationProperties, with the specified \a parent object.
     */
    QgsMapLayerElevationProperties( QObject *parent SIP_TRANSFERTHIS );

    /**
     * Returns TRUE if the layer has an elevation or z component.
     */
    virtual bool hasElevation() const;

    /**
     * Writes the properties to a DOM \a element, to be used later with readXml().
     *
     * \see readXml()
     */
    virtual QDomElement writeXml( QDomElement &element, QDomDocument &doc, const QgsReadWriteContext &context ) = 0;

    /**
     * Reads the elevation properties from a DOM \a element previously written by writeXml().
     *
     * \see writeXml()
     */
    virtual bool readXml( const QDomElement &element, const QgsReadWriteContext &context ) = 0;

    /**
     * Sets default properties based on sensible choices for the given map \a layer.
     *
     * \since QGIS 3.26
     */
    virtual void setDefaultsFromLayer( QgsMapLayer *layer );

    /**
     * Returns a HTML formatted summary of the properties.
     *
     * \since QGIS 3.26
     */
    virtual QString htmlSummary() const;

    /**
     * Creates a clone of the properties.
     *
     * \since QGIS 3.26
     */
    virtual QgsMapLayerElevationProperties *clone() const = 0 SIP_FACTORY;

    /**
     * Returns TRUE if the layer should be visible and rendered for the specified z \a range.
     */
    virtual bool isVisibleInZRange( const QgsDoubleRange &range ) const;

    /**
     * Returns flags associated to the elevation properties.
     */
    virtual QgsMapLayerElevationProperties::Flags flags() const { return QgsMapLayerElevationProperties::Flags(); }

    /**
     * Attempts to calculate the overall elevation or z range for the specified \a layer, using
     * the settings defined by this elevation properties object.
     *
     * May return an infinite range if the extent could not be calculated.
     */
    virtual QgsDoubleRange calculateZRange( QgsMapLayer *layer ) const;

    /**
     * Returns TRUE if the layer should be visible by default in newly created elevation
     * profile plots.
     *
     * Subclasses should override this with logic which determines whether the layer is
     * likely desirable to be initially checked in these plots.
     *
     * \since QGIS 3.26
     */
    virtual bool showByDefaultInElevationProfilePlots() const;

    /**
     * Returns the z offset, which is a fixed offset amount which should be added to z values from
     * the layer.
     *
     * \note Any scaling specified via zScale() is applied before any offset value specified via zOffset()
     *
     * \see setZOffset()
     */
    double zOffset() const { return mZOffset; }

    /**
     * Sets the z \a offset, which is a fixed offset amount which will be added to z values from
     * the layer.
     *
     * \note Any scaling specified via zScale() is applied before any offset value specified via zOffset()
     *
     * \see zOffset()
     */
    void setZOffset( double offset );

    /**
     * Returns the z scale, which is a scaling factor which should be applied to z values from
     * the layer.
     *
     * This can be used to correct or manually adjust for incorrect elevation values in a layer, such
     * as conversion of elevation values in feet to meters.
     *
     * \note Any scaling specified via zScale() is applied before any offset value specified via zOffset()
     *
     * \see setZScale()
     */
    double zScale() const { return mZScale; }

    /**
     * Sets the z \a scale, which is a scaling factor which will be applied to z values from
     * the layer.
     *
     * This can be used to correct or manually adjust for incorrect elevation values in a layer, such
     * as conversion of elevation values in feet to meters.
     *
     * \note Any scaling specified via zScale() is applied before any offset value specified via zOffset()
     *
     * \see zScale()
     */
    void setZScale( double scale );

    /**
     * Returns a reference to the object's property collection, used for data defined overrides.
     * \see setDataDefinedProperties()
     * \since QGIS 3.26
     */
    QgsPropertyCollection &dataDefinedProperties() { return mDataDefinedProperties; }

    /**
     * Returns a reference to the object's property collection, used for data defined overrides.
     * \see setDataDefinedProperties()
     * \see Property
     * \note not available in Python bindings
     * \since QGIS 3.26
     */
    const QgsPropertyCollection &dataDefinedProperties() const SIP_SKIP { return mDataDefinedProperties; }

    /**
     * Sets the object's property \a collection, used for data defined overrides.
     *
     * Any existing properties will be discarded.
     *
     * \see dataDefinedProperties()
     * \see Property
     * \since QGIS 3.26
     */
    void setDataDefinedProperties( const QgsPropertyCollection &collection );

    /**
     * Returns the definitions for data defined properties available for use in elevation properties.
     *
     * \since QGIS 3.26
     */
    static QgsPropertiesDefinition propertyDefinitions();

  signals:

    /**
     * Emitted when any of the elevation properties have changed.
     *
     * See renderingPropertyChanged() and profileGenerationPropertyChanged() for more fine-grained signals.
     */
    void changed();

    /**
     * Emitted when the z offset changes.
     *
     * \since QGIS 3.26
     */
    void zOffsetChanged();

    /**
     * Emitted when the z scale changes.
     *
     * \since QGIS 3.26
     */
    void zScaleChanged();

    /**
     * Emitted when any of the elevation properties which relate solely to presentation of elevation
     * results have changed.
     *
     * \see changed()
     * \see profileGenerationPropertyChanged()
     * \since QGIS 3.26
     */
    void profileRenderingPropertyChanged();

    /**
     * Emitted when any of the elevation properties which relate solely to generation of elevation
     * profiles have changed.
     *
     * \see changed()
     * \see profileRenderingPropertyChanged()
     * \since QGIS 3.26
     */
    void profileGenerationPropertyChanged();

  protected:
    //! Z scale
    double mZScale = 1.0;
    //! Z offset
    double mZOffset = 0.0;

    //! Property collection for data defined elevation settings
    QgsPropertyCollection mDataDefinedProperties;

    //! Property definitions
    static QgsPropertiesDefinition sPropertyDefinitions;

    /**
     * Writes common class properties to a DOM \a element, to be used later with readXml().
     *
     * \see readCommonProperties()
     * \since QGIS 3.26
     */
    void writeCommonProperties( QDomElement &element, QDomDocument &doc, const QgsReadWriteContext &context );

    /**
     * Reads common class properties from a DOM \a element previously written by writeXml().
     *
     * \see writeCommonProperties()
     * \since QGIS 3.26
     */
    void readCommonProperties( const QDomElement &element, const QgsReadWriteContext &context );

    /**
     * Copies common properties from another object.
     *
     * \since QGIS 3.26
     */
    void copyCommonProperties( const QgsMapLayerElevationProperties *other );

  private:

    /**
     * Initializes property definitions.
     */
    static void initPropertyDefinitions();

};

#endif // QGSMAPLAYERELEVATIONPROPERTIES_H
