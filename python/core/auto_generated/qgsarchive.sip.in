/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/qgsarchive.h                                                *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/





class QgsArchive
{
%Docstring
Class allowing to manage the zip/unzip actions

.. versionadded:: 3.0
%End

%TypeHeaderCode
#include "qgsarchive.h"
%End
  public:

    QgsArchive();
%Docstring
Constructor
%End

    QgsArchive( const QgsArchive &other );
%Docstring
Copy constructor
%End


    virtual ~QgsArchive();

    bool zip( const QString &zipFilename );
%Docstring
Zip the content of this archive

:param zipFilename: The name of the zip to generate

:return: ``False`` if something goes wrong, ``True`` otherwise
%End

    virtual bool unzip( const QString &zipFilename );
%Docstring
Clear the current content of this archive and unzip. Files are unzipped
in the temporary directory.

:param zipFilename: The zip file to unzip

:return: ``True`` if unzip action is a success, ``False`` otherwise
%End

    void clear();
%Docstring
Clear the current content of this archive and create a new temporary
directory.
%End

    void addFile( const QString &filename );
%Docstring
Add a new file to this archive. During a zip action, this file will be
part of the resulting zipped file.

:param filename: A file to add when zipping this archive
%End

    bool removeFile( const QString &filename );
%Docstring
Remove a file from this archive and from the filesystem.

:param filename: The path of the file to remove

:return: ``True`` if the file has been removed from the filesystem, ``False`` otherwise
%End

    QStringList files() const;
%Docstring
Returns the list of files within this archive
%End

    QString dir() const;
%Docstring
Returns the current temporary directory.
%End

};

class QgsProjectArchive : QgsArchive
{
%Docstring
Class allowing to manage the zip/unzip actions on project file

.. versionadded:: 3.0
%End

%TypeHeaderCode
#include "qgsarchive.h"
%End
  public:

    virtual bool unzip( const QString &zipFilename );

%Docstring
Clear the current content of this archive and unzip. If a project file
is found in the content, then this archive may be considered as a valid
one. Files are unzipped in the temporary directory.

:param zipFilename: The zip file to unzip

:return: ``True`` if a project file has been found, ``False`` otherwise
%End

    QString projectFile() const;
%Docstring
Returns the current .qgs project file or an empty string if there's none
%End

    bool clearProjectFile();
%Docstring
Remove the current .qgs project file from the temporary directory.

:return: ``True`` if the file is well removed, ``False`` otherwise
%End

    QString auxiliaryStorageFile() const;
%Docstring
Returns the current .qgd auxiliary storage file or an empty string if
there's none
%End
};

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/qgsarchive.h                                                *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
