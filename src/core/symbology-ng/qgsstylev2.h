/***************************************************************************
    qgsstylev2.h
    ---------------------
    begin                : November 2009
    copyright            : (C) 2009 by Martin Dobias
    email                : wonder.sk at gmail.com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSSTYLEV2_H
#define QGSSTYLEV2_H

#include <QMap>
#include <QMultiMap>
#include <QString>

#include <sqlite3.h>

#include "qgssymbollayerv2utils.h" // QgsStringMap

class QgsSymbolV2;
class QgsSymbolLayerV2;
class QgsVectorColorRampV2;

class QDomDocument;
class QDomElement;

typedef QMap<QString, QgsVectorColorRampV2* > QgsVectorColorRampV2Map;
typedef QMap<int, QString> QgsSymbolGroupMap;

/*!
 *  A multimap to hold the smart group conditions as constraint and parameter pairs.
 *  Both the key and the value of the map are QString. The key is the constraint of the condition and the value is the parameter which is applied for the constraint.
 *
 *  The supported constraints are:
 *  tag -> symbol has the tag matching the parameter
 *  !tag -> symbol doesnot have the tag matching the parameter
 *  group -> symbol belongs to group specified by the parameter
 *  !group -> symbol doesn't belong to the group specified by the parameter
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
enum SymbolTable { SymbolId, SymbolName, SymbolXML, SymbolGroupId };
enum SymgroupTable { SymgroupId, SymgroupName, SymgroupParent };
enum TagTable { TagId, TagName };
enum TagmapTable { TagmapTagId, TagmapSymbolId };
enum ColorrampTable { ColorrampId, ColorrampName, ColorrampXML, ColorrampGroupId };
enum SmartgroupTable { SmartgroupId, SmartgroupName, SmartgroupXML };

//! Enum for Entities involved in a style
/*!
    The enumarator is used for identifying the entity being operated on when generic
    database functions are being run.
    \sa group(), rename(), remove(), symbolsOfGroup(), symbolsWithTag(), symbolsOfSmartgroup()
 */
enum StyleEntity { SymbolEntity, GroupEntity, TagEntity, ColorrampEntity, SmartgroupEntity };

class CORE_EXPORT QgsStyleV2
{
  public:

    QgsStyleV2();
    ~QgsStyleV2();

    //! add color ramp to style. takes ramp's ownership
    /*!
     *  \note Adding a color ramp with the name of existing one replaces it.
     *  \param name is the name of the color ramp being added or updated
     *  \param colorRamp is the Vector color ramp
     *  \param update set to true when the style DB has to be updated, by default it is false
     *  \return sucess status of the operation
     */
    bool addColorRamp( QString name, QgsVectorColorRampV2* colorRamp, bool update = false );

    //! adds a new group and returns the group's id
    /*!
     *  \param groupname the name of the new group as QString
     *  \param parent is the id of the parent group when a subgrouo is to be created. By default it is 0 indicating it is not a sub-group
     *  \return returns an int, which is the DB id of the new group created, 0 if the group couldn't be created
     */
    int addGroup( QString groupName, int parent = 0 );

    //! adds new smartgroup to the database and returns the id
    /*!
     *  \param name is the name of the new Smart Group to be added
     *  \param op is the operator between the conditions; AND/OR as QString
     *  \param conditions are the smart group conditions
     */
    int addSmartgroup( QString name, QString op, QgsSmartConditionMap conditions );

    //! add symbol to style. takes symbol's ownership
    /*!
     *  \note Adding a symbol with the name of existing one replaces it.
     *  \param name is the name of the symbol being added or updated
     *  \param symbol is the Vector symbol
     *  \param update set to true when the style DB has to be updated, by default it is false
     *  \return sucess status of the operation
     */
    bool addSymbol( QString name, QgsSymbolV2* symbol, bool update = false );

    //! adds a new tag and returns the tag's id
    /*!
     *  \param tagName the name of the new tag to be created
     *  \return returns an int, which is the DB id of the new tag created, 0 if the tag couldn't be created
     */
    int addTag( QString tagName );

