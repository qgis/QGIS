/***************************************************************************
                         qgslayoutitemmapoverview.h
                         --------------------
    begin                : October 2017
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

#ifndef QgsLayoutItemMapOVERVIEW_H
#define QgsLayoutItemMapOVERVIEW_H

#include "qgis_core.h"
#include "qgis_sip.h"
#include "qgis.h"
#include "qgslayoutitemmapitem.h"
#include "qgssymbol.h"
#include <QString>
#include <QObject>
#include <QPainter>

class QDomDocument;
class QDomElement;
class QgsLayoutItemMapOverview;

/**
 * \ingroup core
 * \class QgsLayoutItemMapOverviewStack
 * \brief A collection of overviews which are drawn above the map content in a
 * QgsLayoutItemMap. The overview stack controls which overviews are drawn and the
 * order they are drawn in.
 * \see QgsLayoutItemMapOverview
 * \since QGIS 3.0
 */
class CORE_EXPORT QgsLayoutItemMapOverviewStack : public QgsLayoutItemMapItemStack
{
  public:

    /**
     * Constructor for QgsLayoutItemMapOverviewStack, attached to the specified
     * \a map.
     */
    QgsLayoutItemMapOverviewStack( QgsLayoutItemMap *map );

    /**
     * Adds a new map \a overview to the stack and takes ownership of the overview.
     * The overview will be added to the end of the stack, and rendered
     * above any existing map overviews already present in the stack.
     * \note After adding a overview to the stack, update()
     * should be called for the QgsLayoutItemMap to prevent rendering artifacts.
     * \see removeOverview()
     */
    void addOverview( QgsLayoutItemMapOverview *overview SIP_TRANSFER );

    /**
     * Removes an overview with matching overviewId from the stack and deletes the corresponding QgsLayoutItemMapOverview
     * \note After removing an overview from the stack, update()
     * should be called for the QgsLayoutItemMap to prevent rendering artifacts.
     * \see addOverview()
     */
    void removeOverview( const QString &overviewId );

    /**
     * Moves an overview with matching overviewId up the stack, causing it to be rendered above other overviews.
     * \note After moving an overview within the stack, update() should be
     * called for the QgsLayoutItemMap to redraw the map with the new overview stack order.
     * \see moveOverviewDown()
     */
    void moveOverviewUp( const QString &overviewId );

    /**
     * Moves an overview with matching overviewId down the stack, causing it to be rendered below other overviews.
     * \note After moving an overview within the stack, update() should be
     * called for the QgsLayoutItemMap to redraw the map with the new overview stack order.
     * \see moveOverviewUp()
     */
    void moveOverviewDown( const QString &overviewId );

    /**
     * Returns a reference to an overview with matching overviewId within the stack.
     */
    QgsLayoutItemMapOverview *overview( const QString &overviewId ) const;

    /**
     * Returns a reference to an overview at the specified \a index within the stack.
     */
    QgsLayoutItemMapOverview *overview( int index ) const;

    /**
     * Returns a reference to an overview at the specified \a index within the stack.
     * \see overview()
     */
    QgsLayoutItemMapOverview &operator[]( int index );

    /**
     * Returns a list of QgsLayoutItemMapOverviews contained by the stack.
     */
    QList< QgsLayoutItemMapOverview * > asList() const;
    bool readXml( const QDomElement &elem, const QDomDocument &doc, const QgsReadWriteContext &context ) override;

    /**
     * Alters the list of map \a layers which will be rendered for the link map item, inserting
     * temporary layers which represent overview extents as required.
     *
     * \since QGIS 3.6
     */
    QList< QgsMapLayer * > modifyMapLayerList( const QList< QgsMapLayer * > &layers );

};

/**
 * \ingroup core
 * \class QgsLayoutItemMapOverview
 * \brief An individual overview which is drawn above the map content in a
 * QgsLayoutItemMap, and shows the extent of another QgsLayoutItemMap.
 * \see QgsLayoutItemMapOverviewStack
 * \since QGIS 3.0
 */
