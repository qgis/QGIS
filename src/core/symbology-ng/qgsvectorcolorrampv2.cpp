
#include "qgsvectorcolorrampv2.h"

#include "qgssymbollayerv2utils.h"

QgsVectorGradientColorRampV2::QgsVectorGradientColorRampV2(QColor color1, QColor color2)
  : mColor1(color1), mColor2(color2)
{
}

QgsVectorColorRampV2* QgsVectorGradientColorRampV2::create(const QgsStringMap& props)
{
  QColor color1 = DEFAULT_GRADIENT_COLOR1;
  QColor color2 = DEFAULT_GRADIENT_COLOR2;
  if (props.contains("color1"))
    color1 = QgsSymbolLayerV2Utils::decodeColor(props["color1"]);
  if (props.contains("color2"))
    color2 = QgsSymbolLayerV2Utils::decodeColor(props["color2"]);
  return new QgsVectorGradientColorRampV2(color1, color2);
}

QColor QgsVectorGradientColorRampV2::color(double value) const
{
  int r = (int) ( mColor1.red() + value*(mColor2.red() - mColor1.red()) );
  int g = (int) ( mColor1.green() + value*(mColor2.green() - mColor1.green()) );
  int b = (int) ( mColor1.blue() + value*(mColor2.blue() - mColor1.blue()) );
  
  return QColor::fromRgb(r,g,b);
}

QgsVectorColorRampV2* QgsVectorGradientColorRampV2::clone() const
{
  return new QgsVectorGradientColorRampV2(mColor1, mColor2);
}

QgsStringMap QgsVectorGradientColorRampV2::properties() const
{
  QgsStringMap map;
  map["color1"] = QgsSymbolLayerV2Utils::encodeColor(mColor1);
  map["color2"] = QgsSymbolLayerV2Utils::encodeColor(mColor2);
  return map;
}
