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

/// @cond PRIVATE

inline QList<QgsMapLayer*> _qgis_listQPointerToRaw( const QList< QPointer<QgsMapLayer> >& layers )
{
  QList<QgsMapLayer*> lst;
  lst.reserve( layers.count() );
  Q_FOREACH ( const QPointer<QgsMapLayer>& layerPtr, layers )
  {
    if ( layerPtr )
      lst.append( layerPtr.data() );
  }
  return lst;
}

inline QList< QPointer<QgsMapLayer> > _qgis_listRawToQPointer( const QList<QgsMapLayer*>& layers )
{
  QList< QPointer<QgsMapLayer> > lst;
  lst.reserve( layers.count() );
  Q_FOREACH ( QgsMapLayer* layer, layers )
  {
    lst.append( layer );
  }
  return lst;
}

inline QStringList _qgis_listQPointerToIDs( const QList< QPointer<QgsMapLayer> >& layers )
{
  QStringList lst;
  lst.reserve( layers.count() );
  Q_FOREACH ( const QPointer<QgsMapLayer>& layerPtr, layers )
  {
    if ( layerPtr )
      lst << layerPtr->id();
  }
  return lst;
}

///@endcond

#endif // QGSMAPLAYERLISTUTILS_H
