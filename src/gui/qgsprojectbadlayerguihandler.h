/***************************************************************************
    qgsprojectbadlayerguihandler.h - handle bad layers
    ---------------------
    begin                : December 2009
    copyright            : (C) 2009 by Martin Dobias
    email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSPROJECTBADLAYERGUIHANDLER_H
#define QGSPROJECTBADLAYERGUIHANDLER_H

#include "qgsprojectbadlayerhandler.h"
#include <QObject>

/** \ingroup gui
  Handler for missing layers within project.

  Gives user a chance to select path to the missing layers.
 */
class GUI_EXPORT QgsProjectBadLayerGuiHandler : public QObject, public QgsProjectBadLayerHandler
{
    Q_OBJECT

  public:
    QgsProjectBadLayerGuiHandler();

    virtual void handleBadLayers( const QList<QDomNode>& layers ) override;

    /** Flag to store the Ignore button press of MessageBox used by QgsLegend */
    static bool mIgnore;

  protected:
    /** This is used to locate files that have moved or otherwise are missing */
    bool findMissingFile( const QString& fileFilters, QDomNode& layerNode );

    /**
     * Find relocated data source for the given layer
     *
     * This QDom object represents a QgsProject node that maps to a specific layer.
     *
     * @param fileFilters file filters to use
     * @param constLayerNode QDom node containing layer project information
     *
     * @todo
     *
     * XXX Only implemented for file based layers.  It will need to be extended for
     * XXX other data source types such as databases.
     */
    bool findLayer( const QString& fileFilters, const QDomNode& constLayerNode );

    /**
     * Find relocated data sources for given layers
     * These QDom objects represent QgsProject nodes that map to specific layers.
     */
    void findLayers( const QString& fileFilters, const QList<QDomNode>& layerNodes );

};

#endif // QGSPROJECTBADLAYERGUIHANDLER_H
