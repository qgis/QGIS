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


#include "qgsprojectservervalidator.h"

#include "qgsapplication.h"
#include "qgslayertreelayer.h"
#include "qgsvectorlayer.h"

#include <QRegularExpression>

QString QgsProjectServerValidator::displayValidationError( QgsProjectServerValidator::ValidationError error )
{
  switch ( error )
  {
    case QgsProjectServerValidator::LayerEncoding:
      return QObject::tr( "Encoding is not correctly set. A non 'System' encoding is required" );
    case QgsProjectServerValidator::LayerShortName:
      return QObject::tr( "Layer short name is not valid. It must start with an unaccented alphabetical letter, followed by any alphanumeric letters, dot, dash or underscore" );
    case QgsProjectServerValidator::DuplicatedNames:
      return QObject::tr( "One or more layers or groups have the same name or short name. Both the 'name' and 'short name' for layers and groups must be unique" );
    case QgsProjectServerValidator::ProjectShortName:
      return QObject::tr( "The project root name (either the project short name or project title) is not valid. It must start with an unaccented alphabetical letter, followed by any alphanumeric letters, dot, dash or underscore" );
    case QgsProjectServerValidator::ProjectRootNameConflict:
      return QObject::tr( "The project root name (either the project short name or project title) is already used by a layer or a group" );
    case QgsProjectServerValidator::OnlyMaptipTrueButEmptyMaptip:
      return QObject::tr( "Use only maptip for HTML GetFeatureInfo response is enabled but the HTML maptip is empty" );
  }
  return QString();
}

//! helper method to retrieve a layer or layer tree group short name
template <class T>
QString getShortName( T *node )
{
  const QString shortName = node->serverProperties()->shortName();
  return shortName.isEmpty() ? node->name() : shortName;
}

void QgsProjectServerValidator::browseLayerTree( QgsLayerTreeGroup *treeGroup, QStringList &owsNames, QStringList &encodingMessages, QStringList &layerNames, QStringList &maptipTemplates )
{
  const QList< QgsLayerTreeNode * > treeGroupChildren = treeGroup->children();
  for ( int i = 0; i < treeGroupChildren.size(); ++i )
  {
    QgsLayerTreeNode *treeNode = treeGroupChildren.at( i );
    if ( treeNode->nodeType() == QgsLayerTreeNode::NodeGroup )
    {
      QgsLayerTreeGroup *treeGroupChild = static_cast<QgsLayerTreeGroup *>( treeNode );
      owsNames << getShortName( treeGroupChild );
      browseLayerTree( treeGroupChild, owsNames, encodingMessages, layerNames, maptipTemplates );
    }
    else
    {
      QgsLayerTreeLayer *treeLayer = static_cast<QgsLayerTreeLayer *>( treeNode );
      QgsMapLayer *layer = treeLayer->layer();
      if ( layer )
      {
        owsNames << getShortName( layer );
        if ( layer->type() == Qgis::LayerType::Vector )
        {
          QgsVectorLayer *vl = static_cast<QgsVectorLayer *>( layer );
          if ( vl->dataProvider() && vl->dataProvider()->encoding() == "System"_L1 )
            encodingMessages << layer->name();
        }
        layerNames << treeLayer->name();
        maptipTemplates << layer->mapTipTemplate();
      }
    }
  }
}

bool QgsProjectServerValidator::isOnlyMaptipEnabled( QgsProject *project )
{
  return project->readBoolEntry(
           u"WMSHTMLFeatureInfoUseOnlyMaptip"_s,
           QString(),
           false
         );
}

bool QgsProjectServerValidator::validate( QgsProject *project, QList<QgsProjectServerValidator::ValidationResult> &results )
{
  results.clear();
  bool result = true;

  if ( !project )
    return false;

  if ( !project->layerTreeRoot() )
    return false;

  QStringList owsNames, encodingMessages, layerNames, maptipTemplates;
  browseLayerTree( project->layerTreeRoot(), owsNames, encodingMessages, layerNames, maptipTemplates );

  QStringList duplicateNames, regExpMessages;
  const thread_local QRegularExpression snRegExp = QgsApplication::shortNameRegularExpression();
  const auto constOwsNames = owsNames;
  for ( const QString &name : constOwsNames )
  {
    if ( !snRegExp.match( name ).hasMatch() )
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
    results << ValidationResult( QgsProjectServerValidator::DuplicatedNames, duplicateNames.join( ", "_L1 ) );
  }

  if ( !regExpMessages.empty() )
  {
    result = false;
    results << ValidationResult( QgsProjectServerValidator::LayerShortName, regExpMessages.join( ", "_L1 ) );
  }

  if ( !encodingMessages.empty() )
  {
    result = false;
    results << ValidationResult( QgsProjectServerValidator::LayerEncoding, encodingMessages.join( ", "_L1 ) );
  }

  // Determine the root layername
  QString rootLayerName = project->readEntry( u"WMSRootName"_s, u"/"_s, "" );
  if ( rootLayerName.isEmpty() && !project->title().isEmpty() )
  {
    rootLayerName = project->title();
  }
  if ( !rootLayerName.isEmpty() )
  {
    if ( owsNames.count( rootLayerName ) >= 1 )
    {
      result = false;
      results << ValidationResult( QgsProjectServerValidator::ProjectRootNameConflict, rootLayerName );
    }

    if ( !snRegExp.match( rootLayerName ).hasMatch() )
    {
      result = false;
      results << ValidationResult( QgsProjectServerValidator::ProjectShortName, rootLayerName );
    }
  }
  if ( isOnlyMaptipEnabled( project ) )
  {
    QStringList emptyLayers;
    for ( int i = 0; i < maptipTemplates.size(); ++i )
    {
      if ( maptipTemplates[i].trimmed().isEmpty() )
        emptyLayers << layerNames[i];
    }

    if ( !emptyLayers.isEmpty() )
    {
      result = false;
      QString details = emptyLayers.join( ", "_L1 ).toHtmlEscaped();
      results << ValidationResult(
                QgsProjectServerValidator::OnlyMaptipTrueButEmptyMaptip,
                details );
    }
  }

  return result;
}
