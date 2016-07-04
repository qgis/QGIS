/***************************************************************************
                          qgseditformconfig.h
                             -------------------
    begin                : Nov 18, 2015
    copyright            : (C) 2015 by Matthias Kuhn
    email                : matthias at opengis dot ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSEDITFORMCONFIG_H
#define QGSEDITFORMCONFIG_H

#include <QMap>

#include "qgseditorwidgetconfig.h"
#include "qgsrelationmanager.h"

/** \ingroup core
 * This is an abstract base class for any elements of a drag and drop form.
 *
 * This can either be a container which will be represented on the screen
 * as a tab widget or ca collapsible group box. Or it can be a field which will
 * then be represented based on the QgsEditorWidgetV2 type and configuration.
 * Or it can be a relation and embed the form of several children of another
 * layer.
 */
class CORE_EXPORT QgsAttributeEditorElement : public QObject
{
    Q_OBJECT
  public:

    enum AttributeEditorType
    {
      AeTypeContainer, //!< A container
      AeTypeField,     //!< A field
      AeTypeRelation,  //!< A relation
      AeTypeInvalid    //!< Invalid
    };

    /**
     * Constructor
     *
     * @param type The type of the new element. Should never
     * @param name
     * @param parent
     */
    QgsAttributeEditorElement( AttributeEditorType type, const QString& name, QObject *parent = nullptr )
        : QObject( parent )
        , mType( type )
        , mName( name )
    {}

    //! Destructor
    virtual ~QgsAttributeEditorElement() {}

    /**
     * Return the name of this element
     *
     * @return The name for this element
     */
    QString name() const { return mName; }

    /**
     * The type of this element
     *
     * @return The type
     */
    AttributeEditorType type() const { return mType; }

    /**
     * Is reimplemented in classes inheriting from this to serialize it.
     *
     * @param doc The QDomDocument which is used to create new XML elements
     *
     * @return An DOM element which represents this element
     */
    virtual QDomElement toDomElement( QDomDocument& doc ) const = 0;

  protected:
    AttributeEditorType mType;
    QString mName;
};


/** \ingroup core
 * This is a container for attribute editors, used to group them visually in the
 * attribute form if it is set to the drag and drop designer.
 */
class CORE_EXPORT QgsAttributeEditorContainer : public QgsAttributeEditorElement
{
    Q_OBJECT

  public:
    /**
     * Creates a new attribute editor container
     *
     * @param name   The name to show as title
     * @param parent The parent. May be another container.
     */
    QgsAttributeEditorContainer( const QString& name, QObject *parent )
        : QgsAttributeEditorElement( AeTypeContainer, name, parent )
        , mIsGroupBox( true )
        , mColumnCount( 1 )
    {}

    //! Destructor
    virtual ~QgsAttributeEditorContainer() {}

    /**
     * Will serialize this containers information into a QDomElement for saving it in an XML file.
     *
     * @param doc The QDomDocument used to generate the QDomElement
     *
     * @return The XML element
     */
    virtual QDomElement toDomElement( QDomDocument& doc ) const override;

    /**
     * Add a child element to this container. This may be another container, a field or a relation.
     *
     * @param element The element to add as child
     */
    virtual void addChildElement( QgsAttributeEditorElement* element );

    /**
     * Determines if this container is rendered as collapsible group box or tab in a tabwidget
     *
     * @param isGroupBox If true, this will be a group box
     */
    virtual void setIsGroupBox( bool isGroupBox ) { mIsGroupBox = isGroupBox; }

    /**
     * Returns if this container is going to be rendered as a group box
     *
     * @return True if it will be a group box, false if it will be a tab
     */
    virtual bool isGroupBox() const { return mIsGroupBox; }

    /**
     * Get a list of the children elements of this container
     *
     * @return A list of elements
     */
    QList<QgsAttributeEditorElement*> children() const { return mChildren; }

    /**
     * Traverses the element tree to find any element of the specified type
     *
     * @param type The type which should be searched
     *
     * @return A list of elements of the type which has been searched for
     */
    virtual QList<QgsAttributeEditorElement*> findElements( AttributeEditorType type ) const;

    /**
     * Change the name of this container
     */
    void setName( const QString& name );

