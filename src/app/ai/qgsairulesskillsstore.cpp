/***************************************************************************
    qgsairulesskillsstore.cpp
    ---------------------
    begin                : July 2026
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

#include "qgsairulesskillsstore.h"

#include <algorithm>

#include "qgsaifilecontextprovider.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QRegularExpression>
#include <QString>

using namespace Qt::StringLiterals;

namespace
{
  struct Frontmatter
  {
      QList<QPair<QString, QString>> fields;
      QString body;
      bool present = false;

      QString value( const QString &key, const QString &defaultValue = QString() ) const
      {
        for ( const auto &field : fields )
        {
          if ( field.first.compare( key, Qt::CaseInsensitive ) == 0 )
            return field.second;
        }
        return defaultValue;
      }
  };

  QString stripQuotes( const QString &value )
  {
    if ( value.length() >= 2 && ( ( value.startsWith( '"' ) && value.endsWith( '"' ) ) || ( value.startsWith( '\'' ) && value.endsWith( '\'' ) ) ) )
      return value.mid( 1, value.length() - 2 );
    return value;
  }

  bool parseBool( const QString &value, bool defaultValue )
  {
    const QString normalized = value.trimmed().toLower();
    if ( normalized == "true"_L1 || normalized == "yes"_L1 || normalized == "1"_L1 )
      return true;
    if ( normalized == "false"_L1 || normalized == "no"_L1 || normalized == "0"_L1 )
      return false;
    return defaultValue;
  }

  QStringList parseGlobs( const QString &value )
  {
    QString trimmed = value.trimmed();
    // Tolerate a `[a, b]` bracketed list as well as a bare comma-separated string.
    if ( trimmed.startsWith( '[' ) && trimmed.endsWith( ']' ) )
      trimmed = trimmed.mid( 1, trimmed.length() - 2 );
    QStringList globs;
    for ( const QString &part : trimmed.split( ',', Qt::SkipEmptyParts ) )
    {
      const QString cleaned = stripQuotes( part.trimmed() );
      if ( !cleaned.isEmpty() )
        globs << cleaned;
    }
    return globs;
  }

  QString titleCaseFromSlug( const QString &slug )
  {
    QStringList words = slug.split( QRegularExpression( u"[-_]+"_s ), Qt::SkipEmptyParts );
    for ( QString &word : words )
    {
      if ( !word.isEmpty() )
        word[0] = word[0].toUpper();
    }
    return words.join( ' ' );
  }

  /**
   * Parses a leading `---\n...\n---` frontmatter block, if present. Malformed blocks
   * (no closing marker) are treated as absent and the whole content becomes the body.
   */
  Frontmatter parseFrontmatter( const QString &content )
  {
    Frontmatter result;
    QString normalized = content;
    normalized.replace( "\r\n"_L1, "\n"_L1 );
    const QStringList lines = normalized.split( '\n' );

    if ( lines.isEmpty() || lines.first().trimmed() != "---"_L1 )
    {
      result.body = content.trimmed();
      return result;
    }

    int closingIndex = -1;
    for ( int i = 1; i < lines.size(); ++i )
    {
      if ( lines.at( i ).trimmed() == "---"_L1 )
      {
        closingIndex = i;
        break;
      }
    }
    if ( closingIndex < 0 )
    {
      result.body = content.trimmed();
      return result;
    }

    result.present = true;
    for ( int i = 1; i < closingIndex; ++i )
    {
      const QString &line = lines.at( i );
      const int colon = line.indexOf( ':' );
      if ( colon < 0 )
        continue;
      const QString key = line.left( colon ).trimmed();
      const QString value = stripQuotes( line.mid( colon + 1 ).trimmed() );
      if ( !key.isEmpty() )
        result.fields.append( { key, value } );
    }

    QStringList bodyLines = lines.mid( closingIndex + 1 );
    while ( !bodyLines.isEmpty() && bodyLines.first().trimmed().isEmpty() )
      bodyLines.removeFirst();
    result.body = bodyLines.join( '\n' ).trimmed();
    return result;
  }

  QString serializeFrontmatter( const QList<QPair<QString, QString>> &fields, const QString &body )
  {
    QString out = "---\n"_L1;
    for ( const auto &field : fields )
      out += u"%1: %2\n"_s.arg( field.first, field.second );
    out += "---\n\n"_L1;
    out += body.trimmed();
    out += '\n';
    return out;
  }

  QString readFile( const QString &path, QString *errorMessage )
  {
    QFile file( path );
    if ( !file.open( QIODevice::ReadOnly | QIODevice::Text ) )
    {
      if ( errorMessage )
        *errorMessage = u"Could not open %1 for reading."_s.arg( path );
      return QString();
    }
    return QString::fromUtf8( file.readAll() );
  }

  bool writeFile( const QString &path, const QString &content, QString *errorMessage )
  {
    QFileInfo info( path );
    QDir dir;
    if ( !dir.mkpath( info.absolutePath() ) )
    {
      if ( errorMessage )
        *errorMessage = u"Could not create directory %1."_s.arg( info.absolutePath() );
      return false;
    }
    QFile file( path );
    if ( !file.open( QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate ) )
    {
      if ( errorMessage )
        *errorMessage = u"Could not open %1 for writing."_s.arg( path );
      return false;
    }
    file.write( content.toUtf8() );
    return true;
  }
} // namespace

