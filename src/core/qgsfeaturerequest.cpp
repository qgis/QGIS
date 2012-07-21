#include "qgsfeaturerequest.h"

#include "qgsfield.h"

#include <QStringList>

QgsFeatureRequest::QgsFeatureRequest()
    : mFlags( 0 )
{
}


QgsFeatureRequest& QgsFeatureRequest::setSubsetOfAttributes( const QStringList& attrNames, const QgsFieldMap& fields )
{
  mFlags |= SubsetOfAttributes;
  mAttrs.clear();

  foreach( const QString& attrName, attrNames )
  {
    int idx = fields.key( attrName, -1 );
    if ( idx != -1 )
      mAttrs.append( idx );
  }

  return *this;
}