    /**
     * Get the number of columns in this group
     */
    int columnCount() const;

    /**
     * Set the number of columns in this group
     */
    void setColumnCount( int columnCount );

  private:
    bool mIsGroupBox;
    QList<QgsAttributeEditorElement*> mChildren;
    int mColumnCount;
};

/** \ingroup core
 * This element will load a field's widget onto the form.
 */
class CORE_EXPORT QgsAttributeEditorField : public QgsAttributeEditorElement
{
    Q_OBJECT

  public:
    /**
     * Creates a new attribute editor element which represents a field
     *
     * @param name   The name of the element
     * @param idx    The index of the field which should be embedded
     * @param parent The parent of this widget (used as container)
     */
    QgsAttributeEditorField( const QString& name, int idx, QObject *parent )
        : QgsAttributeEditorElement( AeTypeField, name, parent )
        , mIdx( idx )
    {}

    //! Destructor
    virtual ~QgsAttributeEditorField() {}

    /**
     * Will serialize this elements information into a QDomElement for saving it in an XML file.
     *
     * @param doc The QDomDocument used to generate the QDomElement
     *
     * @return The XML element
     */
    virtual QDomElement toDomElement( QDomDocument& doc ) const override;

    /**
     * Return the index of the field
     * @return
     */
    int idx() const { return mIdx; }

  private:
    int mIdx;
};

/** \ingroup core
 * This element will load a relation editor onto the form.
 */
class CORE_EXPORT QgsAttributeEditorRelation : public QgsAttributeEditorElement
{
    Q_OBJECT

  public:
    /**
     * Creates a new element which embeds a relation.
     *
     * @param name         The name of this element
     * @param relationId   The id of the relation to embed
     * @param parent       The parent (used as container)
     */
    QgsAttributeEditorRelation( const QString& name, const QString &relationId, QObject *parent )
        : QgsAttributeEditorElement( AeTypeRelation, name, parent )
        , mRelationId( relationId ) {}

    /**
     * Creates a new element which embeds a relation.
     *
     * @param name         The name of this element
     * @param relation     The relation to embed
     * @param parent       The parent (used as container)
     */
    QgsAttributeEditorRelation( const QString& name, const QgsRelation& relation, QObject *parent )
        : QgsAttributeEditorElement( AeTypeRelation, name, parent )
        , mRelationId( relation.id() )
        , mRelation( relation ) {}

    //! Destructor
    virtual ~QgsAttributeEditorRelation() {}

    /**
     * Will serialize this elements information into a QDomElement for saving it in an XML file.
     *
     * @param doc The QDomDocument used to generate the QDomElement
     *
     * @return The XML element
     */
    virtual QDomElement toDomElement( QDomDocument& doc ) const override;

    /**
     * Get the id of the relation which shall be embedded
     *
     * @return the id
     */
    const QgsRelation& relation() const { return mRelation; }

    /**
     * Initializes the relation from the id
     *
     * @param relManager The relation manager to use for the initialization
     * @return true if the relation was found in the relationmanager
     */
    bool init( QgsRelationManager *relManager );

  private:
    QString mRelationId;
    QgsRelation mRelation;
};


/** \ingroup core
 * \class QgsEditFormConfig
 */
class CORE_EXPORT QgsEditFormConfig : public QObject
{
    Q_OBJECT

  public:

    /** The different types to layout the attribute editor. */
    enum EditorLayout
    {
      GeneratedLayout = 0, //!< Autogenerate a simple tabular layout for the form
      TabLayout = 1,       //!< Use a layout with tabs and group boxes. Needs to be configured.
      UiFileLayout = 2     //!< Load a .ui file for the layout. Needs to be configured.
    };

    struct GroupData
    {
      GroupData() {}
      GroupData( const QString& name, const QList<QString>& fields )
          : mName( name )
          , mFields( fields )
      {}
      QString mName;
      QList<QString> mFields;
    };

    struct TabData
    {
      TabData() {}
      TabData( const QString& name, const QList<QString>& fields, const QList<GroupData>& groups )
          : mName( name )
          , mFields( fields )
          , mGroups( groups )
      {}
      QString mName;
      QList<QString> mFields;
      QList<GroupData> mGroups;
    };

