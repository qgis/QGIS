class QgsRasterIdentifyResult
{
%TypeHeaderCode
#include <qgsrasteridentifyresult.h>
%End
  public:
    QgsRasterIdentifyResult();

    QgsRasterIdentifyResult( QgsRaster::IdentifyFormat theFormat, QMap<int, QVariant> theResults );

    QgsRasterIdentifyResult( QgsError theError );

    virtual ~QgsRasterIdentifyResult();

    bool isValid() const;

    QgsRaster::IdentifyFormat format() const;

    QMap<int, QVariant> results() const;

    void setParams( const QMap<QString, QVariant> & theParams );

    QMap<QString, QVariant> params() const;

    QgsError error() const;

    void setError( const QgsError & theError );
};

