/***************************************************************************
                             qgsprojectservervalidator.cpp
                             ---------------------------
    begin                : March 2020
    copyright            : (C) 2020 by Etienne Trimaille
    email                : etienne dot trimaille at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


#include "qgsapplication.h"
#include "qgslayertreelayer.h"
#include "qgsprojectservervalidator.h"
#include "qgsvectorlayer.h"


QString QgsProjectServerValidator::displayValidationError( QgsProjectServerValidator::ValidationError error )
{
  switch ( error )
  {
    case QgsProjectServerValidator::Encoding:
      return QObject::tr( "Encoding is not set properly. It shouldn't be 'System'" );
    case QgsProjectServerValidator::ShortNames:
      return QObject::tr( "Layer short name needs to be updated" );
    case QgsProjectServerValidator::DuplicatedNames:
      return QObject::tr( "Layers/groups have the same name or short name" );
  }
  return QString();
}

void QgsProjectServerValidator::browseLayerTree( QgsLayerTreeGroup *treeGroup, QStringList &owsNames, QStringList &encodingMessages )
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
      browseLayerTree( treeGroupChild, owsNames, encodingMessages );
    }
    else
    {
      QgsLayerTreeLayer *treeLayer = static_cast<QgsLayerTreeLayer *>( treeNode );
      QgsMapLayer *layer = treeLayer->layer();
      if ( layer )
      {
        QString shortName = layer->shortName();
        if ( shortName.isEmpty() )
          owsNames << layer->name();
        else
          owsNames << shortName;

        if ( layer->type() == QgsMapLayerType::VectorLayer )
        {
          QgsVectorLayer *vl = static_cast<QgsVectorLayer *>( layer );
          if ( vl->dataProvider()->encoding() == QLatin1String( "System" ) )
            encodingMessages << layer->name();
        }
      }
    }
  }
}

bool QgsProjectServerValidator::validate( QgsLayerTree *layerTree, QList<QgsProjectServerValidator::ValidationResult> &results ) const
{
  results.clear();
  bool result = true;

  if ( !layerTree )
    return false;

  QStringList owsNames, encodingMessages;
  browseLayerTree( layerTree, owsNames, encodingMessages );

  QStringList duplicateNames, regExpMessages;
  QRegExp snRegExp = QgsApplication::shortNameRegExp();
  const auto constOwsNames = owsNames;
  for ( const QString &name : constOwsNames )
  {
    if ( !snRegExp.exactMatch( name ) )
    {
      regExpMessages << name;
    }

    if ( duplicateNames.contains( name ) )
    {
      continue;
    }

    if ( owsNames.count( name ) > 1 )
    {
      duplicateNames << name;
    }
  }

  if ( !duplicateNames.empty() )
  {
    result = false;
    results << ValidationResult( QgsProjectServerValidator::DuplicatedNames, duplicateNames.join( QStringLiteral( ", " ) ) );
  }

  if ( !regExpMessages.empty() )
  {
    result = false;
    results << ValidationResult( QgsProjectServerValidator::ShortNames, regExpMessages.join( QStringLiteral( ", " ) ) );
  }

  if ( !encodingMessages.empty() )
  {
    result = false;
    results << ValidationResult( QgsProjectServerValidator::Encoding, encodingMessages.join( QStringLiteral( ", " ) ) );
  }

  return result;
}
