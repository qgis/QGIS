/***************************************************************************
  qgscolorramplegendnode.h
  --------------------------------------
  Date                 : December 2020
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

#ifndef QGSCOLORRAMPLEGENDNODESETTINGS_H
#define QGSCOLORRAMPLEGENDNODESETTINGS_H

#include "qgis_core.h"
#include "qgis_sip.h"

#include <QString>
#include <memory>

class QgsNumericFormat;
class QgsReadWriteContext;
class QDomDocument;
class QDomElement;

/**
 * \ingroup core
 * Settings for a color ramp legend node.
 *
 * \since QGIS 3.18
 */
class CORE_EXPORT QgsColorRampLegendNodeSettings
{
  public:

    /**
     * Ramp directions
     */
    enum Direction
    {
      MinimumToMaximum, //!< Minimum value on bottom, maximum value on top
      MaximumToMinimum,//!< Maximum value on bottom, minimum value on top
    };

    QgsColorRampLegendNodeSettings();

    ~QgsColorRampLegendNodeSettings();

    //! Copy constructor
    QgsColorRampLegendNodeSettings( const QgsColorRampLegendNodeSettings &other );

    QgsColorRampLegendNodeSettings &operator=( const QgsColorRampLegendNodeSettings &other );

    /**
     * Returns the direction of the ramp.
     *
     * \see setDirection()
     */
    QgsColorRampLegendNodeSettings::Direction direction() const;

    /**
     * Sets the \a direction of the ramp.
     *
     * \see direction()
     */
    void setDirection( QgsColorRampLegendNodeSettings::Direction direction );

    /**
     * Returns the label for the minimum value on the ramp.
     *
     * If the returned string is empty than a default value will be generated based on the associated minimum value.
     *
     * \see maximumLabel()
     * \see setMinimumLabel()
     */
    QString minimumLabel() const;

    /**
     * Sets the \a label for the minimum value on the ramp.
     *
     * If the \a label is empty than a default value will be generated based on the associated minimum value.
     *
     * \see setMaximumLabel()
     * \see minimumLabel()
     */
    void setMinimumLabel( const QString &label );

    /**
     * Returns the label for the maximum value on the ramp.
     *
     * If the returned string is empty than a default value will be generated based on the associated maximum value.
     *
     * \see minimumLabel()
     * \see setMaximumLabel()
     */
    QString maximumLabel() const;

    /**
     * Sets the \a label for the maximum value on the ramp.
     *
     * If the \a label is empty than a default value will be generated based on the associated maximum value.
     *
     * \see setMinimumLabel()
     * \see maximumLabel()
     */
    void setMaximumLabel( const QString &label );

    /**
     * Returns the numeric format used for numbers in the scalebar.
     *
     * \see setNumericFormat()
     * \since QGIS 3.12
     */
    const QgsNumericFormat *numericFormat() const;

    /**
     * Sets the numeric \a format used for numbers in the scalebar.
     *
     * Ownership of \a format is transferred to the scalebar.
     *
     * \see numericFormat()
     * \since QGIS 3.12
     */
    void setNumericFormat( QgsNumericFormat *format SIP_TRANSFER );

    /**
     * Writes settings to an XML \a element.
     */
    void writeXml( QDomDocument &doc, QDomElement &element, const QgsReadWriteContext &context ) const;

    /**
     * Reads settings from an XML \a element.
     */
    void readXml( const QDomElement &element, const QgsReadWriteContext &context );

    /**
     * Returns the prefix to show before legend text.
     *
     * \see setPrefix()
     * \see suffix()
     */
    QString prefix() const;

    /**
     * Sets the \a prefix to show before legend text.
     *
     * \see prefix()
     * \see setSuffix()
     */
    void setPrefix( const QString &prefix );

    /**
     * Returns the suffix to show after legend text.
     *
     * \see setSuffix()
     * \see prefix()
     */
    QString suffix() const;

    /**
     * Sets the \a suffix to show after legend text.
     *
     * \see suffix()
     * \see setPrefix()
     */
    void setSuffix( const QString &suffix );

  private:
    QString mMinimumLabel;
    QString mMaximumLabel;
    QString mPrefix;
    QString mSuffix;
    Direction mDirection = MinimumToMaximum;
    std::unique_ptr< QgsNumericFormat > mNumericFormat;
};

#endif // QGSCOLORRAMPLEGENDNODESETTINGS_H