class CORE_EXPORT QgsLayoutItemMapOverview : public QgsLayoutItemMapItem
{
    Q_OBJECT

  public:

    /**
     * Constructor for QgsLayoutItemMapOverview.
     * \param name friendly display name for overview
     * \param map QgsLayoutItemMap the overview is attached to
     */
    QgsLayoutItemMapOverview( const QString &name, QgsLayoutItemMap *map );
    ~QgsLayoutItemMapOverview() override;

    void draw( QPainter *painter ) override;
    bool writeXml( QDomElement &elem, QDomDocument &doc, const QgsReadWriteContext &context ) const override;
    bool readXml( const QDomElement &itemElem, const QDomDocument &doc, const QgsReadWriteContext &context ) override;
    void finalizeRestoreFromXml() override;
    bool usesAdvancedEffects() const override;

    /**
     * Sets the \a map to show the overview extent of.
     * \see linkedMap()
     */
    void setLinkedMap( QgsLayoutItemMap *map );

    /**
     * Returns the source map to show the overview extent of.
     * \see setLinkedMap()
     */
    QgsLayoutItemMap *linkedMap();

    /**
     * Sets the fill \a symbol used for drawing the overview extent. Ownership
     * is transferred to the overview.
     * \see frameSymbol()
     */
    void setFrameSymbol( QgsFillSymbol *symbol SIP_TRANSFER );

    /**
     * Returns the fill symbol used for drawing the overview extent.
     * \see setFrameSymbol()
     */
    QgsFillSymbol *frameSymbol();

    /**
     * Returns the fill symbol used for drawing the overview extent.
     * \see setFrameSymbol()
     * \note not available in Python bindings
     */
    const QgsFillSymbol *frameSymbol() const; SIP_SKIP

    /**
     * Retrieves the blending mode used for drawing the overview.
     * \see setBlendMode()
     */
    QPainter::CompositionMode blendMode() const { return mBlendMode; }

    /**
     * Sets the blending \a mode used for drawing the overview.
     * \see blendMode()
     */
    void setBlendMode( QPainter::CompositionMode mode );

    /**
     * Returns whether the overview frame is inverted, ie, whether the shaded area is drawn outside
     * the extent of the overview map.
     * \see setInverted()
     */
    bool inverted() const { return mInverted; }

    /**
     * Sets whether the overview frame is \a inverted, ie, whether the shaded area is drawn outside
     * the extent of the overview map.
     * \see inverted()
     */
    void setInverted( bool inverted );

    /**
     * Returns whether the extent of the map is forced to center on the overview.
     * \see setCentered()
     */
    bool centered() const { return mCentered; }

    /**
     * Sets whether the extent of the map is forced to center on the overview
     * \see centered()
     */
    void setCentered( bool centered );

    /**
     * Reconnects signals for overview map, so that overview correctly follows changes to source
     * map's extent.
     */
    void connectSignals();

    /**
     * Returns a vector layer to render as part of the QgsLayoutItemMap render, containing
     * a feature representing the overview extent (and with an appropriate renderer set matching
     * the overview's frameSymbol() ).
     *
     * Ownership of the layer remain with the overview item.
     *
     * \since QGIS 3.6
     */
    QgsVectorLayer *asMapLayer();

  public slots:

    /**
     * Handles recentering of the map and redrawing of the map's overview
     */
    void overviewExtentChanged();

  private:

    QgsLayoutItemMapOverview() = delete;

    QString mFrameMapUuid;
    QPointer< QgsLayoutItemMap > mFrameMap;

    //! Drawing style for overview farme
    std::unique_ptr< QgsFillSymbol > mFrameSymbol;

    //! Blend mode for overview
    QPainter::CompositionMode mBlendMode = QPainter::CompositionMode_SourceOver;

    //! True if overview is inverted
    bool mInverted = false;

    //! True if map is centered on overview
    bool mCentered = false;

    std::unique_ptr< QgsVectorLayer > mExtentLayer;

    //! Creates default overview symbol
    void createDefaultFrameSymbol();

};

#endif // QgsLayoutItemMapOVERVIEW_H
