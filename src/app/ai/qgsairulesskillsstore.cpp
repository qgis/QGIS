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

  QStringList parseInlineList( const QString &value )
  {
    QString trimmed = value.trimmed();
    if ( trimmed.startsWith( '[' ) && trimmed.endsWith( ']' ) )
      trimmed = trimmed.mid( 1, trimmed.length() - 2 );
    QStringList values;
    for ( const QString &part : trimmed.split( ',', Qt::SkipEmptyParts ) )
    {
      const QString cleaned = stripQuotes( part.trimmed() );
      if ( !cleaned.isEmpty() )
        values << cleaned;
    }
    return values;
  }

  QStringList parseGlobs( const QStringList &values )
  {
    QStringList globs;
    for ( const QString &value : values )
    {
      for ( const QString &part : parseInlineList( value ) )
      {
        if ( !part.isEmpty() )
          globs << part;
      }
    }
    return globs;
  }

  QString sanitizeYamlScalar( const QString &value )
  {
    QString out = value;
    out.replace( "\r\n"_L1, " "_L1 );
    out.replace( '\n', ' ' );
    return out.trimmed();
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

  QString titleFromBody( const QString &body, const QString &fallbackSlug )
  {
    const QStringList lines = body.split( '\n' );
    for ( QString line : lines )
    {
      line = line.trimmed();
      if ( line.isEmpty() )
        continue;
      if ( line.startsWith( '#' ) )
      {
        while ( line.startsWith( '#' ) )
          line.remove( 0, 1 );
        line = line.trimmed();
      }
      if ( !line.isEmpty() )
        return line.left( 120 );
    }
    return titleCaseFromSlug( fallbackSlug );
  }

} // namespace

QString QgsAiMarkdownDocument::value( const QString &key, const QString &defaultValue ) const
{
  for ( const QgsAiFrontmatterProperty &property : properties )
  {
    if ( property.key.compare( key, Qt::CaseInsensitive ) == 0 )
      return property.value();
  }
  return defaultValue;
}

QStringList QgsAiMarkdownDocument::values( const QString &key ) const
{
  for ( const QgsAiFrontmatterProperty &property : properties )
  {
    if ( property.key.compare( key, Qt::CaseInsensitive ) == 0 )
      return property.values;
  }
  return QStringList();
}

QgsAiMarkdownDocument QgsAiRulesSkillsStore::parseMarkdownDocument( const QString &content )
{
  QgsAiMarkdownDocument result;
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

  result.hasFrontmatter = true;
  int currentPropertyIndex = -1;
  for ( int i = 1; i < closingIndex; ++i )
  {
    const QString line = lines.at( i );
    const QString trimmedLine = line.trimmed();
    if ( trimmedLine.isEmpty() )
      continue;

    if ( currentPropertyIndex >= 0 && line.at( 0 ).isSpace() && trimmedLine.startsWith( "- "_L1 ) )
    {
      QgsAiFrontmatterProperty &property = result.properties[currentPropertyIndex];
      property.isList = true;
      const QString value = stripQuotes( trimmedLine.mid( 2 ).trimmed() );
      if ( !value.isEmpty() )
        property.values << value;
      continue;
    }

    const int colon = line.indexOf( ':' );
    if ( colon < 0 )
      continue;

    QgsAiFrontmatterProperty property;
    property.key = line.left( colon ).trimmed();
    QString value = line.mid( colon + 1 ).trimmed();
    if ( property.key.isEmpty() )
      continue;

    if ( value.startsWith( '[' ) && value.endsWith( ']' ) )
    {
      property.isList = true;
      property.values = parseInlineList( value );
    }
    else if ( value.isEmpty() )
    {
      property.isList = true;
    }
    else
    {
      property.values << stripQuotes( value );
    }

    result.properties << property;
    currentPropertyIndex = result.properties.size() - 1;
  }

  QStringList bodyLines = lines.mid( closingIndex + 1 );
  while ( !bodyLines.isEmpty() && bodyLines.first().trimmed().isEmpty() )
    bodyLines.removeFirst();
  result.body = bodyLines.join( '\n' ).trimmed();
  return result;
}

QString QgsAiRulesSkillsStore::serializeMarkdownDocument( const QgsAiMarkdownDocument &document )
{
  QString out = "---\n"_L1;
  for ( const QgsAiFrontmatterProperty &property : document.properties )
  {
    const QString key = property.key.trimmed();
    if ( key.isEmpty() )
      continue;
    if ( property.isList )
    {
      out += u"%1:\n"_s.arg( key );
      for ( const QString &value : property.values )
      {
        const QString cleanValue = sanitizeYamlScalar( value );
        if ( !cleanValue.isEmpty() )
          out += u"  - %1\n"_s.arg( cleanValue );
      }
      continue;
    }

    out += u"%1: %2\n"_s.arg( key, sanitizeYamlScalar( property.value() ) );
  }
  out += "---\n\n"_L1;
  out += document.body.trimmed();
  out += '\n';
  return out;
}

QString QgsAiRulesSkillsStore::titleFromMarkdown( const QString &content, const QString &fallbackSlug )
{
  const QgsAiMarkdownDocument document = parseMarkdownDocument( content );
  const QString frontmatterName = document.value( u"name"_s ).trimmed();
  if ( !frontmatterName.isEmpty() )
    return frontmatterName;
  return titleFromBody( document.body, fallbackSlug );
}

namespace
{

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
    const QgsAiMarkdownDocument document = parseMarkdownDocument( content );

