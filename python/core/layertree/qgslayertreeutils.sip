/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/layertree/qgslayertreeutils.h                               *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/





class QgsLayerTreeUtils
{
%Docstring
 Assorted functions for dealing with layer trees.

.. versionadded:: 2.4
%End

%TypeHeaderCode
#include "qgslayertreeutils.h"
%End
  public:

    static bool readOldLegend( QgsLayerTreeGroup *root, const QDomElement &legendElem );
%Docstring
Try to load layer tree from \verbatim <legend> \endverbatim tag from project files from QGIS 2.2 and below
 :rtype: bool
%End
    static bool readOldLegendLayerOrder( const QDomElement &legendElem, bool &hasCustomOrder, QStringList &order );
%Docstring
Try to load custom layer order from \verbatim <legend> \endverbatim tag from project files from QGIS 2.2 and below
 :rtype: bool
%End
    static QDomElement writeOldLegend( QDomDocument &doc, QgsLayerTreeGroup *root, bool hasCustomOrder, const QList<QgsMapLayer *> &order );
%Docstring
Return \verbatim <legend> \endverbatim tag used in QGIS 2.2 and below
 :rtype: QDomElement
%End

    static QString checkStateToXml( Qt::CheckState state );
%Docstring
Convert Qt.CheckState to QString
 :rtype: str
%End
    static Qt::CheckState checkStateFromXml( const QString &txt );
%Docstring
Convert QString to Qt.CheckState
 :rtype: Qt.CheckState
%End

    static bool layersEditable( const QList<QgsLayerTreeLayer *> &layerNodes );
%Docstring
Return true if any of the layers is editable
 :rtype: bool
%End
    static bool layersModified( const QList<QgsLayerTreeLayer *> &layerNodes );
%Docstring
Return true if any of the layers is modified
 :rtype: bool
%End

    static void removeInvalidLayers( QgsLayerTreeGroup *group );
%Docstring
Remove layer nodes that refer to invalid layers
%End

    static void replaceChildrenOfEmbeddedGroups( QgsLayerTreeGroup *group );
%Docstring
Remove subtree of embedded groups and replaces it with a custom property embedded-visible-layers
%End

    static void updateEmbeddedGroupsProjectPath( QgsLayerTreeGroup *group, const QgsProject *project );
%Docstring
.. note::

   not available in Python bindings
%End

    static QStringList invisibleLayerList( QgsLayerTreeNode *node );
%Docstring
get invisible layers
 :rtype: list of str
%End

    static void setLegendFilterByExpression( QgsLayerTreeLayer &layer, const QString &expr, bool enabled = true );
%Docstring
Set the expression filter of a legend layer
%End
    static QString legendFilterByExpression( const QgsLayerTreeLayer &layer, bool *enabled = 0 );
%Docstring
Return the expression filter of a legend layer
 :rtype: str
%End
    static bool hasLegendFilterExpression( const QgsLayerTreeGroup &group );
%Docstring
Test if one of the layers in a group has an expression filter
 :rtype: bool
%End

    static QgsLayerTreeLayer *insertLayerBelow( QgsLayerTreeGroup *group, const QgsMapLayer *refLayer, QgsMapLayer *layerToInsert );
%Docstring
 Insert a QgsMapLayer just below another one
 \param group the tree group where layers are (can be the root group)
 \param refLayer the reference layer
 \param layerToInsert the new layer to insert just below the reference layer
 :return: the new tree layer
 :rtype: QgsLayerTreeLayer
%End
};

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/layertree/qgslayertreeutils.h                               *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
