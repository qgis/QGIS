/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/analysis/openstreetmap/qgsosmimport.h                            *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/





class QgsOSMXmlImport : QObject
{
%Docstring
 The QgsOSMXmlImport class imports OpenStreetMap XML format to our topological representation
 in a SQLite database (see QgsOSMDatabase for details).

 How to use the class:
 1. set input XML file name and output DB file name (in constructor or with respective functions)
 2. run import()
 3. check errorString() if the import failed
%End

%TypeHeaderCode
#include "qgsosmimport.h"
%End
  public:
    explicit QgsOSMXmlImport( const QString &xmlFileName = QString(), const QString &dbFileName = QString() );

    void setInputXmlFileName( const QString &xmlFileName );
    QString inputXmlFileName() const;
%Docstring
 :rtype: str
%End

    void setOutputDatabaseFileName( const QString &fileName );
%Docstring
 Sets the filename for the output database.
.. seealso:: outputDatabaseFileName()
%End

    QString outputDatabaseFileName() const;
%Docstring
 Returns the filename for the output database.
.. seealso:: setOutputDatabaseFileName()
 :rtype: str
%End

    bool import();
%Docstring
 Run import. This will parse the XML file and store the data in a SQLite database.
 :return: true on success, false when import failed (see errorString() for the error)
 :rtype: bool
%End

    bool hasError() const;
%Docstring
 :rtype: bool
%End
    QString errorString() const;
%Docstring
 :rtype: str
%End

  signals:
    void progress( int percent );

  protected:

    bool createDatabase();
%Docstring
 :rtype: bool
%End
    bool closeDatabase();
%Docstring
 :rtype: bool
%End


    bool createIndexes();
%Docstring
 :rtype: bool
%End

    void readRoot( QXmlStreamReader &xml );
    void readNode( QXmlStreamReader &xml );
    void readWay( QXmlStreamReader &xml );
    void readTag( bool way, QgsOSMId id, QXmlStreamReader &xml );

};



/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/analysis/openstreetmap/qgsosmimport.h                            *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
