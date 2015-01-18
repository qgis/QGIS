#include <QDomNode>
#include <QFileInfo>
#include <QFile>
#include <QDir>
#include <QTextStream>

#include "qgslogger.h"
#include "qgsmaplayer.h"
#include "qgslayertree.h"
#include "qgsmaplayerregistry.h"
#include "qgslayerdefinition.h"

bool QgsLayerDefinition::loadLayerDefinition( const QString &path, QgsLayerTreeGroup *rootGroup, QString &errorMessage )
{
  QFile file( path );
  if ( !file.open( QIODevice::ReadOnly ) )
  {
    errorMessage = QString( "Can not open file" );
    return false;
  }

  QDomDocument doc;
  QString message;
  if ( !doc.setContent( &file, &message ) )
  {
    errorMessage = message;
    return false;
  }

  QFileInfo fileinfo( file );
  QDir::setCurrent( fileinfo.absoluteDir().path() );

  return loadLayerDefinition( doc, rootGroup, errorMessage );
}

bool QgsLayerDefinition::loadLayerDefinition( QDomDocument doc, QgsLayerTreeGroup *rootGroup, QString &errorMessage )
{
  QgsLayerTreeGroup* root = new QgsLayerTreeGroup;
  // We have to replace the IDs before we load them because it's too late once they are loaded
  QDomNodeList ids = doc.elementsByTagName( "id" );
  for ( int i = 0; i < ids.size(); ++i )
  {
    QDomNode idnode = ids.at( i );
    QDomElement idElem = idnode.toElement();
    QString oldid = idElem.text();
    // Strip the date part because we will replace it.
    QString layername = oldid.left( oldid.length() - 17 );
    QDateTime dt = QDateTime::currentDateTime();
    QString newid = layername + dt.toString( "yyyyMMddhhmmsszzz" );
    idElem.firstChild().setNodeValue( newid );
    QDomNodeList treeLayerNodes = doc.elementsByTagName( "layer-tree-layer" );

    for ( int i = 0; i < treeLayerNodes.count(); ++i )
    {
      QDomNode layerNode = treeLayerNodes.at( i );
      QDomElement layerElem = layerNode.toElement();
      if ( layerElem.attribute( "id" ) == oldid )
      {
        layerNode.toElement().setAttribute( "id", newid );
      }
    }
  }

  QDomElement layerTreeElem = doc.documentElement().firstChildElement( "layer-tree-group" );
  bool loadInLegend = true;
  if ( !layerTreeElem.isNull() )
  {
    root->readChildrenFromXML( layerTreeElem );
    loadInLegend = false;
  }

  QList<QgsMapLayer*> layers = QgsMapLayer::fromLayerDefinition( doc );
  QgsMapLayerRegistry::instance()->addMapLayers( layers, loadInLegend );

  QList<QgsLayerTreeNode*> nodes = root->children();
  rootGroup->insertChildNodes( -1, nodes );
  return true;

}

bool QgsLayerDefinition::exportLayerDefinition( QString path, QList<QgsLayerTreeNode*> selectedTreeNodes, QString &errorMessage )
{
  if ( !path.endsWith( ".qlr" ) )
    path = path.append( ".qlr" );

  QFile file( path );
  QFileInfo fileinfo( file );

  QDomDocument doc( "qgis-layer-definition" );
  QDomElement qgiselm = doc.createElement( "qlr" );
  doc.appendChild( qgiselm );
  QList<QgsLayerTreeNode*> nodes =  selectedTreeNodes;
  QgsLayerTreeGroup* root = new QgsLayerTreeGroup;
  foreach ( QgsLayerTreeNode* node, nodes )
  {
    QgsLayerTreeNode* newnode = node->clone();
    root->addChildNode( newnode );
  }
  root->writeXML( qgiselm );

  QDomElement layerselm = doc.createElement( "maplayers" );
  QList<QgsLayerTreeLayer*> layers = root->findLayers();
  foreach ( QgsLayerTreeLayer* layer, layers )
  {
    QDomElement layerelm = doc.createElement( "maplayer" );
    layer->layer()->writeLayerXML( layerelm, doc, fileinfo.canonicalFilePath() );
    layerselm.appendChild( layerelm );
  }
  qgiselm.appendChild( layerselm );

  if ( file.open( QFile::WriteOnly | QFile::Truncate ) )
  {
    QTextStream qlayerstream( &file );
    doc.save( qlayerstream, 2 );
    return true;
  }
  else
  {
    errorMessage = file.errorString();
    return false;
  }
}
