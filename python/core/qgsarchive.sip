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
%Docstring
 Destructor
%End

    bool zip( const QString &zipFilename );
%Docstring
 Zip the content of this archive
 \param zipFilename The name of the zip to generate
 :return: false if something goes wrong, true otherwise
 :rtype: bool
%End

    virtual bool unzip( const QString &zipFilename );
%Docstring
 Clear the current content of this archive and unzip. Files are unzipped
 in the temporary directory.
 \param zipFilename The zip file to unzip
 :return: true if unzip action is a success, false otherwise
 :rtype: bool
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
 \param filename A file to add when zipping this archive
%End

    bool removeFile( const QString &filename );
%Docstring
 Remove a file from this archive and from the filesystem.
 \param filename The path of the file to remove
 :return: true if the file has been removed from the filesystem, false otherwise
 :rtype: bool
%End

    QStringList files() const;
%Docstring
 Returns the list of files within this archive
 :rtype: list of str
%End

    QString dir() const;
%Docstring
 Returns the current temporary directory.
 :rtype: str
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
 \param zipFilename The zip file to unzip
 :return: true if a project file has been found, false otherwise
 :rtype: bool
%End

    QString projectFile() const;
%Docstring
 Returns the current .qgs project file or an empty string if there's none
 :rtype: str
%End

    bool clearProjectFile();
%Docstring
 Remove the current .qgs project file from the temporary directory.
 :return: true if the file is well removed, false otherwise
 :rtype: bool
%End

    QString auxiliaryStorageFile() const;
%Docstring
 Returns the current .qgd auxiliary storage file or an empty string if
 there's none
 :rtype: str
%End
};

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/qgsarchive.h                                                *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
