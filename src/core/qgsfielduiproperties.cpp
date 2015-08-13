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

bool QgsFieldUIProperties::writeXml( QDomNode &node, QDomDocument &doc )
{
  QDomElement stylesel = doc.createElement( "conditionalstyles" );
  foreach ( QgsConditionalStyle style, mStyles )
  {
    style.writeXml( stylesel, doc );
  }
  node.appendChild( stylesel );
}

bool QgsFieldUIProperties::readXml( const QDomNode &node )
{
  mStyles.clear();
  QDomElement condel = node.firstChildElement( "conditionalstyles" );
  QDomNodeList stylesList = condel.elementsByTagName( "style" );
  for ( int i = 0; i < stylesList.size(); ++i )
  {
    QDomElement styleElm = stylesList.at( i ).toElement();
    QgsConditionalStyle style = QgsConditionalStyle();
    style.readXml( styleElm );
    mStyles.append( style );
  }
}

