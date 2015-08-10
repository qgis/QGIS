
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

QgsConditionalStyle QgsFieldUIProperties::matchingConditionalStyle( QgsFeature *feature, QgsFields fields )
{
  foreach ( QgsConditionalStyle style, mStyles )
  {
    if ( style.matchForFeature( feature, fields ) )
      return style;
  }
  return QgsConditionalStyle();
}
