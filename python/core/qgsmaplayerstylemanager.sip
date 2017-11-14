/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/qgsmaplayerstylemanager.h                                   *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/







class QgsMapLayerStyle
{
%Docstring
 Stores style information (renderer, opacity, labeling, diagrams etc.) applicable to a map layer.

 Stored data are considered as opaque - it is not possible to access them directly or modify them - it is
 only possible to read or write layer's current style.

.. versionadded:: 2.8
%End

%TypeHeaderCode
#include "qgsmaplayerstylemanager.h"
%End
  public:
    QgsMapLayerStyle();
%Docstring
construct invalid style
%End

    explicit QgsMapLayerStyle( const QString &xmlData );
%Docstring
construct style from QML definition (XML)
%End

    bool isValid() const;
%Docstring
Tell whether the style is valid (i.e. there is something stored in it)
 :rtype: bool
%End

    void clear();
%Docstring
Remove any stored style data (will get invalid)
%End

    QString xmlData() const;
%Docstring
Return XML content of the style
 :rtype: str
%End

    void readFromLayer( QgsMapLayer *layer );
%Docstring
Store layer's active style information in the instance
%End
    void writeToLayer( QgsMapLayer *layer ) const;
%Docstring
Apply stored layer's style information to the layer
%End

    void readXml( const QDomElement &styleElement );
%Docstring
Read style configuration (for project file reading)
%End
    void writeXml( QDomElement &styleElement ) const;
%Docstring
Write style configuration (for project file writing)
%End

};


class QgsMapLayerStyleManager : QObject
{
%Docstring
 Management of styles for use with one map layer. Stored styles are identified by their names. The manager
 always keep track of which style of the stored ones is currently active. When the current style is changed,
 the new style is applied to the associated layer.

 The class takes care of updating itself when the layer's current style configuration changes.
 When some of layer style's properties change (e.g. opacity / colors), the style manager will
 record them in the currently active style without any extra effort required.

 When an instance is created, it creates "default" style (with empty name) recorded from the associated map layer
 and it is set as the current style. The instance must always contain at least one style (which is the current).

 The style which is marked as current has no style data stored in its entry. This is to avoid duplication
 of data as all data is stored in map layer and can be retrieved easily. Also when saving, the entry for
 the current style will be saved with no data because everything is already saved by the map layer itself.

 The class also features support for temporary change of the layer's style, ideal for short-term use of a custom
 style followed by restoration of the original style (for example, when rendering a map with a different than current style).

.. versionadded:: 2.8
%End

%TypeHeaderCode
#include "qgsmaplayerstylemanager.h"
%End
  public:
    QgsMapLayerStyleManager( QgsMapLayer *layer );
%Docstring
Construct a style manager associated with a map layer (must not be null)
%End

    QgsMapLayer *layer() const;
%Docstring
Get pointer to the associated map layer
 :rtype: QgsMapLayer
%End

    void reset();
%Docstring
Reset the style manager to a basic state - with one default style which is set as current
%End

    void readXml( const QDomElement &mgrElement );
%Docstring
Read configuration (for project loading)
%End
    void writeXml( QDomElement &mgrElement ) const;
%Docstring
Write configuration (for project saving)
%End

    QStringList styles() const;
%Docstring
Return list of all defined style names
 :rtype: list of str
%End

    QMap<QString, QgsMapLayerStyle> mapLayerStyles() const;
%Docstring
 Gets available styles for the associated map layer.
 :return: A map of map layer style by style name
.. versionadded:: 3.0
 :rtype: QMap<str, QgsMapLayerStyle>
%End

    QgsMapLayerStyle style( const QString &name ) const;
%Docstring
Return data of a stored style - accessed by its unique name
 :rtype: QgsMapLayerStyle
%End

    bool addStyle( const QString &name, const QgsMapLayerStyle &style );
%Docstring
 Add a style with given name and data
 :return: true on success (name is unique and style is valid)
 :rtype: bool
%End

    bool addStyleFromLayer( const QString &name );
%Docstring
 Add style by cloning the current one
 :return: true on success
 :rtype: bool
%End

    bool removeStyle( const QString &name );
%Docstring
 Remove a stored style
 :return: true on success (style exists and it is not the last one)
 :rtype: bool
%End

    bool renameStyle( const QString &name, const QString &newName );
%Docstring
 Rename a stored style to a different name
 :return: true on success (style exists and new name is unique)
 :rtype: bool
%End

    QString currentStyle() const;
%Docstring
Return name of the current style
 :rtype: str
%End

    bool setCurrentStyle( const QString &name );
%Docstring
 Set a different style as the current style - will apply it to the layer
 :return: true on success
 :rtype: bool
%End

    bool setOverrideStyle( const QString &styleDef );
%Docstring
 Temporarily apply a different style to the layer. The argument
 can be either a style name or a full QML style definition.
 Each call must be paired with restoreOverrideStyle()
 :rtype: bool
%End
    bool restoreOverrideStyle();
%Docstring
Restore the original store after a call to setOverrideStyle()
 :rtype: bool
%End

  signals:
    void styleAdded( const QString &name );
%Docstring
Emitted when a new style has been added
%End
    void styleRemoved( const QString &name );
%Docstring
Emitted when a style has been removed
%End
    void styleRenamed( const QString &oldName, const QString &newName );
%Docstring
Emitted when a style has been renamed
%End
    void currentStyleChanged( const QString &currentName );
%Docstring
Emitted when the current style has been changed
%End

};

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/qgsmaplayerstylemanager.h                                   *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
