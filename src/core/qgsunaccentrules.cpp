// qgsunaccentrules.cpp
#include "qgsunaccentrules.h"
#include <QFile>
#include <QTextStream>
#include <QRegularExpression>
#include <QCoreApplication>

QgsUnaccentRules::QgsUnaccentRules()
{
  ensureLoaded();
}

QgsUnaccentRules &QgsUnaccentRules::instance()
{
  static QgsUnaccentRules inst;
  return inst;
}

void QgsUnaccentRules::ensureLoaded()
{
  if ( m_loaded ) return;

  // Load the rules from the file
  const QString path = QCoreApplication::applicationDirPath() + "/resources/unaccent.rules";
  QFile file( path );
  if ( !file.open( QIODevice::ReadOnly | QIODevice::Text ) ) return;

  QTextStream ts( &file );
  QRegularExpression wsRe( "\\s+" );
  QVector<QPair<QString, QString>> parsed;

  // -----------------------------------------------------------------------------
  // The following unaccent.rules parsing logic is derived from PostgreSQL's
  // unaccent extension, which is licensed under the PostgreSQL License
  // Original source:
  //   https://github.com/postgres/postgres/blob/master/contrib/unaccent/unaccent.c
  // -----------------------------------------------------------------------------

  while ( !ts.atEnd() )
  {
    QString line = ts.readLine();
    if ( line.isEmpty() || line.startsWith( '#' ) ) continue;

    int firstWs = line.indexOf( wsRe );
    QString from = line.left( firstWs ).trimmed();
    QString to = line.mid( firstWs ).trimmed();

    if ( from.isEmpty() ) continue;

    if ( from.size() > m_maxLen ) m_maxLen = from.size();
    m_rulesByLen[from.size()].append( qMakePair( from, to ) );
  }

  m_loaded = true;
}

QString QgsUnaccentRules::transform( const QString &in )
{
  if ( !m_loaded || in.isEmpty() ) return in;

  QString out;
  out.reserve( in.size() );
  int i = 0, n = in.size();

  while ( i < n )
  {
    bool applied = false;
    const int maxTry = std::min( m_maxLen, n - i );
    for ( int L = maxTry; L >= 1 && !applied; --L )
    {
      const QStringView probe = QStringView( in ).mid( i, L );
      const auto bucket = m_rulesByLen.constFind( L );
      if ( bucket == m_rulesByLen.constEnd() ) continue;

      for ( const auto &rule : *bucket )
      {
        if ( probe == rule.first )
        {
          out += rule.second;
          i += L;
          applied = true;
          break;
        }
      }
    }
    if ( !applied )
    {
      out += in.at( i );
      ++i;
    }
  }
  return out;
}
