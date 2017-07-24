#include "phongmaterialsettings.h"

#include "qgssymbollayerutils.h"


void PhongMaterialSettings::readXml( const QDomElement &elem )
{
  mAmbient = QgsSymbolLayerUtils::decodeColor( elem.attribute( "ambient" ) );
  mDiffuse = QgsSymbolLayerUtils::decodeColor( elem.attribute( "diffuse" ) );
  mSpecular = QgsSymbolLayerUtils::decodeColor( elem.attribute( "specular" ) );
  mShininess = elem.attribute( "shininess" ).toFloat();
}

void PhongMaterialSettings::writeXml( QDomElement &elem ) const
{
  elem.setAttribute( "ambient", QgsSymbolLayerUtils::encodeColor( mAmbient ) );
  elem.setAttribute( "diffuse", QgsSymbolLayerUtils::encodeColor( mDiffuse ) );
  elem.setAttribute( "specular", QgsSymbolLayerUtils::encodeColor( mSpecular ) );
  elem.setAttribute( "shininess", mShininess );
}
