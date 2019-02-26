/***************************************************************************
    qgsstyle.h
    ---------------------
    begin                : November 2009
    copyright            : (C) 2009 by Martin Dobias
    email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSSTYLE_H
#define QGSSTYLE_H

#include "qgis_core.h"
#include "qgis.h"
#include <QMap>
#include <QMultiMap>
#include <QString>

#include <sqlite3.h>

#include "qgssqliteutils.h"
#include "qgssymbollayerutils.h" // QgsStringMap

class QgsSymbol;
class QgsSymbolLayer;
class QgsColorRamp;

class QDomDocument;
class QDomElement;

typedef QMap<QString, QgsColorRamp * > QgsVectorColorRampMap;
typedef QMap<int, QString> QgsSymbolGroupMap;

/*
 * Constants used to describe copy-paste MIME types
 */
#define QGSCLIPBOARD_STYLE_MIME "application/qgis.style"

/**
 * \ingroup core
 *  A multimap to hold the smart group conditions as constraint and parameter pairs.
 *  Both the key and the value of the map are QString. The key is the constraint of the condition and the value is the parameter which is applied for the constraint.
 *
 *  The supported constraints are:
 *  tag -> symbol has the tag matching the parameter
 *  !tag -> symbol doesn't have the tag matching the parameter
 *  name -> symbol has a part of its name matching the parameter
 *  !name -> symbol doesn't have any part of the name matching the parameter
 *
 *  Example Usage:
 *  QgsSmartConditionMap conditions;
 *  conditions.insert( "tag", "red" ); // adds the condition: Symbol has the tag red
 *  conditions.insert( "!name", "way" ); // add the condition: Symbol doesn't have any part of its name matching `way`
 *
 *  \note This is a Multimap, which means it will contain multiple values for the same key.
 */
typedef QMultiMap<QString, QString> QgsSmartConditionMap;

// enumerators representing sqlite DB columns
enum SymbolTable { SymbolId, SymbolName, SymbolXML, SymbolFavoriteId };
enum TagTable { TagId, TagName };
enum TagmapTable { TagmapTagId, TagmapSymbolId };
enum ColorrampTable { ColorrampId, ColorrampName, ColorrampXML, ColorrampFavoriteId };
enum SmartgroupTable { SmartgroupId, SmartgroupName, SmartgroupXML };

/**
 * \ingroup core
 * \class QgsStyle
 */
class CORE_EXPORT QgsStyle : public QObject
{
    Q_OBJECT

  public:

    /**
     * Constructor for QgsStyle.
     */
    QgsStyle() = default;
    ~QgsStyle() override;

    /**
     * Enum for Entities involved in a style
     *
     *  The enumerator is used for identifying the entity being operated on when generic
     *  database functions are being run.
     *  \sa rename(), remove(), symbolsOfFavorite(), symbolsWithTag(), symbolsOfSmartgroup()
     */
    enum StyleEntity
    {
      SymbolEntity,
      TagEntity,
      ColorrampEntity,
      SmartgroupEntity
    };

    /**
     * Adds a symbol to style and takes symbol's ownership
     *
     *  \note Adding a symbol with the name of existing one replaces it.
     *  \param name is the name of the symbol being added or updated
     *  \param symbol is the Vector symbol
     *  \param update set to TRUE when the style DB has to be updated, by default it is FALSE
     *  \returns success status of the operation
     */
    bool addSymbol( const QString &name, QgsSymbol *symbol SIP_TRANSFER, bool update = false );

    /**
     * Adds a color ramp to the style. Calling this method takes the ramp's ownership.
     *  \note Adding a color ramp with the name of existing one replaces it.
     *  \param name is the name of the color ramp being added or updated
     *  \param colorRamp is the color ramp. Ownership is transferred.
     *  \param update set to TRUE when the style DB has to be updated, by default it is FALSE
     *  \returns success status of the operation
     */
    bool addColorRamp( const QString &name, QgsColorRamp *colorRamp SIP_TRANSFER, bool update = false );

    /**
     * Adds a new tag and returns the tag's id
     *
     *  \param tagName the name of the new tag to be created
     *  \returns returns an int, which is the DB id of the new tag created, 0 if the tag couldn't be created
     */
    int addTag( const QString &tagName );

    /**
     * Adds a new smartgroup to the database and returns the id
     *
     *  \param name is the name of the new Smart Group to be added
     *  \param op is the operator between the conditions; AND/OR as QString
     *  \param conditions are the smart group conditions
     *
     * \note Not available from Python bindings
     */
    int addSmartgroup( const QString &name, const QString &op, const QgsSmartConditionMap &conditions ) SIP_SKIP;

