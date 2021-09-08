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
 * \class QgsCreateAnnotationItemMapTool
 * \ingroup gui
 *
 * \brief A base class for map tools which create annotation items.
 *
 * Clients should connect to the map tool's itemCreated() signal, and call the
 * takeCreatedItem() implementation to take ownership of the newly created item
 * whenever this signal is emitted.
 *
 * \since QGIS 3.22
*/
class GUI_EXPORT QgsCreateAnnotationItemMapTool: public QgsMapToolAdvancedDigitizing
{
    Q_OBJECT

  public:

    /**
     * Constructor for QgsCreateAnnotationItemMapTool.
     */
    QgsCreateAnnotationItemMapTool( QgsMapCanvas *canvas, QgsAdvancedDigitizingDockWidget *cadDockWidget );

    /**
     * Takes the newly created item from the tool, transferring ownership to the caller.
     *
     * Subclasses must implement this method, and ensure that they emit the itemCreated() signal whenever
     * they have a created item ready for clients to take.
     */
    virtual QgsAnnotationItem *takeCreatedItem() = 0 SIP_TRANSFERBACK;

  signals:

    /**
     * Emitted by the tool when a new annotation item has been created.
     *
     * Clients should connect to this signal and call takeCreatedItem() to take the newly created item from the map tool.
     */
    void itemCreated();

  protected:

    /**
     * Returns the target layer for newly created items.
     */
    QgsAnnotationLayer *targetLayer();

};

#endif // QGSCREATEANNOTATIONITEMMAPTOOL_H
