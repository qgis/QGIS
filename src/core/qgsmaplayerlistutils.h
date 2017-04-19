#ifndef QGSMAPLAYERLISTUTILS_H
#define QGSMAPLAYERLISTUTILS_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the QGIS API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//

#include <QPointer>

#include "qgsmaplayer.h"
#include "qgsmaplayerref.h"

/// @cond PRIVATE

inline QList<QgsMapLayer *> _qgis_listRefToRaw( const QList< QgsMapLayerRef > &layers )
{
  QList<QgsMapLayer *> lst;
  lst.reserve( layers.count() );
  Q_FOREACH ( const QgsMapLayerRef &layer, layers )
  {
    if ( layer )
      lst.append( layer.get() );
  }
  return lst;
}

inline QList< QgsMapLayerRef > _qgis_listRawToRef( const QList<QgsMapLayer *> &layers )
{
  QList< QgsMapLayerRef > lst;
  lst.reserve( layers.count() );
  Q_FOREACH ( QgsMapLayer *layer, layers )
  {
    lst.append( QgsMapLayerRef( layer ) );
  }
  return lst;
}

inline void _qgis_removeLayers( QList< QgsMapLayerRef > &list, QList< QgsMapLayer *> layersToRemove )
{
  QMutableListIterator<QgsMapLayerRef> it( list );
  while ( it.hasNext() )
  {
    QgsMapLayerRef &ref = it.next();
    if ( layersToRemove.contains( ref.get() ) )
      it.remove();
  }
}


///@endcond

#endif // QGSMAPLAYERLISTUTILS_H
