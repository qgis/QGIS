#ifndef QGSFEATUREREQUEST_H
#define QGSFEATUREREQUEST_H

#include <QFlags>

#include "qgsrectangle.h"

#include <QList>
typedef QList<int> QgsAttributeList;

/**
 * This class wraps a request for features to a vector layer (or directly its vector data provider).
 *
 * The options may be chained, e.g.:
 *   QgsFeatureRequest().setExtent(QgsRectangle(0,0,1,1)).setFlags(QgsFeatureRequest::ExactIntersect)
 */
class QgsFeatureRequest
{
  public:
    enum Flag
    {
      NoGeometry     = 0x01,  //!< Do not fetch geometry
      NoAttributes   = 0x02,  //!< Do not fetch any attributes
      ExactIntersect = 0x04   //!< Use exact geometry intersection (slower) instead of bounding boxes
    };
    Q_DECLARE_FLAGS( Flags, Flag )

    //! construct a default request: for all features get attributes and geometries
    QgsFeatureRequest();

    //! Set rectangle from which features will be taken. Empty rectangle removes the filter.
    QgsFeatureRequest& setExtent( const QgsRectangle& rect ) { mRect = rect; return *this; }
    const QgsRectangle& extent() const { return mRect; }

    //! Set flags that affect how features will be fetched
    QgsFeatureRequest& setFlags( Flags flags ) { mFlags = flags; return *this; }
    const Flags& flags() const { return mFlags; }

    //! Set a subset of attributes that will be fetched. Empty list means that all attributes are used.
    //! To disable fetching attributes, reset the FetchAttributes flag (which is set by default)
    QgsFeatureRequest& setAttributes( const QgsAttributeList& attrs ) { mAttrs = attrs; return *this; }
    const QgsAttributeList& attributes() const { return mAttrs; }

    // TODO: maybe set attributes as a list of strings?

    // TODO: in future
    // void setExpression(const QString& expression);
    // void setLimit(int limit);

  protected:
    QgsRectangle mRect;
    Flags mFlags;
    QgsAttributeList mAttrs;
};

Q_DECLARE_OPERATORS_FOR_FLAGS( QgsFeatureRequest::Flags )


#endif // QGSFEATUREREQUEST_H
