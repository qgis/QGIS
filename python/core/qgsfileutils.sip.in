/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/qgsfileutils.h                                              *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/




class QgsFileUtils
{
%Docstring
 Class for file utilities.

.. versionadded:: 3.0
%End

%TypeHeaderCode
#include "qgsfileutils.h"
%End
  public:

    static QString representFileSize( qint64 bytes );
%Docstring
Return the human size from bytes
%End


    static QStringList extensionsFromFilter( const QString &filter );
%Docstring
Returns a list of the extensions contained within a file ``filter`` string.
E.g. a ``filter`` of "GeoTIFF Files (*.tiff *.tif)" would return a list
containing "tiff", "tif". The initial '.' is stripped off the extension.

.. seealso:: :py:func:`ensureFileNameHasExtension`

.. seealso:: :py:func:`addExtensionFromFilter`
%End

    static QString ensureFileNameHasExtension( const QString &fileName, const QStringList &extensions );
%Docstring
Ensures that a ``fileName`` ends with an extension from the provided list of
``extensions``.

E.g. if extensions contains "tif" and "tiff", then a ``fileName`` of
"d:/my_file" will return "d:/my_file.tif". A ``fileName`` of
"d:/my_file.TIFF" or "d:/my_file.TIF" will be returned unchanged.

.. seealso:: :py:func:`extensionsFromFilter`

.. seealso:: :py:func:`addExtensionFromFilter`
%End

    static QString addExtensionFromFilter( const QString &fileName, const QString &filter );
%Docstring
Ensures that a ``fileName`` ends with an extension from the specified ``filter``
string.

E.g. a ``fileName`` of "d:/my_file" with a filter of "GeoTIFF Files (*.tiff *.tif)"
will return "d:/my_file.tif", where as ``fileName`` of "d:/my_file.TIFF" or "d:/my_file.TIF"
will be returned unchanged.

.. seealso:: :py:func:`extensionsFromFilter`

.. seealso:: :py:func:`ensureFileNameHasExtension`
%End

    static QString stringToSafeFilename( const QString &string );
%Docstring
Converts a ``string`` to a safe filename, replacing characters which are not safe
for filenames with an '_' character.

This method should be called with file names only, not complete paths.
%End
};

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/qgsfileutils.h                                              *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
