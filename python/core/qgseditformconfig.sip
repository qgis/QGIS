/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/qgseditformconfig.h                                         *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/






class QgsEditFormConfig
{

%TypeHeaderCode
#include "qgseditformconfig.h"
%End
  public:

    enum EditorLayout
    {
      GeneratedLayout,
      TabLayout,
      UiFileLayout
    };

    struct GroupData
    {
      GroupData();
%Docstring
Constructor for GroupData
%End
      GroupData( const QString &name, const QList<QString> &fields );
      QString mName;
      QList<QString> mFields;
    };

    struct TabData
    {
      TabData();
%Docstring
Constructor for TabData
%End
      TabData( const QString &name, const QList<QString> &fields, const QList<QgsEditFormConfig::GroupData> &groups );
      QString mName;
      QList<QString> mFields;
      QList<QgsEditFormConfig::GroupData> mGroups;
    };

    enum FeatureFormSuppress
    {
      SuppressDefault,
      SuppressOn,
      SuppressOff
    };

    enum PythonInitCodeSource
    {
      CodeSourceNone,
      CodeSourceFile,
      CodeSourceDialog,
      CodeSourceEnvironment
    };

    QgsEditFormConfig( const QgsEditFormConfig &o );
%Docstring
Copy constructor

.. versionadded:: 3.0
%End
    ~QgsEditFormConfig();


    bool operator==( const QgsEditFormConfig &o );

    void addTab( QgsAttributeEditorElement *data /Transfer/ );
%Docstring
Adds a new element to the invisible root container in the layout.

This is only useful in combination with EditorLayout.TabLayout.
%End

    QList< QgsAttributeEditorElement * > tabs() const;
%Docstring
Returns a list of tabs for EditorLayout.TabLayout obtained from the invisible root container.
%End

    void clearTabs();
%Docstring
Clears all the tabs for the attribute editor form with EditorLayout.TabLayout.
%End

    QgsAttributeEditorContainer *invisibleRootContainer();
%Docstring
Get the invisible root container for the drag and drop designer form (EditorLayout.TabLayout).

.. versionadded:: 3
%End

    EditorLayout layout() const;
%Docstring
Get the active layout style for the attribute editor for this layer
%End

    void setLayout( EditorLayout editorLayout );
%Docstring
Set the active layout style for the attribute editor for this layer
%End

    QString uiForm() const;
%Docstring
Get path to the .ui form. Only meaningful with EditorLayout.UiFileLayout.
%End

    void setUiForm( const QString &ui );
%Docstring
Set path to the .ui form.
When a string is provided, the layout style will be set to EditorLayout.UiFileLayout,
if an empty or a null string is provided, the layout style will be set to
EditorLayout.GeneratedLayout.
%End

    bool setWidgetConfig( const QString &widgetName, const QVariantMap &config );
%Docstring
Set the editor widget config for a widget which is not for a simple field.

Example:
.. code-block:: python

       editFormConfig = layer.editFormConfig()
       editFormConfig.setWidgetConfig( 'relation_id', { 'nm-rel': 'other_relation' } )
       layer.setEditFormConfig(editFormConfig)

:param widgetName:  The name of the widget to configure
:param config:      The config to set for this widget

:return: false if a field exists with the provided widgetName. In this case
QgsVectorLayer.setEditorWidgetSetup should be used.

.. seealso:: :py:func:`QgsVectorLayer.setEditorWidgetSetup`
%End

    QVariantMap widgetConfig( const QString &widgetName ) const;
%Docstring
Get the configuration for the editor widget with the given name.

:param widgetName: The name of the widget.

:return: The configuration for the editor widget or an empty config if the field does not exist
%End

    bool removeWidgetConfig( const QString &widgetName );
%Docstring
Remove the configuration for the editor widget with the given name

:param widgetName: The name of the widget.

:return: true if a configuration has been removed
%End

    bool readOnly( int idx ) const;
%Docstring
This returns true if the field is manually set to read only or if the field
does not support editing like joins or virtual fields.
%End

    void setReadOnly( int idx, bool readOnly = true );
%Docstring
If set to false, the widget at the given index will be read-only.
%End

    bool labelOnTop( int idx ) const;
%Docstring
If this returns true, the widget at the given index will receive its label on the previous line
while if it returns false, the widget will receive its label on the left hand side.
Labeling on top leaves more horizontal space for the widget itself.
*
%End

    void setLabelOnTop( int idx, bool onTop );
%Docstring
If this is set to true, the widget at the given index will receive its label on
the previous line while if it is set to false, the widget will receive its label
on the left hand side.
Labeling on top leaves more horizontal space for the widget itself.
*
%End



    QString initFunction() const;
%Docstring
Get Python function for edit form initialization.
Will be looked up in a Python file relative to the project folder if it
includes a module name or if it's a pure function name it will searched
in the Python code set with setInitCode().
%End

    void setInitFunction( const QString &function );
%Docstring
Set Python function for edit form initialization.
Will be looked up in a Python file relative to the project folder if it
includes a module name or if it's a pure function name it will searched
in the Python code set with setInitCode().
%End

    QString initCode() const;
%Docstring
Get Python code for edit form initialization.
%End

    void setInitCode( const QString &code );
%Docstring
Set Python code for edit form initialization.
Make sure that you also set the appropriate function name in
setInitFunction().
%End

    QString initFilePath() const;
%Docstring
Get Python external file path for edit form initialization.
%End

    void setInitFilePath( const QString &filePath );
%Docstring
Set Python external file path for edit form initialization.
Make sure that you also set the appropriate function name in
setInitFunction().
%End

    PythonInitCodeSource initCodeSource() const;
%Docstring
Return Python code source for edit form initialization
(if it shall be loaded from a file, read from the
provided dialog editor or inherited from the environment)
%End

    void setInitCodeSource( PythonInitCodeSource initCodeSource );
%Docstring
Set if Python code shall be used for edit form initialization and its origin
%End

    FeatureFormSuppress suppress() const;
%Docstring
Type of feature form pop-up suppression after feature creation (overrides app setting)
%End
    void setSuppress( FeatureFormSuppress s );
%Docstring
Set type of feature form pop-up suppression after feature creation (overrides app setting)
%End


    void readXml( const QDomNode &node, const QgsReadWriteContext &context );
%Docstring
Read XML information
Deserialize on project load
%End

    void writeXml( QDomNode &node, const QgsReadWriteContext &context ) const;
%Docstring
Write XML information
Serialize on project save
%End

    QgsAttributeEditorElement *attributeEditorElementFromDomElement( QDomElement &elem, QgsAttributeEditorElement *parent );
%Docstring
Deserialize drag and drop designer elements.
%End

    explicit QgsEditFormConfig();
%Docstring
Create a new edit form config. Normally invoked by :py:class:`QgsVectorLayer`
%End

};

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/qgseditformconfig.h                                         *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
