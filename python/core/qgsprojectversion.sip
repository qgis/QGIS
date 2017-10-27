/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/qgsprojectversion.h                                         *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/





class QgsProjectVersion
{
%Docstring
 A class to describe the version of a project.
 Used in places where you need to check if the current version
 of QGIS is greater than the one used to create a project file.
%End

%TypeHeaderCode
#include "qgsprojectversion.h"
%End
  public:

    QgsProjectVersion();
%Docstring
 Creates a new NULL version
%End

    QgsProjectVersion( int major, int minor, int sub, const QString &name = "" );
    QgsProjectVersion( const QString &string );
    int majorVersion();
%Docstring
 :rtype: int
%End
    int minorVersion();
%Docstring
 :rtype: int
%End
    int subVersion();
%Docstring
 :rtype: int
%End
    QString text();
%Docstring
 :rtype: str
%End

    bool isNull() const;
%Docstring
 Returns true if this is a NULL project version.
 :rtype: bool
%End

    bool operator==( const QgsProjectVersion &other ) const;

    bool operator!=( const QgsProjectVersion &other ) const;
%Docstring
 Boolean not equal operator
 :rtype: bool
%End

    bool operator>=( const QgsProjectVersion &other ) const;
%Docstring
 Boolean >= operator
 :rtype: bool
%End

    bool operator>( const QgsProjectVersion &other ) const;
%Docstring
 Boolean > operator
 :rtype: bool
%End

};


/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/qgsprojectversion.h                                         *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