    //! return a map of groupid and names for the given parent group
    QgsSymbolGroupMap childGroupNames( QString parent = "" );

    //! remove all contents of the style
    void clear();

    //! return a NEW copy of color ramp
    QgsVectorColorRampV2* colorRamp( QString name );

    //! return count of color ramps
    int colorRampCount();

    //! return a list of names of color ramps
    QStringList colorRampNames();

    //! return a const pointer to a symbol (doesn't create new instance)
    const QgsVectorColorRampV2* colorRampRef( QString name ) const;

    //! return the id in the style database for the given colorramp name
    //! returns 0 if not found
    int colorrampId( QString name );

    //! return default application-wide style
    static QgsStyleV2* defaultStyle();

    //! tags the symbol with the tags in the list
    /*!
     *  Applies the given tags to the given symbol or colorramp
     *  \param type is either SymbolEntity or ColorrampEntity
     *  \param symbol is the name of the symbol or colorramp as QString
     *  \param tags is the list of the tags that are to be applied as QStringList
     *  \return returns the success state of the operation
     */
    bool tagSymbol( StyleEntity type, QString symbol, QStringList tags );

    //! detags the symbol with the given list
    /*!
     *  Removes the given tags for the specified symbol or colorramp
     *  \param type is either SymbolEntity or ColorrampEntity
     *  \param symbol is the name of the symbol or colorramp
     *  \param tags is the list of tags that are to be removed as QStringList
     *  \return returns the sucess state of the operation
     */
    bool detagSymbol( StyleEntity type, QString symbol, QStringList tags );

    //! remove symbol from style (and delete it)
    bool removeSymbol( QString name );

    //! change symbol's name
    //! @note added in v1.7
    bool renameSymbol( QString oldName, QString newName );

    //! return a NEW copy of symbol
    QgsSymbolV2* symbol( QString name );

    //! return a const pointer to a symbol (doesn't create new instance)
    const QgsSymbolV2* symbolRef( QString name ) const;

    //! return count of symbols in style
    int symbolCount();

    //! return a list of names of symbols
    QStringList symbolNames();

    //! return the id in the style database for the given symbol name
    //! returns 0 if not found
    int symbolId( QString name );
    //! return the DB id for the given group name
    int groupId( QString group );
    //! return the DB id for the given tag name
    int tagId( QString tag );
    //! return the DB id for the given smartgroup name
    int smartgroupId( QString smartgroup );

    //! return the all the groups in the style
    QStringList groupNames();

    //! returns the symbolnames of a given groupid
    /*!
     *  \param type is either SymbolEntity or ColorampEntity
     *  \param groupid is id of the group to which the symbols belong to, as int
     *  \return A QStringList of the symbol or colorramp names for the given group id
     */
    QStringList symbolsOfGroup( StyleEntity type, int groupid );

    //! returns the symbol names with which have the given tag
    /*!
     *  \param type is either SymbolEntity or ColorampEntity
     *  \param tagid is id of the tag which has been applied over the symbol as int
     *  \return A QStringList of the symbol or colorramp names for the given tag id
     */
    QStringList symbolsWithTag( StyleEntity type, int tagid );

    //! applies the specifed group to the symbol or colorramp specified by StyleEntity
    /*!
     *  \param type is either SymbolEntity of ColorrampEntity
     *  \param name is the name of the symbol or coloramp whose group is to be set
     *  \param groupid is the id of the group to which the entity is assigned
     *  \return returns the success state as bool
     */
    bool group( StyleEntity type, QString name, int groupid );

    //! rename the given entity with the specified id
    /*!
     *  \param type is any of the style entites. Refer enum StyleEntity.
     *  \param id is the DB id of the entity which is to be renamed
     *  \param newName is the new name of the entity
     */
    void rename( StyleEntity type, int id, QString newName );

