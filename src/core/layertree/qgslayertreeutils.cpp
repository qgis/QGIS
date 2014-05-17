#include "qgslayertreeutils.h"

#include "qgslayertree.h"

#include "qgsvectorlayer.h"

#include <QDomElement>

bool QgsLayerTreeUtils::readOldLegend(QgsLayerTreeGroup* root, const QDomElement& legendElem)
{
  if (legendElem.isNull())
    return false;

  QDomNodeList legendChildren = legendElem.childNodes();

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

  return true;
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

  QgsLayerTreeGroup* groupNode = new QgsLayerTreeGroup(groupElem.attribute( "name" ));
  parent->addChildNode(groupNode);

  groupNode->setVisible(checkStateFromXml(groupElem.attribute("checked")));

  if (groupElem.attribute("embedded") == "1")
  {
    groupNode->setCustomProperty("embedded", true);
    groupNode->setCustomProperty("embedded_project", groupElem.attribute("project"));
  }

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
  QString layerId = layerElem.firstChildElement( "filegroup" ).firstChildElement( "legendlayerfile" ).attribute( "layerid" );
  QgsLayerTreeLayer* layerNode = new QgsLayerTreeLayer(layerId, layerElem.attribute( "name" ) );

  layerNode->setVisible(checkStateFromXml(layerElem.attribute("checked")));

  if (layerElem.attribute("embedded") == "1")
    layerNode->setCustomProperty("embedded", true);

  // TODO: is in overview, drawing order, show feature count

  parent->addChildNode(layerNode);
}



bool QgsLayerTreeUtils::layersEditable( const QList<QgsLayerTreeLayer*>& layerNodes )
{
  foreach ( QgsLayerTreeLayer* layerNode, layerNodes )
  {
    QgsVectorLayer *vl = qobject_cast<QgsVectorLayer*>( layerNode->layer() );
    if ( !vl )
      continue;

    if ( vl->isEditable() )
      return true;
  }
  return false;
}

bool QgsLayerTreeUtils::layersModified( const QList<QgsLayerTreeLayer*>& layerNodes )
{
  foreach ( QgsLayerTreeLayer* layerNode, layerNodes )
  {
    QgsVectorLayer *vl = qobject_cast<QgsVectorLayer*>( layerNode->layer() );
    if ( !vl )
      continue;

    if ( vl->isEditable() && vl->isModified() )
      return true;
  }
  return false;
}
