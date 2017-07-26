#ifndef QGS3DRENDERERREGISTRY_H
#define QGS3DRENDERERREGISTRY_H

#include "qgis_core.h"

#include <QMap>

class QDomElement;
class QgsAbstract3DRenderer;
class QgsReadWriteContext;


/**
 * Base metadata class for 3D renderers. Instances of derived classes may be registered in Qgs3DRendererRegistry.
 * \since QGIS 3.0
 * \ingroup core
 */
class CORE_EXPORT Qgs3DRendererAbstractMetadata
{
  public:

    Qgs3DRendererAbstractMetadata( const QString &name );

    QString name() const;

    /** Return new instance of the renderer given the DOM element. Returns NULL on error.
     * Pure virtual function: must be implemented in derived classes.  */
    virtual QgsAbstract3DRenderer *createRenderer( QDomElement &elem, const QgsReadWriteContext &context ) = 0;

  protected:
    //! name used within QGIS for identification (the same what renderer's type() returns)
    QString mName;
};


/**
 * Keeps track of available 3D renderers. Should be accessed through QgsApplication::renderer3DRegistry() singleton.
 * \since QGIS 3.0
 * \ingroup core
 */
class CORE_EXPORT Qgs3DRendererRegistry
{
  public:
    Qgs3DRendererRegistry();

    ~Qgs3DRendererRegistry();

    //! takes ownership
    void addRenderer( Qgs3DRendererAbstractMetadata *metadata );

    void removeRenderer( const QString &name );

    Qgs3DRendererAbstractMetadata *rendererMetadata( const QString &name ) const;

    QStringList renderersList() const;

  private:
    QMap<QString, Qgs3DRendererAbstractMetadata *> mRenderers;
};

#endif // QGS3DRENDERERREGISTRY_H
