
#include "qgsconditionalstyle.h"
#include "qgsfielduiproperties.h"


QgsFieldUIProperties::QgsFieldUIProperties()
    : mStyles( QList<QgsConditionalStyle>() )
{}

void QgsFieldUIProperties::setConditionalStyles( QList<QgsConditionalStyle> styles )
{
  mStyles = styles;
}

QList<QgsConditionalStyle> QgsFieldUIProperties::getConditionalStyles()
{
  return mStyles;
}

QgsConditionalStyle QgsFieldUIProperties::matchingConditionalStyle( QVariant value,  QgsFeature *feature )
{
  foreach ( QgsConditionalStyle style, mStyles )
  {
    if ( style.matches( value, feature ) )
      return style;
  }
  return QgsConditionalStyle();
}
