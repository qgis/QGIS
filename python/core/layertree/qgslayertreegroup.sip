/**
 * Layer tree group node serves as a container for layers and further groups.
 *
 * @note added in 2.4
 */
class QgsLayerTreeGroup : QgsLayerTreeNode
{
%TypeHeaderCode
#include <qgslayertreegroup.h>
%End

  public:
    QgsLayerTreeGroup( const QString& name = QString(), Qt::CheckState checked = Qt::Checked );

    QString name() const;
    void setName( const QString& n );

    QgsLayerTreeGroup* addGroup( const QString& name );
    QgsLayerTreeLayer* insertLayer( int index, QgsMapLayer* layer );
    QgsLayerTreeLayer* addLayer( QgsMapLayer* layer );

    void insertChildNodes( int index, QList<QgsLayerTreeNode*> nodes /Transfer/ );
    void insertChildNode( int index, QgsLayerTreeNode* node /Transfer/ );
    void addChildNode( QgsLayerTreeNode* node /Transfer/ );

    void removeChildNode( QgsLayerTreeNode* node );

    void removeLayer( QgsMapLayer* layer );

    void removeChildren( int from, int count );

    void removeAllChildren();

    QgsLayerTreeLayer* findLayer( const QString& layerId );
    QList<QgsLayerTreeLayer*> findLayers() const;
    QStringList findLayerIds() const;
    QgsLayerTreeGroup* findGroup( const QString& name );

    static QgsLayerTreeGroup* readXML( QDomElement& element ) /Factory/;
    virtual void writeXML( QDomElement& parentElement );

    void readChildrenFromXML( QDomElement& element );

    virtual QString dump() const;

    virtual QgsLayerTreeNode* clone() const /Factory/;

    Qt::CheckState isVisible() const;
    void setVisible( Qt::CheckState state );

  private:
    QgsLayerTreeGroup( const QgsLayerTreeGroup& other );
};
