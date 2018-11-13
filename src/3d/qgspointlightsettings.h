/***************************************************************************
  qgspointlightsettings.h
  --------------------------------------
  Date                 : November 2018
  Copyright            : (C) 2018 by Martin Dobias
  Email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSPOINTLIGHTSETTINGS_H
#define QGSPOINTLIGHTSETTINGS_H

#include "qgsvector3d.h"
#include <QColor>

class QDomDocument;
class QDomElement;

/**
 * \ingroup 3d
 * Definition of a point light in a 3D map scene
 *
 * Total light at the distance D from a point light with intensity I
 * is (I / TA) where TA is total attenuation which is calculated as
 * (A_0 + A_1 * D + A_2 * D^2). The terms A_0, A_1 and A_2 stand for
 * constant, linear and quadratic attenuation.
 *
 * \since QGIS 3.6
 */
class QgsPointLightSettings
{
  public:
    //! Construct a point light with default values
    QgsPointLightSettings() = default;

    //! Returns position of the light (in 3D world coordinates)
    QgsVector3D position() const { return mPosition; }
    //! Sets position of the light (in 3D world coordinates)
    void setPosition( const QgsVector3D &pos ) { mPosition = pos; }

    //! Returns color of the light
    QColor color() const { return mColor; }
    //! Sets color of the light
    void setColor( const QColor &color ) { mColor = color; }

    //! Returns intensity of the light
    float intensity() const { return mIntensity; }
    //! Sets intensity of the light
    void setIntensity( float intensity ) { mIntensity = intensity; }

    //! Returns constant attenuation (A_0)
    float constantAttenuation() const { return mConstantAttenuation; }
    //! Sets constant attenuation (A_0)
    void setConstantAttenuation( float value ) { mConstantAttenuation = value; }

    //! Returns linear attenuation (A_1)
    float linearAttenuation() const { return mLinearAttenuation; }
    //! Sets linear attenuation (A_1)
    void setLinearAttenuation( float value ) { mLinearAttenuation = value; }

    //! Returns quadratic attenuation (A_2)
    float quadraticAttenuation() const { return mQuadraticAttenuation; }
    //! Sets quadratic attenuation (A_2)
    void setQuadraticAttenuation( float value ) { mQuadraticAttenuation = value; }

    //! Writes configuration to a new DOM element and returns it
    QDomElement writeXml( QDomDocument &doc ) const;
    //! Reads configuration from a DOM element previously written using writeXml()
    void readXml( const QDomElement &elem );

    bool operator==( const QgsPointLightSettings &other );

  private:
    QgsVector3D mPosition;
    QColor mColor = Qt::white;
    float mIntensity = 0.5;
    float mConstantAttenuation = 1.0f;
    float mLinearAttenuation = 0.0f;
    float mQuadraticAttenuation = 0.0f;
};

#endif // QGSPOINTLIGHTSETTINGS_H
