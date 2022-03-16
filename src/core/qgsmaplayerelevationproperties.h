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
    else
    {
      sipType = 0;
    }
    SIP_END
#endif

  public:

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
    void setZOffset( double offset ) { mZOffset = offset; }

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
    void setZScale( double scale ) { mZScale = scale; }

  signals:

    /**
     * Emitted when the elevation properties have changed.
     */
    void changed();

  protected:
    //! Z scale
    double mZScale = 1.0;
    //! Z offset
    double mZOffset = 0.0;
};

#endif // QGSMAPLAYERELEVATIONPROPERTIES_H
