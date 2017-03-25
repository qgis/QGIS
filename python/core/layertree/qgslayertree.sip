class QgsLayerTree : QgsLayerTreeGroup
{
%TypeHeaderCode
#include <qgslayertree.h>
%End

  public:
    static bool isGroup( QgsLayerTreeNode *node );
    static bool isLayer( const QgsLayerTreeNode *node );

    //! Cast node to a group. No type checking is done - use isGroup() to find out whether this operation is legal.
    // PYTHON: automatic cast
    //inline QgsLayerTreeGroup* toGroup( QgsLayerTreeNode* node );

    //! Cast node to a layer. No type checking is done - use isLayer() to find out whether this operation is legal.
    // PYTHON: automatic cast
    //inline QgsLayerTreeLayer* toLayer( QgsLayerTreeNode* node );

    QList<QgsMapLayer *> customLayerOrder() const;
    void setCustomLayerOrder( const QList<QgsMapLayer *> &customLayerOrder );
    void setCustomLayerOrder( const QStringList &customLayerOrder ) /PyName=setCustomLayerOrderByIds/;
    QList<QgsMapLayer *> layerOrder() const;
    bool hasCustomLayerOrder() const;
    void setHasCustomLayerOrder( bool hasCustomLayerOrder );
    static QgsLayerTree *readXml( QDomElement &element );







    void readLayerOrderFromXml( const QDomElement &doc );

    virtual void writeXml( QDomElement &parentElement );

    virtual QgsLayerTree *clone() const;






    void clear();

  signals:





    void customLayerOrderChanged();






    void layerOrderChanged();
    void hasCustomLayerOrderChanged( bool hasCustomLayerOrder );
};
