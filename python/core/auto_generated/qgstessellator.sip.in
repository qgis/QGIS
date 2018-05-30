/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/qgstessellator.h                                            *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/





class QgsTessellator
{
%Docstring
Class that takes care of tessellation of polygons into triangles.

It is expected that client code will create the tessellator object, then repeatedly call
addPolygon() method that will generate triangles, and finally call data() to get final vertex data.

Optionally provides extrusion by adding triangles that serve as walls when extrusion height is non-zero.

.. versionadded:: 3.4
%End

%TypeHeaderCode
#include "qgstessellator.h"
%End
  public:
    QgsTessellator( double originX, double originY, bool addNormals, bool invertNormals = false, bool addBackFaces = false );
%Docstring
Creates tessellator with a specified origin point of the world (in map coordinates)
%End

    void addPolygon( const QgsPolygon &polygon, float extrusionHeight );
%Docstring
Tessellates a triangle and adds its vertex entries to the output data array
%End

    QVector<float> data() const;
%Docstring
Returns array of triangle vertex data

Vertice coordinates are stored as (x, z, -y)
%End

    int dataVerticesCount() const;
%Docstring
Returns the number of vertices stored in the output data array
%End

    int stride() const;
%Docstring
Returns size of one vertex entry in bytes
%End


};

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/qgstessellator.h                                            *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
