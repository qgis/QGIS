/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/mesh/qgsmeshtimesettings.h                                  *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/





class QgsMeshTimeSettings
{
%Docstring

Represents a mesh time settings for mesh datasets

.. versionadded:: 3.8
%End

%TypeHeaderCode
#include "qgsmeshtimesettings.h"
%End
  public:
    QgsMeshTimeSettings();
%Docstring
Default constructor for relative time formate and 0 offset
%End
    QgsMeshTimeSettings( double relativeTimeOffsetHours, const QString &relativeTimeFormat );
%Docstring
Constructs relative time format settings with defined offset in hours
%End
    QgsMeshTimeSettings( const QDateTime &absoluteTimeReferenceTime, const QString &absoluteTimeFormat );
%Docstring
Constructs absolute time format settings with defined reference time
%End

    QDomElement writeXml( QDomDocument &doc, const QgsReadWriteContext &context ) const;
%Docstring
Writes configuration to a new DOM element
%End
    void readXml( const QDomElement &elem, const QgsReadWriteContext &context );
%Docstring
Reads configuration from the given DOM element
%End

    bool useAbsoluteTime() const;
%Docstring
Returns whether to use absolute time format
%End
    void setUseAbsoluteTime( bool useAbsoluteTime );
%Docstring
Sets use absolute time flag
%End

    double relativeTimeOffsetHours() const;
%Docstring
Returns number of offset hours for relative time formatting
%End
    void setRelativeTimeOffsetHours( double relativeTimeOffsetHours );
%Docstring
Sets number of offset hours for relative time formatting
%End

    QString relativeTimeFormat() const;
%Docstring
Returns format used for relative time
%End
    void setRelativeTimeFormat( const QString &relativeTimeFormat );
%Docstring
Sets format used for relative time
%End

    QDateTime absoluteTimeReferenceTime() const;
%Docstring
Returns reference time used for absolute time format
%End
    void setAbsoluteTimeReferenceTime( const QDateTime &absoluteTimeReferenceTime );
%Docstring
Sets reference time used for absolute time format
%End

    QString absoluteTimeFormat() const;
%Docstring
Returns format used for absolute time
%End
    void setAbsoluteTimeFormat( const QString &absoluteTimeFormat );
%Docstring
Sets format used for absolute time
%End

};


/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/mesh/qgsmeshtimesettings.h                                  *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
