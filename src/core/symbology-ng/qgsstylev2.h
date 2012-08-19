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
typedef QMultiMap<QString, QString> QgsSmartConditionMap;

// enumerators representing sqlite DB columns
enum SymbolTable { SymbolId, SymbolName, SymbolXML, SymbolGroupId };
enum SymgroupTable { SymgroupId, SymgroupName, SymgroupParent };
enum TagTable { TagId, TagName };
enum TagmapTable { TagmapTagId, TagmapSymbolId };
enum ColorrampTable { ColorrampId, ColorrampName, ColorrampXML, ColorrampGroupId };
enum SmartgroupTable { SmartgroupId, SmartgroupName, SmartgroupXML };

// Enums for types
enum StyleEntity { SymbolEntity, GroupEntity, TagEntity, ColorrampEntity, SmartgroupEntity };

class CORE_EXPORT QgsStyleV2
{
  public:

    QgsStyleV2();
    ~QgsStyleV2();

    //! return default application-wide style
    static QgsStyleV2* defaultStyle();

    //! remove all contents of the style
    void clear();

    //! add symbol to style. takes symbol's ownership
    bool addSymbol( QString name, QgsSymbolV2* symbol );

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
    int colorrampId( QString name );

    //! return the id in the style database for the given group name
    int groupId( QString group );
    int tagId( QString tag );
    int smartgroupId( QString smartgroup );

    //! return the all the groups in the style
    QStringList groupNames();

    //! return a map of groupid and names for the given parent
    QgsSymbolGroupMap childGroupNames( QString parent = "" );

    //! returns the symbolnames of a given groupid
    QStringList symbolsOfGroup( StyleEntity type, int groupid );
    //! returns the symbol names with which have the given tag
    QStringList symbolsWithTag( StyleEntity type, int tagid );
    //! adds a new group and returns the group's id
    int addGroup( QString groupName, int parent = 0 );
    //! adds a new tag and returns the tag's id
    int addTag( QString tagName );

    //! applies the specifed group to the symbol or colorramp specified by StyleEntity
    bool group( StyleEntity type, QString name, int groupid );

    //! rename the given entity with the specified id
    void rename( StyleEntity type, int id, QString newName );
    //! remove the specified entity from the db
    void remove( StyleEntity type, int id );

    //! add the symbol to the DB with the tags
    bool saveSymbol( QString name, QgsSymbolV2* symbol, int groupid, QStringList tags );
    //! add the colorramp to the DB
    bool saveColorRamp( QString name, QgsVectorColorRampV2* ramp, int groupid, QStringList tags );

    //! add color ramp to style. takes ramp's ownership
    bool addColorRamp( QString name, QgsVectorColorRampV2* colorRamp );

    //! remove color ramp from style (and delete it)
    bool removeColorRamp( QString name );

    //! change ramp's name
    //! @note added in v1.7
    bool renameColorRamp( QString oldName, QString newName );

    //! return a NEW copy of color ramp
    QgsVectorColorRampV2* colorRamp( QString name );

    //! return a const pointer to a symbol (doesn't create new instance)
    const QgsVectorColorRampV2* colorRampRef( QString name ) const;

    //! return count of color ramps
    int colorRampCount();

    //! return a list of names of color ramps
    QStringList colorRampNames();


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

    //! tags the symbol with the tags in the list, the remove flag DE-TAGS
    bool tagSymbol( StyleEntity type, QString symbol, QStringList tags );

    //! detags the symbol with the given list
    bool detagSymbol( StyleEntity type, QString symbol, QStringList tags );

    //! return the tags associated with the symbol
    QStringList tagsOfSymbol( StyleEntity type, QString symbol );

    //! adds the smartgroup to the database and returns the id
    int addSmartgroup( QString name, QString op, QgsSmartConditionMap conditions );

    //! returns the smart groups map
    QgsSymbolGroupMap smartgroupsListMap();

    //! returns the smart groups list
    QStringList smartgroupNames();

    //! returns the QgsSmartConditionMap for the given id
    QgsSmartConditionMap smartgroup( int id );

    //! returns the operator for the smartgroup
    //! @note: clumsy implementation TODO create a class for smartgroups
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
    //! @param query query to run
    //! @param freeQuery release query memory
    //! @return success true on success
    bool runEmptyQuery( char* query, bool freeQuery = true );
    //! prepares the complex query for removing a group, so that the children are not abandoned
    char* getGroupRemoveQuery( int id );
    //! gets the id from the table for the given name from the database, 0 if not found
    int getId( QString table, QString name );
};


#endif
