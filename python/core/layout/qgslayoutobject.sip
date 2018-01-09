/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/layout/qgslayoutobject.h                                    *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/




class QgsLayoutObject: QObject, QgsExpressionContextGenerator
{
%Docstring
A base class for objects which belong to a layout.

.. versionadded:: 3.0
%End

%TypeHeaderCode
#include "qgslayoutobject.h"
%End
  public:

    enum DataDefinedProperty
    {
      NoProperty,
      AllProperties,
      TestProperty,
      //composer
      PresetPaperSize,
      PaperWidth,
      PaperHeight,
      NumPages,
      PaperOrientation,
      //general
      PageNumber,
      PositionX,
      PositionY,
      ItemWidth,
      ItemHeight,
      ItemRotation,
      Transparency,
      Opacity,
      BlendMode,
      ExcludeFromExports,
      FrameColor,
      BackgroundColor,
      //composer
      MapRotation,
      MapScale,
      MapXMin,
      MapYMin,
      MapXMax,
      MapYMax,
      MapAtlasMargin,
      MapLayers,
      MapStylePreset,
      //composer
      PictureSource,
      PictureSvgBackgroundColor,
      PictureSvgStrokeColor,
      PictureSvgStrokeWidth,
      //html
      SourceUrl,
      //legend
      LegendTitle,
      LegendColumnCount,
      //scalebar
      ScalebarFillColor,
      ScalebarFillColor2,
      ScalebarLineColor,
      ScalebarLineWidth,
    };

    enum PropertyValueType
    {
      EvaluatedValue,
      OriginalValue
    };

    static const QgsPropertiesDefinition &propertyDefinitions();
%Docstring
Returns the layout object property definitions.
%End

    explicit QgsLayoutObject( QgsLayout *layout );
%Docstring
Constructor for QgsLayoutObject, with the specified parent ``layout``.

.. note::

   While ownership of a QgsLayoutObject is not passed to the layout,
classes which are derived from QgsLayoutObject (such as QgsLayoutItem)
may transfer their ownership to a layout upon construction.
%End


    QgsLayout *layout();
%Docstring
Returns the layout the object is attached to.
%End

    QgsPropertyCollection &dataDefinedProperties();
%Docstring
Returns a reference to the object's property collection, used for data defined overrides.

.. seealso:: :py:func:`setDataDefinedProperties`
%End


    void setDataDefinedProperties( const QgsPropertyCollection &collection );
%Docstring
Sets the objects's property collection, used for data defined overrides.

:param collection: property collection. Existing properties will be replaced.

.. seealso:: :py:func:`dataDefinedProperties`
%End


    void setCustomProperty( const QString &key, const QVariant &value );
%Docstring
Set a custom property for the object.

:param key: property key. If a property with the same key already exists it will be overwritten.
:param value: property value

.. seealso:: :py:func:`customProperty`

.. seealso:: :py:func:`removeCustomProperty`

.. seealso:: :py:func:`customProperties`
%End

    QVariant customProperty( const QString &key, const QVariant &defaultValue = QVariant() ) const;
%Docstring
Read a custom property from the object.

:param key: property key
:param defaultValue: default value to return if property with matching key does not exist

:return: value of matching property

.. seealso:: :py:func:`setCustomProperty`

.. seealso:: :py:func:`removeCustomProperty`

.. seealso:: :py:func:`customProperties`
%End

    void removeCustomProperty( const QString &key );
%Docstring
Remove a custom property from the object.

:param key: property key

.. seealso:: :py:func:`setCustomProperty`

.. seealso:: :py:func:`customProperty`

.. seealso:: :py:func:`customProperties`
%End

    QStringList customProperties() const;
%Docstring
Return list of keys stored in custom properties for the object.

.. seealso:: :py:func:`setCustomProperty`

.. seealso:: :py:func:`customProperty`

.. seealso:: :py:func:`removeCustomProperty`
%End

    virtual QgsExpressionContext createExpressionContext() const;

%Docstring
Creates an expression context relating to the objects' current state. The context includes
scopes for global, project and layout properties.
%End

  public slots:

    virtual void refresh();
%Docstring
Refreshes the object, causing a recalculation of any property overrides.
%End

  signals:

    void changed();
%Docstring
Emitted when the object's properties change.
%End

  protected:

    bool writeObjectPropertiesToElement( QDomElement &parentElement, QDomDocument &document, const QgsReadWriteContext &context ) const;
%Docstring
Stores object properties within an XML DOM element.

:param parentElement: is the parent DOM element to store the object's properties in
:param document: DOM document
:param context: read write context

:return: true if write was successful

.. seealso:: :py:func:`readObjectPropertiesFromElement`
%End

    bool readObjectPropertiesFromElement( const QDomElement &parentElement, const QDomDocument &document, const QgsReadWriteContext &context );
%Docstring
Sets object properties from a DOM element

:param parentElement: is the parent DOM element for the object
:param document: DOM document
:param context: read write context

:return: true if read was successful

.. seealso:: :py:func:`writeObjectPropertiesToElement`
%End




};

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/layout/qgslayoutobject.h                                    *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
