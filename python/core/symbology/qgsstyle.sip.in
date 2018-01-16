/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/symbology/qgsstyle.h                                        *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/







typedef QMap<QString, QgsColorRamp * > QgsVectorColorRampMap;
typedef QMap<int, QString> QgsSymbolGroupMap;


typedef QMultiMap<QString, QString> QgsSmartConditionMap;

enum SymbolTable { SymbolId, SymbolName, SymbolXML, SymbolFavoriteId };
enum TagTable { TagId, TagName };
enum TagmapTable { TagmapTagId, TagmapSymbolId };
enum ColorrampTable { ColorrampId, ColorrampName, ColorrampXML, ColorrampFavoriteId };
enum SmartgroupTable { SmartgroupId, SmartgroupName, SmartgroupXML };

class QgsStyle : QObject
{

%TypeHeaderCode
#include "qgsstyle.h"
%End
  public:

    QgsStyle();
%Docstring
Constructor for QgsStyle.
%End
    ~QgsStyle();

    enum StyleEntity { SymbolEntity, TagEntity, ColorrampEntity, SmartgroupEntity };

    bool addSymbol( const QString &name, QgsSymbol *symbol /Transfer/, bool update = false );
%Docstring
Adds a symbol to style and takes symbol's ownership

.. note::

   Adding a symbol with the name of existing one replaces it.

:param name: is the name of the symbol being added or updated
:param symbol: is the Vector symbol
:param update: set to true when the style DB has to be updated, by default it is false

:return: success status of the operation
%End

    bool addColorRamp( const QString &name, QgsColorRamp *colorRamp /Transfer/, bool update = false );
%Docstring
Adds a color ramp to the style. Calling this method takes the ramp's ownership.

.. note::

   Adding a color ramp with the name of existing one replaces it.

:param name: is the name of the color ramp being added or updated
:param colorRamp: is the color ramp. Ownership is transferred.
:param update: set to true when the style DB has to be updated, by default it is false

:return: success status of the operation
%End

    int addTag( const QString &tagName );
%Docstring
Adds a new tag and returns the tag's id

:param tagName: the name of the new tag to be created

:return: returns an int, which is the DB id of the new tag created, 0 if the tag couldn't be created
%End

    int addSmartgroup( const QString &name, const QString &op, const QgsSmartConditionMap &conditions );
%Docstring
Adds a new smartgroup to the database and returns the id

:param name: is the name of the new Smart Group to be added
:param op: is the operator between the conditions; AND/OR as QString
:param conditions: are the smart group conditions
%End

    QStringList tags() const;
%Docstring
Returns a list of all tags in the style database

.. versionadded:: 2.16

.. seealso:: :py:func:`addTag`
%End

    void clear();
%Docstring
Removes all contents of the style
%End

    QgsColorRamp *colorRamp( const QString &name ) const /Factory/;
%Docstring
Returns a new copy of the specified color ramp. The caller
takes responsibility for deleting the returned object.
%End

    int colorRampCount();
%Docstring
Returns count of color ramps
%End

    QStringList colorRampNames();
%Docstring
Returns a list of names of color ramps
%End

