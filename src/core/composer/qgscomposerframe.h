/***************************************************************************
                              qgscomposerframe.h
    ------------------------------------------------------------
    begin                : July 2012
    copyright            : (C) 2012 by Marco Hugentobler
    email                : marco dot hugentobler at sourcepole dot ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSCOMPOSERFRAME_H
#define QGSCOMPOSERFRAME_H

#include "qgscomposeritem.h"

class QgsComposition;
class QgsComposerMultiFrame;

/**Frame item for a composer multiframe item*/
class CORE_EXPORT QgsComposerFrame: public QgsComposerItem
{

  public:

    QgsComposerFrame( QgsComposition* c, QgsComposerMultiFrame* mf, qreal x, qreal y, qreal width, qreal height );

    ~QgsComposerFrame();

    /**Sets the visible part of the multiframe's content which is visible within
     * this frame (relative to the total multiframe extent in mm).
     * @param section visible portion of content
     * @see extent
    */
    void setContentSection( const QRectF& section ) { mSection = section; }

    /**Returns the parent multiframe for the frame.
     * @returns parent multiframe
     */
    QgsComposerMultiFrame* multiFrame() const { return mMultiFrame; }

    //Overriden to allow multiframe to set display name
    virtual QString displayName() const override;

    //Overriden to handle fixed frame sizes set by multi frame
    void setSceneRect( const QRectF& rectangle ) override;

    void paint( QPainter* painter, const QStyleOptionGraphicsItem* itemStyle, QWidget* pWidget ) override;
    void beginItemCommand( const QString& text ) override;
    void endItemCommand() override;
    bool writeXML( QDomElement& elem, QDomDocument & doc ) const override;
    bool readXML( const QDomElement& itemElem, const QDomDocument& doc ) override;
    int type() const override { return ComposerFrame; }

    /**Returns the visible portion of the multi frame's content which
     * is shown in this frame.
     * @returns extent of visible portion
     * @note added in QGIS 2.5
     * @see setContentSection
     */
    QRectF extent() const { return mSection; }

    /**Returns whether the page should be hidden (ie, not included in composer exports) if this frame is empty
     * @returns true if page should be hidden if frame is empty
     * @note added in QGIS 2.5
     * @see setHidePageIfEmpty
     */
    bool hidePageIfEmpty() const { return mHidePageIfEmpty; }

    /**Sets whether the page should be hidden (ie, not included in composer exports) if this frame is empty
     * @param hidePageIfEmpty set to true if page should be hidden if frame is empty
     * @note added in QGIS 2.5
     * @see hidePageIfEmpty
     */
    void setHidePageIfEmpty( const bool hidePageIfEmpty );

    /**Returns whether the background and frame border should be hidden if this frame is empty
     * @returns true if background and border should be hidden if frame is empty
     * @note added in QGIS 2.5
     * @see setHideBackgroundIfEmpty
     */
    bool hideBackgroundIfEmpty() const { return mHideBackgroundIfEmpty; }

    /**Sets whether the background and frame border should be hidden if this frame is empty
     * @param hideBackgroundIfEmpty set to true if background and border should be hidden if frame is empty
     * @note added in QGIS 2.5
     * @see hideBackgroundIfEmpty
     */
    void setHideBackgroundIfEmpty( const bool hideBackgroundIfEmpty );

    /**Returns whether the frame is empty
     * @returns true if frame is empty
     * @note added in QGIS 2.5
     * @see hidePageIfEmpty
     */
    bool isEmpty() const;

  private:
    QgsComposerFrame(); //forbidden
    QgsComposerMultiFrame* mMultiFrame;
    QRectF mSection;

    /**if true, composition will not export page if this frame is empty*/
    bool mHidePageIfEmpty;
    /**if true, background and outside frame will not be drawn if frame is empty*/
    bool mHideBackgroundIfEmpty;

};

#endif // QGSCOMPOSERFRAME_H
