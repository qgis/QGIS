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

#include <QObject>
#include <QDomNode>
#include <QPointF>
#include <QRectF>
#include <QMap>

class QgsComposition;
class QPainter;
class QgsDataDefined;

/** \ingroup MapComposer
 * A base class for objects which belong to a map composition.
 */
class CORE_EXPORT QgsComposerObject: public QObject
{
    Q_OBJECT
  public:

    /** Data defined properties for different item types
     */
    enum DataDefinedProperty
    {
      NoProperty = 0, /*< no property */
      AllProperties, /*< all properties for item */
      TestProperty, /*< dummy property with no effect on item*/
      //composer page properties
      PresetPaperSize, /*< preset paper size for composition */
      PaperWidth, /*< paper width */
      PaperHeight, /*< paper height */
      NumPages, /*< number of pages in composition */
      PaperOrientation, /*< paper orientation */
      //general composer item properties
      PageNumber, /*< page number for item placement */
      PositionX, /*< x position on page */
      PositionY, /*< y position on page */
      ItemWidth, /*< width of item */
      ItemHeight, /*< height of item */
      ItemRotation, /*< rotation of item */
      Transparency, /*< item transparency */
      BlendMode, /*< item blend mode */
      ExcludeFromExports, /*< exclude item from exports */
      //composer map
      MapRotation, /*< map rotation */
      MapScale, /*< map scale */
      MapXMin, /*< map extent x minimum */
      MapYMin, /*< map extent y minimum */
      MapXMax, /*< map extent x maximum */
      MapYMax, /*< map extent y maximum */
      MapAtlasMargin, /*< map atlas margin*/
      //composer picture
      PictureSource, /*< picture source url */
      //html item
      SourceUrl /*< html source utl */
    };

    /** Specifies whether the value returned by a function should be the original, user
     * set value, or the current evaluated value for the property. This may differ if
     * a property has a data defined expression active.
     */
    enum PropertyValueType
    {
      EvaluatedValue = 0, /*< return the current evaluated value for the property */
      OriginalValue /*< return the original, user set value */
    };

    /**Constructor
     * @param composition parent composition
     */
    QgsComposerObject( QgsComposition* composition );
    virtual ~QgsComposerObject();

    /**Returns the composition the item is attached to.
     * @returns QgsComposition for item.
     */
    const QgsComposition* composition() const { return mComposition; }
    QgsComposition* composition() { return mComposition; }

    /**Stores item state in DOM element
     * @param elem is DOM element corresponding to item tag
     * @param doc is the DOM document
     */
    virtual bool writeXML( QDomElement& elem, QDomDocument & doc ) const;

    /**Sets item state from DOM element
     * @param itemElem is DOM node corresponding to item tag
     * @param doc is DOM document
     */
    virtual bool readXML( const QDomElement& itemElem, const QDomDocument& doc );

    /**Returns a reference to the data defined settings for one of the item's data defined properties.
     * @param property data defined property to return
     * @note this method was added in version 2.5
    */
    QgsDataDefined* dataDefinedProperty( const DataDefinedProperty property ) const;

    /**Sets parameters for a data defined property for the item
     * @param property data defined property to set
     * @param active true if data defined property is active, false if it is disabled
     * @param useExpression true if the expression should be used
     * @param expression expression for data defined property
     * @param field field name if the data defined property should take its value from a field
     * @note this method was added in version 2.5
    */
    void setDataDefinedProperty( const DataDefinedProperty property, const bool active, const bool useExpression, const QString &expression, const QString &field );

  public slots:

    /**Triggers a redraw for the item*/
    virtual void repaint();

    /**Refreshes a data defined property for the item by reevaluating the property's value
     * and redrawing the item with this new value.
     * @param property data defined property to refresh. If property is set to
     * QgsComposerItem::AllProperties then all data defined properties for the item will be
     * refreshed.
     * @note this method was added in version 2.5
    */
    virtual void refreshDataDefinedProperty( const DataDefinedProperty property = AllProperties );

  protected:

    QgsComposition* mComposition;

    /**Map of data defined properties for the item to string name to use when exporting item to xml*/
    QMap< QgsComposerObject::DataDefinedProperty, QString > mDataDefinedNames;

    /**Evaluate a data defined property and return the calculated value
     * @returns true if data defined property could be successfully evaluated
     * @param property data defined property to evaluate
     * @param expressionValue QVariant for storing the evaluated value
     * @note this method was added in version 2.5
    */
    bool dataDefinedEvaluate( const QgsComposerObject::DataDefinedProperty property, QVariant &expressionValue );

  signals:
    /**Emitted when the item changes. Signifies that the item widgets must update the
     * gui elements.
    */
    void itemChanged();

  private slots:
    /**Prepares all composer item data defined expressions using the current atlas coverage layer if set.
     * @note this method was added in version 2.5
    */
    void prepareDataDefinedExpressions() const;

  private:

    /**Map of current data defined properties*/
    QMap< QgsComposerObject::DataDefinedProperty, QgsDataDefined* > mDataDefinedProperties;

    friend class TestQgsComposerObject;
};

#endif
