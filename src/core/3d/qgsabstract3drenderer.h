#ifndef QGSABSTRACT3DRENDERER_H
#define QGSABSTRACT3DRENDERER_H

#include "qgis_core.h"

#include <QString>

class QDomElement;
class QgsProject;
class QgsReadWriteContext;
class Map3D;

namespace Qt3DCore
{
  class QEntity;
}

//! Base class for all renderers that may to participate in 3D view.
class CORE_EXPORT QgsAbstract3DRenderer //: public QObject
{
    //Q_OBJECT
  public:
    virtual ~QgsAbstract3DRenderer();

    virtual QString type() const = 0;
    virtual QgsAbstract3DRenderer *clone() const = 0;
    virtual Qt3DCore::QEntity *createEntity( const Map3D &map ) const = 0;

    virtual void writeXml( QDomElement &elem, const QgsReadWriteContext &context ) const = 0;
    virtual void readXml( const QDomElement &elem, const QgsReadWriteContext &context ) = 0;
    virtual void resolveReferences( const QgsProject &project );
};



#endif // QGSABSTRACT3DRENDERER_H
