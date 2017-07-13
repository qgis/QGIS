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
 Class allowing to manage the zip/unzip actions on project
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


    ~QgsArchive();
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

    bool zip();
%Docstring
 Zip the content of this archive. THe current filename is used.
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

    bool unzip();
%Docstring
 Clear the current content of this archive and unzip. If a project file
 is found in the content, then this archive may be considered as a valid
 one. Files are unzipped in the temporary directory. The current filename
 is used.
 :return: true if a project file has been found, false otherwise
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

    void setFileName( const QString &filename );
%Docstring
 Set the filename to use when zipping/unzipping this archive.
 \param filename The zip filename
%End

    QString filename() const;
%Docstring
 Returns the current zip filename.
 :rtype: str
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

  protected:

};

class QgsProjectArchive : QgsArchive
{

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
};

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/qgsarchive.h                                                *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
