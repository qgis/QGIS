#ifndef QGSVECTORLAYER3DRENDERER_H
#define QGSVECTORLAYER3DRENDERER_H

#include "qgis_3d.h"

#include "qgs3drendererregistry.h"
#include "qgsabstract3drenderer.h"

#include "qgsphongmaterialsettings.h"
#include "qgs3dutils.h"

#include "qgsmaplayerref.h"

#include <QObject>

class QgsVectorLayer;

class QgsAbstract3DSymbol;


//! Metadata for vector layer 3D renderer to allow creation of its instances from XML
class _3D_EXPORT QgsVectorLayer3DRendererMetadata : public Qgs3DRendererAbstractMetadata
{
  public:
    QgsVectorLayer3DRendererMetadata();

    virtual QgsAbstract3DRenderer *createRenderer( QDomElement &elem, const QgsReadWriteContext &context ) override;
};


/** 3D renderer that renders all features of a vector layer with the same 3D symbol.
 * The appearance is completely defined by the symbol.
 */
class _3D_EXPORT QgsVectorLayer3DRenderer : public QgsAbstract3DRenderer
{
  public:
    //! Takes ownership of the symbol object
    explicit QgsVectorLayer3DRenderer( QgsAbstract3DSymbol *s = nullptr );
    ~QgsVectorLayer3DRenderer();

    void setLayer( QgsVectorLayer *layer );
    QgsVectorLayer *layer() const;

    //! takes ownership of the symbol
    void setSymbol( QgsAbstract3DSymbol *symbol );
    const QgsAbstract3DSymbol *symbol() const;

    QString type() const override { return "vector"; }
    QgsVectorLayer3DRenderer *clone() const override;
    Qt3DCore::QEntity *createEntity( const Qgs3DMapSettings &map ) const override;

    void writeXml( QDomElement &elem, const QgsReadWriteContext &context ) const override;
    void readXml( const QDomElement &elem, const QgsReadWriteContext &context ) override;
    void resolveReferences( const QgsProject &project ) override;

  private:
    QgsMapLayerRef layerRef; //!< Layer used to extract polygons from
    std::unique_ptr<QgsAbstract3DSymbol> mSymbol;  //!< 3D symbol that defines appearance
};


#endif // QGSVECTORLAYER3DRENDERER_H
