/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/composer/qgscomposerobject.h                                *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/





class QgsComposerObject: QObject, QgsExpressionContextGenerator
{
%Docstring
 A base class for objects which belong to a map composition.
%End

%TypeHeaderCode
#include "qgscomposerobject.h"
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
 Returns the composer object property definitions.
.. versionadded:: 3.0
 :rtype: QgsPropertiesDefinition
%End

    QgsComposerObject( QgsComposition *composition );
%Docstring
 Constructor
 \param composition parent composition
%End
    virtual ~QgsComposerObject();

    const QgsComposition *composition() const;
%Docstring
 Returns the composition the item is attached to.
 :return: QgsComposition for item.
 :rtype: QgsComposition
%End


    virtual bool writeXml( QDomElement &elem, QDomDocument &doc ) const;
%Docstring
 Stores item state in DOM element
 \param elem is DOM element corresponding to item tag
 \param doc is the DOM document
 :rtype: bool
%End

    virtual bool readXml( const QDomElement &itemElem, const QDomDocument &doc );
%Docstring
 Sets item state from DOM element
 \param itemElem is DOM node corresponding to item tag
 \param doc is DOM document
 :rtype: bool
%End

    QgsPropertyCollection &dataDefinedProperties();
%Docstring
 Returns a reference to the object's property collection, used for data defined overrides.
.. versionadded:: 3.0
.. seealso:: setDataDefinedProperties()
 :rtype: QgsPropertyCollection
%End


    void setDataDefinedProperties( const QgsPropertyCollection &collection );
%Docstring
 Sets the objects's property collection, used for data defined overrides.
 \param collection property collection. Existing properties will be replaced.
.. versionadded:: 3.0
.. seealso:: dataDefinedProperties()
%End

    void setCustomProperty( const QString &key, const QVariant &value );
%Docstring
 Set a custom property for the object.
 \param key property key. If a property with the same key already exists it will be overwritten.
 \param value property value
.. seealso:: customProperty()
.. seealso:: removeCustomProperty()
.. seealso:: customProperties()
.. versionadded:: 2.12
%End

    QVariant customProperty( const QString &key, const QVariant &defaultValue = QVariant() ) const;
%Docstring
 Read a custom property from the object.
 \param key property key
 \param defaultValue default value to return if property with matching key does not exist
 :return: value of matching property
.. seealso:: setCustomProperty()
.. seealso:: removeCustomProperty()
.. seealso:: customProperties()
.. versionadded:: 2.12
 :rtype: QVariant
%End

    void removeCustomProperty( const QString &key );
%Docstring
 Remove a custom property from the object.
 \param key property key
.. seealso:: setCustomProperty()
.. seealso:: customProperty()
.. seealso:: customProperties()
.. versionadded:: 2.12
%End

    QStringList customProperties() const;
%Docstring
 Return list of keys stored in custom properties for the object.
.. seealso:: setCustomProperty()
.. seealso:: customProperty()
.. seealso:: removeCustomProperty()
.. versionadded:: 2.12
 :rtype: list of str
%End

    virtual QgsExpressionContext createExpressionContext() const;
%Docstring
 Creates an expression context relating to the objects' current state. The context includes
 scopes for global, project and composition properties.
.. versionadded:: 2.12
 :rtype: QgsExpressionContext
%End

  public slots:

    virtual void repaint();
%Docstring
Triggers a redraw for the item
%End

    virtual void refreshDataDefinedProperty( const DataDefinedProperty property = AllProperties, const QgsExpressionContext *context = 0 );
%Docstring
 Refreshes a data defined property for the item by reevaluating the property's value
 and redrawing the item with this new value.
 \param property data defined property to refresh. If property is set to
 QgsComposerItem.AllProperties then all data defined properties for the item will be
 refreshed.
 \param context expression context for evaluating data defined expressions
.. versionadded:: 2.5
%End

  protected:




  signals:

    void itemChanged();
%Docstring
 Emitted when the item changes. Signifies that the item widgets must update the
 gui elements.
%End

};

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/composer/qgscomposerobject.h                                *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
