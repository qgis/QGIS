/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/qgsmaplayerdependency.h                                     *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/





class QgsMapLayerDependency
{
%Docstring
This class models dependencies with or between map layers.
A dependency is defined by a layer ID, a type and an origin.
The two combinations of type/origin that are currently supported are:
- PresenceDependency && FromProvider: virtual layers for instance which may depend on other layers already loaded to work
- DataDependency && FromUser: dependencies given by the user, mainly to represent database triggers

.. versionadded:: 3.0
%End

%TypeHeaderCode
#include "qgsmaplayerdependency.h"
%End
  public:
    enum Type
    {
      PresenceDependency,
      DataDependency
    };

    enum Origin
    {
      FromProvider,
      FromUser
    };

    QgsMapLayerDependency( const QString &layerId, Type type = DataDependency, Origin origin = FromUser );
%Docstring
Standard constructor
%End

    Type type() const;
%Docstring
Returns the dependency type
%End

    Origin origin() const;
%Docstring
Returns the dependency origin
%End

    QString layerId() const;
%Docstring
Returns the ID of the layer this dependency depends on
%End

    bool operator==( const QgsMapLayerDependency &other ) const;

    //! hash operator
    long __hash__() const;
%MethodCode
    sipRes = qHash( *sipCpp );
%End
};


/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/qgsmaplayerdependency.h                                     *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
