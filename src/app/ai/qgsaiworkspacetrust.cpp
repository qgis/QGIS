/***************************************************************************
    qgsaiworkspacetrust.cpp
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

#include "qgsaiworkspacetrust.h"

#include "qgsaifilecontextprovider.h"
#include "qgssettings.h"

#include <QCryptographicHash>
#include <QDir>

using namespace Qt::StringLiterals;

QString QgsAiWorkspaceTrust::workspaceHash( const QString &root )
{
  const QString normalized = QDir( root.trimmed() ).absolutePath();
  return QString::fromLatin1( QCryptographicHash::hash( normalized.toUtf8(), QCryptographicHash::Sha1 ).toHex().left( 16 ) );
}

QgsAiWorkspaceTrust::State QgsAiWorkspaceTrust::state( const QString &root )
{
  if ( root.trimmed().isEmpty() )
    return State::Trusted; // nothing to trust

  const QgsSettings settings;
  const QString stored = settings.value( u"ai/trust/ws_%1/state"_s.arg( workspaceHash( root ) ) ).toString().trimmed();
  if ( stored.compare( u"trusted"_s, Qt::CaseInsensitive ) == 0 )
    return State::Trusted;
  if ( stored.compare( u"untrusted"_s, Qt::CaseInsensitive ) == 0 )
    return State::Untrusted;
  return State::Unknown;
}

void QgsAiWorkspaceTrust::setState( const QString &root, State state )
{
  if ( root.trimmed().isEmpty() )
    return;

  QgsSettings settings;
  const QString hash = workspaceHash( root );
  switch ( state )
  {
    case State::Trusted:
      settings.setValue( u"ai/trust/ws_%1/state"_s.arg( hash ), u"trusted"_s );
      break;
    case State::Untrusted:
      settings.setValue( u"ai/trust/ws_%1/state"_s.arg( hash ), u"untrusted"_s );
      break;
    case State::Unknown:
      settings.remove( u"ai/trust/ws_%1/state"_s.arg( hash ) );
      break;
  }
  settings.setValue( u"ai/trust/ws_%1/path"_s.arg( hash ), QDir( root.trimmed() ).absolutePath() );
}

bool QgsAiWorkspaceTrust::isTrusted( const QString &root )
{
  // Unknown counts as NOT trusted: restricted until the user decides.
  return state( root ) == State::Trusted;
}

QString QgsAiWorkspaceTrust::currentWorkspaceRoot()
{
  return QgsAiFileContextProvider::resolveWorkspaceRoot();
}

bool QgsAiWorkspaceTrust::isCurrentWorkspaceTrusted()
{
  return isTrusted( currentWorkspaceRoot() );
}
