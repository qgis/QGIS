class QgsVectorLayerJoinBuffer : QObject
{
%TypeHeaderCode
#include <qgsvectorlayerjoinbuffer.h>
%End
  public:
    QgsVectorLayerJoinBuffer();
    ~QgsVectorLayerJoinBuffer();

    /**Joins another vector layer to this layer
      @param joinInfo join object containing join layer id, target and source field */
    void addJoin( const QgsVectorJoinInfo& joinInfo );

    /**Removes  a vector layer join*/
    void removeJoin( const QString& joinLayerId );

    /**Updates field map with joined attributes
      @param fields map to append joined attributes
     */
    void updateFields( QgsFields& fields );

    /**Calls cacheJoinLayer() for all vector joins*/
    void createJoinCaches();

    /**Saves mVectorJoins to xml under the layer node*/
    void writeXml( QDomNode& layer_node, QDomDocument& document ) const;

    /**Reads joins from project file*/
    void readXml( const QDomNode& layer_node );

    /**Quick way to test if there is any join at all*/
    bool containsJoins() const;

    const QList< QgsVectorJoinInfo >& vectorJoins() const;

    /**Finds the vector join for a layer field index.
      @param index this layers attribute index
      @param fields fields of the vector layer (including joined fields)
      @param sourceFieldIndex Output: field's index in source layer */
    const QgsVectorJoinInfo* joinForFieldIndex( int index, const QgsFields& fields, int& sourceFieldIndex /Out/ ) const;

    //! Create a copy of the join buffer
    //! @note added in 2.6
    QgsVectorLayerJoinBuffer* clone() const /Factory/;

  signals:
    //! Emitted whenever the list of joined fields changes (e.g. added join or joined layer's fields change)
    //! @note added in 2.6
    void joinedFieldsChanged();
};
