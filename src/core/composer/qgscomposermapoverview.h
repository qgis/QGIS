/***************************************************************************
                         qgscomposermapoverview.h
                         --------------------
    begin                : July 2014
    copyright            : (C) 2014 by Nyall Dawson
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

#ifndef QGSCOMPOSERMAPOVERVIEW_H
#define QGSCOMPOSERMAPOVERVIEW_H

#include "qgscomposermapitem.h"
#include <QString>
#include <QObject>
#include <QPainter>

class QDomDocument;
class QDomElement;
class QgsFillSymbolV2;
class QgsComposerMapOverview;

/**\ingroup MapComposer
 * \class QgsComposerMapOverviewStack
 * \brief A collection of overviews which are drawn above the map content in a
 * QgsComposerMap. The overview stack controls which overviews are drawn and the
 * order they are drawn in.
 * \note added in QGIS 2.5
 * \see QgsComposerMapOverview
 */
class CORE_EXPORT QgsComposerMapOverviewStack : public QgsComposerMapItemStack
{
  public:

    /**Constructor for QgsComposerMapOverviewStack.
     * @param map QgsComposerMap the overview stack is attached to
    */
    QgsComposerMapOverviewStack( QgsComposerMap* map );

    virtual ~QgsComposerMapOverviewStack();

    /**Adds a new map overview to the stack and takes ownership of the overview.
     * The overview will be added to the end of the stack, and rendered
     * above any existing map overviews already present in the stack.
     * @param overview QgsComposerMapOverview to add to the stack
     * @note after adding a overview to the stack, update()
     * should be called for the QgsComposerMap to prevent rendering artifacts
     * @see removeOverview
    */
    void addOverview( QgsComposerMapOverview* overview );

    /**Removes an overview from the stack and deletes the corresponding QgsComposerMapOverview
     * @param overviewId id for the QgsComposerMapOverview to remove
     * @note after removing an overview from the stack, update()
     * should be called for the QgsComposerMap to prevent rendering artifacts
     * @see addOverview
    */
    void removeOverview( const QString& overviewId );

    /**Moves an overview up the stack, causing it to be rendered above other overviews
     * @param overviewId id for the QgsComposerMapOverview to move up
     * @note after moving an overview within the stack, update() should be
     * called for the QgsComposerMap to redraw the map with the new overview stack order
     * @see moveOverviewDown
    */
    void moveOverviewUp( const QString& overviewId );

    /**Moves an overview down the stack, causing it to be rendered below other overviews
     * @param overviewId id for the QgsComposerMapOverview to move down
     * @note after moving an overview within the stack, update() should be
     * called for the QgsComposerMap to redraw the map with the new overview stack order
     * @see moveOverviewUp
    */
    void moveOverviewDown( const QString& overviewId );

    /**Returns a const reference to an overview within the stack
     * @param overviewId id for the QgsComposerMapOverview to find
     * @returns const reference to overview, if found
     * @see overview
    */
    const QgsComposerMapOverview* constOverview( const QString& overviewId ) const;

    /**Returns a reference to an overview within the stack
     * @param overviewId id for the QgsComposerMapOverview to find
     * @returns reference to overview if found
     * @see constOverview
    */
    QgsComposerMapOverview* overview( const QString& overviewId ) const;

    /**Returns a reference to an overview within the stack
     * @param index overview position in the stack
     * @returns reference to overview if found
     * @see constOverview
    */
    QgsComposerMapOverview* overview( const int index ) const;

    /**Returns a reference to an overview within the stack
     * @param idx overview position in the stack
     * @returns reference to overview if found
     * @see constOverview
     * @see overview
    */
    QgsComposerMapOverview &operator[]( int idx );

    /**Returns a list of QgsComposerMapOverviews contained by the stack
     * @returns list of overviews
    */
    QList< QgsComposerMapOverview* > asList() const;

    /**Sets the overview stack's state from a DOM document
     * @param elem is DOM node corresponding to a 'ComposerMap' tag
     * @param doc DOM document
     * @returns true if read was successful
     * @see writeXML
     */
    bool readXML( const QDomElement& elem, const QDomDocument& doc ) override;

};

