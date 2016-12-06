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
#include <QDomElement>
#include <QDomDocument>

#include "qgsattributeeditorelement.h"

class QgsRelationManager;
class QgsEditFormConfigPrivate;

/** \ingroup core
 * \class QgsEditFormConfig
 */
class CORE_EXPORT QgsEditFormConfig
{
  public:

    //! The different types to layout the attribute editor.
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
     * Copy constructor
     *
     * @note Added in QGIS 3.0
     */
    QgsEditFormConfig( const QgsEditFormConfig& o );

    QgsEditFormConfig& operator=( const QgsEditFormConfig& o );

    bool operator==( const QgsEditFormConfig& o );

    ~QgsEditFormConfig();

    /**
     * Adds a new element to the invisible root container in the layout.
     *
     * This is only useful in combination with EditorLayout::TabLayout.
     */
    void addTab( QgsAttributeEditorElement* data );

    /**
     * Returns a list of tabs for EditorLayout::TabLayout obtained from the invisible root container.
     */
    QList< QgsAttributeEditorElement* > tabs() const;

    /**
     * Clears all the tabs for the attribute editor form with EditorLayout::TabLayout.
     */
    void clearTabs();

    /**
     * Get the invisible root container for the drag and drop designer form (EditorLayout::TabLayout).
     *
     * @note Added in QGIS 3
     */
    QgsAttributeEditorContainer* invisibleRootContainer();

    //! Get the active layout style for the attribute editor for this layer
    EditorLayout layout() const;

    //! Set the active layout style for the attribute editor for this layer
    void setLayout( EditorLayout editorLayout );

    //! Get path to the .ui form. Only meaningful with EditorLayout::UiFileLayout.
    QString uiForm() const;

    /**
     * Set path to the .ui form.
     * When a string is provided, the layout style will be set to EditorLayout::UiFileLayout,
     * if an empty or a null string is provided, the layout style will be set to
     * EditorLayout::GeneratedLayout.
     */
    void setUiForm( const QString& ui );

    /**
     * Set the editor widget config for a widget which is not for a simple field.
     *
     * Example:
     * \code{.py}
     *   editFormConfig = layer.editFormConfig()
     *   editFormConfig.setWidgetConfig( 'relation_id', { 'nm-rel': 'other_relation' } )
     *   layer.setEditFormConfig(editFormConfig)
     * \endcode
     *
     * @param widgetName  The name of the widget to configure
     * @param config      The config to set for this widget
     * @returns false if a field exists with the provided widgetName. In this case
     *          QgsVectorLayer::setEditorWidgetSetup should be used.
     *
     * @see QgsVectorLayer::setEditorWidgetSetup() for field configurations.
     */
    bool setWidgetConfig( const QString& widgetName, const QVariantMap& config );

    /**
     * Get the configuration for the editor widget with the given name.
     *
     * @param widgetName The name of the widget.
     *
     * @return The configuration for the editor widget or an empty config if the field does not exist
     */
    QVariantMap widgetConfig( const QString& fieldName ) const;

    /**
     * Remove the configuration for the editor widget with the given name
     *
     * @param widgetName The name of the widget.
     *
     * @return true if a configuration has been removed
     */
    bool removeWidgetConfig( const QString& fieldName );

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
    QString initFunction() const;

    /**
     * Set python function for edit form initialization.
     * Will be looked up in a python file relative to the project folder if it
     * includes a module name or if it's a pure function name it will searched
     * in the python code set with @link setInitCode @endlink.
     */
    void setInitFunction( const QString& function );

    /**
     * Get python code for edit form initialization.
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

    //! Set if python code shall be used for edit form initialization and its origin
    void setInitCodeSource( PythonInitCodeSource initCodeSource );

    //! Type of feature form pop-up suppression after feature creation (overrides app setting)
    FeatureFormSuppress suppress() const;
    //! Set type of feature form pop-up suppression after feature creation (overrides app setting)
    void setSuppress( FeatureFormSuppress s );

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
    QgsAttributeEditorElement* attributeEditorElementFromDomElement( QDomElement &elem, QgsAttributeEditorElement* parent );

    /**
     * Create a new edit form config. Normally invoked by QgsVectorLayer
     */
    explicit QgsEditFormConfig();

  private:

    /**
     * Used internally to set the fields when they change.
     * This should only be called from QgsVectorLayer for synchronization reasons
     */
    void setFields( const QgsFields& fields );

    /**
     * Will be called by friend class QgsVectorLayer
     */
    void onRelationsLoaded();

  private:
    QExplicitlySharedDataPointer<QgsEditFormConfigPrivate> d;

    friend class QgsVectorLayer;
};

#endif // QGSEDITFORMCONFIG_H
