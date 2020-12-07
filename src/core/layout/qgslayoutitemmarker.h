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
#include "qgslayoutnortharrowhandler.h"

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

    /**
     * Sets the \a map object for rotation.
     *
     * If this is set then the marker will be rotated by the same
     * amount as the specified map object. This is useful especially for
     * syncing north arrows with a map item.
     *
     * \see linkedMap()
     */
    void setLinkedMap( QgsLayoutItemMap *map );

    /**
     * Returns the linked rotation map, if set. An NULLPTR means map rotation is
     * disabled.  If this is set then the marker is rotated by the same amount
     * as the specified map object.
     * \see setLinkedMap()
     */
    QgsLayoutItemMap *linkedMap() const;

    /**
     * When the marker is linked to a map in north arrow rotation mode,
     * returns the current north arrow rotation for the marker.
     *
     * \see setLinkedMap()
     */
    double northArrowRotation() const { return mNorthArrowRotation; }

    /**
     * Returns the mode used to align the marker to a map's North.
     * \see setNorthMode()
     * \see northOffset()
     */
    QgsLayoutNorthArrowHandler::NorthMode northMode() const;

    /**
     * Sets the \a mode used to align the marker to a map's North.
     * \see northMode()
     * \see setNorthOffset()
     */
    void setNorthMode( QgsLayoutNorthArrowHandler::NorthMode mode );

    /**
     * Returns the offset added to the marker's rotation from a map's North.
     * \see setNorthOffset()
     * \see northMode()
     */
    double northOffset() const;

    /**
     * Sets the \a offset added to the marker's rotation from a map's North.
     * \see northOffset()
     * \see setNorthMode()
     */
    void setNorthOffset( double offset );

    // Depending on the symbol style, the bounding rectangle can be larger than the shape
    QRectF boundingRect() const override;

    QgsLayoutSize fixedSize() const override;

    bool accept( QgsStyleEntityVisitorInterface *visitor ) const override;

  protected:

    void draw( QgsLayoutItemRenderContext &context ) override;

    bool writePropertiesToElement( QDomElement &element, QDomDocument &document, const QgsReadWriteContext &context ) const override;
    bool readPropertiesFromElement( const QDomElement &element, const QDomDocument &document, const QgsReadWriteContext &context ) override;

    void finalizeRestoreFromXml() override;
  private slots:

    /**
     * Should be called after the shape's symbol is changed. Redraws the shape and recalculates
     * its selection bounds.
    */
    void refreshSymbol();

    //! Updates the bounding rect of this item
    void updateBoundingRect();

    void northArrowRotationChanged( double rotation );

  private:

    std::unique_ptr< QgsMarkerSymbol > mShapeStyleSymbol;

    QPointF mPoint;
    QRectF mCurrentRectangle;
    QgsLayoutSize mFixedSize;

    QString mRotationMapUuid;
    QgsLayoutNorthArrowHandler *mNorthArrowHandler = nullptr;
    double mNorthArrowRotation = 0;

    QgsLayoutItemMarker( const QgsLayoutItemMarker & ) = delete;
    QgsLayoutItemMarker &operator=( const QgsLayoutItemMarker & ) = delete;
};


#endif //QGSLAYOUTITEMMARKER_H
