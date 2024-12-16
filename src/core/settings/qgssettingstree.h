/***************************************************************************
  qgssettingstree.h
  --------------------------------------
  Date                 : February 2023
  Copyright            : (C) 2023 by Denis Rouzaud
  Email                : denis@opengis.ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSSETTINGSTREE_H
#define QGSSETTINGSTREE_H

#include "qgssettingstreenode.h"

/**
 * \ingroup core
 * \class QgsSettingsTree
 * \brief QgsSettingsTree holds the tree structure for the settings in QGIS core
 *
 * \see QgsSettingsEntryBase
 *
 * \since QGIS 3.30
 */
class CORE_EXPORT QgsSettingsTree
{

  public:

    /**
     * Returns the tree root node for the settings tree
     */
    static QgsSettingsTreeNode *treeRoot();

#ifndef SIP_RUN

    // only create first level here
    static inline QgsSettingsTreeNode *sTreeApp = treeRoot()->createChildNode( QStringLiteral( "app" ) );
    static inline QgsSettingsTreeNode *sTreeConnections = treeRoot()->createChildNode( QStringLiteral( "connections" ) );
    static inline QgsSettingsTreeNode *sTreeCore = treeRoot()->createChildNode( QStringLiteral( "core" ) );
    static inline QgsSettingsTreeNode *sTreeDigitizing = treeRoot()->createChildNode( QStringLiteral( "digitizing" ) );
    static inline QgsSettingsTreeNode *sTreeElevationProfile = treeRoot()->createChildNode( QStringLiteral( "elevation-profile" ) );
    static inline QgsSettingsTreeNode *sTreeFonts = treeRoot()->createChildNode( QStringLiteral( "fonts" ) );
    static inline QgsSettingsTreeNode *sTreeGeometryValidation = treeRoot()->createChildNode( QStringLiteral( "geometry_validation" ) );
    static inline QgsSettingsTreeNode *sTreeGps = treeRoot()->createChildNode( QStringLiteral( "gps" ) );
    static inline QgsSettingsTreeNode *sTreeGui = treeRoot()->createChildNode( QStringLiteral( "gui" ) );
    static inline QgsSettingsTreeNode *sTreeLayerTree = treeRoot()->createChildNode( QStringLiteral( "layer-tree" ) );
    static inline QgsSettingsTreeNode *sTreeLayout = treeRoot()->createChildNode( QStringLiteral( "layout" ) );
    static inline QgsSettingsTreeNode *sTreeLocale = treeRoot()->createChildNode( QStringLiteral( "locale" ) );
    static inline QgsSettingsTreeNode *sTreeMap = treeRoot()->createChildNode( QStringLiteral( "map" ) );
    static inline QgsSettingsTreeNode *sTreeNetwork = treeRoot()->createChildNode( QStringLiteral( "network" ) );
    static inline QgsSettingsTreeNode *sTreeQgis = treeRoot()->createChildNode( QStringLiteral( "qgis" ) );
    static inline QgsSettingsTreeNode *sTreePlugins = treeRoot()->createChildNode( QStringLiteral( "plugins" ) );
    static inline QgsSettingsTreeNode *sTreeProcessing = treeRoot()->createChildNode( QStringLiteral( "processing" ) );
    static inline QgsSettingsTreeNode *sTreeRaster = treeRoot()->createChildNode( QStringLiteral( "raster" ) );
    static inline QgsSettingsTreeNode *sTreeRendering = treeRoot()->createChildNode( QStringLiteral( "rendering" ) );
    static inline QgsSettingsTreeNode *sTreeSvg = treeRoot()->createChildNode( QStringLiteral( "svg" ) );
    static inline QgsSettingsTreeNode *sTreeWms = treeRoot()->createChildNode( QStringLiteral( "wms" ) );
    static inline QgsSettingsTreeNode *sTreeMeasure = treeRoot()->createChildNode( QStringLiteral( "measure" ) );
    static inline QgsSettingsTreeNode *sTreeAnnotations = treeRoot()->createChildNode( QStringLiteral( "annotations" ) );
    static inline QgsSettingsTreeNode *sTreeNetworkCache = treeRoot()->createChildNode( QStringLiteral( "cache" ) );
    static inline QgsSettingsTreeNode *sTreeAttributeTable = treeRoot()->createChildNode( QStringLiteral( "attribute-table" ) );
    static inline QgsSettingsTreeNode *sTreeWindowState = sTreeGui->createChildNode( QStringLiteral( "window-state" ) );

#endif

    /**
     * Returns the tree node at the given \a key
     * \note For Plugins, use createPluginTreeNode() to create nodes for plugin settings.
     */
    static const QgsSettingsTreeNode *node( const QString &key ) {return treeRoot()->childNode( key );}

    /**
     * Creates a settings tree node for the given \a pluginName
     */
    static QgsSettingsTreeNode *createPluginTreeNode( const QString &pluginName );


    /**
     * Unregisters the tree node for the given plugin
     */
    static void unregisterPluginTreeNode( const QString &pluginName );
};

#endif // QGSSETTINGSTREE_H
