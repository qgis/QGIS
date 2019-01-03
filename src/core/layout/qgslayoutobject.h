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
#include <QPointer>

class QgsLayout;
class QPainter;
class QgsReadWriteContext;

/**
 * \ingroup core
 * A base class for objects which belong to a layout.
 * \since QGIS 3.0
 */
class CORE_EXPORT QgsLayoutObject: public QObject, public QgsExpressionContextGenerator
{
#ifdef SIP_RUN
#include <qgslayoutitem.h>
#include "qgslayoutitemgroup.h"
#include "qgslayoutitemmap.h"
#include "qgslayoutitempicture.h"
#include "qgslayoutitemlabel.h"
#include "qgslayoutitemlegend.h"
#include "qgslayoutitempolygon.h"
#include "qgslayoutitempolyline.h"
#include "qgslayoutitemscalebar.h"
#include "qgslayoutframe.h"
#include "qgslayoutitemshape.h"
#include "qgslayoutitempage.h"
#endif

#ifdef SIP_RUN
    SIP_CONVERT_TO_SUBCLASS_CODE
    if ( QgsLayoutItem *item = qobject_cast< QgsLayoutItem * >( sipCpp ) )
    {
      // the conversions have to be static, because they're using multiple inheritance
      // (seen in PyQt4 .sip files for some QGraphicsItem classes)
      switch ( item->type() )
      {
        // FREAKKKKIIN IMPORTANT!
        // IF YOU PUT SOMETHING HERE, PUT IT IN QgsLayoutItem CASTING **ALSO**
        // (it's not enough for it to be in only one of the places, as sip inconsistently
        // decides which casting code to perform here)

        // really, these *should* use the constants from QgsLayoutItemRegistry, but sip doesn't like that!
        case QGraphicsItem::UserType + 101:
          sipType = sipType_QgsLayoutItemGroup;
          *sipCppRet = static_cast<QgsLayoutItemGroup *>( sipCpp );
          break;
        case QGraphicsItem::UserType + 102:
          sipType = sipType_QgsLayoutItemPage;
          *sipCppRet = static_cast<QgsLayoutItemPage *>( sipCpp );
          break;
        case QGraphicsItem::UserType + 103:
          sipType = sipType_QgsLayoutItemMap;
          *sipCppRet = static_cast<QgsLayoutItemMap *>( sipCpp );
          break;
        case QGraphicsItem::UserType + 104:
          sipType = sipType_QgsLayoutItemPicture;
          *sipCppRet = static_cast<QgsLayoutItemPicture *>( sipCpp );
          break;
        case QGraphicsItem::UserType + 105:
          sipType = sipType_QgsLayoutItemLabel;
          *sipCppRet = static_cast<QgsLayoutItemLabel *>( sipCpp );
          break;
        case QGraphicsItem::UserType + 106:
          sipType = sipType_QgsLayoutItemLegend;
          *sipCppRet = static_cast<QgsLayoutItemLegend *>( sipCpp );
          break;
        case QGraphicsItem::UserType + 107:
          sipType = sipType_QgsLayoutItemShape;
          *sipCppRet = static_cast<QgsLayoutItemShape *>( sipCpp );
          break;
        case QGraphicsItem::UserType + 108:
          sipType = sipType_QgsLayoutItemPolygon;
          *sipCppRet = static_cast<QgsLayoutItemPolygon *>( sipCpp );
          break;
        case QGraphicsItem::UserType + 109:
          sipType = sipType_QgsLayoutItemPolyline;
          *sipCppRet = static_cast<QgsLayoutItemPolyline *>( sipCpp );
          break;
        case QGraphicsItem::UserType + 110:
          sipType = sipType_QgsLayoutItemScaleBar;
          *sipCppRet = static_cast<QgsLayoutItemScaleBar *>( sipCpp );
          break;
        case QGraphicsItem::UserType + 111:
          sipType = sipType_QgsLayoutFrame;
          *sipCppRet = static_cast<QgsLayoutFrame *>( sipCpp );
          break;

        // did you read that comment above? NO? Go read it now. You're about to break stuff.

        default:
          sipType = sipType_QgsLayoutItem;
      }
    }
    else
      sipType = NULL;
    SIP_END
#endif

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
      PaperWidth, //!< Paper width (deprecated)
      PaperHeight, //!< Paper height (deprecated)
      NumPages, //!< Number of pages in composition (deprecated)
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
      MapLabelMargin, //!< Map label margin
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
      ScalebarLineWidth, //!< Scalebar line width,
      //table item
      AttributeTableSourceLayer, //!< Attribute table source layer
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
    SIP_SKIP const QgsLayout *layout() const;

    /**
     * Returns the layout the object is attached to.
     */
    QgsLayout *layout();

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
     * Returns list of keys stored in custom properties for the object.
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

  signals:

    /**
     * Emitted when the object's properties change.
     */
    void changed();

  protected:

    /**
     * Stores object properties within an XML DOM element.
     * \param parentElement is the parent DOM element to store the object's properties in
     * \param document DOM document
     * \param context read write context
     * \returns true if write was successful
     * \see readObjectPropertiesFromElement()
     */
    bool writeObjectPropertiesToElement( QDomElement &parentElement, QDomDocument &document, const QgsReadWriteContext &context ) const;

    /**
     * Sets object properties from a DOM element
     * \param parentElement is the parent DOM element for the object
     * \param document DOM document
     * \param context read write context
     * \returns true if read was successful
     * \see writeObjectPropertiesToElement()
     */
    bool readObjectPropertiesFromElement( const QDomElement &parentElement, const QDomDocument &document, const QgsReadWriteContext &context );

    QPointer< QgsLayout > mLayout;

    QgsPropertyCollection mDataDefinedProperties;

    //! Custom properties for object
    QgsObjectCustomProperties mCustomProperties;

  private:

    //! Property definitions
    static QgsPropertiesDefinition sPropertyDefinitions;

    static void initPropertyDefinitions();

    friend class TestQgsLayoutObject;
    friend class QgsCompositionConverter;
};

#endif //QGSLAYOUTOBJECT_H
