/***************************************************************************
                         qgsprocessingmodelprovider.cpp
                         ------------------------
    begin                : September 2026
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

#include "qgsprocessingmodelprovider.h"

#include "qgsapplication.h"
#include "qgsmessagelog.h"
#include "qgsprocessingmodelalgorithm.h"
#include "qgsprocessingregistry.h"
#include "qgsruntimeprofiler.h"

#include <QDirIterator>
#include <QString>

#include "moc_qgsprocessingmodelprovider.cpp"

using namespace Qt::StringLiterals;

QgsProcessingModelProvider::QgsProcessingModelProvider( QObject *parent )
  : QgsProcessingProvider( parent )
{
  // must reload models if providers list is changed - previously unavailable algorithms
  // which models depend on may now be available
  connect( QgsApplication::processingRegistry(), &QgsProcessingRegistry::providerAdded, this, &QgsProcessingModelProvider::onProviderAdded );
}

QString QgsProcessingModelProvider::name() const
{
  return tr( "Models" );
}

QString QgsProcessingModelProvider::id() const
{
  return QgsProcessing::MODEL_PROVIDER_ID;
}

QIcon QgsProcessingModelProvider::icon() const
{
  return QgsApplication::getThemeIcon( u"/processingModel.svg"_s );
}

QString QgsProcessingModelProvider::svgIconPath() const
{
  return QgsApplication::iconPath( u"processingModel.svg"_s );
}

bool QgsProcessingModelProvider::supportsNonFileBasedOutput() const
{
  return true;
}

bool QgsProcessingModelProvider::load()
{
  QgsScopedRuntimeProfile profiler( u"Model Provider"_s );
  refreshAlgorithms();
  return true;
}

void QgsProcessingModelProvider::loadAlgorithms()
{
  if ( mIsLoading )
    return;

  mIsLoading = true;

  const QStringList folders = QgsProcessingUtils::modelFolders();
  for ( const QString &folder : folders )
  {
    loadFromFolder( folder );
  }

  mIsLoading = false;
}

void QgsProcessingModelProvider::onProviderAdded( const QString &providerId )
{
  if ( providerId == id() )
    return;

  refreshAlgorithms();
}

void QgsProcessingModelProvider::loadFromFolder( const QString &folder )
{
  if ( !QFile::exists( folder ) )
    return;

  QDirIterator it( folder, QDir::Files, QDirIterator::Subdirectories );
  while ( it.hasNext() )
  {
    it.next();
    const QString modelFile = it.fileName();
    if ( QFileInfo( modelFile ).suffix().compare( "model3"_L1, Qt::CaseInsensitive ) == 0 )
    {
      const QString fullpath = it.filePath();
      auto modelAlgorithm = std::make_unique<QgsProcessingModelAlgorithm>();
      if ( modelAlgorithm->fromFile( fullpath ) )
      {
        if ( !modelAlgorithm->name().isEmpty() )
        {
          modelAlgorithm->setSourceFilePath( fullpath );
          addAlgorithm( modelAlgorithm.release() );
        }
      }
      else
      {
        QgsMessageLog::logMessage( tr( "Could not load model %1" ).arg( modelFile ), tr( "Processing" ), Qgis::MessageLevel::Critical );
      }
    }
  }
}
