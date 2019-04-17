/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/./3d/qgs3drendererregistry.h                                *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/






class Qgs3DRendererAbstractMetadata
{
%Docstring
Base metadata class for 3D renderers. Instances of derived classes may be registered in Qgs3DRendererRegistry.

.. versionadded:: 3.0
%End

%TypeHeaderCode
#include "qgs3drendererregistry.h"
%End
  public:

    virtual ~Qgs3DRendererAbstractMetadata();

    QString type() const;
%Docstring
Returns unique identifier of the 3D renderer class
%End

    virtual QgsAbstract3DRenderer *createRenderer( QDomElement &elem, const QgsReadWriteContext &context ) = 0 /Factory/;
%Docstring
Returns new instance of the renderer given the DOM element. Returns ``None`` on error.
Pure virtual function: must be implemented in derived classes.
%End

  protected:

    explicit Qgs3DRendererAbstractMetadata( const QString &type );
%Docstring
Constructor of the base class
%End

  protected:
};


class Qgs3DRendererRegistry
{
%Docstring
Keeps track of available 3D renderers. Should be accessed through QgsApplication.renderer3DRegistry() singleton.

.. versionadded:: 3.0
%End

%TypeHeaderCode
#include "qgs3drendererregistry.h"
%End
  public:
    Qgs3DRendererRegistry();
%Docstring
Creates registry of 3D renderers
%End

    ~Qgs3DRendererRegistry();

    void addRenderer( Qgs3DRendererAbstractMetadata *metadata /Transfer/ );
%Docstring
Registers a new 3D renderer type. The call takes ownership of the passed metadata object.
%End

    void removeRenderer( const QString &type );
%Docstring
Unregisters a 3D renderer type
%End

    Qgs3DRendererAbstractMetadata *rendererMetadata( const QString &type ) const;
%Docstring
Returns metadata for a 3D renderer type (may be used to create a new instance of the type)
%End

    QStringList renderersList() const;
%Docstring
Returns a list of all available 3D renderer types.
%End

};

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/./3d/qgs3drendererregistry.h                                *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
