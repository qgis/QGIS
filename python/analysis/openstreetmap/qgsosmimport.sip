
/**
 * @brief The QgsOSMXmlImport class imports OpenStreetMap XML format to our topological representation
 * in a SQLite database (see QgsOSMDatabase for details).
 *
 * How to use the classs:
 * 1. set input XML file name and output DB file name (in constructor or with respective functions)
 * 2. run import()
 * 3. check errorString() if the import failed
 */
class QgsOSMXmlImport : public QObject
{
%TypeHeaderCode
#include <qgsosmimport.h>
%End
  public:
    explicit QgsOSMXmlImport( const QString& xmlFileName = QString(), const QString& dbFileName = QString() );

    void setInputXmlFileName( const QString& xmlFileName );
    QString inputXmlFileName() const;

    void setOutputDbFileName( const QString& dbFileName );
    QString outputDbFileName() const;

    /**
     * Run import. This will parse the XML file and store the data in a SQLite database.
     * @return true on success, false when import failed (see errorString() for the error)
     */
    bool import();

    bool hasError() const;
    QString errorString() const;

  signals:
    void progress( int percent );

  protected:

    bool createDatabase();
    bool closeDatabase();

    //! @note not available in Python bindings
    //void deleteStatement( sqlite3_stmt*& stmt );

    bool createIndexes();

    void readRoot( QXmlStreamReader& xml );
    void readNode( QXmlStreamReader& xml );
    void readWay( QXmlStreamReader& xml );
    void readTag( bool way, qint64 id, QXmlStreamReader& xml );

};

