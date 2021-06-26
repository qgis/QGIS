/***************************************************************************
  qgstextmasksettings.h
  -----------------
   begin                : May 2020
   copyright            : (C) Nyall Dawson
   email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSTEXTMASKSETTINGS_H
#define QGSTEXTMASKSETTINGS_H

#include "qgis_sip.h"
#include "qgis_core.h"
#include "qgssymbollayerreference.h"
#include "qgsunittypes.h"
#include "qgsmapunitscale.h"

#include <QSharedDataPointer>
#include <QDomElement>

class QgsTextMaskSettingsPrivate;
class QgsPaintEffect;
class QgsPropertyCollection;

/**
 * \class QgsTextMaskSettings
  * \ingroup core
  * Container for settings relating to a selective masking around a text.
  * A selective masking only makes sense in contexts where the text is rendered over some other map layers, especially for labeling.
  * \note QgsTextMaskSettings objects are implicitly shared.
  * \since QGIS 3.12
 */

class CORE_EXPORT QgsTextMaskSettings
{
  public:

    /**
     * Mask shape types.
     */
    enum MaskType
    {
      MaskBuffer = 0 //!< Buffer
    };

    QgsTextMaskSettings();

    /**
     * Copy constructor.
     * \param other source settings
     */
    QgsTextMaskSettings( const QgsTextMaskSettings &other );

    /**
     * Copy constructor.
     * \param other source QgsTextMaskSettings
     */
    QgsTextMaskSettings &operator=( const QgsTextMaskSettings &other );

    ~QgsTextMaskSettings();

    bool operator==( const QgsTextMaskSettings &other ) const;
    bool operator!=( const QgsTextMaskSettings &other ) const;

    /**
     * Returns whether the mask is enabled.
     */
    bool enabled() const;

    /**
     * Returns whether the mask is enabled.
     */
    void setEnabled( bool );

    /**
     * Returns the type of mask shape.
     * \see setType()
     */
    MaskType type() const;

    /**
     * Sets the type of mask shape.
     * \param type shape type
     * \see type()
     */
    void setType( MaskType type );

    /**
     * Returns the size of the buffer.
     * \see sizeUnit()
     * \see setSize()
     */
    double size() const;

    /**
     * Sets the size of the buffer. The size units are specified using setSizeUnit().
     * \param size buffer size
     * \see size()
     * \see setSizeUnit()
     */
    void setSize( double size );

    /**
     * Returns the units for the buffer size.
     * \see size()
     * \see setSizeUnit()
     */
    QgsUnitTypes::RenderUnit sizeUnit() const;

    /**
     * Sets the units used for the buffer size.
     * \param unit size unit
     * \see setSize()
     * \see sizeUnit()
     */
    void setSizeUnit( QgsUnitTypes::RenderUnit unit );

    /**
     * Returns the map unit scale object for the buffer size. This is only used if the
     * buffer size is set to QgsUnitTypes::RenderMapUnit.
     * \see setSizeMapUnitScale()
     * \see sizeUnit()
     */
    QgsMapUnitScale sizeMapUnitScale() const;

    /**
     * Sets the map unit scale object for the buffer size. This is only used if the
     * buffer size is set to QgsUnitTypes::RenderMapUnit.
     * \param scale scale for buffer size
     * \see sizeMapUnitScale()
     * \see setSizeUnit()
     */
    void setSizeMapUnitScale( const QgsMapUnitScale &scale );

    /**
     * Returns the buffer join style.
     * \see setJoinStyle
     */
    Qt::PenJoinStyle joinStyle() const;

    /**
     * Sets the join style used for drawing the buffer.
     * \param style join style
     * \see joinStyle()
     */
    void setJoinStyle( Qt::PenJoinStyle style );

    /**
     * Returns the mask's opacity. The opacity is a double value between 0 (fully transparent) and 1 (totally
     * opaque).
     * \see setOpacity()
     */
    double opacity() const;

    /**
     * Sets the mask's opacity.
     * \param opacity opacity as a double value between 0 (fully transparent) and 1 (totally
     * opaque)
     * \see opacity()
     */
    void setOpacity( double opacity );

    /**
     * Returns the current paint effect for the mask.
     * \returns paint effect
     * \see setPaintEffect()
     */
    QgsPaintEffect *paintEffect() const;

    /**
     * Sets the current paint \a effect for the mask.
     * \param effect paint effect. Ownership is transferred to the mask settings.
     * \see paintEffect()
     */
    void setPaintEffect( QgsPaintEffect *effect SIP_TRANSFER );

    /**
     * Read settings from a DOM element.
     * \see writeXml()
     */
    void readXml( const QDomElement &elem );

    /**
     * Write settings into a DOM element.
     * \see readXml()
     */
    QDomElement writeXml( QDomDocument &doc ) const;

    /**
     * Returns a list of references to symbol layers that are masked by this buffer.
     * \returns a list of references to masked symbol layers
     * \see setMaskedSymbolLayers
     */
    QgsSymbolLayerReferenceList maskedSymbolLayers() const;

    /**
     * Sets the symbol layers that will be masked by this buffer.
     * \param maskedLayers list of references to symbol layers
     * \see setMaskedSymbolLayers
     */
    void setMaskedSymbolLayers( QgsSymbolLayerReferenceList maskedLayers );

    /**
     * Updates the format by evaluating current values of data defined properties.
     */
    void updateDataDefinedProperties( QgsRenderContext &context, const QgsPropertyCollection &properties );

    /**
     * Returns all field names referenced by the configuration (e.g. from data defined properties).
     * \since QGIS 3.14
     */
    QSet<QString> referencedFields( const QgsRenderContext &context ) const;

  private:

    QSharedDataPointer<QgsTextMaskSettingsPrivate> d;
};

#endif // QGSTEXTMASKSETTINGS_H
