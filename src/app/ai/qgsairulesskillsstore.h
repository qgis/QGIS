/***************************************************************************
    qgsairulesskillsstore.h
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

#ifndef QGSAIRULESSKILLSSTORE_H
#define QGSAIRULESSKILLSSTORE_H

#include "qgis_app.h"

#include <QList>
#include <QMetaType>
#include <QString>
#include <QStringList>

class QgsAiFileContextProvider;

/**
 * Single frontmatter property from a Markdown rule/skill file.
 *
 * Scalar properties use one value with isList=false. List properties preserve
 * their item order with isList=true and serialize as YAML-style block lists.
 */
struct APP_EXPORT QgsAiFrontmatterProperty
{
    QString key;
    QStringList values;
    bool isList = false;

    QString value() const { return values.isEmpty() ? QString() : values.first(); }
};

/**
 * Parsed Markdown document with optional YAML frontmatter.
 */
struct APP_EXPORT QgsAiMarkdownDocument
{
    QList<QgsAiFrontmatterProperty> properties;
    QString body;
    bool hasFrontmatter = false;

    QString value( const QString &key, const QString &defaultValue = QString() ) const;
    QStringList values( const QString &key ) const;
};

/**
 * Metadata for a single Cursor-style rule file (`<rulesDir>/<slug>.md`).
 *
 * `alwaysApply` mirrors Cursor's rule model: true means the rule body is always
 * injected into the system prompt; false means only a `name`/`description`
 * reference is injected and the agent can fetch the full body on demand via
 * the `read_file` tool.
 */
struct APP_EXPORT QgsAiRuleInfo
{
    QString slug;
    //! Absolute path to the rule's .md file.
    QString path;
    QString name;
    QString description;
    QStringList globs;
    bool alwaysApply = true;
    bool enabled = true;

    bool isValid() const { return !slug.isEmpty() && !path.isEmpty(); }
};

/**
 * Metadata for a single Cursor-style skill (`<skillsDir>/<slug>/SKILL.md`).
 *
 * Skills use progressive disclosure: only `name`/`description` are injected
 * into the system prompt as a compact index; the agent reads the full
 * `SKILL.md` body on demand via the `read_file` tool.
 */
struct APP_EXPORT QgsAiSkillInfo
{
    QString slug;
    //! Absolute path to the skill's folder.
    QString folderPath;
    //! Absolute path to the skill's SKILL.md file.
    QString skillFilePath;
    QString name;
    QString description;
    bool enabled = true;

    bool isValid() const { return !slug.isEmpty() && !skillFilePath.isEmpty(); }
};

/**
 * Reads and writes individual Cursor-style rule and skill files inside a
 * workspace folder, replacing the previous "one big text blob" model with a
 * per-file catalog (list / create / edit / delete), matching Cursor's Rules
 * & Skills UX.
 *
 * File formats:
 *
 * Rule (`<rulesDir>/<slug>.md`):
 * \code
 *   ---
 *   description: One-line summary shown in the rules list
 *   globs: *.qgz, *.gpkg
 *   alwaysApply: true
 *   enabled: true
 *   ---
 *   <markdown body injected into the system prompt>
 * \endcode
 *
 * Skill (`<skillsDir>/<slug>/SKILL.md`):
 * \code
 *   ---
 *   name: Human-readable name
 *   description: When the agent should reach for this skill
 *   enabled: true
 *   ---
 *   <markdown body, fetched on demand by the agent>
 * \endcode
 *
 * The frontmatter block is optional: plain `.md`/`.txt` files without one are
 * still readable (backward compatible with the legacy workspace-folder
 * behavior), defaulting to `alwaysApply = true` and deriving `name` from the
 * file/folder name.
 *
 * All paths are validated against the workspace root via
 * QgsAiFileContextProvider::isInWorkspace(); this class does not itself
 * enforce workspace trust — callers (typically the settings dialog) must
 * gate writes on QgsAiWorkspaceTrust::isTrusted().
 */
