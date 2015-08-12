#include <QDomElement>

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

void QgsFieldUIProperties::writeXml(QDomNode &layer_node, QDomDocument &doc)
{
  QDomElement stylesel = document.createElement( "conditionalstyles" );
  layer_node.appendChild( stylesel );
  foreach ( QgsConditionalStyle style, mStyles)
    {
      style.writeXml(layer_node, doc);
    }
}

void QgsFieldUIProperties::readXml(const QDomNode &layer_node)
{

}

