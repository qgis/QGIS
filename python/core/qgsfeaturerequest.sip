
class QgsFeatureRequest
{
%TypeHeaderCode
#include <qgsfeaturerequest.h>
%End

  public:
    enum Flag
    {
      NoFlags,
      NoGeometry,          //!< Geometry is not required. It may still be returned if e.g. required for a filter condition.
      SubsetOfAttributes,  //!< Fetch only a subset of attributes (setSubsetOfAttributes sets this flag)
      ExactIntersect       //!< Use exact geometry intersection (slower) instead of bounding boxes
    };
    typedef QFlags<QgsFeatureRequest::Flag> Flags;

    enum FilterType
    {
      FilterNone,       //!< No filter is applied
      FilterRect,       //!< Filter using a rectangle, no need to set NoGeometry
      FilterFid,        //!< Filter using feature ID
      FilterExpression, //!< Filter using expression
      FilterFids        //!< Filter using feature IDs
    };

    static const QString AllAttributes;

    //! construct a default request: for all features get attributes and geometries
    QgsFeatureRequest();
    //! construct a request with feature ID filter
    explicit QgsFeatureRequest( QgsFeatureId fid );
    //! construct a request with rectangle filter
    explicit QgsFeatureRequest( const QgsRectangle& rect );
    //! construct a request with a filter expression
    explicit QgsFeatureRequest( const QgsExpression& expr );

    FilterType filterType() const;

    //! Set rectangle from which features will be taken. Empty rectangle removes the filter.
    //!
    QgsFeatureRequest& setFilterRect( const QgsRectangle& rect );
    const QgsRectangle& filterRect() const;

    //! Set feature ID that should be fetched.
    QgsFeatureRequest& setFilterFid( qint64 fid );
    qint64 filterFid() const;

    //! Set feature ID that should be fetched.
    QgsFeatureRequest& setFilterFids( QgsFeatureIds fids );
    const QgsFeatureIds& filterFids() const;

    //! Set filter expression. {@see QgsExpression}
    QgsFeatureRequest& setFilterExpression( const QString& expression );
    QgsExpression* filterExpression() const;

    //! Set flags that affect how features will be fetched
    QgsFeatureRequest& setFlags( Flags flags );
    const Flags& flags() const;

    //! Set a subset of attributes that will be fetched. Empty list means that all attributes are used.
    //! To disable fetching attributes, reset the FetchAttributes flag (which is set by default)
    QgsFeatureRequest& setSubsetOfAttributes( const QgsAttributeList& attrs );
    const QgsAttributeList& subsetOfAttributes() const;

    //! Set a subset of attributes by names that will be fetched
    QgsFeatureRequest& setSubsetOfAttributes( const QStringList& attrNames, const QgsFields& fields );

    //! Set a simplification method for geometries that will be fetched
    //! @note added in 2.2
    QgsFeatureRequest& setSimplifyMethod( const QgsSimplifyMethod& simplifyMethod );
    //! Get simplification method for geometries that will be fetched
    //! @note added in 2.2
    const QgsSimplifyMethod& simplifyMethod() const;

    /**
     * Check if a feature is accepted by this requests filter
     *
     * @param feature  The feature which will be tested
     *
     * @return true, if the filter accepts the feature
     *
     * @note added in 2.1
     */
    bool acceptFeature( const QgsFeature& feature );

};


/** Base class that can be used for any class that is capable of returning features
 * @note added in 2.4
 */
class QgsAbstractFeatureSource
{
%TypeHeaderCode
#include <qgsfeaturerequest.h>
%End
  public:
    virtual ~QgsAbstractFeatureSource();

    virtual QgsFeatureIterator getFeatures( const QgsFeatureRequest& request ) = 0;

  protected:
    void iteratorOpened( QgsAbstractFeatureIterator* it );
    void iteratorClosed( QgsAbstractFeatureIterator* it );
};