    /**
     * Adds a new smartgroup to the database and returns the id.
     *
     * \param name is the name of the new Smart Group to be added
     * \param op is the operator between the conditions; AND/OR as QString
     * \param matchTag list of strings to match within tags
     * \param noMatchTag list of strings to exclude matches from tags
     * \param matchName list of string to match within names
     * \param noMatchName list of strings to exclude matches from names
     *
     * \since QGIS 3.4
     */
    int addSmartgroup( const QString &name, const QString &op, const QStringList &matchTag, const QStringList &noMatchTag,
                       const QStringList &matchName, const QStringList &noMatchName );

    /**
     * Returns a list of all tags in the style database
     *
     * \see addTag()
     * \since QGIS 2.16
     */
    QStringList tags() const;

    //! Removes all contents of the style
    void clear();

    /**
     * Returns a new copy of the specified color ramp. The caller
     * takes responsibility for deleting the returned object.
     */
    QgsColorRamp *colorRamp( const QString &name ) const SIP_FACTORY;

    //! Returns count of color ramps
    int colorRampCount();

    //! Returns a list of names of color ramps
    QStringList colorRampNames();

    //! Returns a const pointer to a symbol (doesn't create new instance)
    const QgsColorRamp *colorRampRef( const QString &name ) const;

    /**
     * Returns the id in the style database for the given colorramp name
     * returns 0 if not found
     */
    int colorrampId( const QString &name );

    //! Returns default application-wide style
    static QgsStyle *defaultStyle();

    //! Deletes the default style. Only to be used by QgsApplication::exitQgis()
    static void cleanDefaultStyle() SIP_SKIP;

    /**
     * Tags the symbol with the tags in the list
     *
     *  Applies the given tags to the given symbol or colorramp
     *  \param type is either SymbolEntity or ColorrampEntity
     *  \param symbol is the name of the symbol or colorramp as QString
     *  \param tags is the list of the tags that are to be applied as QStringList
     *  \returns returns the success state of the operation
     */
    bool tagSymbol( StyleEntity type, const QString &symbol, const QStringList &tags );

    /**
     * Detags the symbol with the given list
     *
     *  Removes the given tags for the specified symbol or colorramp
     *  \param type is either SymbolEntity or ColorrampEntity
     *  \param symbol is the name of the symbol or colorramp
     *  \param tags is the list of tags that are to be removed as QStringList
     *  \returns returns the success state of the operation
     */
    bool detagSymbol( StyleEntity type, const QString &symbol, const QStringList &tags );

    /**
     * Clears the symbol from all attached tags
     *
     *  Removes all tags for the specified symbol or colorramp
     *  \param type is either SymbolEntity or ColorrampEntity
     *  \param symbol is the name of the symbol or colorramp
     *  \returns returns the success state of the operation
     */
    bool detagSymbol( StyleEntity type, const QString &symbol );

    //! Removes symbol from style (and delete it)
    bool removeSymbol( const QString &name );

    /**
     * Renames a symbol from \a oldName to \a newName.
     *
     * Returns TRUE if symbol was successfully renamed.
     */
    bool renameSymbol( const QString &oldName, const QString &newName );

    //! Returns a NEW copy of symbol
    QgsSymbol *symbol( const QString &name ) SIP_FACTORY;

    //! Returns a const pointer to a symbol (doesn't create new instance)
    const QgsSymbol *symbolRef( const QString &name ) const;

    //! Returns count of symbols in style
    int symbolCount();

    //! Returns a list of names of symbols
    QStringList symbolNames();

    /**
     * Returns the id in the style database for the given symbol name
     * returns 0 if not found
     */
    int symbolId( const QString &name );
    //! Returns the DB id for the given tag name
    int tagId( const QString &tag );
    //! Returns the DB id for the given smartgroup name
    int smartgroupId( const QString &smartgroup );

    /**
     * Returns the symbol names which are flagged as favorite
     *
     *  \param type is either SymbolEntity or ColorampEntity
     *  \returns A QStringList of the symbol or colorramp names flagged as favorite
     */
    QStringList symbolsOfFavorite( StyleEntity type ) const;

    /**
     * Returns the symbol names with which have the given tag
     *
     *  \param type is either SymbolEntity or ColorampEntity
     *  \param tagid is id of the tag which has been applied over the symbol as int
     *  \returns A QStringList of the symbol or colorramp names for the given tag id
     */
    QStringList symbolsWithTag( StyleEntity type, int tagid ) const;

    /**
     * Adds the specified symbol to favorites
     *
     *  \param type is either SymbolEntity of ColorrampEntity
     *  \param name is the name of the symbol or coloramp whose is to be added to favorites
     *  \returns returns the success state as bool
     */
    bool addFavorite( StyleEntity type, const QString &name );

