
/**Exception class for WMS service exceptions. The most important codes are:
"InvalidFormat"
"Invalid CRS"
"LayerNotDefined" / "StyleNotDefined"
"OperationNotSupported"*/

class QgsMapServiceException
{
%TypeHeaderCode
#include <qgsmapserviceexception.h>
%End
  public:
    QgsMapServiceException( const QString& code, const QString& message );
    QString code() const;
    QString message() const;

};
