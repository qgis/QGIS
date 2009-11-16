
#include "qgsvectorcolorrampv2.h"

#include "qgssymbollayerv2utils.h"

#include <stdlib.h> // for random()

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

//////////////


QgsVectorRandomColorRampV2::QgsVectorRandomColorRampV2(int count, int hueMin, int hueMax,
                           int satMin, int satMax, int valMin, int valMax )
 : mCount(count), mHueMin(hueMin), mHueMax(hueMax),
   mSatMin(satMin), mSatMax(satMax), mValMin(valMin), mValMax(valMax)
{
  updateColors();
}

QgsVectorColorRampV2* QgsVectorRandomColorRampV2::create(const QgsStringMap& props)
{
  int count = DEFAULT_RANDOM_COUNT;
  int hueMin = DEFAULT_RANDOM_HUE_MIN, hueMax = DEFAULT_RANDOM_HUE_MAX;
  int satMin = DEFAULT_RANDOM_SAT_MIN, satMax = DEFAULT_RANDOM_SAT_MAX;
  int valMin = DEFAULT_RANDOM_VAL_MIN, valMax = DEFAULT_RANDOM_VAL_MAX;

  if (props.contains("count")) count = props["count"].toInt();
  if (props.contains("hueMin")) hueMin = props["hueMin"].toInt();
  if (props.contains("hueMax")) hueMax = props["hueMax"].toInt();
  if (props.contains("satMin")) satMin = props["satMin"].toInt();
  if (props.contains("satMax")) satMax = props["satMax"].toInt();
  if (props.contains("valMin")) valMin = props["valMin"].toInt();
  if (props.contains("valMax")) valMax = props["valMax"].toInt();

  return new QgsVectorRandomColorRampV2(count, hueMin, hueMax, satMin, satMax, valMin, valMax);
}

QColor QgsVectorRandomColorRampV2::color(double value) const
{
  int colorCnt = mColors.count();
  int colorIdx = (int) ( value * colorCnt );

  if (colorIdx >= 0 && colorIdx < colorCnt)
    return mColors.at( colorIdx );

  return QColor();
}

QgsVectorColorRampV2* QgsVectorRandomColorRampV2::clone() const
{
  return new QgsVectorRandomColorRampV2(mCount, mHueMin, mHueMax, mSatMin, mSatMax, mValMin, mValMax);
}

QgsStringMap QgsVectorRandomColorRampV2::properties() const
{
  QgsStringMap map;
  map["count"] = QString::number(mCount);
  map["hueMin"] = QString::number(mHueMin);
  map["hueMax"] = QString::number(mHueMax);
  map["satMin"] = QString::number(mSatMin);
  map["satMax"] = QString::number(mSatMax);
  map["valMin"] = QString::number(mValMin);
  map["valMax"] = QString::number(mValMax);
  return map;
}

void QgsVectorRandomColorRampV2::updateColors()
{
  int h,s,v;

  mColors.clear();
  for (int i = 0; i < mCount; i++)
  {
    h = (random() % (mHueMax-mHueMin+1)) + mHueMin;
    s = (random() % (mSatMax-mSatMin+1)) + mSatMin;
    v = (random() % (mValMax-mValMin+1)) + mValMin;
    mColors.append( QColor::fromHsv(h,s,v) );
  }
}


////////////

QgsVectorColorBrewerColorRampV2::QgsVectorColorBrewerColorRampV2(QString schemeName, int colors)
 : mSchemeName(schemeName), mColors(colors)
{
  loadPalette();
}

QgsVectorColorRampV2* QgsVectorColorBrewerColorRampV2::create(const QgsStringMap& props)
{
  QString schemeName = DEFAULT_COLORBREWER_SCHEMENAME;
  int colors = DEFAULT_COLORBREWER_COLORS;

  if (props.contains("schemeName"))
    schemeName = props["schemeName"];
  if (props.contains("colors"))
    colors = props["colors"].toInt();

  return new QgsVectorColorBrewerColorRampV2(schemeName, colors);
}

#include "qgscolorbrewerpalette.h"

void QgsVectorColorBrewerColorRampV2::loadPalette()
{
  mPalette = QgsColorBrewerPalette::listSchemeColors(mSchemeName, mColors);
}

QStringList QgsVectorColorBrewerColorRampV2::listSchemeNames()
{
  return QgsColorBrewerPalette::listSchemes();
}

QList<int> QgsVectorColorBrewerColorRampV2::listSchemeVariants(QString schemeName)
{
  return QgsColorBrewerPalette::listSchemeVariants(schemeName);
}

QColor QgsVectorColorBrewerColorRampV2::color(double value) const
{
  if (mPalette.isEmpty() || value < 0 || value > 1)
    return QColor(255,0,0); // red color as a warning :)

  int paletteEntry = (int) (value * mPalette.count());
  if (paletteEntry > mPalette.count())
    paletteEntry = mPalette.count()-1;
  return mPalette.at(paletteEntry);
}

QgsVectorColorRampV2* QgsVectorColorBrewerColorRampV2::clone() const
{
  return new QgsVectorColorBrewerColorRampV2(mSchemeName, mColors);
}

QgsStringMap QgsVectorColorBrewerColorRampV2::properties() const
{
  QgsStringMap map;
  map["schemeName"] = mSchemeName;
  map["colors"] = QString::number(mColors);
  return map;
}
