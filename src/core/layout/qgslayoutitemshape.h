/***************************************************************************
                              qgslayoutitemshape.h
                             ---------------------
    begin                : July 2017
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

#ifndef QGSLAYOUTITEMSHAPE_H
#define QGSLAYOUTITEMSHAPE_H

#include "qgis_core.h"
#include "qgslayoutitem.h"
#include "qgslayoutitemregistry.h"
#include "qgssymbol.h"
#include "qgslayoutmeasurement.h"

/**
 * \ingroup core
 * \class QgsLayoutItemShape
 * \brief Layout item for basic filled shapes (e.g. rectangles, ellipses).
 * \since QGIS 3.0
 */
class CORE_EXPORT QgsLayoutItemShape : public QgsLayoutItem
{
    Q_OBJECT

  public:

    //! Shape type
    enum Shape
    {
      Ellipse, //!< Ellipse shape
      Rectangle, //!< Rectangle shape
      Triangle //!< Triangle shape
    };


    /**
     * Constructor for QgsLayoutItemShape, with the specified parent \a layout.
     */
    explicit QgsLayoutItemShape( QgsLayout *layout );

    /**
     * Returns a new shape item for the specified \a layout.
     *
     * The caller takes responsibility for deleting the returned object.
     */
    static QgsLayoutItemShape *create( QgsLayout *layout ) SIP_FACTORY;


    int type() const override;
    QIcon icon() const override;

    //Overridden to return shape type
    QString displayName() const override;

    /**
     * Returns the type of shape (e.g. rectangle, ellipse, etc).
     * \see setShapeType()
     */
    QgsLayoutItemShape::Shape shapeType() const { return mShape; }

    /**
     * Sets the \a type of shape (e.g. rectangle, ellipse, etc).
     * \see shapeType()
     */
    void setShapeType( QgsLayoutItemShape::Shape type );

    /**
     * Sets the fill \a symbol used to draw the shape. Ownership is not transferred
     * and a clone of the symbol is made.
     * \see symbol()
     */
    void setSymbol( QgsFillSymbol *symbol );

    /**
     * Returns the fill symbol used to draw the shape.
     * \see setSymbol()
     */
    QgsFillSymbol *symbol() { return mShapeStyleSymbol.get(); }

    /**
     * Sets the corner \a radius for rounded rectangle corners.
     * \see cornerRadius()
     */
    void setCornerRadius( QgsLayoutMeasurement radius ) { mCornerRadius = radius; }

    /**
     * Returns the corner radius for rounded rectangle corners.
     * \see setCornerRadius()
     */
    QgsLayoutMeasurement cornerRadius() const { return mCornerRadius; }

    // Depending on the symbol style, the bounding rectangle can be larger than the shape
    QRectF boundingRect() const override;

    // Reimplement estimatedFrameBleed, since frames on shapes are drawn using symbology
    // rather than the item's pen
    double estimatedFrameBleed() const override;

  protected:

    void draw( QgsLayoutItemRenderContext &context ) override;

    bool writePropertiesToElement( QDomElement &element, QDomDocument &document, const QgsReadWriteContext &context ) const override;
    bool readPropertiesFromElement( const QDomElement &element, const QDomDocument &document, const QgsReadWriteContext &context ) override;

  private slots:

    /**
     * Should be called after the shape's symbol is changed. Redraws the shape and recalculates
     * its selection bounds.
    */
    void refreshSymbol();

    //! Updates the bounding rect of this item
    void updateBoundingRect();

  private:

    Shape mShape = Rectangle;

    std::unique_ptr< QgsFillSymbol > mShapeStyleSymbol;

    double mMaxSymbolBleed = 0.0;
    //! Current bounding rectangle of shape
    QRectF mCurrentRectangle;

    QgsLayoutMeasurement mCornerRadius;
};


#endif //QGSLAYOUTITEMSHAPE_H
