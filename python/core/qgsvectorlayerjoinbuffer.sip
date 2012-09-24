class QgsVectorLayerJoinBuffer
{
%TypeHeaderCode
#include <qgsvectorlayerjoinbuffer.h>
%End
  public:
    QgsVectorLayerJoinBuffer();
    ~QgsVectorLayerJoinBuffer();

    /**Joins another vector layer to this layer
      @param joinInfo join object containing join layer id, target and source field */
    void addJoin( QgsVectorJoinInfo joinInfo );

    /**Removes  a vector layer join*/
    void removeJoin( const QString& joinLayerId );

    /**Creates QgsVectorLayerJoinBuffer for the joins containing attributes to fetch*/
    void select( const QgsAttributeList& fetchAttributes,
                 QgsAttributeList& sourceJoinFields, int maxProviderIndex );

    /**Updates field map with joined attributes
      @param fields map to append joined attributes
      @param maxIndex in/out: maximum attribute index*/
    void updateFieldMap( QMap<int, QgsField> &fields, int& maxIndex );

    /**Update feature with uncommited attribute updates and joined attributes*/
    void updateFeatureAttributes( QgsFeature &f, int maxProviderIndex, bool all = false );

    /**Calls cacheJoinLayer() for all vector joins*/
    void createJoinCaches();

    /**Saves mVectorJoins to xml under the layer node*/
    void writeXml( QDomNode& layer_node, QDomDocument& document ) const;

    /**Reads joins from project file*/
    void readXml( const QDomNode& layer_node );

    /**Quick way to test if there is any join at all*/
    bool containsJoins() const;
    /**Quick way to test if there is a join to be fetched*/
    bool containsFetchJoins() const;

    const QList< QgsVectorJoinInfo >& vectorJoins() const;

    /**Finds the vector join for a layer field index.
      @param index this layers attribute index
      @param maxProviderIndex maximum attribute index of the vectorlayer provider
      @param indexOffset out: offset between layer index and original provider index
       @return pointer to the join if the index belongs to a joined field, otherwise 0 (possibily provider field or added field)*/
    const QgsVectorJoinInfo* joinForFieldIndex( int index, int maxProviderIndex, int& indexOffset ) const;

    /** Helper function to find out the maximum index of a field map
        @return true in case of success, otherwise false (e.g. empty map)*/
    static bool maximumIndex( const QMap<int, QgsField> &fMap, int& index );
};
