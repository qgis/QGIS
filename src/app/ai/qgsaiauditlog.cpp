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
#include <QFileInfo>
#include <QString>
#include <QStringList>
#include <QUrl>

using namespace Qt::StringLiterals;

namespace
{
  QString sAuditFilePathOverride;
  bool sWriteFailureWarned = false;

  QString redactedDetailSummary( const QString &tool, const QString &detail )
  {
    if ( tool == "run_python"_L1 )
      return u"code_chars=%1"_s.arg( detail.size() );

    if ( tool == "download_file"_L1 )
    {
      const int separator = detail.indexOf( " -> "_L1 );
      const QString urlText = separator >= 0 ? detail.left( separator ) : detail;
      const QString destText = separator >= 0 ? detail.mid( separator + 4 ) : QString();
      const QUrl url( urlText );
      const QString host = url.host().isEmpty() ? u"unknown-host"_s : url.host();
      const QString path = url.path().isEmpty() ? u"/"_s : url.path();
      const QString destFile = QFileInfo( destText ).fileName();
      return u"host=%1 path=%2 dest_file=%3"_s.arg( host, path, destFile.isEmpty() ? u"<none>"_s : destFile );
    }

    if ( tool == "install_python_package"_L1 )
    {
      const QStringList packages = detail.split( ' ', Qt::SkipEmptyParts );
      return u"package_count=%1"_s.arg( packages.size() );
    }

    return u"detail_chars=%1"_s.arg( detail.size() );
  }
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
  const QString workspaceRoot = QgsAiWorkspaceTrust::currentWorkspaceRoot();
  const QString workspaceHash = workspaceRoot.isEmpty() ? u"none"_s : QgsAiWorkspaceTrust::workspaceHash( workspaceRoot );
  QString summary = redactedDetailSummary( tool, detail );
  summary.replace( '\r', ' ' );
  summary.replace( '\n', ' ' );

  const QString line
    = u"%1 | %2 | workspace=%3 | sha256=%4 | bytes=%5 | %6\n"_s.arg( QDateTime::currentDateTimeUtc().toString( Qt::ISODate ), tool, workspaceHash, digest, QString::number( detail.toUtf8().size() ), summary );

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
