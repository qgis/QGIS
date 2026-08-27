/***************************************************************************
                         qgsprocessingprojectmodelprovider.cpp
                         ------------------------
    begin                : August 2026
    copyright            : (C) 2026 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsprocessingprojectmodelprovider.h"

#include "qgsapplication.h"
#include "qgsmessagelog.h"
#include "qgsprocessingmodelalgorithm.h"
#include "qgsprocessingregistry.h"
#include "qgsproject.h"
#include "qgsruntimeprofiler.h"
#include "qgsxmlutils.h"

#include <QString>

#include "moc_qgsprocessingprojectmodelprovider.cpp"

using namespace Qt::StringLiterals;

QgsProcessingProjectModelProvider::QgsProcessingProjectModelProvider( QgsProject *project, QObject *parent )
  : QgsProcessingProvider( parent )
  , mProject( project )
{
  // must reload models if providers list is changed - previously unavailable algorithms
  // which models depend on may now be available
  connect( QgsApplication::processingRegistry(), &QgsProcessingRegistry::providerAdded, this, &QgsProcessingProjectModelProvider::onProviderAdded );

  if ( mProject )
  {
    connect( mProject, &QgsProject::readProject, this, &QgsProcessingProjectModelProvider::readProject );
    connect( mProject, &QgsProject::writeProject, this, &QgsProcessingProjectModelProvider::writeProject );
    connect( mProject, &QgsProject::cleared, this, &QgsProcessingProjectModelProvider::clear );
  }
}

QString QgsProcessingProjectModelProvider::name() const
{
  return tr( "Project models" );
}

QString QgsProcessingProjectModelProvider::longName() const
{
  return tr( "Models embedded in the current project" );
}

QString QgsProcessingProjectModelProvider::id() const
{
  return QgsProcessing::PROJECT_PROVIDER_ID;
}

QIcon QgsProcessingProjectModelProvider::icon() const
{
  return QgsApplication::getThemeIcon( u"/mIconQgsProjectFile.svg"_s );
}

QString QgsProcessingProjectModelProvider::svgIconPath() const
{
  return QgsApplication::iconPath( u"mIconQgsProjectFile.svg"_s );
}

bool QgsProcessingProjectModelProvider::supportsNonFileBasedOutput() const
{
  return true;
}

bool QgsProcessingProjectModelProvider::load()
{
  QgsScopedRuntimeProfile profiler( u"Project Provider"_s );
  refreshAlgorithms();
  return true;
}

void QgsProcessingProjectModelProvider::clear()
{
  mModelDefinitions.clear();
  refreshAlgorithms();
}

void QgsProcessingProjectModelProvider::addModel( const QgsProcessingModelAlgorithm &model )
{
  const QVariant definition = model.toVariant();
  mModelDefinitions.insert( model.name(), definition );
  refreshAlgorithms();
}

void QgsProcessingProjectModelProvider::removeModel( const QgsProcessingModelAlgorithm *model )
{
  if ( !model )
    return;

  if ( mModelDefinitions.contains( model->name() ) )
  {
    mModelDefinitions.remove( model->name() );
    refreshAlgorithms();
  }
}

void QgsProcessingProjectModelProvider::readProject( const QDomDocument &document )
{
  mModelDefinitions.clear();
  const QDomNodeList projectModelsNodes = document.elementsByTagName( u"projectModels"_s );
  if ( !projectModelsNodes.isEmpty() )
  {
    const QDomNode projectModelsNode = projectModelsNodes.at( 0 );
    const QDomNodeList modelNodes = projectModelsNode.childNodes();
    for ( int i = 0; i < modelNodes.count(); ++i )
    {
      const QDomElement modelElement = modelNodes.at( i ).toElement();
      const QVariant definition = QgsXmlUtils::readVariant( modelElement );
      QgsProcessingModelAlgorithm algorithm;
      if ( algorithm.loadVariant( definition ) )
      {
        mModelDefinitions.insert( algorithm.name(), definition );
      }
    }
  }

  refreshAlgorithms();
}

void QgsProcessingProjectModelProvider::writeProject( QDomDocument &document )
{
  const QDomNodeList qgisNodes = document.elementsByTagName( u"qgis"_s );
  if ( qgisNodes.isEmpty() )
    return;

  QDomNode qgisNode = qgisNodes.at( 0 );
  QDomElement projectModelsNode = document.createElement( u"projectModels"_s );

  const QList< const QgsProcessingAlgorithm * > algs = algorithms();
  for ( const QgsProcessingAlgorithm *algorithm : std::as_const( algs ) )
  {
    auto model = dynamic_cast< const QgsProcessingModelAlgorithm *>( algorithm );
    if ( !model )
      continue;

    const QVariant definition = model->toVariant();
    const QDomElement element = QgsXmlUtils::writeVariant( definition, document );
    projectModelsNode.appendChild( element );
  }

  qgisNode.appendChild( projectModelsNode );
}

void QgsProcessingProjectModelProvider::loadAlgorithms()
{
  if ( mIsLoading )
    return;

  mIsLoading = true;

  for ( auto it = mModelDefinitions.constBegin(); it != mModelDefinitions.constEnd(); ++it )
  {
    auto algorithm = std::make_unique< QgsProcessingModelAlgorithm >();
    if ( algorithm->loadVariant( it.value() ) )
    {
      addAlgorithm( algorithm.release() );
    }
    else
    {
      QgsMessageLog::logMessage( tr( "Could not load model from project" ), tr( "Processing" ), Qgis::MessageLevel::Critical );
    }
  }

  mIsLoading = false;
}

void QgsProcessingProjectModelProvider::onProviderAdded( const QString &providerId )
{
  if ( providerId == id() )
    return;

  refreshAlgorithms();
}
