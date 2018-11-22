/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/qgsgeometryoptions.h                                        *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/





class QgsGeometryOptions : QObject
{
%Docstring

The QgsGeometryOptions class contains options to automatically adjust geometries to
constraints on a layer.

.. versionadded:: 3.4
%End

%TypeHeaderCode
#include "qgsgeometryoptions.h"
%End
  public:

    QgsGeometryOptions();
%Docstring
Create a new QgsGeometryOptions object.
%End

    bool removeDuplicateNodes() const;
%Docstring
Automatically remove duplicate nodes on all geometries which are edited on this layer.

.. versionadded:: 3.4
%End

    void setRemoveDuplicateNodes( bool value );
%Docstring
Automatically remove duplicate nodes on all geometries which are edited on this layer.

.. versionadded:: 3.4
%End

    double geometryPrecision() const;
%Docstring
The precision in which geometries on this layer should be saved.
Geometries which are edited on this layer will be rounded to multiples of this value (snap to grid).
Set to 0.0 to disable.

.. versionadded:: 3.4
%End

    void setGeometryPrecision( double value );
%Docstring
The precision in which geometries on this layer should be saved.
Geometries which are edited on this layer will be rounded to multiples of this value (snap to grid).
Set to 0.0 to disable.

.. versionadded:: 3.4
%End

    bool isActive() const;
%Docstring
Determines if at least one fix is enabled.

.. versionadded:: 3.4
%End

    void apply( QgsGeometry &geometry ) const;
%Docstring
Apply any fixes configured on this class to ``geometry``.

.. versionadded:: 3.4
%End

    QStringList geometryChecks() const;
%Docstring
A list of activated geometry checks.

.. versionadded:: 3.4
%End

    void setGeometryChecks( const QStringList &geometryChecks );
%Docstring
A list of activated geometry checks.

.. versionadded:: 3.4
%End

    QVariantMap checkConfiguration( const QString &checkId ) const;
%Docstring
Access the configuration for the check ``checkId``.

.. versionadded:: 3.4
%End

    void setCheckConfiguration( const QString &checkId, const QVariantMap &checkConfiguration );
%Docstring
Set the configuration for the check ``checkId``.

.. versionadded:: 3.4
%End

    void writeXml( QDomNode &node ) const;
%Docstring
Write the geometry options to the ``node``.

.. versionadded:: 3.4
%End

    void readXml( const QDomNode &node );
%Docstring
Read the geometry options from ``node``.

.. versionadded:: 3.4
%End

  signals:

    void checkConfigurationChanged();
%Docstring
Access the configuration for the check ``checkId``.

.. versionadded:: 3.4
%End

    void geometryChecksChanged();
%Docstring
A list of activated geometry checks.

.. versionadded:: 3.4
%End

    void removeDuplicateNodesChanged();
%Docstring
Automatically remove duplicate nodes on all geometries which are edited on this layer.

.. versionadded:: 3.4
%End

    void geometryPrecisionChanged();
%Docstring
The precision in which geometries on this layer should be saved.
Geometries which are edited on this layer will be rounded to multiples of this value (snap to grid).
Set to 0.0 to disable.

.. versionadded:: 3.4
%End

};

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/qgsgeometryoptions.h                                        *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
