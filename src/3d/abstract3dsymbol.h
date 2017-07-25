#ifndef ABSTRACT3DSYMBOL_H
#define ABSTRACT3DSYMBOL_H

#include "qgis_3d.h"

#include "phongmaterialsettings.h"
#include "utils.h"


/** 3D symbols are used by VectorLayer3DRenderer. They define appearance of data in 3D. */
class _3D_EXPORT Abstract3DSymbol
{
  public:
    virtual ~Abstract3DSymbol() {}

    virtual QString type() const = 0;
    virtual Abstract3DSymbol *clone() const = 0;

    virtual void writeXml( QDomElement &elem ) const = 0;
    virtual void readXml( const QDomElement &elem ) = 0;
};


/** 3D symbol that draws polygon geometries as planar polygons, optionally extruded (with added walls). */
class _3D_EXPORT Polygon3DSymbol : public Abstract3DSymbol
{
  public:
    Polygon3DSymbol();

    QString type() const override { return "polygon"; }
    Abstract3DSymbol *clone() const override;

    void writeXml( QDomElement &elem ) const override;
    void readXml( const QDomElement &elem ) override;

    AltitudeClamping altClamping;  //! how to handle altitude of vector features
    AltitudeBinding altBinding;    //! how to handle clamping of vertices of individual features

    float height;           //!< Base height of polygons
    float extrusionHeight;  //!< How much to extrude (0 means no walls)
    PhongMaterialSettings material;  //!< Defines appearance of objects
};


/** 3D symbol that draws point geometries as 3D objects using one of the predefined shapes. */
class _3D_EXPORT Point3DSymbol : public Abstract3DSymbol
{
  public:
    Point3DSymbol();

    QString type() const override { return "point"; }
    Abstract3DSymbol *clone() const override;

    void writeXml( QDomElement &elem ) const override;
    void readXml( const QDomElement &elem ) override;

    float height;
    PhongMaterialSettings material;  //!< Defines appearance of objects
    QVariantMap shapeProperties;  //!< What kind of shape to use and what
    QMatrix4x4 transform;  //!< Transform of individual instanced models
};


/** 3D symbol that draws linestring geometries as planar polygons (created from lines using a buffer with given thickness). */
class _3D_EXPORT Line3DSymbol : public Abstract3DSymbol
{
  public:
    Line3DSymbol();

    QString type() const override { return "line"; }
    Abstract3DSymbol *clone() const override;

    void writeXml( QDomElement &elem ) const override;
    void readXml( const QDomElement &elem ) override;

    AltitudeClamping altClamping;  //! how to handle altitude of vector features
    AltitudeBinding altBinding;    //! how to handle clamping of vertices of individual features

    float height;           //!< Base height of polygons
    float extrusionHeight;  //!< How much to extrude (0 means no walls)
    PhongMaterialSettings material;  //!< Defines appearance of objects

    float distance;  //!< Distance of buffer of lines
};


#endif // ABSTRACT3DSYMBOL_H
