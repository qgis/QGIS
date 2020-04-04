/***************************************************************************
                              qgslayoutitemmarker.h
                             ---------------------
    begin                : April 2020
    copyright            : (C) 2020 by Nyall Dawson
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

#ifndef QGSLAYOUTITEMMARKER_H
#define QGSLAYOUTITEMMARKER_H

#include "qgis_core.h"
#include "qgslayoutitem.h"
#include "qgslayoutitemregistry.h"

class QgsMarkerSymbol;

/**
 * \ingroup core
 * \class QgsLayoutItemMarker
 * \brief A layout item for showing marker symbols.
 * \since QGIS 3.14
 */
class CORE_EXPORT QgsLayoutItemMarker : public QgsLayoutItem
{
    Q_OBJECT

  public:

    /**
     * Constructor for QgsLayoutItemMarker, with the specified parent \a layout.
     */
    explicit QgsLayoutItemMarker( QgsLayout *layout );
    ~QgsLayoutItemMarker() override;

    /**
     * Returns a new marker item for the specified \a layout.
     *
     * The caller takes responsibility for deleting the returned object.
     */
    static QgsLayoutItemMarker *create( QgsLayout *layout ) SIP_FACTORY;


    int type() const override;
    QIcon icon() const override;

    /**
     * Sets the marker \a symbol used to draw the shape. Ownership is transferred.
     * \see symbol()
     */
    void setSymbol( QgsMarkerSymbol *symbol SIP_TRANSFER );

    /**
     * Returns the marker symbol used to draw the shape.
     * \see setSymbol()
     */
    QgsMarkerSymbol *symbol();

    // Depending on the symbol style, the bounding rectangle can be larger than the shape
    QRectF boundingRect() const override;

    QgsLayoutSize fixedSize() const override;

    bool accept( QgsStyleEntityVisitorInterface *visitor ) const override;

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

    std::unique_ptr< QgsMarkerSymbol > mShapeStyleSymbol;

    QPointF mPoint;
    QRectF mCurrentRectangle;
    QgsLayoutSize mFixedSize;

};


#endif //QGSLAYOUTITEMMARKER_H
