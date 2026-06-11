/***************************************************************************
    qgsaiauditlog.cpp
    ---------------------
    begin                : June 2026
    copyright            : (C) 2026 by Francesco Mazzi
    email                : francemazzi at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsaiauditlog.h"

#include "qgsaiworkspacetrust.h"
#include "qgsapplication.h"
#include "qgsmessagelog.h"

#include <QCryptographicHash>
#include <QDateTime>
#include <QDir>
#include <QFile>

using namespace Qt::StringLiterals;

namespace
{
  QString sAuditFilePathOverride;
  bool sWriteFailureWarned = false;
} //namespace

QString QgsAiAuditLog::filePath()
{
  if ( !sAuditFilePathOverride.isEmpty() )
    return sAuditFilePathOverride;
  return QDir( QgsApplication::qgisSettingsDirPath() ).filePath( u"ai_audit.log"_s );
}

void QgsAiAuditLog::setFilePathOverride( const QString &path )
{
  sAuditFilePathOverride = path.trimmed();
}

void QgsAiAuditLog::append( const QString &tool, const QString &detail )
{
  const QString digest = QString::fromLatin1( QCryptographicHash::hash( detail.toUtf8(), QCryptographicHash::Sha256 ).toHex() );
  QString excerpt = detail.left( 200 );
  excerpt.replace( '\r', ' ' );
  excerpt.replace( '\n', ' ' );

  const QString line = u"%1 | %2 | %3 | sha256=%4 | %5\n"_s.arg( QDateTime::currentDateTimeUtc().toString( Qt::ISODate ), tool, QgsAiWorkspaceTrust::currentWorkspaceRoot(), digest, excerpt );

  QFile file( filePath() );
  if ( !file.open( QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text ) )
  {
    // Best-effort: never block tool execution on an unwritable audit log.
    if ( !sWriteFailureWarned )
    {
      sWriteFailureWarned = true;
      QgsMessageLog::logMessage( u"Cannot write AI audit log at %1."_s.arg( filePath() ), u"AI/Security"_s, Qgis::MessageLevel::Warning, false );
    }
    return;
  }
  file.write( line.toUtf8() );
}
