#include "qgslayertreeutils.h"

#include "qgslayertreenode.h"

#include <QDomElement>

QgsLayerTreeGroup* QgsLayerTreeUtils::readOldLegend(const QDomElement& legendElem)
{
  QDomNodeList legendChildren = legendElem.childNodes();
  QgsLayerTreeGroup* root = new QgsLayerTreeGroup;

  for ( int i = 0; i < legendChildren.size(); ++i )
  {
    QDomElement currentChildElem = legendChildren.at( i ).toElement();
    if ( currentChildElem.tagName() == "legendlayer" )
    {
      addLegendLayerToTreeWidget( currentChildElem, root );
    }
    else if ( currentChildElem.tagName() == "legendgroup" )
    {
      addLegendGroupToTreeWidget( currentChildElem, root );
    }
  }

  return root;
}

QString QgsLayerTreeUtils::checkStateToXml(Qt::CheckState state)
{
  switch (state)
  {
  case Qt::Unchecked:        return "Qt::Unchecked";
  case Qt::PartiallyChecked: return "Qt::PartiallyChecked";
  case Qt::Checked: default: return "Qt::Checked";
  }
}

Qt::CheckState QgsLayerTreeUtils::checkStateFromXml(QString txt)
{
  if (txt == "Qt::Unchecked")
    return Qt::Unchecked;
  else if (txt == "Qt::PartiallyChecked")
    return Qt::PartiallyChecked;
  else // "Qt::Checked"
    return Qt::Checked;
}



void QgsLayerTreeUtils::addLegendGroupToTreeWidget( const QDomElement& groupElem, QgsLayerTreeGroup* parent )
{
  QDomNodeList groupChildren = groupElem.childNodes();

  // TODO: embedded
  //if ( !mShowEmbeddedContent && groupElem.attribute( "embedded" ) == "1" )
  //  return;

  QgsLayerTreeGroup* groupNode = new QgsLayerTreeGroup(groupElem.attribute( "name" ));
  parent->addChildNode(groupNode);

  for ( int i = 0; i < groupChildren.size(); ++i )
  {
    QDomElement currentChildElem = groupChildren.at( i ).toElement();
    if ( currentChildElem.tagName() == "legendlayer" )
    {
      addLegendLayerToTreeWidget( currentChildElem, groupNode );
    }
    else if ( currentChildElem.tagName() == "legendgroup" )
    {
      addLegendGroupToTreeWidget( currentChildElem, groupNode );
    }
  }
}

void QgsLayerTreeUtils::addLegendLayerToTreeWidget( const QDomElement& layerElem, QgsLayerTreeGroup* parent )
{
  // TODO: embedded
  //if ( !mShowEmbeddedContent && layerElem.attribute( "embedded" ) == "1" )
  //  return;

  QString layerId = layerElem.firstChildElement( "filegroup" ).firstChildElement( "legendlayerfile" ).attribute( "layerid" );
  QgsLayerTreeLayer* layerNode = new QgsLayerTreeLayer(layerId, layerElem.attribute( "name" ) );
  parent->addChildNode(layerNode);
}
