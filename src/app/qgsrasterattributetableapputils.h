/***************************************************************************
  qgsrasterattributetableapputils.h - QgsRasterAttributeTableAppUtils

 ---------------------
 begin                : 3.11.2022
 copyright            : (C) 2022 by Alessandro Pasotti
 email                : elpaso at itopen dot it
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSRASTERATTRIBUTETABLEAPPUTILS_H
#define QGSRASTERATTRIBUTETABLEAPPUTILS_H

class QgsLayerTreeView;
class QgsMessageBar;

/**
 * \brief The QgsRasterAttributeTableAppUtils class contains application utilities to manage attribute tables
 * \since QGIS 3.30
 */
struct QgsRasterAttributeTableAppUtils
{

  /**
   * Open the Raster Attribute Table for the raster layer.

   * Only works on raster layers.
   */
  static void openRasterAttributeTable( QgsLayerTreeView *treeView );

  /**
   * Creates a new Raster Attribute Table from the raster layer renderer if the
   * renderer supports it.
   *
   * Only works on raster layers.
   */
  static void createRasterAttributeTable( QgsLayerTreeView *treeView, QgsMessageBar *messageBar );

  /**
   * Loads a Raster Attribute Table from a VAT.DBF file.
   *
   * Only works on raster layers.
   *
   */
  static void loadRasterAttributeTableFromFile( QgsLayerTreeView *treeView, QgsMessageBar *messageBar );

};

#endif // QGSRASTERATTRIBUTETABLEAPPUTILS_H
