#ifndef ABSTRACT3DRENDERER_H
#define ABSTRACT3DRENDERER_H

#include "qgis_3d.h"

#include "phongmaterialsettings.h"
#include "utils.h"

#include "qgsmaplayerref.h"

#include <QObject>

class QgsVectorLayer;

class Map3D;


namespace Qt3DCore
{
  class QEntity;
}

class Abstract3DRenderer //: public QObject
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


class _3D_EXPORT PolygonRenderer : public Abstract3DRenderer
{
  public:
    PolygonRenderer();

    void setLayer( QgsVectorLayer *layer );
    QgsVectorLayer *layer() const;

    QString type() const override { return "polygon"; }
    Abstract3DRenderer *clone() const override;
    Qt3DCore::QEntity *createEntity( const Map3D &map ) const override;

    void writeXml( QDomElement &elem ) const override;
    void readXml( const QDomElement &elem ) override;
    void resolveReferences( const QgsProject &project ) override;

    AltitudeClamping altClamping;  //! how to handle altitude of vector features
    AltitudeBinding altBinding;    //! how to handle clamping of vertices of individual features

    float height;           //!< Base height of polygons
    float extrusionHeight;  //!< How much to extrude (0 means no walls)
    PhongMaterialSettings material;  //!< Defines appearance of objects

  private:
    QgsMapLayerRef layerRef; //!< Layer used to extract polygons from
};

class _3D_EXPORT PointRenderer : public Abstract3DRenderer
{
  public:
    PointRenderer();

    void setLayer( QgsVectorLayer *layer );
    QgsVectorLayer *layer() const;

    QString type() const override { return "point"; }
    Abstract3DRenderer *clone() const override;
    Qt3DCore::QEntity *createEntity( const Map3D &map ) const override;

    void writeXml( QDomElement &elem ) const override;
    void readXml( const QDomElement &elem ) override;
    void resolveReferences( const QgsProject &project ) override;

    float height;
    PhongMaterialSettings material;  //!< Defines appearance of objects
    QVariantMap shapeProperties;  //!< What kind of shape to use and what
    QMatrix4x4 transform;  //!< Transform of individual instanced models

  private:
    QgsMapLayerRef layerRef; //!< Layer used to extract points from
};

class _3D_EXPORT LineRenderer : public Abstract3DRenderer
{
  public:
    LineRenderer();

    void setLayer( QgsVectorLayer *layer );
    QgsVectorLayer *layer() const;

    QString type() const override { return "line"; }
    Abstract3DRenderer *clone() const override;
    Qt3DCore::QEntity *createEntity( const Map3D &map ) const override;

    void writeXml( QDomElement &elem ) const override;
    void readXml( const QDomElement &elem ) override;
    void resolveReferences( const QgsProject &project ) override;

    AltitudeClamping altClamping;  //! how to handle altitude of vector features
    AltitudeBinding altBinding;    //! how to handle clamping of vertices of individual features

    float height;           //!< Base height of polygons
    float extrusionHeight;  //!< How much to extrude (0 means no walls)
    PhongMaterialSettings material;  //!< Defines appearance of objects

    float distance;  //!< Distance of buffer of lines

  private:
    QgsMapLayerRef layerRef; //!< Layer used to extract points from
};

#endif // ABSTRACT3DRENDERER_H
