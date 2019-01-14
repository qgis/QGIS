#include "qgsstorebadlayerinfo.h"
#include <QDomElement>

QgsStoreBadLayerInfo::QgsStoreBadLayerInfo(): QgsProjectBadLayerHandler()
{
}

QgsStoreBadLayerInfo::~QgsStoreBadLayerInfo()
{
}

void QgsStoreBadLayerInfo::handleBadLayers( const QList<QDomNode> &layers )
{
  mBadLayerIds.clear();
  QList<QDomNode>::const_iterator it = layers.constBegin();
  for ( ; it != layers.constEnd(); ++it )
  {
    if ( !it->isNull() )
    {
      QDomElement idElem = it->firstChildElement( "id" );
      if ( !idElem.isNull() )
      {
        mBadLayerIds.append( idElem.text() );
      }
    }
  }
}
