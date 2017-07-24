/***************************************************************************
                         qgslayoutobject.h
                             -------------------
    begin                : June 2017
    copyright            : (C) 2017 by Nyall Dawson
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
#ifndef QGSLAYOUTOBJECT_H
#define QGSLAYOUTOBJECT_H

#include "qgis_core.h"
#include "qgis_sip.h"
#include "qgspropertycollection.h"
#include "qgsobjectcustomproperties.h"
#include "qgsexpressioncontextgenerator.h"
#include <QObject>
#include <QDomNode>
#include <QMap>

class QgsLayout;
class QPainter;

/**
 * \ingroup core
 * A base class for objects which belong to a layout.
 * \since QGIS 3.0
 */
class CORE_EXPORT QgsLayoutObject: public QObject, public QgsExpressionContextGenerator
{
    Q_OBJECT
  public:

    /** Data defined properties for different item types
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
     * Returns the layout object property definitions.
     */
    static const QgsPropertiesDefinition &propertyDefinitions();

    /**
     * Constructor for QgsLayoutObject, with the specified parent \a layout.
     * \note While ownership of a QgsLayoutObject is not passed to the layout,
     * classes which are derived from QgsLayoutObject (such as QgsLayoutItem)
     * may transfer their ownership to a layout upon construction.
     */
    explicit QgsLayoutObject( QgsLayout *layout );

    /**
     * Returns the layout the object is attached to.
     */
    SIP_SKIP const QgsLayout *layout() const { return mLayout; }

    /**
     * Returns the layout the object is attached to.
     */
    QgsLayout *layout() { return mLayout; }

    /**
     * Returns a reference to the object's property collection, used for data defined overrides.
     * \see setDataDefinedProperties()
     */
    QgsPropertyCollection &dataDefinedProperties() { return mDataDefinedProperties; }

    /**
     * Returns a reference to the object's property collection, used for data defined overrides.
     * \see setDataDefinedProperties()
     */
    const QgsPropertyCollection &dataDefinedProperties() const { return mDataDefinedProperties; } SIP_SKIP

    /**
     * Sets the objects's property collection, used for data defined overrides.
     * \param collection property collection. Existing properties will be replaced.
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
     */
    QVariant customProperty( const QString &key, const QVariant &defaultValue = QVariant() ) const;

    /**
     * Remove a custom property from the object.
     * \param key property key
     * \see setCustomProperty()
     * \see customProperty()
     * \see customProperties()
     */
    void removeCustomProperty( const QString &key );

    /**
     * Return list of keys stored in custom properties for the object.
     * \see setCustomProperty()
     * \see customProperty()
     * \see removeCustomProperty()
     */
    QStringList customProperties() const;

    /**
     * Creates an expression context relating to the objects' current state. The context includes
     * scopes for global, project and layout properties.
     */
    QgsExpressionContext createExpressionContext() const override;

  public slots:

    /**
     * Refreshes the object, causing a recalculation of any property overrides.
     */
    virtual void refresh() {}

  protected:

    QgsLayout *mLayout = nullptr;

    QgsPropertyCollection mDataDefinedProperties;

    //! Custom properties for object
    QgsObjectCustomProperties mCustomProperties;

  private:

    //! Property definitions
    static QgsPropertiesDefinition sPropertyDefinitions;

    static void initPropertyDefinitions();

    friend class TestQgsLayoutObject;
};

#endif //QGSLAYOUTOBJECT_H
