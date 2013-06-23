
class QgsFeatureRequest
{
%TypeHeaderCode
#include <qgsfeaturerequest.h>
%End

  public:
    enum Flag
    {
      NoFlags            = 0,
      NoGeometry         = 1,
      SubsetOfAttributes = 2,
      ExactIntersect     = 4
    };
    typedef QFlags<QgsFeatureRequest::Flag> Flags;

    enum FilterType
    {
      FilterNone,   //!< No filter is applied
      FilterRect,   //!< Filter using a rectangle
      FilterFid     //!< Filter using feature ID
    };

    //! construct a default request: for all features get attributes and geometries
    QgsFeatureRequest();
    //! construct a request with feature ID filter
    explicit QgsFeatureRequest( QgsFeatureId fid );
    //! construct a request with rectangle filter
    explicit QgsFeatureRequest( const QgsRectangle& rect );

    FilterType filterType() const;

    //! Set rectangle from which features will be taken. Empty rectangle removes the filter.
    //!
    QgsFeatureRequest& setFilterRect( const QgsRectangle& rect );
    const QgsRectangle& filterRect() const;

    //! Set feature ID that should be fetched.
    QgsFeatureRequest& setFilterFid( qint64 fid );
    qint64 filterFid() const;

    //! Set flags that affect how features will be fetched
    QgsFeatureRequest& setFlags( Flags flags );
    const Flags& flags() const;

    //! Set a subset of attributes that will be fetched. Empty list means that all attributes are used.
    //! To disable fetching attributes, reset the FetchAttributes flag (which is set by default)
    QgsFeatureRequest& setSubsetOfAttributes( const QgsAttributeList& attrs );
    const QgsAttributeList& subsetOfAttributes() const;

    //! Set a subset of attributes by names that will be fetched
    QgsFeatureRequest& setSubsetOfAttributes( const QStringList& attrNames, const QgsFields& fields );

};