    /**
     * Types of feature form suppression after feature creation
     */
    enum FeatureFormSuppress
    {
      SuppressDefault = 0, //!< Use the application-wide setting
      SuppressOn = 1,      //!< Suppress feature form
      SuppressOff = 2      //!< Do not suppress feature form
    };

    /**
     * The python init code source options.
     */
    enum PythonInitCodeSource
    {
      CodeSourceNone = 0,             //!< Do not use python code at all
      CodeSourceFile = 1,             //!< Load the python code from an external file
      CodeSourceDialog = 2,           //!< Use the python code provided in the dialog
      CodeSourceEnvironment = 3       //!< Use the python code available in the python environment
    };

    /**
     * This is only useful in combination with EditorLayout::TabLayout.
     *
     * Adds a new element to the layout.
     */
    void addTab( QgsAttributeEditorElement* data ) { mAttributeEditorElements.append( data ); }

    /**
     * Returns a list of tabs for EditorLayout::TabLayout.
     */
    QList< QgsAttributeEditorElement* > tabs() const { return mAttributeEditorElements; }

    /**
     * Clears all the tabs for the attribute editor form with EditorLayout::TabLayout.
     */
    void clearTabs() { mAttributeEditorElements.clear(); }

    /** Get the active layout style for the attribute editor for this layer */
    EditorLayout layout() const { return mEditorLayout; }

    /** Set the active layout style for the attribute editor for this layer */
    void setLayout( EditorLayout editorLayout ) { mEditorLayout = editorLayout; }

    /** Get path to the .ui form. Only meaningful with EditorLayout::UiFileLayout. */
    QString uiForm() const { return mUiFormPath; }

    /**
     * Set path to the .ui form.
     * When a string is provided, the layout style will be set to EditorLayout::UiFileLayout,
     * if an empty or a null string is provided, the layout style will be set to
     * EditorLayout::GeneratedLayout.
     */
    void setUiForm( const QString& ui );


    // Widget stuff

    /**
     * Set the editor widget type for a field
     *
     * QGIS ships the following widget types, additional types may be available depending
     * on plugins.
     *
     * <ul>
     * <li>CheckBox (QgsCheckboxWidgetWrapper)</li>
     * <li>Classification (QgsClassificationWidgetWrapper)</li>
     * <li>Color (QgsColorWidgetWrapper)</li>
     * <li>DateTime (QgsDateTimeEditWrapper)</li>
     * <li>Enumeration (QgsEnumerationWidgetWrapper)</li>
     * <li>FileName (QgsFileNameWidgetWrapper)</li>
     * <li>Hidden (QgsHiddenWidgetWrapper)</li>
     * <li>Photo (QgsPhotoWidgetWrapper)</li>
     * <li>Range (QgsRangeWidgetWrapper)</li>
     * <li>RelationReference (QgsRelationReferenceWidgetWrapper)</li>
     * <li>TextEdit (QgsTextEditWrapper)</li>
     * <li>UniqueValues (QgsUniqueValuesWidgetWrapper)</li>
     * <li>UuidGenerator (QgsUuidWidgetWrapper)</li>
     * <li>ValueMap (QgsValueMapWidgetWrapper)</li>
     * <li>ValueRelation (QgsValueRelationWidgetWrapper)</li>
     * <li>WebView (QgsWebViewWidgetWrapper)</li>
     * </ul>
     *
     * @param fieldIdx    Index of the field
     * @param widgetType  Type id of the editor widget to use
     */
    void setWidgetType( int fieldIdx, const QString& widgetType );

    /**
     * Get the id for the editor widget used to represent the field at the given index
     *
     * @param fieldIdx  The index of the field
     *
     * @return The id for the editor widget or a NULL string if not applicable
     */
    QString widgetType( int fieldIdx ) const;

    /**
     * Get the id for the editor widget used to represent the field at the given index
     *
     * @param fieldName  The name of the field
     *
     * @return The id for the editor widget or a NULL string if not applicable
     */
    QString widgetType( const QString& fieldName ) const;

