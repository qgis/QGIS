/** \ingroup core
 * A base class for objects which belong to a map composition.
 */
class QgsComposerObject : QObject, QgsExpressionContextGenerator
{
%TypeHeaderCode
#include <qgscomposerobject.h>
%End

  public:

    /** Data defined properties for different item types
     */
    enum DataDefinedProperty
    {
      NoProperty = 0, /*!< no property */
      AllProperties, /*!< all properties for item */
      TestProperty, /*!< dummy property with no effect on item*/
      //composer page properties
      PresetPaperSize, /*!< preset paper size for composition */
      PaperWidth, /*!< paper width */
      PaperHeight, /*!< paper height */
      NumPages, /*!< number of pages in composition */
      PaperOrientation, /*!< paper orientation */
      //general composer item properties
      PageNumber, /*!< page number for item placement */
      PositionX, /*!< x position on page */
      PositionY, /*!< y position on page */
      ItemWidth, /*!< width of item */
      ItemHeight, /*!< height of item */
      ItemRotation, /*!< rotation of item */
      Transparency, /*!< item transparency */
      BlendMode, /*!< item blend mode */
      ExcludeFromExports, /*!< exclude item from exports */
      FrameColor, //!< Item frame color
      BackgroundColor, //!< Item background color
      //composer map
      MapRotation, /*!< map rotation */
      MapScale, /*!< map scale */
      MapXMin, /*!< map extent x minimum */
      MapYMin, /*!< map extent y minimum */
      MapXMax, /*!< map extent x maximum */
      MapYMax, /*!< map extent y maximum */
      MapAtlasMargin, /*!< map atlas margin*/
      MapLayers, /*!< map layer set*/
      MapStylePreset, /*!< layer and style visibility preset */
      //composer picture
      PictureSource, /*!< picture source url */
      PictureSvgBackgroundColor, //!< SVG background color
      PictureSvgOutlineColor, //!< SVG outline color
      PictureSvgOutlineWidth, //!< SVG outline width
      //html item
      SourceUrl /*!< html source url */
    };

    /** Specifies whether the value returned by a function should be the original, user
     * set value, or the current evaluated value for the property. This may differ if
     * a property has a data defined expression active.
     */
    enum PropertyValueType
    {
      EvaluatedValue = 0, /*!< return the current evaluated value for the property */
      OriginalValue /*!< return the original, user set value */
    };

    /** Constructor
     * @param composition parent composition
     */
    QgsComposerObject( QgsComposition* composition );
    virtual ~QgsComposerObject();

    /** Returns the composition the item is attached to.
     * @returns QgsComposition for item.
     */
    const QgsComposition* composition() const;

    //! @note not available in python bindings
    // QgsComposition* composition() { return mComposition; }

    /** Stores item state in DOM element
     * @param elem is DOM element corresponding to item tag
     * @param doc is the DOM document
     */
    virtual bool writeXml( QDomElement& elem, QDomDocument & doc ) const;

    /** Sets item state from DOM element
     * @param itemElem is DOM node corresponding to item tag
     * @param doc is DOM document
     */
    virtual bool readXml( const QDomElement& itemElem, const QDomDocument& doc );

    /** Returns a reference to the object's property collection, used for data defined overrides.
     * @note added in QGIS 3.0
     * @see setDataDefinedProperties()
     */
    QgsPropertyCollection& dataDefinedProperties();

    /** Returns a reference to the object's property collection, used for data defined overrides.
     * @note added in QGIS 3.0
     * @see setDataDefinedProperties()
     */
    //const QgsPropertyCollection& dataDefinedProperties() const;

    /** Sets the objects's property collection, used for data defined overrides.
     * @param collection property collection. Existing properties will be replaced.
     * @note added in QGIS 3.0
     * @see dataDefinedProperties()
     */
    void setDataDefinedProperties( const QgsPropertyCollection& collection );

    /** Set a custom property for the object.
     * @param key property key. If a property with the same key already exists it will be overwritten.
     * @param value property value
     * @see customProperty()
     * @see removeCustomProperty()
     * @see customProperties()
     * @note added in QGIS 2.12
     */
    void setCustomProperty( const QString &key, const QVariant &value );

    /** Read a custom property from the object.
     * @param key property key
     * @param defaultValue default value to return if property with matching key does not exist
     * @returns value of matching property
     * @see setCustomProperty()
     * @see removeCustomProperty()
     * @see customProperties()
     * @note added in QGIS 2.12
     */
    QVariant customProperty( const QString &key, const QVariant &defaultValue = QVariant() ) const;

    /** Remove a custom property from the object.
     * @param key property key
     * @see setCustomProperty()
     * @see customProperty()
     * @see customProperties()
     * @note added in QGIS 2.12
     */
    void removeCustomProperty( const QString &key );

    /** Return list of keys stored in custom properties for the object.
     * @see setCustomProperty()
     * @see customProperty()
     * @see removeCustomProperty()
     * @note added in QGIS 2.12
     */
    QStringList customProperties() const;

    /** Creates an expression context relating to the objects' current state. The context includes
     * scopes for global, project and composition properties.
     * @note added in QGIS 2.12
     */
    virtual QgsExpressionContext createExpressionContext() const;

  public slots:

    /** Triggers a redraw for the item*/
    virtual void repaint();

    /** Refreshes a data defined property for the item by reevaluating the property's value
     * and redrawing the item with this new value.
     * @param property data defined property to refresh. If property is set to
     * QgsComposerItem::AllProperties then all data defined properties for the item will be
     * refreshed.
     * @param context expression context for evaluating data defined expressions
     * @note this method was added in version 2.5
     */
    virtual void refreshDataDefinedProperty( const DataDefinedProperty property = AllProperties, const QgsExpressionContext* context = 0 );

  signals:
    /** Emitted when the item changes. Signifies that the item widgets must update the
     * gui elements.
     */
    void itemChanged();

};
