#ifndef PHONGMATERIALSETTINGS_H
#define PHONGMATERIALSETTINGS_H

#include "qgis_3d.h"

#include <QColor>

class QDomElement;

//! Basic shading material used for rendering
class _3D_EXPORT PhongMaterialSettings
{
  public:
    PhongMaterialSettings()
      : mAmbient( QColor::fromRgbF( 0.1f, 0.1f, 0.1f, 1.0f ) )
      , mDiffuse( QColor::fromRgbF( 0.7f, 0.7f, 0.7f, 1.0f ) )
      , mSpecular( QColor::fromRgbF( 1.0f, 1.0f, 1.0f, 1.0f ) )
      , mShininess( 0.0f )
    {
    }

    QColor ambient() const { return mAmbient; }
    QColor diffuse() const { return mDiffuse; }
    QColor specular() const { return mSpecular; }
    float shininess() const { return mShininess; }

    void setAmbient( const QColor &ambient ) { mAmbient = ambient; }
    void setDiffuse( const QColor &diffuse ) { mDiffuse = diffuse; }
    void setSpecular( const QColor &specular ) { mSpecular = specular; }
    void setShininess( float shininess ) { mShininess = shininess; }

    void readXml( const QDomElement &elem );
    void writeXml( QDomElement &elem ) const;

  private:
    QColor mAmbient;
    QColor mDiffuse;
    QColor mSpecular;
    float mShininess;
};


#endif // PHONGMATERIALSETTINGS_H
