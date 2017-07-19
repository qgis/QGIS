#ifndef TESSELLATOR_H
#define TESSELLATOR_H

class QgsPolygonV2;

#include <QVector>

class Tessellator
{
  public:
    Tessellator( double originX, double originY, bool addNormals );

    void addPolygon( const QgsPolygonV2 &polygon, float extrusionHeight );

    // input:
    // - origin X/Y
    // - whether to add walls
    // - stream of geometries
    // output:
    // - vertex buffer data (+ index buffer data ?)

    double originX, originY;
    bool addNormals;
    //QByteArray data;
    QVector<float> data;
    int stride;  //!< Size of one vertex entry in bytes
};

#endif // TESSELLATOR_H