    const QgsColorRamp *colorRampRef( const QString &name ) const;
%Docstring
Returns a const pointer to a symbol (doesn't create new instance)
%End

    int colorrampId( const QString &name );
%Docstring
Returns the id in the style database for the given colorramp name
returns 0 if not found
%End

    static QgsStyle *defaultStyle();
%Docstring
Returns default application-wide style
%End

    bool tagSymbol( StyleEntity type, const QString &symbol, const QStringList &tags );
%Docstring
Tags the symbol with the tags in the list

Applies the given tags to the given symbol or colorramp

:param type: is either SymbolEntity or ColorrampEntity
:param symbol: is the name of the symbol or colorramp as QString
:param tags: is the list of the tags that are to be applied as QStringList

:return: returns the success state of the operation
%End

    bool detagSymbol( StyleEntity type, const QString &symbol, const QStringList &tags );
%Docstring
Detags the symbol with the given list

Removes the given tags for the specified symbol or colorramp

:param type: is either SymbolEntity or ColorrampEntity
:param symbol: is the name of the symbol or colorramp
:param tags: is the list of tags that are to be removed as QStringList

:return: returns the success state of the operation
%End

    bool detagSymbol( StyleEntity type, const QString &symbol );
%Docstring
Clears the symbol from all attached tags

Removes all tags for the specified symbol or colorramp

:param type: is either SymbolEntity or ColorrampEntity
:param symbol: is the name of the symbol or colorramp

:return: returns the success state of the operation
%End

    bool removeSymbol( const QString &name );
%Docstring
Removes symbol from style (and delete it)
%End

    bool renameSymbol( const QString &oldName, const QString &newName );
%Docstring
Changessymbol's name
%End

    QgsSymbol *symbol( const QString &name ) /Factory/;
%Docstring
Returns a NEW copy of symbol
%End

    const QgsSymbol *symbolRef( const QString &name ) const;
%Docstring
Returns a const pointer to a symbol (doesn't create new instance)
%End

    int symbolCount();
%Docstring
Returns count of symbols in style
%End

    QStringList symbolNames();
%Docstring
Returns a list of names of symbols
%End

    int symbolId( const QString &name );
%Docstring
Returns the id in the style database for the given symbol name
returns 0 if not found
%End
    int tagId( const QString &tag );
%Docstring
Returns the DB id for the given tag name
%End
    int smartgroupId( const QString &smartgroup );
%Docstring
Returns the DB id for the given smartgroup name
%End

    QStringList symbolsOfFavorite( StyleEntity type ) const;
%Docstring
Returns the symbol names which are flagged as favorite

:param type: is either SymbolEntity or ColorampEntity

:return: A QStringList of the symbol or colorramp names flagged as favorite
%End

    QStringList symbolsWithTag( StyleEntity type, int tagid ) const;
%Docstring
Returns the symbol names with which have the given tag

:param type: is either SymbolEntity or ColorampEntity
:param tagid: is id of the tag which has been applied over the symbol as int

:return: A QStringList of the symbol or colorramp names for the given tag id
%End

    bool addFavorite( StyleEntity type, const QString &name );
%Docstring
Adds the specified symbol to favorites

:param type: is either SymbolEntity of ColorrampEntity
:param name: is the name of the symbol or coloramp whose is to be added to favorites

:return: returns the success state as bool
%End

    bool removeFavorite( StyleEntity type, const QString &name );
%Docstring
Removes the specified symbol from favorites

:param type: is either SymbolEntity of ColorrampEntity
:param name: is the name of the symbol or coloramp whose is to be removed from favorites

:return: returns the success state as bool
%End

    void rename( StyleEntity type, int id, const QString &newName );
%Docstring
Renames the given entity with the specified id

:param type: is any of the style entities. Refer enum StyleEntity.
:param id: is the DB id of the entity which is to be renamed
:param newName: is the new name of the entity
%End

    void remove( StyleEntity type, int id );
%Docstring
Removes the specified entity from the db

:param type: is any of the style entities. Refer enum StyleEntity.
:param id: is the DB id of the entity to be removed
%End

    bool saveSymbol( const QString &name, QgsSymbol *symbol, bool favorite, const QStringList &tags );
%Docstring
Adds the symbol to the DB with the tags

:param name: is the name of the symbol as QString
:param symbol: is the pointer to the new QgsSymbol being saved
:param favorite: is a boolean value to specify whether the symbol should be added to favorites
:param tags: is a list of tags that are associated with the symbol as a QStringList.

:return: returns the success state of the save operation
%End

    bool saveColorRamp( const QString &name, QgsColorRamp *ramp, bool favorite, const QStringList &tags );
%Docstring
Adds the colorramp to the DB

:param name: is the name of the colorramp as QString
:param ramp: is the pointer to the new QgsColorRamp being saved
:param favorite: is a boolean value to specify whether the colorramp should be added to favorites
:param tags: is a list of tags that are associated with the color ramp as a QStringList.

:return: returns the success state of the save operation
%End

    bool removeColorRamp( const QString &name );
%Docstring
Removes color ramp from style (and delete it)
%End

    bool renameColorRamp( const QString &oldName, const QString &newName );
%Docstring
Changes ramp's name
%End

    bool createDatabase( const QString &filename );
%Docstring
Creates an on-disk database

This function creates a new on-disk permanent style database.

:return: returns the success state of the database creation

.. versionadded:: 3.0

.. seealso:: :py:func:`createMemoryDb`
%End

    bool createMemoryDatabase();
%Docstring
Creates a temporary memory database

This function is used to create a temporary style database in case a permanent on-disk database is not needed.

:return: returns the success state of the temporary memory database creation

.. versionadded:: 3.0

.. seealso:: :py:func:`createDb`
%End

    void createTables();
%Docstring
Creates tables structure for new database

This function is used to create the tables structure in a newly-created database.

:return: returns the success state of the temporary memory database creation

.. versionadded:: 3.0

.. seealso:: :py:func:`createDB`
%End

    bool load( const QString &filename );
%Docstring
Loads a file into the style

This function will load an on-disk database and populate styles.

:param filename: location of the database to load styles from

:return: returns the success state of the database being loaded
%End

    bool save( QString filename = QString() );
%Docstring
Saves style into a file (will use current filename if empty string is passed)
%End

    QString errorString();
%Docstring
Returns last error from load/save operation
%End

    QString fileName();
%Docstring
Returns current file name of the style
%End

    QStringList findSymbols( StyleEntity type, const QString &qword );
%Docstring
Returns the names of the symbols which have a matching 'substring' in its definition

:param type: is either SymbolEntity or ColorrampEntity
:param qword: is the query string to search the symbols or colorramps.

:return: A QStringList of the matched symbols or colorramps
*
%End

    QStringList tagsOfSymbol( StyleEntity type, const QString &symbol );
%Docstring
Returns the tags associated with the symbol

:param type: is either SymbolEntity or ColorrampEntity
:param symbol: is the name of the symbol or color ramp

:return: A QStringList of the tags that have been applied to that symbol/colorramp
%End

    bool symbolHasTag( StyleEntity type, const QString &symbol, const QString &tag );
%Docstring
Returns whether a given tag is associated with the symbol

:param type: is either SymbolEntity or ColorrampEntity
:param symbol: is the name of the symbol or color ramp
:param tag: the name of the tag to look for

:return: A boolean value identicating whether a tag was found attached to the symbol
%End

    QString tag( int id ) const;
%Docstring
Returns the tag name for the given id
%End

    QgsSymbolGroupMap smartgroupsListMap();
%Docstring
Returns the smart groups map with id as key and name as value
%End

    QStringList smartgroupNames();
%Docstring
Returns the smart groups list
%End

    QgsSmartConditionMap smartgroup( int id );
%Docstring
Returns the QgsSmartConditionMap for the given id
%End

    QString smartgroupOperator( int id );
%Docstring
Returns the operator for the smartgroup
clumsy implementation TODO create a class for smartgroups
%End

    QStringList symbolsOfSmartgroup( StyleEntity type, int id );
%Docstring
Returns the symbols for the smartgroup
%End

    bool exportXml( const QString &filename );
%Docstring
Exports the style as a XML file
%End

    bool importXml( const QString &filename );
%Docstring
Imports the symbols and colorramps into the default style database from the given XML file
%End

  signals:
    void symbolSaved( const QString &name, QgsSymbol *symbol );
%Docstring
Is emitted every time a new symbol has been added to the database
%End
    void groupsModified();
%Docstring
Is emitted every time a tag or smartgroup has been added, removed, or renamed
%End

  protected:





    bool openDatabase( const QString &filename );
%Docstring
Convenience function to open the DB and return a sqlite3 object
%End

    bool runEmptyQuery( char *query, bool freeQuery = true );
%Docstring
Convenience function that would run queries which don't generate return values

:param query: query to run
:param freeQuery: release query memory

:return: success true on success
%End

    int getId( const QString &table, const QString &name );
%Docstring
Gets the id from the table for the given name from the database, 0 if not found
%End

    QString getName( const QString &table, int id ) const;
%Docstring
Gets the name from the table for the given id from the database, empty if not found
%End

    bool updateSymbol( StyleEntity type, const QString &name );
%Docstring
Updates the properties of an existing symbol/colorramp

.. note::

   This should not be called separately, only called through addSymbol or addColorRamp

:param type: is either SymbolEntity or ColorrampEntity
:param name: is the name of an existing symbol or a color ramp

:return: Success state of the update operation
%End

};


/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/symbology/qgsstyle.h                                        *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
