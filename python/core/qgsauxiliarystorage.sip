/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/qgsauxiliarystorage.h                                       *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/







class QgsAuxiliaryStorage
{
%Docstring


 Class providing some utility methods to manage auxiliary storage.

.. versionadded:: 3.0
%End

%TypeHeaderCode
#include "qgsauxiliarystorage.h"
%End
  public:

    QgsAuxiliaryStorage( const QgsProject &project, bool copy = true );
%Docstring
 Constructor.

 \param project The project for which the auxiliary storage has to be used
 \param copy Parameter indicating if a copy of the database has to be used
%End

    QgsAuxiliaryStorage( const QString &filename = QString(), bool copy = true );
%Docstring
 Constructor.

 \param filename The path of the database
 \param copy Parameter indicating if a copy of the database has to be used
%End

    virtual ~QgsAuxiliaryStorage();
%Docstring
 Destructor.
%End

    bool isValid() const;
%Docstring
 Returns the status of the auxiliary storage currently definied.

 :return: true if the auxiliary storage is valid, false otherwise
 :rtype: bool
%End

    QString fileName() const;
%Docstring
 Returns the target filename of the database.
 :rtype: str
%End

    QString currentFileName() const;
%Docstring
 Returns the path of current database used. It may be different from the
 target filename if the auxiliary storage is opened in copy mode.
 :rtype: str
%End

    bool saveAs( const QString &filename ) const;
%Docstring
 Saves the current database to a new path.

 :return: true if everything is saved, false otherwise
 :rtype: bool
%End

    bool saveAs( const QgsProject &project ) const;
%Docstring
 Saves the current database to a new path for a specific project.
 Actually, the current filename of the project is used to deduce the
 path of the database to save.

 :return: true if everything is saved, false otherwise
 :rtype: bool
%End

    bool save() const;
%Docstring
 Saves the current database.

 :return: true if everything is saved, false otherwise
 :rtype: bool
%End

    static QString extension();
%Docstring
 Returns the extension used for auxiliary databases.
 :rtype: str
%End

};

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/qgsauxiliarystorage.h                                       *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