/**\ingroup MapComposer
 * \class QgsComposerMapOverview
 * \brief An individual overview which is drawn above the map content in a
 * QgsComposerMap, and shows the extent of another QgsComposerMap.
 * \note added in QGIS 2.5
 * \see QgsComposerMapOverviewStack
 */
class CORE_EXPORT QgsComposerMapOverview : public QgsComposerMapItem
{
    Q_OBJECT

  public:

    /**Constructor for QgsComposerMapOverview.
     * @param name friendly display name for overview
     * @param map QgsComposerMap the overview is attached to
    */
    QgsComposerMapOverview( const QString& name, QgsComposerMap* map );

    virtual ~QgsComposerMapOverview();

    /**Draws an overview
     * @param painter destination QPainter
     */
    void draw( QPainter* painter ) override;

    /**Stores overview state in DOM element
     * @param elem is DOM element corresponding to a 'ComposerMap' tag
     * @param doc DOM document
     * @see readXML
    */
    bool writeXML( QDomElement& elem, QDomDocument & doc ) const override;

    /**Sets overview state from a DOM document
     * @param itemElem is DOM node corresponding to a 'ComposerMapOverview' tag
     * @param doc is DOM document
     * @see writeXML
    */
    bool readXML( const QDomElement& itemElem, const QDomDocument& doc ) override;

    bool usesAdvancedEffects() const override;

    /**Sets overview frame map.
     * @param mapId source map id. -1 disables the overview frame
     * @see frameMapId
    */
    void setFrameMap( const int mapId );

    /**Returns id of source map.
     * @returns source map id, or -1 if no source map set
    */
    int frameMapId() const { return mFrameMapId; }

    /**Sets the fill symbol used for drawing the overview extent.
     * @param symbol fill symbol for overview
     * @see frameSymbol
    */
    void setFrameSymbol( QgsFillSymbolV2* symbol );

    /**Gets the fill symbol used for drawing the overview extent.
     * @returns fill symbol for overview
     * @see setFrameSymbol
    */
    QgsFillSymbolV2* frameSymbol() { return mFrameSymbol; }

    /**Gets the fill symbol used for drawing the overview extent.
     * @returns fill symbol for overview
     * @see setFrameSymbol
    */
    const QgsFillSymbolV2* frameSymbol() const { return mFrameSymbol; }

    /**Retrieves the blending mode used for drawing the overview.
     * @returns blending mode for overview
     * @see setBlendMode
    */
    QPainter::CompositionMode blendMode() const { return mBlendMode; }

    /**Sets the blending mode used for drawing the overview.
     * @param blendMode blending mode for overview
     * @see blendMode
    */
    void setBlendMode( const QPainter::CompositionMode blendMode );

    /**Returns whether the overview frame is inverted, ie, whether the shaded area is drawn outside
     * the extent of the overview map.
     * @returns true if overview frame is inverted
     * @see setInverted
    */
    bool inverted() const { return mInverted; }

    /**Sets whether the overview frame is inverted, ie, whether the shaded area is drawn outside
     * the extent of the overview map.
     * @param inverted set to true if overview frame is to be inverted
     * @see inverted
    */
    void setInverted( const bool inverted );

    /**Returns whether the extent of the map is forced to center on the overview
     * @returns true if map will be centered on overview
     * @see setCentered
    */
    bool centered() const { return mCentered; }

    /**Sets whether the extent of the map is forced to center on the overview
     * @param centered set to true if map will be centered on overview
     * @see centered
    */
    void setCentered( const bool centered );

    /**Reconnects signals for overview map, so that overview correctly follows changes to source
     * map's extent
    */
    void connectSignals();

  public slots:

    /**Handles recentering of the map and redrawing of the map's overview
    */
    void overviewExtentChanged();

  private:

    QgsComposerMapOverview(); //forbidden

    /**Id of map which displays its extent rectangle into this composer map (overview map functionality). -1 if not present*/
    int mFrameMapId;

    /**Drawing style for overview farme*/
    QgsFillSymbolV2* mFrameSymbol;

    /**Blend mode for overview*/
    QPainter::CompositionMode mBlendMode;

    /**True if overview is inverted*/
    bool mInverted;

    /**True if map is centered on overview*/
    bool mCentered;

    /**Creates default overview symbol*/
    void createDefaultFrameSymbol();

};

#endif // QGSCOMPOSERMAPOVERVIEW_H
