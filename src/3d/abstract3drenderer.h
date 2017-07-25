#ifndef ABSTRACT3DRENDERER_H
#define ABSTRACT3DRENDERER_H

#include "qgis_3d.h"

#include "phongmaterialsettings.h"
#include "utils.h"

#include "qgsmaplayerref.h"

#include <QObject>

class QgsVectorLayer;

class Abstract3DSymbol;
class Map3D;


namespace Qt3DCore
{
  class QEntity;
}

class _3D_EXPORT Abstract3DRenderer //: public QObject
{
    //Q_OBJECT
  public:
    virtual ~Abstract3DRenderer() {}

    virtual QString type() const = 0;
    virtual Abstract3DRenderer *clone() const = 0;
    virtual Qt3DCore::QEntity *createEntity( const Map3D &map ) const = 0;

    virtual void writeXml( QDomElement &elem ) const = 0;
    virtual void readXml( const QDomElement &elem ) = 0;
    virtual void resolveReferences( const QgsProject &project ) { Q_UNUSED( project ); }
};


/** 3D renderer that renders all features of a vector layer with the same 3D symbol.
 * The appearance if completely defined by the symbol.
 */
class _3D_EXPORT VectorLayer3DRenderer : public Abstract3DRenderer
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
    Abstract3DRenderer *clone() const override;
    Qt3DCore::QEntity *createEntity( const Map3D &map ) const override;

    void writeXml( QDomElement &elem ) const override;
    void readXml( const QDomElement &elem ) override;
    void resolveReferences( const QgsProject &project ) override;

  private:
    QgsMapLayerRef layerRef; //!< Layer used to extract polygons from
    std::unique_ptr<Abstract3DSymbol> mSymbol;  //!< 3D symbol that defines appearance
};


#endif // ABSTRACT3DRENDERER_H