QgsAiRulesSkillsStore::QgsAiRulesSkillsStore( QgsAiFileContextProvider *contextProvider )
  : mContextProvider( contextProvider )
{}

QString QgsAiRulesSkillsStore::resolveDir( const QString &relativeDir, QString *errorMessage ) const
{
  if ( !mContextProvider || relativeDir.trimmed().isEmpty() )
  {
    if ( errorMessage )
      *errorMessage = u"No workspace configured."_s;
    return QString();
  }
  const QString root = mContextProvider->workspaceRoot();
  if ( root.isEmpty() )
  {
    if ( errorMessage )
      *errorMessage = u"No workspace configured."_s;
    return QString();
  }
  const QString absolutePath = QDir::cleanPath( QDir( root ).filePath( relativeDir ) );
  if ( !mContextProvider->isInWorkspace( absolutePath ) )
  {
    if ( errorMessage )
      *errorMessage = u"Path escapes the workspace root."_s;
    return QString();
  }
  return absolutePath;
}

QList<QgsAiRuleInfo> QgsAiRulesSkillsStore::listRules( const QString &rulesRelativeDir ) const
{
  QList<QgsAiRuleInfo> rules;
  const QString dirPath = resolveDir( rulesRelativeDir, nullptr );
  if ( dirPath.isEmpty() )
    return rules;

  QDir dir( dirPath );
  const QFileInfoList entries = dir.entryInfoList( { u"*.md"_s, u"*.markdown"_s, u"*.txt"_s }, QDir::Files | QDir::Readable, QDir::Name );
  for ( const QFileInfo &entry : entries )
  {
    const QString content = readFile( entry.absoluteFilePath(), nullptr );
    const Frontmatter fm = parseFrontmatter( content );

    QgsAiRuleInfo rule;
    rule.slug = entry.completeBaseName();
    rule.path = entry.absoluteFilePath();
    rule.name = fm.value( u"name"_s, titleCaseFromSlug( rule.slug ) );
    rule.description = fm.value( u"description"_s );
    rule.globs = parseGlobs( fm.value( u"globs"_s ) );
    rule.alwaysApply = parseBool( fm.value( u"alwaysApply"_s ), true );
    rule.enabled = parseBool( fm.value( u"enabled"_s ), true );
    rules << rule;
  }
  std::sort( rules.begin(), rules.end(), []( const QgsAiRuleInfo &a, const QgsAiRuleInfo &b ) { return a.name.localeAwareCompare( b.name ) < 0; } );
  return rules;
}

QList<QgsAiSkillInfo> QgsAiRulesSkillsStore::listSkills( const QString &skillsRelativeDir ) const
{
  QList<QgsAiSkillInfo> skills;
  const QString dirPath = resolveDir( skillsRelativeDir, nullptr );
  if ( dirPath.isEmpty() )
    return skills;

  QDir dir( dirPath );
  const QFileInfoList folders = dir.entryInfoList( QDir::Dirs | QDir::NoDotAndDotDot, QDir::Name );
  for ( const QFileInfo &folder : folders )
  {
    const QString skillFile = QDir( folder.absoluteFilePath() ).filePath( u"SKILL.md"_s );
    if ( !QFileInfo::exists( skillFile ) )
      continue;

    const QString content = readFile( skillFile, nullptr );
    const Frontmatter fm = parseFrontmatter( content );

    QgsAiSkillInfo skill;
    skill.slug = folder.fileName();
    skill.folderPath = folder.absoluteFilePath();
    skill.skillFilePath = skillFile;
    skill.name = fm.value( u"name"_s, titleCaseFromSlug( skill.slug ) );
    skill.description = fm.value( u"description"_s );
    skill.enabled = parseBool( fm.value( u"enabled"_s ), true );
    skills << skill;
  }
  std::sort( skills.begin(), skills.end(), []( const QgsAiSkillInfo &a, const QgsAiSkillInfo &b ) { return a.name.localeAwareCompare( b.name ) < 0; } );
  return skills;
}

QString QgsAiRulesSkillsStore::readRuleBody( const QgsAiRuleInfo &rule ) const
{
  if ( !rule.isValid() )
    return QString();
  return parseFrontmatter( readFile( rule.path, nullptr ) ).body;
}

