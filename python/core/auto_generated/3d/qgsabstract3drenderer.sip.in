/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/./3d/qgsabstract3drenderer.h                                *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/






class QgsAbstract3DRenderer /Abstract/
{
%Docstring
Base class for all renderers that may to participate in 3D view.

3D renderers implement the method createEntity() that returns a new 3D entity - that entity
will be added to the 3D scene to represent data in renderer's display style.

Renderers may store some custom properties (e.g. materials, sizes) that are written to and read from
XML. It is therefore not recommended to store large amount of data within a renderer (e.g. arrays of vertices).

.. versionadded:: 3.0
%End

%TypeHeaderCode
#include "qgsabstract3drenderer.h"
%End
  public:
    virtual ~QgsAbstract3DRenderer();

    virtual QString type() const = 0;
%Docstring
Returns unique identifier of the renderer class (used to identify subclass)
%End
    virtual QgsAbstract3DRenderer *clone() const = 0 /Factory/;
%Docstring
Returns a cloned instance
%End

    virtual void writeXml( QDomElement &elem, const QgsReadWriteContext &context ) const = 0;
%Docstring
Writes renderer's properties to given XML element
%End
    virtual void readXml( const QDomElement &elem, const QgsReadWriteContext &context ) = 0;
%Docstring
Reads renderer's properties from given XML element
%End
    virtual void resolveReferences( const QgsProject &project );
%Docstring
Resolves references to other objects - second phase of loading - after readXml()
%End

  protected:
    QgsAbstract3DRenderer();
%Docstring
Default constructor
%End

  private:
    QgsAbstract3DRenderer( const QgsAbstract3DRenderer & );
    QgsAbstract3DRenderer &operator=( const QgsAbstract3DRenderer & );
};


/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/./3d/qgsabstract3drenderer.h                                *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
