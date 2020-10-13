/***************************************************************************
                         qgsalgorithmcreatedirectory.cpp
                         ---------------------
    begin                : June 2020
    copyright            : (C) 2020 by Nyall Dawson
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
#include "qgsalgorithmcreatedirectory.h"

///@cond PRIVATE

QString QgsCreateDirectoryAlgorithm::name() const
{
  return QStringLiteral( "createdirectory" );
}

QgsProcessingAlgorithm::Flags QgsCreateDirectoryAlgorithm::flags() const
{
  return FlagHideFromToolbox | FlagSkipGenericModelLogging;
}

QString QgsCreateDirectoryAlgorithm::displayName() const
{
  return QObject::tr( "Create directory" );
}

QStringList QgsCreateDirectoryAlgorithm::tags() const
{
  return QObject::tr( "new,make,folder" ).split( ',' );
}

QString QgsCreateDirectoryAlgorithm::group() const
{
  return QObject::tr( "File tools" );
}

QString QgsCreateDirectoryAlgorithm::groupId() const
{
  return QStringLiteral( "filetools" );
}

QString QgsCreateDirectoryAlgorithm::shortHelpString() const
{
  return QObject::tr( "This algorithm creates a new directory on a file system. Directories will be created recursively, creating all "
                      "required parent directories in order to construct the full specified directory path.\n\n"
                      "No errors will be raised if the directory already exists." );
}

QString QgsCreateDirectoryAlgorithm::shortDescription() const
{
  return QObject::tr( "Creates a new directory on a file system." );
}

QgsCreateDirectoryAlgorithm *QgsCreateDirectoryAlgorithm::createInstance() const
{
  return new QgsCreateDirectoryAlgorithm();
}

void QgsCreateDirectoryAlgorithm::initAlgorithm( const QVariantMap & )
{
  addParameter( new QgsProcessingParameterMapLayer( QStringLiteral( "PATH" ), QObject::tr( "Directory path" ) ) );
  addOutput( new QgsProcessingOutputFolder( QStringLiteral( "OUTPUT" ), QObject::tr( "Directory" ) ) );
}

QVariantMap QgsCreateDirectoryAlgorithm::processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  const QString path = parameterAsString( parameters, QStringLiteral( "PATH" ), context );

  if ( !path.isEmpty() )
  {
    if ( QFile::exists( path ) )
    {
      if ( !QFileInfo( path ).isDir() )
        throw QgsProcessingException( QObject::tr( "A file with the name %1 already exists -- cannot create a new directory here." ).arg( path ) );
      if ( feedback )
        feedback->pushInfo( QObject::tr( "The directory %1 already exists." ).arg( path ) );
    }
    else
    {
      if ( !QDir().mkpath( path ) )
      {
        throw QgsProcessingException( QObject::tr( "Could not create directory %1. Please check that you have write permissions for the specified path." ).arg( path ) );
      }

      if ( feedback )
        feedback->pushInfo( QObject::tr( "Created %1" ).arg( path ) );
    }
  }

  QVariantMap results;
  results.insert( QStringLiteral( "OUTPUT" ), path );
  return results;
}

///@endcond
