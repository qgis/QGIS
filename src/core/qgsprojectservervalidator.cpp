#include "qgsapplication.h"
#include "qgslayertreelayer.h"
#include "qgsprojectservervalidator.h"
#include "qgsvectorlayer.h"


void QgsProjectServerValidator::checkOWS( QgsLayerTreeGroup *treeGroup, QStringList &owsNames, QStringList &encodingMessages )
{
  QList< QgsLayerTreeNode * > treeGroupChildren = treeGroup->children();
  for ( int i = 0; i < treeGroupChildren.size(); ++i )
  {
    QgsLayerTreeNode *treeNode = treeGroupChildren.at( i );
    if ( treeNode->nodeType() == QgsLayerTreeNode::NodeGroup )
    {
      QgsLayerTreeGroup *treeGroupChild = static_cast<QgsLayerTreeGroup *>( treeNode );
      QString shortName = treeGroupChild->customProperty( QStringLiteral( "wmsShortName" ) ).toString();
      if ( shortName.isEmpty() )
        owsNames << treeGroupChild->name();
      else
        owsNames << shortName;
      checkOWS( treeGroupChild, owsNames, encodingMessages );
    }
    else
    {
      QgsLayerTreeLayer *treeLayer = static_cast<QgsLayerTreeLayer *>( treeNode );
      QgsMapLayer *l = treeLayer->layer();
      if ( l )
      {
        QString shortName = l->shortName();
        if ( shortName.isEmpty() )
          owsNames << l->name();
        else
          owsNames << shortName;
        if ( l->type() == QgsMapLayerType::VectorLayer )
        {
          QgsVectorLayer *vl = static_cast<QgsVectorLayer *>( l );
          if ( vl->dataProvider()->encoding() == QLatin1String( "System" ) )
            encodingMessages << QObject::tr( "Update layer \"%1\" encoding" ).arg( l->name() );
        }
      }
    }
  }
}


QString QgsProjectServerValidator::projectStatusHtml( QgsLayerTree *layerTree )
{

  QString html = QStringLiteral( "<h1>" ) + QObject::tr( "Start checking QGIS Server" ) + QStringLiteral( "</h1>" );

  QStringList owsNames, encodingMessages;
  checkOWS( layerTree, owsNames, encodingMessages );

  QStringList duplicateNames, regExpMessages;
  QRegExp snRegExp = QgsApplication::shortNameRegExp();
  const auto constOwsNames = owsNames;
  for ( const QString &name : constOwsNames )
  {
    if ( !snRegExp.exactMatch( name ) )
      regExpMessages <<  QObject::tr( "Use short name for \"%1\"" ).arg( name );
    if ( duplicateNames.contains( name ) )
      continue;
    if ( owsNames.count( name ) > 1 )
      duplicateNames << name;
  }

  if ( !duplicateNames.empty() )
  {
    QString nameMessage = QStringLiteral( "<h1>" ) +  QObject::tr( "Some layers and groups have the same name or short name" ) + QStringLiteral( "</h1>" );
    nameMessage += "<h2>" +  QObject::tr( "Duplicate names:" ) + "</h2>";
    nameMessage += duplicateNames.join( QStringLiteral( "</li><li>" ) ) + QStringLiteral( "</li></ul>" );
    html += nameMessage;
  }
  else
  {
    html += QStringLiteral( "<h1>" ) + QObject::tr( "All names and short names of layer and group are unique" ) + QStringLiteral( "</h1>" );
  }

  if ( !regExpMessages.empty() )
  {
    html += QStringLiteral( "<h1>" ) + QObject::tr( "Some layer short names have to be updated:" ) + QStringLiteral( "</h1><ul><li>" );
    html += regExpMessages.join( QStringLiteral( "</li><li>" ) );
    html += QStringLiteral( "</li></ul>" );
  }
  else
  {
    html += QStringLiteral( "<h1>" ) + QObject::tr( "All layer short names are well formed" ) + QStringLiteral( "</h1>" );
  }

  if ( !encodingMessages.empty() )
  {
    html += QStringLiteral( "<h1>" ) + QObject::tr( "Some layer encodings are not set:" ) + QStringLiteral( "</h1><ul><li>" );
    html += encodingMessages.join( QStringLiteral( "</li><li>" ) );
    html += QStringLiteral( "</li></ul>" );
  }
  else
  {
    html += QStringLiteral( "<h1>" ) + QObject::tr( "All layer encodings are set" ) + QStringLiteral( "</h1>" );
  }

  return html;
}
