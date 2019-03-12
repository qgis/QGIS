/***************************************************************************
                              qgslayoutframe.cpp
                              ------------------
    begin                : October 2017
    copyright            : (C) 2017 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSLAYOUTFRAME_H
#define QGSLAYOUTFRAME_H

#include "qgis_core.h"
#include "qgis_sip.h"
#include "qgslayoutitem.h"

class QgsLayout;
class QgsLayoutMultiFrame;

/**
 * \ingroup core
 * Base class for frame items, which form a layout multiframe item.
 * \since QGIS 3.0
*/
class CORE_EXPORT QgsLayoutFrame: public QgsLayoutItem
{
    Q_OBJECT

  public:

    /**
     * Constructor for QgsLayoutFrame, with the specified parent \a layout
     * and belonging to a \a multiFrame.
     */
    QgsLayoutFrame( QgsLayout *layout, QgsLayoutMultiFrame *multiFrame );

    /**
     * Creates a new QgsLayoutFrame belonging to the specified \a layout.
     */
    static QgsLayoutFrame *create( QgsLayout *layout ) SIP_FACTORY;

    int type() const override;
    QIcon icon() const override;

    //Overridden to allow multiframe to set display name
    QString displayName() const override;

    void cleanup() override;

    /**
     * Sets the visible part of the multiframe's content which is visible within
     * this frame (relative to the total multiframe extent in layout units).
     * \see extent()
     */
    void setContentSection( const QRectF &section ) { mSection = section; }

    /**
     * Returns the parent multiframe for the frame.
     */
    QgsLayoutMultiFrame *multiFrame() const;

    QgsLayoutSize minimumSize() const override;
    QgsLayoutSize fixedSize() const override;

    /**
     * Returns the visible portion of the multi frame's content which
     * is shown in this frame, in layout units.
     * \see setContentSection()
     */
    QRectF extent() const { return mSection; }

    /**
     * Returns whether the page should be hidden (ie, not included in layout exports) if this frame is empty
     * \returns TRUE if page should be hidden if frame is empty
     * \see setHidePageIfEmpty()
     */
    bool hidePageIfEmpty() const { return mHidePageIfEmpty; }

    /**
     * Sets whether the page should be hidden (ie, not included in layout exports) if this frame is empty
     * \param hidePageIfEmpty set to TRUE if page should be hidden if frame is empty
     * \see hidePageIfEmpty()
     */
    void setHidePageIfEmpty( bool hidePageIfEmpty );

    /**
     * Returns whether the background and frame stroke should be hidden if this frame is empty
     * \returns TRUE if background and stroke should be hidden if frame is empty
     * \see setHideBackgroundIfEmpty()
     */
    bool hideBackgroundIfEmpty() const { return mHideBackgroundIfEmpty; }

    /**
     * Sets whether the background and frame stroke should be hidden if this frame is empty
     * \param hideBackgroundIfEmpty set to TRUE if background and stroke should be hidden if frame is empty
     * \see hideBackgroundIfEmpty()
     */
    void setHideBackgroundIfEmpty( bool hideBackgroundIfEmpty );

    /**
     * Returns whether the frame is empty.
     * \see hidePageIfEmpty()
     */
    bool isEmpty() const;

    QgsExpressionContext createExpressionContext() const override;

  protected:

    void draw( QgsLayoutItemRenderContext &context ) override;
    void drawFrame( QgsRenderContext &context ) override;
    void drawBackground( QgsRenderContext &context ) override;
    bool writePropertiesToElement( QDomElement &parentElement, QDomDocument &document, const QgsReadWriteContext &context ) const override;
    bool readPropertiesFromElement( const QDomElement &itemElement, const QDomDocument &document, const QgsReadWriteContext &context ) override;

  private:
    QgsLayoutFrame() = delete;
    QgsLayoutMultiFrame *mMultiFrame = nullptr;
    QString mMultiFrameUuid;
    QRectF mSection;

    //! If TRUE, layout will not export page if this frame is empty
    bool mHidePageIfEmpty = false;
    //! If TRUE, background and outside frame will not be drawn if frame is empty
    bool mHideBackgroundIfEmpty = false;

    friend class QgsLayoutMultiFrame;

};

#endif // QGSLAYOUTFRAME_H
