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

QList<QgsConditionalStyle> QgsFieldUIProperties::getConditionalStyles() const
{
  return mStyles;
}

QList<QgsConditionalStyle> QgsFieldUIProperties::matchingConditionalStyles( QVariant value, QgsExpressionContext& context ) const
{
  QList<QgsConditionalStyle> styles;
  foreach ( QgsConditionalStyle style, mStyles )
  {
    if ( style.matches( value, context ) )
      styles.append( style );
  }
  return styles;
}

QgsConditionalStyle QgsFieldUIProperties::matchingConditionalStyle( QVariant value, QgsExpressionContext& context ) const
{
  foreach ( QgsConditionalStyle style, mStyles )
  {
    if ( style.matches( value, context ) )
      return style;
  }
  return QgsConditionalStyle();
}

bool QgsFieldUIProperties::writeXml( QDomNode &node, QDomDocument &doc ) const
{
  QDomElement stylesel = doc.createElement( "conditionalstyles" );
  foreach ( QgsConditionalStyle style, mStyles )
  {
    style.writeXml( stylesel, doc );
  }
  node.appendChild( stylesel );
  return true;
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
  return true;
}