QString QgsAiRulesSkillsStore::readSkillBody( const QgsAiSkillInfo &skill ) const
{
  if ( !skill.isValid() )
    return QString();
  return parseFrontmatter( readFile( skill.skillFilePath, nullptr ) ).body;
}

bool QgsAiRulesSkillsStore::writeRule( const QString &rulesRelativeDir, const QgsAiRuleInfo &info, const QString &body, QString *errorMessage ) const
{
  if ( info.slug.trimmed().isEmpty() )
  {
    if ( errorMessage )
      *errorMessage = u"A rule needs a slug."_s;
    return false;
  }
  const QString dirPath = resolveDir( rulesRelativeDir, errorMessage );
  if ( dirPath.isEmpty() )
    return false;

  const QString filePath = QDir::cleanPath( QDir( dirPath ).filePath( info.slug + ".md"_L1 ) );
  if ( !mContextProvider->isInWorkspace( filePath ) )
  {
    if ( errorMessage )
      *errorMessage = u"Path escapes the workspace root."_s;
    return false;
  }

  QList<QPair<QString, QString>> fields;
  if ( !info.name.trimmed().isEmpty() )
    fields.append( { u"name"_s, info.name.trimmed() } );
  if ( !info.description.trimmed().isEmpty() )
    fields.append( { u"description"_s, info.description.trimmed() } );
  if ( !info.globs.isEmpty() )
    fields.append( { u"globs"_s, info.globs.join( ", "_L1 ) } );
  fields.append( { u"alwaysApply"_s, info.alwaysApply ? u"true"_s : u"false"_s } );
  if ( !info.enabled )
    fields.append( { u"enabled"_s, u"false"_s } );

  return writeFile( filePath, serializeFrontmatter( fields, body ), errorMessage );
}

bool QgsAiRulesSkillsStore::writeSkill( const QString &skillsRelativeDir, const QgsAiSkillInfo &info, const QString &body, QString *errorMessage ) const
{
  if ( info.slug.trimmed().isEmpty() )
  {
    if ( errorMessage )
      *errorMessage = u"A skill needs a slug."_s;
    return false;
  }
  const QString dirPath = resolveDir( skillsRelativeDir, errorMessage );
  if ( dirPath.isEmpty() )
    return false;

  const QString folderPath = QDir::cleanPath( QDir( dirPath ).filePath( info.slug ) );
  const QString filePath = QDir( folderPath ).filePath( u"SKILL.md"_s );
  if ( !mContextProvider->isInWorkspace( filePath ) )
  {
    if ( errorMessage )
      *errorMessage = u"Path escapes the workspace root."_s;
    return false;
  }

  QList<QPair<QString, QString>> fields;
  fields.append( { u"name"_s, info.name.trimmed().isEmpty() ? titleCaseFromSlug( info.slug ) : info.name.trimmed() } );
  fields.append( { u"description"_s, info.description.trimmed() } );
  if ( !info.enabled )
    fields.append( { u"enabled"_s, u"false"_s } );

  return writeFile( filePath, serializeFrontmatter( fields, body ), errorMessage );
}

bool QgsAiRulesSkillsStore::deleteRule( const QgsAiRuleInfo &rule, QString *errorMessage ) const
{
  if ( !rule.isValid() || !mContextProvider || !mContextProvider->isInWorkspace( rule.path ) )
  {
    if ( errorMessage )
      *errorMessage = u"Invalid rule path."_s;
    return false;
  }
  if ( !QFile::remove( rule.path ) )
  {
    if ( errorMessage )
      *errorMessage = u"Could not delete %1."_s.arg( rule.path );
    return false;
  }
  return true;
}

bool QgsAiRulesSkillsStore::deleteSkill( const QgsAiSkillInfo &skill, QString *errorMessage ) const
{
  if ( !skill.isValid() || !mContextProvider || !mContextProvider->isInWorkspace( skill.folderPath ) )
  {
    if ( errorMessage )
      *errorMessage = u"Invalid skill path."_s;
    return false;
  }
  QDir folder( skill.folderPath );
  if ( !folder.removeRecursively() )
  {
    if ( errorMessage )
      *errorMessage = u"Could not delete %1."_s.arg( skill.folderPath );
    return false;
  }
  return true;
}

QString QgsAiRulesSkillsStore::slugify( const QString &name )
{
  QString slug = name.trimmed().toLower();
  slug.replace( QRegularExpression( u"[^a-z0-9]+"_s ), u"-"_s );
  slug.replace( QRegularExpression( u"-+"_s ), u"-"_s );
  while ( slug.startsWith( '-' ) )
    slug.remove( 0, 1 );
  while ( slug.endsWith( '-' ) )
    slug.chop( 1 );
  return slug.isEmpty() ? u"untitled"_s : slug;
}
