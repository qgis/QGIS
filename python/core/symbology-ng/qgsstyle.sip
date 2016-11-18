class QgsStyle : QObject
{
%TypeHeaderCode
#include <qgsstyle.h>
%End

  public:
    QgsStyle();
    ~QgsStyle();

    /** Enum for Entities involved in a style
    /*
     *  The enumerator is used for identifying the entity being operated on when generic
     *  database functions are being run.
     *  \sa group(), rename(), remove(), symbolsOfGroup(), symbolsWithTag(), symbolsOfSmartgroup()
     */
    enum StyleEntity { SymbolEntity, TagEntity, ColorrampEntity, SmartgroupEntity };

    /** Adds a color ramp to the style. Calling this method takes the ramp's ownership.
     *
     *  \note Adding a color ramp with the name of existing one replaces it.
     *  \param name is the name of the color ramp being added or updated
     *  \param colorRamp is the color ramp. Ownership is transferred.
     *  \param update set to true when the style DB has to be updated, by default it is false
     *  \return success status of the operation
     */
    bool addColorRamp( const QString& name, QgsColorRamp* colorRamp /Transfer/, bool update = false );

    /** Adds new smartgroup to the database and returns the id
     *
     *  \param name is the name of the new Smart Group to be added
     *  \param op is the operator between the conditions; AND/OR as QString
     *  \param conditions are the smart group conditions
     */
    int addSmartgroup( const QString& name, const QString& op, QMultiMap<QString, QString> conditions );

    /** Adds symbol to style and takes symbol's ownership
     *
     *  \note Adding a symbol with the name of existing one replaces it.
     *  \param name is the name of the symbol being added or updated
     *  \param symbol is the Vector symbol
     *  \param update set to true when the style DB has to be updated, by default it is false
     *  \return success status of the operation
     */
    bool addSymbol( const QString& name, QgsSymbol* symbol /Transfer/, bool update = false );

    /** Adds a new tag and returns the tag's id
     *
     *  \param tagName the name of the new tag to be created
     *  \return returns an int, which is the DB id of the new tag created, 0 if the tag couldn't be created
     */
    int addTag( const QString& tagName );

    /** Returns a list of all tags in the style database
     *  @note added in QGIS 2.16
     *  @see addTag()
     */
    QStringList tags() const;

    //! remove all contents of the style
    void clear();

    /** Returns a new copy of the specified color ramp. The caller
     * takes responsibility for deleting the returned object.
     */
    QgsColorRamp* colorRamp( const QString& name ) const /Factory/;

    //! Returns count of color ramps
    int colorRampCount();

    //! Returns a list of names of color ramps
    QStringList colorRampNames();

    //! Returns a const pointer to a symbol (doesn't create new instance)
    const QgsColorRamp* colorRampRef( const QString& name ) const;

    /** Returns the id in the style database for the given colorramp name
     *  returns 0 if not found
     */
    int colorrampId( const QString& name );

    //! Returns default application-wide style
    static QgsStyle* defaultStyle();

    /** Tags the symbol with the tags in the list
     *
     *  Applies the given tags to the given symbol or colorramp
     *  \param type is either SymbolEntity or ColorrampEntity
     *  \param symbol is the name of the symbol or colorramp as QString
     *  \param tags is the list of the tags that are to be applied as QStringList
     *  \return returns the success state of the operation
     */
    bool tagSymbol( StyleEntity type, const QString& symbol, const QStringList& tags );

    /** Detags the symbol with the given list
     *
     *  Removes the given tags for the specified symbol or colorramp
     *  \param type is either SymbolEntity or ColorrampEntity
     *  \param symbol is the name of the symbol or colorramp
     *  \param tags is the list of tags that are to be removed as QStringList
     *  \return returns the success state of the operation
     */
    bool detagSymbol( StyleEntity type, QString symbol, QStringList tags );
    /** Clears the symbol from all attached tags
     *
     *  Removes all tags for the specified symbol or colorramp
     *  \param type is either SymbolEntity or ColorrampEntity
     *  \param symbol is the name of the symbol or colorramp
     *  \return returns the success state of the operation
     */
    bool detagSymbol( StyleEntity type, QString symbol );

    //! Removes symbol from style (and delete it)
    bool removeSymbol( const QString& name );

    //! Changes symbol's name
    bool renameSymbol( const QString& oldName, const QString& newName );

    //! Returns a NEW copy of symbol
    QgsSymbol* symbol( const QString& name ) /Factory/;

    //! Returns a const pointer to a symbol (doesn't create new instance)
    const QgsSymbol* symbolRef( const QString& name ) const;

    //! Returns count of symbols in style
    int symbolCount();

    //! Returns a list of names of symbols
    QStringList symbolNames();

    /** Returns the id in the style database for the given symbol name
     *  returns 0 if not found
     */
    int symbolId( const QString& name );
    //! Returns the DB id for the given tag name
    int tagId( const QString& tag );
    //! Returns the DB id for the given smartgroup name
    int smartgroupId( const QString& smartgroup );

    /** Returns the symbol names which are flagged as favorite
     *
     *  \param type is either SymbolEntity or ColorampEntity
     *  \return A QStringList of the symbol or colorramp names flagged as favorite
     */
    QStringList symbolsOfFavorite( StyleEntity type );

    /** Returns the symbol names with which have the given tag
     *
     *  \param type is either SymbolEntity or ColorampEntity
     *  \param tagid is id of the tag which has been applied over the symbol as int
     *  \return A QStringList of the symbol or colorramp names for the given tag id
     */
    QStringList symbolsWithTag( StyleEntity type, int tagid );

    /** Adds the specified symbol to favorites
     *
     *  \param type is either SymbolEntity of ColorrampEntity
     *  \param name is the name of the symbol or coloramp whose is to be added to favorites
     *  \return returns the success state as bool
     */
    bool addFavorite( StyleEntity type, const QString& name );
    /** Removes the specified symbol from favorites
     *
     *  \param type is either SymbolEntity of ColorrampEntity
     *  \param name is the name of the symbol or coloramp whose is to be removed from favorites
     *  \return returns the success state as bool
     */
    bool removeFavorite( StyleEntity type, const QString& name );

    /** Renames the given entity with the specified id
     *
     *  \param type is any of the style entites. Refer enum StyleEntity.
     *  \param id is the DB id of the entity which is to be renamed
     *  \param newName is the new name of the entity
     */
    void rename( StyleEntity type, int id, const QString& newName );

    /** Removes the specified entity from the db
     *
     *  \param type is any of the style entites. Refer enum StyleEntity.
     *  \param id is the DB id of the entity to be removed
     */
    void remove( StyleEntity type, int id );

    /** Adds the symbol to the DB with the tags
     *
     *  \param name is the name of the symbol as QString
     *  \param symbol is the pointer to the new QgsSymbol being saved
     *  \param groupid is the id of the group to which the symbol belongs. Pass 0 if it doesn't belong to any group or not known.
     *  \param tags is a list of tags that are associated with the symbol as a QStringList.
     *  \return returns the success state of the save operation
     */
    bool saveSymbol( const QString& name, QgsSymbol* symbol /Transfer/, int groupid, const QStringList& tags );

    /** Adds the colorramp to the DB
     *
     *  \param name is the name of the colorramp as QString
     *  \param ramp is the pointer to the new QgsColorRamp being saved
     *  \param groupid is the id of the group to which the Color Ramp belongs. Pass 0 if it doesn't belong to any group or not known.
     *  \param tags is a list of tags that are associated with the color ramp as a QStringList.
     *  \return returns the success state of the save operation
     */
    bool saveColorRamp( const QString& name, QgsColorRamp* ramp, int groupid, const QStringList& tags );

    //! Removes color ramp from style (and delete it)
    bool removeColorRamp( const QString& name );

    //! Changes ramp's name
    bool renameColorRamp( const QString& oldName, const QString& newName );

    //! Loads a file into the style
    bool load( const QString& filename );

    //! Saves style into a file (will use current filename if empty string is passed)
    bool save( QString filename = QString() );

    //! Returns last error from load/save operation
    QString errorString();

    //! Returns current file name of the style
    QString fileName();

    /** Returns the names of the symbols which have a matching 'substring' in its defintion
     *
     *  \param type is either SymbolEntity or ColorrampEntity
     *  \param qword is the query string to search the symbols or colorramps.
     *  \return A QStringList of the matched symbols or colorramps
     * */
    QStringList findSymbols( StyleEntity type, const QString& qword );

    /** Returns the tags associated with the symbol
     *
     *  \param type is either SymbolEntity or ColorrampEntity
     *  \param symbol is the name of the symbol or color ramp
     *  \return A QStringList of the tags that have been applied to that symbol/colorramp
     */
    QStringList tagsOfSymbol( StyleEntity type, const QString& symbol );

    //! Returns the tag name for the given id
    QString tag( int id ) const;

    //! Returns the smart groups map with id as key and name as value
    QMap<int, QString> smartgroupsListMap();

    //! Returns the smart groups list
    QStringList smartgroupNames();

    //! Returns the QgsSmartConditionMap for the given id
    QMultiMap<QString, QString> smartgroup( int id );

    //! Returns the operator for the smartgroup
    //clumsy implementation TODO create a class for smartgroups
    QString smartgroupOperator( int id );

    //! Returns the symbols for the smartgroup
    QStringList symbolsOfSmartgroup( StyleEntity type, int id );

    //! Exports the style as a XML file
    bool exportXml( const QString& filename );

    //! Imports the symbols and colorramps into the default style database from the given XML file
    bool importXml( const QString& filename );

  signals:
    //! Is emitted every time a new symbol has been added to the database
    void symbolSaved( const QString& name, QgsSymbol* symbol );
    //! Is emitted every time a tag or smartgroup has been added, removed, or renamed
    void groupsModified();

  protected:
    //! Convenience function to open the DB and return a sqlite3 object
    bool openDB( const QString& filename );

    /** Convenience function that would run queries which don't generate return values
     *  \param query query to run
     *  \param freeQuery release query memory
     *  \return success true on success
     */
    bool runEmptyQuery( char* query, bool freeQuery = true );

    //! Gets the id from the table for the given name from the database, 0 if not found
    int getId( const QString& table, const QString& name );

    //! Gets the name from the table for the given id from the database, empty if not found
    QString getName( const QString& table, int id ) const;

    /** Updates the properties of an existing symbol/colorramp
     *
     *  \note This should not be called separately, only called through addSymbol or addColorRamp
     *  \param type is either SymbolEntity or ColorrampEntity
     *  \param name is the name of an existing symbol or a color ramp
     *  \return Success state of the update operation
     */
    bool updateSymbol( StyleEntity type, const QString& name );
};