class APP_EXPORT QgsAiRulesSkillsStore
{
  public:
    explicit QgsAiRulesSkillsStore( QgsAiFileContextProvider *contextProvider );

    //! Lists rules found under \a rulesRelativeDir (workspace-relative), sorted by name.
    QList<QgsAiRuleInfo> listRules( const QString &rulesRelativeDir ) const;
    //! Lists skills found under \a skillsRelativeDir (workspace-relative), sorted by name.
    QList<QgsAiSkillInfo> listSkills( const QString &skillsRelativeDir ) const;

    //! Returns the markdown body (frontmatter stripped) of \a rule.
    QString readRuleBody( const QgsAiRuleInfo &rule ) const;
    //! Returns the markdown body (frontmatter stripped) of \a skill.
    QString readSkillBody( const QgsAiSkillInfo &skill ) const;

    //! Returns the complete Markdown document for \a rule, including frontmatter.
    QString readRuleMarkdown( const QgsAiRuleInfo &rule ) const;
    //! Returns the complete Markdown document for \a skill, including frontmatter.
    QString readSkillMarkdown( const QgsAiSkillInfo &skill ) const;

    //! Parses a complete Markdown document into frontmatter properties and body.
    static QgsAiMarkdownDocument parseMarkdownDocument( const QString &content );
    //! Serializes frontmatter properties and body into a complete Markdown document.
    static QString serializeMarkdownDocument( const QgsAiMarkdownDocument &document );

    //! Derives a user-facing title from frontmatter, first heading, first line, then slug.
    static QString titleFromMarkdown( const QString &content, const QString &fallbackSlug );

    /**
     * Creates or overwrites the rule file for \a info.slug inside \a rulesRelativeDir
     * with \a body as content. Returns false (and sets \a errorMessage) when the
     * resolved path escapes the workspace or the write fails.
     */
    bool writeRule( const QString &rulesRelativeDir, const QgsAiRuleInfo &info, const QString &body, QString *errorMessage = nullptr ) const;

    /**
     * Creates or overwrites the raw Markdown rule file for \a slug inside
     * \a rulesRelativeDir. The content is written as provided.
     */
    bool writeRuleMarkdown( const QString &rulesRelativeDir, const QString &slug, const QString &markdown, QString *errorMessage = nullptr ) const;

    /**
     * Creates or overwrites the skill folder/SKILL.md for \a info.slug inside
     * \a skillsRelativeDir with \a body as content. Returns false (and sets
     * \a errorMessage) when the resolved path escapes the workspace or the write fails.
     */
    bool writeSkill( const QString &skillsRelativeDir, const QgsAiSkillInfo &info, const QString &body, QString *errorMessage = nullptr ) const;

    /**
     * Creates or overwrites the raw Markdown skill file for \a slug inside
     * \a skillsRelativeDir. The content is written as provided.
     */
    bool writeSkillMarkdown( const QString &skillsRelativeDir, const QString &slug, const QString &markdown, QString *errorMessage = nullptr ) const;

    //! Deletes the rule file. Returns false (and sets \a errorMessage) on failure.
    bool deleteRule( const QgsAiRuleInfo &rule, QString *errorMessage = nullptr ) const;
    //! Deletes the skill folder recursively. Returns false (and sets \a errorMessage) on failure.
    bool deleteSkill( const QgsAiSkillInfo &skill, QString *errorMessage = nullptr ) const;

    //! Derives a filesystem/slug-safe identifier from a display name (lowercase, dashes).
    static QString slugify( const QString &name );

  private:
    QString resolveDir( const QString &relativeDir, QString *errorMessage ) const;

    QgsAiFileContextProvider *mContextProvider = nullptr;
};

Q_DECLARE_METATYPE( QgsAiRuleInfo )
Q_DECLARE_METATYPE( QgsAiSkillInfo )

#endif // QGSAIRULESSKILLSSTORE_H
