
class QgsFeatureRequest
{
%TypeHeaderCode
#include <qgsfeaturerequest.h>
%End

public:
  enum Flag {
    NoGeometry     = 0x01,  //!< Do not fetch geometry
    NoAttributes   = 0x02,  //!< Do not fetch any attributes
    ExactIntersect = 0x04   //!< Use exact geometry intersection (slower) instead of bounding boxes
  };
  typedef QFlags<QgsFeatureRequest::Flag> Flags;

  //! construct a default request: for all features get attributes and geometries
  QgsFeatureRequest();

  //! Set rectangle from which features will be taken. Empty rectangle removes the filter.
  QgsFeatureRequest& setExtent(const QgsRectangle& rect);
  const QgsRectangle& extent() const;

  //! Set flags that affect how features will be fetched
  QgsFeatureRequest& setFlags(Flags flags);
  const Flags& flags() const;

  //! Set a subset of attributes that will be fetched. Empty list means that all attributes are used.
  //! To disable fetching attributes, reset the FetchAttributes flag (which is set by default)
  QgsFeatureRequest& setAttributes(const QgsAttributeList& attrs);
  const QgsAttributeList& attributes() const;

};
