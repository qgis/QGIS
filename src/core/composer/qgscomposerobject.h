/***************************************************************************
                         qgscomposerobject.h
                             -------------------
    begin                : July 2014
    copyright            : (C) 2014 by Nyall Dawson,Radim Blazek
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSCOMPOSEROBJECT_H
#define QGSCOMPOSEROBJECT_H

#include "qgis_core.h"
#include "qgis_sip.h"
#include "qgsobjectcustomproperties.h"
#include "qgsexpressioncontextgenerator.h"
#include "qgspropertycollection.h"
#include "qgsreadwritecontext.h"

#include <QObject>
#include <QDomNode>
#include <QMap>

class QgsComposition;
class QPainter;

/**
 * \ingroup core
 * A base class for objects which belong to a map composition.
 */
class CORE_EXPORT QgsComposerObject: public QObject, public QgsExpressionContextGenerator
{
    Q_OBJECT
  public:

    /**
     * Data defined properties for different item types
     */
    enum DataDefinedProperty
    {
      NoProperty = 0, //!< No property
      AllProperties, //!< All properties for item
      TestProperty, //!< Dummy property with no effect on item
      //composer page properties
      PresetPaperSize, //!< Preset paper size for composition
      PaperWidth, //!< Paper width
      PaperHeight, //!< Paper height
      NumPages, //!< Number of pages in composition
      PaperOrientation, //!< Paper orientation
      //general composer item properties
      PageNumber, //!< Page number for item placement
      PositionX, //!< X position on page
      PositionY, //!< Y position on page
      ItemWidth, //!< Width of item
      ItemHeight, //!< Height of item
      ItemRotation, //!< Rotation of item
      Transparency, //!< Item transparency (deprecated)
      Opacity, //!< Item opacity
      BlendMode, //!< Item blend mode
      ExcludeFromExports, //!< Exclude item from exports
      FrameColor, //!< Item frame color
      BackgroundColor, //!< Item background color
      //composer map
      MapRotation, //!< Map rotation
      MapScale, //!< Map scale
      MapXMin, //!< Map extent x minimum
      MapYMin, //!< Map extent y minimum
      MapXMax, //!< Map extent x maximum
      MapYMax, //!< Map extent y maximum
      MapAtlasMargin, //!< Map atlas margin
      MapLayers, //!< Map layer set
      MapStylePreset, //!< Layer and style map theme
      //composer picture
      PictureSource, //!< Picture source url
      PictureSvgBackgroundColor, //!< SVG background color
      PictureSvgStrokeColor, //!< SVG stroke color
      PictureSvgStrokeWidth, //!< SVG stroke width
      //html item
      SourceUrl, //!< Html source url
      //legend item
      LegendTitle, //!< Legend title
      LegendColumnCount, //!< Legend column count
      //scalebar item
      ScalebarFillColor, //!< Scalebar fill color
      ScalebarFillColor2, //!< Scalebar secondary fill color
      ScalebarLineColor, //!< Scalebar line color
      ScalebarLineWidth, //!< Scalebar line width
    };

    /**
     * Specifies whether the value returned by a function should be the original, user
     * set value, or the current evaluated value for the property. This may differ if
     * a property has a data defined expression active.
     */
    enum PropertyValueType
    {
      EvaluatedValue = 0, //!< Return the current evaluated value for the property
      OriginalValue //!< Return the original, user set value
    };

    /**
     * Returns the composer object property definitions.
     * \since QGIS 3.0
     */
    static const QgsPropertiesDefinition &propertyDefinitions();

    /**
     * Constructor
     * \param composition parent composition
     */
    QgsComposerObject( QgsComposition *composition );
    virtual ~QgsComposerObject() = default;

    /**
     * Returns the composition the item is attached to.
     * \returns QgsComposition for item.
     */
    const QgsComposition *composition() const { return mComposition; }

    //! \note not available in Python bindings
    QgsComposition *composition() { return mComposition; } SIP_SKIP