    /**
     * Removes the specified symbol from favorites
     *
     *  \param type is either SymbolEntity of ColorrampEntity
     *  \param name is the name of the symbol or coloramp whose is to be removed from favorites
     *  \returns returns the success state as bool
     */
    bool removeFavorite( StyleEntity type, const QString &name );

    /**
     * Renames the given entity with the specified id
     *
     *  \param type is any of the style entities. Refer enum StyleEntity.
     *  \param id is the DB id of the entity which is to be renamed
     *  \param newName is the new name of the entity
     */
    bool rename( StyleEntity type, int id, const QString &newName );

    /**
     * Removes the specified entity from the db
     *
     *  \param type is any of the style entities. Refer enum StyleEntity.
     *  \param id is the DB id of the entity to be removed
     */
    bool remove( StyleEntity type, int id );

    /**
     * Adds the symbol to the DB with the tags
     *
     *  \param name is the name of the symbol as QString
     *  \param symbol is the pointer to the new QgsSymbol being saved
     *  \param favorite is a boolean value to specify whether the symbol should be added to favorites
     *  \param tags is a list of tags that are associated with the symbol as a QStringList.
     *  \returns returns the success state of the save operation
     */
    bool saveSymbol( const QString &name, QgsSymbol *symbol, bool favorite, const QStringList &tags );

    /**
     * Adds the colorramp to the DB
     *
     *  \param name is the name of the colorramp as QString
     *  \param ramp is the pointer to the new QgsColorRamp being saved
     *  \param favorite is a boolean value to specify whether the colorramp should be added to favorites
     *  \param tags is a list of tags that are associated with the color ramp as a QStringList.
     *  \returns returns the success state of the save operation
     */
    bool saveColorRamp( const QString &name, QgsColorRamp *ramp, bool favorite, const QStringList &tags );

    //! Removes color ramp from style (and delete it)
    bool removeColorRamp( const QString &name );

    //! Changes ramp's name
    bool renameColorRamp( const QString &oldName, const QString &newName );

    /**
     * Creates an on-disk database
     *
     *  This function creates a new on-disk permanent style database.
     *  \returns returns the success state of the database creation
     *  \see createMemoryDatabase()
     *  \since QGIS 3.0
     */
    bool createDatabase( const QString &filename );

    /**
     * Creates a temporary memory database
     *
     *  This function is used to create a temporary style database in case a permanent on-disk database is not needed.
     *  \returns returns the success state of the temporary memory database creation
     *  \see createDatabase()
     *  \since QGIS 3.0
     */
    bool createMemoryDatabase();

    /**
     * Creates tables structure for new database
     *
     *  This function is used to create the tables structure in a newly-created database.
     *  \returns returns the success state of the temporary memory database creation
     *  \see createDatabase()
     *  \see createMemoryDatabase()
     *  \since QGIS 3.0
     */
    void createTables();

    /**
     * Loads a file into the style
     *
     *  This function will load an on-disk database and populate styles.
     *  \param filename location of the database to load styles from
     *  \returns returns the success state of the database being loaded
     */
    bool load( const QString &filename );

    //! Saves style into a file (will use current filename if empty string is passed)
    bool save( QString filename = QString() );

    //! Returns last error from load/save operation
    QString errorString() { return mErrorString; }

    //! Returns current file name of the style
    QString fileName() { return mFileName; }

    /**
     * Returns the names of the symbols which have a matching 'substring' in its definition
     *
     *  \param type is either SymbolEntity or ColorrampEntity
     *  \param qword is the query string to search the symbols or colorramps.
     *  \returns A QStringList of the matched symbols or colorramps
     * */
    QStringList findSymbols( StyleEntity type, const QString &qword );

    /**
     * Returns the tags associated with the symbol
     *
     *  \param type is either SymbolEntity or ColorrampEntity
     *  \param symbol is the name of the symbol or color ramp
     *  \returns A QStringList of the tags that have been applied to that symbol/colorramp
     */
    QStringList tagsOfSymbol( StyleEntity type, const QString &symbol );

    /**
     * Returns whether a given tag is associated with the symbol
     *
     *  \param type is either SymbolEntity or ColorrampEntity
     *  \param symbol is the name of the symbol or color ramp
     *  \param tag the name of the tag to look for
     *  \returns A boolean value identicating whether a tag was found attached to the symbol
     */
    bool symbolHasTag( StyleEntity type, const QString &symbol, const QString &tag );

    //! Returns the tag name for the given id
    QString tag( int id ) const;

    //! Returns the smart groups map with id as key and name as value
    QgsSymbolGroupMap smartgroupsListMap();

    //! Returns the smart groups list
    QStringList smartgroupNames();

