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

#include <QString>

using namespace Qt::StringLiterals;

/**
 * \ingroup core
 * \class QgsSettingsTree
 * \brief Holds the tree structure for the settings in QGIS core.
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
    static inline QgsSettingsTreeNode *sTreeApp = treeRoot()->createChildNode( u"app"_s );
    static inline QgsSettingsTreeNode *sTreeAttributeForm = treeRoot()->createChildNode( u"attribute-form"_s );
    static inline QgsSettingsTreeNode *sTreeBrowser = treeRoot()->createChildNode( u"browser"_s );
    static inline QgsSettingsTreeNode *sTreeColors = treeRoot()->createChildNode( u"colors"_s );
    static inline QgsSettingsTreeNode *sTreeColorWidgets = treeRoot()->createChildNode( u"color-widgets"_s );
    static inline QgsSettingsTreeNode *sTreeConnections = treeRoot()->createChildNode( u"connections"_s );
    static inline QgsSettingsTreeNode *sTreeCore = treeRoot()->createChildNode( u"core"_s );
    static inline QgsSettingsTreeNode *sTreeCrs = treeRoot()->createChildNode( u"crs"_s );
    static inline QgsSettingsTreeNode *sTreeDigitizing = treeRoot()->createChildNode( u"digitizing"_s );
    static inline QgsSettingsTreeNode *sTreeElevationProfile = treeRoot()->createChildNode( u"elevation-profile"_s );
    static inline QgsSettingsTreeNode *sTreeFonts = treeRoot()->createChildNode( u"fonts"_s );
    static inline QgsSettingsTreeNode *sTreeGeometryValidation = treeRoot()->createChildNode( u"geometry_validation"_s );
    static inline QgsSettingsTreeNode *sTreeGps = treeRoot()->createChildNode( u"gps"_s );
    static inline QgsSettingsTreeNode *sTreeGradientEditor = treeRoot()->createChildNode( u"gradient-editor"_s );
    static inline QgsSettingsTreeNode *sTreeGui = treeRoot()->createChildNode( u"gui"_s );
    static inline QgsSettingsTreeNode *sTreeHistogram = treeRoot()->createChildNode( u"histogram"_s );
    static inline QgsSettingsTreeNode *sTreeIdentify = treeRoot()->createChildNode( u"identify"_s );
    static inline QgsSettingsTreeNode *sTreeLayerTree = treeRoot()->createChildNode( u"layer-tree"_s );
    static inline QgsSettingsTreeNode *sTreeLayout = treeRoot()->createChildNode( u"layout"_s );
    static inline QgsSettingsTreeNode *sTreeLocale = treeRoot()->createChildNode( u"locale"_s );
    static inline QgsSettingsTreeNode *sTreeMap = treeRoot()->createChildNode( u"map"_s );
    static inline QgsSettingsTreeNode *sTreeNetwork = treeRoot()->createChildNode( u"network"_s );
    static inline QgsSettingsTreeNode *sTreeQgis = treeRoot()->createChildNode( u"qgis"_s );
    static inline QgsSettingsTreeNode *sTreePlugins = treeRoot()->createChildNode( u"plugins"_s );
    static inline QgsSettingsTreeNode *sTreeProcessing = treeRoot()->createChildNode( u"processing"_s );
    static inline QgsSettingsTreeNode *sTreeProfile = treeRoot()->createChildNode( u"profile"_s );
    static inline QgsSettingsTreeNode *sTreeProject = treeRoot()->createChildNode( u"project"_s );
    static inline QgsSettingsTreeNode *sTreeProxy = treeRoot()->createChildNode( u"proxy"_s );
    static inline QgsSettingsTreeNode *sTreeRaster = treeRoot()->createChildNode( u"raster"_s );
    static inline QgsSettingsTreeNode *sTreeRendering = treeRoot()->createChildNode( u"rendering"_s );
    static inline QgsSettingsTreeNode *sTreeSvg = treeRoot()->createChildNode( u"svg"_s );
    static inline QgsSettingsTreeNode *sTreeWms = treeRoot()->createChildNode( u"wms"_s );
    static inline QgsSettingsTreeNode *sTreeMeasure = treeRoot()->createChildNode( u"measure"_s );
    static inline QgsSettingsTreeNode *sTreeAnnotations = treeRoot()->createChildNode( u"annotations"_s );
    static inline QgsSettingsTreeNode *sTreeNetworkCache = treeRoot()->createChildNode( u"cache"_s );
    static inline QgsSettingsTreeNode *sTreeAttributeTable = treeRoot()->createChildNode( u"attribute-table"_s );
    static inline QgsSettingsTreeNode *sTreeWindowState = sTreeGui->createChildNode( u"window-state"_s );
    static inline QgsSettingsTreeNode *sTreeAuthentication = treeRoot()->createChildNode( u"authentication"_s );
    static inline QgsSettingsTreeNode *sTreeDatabase = treeRoot()->createChildNode( u"database"_s );
    static inline QgsSettingsTreeNode *sTree3DMap = treeRoot()->createChildNode( u"3dmap"_s );
    static inline QgsSettingsTreeNode *sTreeRasterHistogram = sTreeRaster->createChildNode( u"histogram"_s );
    static inline QgsSettingsTreeNode *sTreeCad = sTreeDigitizing->createChildNode( u"cad"_s );
    static inline QgsSettingsTreeNode *sTreeSqlite3 = sTreeDatabase->createChildNode( u"sqlite3"_s );
    static inline QgsSettingsTreeNode *sTreeLayoutGrid = sTreeLayout->createChildNode( u"grid"_s );

#endif

    /**
     * Returns the tree node at the given \a key
     * \note For Plugins, use createPluginTreeNode() to create nodes for plugin settings.
     */
    static const QgsSettingsTreeNode *node( const QString &key ) { return treeRoot()->childNode( key ); }

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
