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
      CodeSourceNone = 0,             //!< Do not use python code at all
      CodeSourceFile = 1,             //!< Load the python code from a file
      CodeSourceDialog = 2,           //!< Use the python code provided in the dialog
      CodeSourceEnvironment = 3       //!< Use the python code available in the python environment
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
     *
     * @note python method name editorWidgetV2ByName
     */
    QString widgetType( const QString& fieldName ) const;

    /**
     * Set the editor widget config for a field.
     *
     * Python: Will accept a map.
     *
     * Example:
     * \code{.py}
     *   layer.setEditorWidgetV2Config( 1, { 'Layer': 'otherlayerid_1234', 'Key': 'Keyfield', 'Value': 'ValueField' } )
     * \endcode
     *
     * @param attrIdx     Index of the field
     * @param config      The config to set for this field
     *
     * @see setEditorWidgetV2() for a list of widgets and choose the widget to see the available options.
     */
    void setWidgetConfig( int attrIdx, const QgsEditorWidgetConfig& config );

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
     * @param fieldName The name of the field
     *
     * @return The configuration for the editor widget or an empty config if the field does not exist
     *
     * @note python method name is editorWidgetV2ConfigByName
     */
    QgsEditorWidgetConfig widgetConfig( const QString& fieldName ) const;

    /**
     * This returns true if the field is manually set to read only or if the field
     * does not support editing like joins or vitual fields.
     */
    bool readOnly( int idx );

    /**
     * If set to false, the widget at the given index will be read-only.
     */
    void setReadOnly(int idx, bool readOnly );

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


    // Python stuff


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
     * Get python external file path for edit form initialization.
     */
    QString initFilePath() const;

    /**
     * Set python external file path for edit form initialization.
     * Make sure that you also set the appropriate function name in
     * @link setInitFunction @endlink
     */
    void setInitFilePath( const QString& filePath );

    /**
     * Get python code for edit form initialization.
     * Make sure that you also set the appropriate function name in
     * @link setInitFunction @endlink
     */
    void setInitCode( const QString& code );

    /** Return python code source for edit form initialization
     *  (if it shall be loaded from a file, read from the
     *  provided dialog editor or just read from the environment
     */
    PythonInitCodeSource initCodeSource() const;

    /** Set if python code shall be used for edit form initialization */
    void setInitCodeSource( const PythonInitCodeSource initCodeSource );


    /** Type of feature form pop-up suppression after feature creation (overrides app setting) */
    FeatureFormSuppress suppress() const;
    /** Set type of feature form pop-up suppression after feature creation (overrides app setting) */
    void setSuppress( FeatureFormSuppress s );

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
