class QgsGeometryValidator : QThread
{
%TypeHeaderCode
#include <qgsgeometryvalidator.h>
%End

  public:
    //! Constructor
    QgsGeometryValidator( const QgsGeometry *g, QList<QgsGeometry::Error> *errors = 0 );
    ~QgsGeometryValidator();

    void run();
    void stop();

    /** Validate geometry and produce a list of geometry errors */
    static void validateGeometry( const QgsGeometry *g, QList<QgsGeometry::Error> &errors );

    /** Attempts to create a valid representation of a given invalid geometry without loosing any of the input vertices.
     * @note added in 2.4
     **/
    static QgsGeometry *makeValidGeometry( const QgsGeometry *g );

  signals:
    void errorFound( QgsGeometry::Error );

  public slots:
    void addError( QgsGeometry::Error );
};