    /**
     * Set the editor widget config for a field.
     *
     * Python: Will accept a map.
     *
     * Example:
     * \code{.py}
     *   layer.setWidgetConfig( 1, { 'Layer': 'otherlayerid_1234', 'Key': 'Keyfield', 'Value': 'ValueField' } )
     * \endcode
     *
     * @param attrIdx     Index of the field
     * @param config      The config to set for this field
     *
     * @see setWidgetType() for a list of widgets and choose the widget to see the available options.
     */
    void setWidgetConfig( int attrIdx, const QgsEditorWidgetConfig& config );

    /**
     * Set the editor widget config for a widget.
     *
     * Example:
     * \code{.py}
     *   layer.setWidgetConfig( 'relation_id', { 'nm-rel': 'other_relation' } )
     * \endcode
     *
     * @param widgetName  The name of the widget or field to configure
     * @param config      The config to set for this field
     *
     * @see setWidgetType() for a list of widgets and choose the widget to see the available options.
     *
     * @note not available in python bindings
     */
    void setWidgetConfig( const QString& widgetName, const QgsEditorWidgetConfig& config );

    /**
     * Get the configuration for the editor widget used to represent the field at the given index
     *
     * @param fieldIdx  The index of the field
     *
     * @return The configuration for the editor widget or an empty config if the field does not exist
     */
    QgsEditorWidgetConfig widgetConfig( int fieldIdx ) const;

    /**
     * Get the configuration for the editor widget used to represent the field with the given name
     *
     * @param widgetName The name of the widget. This can be a field name or the name of an additional widget.
     *
     * @return The configuration for the editor widget or an empty config if the field does not exist
     */
    QgsEditorWidgetConfig widgetConfig( const QString& widgetName ) const;

    /**
    * Remove the configuration for the editor widget used to represent the field at the given index
    *
    * @param fieldIdx  The index of the field
    *
    * @return true if successful, false if the field does not exist
    */
    bool removeWidgetConfig( int fieldIdx );

    /**
     * Remove the configuration for the editor widget used to represent the field with the given name
     *
     * @param widgetName The name of the widget. This can be a field name or the name of an additional widget.
     *
     * @return true if successful, false if the field does not exist
     */
    bool removeWidgetConfig( const QString& widgetName );

    /**
     * This returns true if the field is manually set to read only or if the field
     * does not support editing like joins or virtual fields.
     */
    bool readOnly( int idx ) const;

    /**
     * If set to false, the widget at the given index will be read-only.
     */
    void setReadOnly( int idx, bool readOnly = true );

    /**
     * Returns the constraint expression of a specific field
     * @param idx The index of the field
     * @return the expression
     * @note added in QGIS 2.16
     */
    QString expression( int idx ) const;

    /**
     * Set the constraint expression for a specific field
     * @param idx the field index
     * @param str the constraint expression
     * @note added in QGIS 2.16
     */
    void setExpression( int idx, const QString& str );

    /**
     * Returns the constraint expression description of a specific filed.
     * @param idx The index of the field
     * @return the expression description
     * @note added in QGIS 2.16
     */
    QString expressionDescription( int idx ) const;

    /**
     * Set the constraint expression description for a specific field.
     * @param idx The index of the field
     * @param descr The description of the expression
     * @note added in QGIS 2.16
     */
    void setExpressionDescription( int idx, const QString &descr );

    /**
     * Returns if the field at fieldidx should be treated as NOT NULL value
     */
    bool notNull( int fieldidx ) const;
    /**
     * Set if the field at fieldidx should be treated as NOT NULL value
     */
    void setNotNull( int idx, bool notnull = true );

    /**
     * If this returns true, the widget at the given index will receive its label on the previous line
     * while if it returns false, the widget will receive its label on the left hand side.
     * Labeling on top leaves more horizontal space for the widget itself.
     **/
    bool labelOnTop( int idx ) const;

    /**
     * If this is set to true, the widget at the given index will receive its label on
     * the previous line while if it is set to false, the widget will receive its label
     * on the left hand side.
     * Labeling on top leaves more horizontal space for the widget itself.
     **/
    void setLabelOnTop( int idx, bool onTop );


    // Python form init function stuff

