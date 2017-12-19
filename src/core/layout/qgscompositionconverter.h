/***************************************************************************
  qgscompositionconverter.h - Convert  a QGIS 2.x composition to a layout

 ---------------------
 begin                : 13.12.2017
 copyright            : (C) 2017 by Alessandro Pasotti
 email                : elpaso at itopen dot it
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSCOMPOSITIONCONVERTER_H
#define QGSCOMPOSITIONCONVERTER_H


#include <QDomDocument>
#include <QDomElement>

#include "qgis.h"
#include "qgis_sip.h"

#define SIP_NO_FILE

#include "qgspropertycollection.h"

class QgsLayout;
class QgsLayoutItem;
class QgsReadWriteContext;
class QgsProperty;
class QgsPropertyCollection;

class QgsLayoutItemLabel;
class QgsLayoutItemShape;
class QgsLayoutItemPicture;
class QgsLayoutItemPolygon;
class QgsLayoutItemPolyline;
class QgsLayoutItemMap;
class QgsLayoutItemScaleBar;
class QgsLayoutItemLegend;


/**
 * QgsCompositionConverter class converts a QGIS 2.x composition to a QGIS 3.x layout
 * \since QGIS 3.0
 * \note Not available in Python bindings.
 * \ingroup core
 */
class CORE_EXPORT QgsCompositionConverter
{
  public:

    /**
     * Composition data defined properties for different item types
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
     * The MarkerMode enum is the old QGIS 2.x arrow marker mode
     */
    enum MarkerMode
    {
      DefaultMarker,
      NoMarker,
      SVGMarker
    };


    /**
     * \brief createLayoutFromCompositionXml is a factory that creates layout instances from a
     *  QGIS 2.x XML composition \a document
     * \param parentElement is the Composition element
     * \param project the QGIS project
     * \return a QgsLayout instance
     * \since QGIS 3.0
     * \note Not available in Python bindings.
     */
    static QgsLayout *createLayoutFromCompositionXml( const QDomElement &parentElement,
        QgsProject *project );


    /**
     * addItemsFromCompositionXml parse a QGIS 2.x composition XML in the \a parentElement,
     * converts the 2.x items to the new layout elements and add them to the  \a layout
     * \param layout the lay
     * \param parentElement
     * \param position for pasting
     * \param pasteInPlace if true element position is translated to \a position
     * \return a list of layout items
     * \since QGIS 3.0
     * \note Not available in Python bindings.
     */
    static QList<QgsLayoutItem *> addItemsFromCompositionXml( QgsLayout *layout,
        const QDomElement &parentElement,
        QPointF *position = nullptr,
        bool pasteInPlace = false );

  private:

    //! Property definitions
    static QgsPropertiesDefinition sPropertyDefinitions;


    static bool readLabelXml( QgsLayoutItemLabel *layoutItem,
                              const QDomElement &itemElem,
                              const QgsProject *project );

    static bool readShapeXml( QgsLayoutItemShape *layoutItem,
                              const QDomElement &itemElem,
                              const QgsProject *project );

    static bool readPictureXml( QgsLayoutItemPicture *layoutItem,
                                const QDomElement &itemElem,
                                const QgsProject *project );

    //! For both polylines and polygons
    template <class T, class T2> static bool readPolyXml( T *layoutItem,
        const QDomElement &itemElem,
        const QgsProject *project );

    static bool readArrowXml( QgsLayoutItemPolyline *layoutItem,
                              const QDomElement &itemElem,
                              const QgsProject *project );

    static bool readMapXml( QgsLayoutItemMap *layoutItem,
                            const QDomElement &itemElem,
                            const QgsProject *project );

    static bool readScaleBarXml( QgsLayoutItemScaleBar *layoutItem,
                                 const QDomElement &itemElem,
                                 const QgsProject *project );

    static bool readLegendXml( QgsLayoutItemLegend *layoutItem,
                               const QDomElement &itemElem,
                               const QgsProject *project );


    /**
     * Sets item state from DOM element
     * \param itemElem is DOM node corresponding to item tag
     * \param doc is DOM document
     */
    static bool readOldComposerObjectXml( QgsLayoutItem *layoutItem, const QDomElement &itemElem );

    /**
     * Reads all pre 3.0 data defined properties from an XML element.
     * \since QGIS 3.0
     * \see readDataDefinedProperty
     * \see writeDataDefinedPropertyMap
     */
    static void readOldDataDefinedPropertyMap( const QDomElement &itemElem,
        QgsPropertyCollection &dataDefinedProperties );

    /**
     * Reads a pre 3.0 data defined property from an XML DOM element.
     * \since QGIS 3.0
     * \see readDataDefinedPropertyMap
     */
    static QgsProperty readOldDataDefinedProperty( const DataDefinedProperty property, const QDomElement &ddElem );

    static void initPropertyDefinitions();
    static QgsPropertiesDefinition propertyDefinitions();

    //! Reads parameter that are not subclass specific in document. Usually called from readXml methods of subclasses
    static bool readXml( QgsLayoutItem *layoutItem, const QDomElement &itemElem );

    //! Make some common import adjustments
    static void adjustPos( QgsLayout *layout, QgsLayoutItem *layoutItem, QDomNode &itemNode, QPointF *position, bool &pasteInPlace, int zOrderOffset, QPointF &pasteShiftPos, int &pageNumber );

    //! Restore general composer item properties
    static void restoreGeneralComposeItemProperties( QgsLayoutItem *layoutItem, const QDomElement &itemElem );

};

#endif // QGSCOMPOSITIONCONVERTER_H
