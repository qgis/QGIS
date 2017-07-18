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
 * \brief Base class for layout items which are basic shapes (e.g. rectangles, ellipses).
 * \since QGIS 3.0
 */
class CORE_EXPORT QgsLayoutItemShape : public QgsLayoutItem
{
    Q_OBJECT

  public:

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

  protected:

    /**
     * Constructor for QgsLayoutItemShape, with the specified parent \a layout.
     */
    explicit QgsLayoutItemShape( QgsLayout *layout );

  private:
    std::unique_ptr< QgsFillSymbol > mShapeStyleSymbol;
};


/**
 * \ingroup core
 * \class QgsLayoutItemRectangularShape
 * \brief A rectangular shape item for layouts.
 * \since QGIS 3.0
 */
class CORE_EXPORT QgsLayoutItemRectangularShape : public QgsLayoutItemShape
{

    Q_OBJECT

  public:

    /**
     * Constructor for QgsLayoutItemRectangularShape, with the specified parent \a layout.
     */
    explicit QgsLayoutItemRectangularShape( QgsLayout *layout );
    virtual int type() const override { return QgsLayoutItemRegistry::LayoutRectangle; }

    /**
     * Returns a new rectangular item for the specified \a layout.
     *
     * The caller takes responsibility for deleting the returned object.
     */
    static QgsLayoutItemRectangularShape *create( QgsLayout *layout, const QVariantMap &settings ) SIP_FACTORY;

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

  protected:

    void draw( QgsRenderContext &context, const QStyleOptionGraphicsItem *itemStyle = nullptr ) override;

  private:
    QgsLayoutMeasurement mCornerRadius;
};

/**
 * \ingroup core
 * \class QgsLayoutItemEllipseShape
 * \brief A ellipse shape item for layouts.
 * \since QGIS 3.0
 */
class CORE_EXPORT QgsLayoutItemEllipseShape : public QgsLayoutItemShape
{

    Q_OBJECT

  public:

    /**
     * Constructor for QgsLayoutItemEllipseShape, with the specified parent \a layout.
     */
    explicit QgsLayoutItemEllipseShape( QgsLayout *layout );
    virtual int type() const override { return QgsLayoutItemRegistry::LayoutEllipse; }

    /**
     * Returns a new ellipse item for the specified \a layout.
     *
     * The caller takes responsibility for deleting the returned object.
     */
    static QgsLayoutItemEllipseShape *create( QgsLayout *layout, const QVariantMap &settings ) SIP_FACTORY;

  protected:

    void draw( QgsRenderContext &context, const QStyleOptionGraphicsItem *itemStyle = nullptr ) override;

};

/**
 * \ingroup core
 * \class QgsLayoutItemTriangleShape
 * \brief A triangle shape item for layouts.
 * \since QGIS 3.0
 */
class CORE_EXPORT QgsLayoutItemTriangleShape : public QgsLayoutItemShape
{

    Q_OBJECT

  public:

    /**
     * Constructor for QgsLayoutItemTriangleShape, with the specified parent \a layout.
     */
    explicit QgsLayoutItemTriangleShape( QgsLayout *layout );
    virtual int type() const override { return QgsLayoutItemRegistry::LayoutTriangle; }

    /**
     * Returns a new triangle item for the specified \a layout.
     *
     * The caller takes responsibility for deleting the returned object.
     */
    static QgsLayoutItemTriangleShape *create( QgsLayout *layout, const QVariantMap &settings ) SIP_FACTORY;

  protected:

    void draw( QgsRenderContext &context, const QStyleOptionGraphicsItem *itemStyle = nullptr ) override;

};

#endif //QGSLAYOUTITEMSHAPE_H
