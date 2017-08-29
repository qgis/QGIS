/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/qgsauxiliarystorage.h                                       *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/







class QgsAuxiliaryLayer : QgsVectorLayer
{
%Docstring


 Class allowing to manage the auxiliary storage for a vector layer

.. versionadded:: 3.0
%End

%TypeHeaderCode
#include "qgsauxiliarystorage.h"
%End
  public:

    QgsAuxiliaryLayer( const QString &pkField, const QString &filename, const QString &table, const QgsVectorLayer *vlayer );
%Docstring
 Constructor

 \param pkField The primary key to use for joining
 \param filename The database path
 \param table The table name
 \param vlayer The target vector layer in join definition
%End

    virtual ~QgsAuxiliaryLayer();
%Docstring
 Destructor
%End



    QgsVectorLayerJoinInfo joinInfo() const;
%Docstring
 Returns information to use for joining with primary key and so on.
 :rtype: QgsVectorLayerJoinInfo
%End

    bool save();
%Docstring
 Commit changes and starts editing then.

 :return: true if commit step passed, false otherwise
 :rtype: bool
%End

};


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

    QgsAuxiliaryLayer *createAuxiliaryLayer( const QgsField &field, const QgsVectorLayer *layer ) const;
%Docstring
 Creates an auxiliary layer for a vector layer. A new table is created if
 necessary. The primary key to use to construct the auxiliary layer is
 given in parameter.

 \param field The primary key to join
 \param layer The vector layer for which the auxiliary layer has to be created

 :return: A new auxiliary layer or a None if an error happened.
 :rtype: QgsAuxiliaryLayer
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
