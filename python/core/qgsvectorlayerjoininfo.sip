
class QgsVectorLayerJoinInfo
{
%TypeHeaderCode
#include "qgsvectorlayerjoininfo.h"
%End

  public:
    QgsVectorLayerJoinInfo();

    //! Sets weak reference to the joined layer
    void setJoinLayer( QgsVectorLayer* layer );
    //! Returns joined layer (may be null if the reference was set by layer ID and not resolved yet)
    QgsVectorLayer* joinLayer() const;

    //! Sets ID of the joined layer. It will need to be overwritten by setJoinLayer() to a reference to real layer
    void setJoinLayerId( const QString& layerId );
    //! ID of the joined layer - may be used to resolve reference to the joined layer
    QString joinLayerId() const;

    //! Sets name of the field of our layer that will be used for join
    void setTargetFieldName( const QString& fieldName );
    //! Returns name of the field of our layer that will be used for join
    QString targetFieldName() const;

    //! Sets name of the field of joined layer that will be used for join
    void setJoinFieldName( const QString& fieldName );
    //! Returns name of the field of joined layer that will be used for join
    QString joinFieldName() const;

    //! Sets prefix of fields from the joined layer. If null, joined layer's name will be used.
    void setPrefix( const QString& prefix );
    //! Returns prefix of fields from the joined layer. If null, joined layer's name will be used.
    QString prefix() const;

    //! Sets whether values from the joined layer should be cached in memory to speed up lookups
    void setUsingMemoryCache( bool enabled );
    //! Returns whether values from the joined layer should be cached in memory to speed up lookups
    bool isUsingMemoryCache() const;

    bool operator==( const QgsVectorLayerJoinInfo& other ) const;

    /** Set subset of fields to be used from joined layer. Takes ownership of the passed pointer. Null pointer tells to use all fields.
      @note added in 2.6 */
    void setJoinFieldNamesSubset( QStringList* fieldNamesSubset /Transfer/ );
    /** Get subset of fields to be used from joined layer. All fields will be used if null is returned.
      @note added in 2.6 */
    QStringList* joinFieldNamesSubset() const;
};