    //! remove the specified entity from the db
    /*!
     *  \param type is any of the style entites. Refer enum StyleEntity.
     *  \param id is the DB id of the entity to be removed
     */
    void remove( StyleEntity type, int id );

    //! add the symbol to the DB with the tags
    /*!
     *  \param name is the name of the symbol as QString
     *  \param symbol is the pointer to the new QgsSymbolV2 being saved
     *  \param groupid is the id of the group to which the symbol belongs. Pass 0 if it doesn't belong to any group or not known.
     *  \param tags is a list of tags that are associated with the symbol as a QStringList.
     *  \return returns the sucess state of the save operation
     */
    bool saveSymbol( QString name, QgsSymbolV2* symbol, int groupid, QStringList tags );

    //! add the colorramp to the DB
    /*!
     *  \param name is the name of the colorramp as QString
     *  \param ramp is the pointer to the new QgsVectorColorRampV2 being saved
     *  \param groupid is the id of the group to which the Color Ramp belongs. Pass 0 if it doesn't belong to any group or not known.
     *  \param tags is a list of tags that are associated with the color ramp as a QStringList.
     *  \return returns the sucess state of the save operation
     */
    bool saveColorRamp( QString name, QgsVectorColorRampV2* ramp, int groupid, QStringList tags );

    //! remove color ramp from style (and delete it)
    bool removeColorRamp( QString name );

    //! change ramp's name
    //! @note added in v1.7
    bool renameColorRamp( QString oldName, QString newName );


    //! load a file into the style
    bool load( QString filename );

    //! save style into a file (will use current filename if empty string is passed)
    bool save( QString filename = QString() );

    //! return last error from load/save operation
    QString errorString() { return mErrorString; }

    //! return current file name of the style
    QString fileName() { return mFileName; }

    //! return the names of the symbols which have a matching 'substring' in its defintion
    QStringList findSymbols( QString qword );

    //! return the tags associated with the symbol
    /*!
     *  \param type is either SymbolEntity or ColorrampEntity
     *  \param symbol is the name of the symbol or color ramp
     *  \return A QStringList of the tags that have been applied to that symbol/colorramp
     */
    QStringList tagsOfSymbol( StyleEntity type, QString symbol );

    //! returns the smart groups map with id as key and name as value
    QgsSymbolGroupMap smartgroupsListMap();

    //! returns the smart groups list
    QStringList smartgroupNames();

    //! returns the QgsSmartConditionMap for the given id
    QgsSmartConditionMap smartgroup( int id );

    //! returns the operator for the smartgroup
    //clumsy implementation TODO create a class for smartgroups
    QString smartgroupOperator( int id );

    //! returns the symbols for the smartgroup
    QStringList symbolsOfSmartgroup( StyleEntity type, int id );

    //! Exports the style as a XML file
    bool exportXML( QString filename );

    //! Imports the symbols and colorramps into the default style database from the given XML file
    bool importXML( QString filename );

  protected:

    QgsSymbolV2Map mSymbols;
    QgsVectorColorRampV2Map mColorRamps;

    QString mErrorString;
    QString mFileName;

    sqlite3* mCurrentDB;

    static QgsStyleV2* mDefaultStyle;

    //! convenience function to open the DB and return a sqlite3 object
    bool openDB( QString filename );

    //! convenience function that would run queries which don't generate return values
    //! \param query query to run
    //! \param freeQuery release query memory
    //! \return success true on success
    bool runEmptyQuery( char* query, bool freeQuery = true );

    //! prepares the complex query for removing a group, so that the children are not abandoned
    char* getGroupRemoveQuery( int id );

    //! gets the id from the table for the given name from the database, 0 if not found
    int getId( QString table, QString name );

    //! updates the properties of an existing symbol/colorramp
    /*!
     *  \note This should not be called seperately, only called through addSymbol or addColorRamp
     *  \param type is either SymbolEntity or ColorrampEntity
     *  \param name is the name of an existing symbol or a color ramp
     *  \return Success state of the update operation
     */
    bool updateSymbol( StyleEntity type, QString name );
};


#endif
