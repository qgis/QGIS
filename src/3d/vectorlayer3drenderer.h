#ifndef VECTORLAYER3DRENDERER_H
#define VECTORLAYER3DRENDERER_H

#include "qgis_3d.h"

#include "qgsabstract3drenderer.h"

#include "phongmaterialsettings.h"
#include "utils.h"

#include "qgsmaplayerref.h"

#include <QObject>

class QgsVectorLayer;

class Abstract3DSymbol;

/** 3D renderer that renders all features of a vector layer with the same 3D symbol.
 * The appearance is completely defined by the symbol.
 */
class _3D_EXPORT VectorLayer3DRenderer : public QgsAbstract3DRenderer
{
  public:
    //! Takes ownership of the symbol object
    explicit VectorLayer3DRenderer( Abstract3DSymbol *s = nullptr );
    ~VectorLayer3DRenderer();

    void setLayer( QgsVectorLayer *layer );
    QgsVectorLayer *layer() const;

    //! takes ownership of the symbol
    void setSymbol( Abstract3DSymbol *symbol );
    const Abstract3DSymbol *symbol() const;

    QString type() const override { return "vector"; }
    VectorLayer3DRenderer *clone() const override;
    Qt3DCore::QEntity *createEntity( const Map3D &map ) const override;

    void writeXml( QDomElement &elem ) const override;
    void readXml( const QDomElement &elem ) override;
    void resolveReferences( const QgsProject &project ) override;

  private:
    QgsMapLayerRef layerRef; //!< Layer used to extract polygons from
    std::unique_ptr<Abstract3DSymbol> mSymbol;  //!< 3D symbol that defines appearance
};


#endif // VECTORLAYER3DRENDERER_H
