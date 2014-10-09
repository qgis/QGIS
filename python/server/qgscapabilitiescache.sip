
/**A cache for capabilities xml documents (by configuration file path)*/
class QgsCapabilitiesCache: public QObject
{
%TypeHeaderCode
#include <QDomDocument>
#include <QFileSystemWatcher>
#include <QHash>
#include <QObject>
#include "qgscapabilitiescache.h"
%End
  public:

    /**Returns cached capabilities document (or 0 if document for configuration file not in cache)*/
    const QDomDocument* searchCapabilitiesDocument( QString configFilePath, QString version );
    /**Inserts new capabilities document (creates a copy of the document, does not take ownership)*/
    void insertCapabilitiesDocument( QString configFilePath, QString version, const QDomDocument* doc );

};


