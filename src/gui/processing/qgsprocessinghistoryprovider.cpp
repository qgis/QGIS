/***************************************************************************
                            qgsprocessinghistoryprovider.cpp
                            -------------------------
    begin                : December 2021
    copyright            : (C) 2021 by Nyall Dawson
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

#include "qgsprocessinghistoryprovider.h"
#include "qgsapplication.h"
#include "qgsgui.h"
#include "qgshistoryproviderregistry.h"
#include "qgssettings.h"

#include <QFile>
#include <QTextStream>
#include <QRegularExpression>
#include <QRegularExpressionMatch>

QgsProcessingHistoryProvider::QgsProcessingHistoryProvider()
{
}

QString QgsProcessingHistoryProvider::id() const
{
  return QStringLiteral( "processing" );
}

void QgsProcessingHistoryProvider::portOldLog()
{
  const QString logPath = oldLogPath();
  if ( !QFile::exists( logPath ) )
    return;

  QFile logFile( logPath );
  if ( logFile.open( QIODevice::ReadOnly ) )
  {
    QTextStream in( &logFile );
    QList< QgsHistoryEntry > entries;
    while ( !in.atEnd() )
    {
      const QString line = in.readLine().trimmed();
      QStringList parts = line.split( QStringLiteral( "|~|" ) );
      if ( parts.size() <= 1 )
        parts = line.split( '|' );

      if ( parts.size() == 3 && parts.at( 0 ).startsWith( QLatin1String( "ALGORITHM" ), Qt::CaseInsensitive ) )
      {
        QVariantMap details;
        details.insert( QStringLiteral( "python_command" ), parts.at( 2 ) );

        const thread_local QRegularExpression algIdRegEx( QStringLiteral( "processing\\.run\\(\"(.*?)\"" ) );
        const QRegularExpressionMatch match = algIdRegEx.match( parts.at( 2 ) );
        if ( match.hasMatch() )
          details.insert( QStringLiteral( "algorithm_id" ), match.captured( 1 ) );

        entries.append( QgsHistoryEntry( id(),
                                         QDateTime::fromString( parts.at( 1 ), QStringLiteral( "yyyy-MM-d hh:mm:ss" ) ),
                                         details ) );
      }
    }

    QgsGui::historyProviderRegistry()->addEntries( entries );
  }
}

QString QgsProcessingHistoryProvider::oldLogPath() const
{
  const QString userDir = QgsApplication::qgisSettingsDirPath() + QStringLiteral( "/processing" );
  return userDir + QStringLiteral( "/processing.log" );
}
