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

#include "qgsproject.h"

/** \ingroup gui
  Handler for missing layers within project.

  Gives user a chance to select path to the missing layers.
 */
class GUI_EXPORT QgsProjectBadLayerGuiHandler : public QObject, public QgsProjectBadLayerHandler
{
    Q_OBJECT

  public:
    QgsProjectBadLayerGuiHandler();

    /** Implementation of the handler */
    virtual void handleBadLayers( QList<QDomNode> layers, QDomDocument projectDom ) override;

    /** Flag to store the Ignore button press of MessageBox used by QgsLegend */
    static bool mIgnore;

  protected:

    //! file data representation
    enum DataType { IS_VECTOR, IS_RASTER, IS_BOGUS };

    //! the three flavors for data
    enum ProviderType { IS_FILE, IS_DATABASE, IS_URL, IS_Unknown };


    /** Returns data type associated with the given QgsProject file Dom node

      The Dom node should represent the state associated with a specific layer.
      */
    DataType dataType( QDomNode & layerNode );

    /** Return the data source for the given layer

      The QDomNode is a QgsProject Dom node corresponding to a map layer state.

      Essentially dumps datasource tag.
    */
    QString dataSource( QDomNode & layerNode );

    /** Return the physical storage type associated with the given layer

      The QDomNode is a QgsProject Dom node corresponding to a map layer state.

      If the provider tag is "ogr", then it's a file type.

      However, if the layer is a raster, then there won't be a
      provider tag.  It will always have an associated file.

      If the layer doesn't fall into either of the previous two categories, then
      it's either a database or URL.  If the datasource tag has "url=", then it's
      URL based and if it has "dbname=">, then the layer data is in a database.
    */
    ProviderType providerType( QDomNode & layerNode );

    /** Set the datasource element to the new value */
    void setDataSource( QDomNode & layerNode, const QString &dataSource );

    /** This is used to locate files that have moved or otherwise are missing */
    bool findMissingFile( const QString &fileFilters, QDomNode &layerNode );

    /** Find relocated data source for the given layer

      This QDom object represents a QgsProject node that maps to a specific layer.

      @param fileFilters file filters to use
      @param constLayerNode QDom node containing layer project information

      @todo

      XXX Only implemented for file based layers.  It will need to be extended for
      XXX other data source types such as databases.
    */
    bool findLayer( const QString &fileFilters, const QDomNode &constLayerNode );

    /** Find relocated data sources for given layers

      These QDom objects represent QgsProject nodes that map to specific layers.
    */
    void findLayers( const QString &fileFilters, const QList<QDomNode> &layerNodes );

};

#endif // QGSPROJECTBADLAYERGUIHANDLER_H
