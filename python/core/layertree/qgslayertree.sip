/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/layertree/qgslayertree.h                                    *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/



class QgsLayerTree : QgsLayerTreeGroup
{
%Docstring
 Namespace with helper functions for layer tree operations.

 Only generally useful routines should be here. Miscellaneous utility functions for work
 with the layer tree are in QgsLayerTreeUtils class.

.. versionadded:: 3.0
%End

%TypeHeaderCode
#include "qgslayertree.h"
%End
  public:

    static bool isGroup( QgsLayerTreeNode *node );
%Docstring
 Check whether the node is a valid group node

.. versionadded:: 2.4
 :rtype: bool
%End

    static bool isLayer( const QgsLayerTreeNode *node );
%Docstring
 Check whether the node is a valid layer node

.. versionadded:: 2.4
 :rtype: bool
%End




    QgsLayerTree();
%Docstring
 Create a new empty layer tree
%End

    QList<QgsMapLayer *> customLayerOrder() const;
%Docstring
 The order in which layers will be rendered on the canvas.
 Will only be used if the property hasCustomLayerOrder is true.
 If you need the current layer order that is active, prefer using layerOrder().

.. seealso:: setCustomLayerOrder
.. seealso:: layerOrder
.. seealso:: hasCustomLayerOrder

.. versionadded:: 3.0
 :rtype: list of QgsMapLayer
%End

    void setCustomLayerOrder( const QList<QgsMapLayer *> &customLayerOrder );
%Docstring
 The order in which layers will be rendered on the canvas.
 Will only be used if the property hasCustomLayerOrder is true.
 If you need the current layer order that is active, prefer using layerOrder().

.. seealso:: customLayerOrder
.. seealso:: layerOrder
.. seealso:: hasCustomLayerOrder

.. versionadded:: 3.0
%End

    void setCustomLayerOrder( const QStringList &customLayerOrder ) /PyName=setCustomLayerOrderByIds/;
%Docstring
 The order in which layers will be rendered on the canvas.
 Will only be used if the property hasCustomLayerOrder is true.
 If you need the current layer order that is active, prefer using layerOrder().

.. seealso:: customLayerOrder
.. seealso:: layerOrder
.. seealso:: hasCustomLayerOrder

.. versionadded:: 3.0
%End

    QList<QgsMapLayer *> layerOrder() const;
%Docstring
 The order in which layers will be rendered on the canvas.
 Depending on hasCustomLayerOrder, this will return either the override
 customLayerOrder or the layer order derived from the tree.
 This property is read only.

.. seealso:: customLayerOrder

.. versionadded:: 3.0
 :rtype: list of QgsMapLayer
%End

    bool hasCustomLayerOrder() const;
%Docstring
 Determines if the layer order should be derived from the layer tree
 or if a custom override order shall be used instead.

.. seealso:: customLayerOrder

.. versionadded:: 3.0
 :rtype: bool
%End

    void setHasCustomLayerOrder( bool hasCustomLayerOrder );
%Docstring
 Determines if the layer order should be derived from the layer tree
 or if a custom override order shall be used instead.

.. seealso:: setCustomLayerOrder

.. versionadded:: 3.0
%End

    static QgsLayerTree *readXml( QDomElement &element, const QgsReadWriteContext &context );
%Docstring
 Load the layer tree from an XML element.
 It is not required that layers are loaded at this point.
 resolveReferences() needs to be called after loading the layers and
 before using the tree.

.. versionadded:: 3.0
 :rtype: QgsLayerTree
%End

    void readLayerOrderFromXml( const QDomElement &doc );
%Docstring
 Load the layer order from an XML element.
 Make sure that this is only called after the layers are loaded.

.. versionadded:: 3.0
%End

    virtual void writeXml( QDomElement &parentElement, const QgsReadWriteContext &context );

    virtual QgsLayerTree *clone() const /Factory/;

    void clear();
%Docstring
 Clear any information from this layer tree.

.. versionadded:: 3.0
%End

  signals:

    void customLayerOrderChanged();
%Docstring
 Emitted when the custom layer order has changed.

.. versionadded:: 3.0
%End

    void layerOrderChanged();
%Docstring
 Emitted when the layer order has changed.

.. versionadded:: 3.0
%End

    void hasCustomLayerOrderChanged( bool hasCustomLayerOrder );
%Docstring
 Emitted when the hasCustomLayerOrder flag changes.

.. seealso:: hasCustomLayerOrder

.. versionadded:: 3.0
%End

};

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/layertree/qgslayertree.h                                    *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
