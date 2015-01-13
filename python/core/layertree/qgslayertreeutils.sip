/**
 * Assorted functions for dealing with layer trees.
 *
 * @note added in 2.4
 */
class QgsLayerTreeUtils
{
%TypeHeaderCode
#include <qgslayertreeutils.h>
%End

  public:

    //! Try to load layer tree from \verbatim <legend> \endverbatim tag from project files from QGIS 2.2 and below
    static bool readOldLegend( QgsLayerTreeGroup* root, const QDomElement& legendElem );
    //! Try to load custom layer order from \verbatim <legend> \endverbatim tag from project files from QGIS 2.2 and below
    static bool readOldLegendLayerOrder( const QDomElement& legendElem, bool& hasCustomOrder, QStringList& order );
    //! Return \verbatim <legend> \endverbatim tag used in QGIS 2.2 and below
    static QDomElement writeOldLegend( QDomDocument& doc, QgsLayerTreeGroup* root, bool hasCustomOrder, const QStringList& order );

    //! Convert Qt::CheckState to QString
    static QString checkStateToXml( Qt::CheckState state );
    //! Convert QString to Qt::CheckState
    static Qt::CheckState checkStateFromXml( QString txt );

    //! Return true if any of the layers is editable
    static bool layersEditable( const QList<QgsLayerTreeLayer*>& layerNodes );
    //! Return true if any of the layers is modified
    static bool layersModified( const QList<QgsLayerTreeLayer*>& layerNodes );

    //! Remove layer nodes that refer to invalid layers
    static void removeInvalidLayers( QgsLayerTreeGroup* group );

    //! Remove subtree of embedded groups and replaces it with a custom property embedded-visible-layers
    static void replaceChildrenOfEmbeddedGroups( QgsLayerTreeGroup* group );


    //! get invisible layers
    static QStringList invisibleLayerList( QgsLayerTreeNode *node );
};
