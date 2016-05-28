/***************************************************************************
                          qgseditformconfig.sip
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

class QgsAttributeEditorElement : QObject
{
%TypeHeaderCode
#include "qgsvectorlayer.h"
%End

%ConvertToSubClassCode
  QgsAttributeEditorElement* e = qobject_cast<QgsAttributeEditorElement*>( sipCpp );

  sipType = 0;
  if ( e )
  {
    switch ( e->type() )
    {
      case QgsAttributeEditorElement::AeTypeContainer: sipType = sipType_QgsAttributeEditorContainer; break;
      case QgsAttributeEditorElement::AeTypeField:     sipType = sipType_QgsAttributeEditorField; break;
      case QgsAttributeEditorElement::AeTypeRelation:  sipType = sipType_QgsAttributeEditorRelation; break;
    }
  }
%End


  public:

    enum AttributeEditorType
    {
      AeTypeContainer,
      AeTypeField,
      AeTypeRelation,
      AeTypeInvalid
    };

    /**
     * Constructor
     *
     * @param type The type of the new element. Should never
     * @param name
     * @param parent
     */
    QgsAttributeEditorElement( AttributeEditorType type, const QString& name, QObject *parent /TransferThis/ = NULL );

    //! Destructor
    virtual ~QgsAttributeEditorElement();

    /**
     * Return the name of this element
     *
     * @return The name for this element
     */
    QString name() const;

    /**
     * The type of this element
     *
     * @return The type
     */
    AttributeEditorType type() const;

    /**
     * Is reimplemented in classes inheriting from this to serialize it.
     *
     * @param doc The QDomDocument which is used to create new XML elements
     *
     * @return An DOM element which represents this element
     */
    virtual QDomElement toDomElement( QDomDocument& doc ) const = 0;
};

class QgsAttributeEditorContainer : QgsAttributeEditorElement
{
%TypeHeaderCode
#include "qgsvectorlayer.h"
%End

  public:
    /**
     * Creates a new attribute editor container
     *
     * @param name   The name to show as title
     * @param parent The parent. May be another container.
     */
    QgsAttributeEditorContainer( const QString& name, QObject *parent  /TransferThis/ );

    //! Destructor
    ~QgsAttributeEditorContainer();

    /**
     * Will serialize this containers information into a QDomElement for saving it in an XML file.
     *
     * @param doc The QDomDocument used to generate the QDomElement
     *
     * @return The XML element
     */
    virtual QDomElement toDomElement( QDomDocument& doc ) const;

    /**
     * Add a child element to this container. This may be another container, a field or a relation.
     *
     * @param element The element to add as child
     */
    virtual void addChildElement( QgsAttributeEditorElement *widget );

    /**
     * Determines if this container is rendered as collapsible group box or tab in a tabwidget
     *
     * @param isGroupBox If true, this will be a group box
     */
    virtual void setIsGroupBox( bool isGroupBox );

    /**
     * Returns if this container is going to be rendered as a group box
     *
     * @return True if it will be a group box, false if it will be a tab
     */
    virtual bool isGroupBox() const;

    /**
     * Get a list of the children elements of this container
     *
     * @return A list of elements
     */
    QList<QgsAttributeEditorElement*> children() const;

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
     *
     * @param name
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

};

/**
 * This element will load a field's widget onto the form.
 */
class QgsAttributeEditorField : QgsAttributeEditorElement
{
%TypeHeaderCode
#include "qgsvectorlayer.h"
%End
  public:
    /**
     * Creates a new attribute editor element which represents a field
     *
     * @param name   The name of the element
     * @param idx    The index of the field which should be embedded
     * @param parent The parent of this widget (used as container)
     */
    QgsAttributeEditorField( const QString& name, int idx, QObject *parent /TransferThis/ );

    //! Destructor
    ~QgsAttributeEditorField();

    /**
     * Will serialize this elements information into a QDomElement for saving it in an XML file.
     *
     * @param doc The QDomDocument used to generate the QDomElement
     *
     * @return The XML element
     */
    virtual QDomElement toDomElement( QDomDocument& doc ) const;

    /**
     * Return the index of the field
     * @return
     */
    int idx() const;
};

/**
 * This element will load a relation editor onto the form.
 *
 * @note Added in 2.1
 */
class QgsAttributeEditorRelation : QgsAttributeEditorElement
{
  public:
    /**
     * Creates a new element which embeds a relation.
     *
     * @param name         The name of this element
     * @param relationId   The id of the relation to embed
     * @param parent       The parent (used as container)
     */
    QgsAttributeEditorRelation( const QString& name, const QString &relationId, QObject *parent /TransferThis/ );

