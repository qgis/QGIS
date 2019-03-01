/***************************************************************************
                              qgslayoutitempage.h
                             --------------------
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

#ifndef QGSLAYOUTITEMPAGE_H
#define QGSLAYOUTITEMPAGE_H

#include "qgis_core.h"
#include "qgslayoutitem.h"
#include "qgslayoutitemregistry.h"
#include "qgis_sip.h"


///@cond PRIVATE
#ifndef SIP_RUN

/**
 * \ingroup core
 * Item representing a grid. This is drawn separately to the underlying page item since the grid needs to be
 * drawn above all other layout items, while the paper item is drawn below all others.
 * \since QGIS 3.0
 */
class CORE_EXPORT QgsLayoutItemPageGrid: public QGraphicsRectItem
{
  public:
    QgsLayoutItemPageGrid( double x, double y, double width, double height, QgsLayout *layout );

    void paint( QPainter *painter, const QStyleOptionGraphicsItem *itemStyle, QWidget *pWidget ) override;

  private:
    QgsLayout *mLayout = nullptr;
};
#endif
///@endcond

/**
 * \ingroup core
 * \class QgsLayoutItemPage
 * \brief Item representing the paper in a layout.
 * \since QGIS 3.0
 */
class CORE_EXPORT QgsLayoutItemPage : public QgsLayoutItem
{

    Q_OBJECT

  public:

    //! Page orientiation
    enum Orientation
    {
      Portrait, //!< Portrait orientation
      Landscape //!< Landscape orientation
    };

    //! Page item undo commands, used for collapsing undo commands
    enum UndoCommand
    {
      UndoPageSymbol = 3000, //!< Layout page symbol change
    };

    /**
     * Constructor for QgsLayoutItemPage, with the specified parent \a layout.
     */
    explicit QgsLayoutItemPage( QgsLayout *layout );

    /**
     * Returns a new page item for the specified \a layout.
     *
     * The caller takes responsibility for deleting the returned object.
     */
    static QgsLayoutItemPage *create( QgsLayout *layout ) SIP_FACTORY;

    int type() const override;

    /**
     * Sets the \a size of the page.
     * \see pageSize()
     */
    void setPageSize( const QgsLayoutSize &size );

    /**
     * Sets the page size to a known page \a size, e.g. "A4" and \a orientation.
     * The known page sizes are managed by QgsPageSizeRegistry. Valid page sizes
     * can be retrieved via QgsPageSizeRegistry::entries().
     * The function returns TRUE if \a size was a valid page size and the page
     * size was changed. If FALSE is returned then \a size could not be matched
     * to a known page size.
     * \see pageSize()
     */
    bool setPageSize( const QString &size, Orientation orientation = Portrait );

    /**
     * Returns the size of the page.
     * \see setPageSize()
     */
    QgsLayoutSize pageSize() const;

    /**
     * Returns the page orientiation.
     * \note There is no direct setter for page orientation - use setPageSize() instead.
     */
    Orientation orientation() const;

    /**
     * Decodes a \a string representing a page orientation. If specified, \a ok
     * will be set to TRUE if string could be successfully interpreted as a
     * page orientation.
    */
    static QgsLayoutItemPage::Orientation decodePageOrientation( const QString &string, bool *ok SIP_OUT = nullptr );

    QRectF boundingRect() const override;
    void attemptResize( const QgsLayoutSize &size, bool includesFrame = false ) override;
    QgsAbstractLayoutUndoCommand *createCommand( const QString &text, int id, QUndoCommand *parent = nullptr ) override SIP_FACTORY;

  public slots:

    void redraw() override;

  protected:

    void draw( QgsLayoutItemRenderContext &context ) override;
    void drawFrame( QgsRenderContext &context ) override;
    void drawBackground( QgsRenderContext &context ) override;

  private:

    double mMaximumShadowWidth = -1;

    std::unique_ptr< QgsLayoutItemPageGrid > mGrid;
    mutable QRectF mBoundingRect;

    friend class TestQgsLayoutPage;
};

#endif //QGSLAYOUTITEMPAGE_H