    /**
     * Get python function for edit form initialization.
     * Will be looked up in a python file relative to the project folder if it
     * includes a module name or if it's a pure function name it will searched
     * in the python code set with @link setInitCode @endlink.
     */
    QString initFunction() const { return mInitFunction; }

    /**
     * Set python function for edit form initialization.
     * Will be looked up in a python file relative to the project folder if it
     * includes a module name or if it's a pure function name it will searched
     * in the python code set with @link setInitCode @endlink.
     */
    void setInitFunction( const QString& function ) { mInitFunction = function; }

    /**
     * Get python code for edit form initialization.
     */
    QString initCode() const { return mInitCode; }

    /**
     * Set python code for edit form initialization.
     * Make sure that you also set the appropriate function name in
     * @link setInitFunction @endlink
     */
    void setInitCode( const QString& code ) { mInitCode = code; }

    /**
     * Get python external file path for edit form initialization.
     */
    QString initFilePath() const { return mInitFilePath; }

    /**
     * Set python external file path for edit form initialization.
     * Make sure that you also set the appropriate function name in
     * @link setInitFunction @endlink
     */
    void setInitFilePath( const QString& filePath ) { mInitFilePath = filePath; }

    /** Return python code source for edit form initialization
     *  (if it shall be loaded from a file, read from the
     *  provided dialog editor or inherited from the environment)
     */
    PythonInitCodeSource initCodeSource() const { return mInitCodeSource; }

    /** Set if python code shall be used for edit form initialization and its origin */
    void setInitCodeSource( const PythonInitCodeSource initCodeSource ) { mInitCodeSource = initCodeSource; }

    /** Type of feature form pop-up suppression after feature creation (overrides app setting) */
    FeatureFormSuppress suppress() const { return mSuppressForm; }
    /** Set type of feature form pop-up suppression after feature creation (overrides app setting) */
    void setSuppress( FeatureFormSuppress s ) { mSuppressForm = s; }

    // Serialization

    /**
     * Read XML information
     * Deserialize on project load
     */
    void readXml( const QDomNode& node );

    /**
     * Write XML information
     * Serialize on project save
     */
    void writeXml( QDomNode& node ) const;

    /**
     * Deserialize drag and drop designer elements.
     */
    QgsAttributeEditorElement* attributeEditorElementFromDomElement( QDomElement &elem, QObject* parent );

  private slots:
    void onRelationsLoaded();

  protected:
    // Internal stuff

    /**
     * Create a new edit form config. Normally invoked by QgsVectorLayer
     */
    explicit QgsEditFormConfig( QObject* parent = nullptr );

  private:

    /**
     * Used internally to set the fields when they change.
     * This should only be called from QgsVectorLayer for synchronization reasons
     *
     * @param fields The fields
     */
    void setFields( const QgsFields& fields );

  private:
    /** Stores a list of attribute editor elements (Each holding a tree structure for a tab in the attribute editor)*/
    QList< QgsAttributeEditorElement* > mAttributeEditorElements;

    /** Map that stores the tab for attributes in the edit form. Key is the tab order and value the tab name*/
    QList< TabData > mTabs;

    QMap< QString, QString> mConstraints;
    QMap< QString, QString> mConstraintsDescription;
    QMap< QString, bool> mFieldEditables;
    QMap< QString, bool> mLabelOnTop;
    QMap< QString, bool> mNotNull;

    QMap<QString, QString> mEditorWidgetV2Types;
    QMap<QString, QgsEditorWidgetConfig > mWidgetConfigs;

    /** Defines the default layout to use for the attribute editor (Drag and drop, UI File, Generated) */
    EditorLayout mEditorLayout;

    /** Init form instance */
    QString mUiFormPath;
    /** Name of the python form init function */
    QString mInitFunction;
    /** Path of the python external file to be loaded */
    QString mInitFilePath;
    /** Choose the source of the init founction */
    PythonInitCodeSource mInitCodeSource;
    /** Python init code provided in the dialog */
    QString mInitCode;

    /** Type of feature form suppression after feature creation */
    FeatureFormSuppress mSuppressForm;

    QgsFields mFields;

    friend class QgsVectorLayer;
};

#endif // QGSEDITFORMCONFIG_H