    /**
     * Creates a new element which embeds a relation.
     *
     * @param name         The name of this element
     * @param relation     The relation to embed
     * @param parent       The parent (used as container)
     */
    QgsAttributeEditorRelation( const QString& name, const QgsRelation& relation, QObject *parent /TransferThis/);

    //! Destructor
    ~QgsAttributeEditorRelation();

    /**
     * Will serialize this elements information into a QDomElement for saving it in an XML file.
     *
     * @param doc The QDomDocument used to generate the QDomElement
     *
     * @return The XML element
     */
    virtual QDomElement toDomElement( QDomDocument& doc ) const;

    /**
     * Get the id of the relation which shall be embedded
     *
     * @return the id
     */
    const QgsRelation& relation() const;

    /**
     * Initializes the relation from the id
     *
     * @param relManager The relation manager to use for the initialization
     * @return true if the relation was found in the relationmanager
     */
    bool init( QgsRelationManager *relManager );
};

class QgsEditFormConfig : QObject
{
%TypeHeaderCode
#include <qgseditformconfig.h>
%End

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
      GroupData();
      GroupData( const QString& name, const QList<QString>& fields );
      QString mName;
      QList<QString> mFields;
    };

    struct TabData
    {
      TabData();
      TabData( const QString& name, const QList<QString>& fields, const QList<QgsEditFormConfig::GroupData>& groups );
      QString mName;
      QList<QString> mFields;
      QList<QgsEditFormConfig::GroupData> mGroups;
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
      CodeSourceNone,             //!< Do not use python code at all
      CodeSourceFile,             //!< Load the python code from an external file
      CodeSourceDialog,           //!< Use the python code provided in the dialog
      CodeSourceEnvironment       //!< Use the python code available in the python environment
    };

    /**
     * This is only useful in combination with EditorLayout::TabLayout.
     *
     * Adds a new tab to the layout. Should be a QgsAttributeEditorContainer.
     */
    void addTab( QgsAttributeEditorElement* data );

    /**
     * Returns a list of tabs for EditorLayout::TabLayout.
     */
    QList< QgsAttributeEditorElement* > tabs();

    /**
     * Clears all the tabs for the attribute editor form with EditorLayout::TabLayout.
     */
    void clearTabs();

    /** Get the active layout style for the attribute editor for this layer */
    EditorLayout layout();

    /** Set the active layout style for the attribute editor for this layer */
    void setLayout( EditorLayout editorLayout );

    /** Get path to the .ui form. Only meaningful with EditorLayout::UiFileLayout. */
    QString uiForm() const;

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
    // void setWidgetConfig( const QString& widgetName, const QgsEditorWidgetConfig& config );

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
    bool readOnly( int idx );

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
    bool notNull( int fieldidx) const;
    /**
     * Set if the field at fieldidx should be treated as NOT NULL value
     */
    void setNotNull( int idx, bool notnull = true );

    /**
     * If this returns true, the widget at the given index will receive its label on the previous line
     * while if it returns false, the widget will receive its label on the left hand side.
     * Labeling on top leaves more horizontal space for the widget itself.
     **/
    bool labelOnTop( int idx );

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
    QString initFunction() const;

    /**
     * Set python function for edit form initialization.
     * Will be looked up in a python file relative to the project folder if it
     * includes a module name or if it's a pure function name it will searched
     * in the python code set with @link setInitCode @endlink.
     */
    void setInitFunction( const QString& function );

    /**
     * Get python code for edit form initialization from the configuration dialog.
     */
    QString initCode() const;

    /**
     * Set python code for edit form initialization.
     * Make sure that you also set the appropriate function name in
     * @link setInitFunction @endlink
     */
    void setInitCode( const QString& code );

    /**
     * Get python external file path for edit form initialization.
     */
    QString initFilePath() const;

    /**
     * Set python external file path for edit form initialization.
     * Make sure that you also set the appropriate function name in
     * @link setInitFunction @endlink
     */
    void setInitFilePath( const QString& filePath );

    /** Return python code source for edit form initialization
     *  (if it shall be loaded from a file, read from the
     *  provided dialog editor or inherited from the environment)
     */
    PythonInitCodeSource initCodeSource() const;

    /** Set if python code shall be used for edit form initialization */
    void setInitCodeSource( const PythonInitCodeSource initCodeSource );

    /** Type of feature form pop-up suppression after feature creation (overrides app setting) */
    FeatureFormSuppress suppress() const;
    /** Set type of feature form pop-up suppression after feature creation (overrides app setting) */
    void setSuppress( QgsEditFormConfig::FeatureFormSuppress s );

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
};