    QgsAiRuleInfo rule;
    rule.slug = entry.completeBaseName();
    rule.path = entry.absoluteFilePath();
    rule.name = titleFromMarkdown( content, rule.slug );
    rule.description = document.value( u"description"_s );
    rule.globs = parseGlobs( document.values( u"globs"_s ) );
    rule.alwaysApply = parseBool( document.value( u"alwaysApply"_s ), true );
    rule.enabled = parseBool( document.value( u"enabled"_s ), true );
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
    const QgsAiMarkdownDocument document = parseMarkdownDocument( content );

    QgsAiSkillInfo skill;
    skill.slug = folder.fileName();
    skill.folderPath = folder.absoluteFilePath();
    skill.skillFilePath = skillFile;
    skill.name = document.value( u"name"_s, titleFromBody( document.body, skill.slug ) );
    skill.description = document.value( u"description"_s );
    skill.enabled = parseBool( document.value( u"enabled"_s ), true );
    skills << skill;
  }
  std::sort( skills.begin(), skills.end(), []( const QgsAiSkillInfo &a, const QgsAiSkillInfo &b ) { return a.name.localeAwareCompare( b.name ) < 0; } );
  return skills;
}

QString QgsAiRulesSkillsStore::readRuleBody( const QgsAiRuleInfo &rule ) const
{
  if ( !rule.isValid() )
    return QString();
  return parseMarkdownDocument( readFile( rule.path, nullptr ) ).body;
}

QString QgsAiRulesSkillsStore::readSkillBody( const QgsAiSkillInfo &skill ) const
{
  if ( !skill.isValid() )
    return QString();
  return parseMarkdownDocument( readFile( skill.skillFilePath, nullptr ) ).body;
}

QString QgsAiRulesSkillsStore::readRuleMarkdown( const QgsAiRuleInfo &rule ) const
{
  if ( !rule.isValid() )
    return QString();
  return readFile( rule.path, nullptr );
}

QString QgsAiRulesSkillsStore::readSkillMarkdown( const QgsAiSkillInfo &skill ) const
{
  if ( !skill.isValid() )
    return QString();
  return readFile( skill.skillFilePath, nullptr );
}

bool QgsAiRulesSkillsStore::writeRule( const QString &rulesRelativeDir, const QgsAiRuleInfo &info, const QString &body, QString *errorMessage ) const
{
  QgsAiMarkdownDocument document;
  if ( !info.name.trimmed().isEmpty() )
    document.properties.append( { u"name"_s, QStringList { info.name.trimmed() }, false } );
  if ( !info.description.trimmed().isEmpty() )
    document.properties.append( { u"description"_s, QStringList { info.description.trimmed() }, false } );
  if ( !info.globs.isEmpty() )
    document.properties.append( { u"globs"_s, info.globs, true } );
  document.properties.append( { u"alwaysApply"_s, QStringList { info.alwaysApply ? u"true"_s : u"false"_s }, false } );
  if ( !info.enabled )
    document.properties.append( { u"enabled"_s, QStringList { u"false"_s }, false } );
  document.body = body;

  return writeRuleMarkdown( rulesRelativeDir, info.slug, serializeMarkdownDocument( document ), errorMessage );
}

bool QgsAiRulesSkillsStore::writeRuleMarkdown( const QString &rulesRelativeDir, const QString &slug, const QString &markdown, QString *errorMessage ) const
{
  if ( slug.trimmed().isEmpty() )
  {
    if ( errorMessage )
      *errorMessage = u"A rule needs a slug."_s;
    return false;
  }
  const QString dirPath = resolveDir( rulesRelativeDir, errorMessage );
  if ( dirPath.isEmpty() )
    return false;

  const QString filePath = QDir::cleanPath( QDir( dirPath ).filePath( slug + ".md"_L1 ) );
  if ( !mContextProvider->isInWorkspace( filePath ) )
  {
    if ( errorMessage )
      *errorMessage = u"Path escapes the workspace root."_s;
    return false;
  }

  return writeFile( filePath, markdown, errorMessage );
}

bool QgsAiRulesSkillsStore::writeSkill( const QString &skillsRelativeDir, const QgsAiSkillInfo &info, const QString &body, QString *errorMessage ) const
{
  QgsAiMarkdownDocument document;
  document.properties.append( { u"name"_s, QStringList { info.name.trimmed().isEmpty() ? titleCaseFromSlug( info.slug ) : info.name.trimmed() }, false } );
  document.properties.append( { u"description"_s, QStringList { info.description.trimmed() }, false } );
  if ( !info.enabled )
    document.properties.append( { u"enabled"_s, QStringList { u"false"_s }, false } );
  document.body = body;

  return writeSkillMarkdown( skillsRelativeDir, info.slug, serializeMarkdownDocument( document ), errorMessage );
}

bool QgsAiRulesSkillsStore::writeSkillMarkdown( const QString &skillsRelativeDir, const QString &slug, const QString &markdown, QString *errorMessage ) const
{
  if ( slug.trimmed().isEmpty() )
  {
    if ( errorMessage )
      *errorMessage = u"A skill needs a slug."_s;
    return false;
  }
  const QString dirPath = resolveDir( skillsRelativeDir, errorMessage );
  if ( dirPath.isEmpty() )
    return false;

  const QString folderPath = QDir::cleanPath( QDir( dirPath ).filePath( slug ) );
  const QString filePath = QDir( folderPath ).filePath( u"SKILL.md"_s );
  if ( !mContextProvider->isInWorkspace( filePath ) )
  {
    if ( errorMessage )
      *errorMessage = u"Path escapes the workspace root."_s;
    return false;
  }

  return writeFile( filePath, markdown, errorMessage );
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