    //! Returns the QgsSmartConditionMap for the given id
    QgsSmartConditionMap smartgroup( int id );

    /**
     * Returns the operator for the smartgroup
     * clumsy implementation TODO create a class for smartgroups
     */
    QString smartgroupOperator( int id );

    //! Returns the symbols for the smartgroup
    QStringList symbolsOfSmartgroup( StyleEntity type, int id );

    //! Exports the style as a XML file
    bool exportXml( const QString &filename );

    //! Imports the symbols and colorramps into the default style database from the given XML file
    bool importXml( const QString &filename );

    /**
     * Tests if the file at \a path is a QGIS style XML file.
     *
     * This method samples only the first line in the file, so is safe to call on
     * large xml files.
     *
     * \since QGIS 3.6
     */
    static bool isXmlStyleFile( const QString &path );

  signals:

    /**
     * Emitted every time a new symbol has been added to the database.
     * Emitted whenever a symbol has been added to the style and the database
     * has been updated as a result.
     * \see symbolRemoved()
     * \see rampAdded()
     * \see symbolChanged()
     */
    void symbolSaved( const QString &name, QgsSymbol *symbol );

    /**
     * Emitted whenever a symbol's definition is changed. This does not include
     * name or tag changes.
     *
     * \see symbolSaved()
     *
     * \since QGIS 3.4
     */
    void symbolChanged( const QString &name );

    //! Emitted every time a tag or smartgroup has been added, removed, or renamed
    void groupsModified();

    /**
     * Emitted whenever an \a entity's tags are changed.
     *
     * \since QGIS 3.4
     */
    void entityTagsChanged( QgsStyle::StyleEntity entity, const QString &name, const QStringList &newTags );

    /**
     * Emitted whenever an \a entity is either favorited or un-favorited.
     *
     * \since QGIS 3.4
     */
    void favoritedChanged( QgsStyle::StyleEntity entity, const QString &name, bool isFavorite );

    /**
     * Emitted whenever a symbol has been removed from the style and the database
     * has been updated as a result.
     * \see symbolSaved()
     * \see rampRemoved()
     * \since QGIS 3.4
     */
    void symbolRemoved( const QString &name );

    /**
     * Emitted whenever a symbol has been renamed from \a oldName to \a newName
     * \see rampRenamed()
     * \since QGIS 3.4
     */
    void symbolRenamed( const QString &oldName, const QString &newName );

    /**
     * Emitted whenever a color ramp has been renamed from \a oldName to \a newName
     * \see symbolRenamed()
     * \since QGIS 3.4
     */
    void rampRenamed( const QString &oldName, const QString &newName );

    /**
     * Emitted whenever a color ramp has been added to the style and the database
     * has been updated as a result.
     * \see rampRemoved()
     * \see symbolSaved()
     * \since QGIS 3.4
     */
    void rampAdded( const QString &name );

    /**
     * Emitted whenever a color ramp has been removed from the style and the database
     * has been updated as a result.
     * \see rampAdded()
     * \see symbolRemoved()
     * \since QGIS 3.4
     */
    void rampRemoved( const QString &name );

    /**
     * Emitted whenever a color ramp's definition is changed. This does not include
     * name or tag changes.
     *
     * \see rampAdded()
     *
     * \since QGIS 3.4
     */
    void rampChanged( const QString &name );

  private:

    QgsSymbolMap mSymbols;
    QgsVectorColorRampMap mColorRamps;

    QHash< QString, QStringList > mCachedSymbolTags;
    QHash< QString, QStringList > mCachedColorRampTags;

    QString mErrorString;
    QString mFileName;

    sqlite3_database_unique_ptr mCurrentDB;

    static QgsStyle *sDefaultStyle;

    //! Convenience function to open the DB and return a sqlite3 object
    bool openDatabase( const QString &filename );

    /**
     * Convenience function that would run queries which don't generate return values
     *
     *  \param query query to run
     *  \returns success TRUE on success
     */
    bool runEmptyQuery( const QString &query );

    //! Gets the id from the table for the given name from the database, 0 if not found
    int getId( const QString &table, const QString &name );

    //! Gets the name from the table for the given id from the database, empty if not found
    QString getName( const QString &table, int id ) const;

    /**
     * Updates the properties of an existing symbol/colorramp
     *
     *  \note This should not be called separately, only called through addSymbol or addColorRamp
     *  \param type is either SymbolEntity or ColorrampEntity
     *  \param name is the name of an existing symbol or a color ramp
     *  \returns Success state of the update operation
     */
    bool updateSymbol( StyleEntity type, const QString &name );

    void clearCachedTags( StyleEntity type, const QString &name );

    Q_DISABLE_COPY( QgsStyle )
};

#endif