    /**
     * Stores item state in DOM element
     * \param elem is DOM element corresponding to item tag
     * \param doc is the DOM document
     */
    virtual bool writeXml( QDomElement &elem, QDomDocument &doc ) const;

    /**
     * Sets item state from DOM element
     * \param itemElem is DOM node corresponding to item tag
     * \param doc is DOM document
     */
    virtual bool readXml( const QDomElement &itemElem, const QDomDocument &doc );

    /**
     * Returns a reference to the object's property collection, used for data defined overrides.
     * \since QGIS 3.0
     * \see setDataDefinedProperties()
     */
    QgsPropertyCollection &dataDefinedProperties() { return mDataDefinedProperties; }

    /**
     * Returns a reference to the object's property collection, used for data defined overrides.
     * \since QGIS 3.0
     * \see setDataDefinedProperties()
     */
    const QgsPropertyCollection &dataDefinedProperties() const { return mDataDefinedProperties; } SIP_SKIP

    /**
     * Sets the objects's property collection, used for data defined overrides.
     * \param collection property collection. Existing properties will be replaced.
     * \since QGIS 3.0
     * \see dataDefinedProperties()
     */
    void setDataDefinedProperties( const QgsPropertyCollection &collection ) { mDataDefinedProperties = collection; }

    /**
     * Set a custom property for the object.
     * \param key property key. If a property with the same key already exists it will be overwritten.
     * \param value property value
     * \see customProperty()
     * \see removeCustomProperty()
     * \see customProperties()
     * \since QGIS 2.12
     */
    void setCustomProperty( const QString &key, const QVariant &value );

    /**
     * Read a custom property from the object.
     * \param key property key
     * \param defaultValue default value to return if property with matching key does not exist
     * \returns value of matching property
     * \see setCustomProperty()
     * \see removeCustomProperty()
     * \see customProperties()
     * \since QGIS 2.12
     */
    QVariant customProperty( const QString &key, const QVariant &defaultValue = QVariant() ) const;

    /**
     * Remove a custom property from the object.
     * \param key property key
     * \see setCustomProperty()
     * \see customProperty()
     * \see customProperties()
     * \since QGIS 2.12
     */
    void removeCustomProperty( const QString &key );

    /**
     * Return list of keys stored in custom properties for the object.
     * \see setCustomProperty()
     * \see customProperty()
     * \see removeCustomProperty()
     * \since QGIS 2.12
     */
    QStringList customProperties() const;

    /**
     * Creates an expression context relating to the objects' current state. The context includes
     * scopes for global, project and composition properties.
     * \since QGIS 2.12
     */
    virtual QgsExpressionContext createExpressionContext() const;

  public slots:

    //! Triggers a redraw for the item
    virtual void repaint();

    /**
     * Refreshes a data defined property for the item by reevaluating the property's value
     * and redrawing the item with this new value.
     * \param property data defined property to refresh. If property is set to
     * QgsComposerItem::AllProperties then all data defined properties for the item will be
     * refreshed.
     * \param context expression context for evaluating data defined expressions
     * \since QGIS 2.5
     */
    virtual void refreshDataDefinedProperty( const DataDefinedProperty property = AllProperties, const QgsExpressionContext *context = nullptr );

  protected:

    QgsComposition *mComposition = nullptr;

    //! Custom properties for object
    QgsObjectCustomProperties mCustomProperties;

    QgsPropertyCollection mDataDefinedProperties;

  signals:

    /**
     * Emitted when the item changes. Signifies that the item widgets must update the
     * gui elements.
     */
    void itemChanged();

  private slots:

    /**
     * Prepares all composer item data defined properties using the current atlas coverage layer if set.
     */
    void prepareProperties() const;

  private:
    //! Property definitions
    static QgsPropertiesDefinition sPropertyDefinitions;

    static void initPropertyDefinitions();

    friend class TestQgsComposerObject;
};

#endif
