class QgsTransaction
{
%TypeHeaderCode
#include <qgstransaction.h>
%End
    public:
        QgsTransaction( const QString& connString, const QString& providerKey );
        ~QgsTransaction();

        bool addLayer( const QString& layerId );

        bool begin( QString& errorMsg );
        bool commit( QString& errorMsg );
        bool rollback( QString& errorMsg );
        bool executeSql( QString& errorMsg ); //error message?
};
