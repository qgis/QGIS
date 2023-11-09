/***************************************************************************
    qgscolorramp.h
    ---------------------
    begin                : November 2009
    copyright            : (C) 2009 by Martin Dobias
    email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSCOLORRAMP_H
#define QGSCOLORRAMP_H

#include "qgis_core.h"
#include <QColor>
#include "qgis.h"

/**
 * \ingroup core
 * \class QgsColorRamp
 * \brief Abstract base class for color ramps
 * \since QGIS 3.0
 */
class CORE_EXPORT QgsColorRamp
{

#ifdef SIP_RUN
    SIP_CONVERT_TO_SUBCLASS_CODE
    if ( sipCpp->type() == QgsGradientColorRamp::typeString() )
      sipType = sipType_QgsGradientColorRamp;
    else if ( sipCpp->type() == QgsLimitedRandomColorRamp::typeString() )
      sipType = sipType_QgsLimitedRandomColorRamp;
    else if ( sipCpp->type() == QgsRandomColorRamp::typeString() )
      sipType = sipType_QgsRandomColorRamp;
    else if ( sipCpp->type() == QgsPresetSchemeColorRamp::typeString() )
      sipType = sipType_QgsPresetSchemeColorRamp;
    else if ( sipCpp->type() == QgsColorBrewerColorRamp::typeString() )
      sipType = sipType_QgsColorBrewerColorRamp;
    else if ( sipCpp->type() == QgsCptCityColorRamp::typeString() )
      sipType = sipType_QgsCptCityColorRamp;
    else
      sipType = 0;
    SIP_END
#endif
  public:

    virtual ~QgsColorRamp();

    /**
     * Returns number of defined colors, or -1 if undefined
     */
    virtual int count() const = 0;

    /**
     * Returns relative value between [0,1] of color at specified index
     */
    virtual double value( int index ) const = 0;

    /**
     * Returns the color corresponding to a specified value.
     * \param value value between [0, 1] inclusive
     * \returns color for value
     */
    virtual QColor color( double value ) const = 0;

    /**
     * Returns a string representing the color ramp type.
     */
    virtual QString type() const = 0;


    /**
     * Inverts the ordering of the color ramp.
     */
    virtual void invert() {}

    /**
     * Creates a clone of the color ramp.
     */
    virtual QgsColorRamp *clone() const = 0 SIP_FACTORY;

    /**
     * Returns a string map containing all the color ramp's properties.
     */
    virtual QVariantMap properties() const = 0;

    /**
     * Returns a list of available ramp types, where the first value in each item is the QgsColorRamp::type() string
     * and the second is a user friendly, translated name for the color ramp type.
     *
     * The ramp types are returned in a order of precedence for exposing in UI, with more commonly used types
     * listed first.
     *
     * \since QGIS 3.16
     */
    static QList< QPair< QString, QString > > rampTypes();
};

#endif
