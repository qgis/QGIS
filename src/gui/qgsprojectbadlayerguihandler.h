#ifndef QGSPROJECTBADLAYERGUIHANDLER_H
#define QGSPROJECTBADLAYERGUIHANDLER_H

#include "qgsproject.h"

/** \ingroup gui
  Handler for missing layers within project.

  Gives user a chance to select path to the missing layers.

  @note added in 1.4
 */
class GUI_EXPORT QgsProjectBadLayerGuiHandler : public QObject, public QgsProjectBadLayerHandler
{
    Q_OBJECT

  public:
    QgsProjectBadLayerGuiHandler();

    /** implementation of the handler */
    virtual void handleBadLayers( QList<QDomNode> layers, QDomDocument projectDom );

    /** Flag to store the Ignore button press of MessageBox used by QgsLegend */
    static bool mIgnore;

  protected:

    //! file data representation
    enum DataType { IS_VECTOR, IS_RASTER, IS_BOGUS };

    //! the three flavors for data
    enum ProviderType { IS_FILE, IS_DATABASE, IS_URL, IS_Unknown };


    /** returns data type associated with the given QgsProject file Dom node

      The Dom node should represent the state associated with a specific layer.
      */
    DataType dataType( QDomNode & layerNode );

    /** return the data source for the given layer

      The QDomNode is a QgsProject Dom node corresponding to a map layer state.

      Essentially dumps datasource tag.
    */
    QString dataSource( QDomNode & layerNode );

    /** return the physical storage type associated with the given layer

      The QDomNode is a QgsProject Dom node corresponding to a map layer state.

      If the provider tag is "ogr", then it's a file type.

      However, if the layer is a raster, then there won't be a
      provider tag.  It will always have an associated file.

      If the layer doesn't fall into either of the previous two categories, then
      it's either a database or URL.  If the datasource tag has "url=", then it's
      URL based and if it has "dbname=">, then the layer data is in a database.
    */
    ProviderType providerType( QDomNode & layerNode );

    /** set the datasource element to the new value */
    void setDataSource( QDomNode & layerNode, QString const & dataSource );

    /** this is used to locate files that have moved or otherwise are missing */
    bool findMissingFile( QString const & fileFilters, QDomNode & layerNode );

    /** find relocated data source for the given layer

      This QDom object represents a QgsProject node that maps to a specific layer.

      @param fileFilters file filters to use
      @param constLayerNode QDom node containing layer project information

      @todo

      XXX Only implemented for file based layers.  It will need to be extended for
      XXX other data source types such as databases.
    */
    bool findLayer( QString const & fileFilters, QDomNode const & constLayerNode );

    /** find relocated data sources for given layers

      These QDom objects represent QgsProject nodes that map to specific layers.
    */
    void findLayers( QString const & fileFilters, QList<QDomNode> const & layerNodes );

};

#endif // QGSPROJECTBADLAYERGUIHANDLER_H
