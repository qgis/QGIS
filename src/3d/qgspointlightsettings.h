#ifndef QGSPOINTLIGHTSETTINGS_H
#define QGSPOINTLIGHTSETTINGS_H

#include "qgsvector3d.h"
#include <QColor>

class QDomDocument;
class QDomElement;

/**
 * \ingroup 3d
 * Definition of a point light in a 3D map scene
 * \since QGIS 3.6
 */
class QgsPointLightSettings
{
  public:
    QgsPointLightSettings() = default;

    //! Returns position of the light (in 3D world coordinates)
    QgsVector3D position() const { return mPosition; }
    //! Sets position of the light (in 3D world coordinates)
    void setPosition( const QgsVector3D &pos ) { mPosition = pos; }

    //! Returns color of the light
    QColor color() const { return mColor; }
    //! Sets color of the light
    void setColor( const QColor &c ) { mColor = c; }

    //! Returns intensity of the light
    float intensity() const { return mIntensity; }
    //! Sets intensity of the light
    void setIntensity( float intensity ) { mIntensity = intensity; }

    float constantAttenuation() const { return mConstantAttenuation; }
    void setConstantAttenuation( float value ) { mConstantAttenuation = value; }

    float linearAttenuation() const { return mLinearAttenuation; }
    void setLinearAttenuation( float value ) { mLinearAttenuation = value; }

    float quadraticAttenuation() const { return mQuadraticAttenuation; }
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
