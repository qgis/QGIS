/***************************************************************************
                             qgscreateannotationitemmaptool.h
                             ------------------------
    Date                 : September 2021
    Copyright            : (C) 2021 Nyall Dawson
    Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSCREATEANNOTATIONITEMMAPTOOL_H
#define QGSCREATEANNOTATIONITEMMAPTOOL_H

#include "qgis_gui.h"
#include "qgis_sip.h"
#include "qgsmaptooladvanceddigitizing.h"

class QgsAnnotationItem;
class QgsAnnotationLayer;

/**
 * \class QgsCreateAnnotationItemMapToolHandler
 * \ingroup gui
 *
 * \brief A handler object for map tools which create annotation items.
 *
 * This object is designed to be used by map tools which implement the
 * QgsCreateAnnotationItemMapToolInterface, following the composition pattern.
 *
 * Clients should connect to the handler's itemCreated() signal, and call the
 * takeCreatedItem() implementation to take ownership of the newly created item
 * whenever this signal is emitted.
 *
 * \since QGIS 3.22
*/
class GUI_EXPORT QgsCreateAnnotationItemMapToolHandler : public QObject
{
    Q_OBJECT

  public:

    /**
     * Constructor for QgsCreateAnnotationItemMapToolHandler, with the specified \a parent object.
     */
    QgsCreateAnnotationItemMapToolHandler( QgsMapCanvas *canvas, QgsAdvancedDigitizingDockWidget *cadDockWidget, QObject *parent = nullptr );

    ~QgsCreateAnnotationItemMapToolHandler() override;

    /**
     * Takes the newly created item from the tool, transferring ownership to the caller.
     */
    QgsAnnotationItem *takeCreatedItem() SIP_TRANSFERBACK;

    /**
     * Returns the target layer for newly created items.
     */
    QgsAnnotationLayer *targetLayer();

    /**
     * Pushes a created \a item to the handler.
     *
     * Ownership of \a item is transferred to the handler.
     *
     * Calling this method causes the object to emit the itemCreated() signal, and queue the item
     * ready for collection via a call to takeCreatedItem().
     */
    void pushCreatedItem( QgsAnnotationItem *item SIP_TRANSFER );

  signals:

    /**
     * Emitted by the tool when a new annotation item has been created.
     *
     * Clients should connect to this signal and call takeCreatedItem() to take the newly created item from the map tool.
     */
    void itemCreated();

  private:

    QgsMapCanvas *mMapCanvas = nullptr;
    QgsAdvancedDigitizingDockWidget *mCadDockWidget = nullptr;
    std::unique_ptr< QgsAnnotationItem > mCreatedItem;

};

/**
 * \class QgsCreateAnnotationItemMapToolInterface
 * \ingroup gui
 *
 * \brief An interface for map tools which create annotation items.
 *
 * Clients should connect to the map tool's itemCreated() signal, and call the
 * takeCreatedItem() implementation to take ownership of the newly created item
 * whenever this signal is emitted.
 *
 * \since QGIS 3.22
*/
class GUI_EXPORT QgsCreateAnnotationItemMapToolInterface
{
  public:

    virtual ~QgsCreateAnnotationItemMapToolInterface() = default;

    /**
     * Returns the handler object for the map tool.
     */
    virtual QgsCreateAnnotationItemMapToolHandler *handler() = 0;

    /**
     * Returns a reference to the associated map tool.
     */
    virtual QgsMapTool *mapTool() = 0;
};

#endif // QGSCREATEANNOTATIONITEMMAPTOOL_H
