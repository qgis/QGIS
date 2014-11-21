class QgsGeometryValidator : QThread
{
%TypeHeaderCode
#include <qgsgeometryvalidator.h>
%End

  public:
    //! Constructor
    QgsGeometryValidator( QgsGeometry *g, QList<QgsGeometry::Error> *errors = 0 );
    ~QgsGeometryValidator();

    void run();
    void stop();

    /** Validate geometry and produce a list of geometry errors */
    static void validateGeometry( QgsGeometry *g, QList<QgsGeometry::Error> &errors );

  signals:
    void errorFound( QgsGeometry::Error );

  public slots:
    void addError( QgsGeometry::Error );
};
