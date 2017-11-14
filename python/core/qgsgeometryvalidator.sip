/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/qgsgeometryvalidator.h                                      *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/



class QgsGeometryValidator : QThread
{

%TypeHeaderCode
#include "qgsgeometryvalidator.h"
%End
  public:

    QgsGeometryValidator( const QgsGeometry &geoemtry, QVector<QgsGeometry::Error> *errors = 0, QgsGeometry::ValidationMethod method = QgsGeometry::ValidatorQgisInternal );
%Docstring
 Constructor for QgsGeometryValidator.
%End
    ~QgsGeometryValidator();

    virtual void run();

    void stop();

    static void validateGeometry( const QgsGeometry &geometry, QVector<QgsGeometry::Error> &errors /Out/, QgsGeometry::ValidationMethod method = QgsGeometry::ValidatorQgisInternal );
%Docstring
Validate geometry and produce a list of geometry errors
%End

  signals:
    void errorFound( const QgsGeometry::Error & );

  public slots:
    void addError( const QgsGeometry::Error & );

};

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/qgsgeometryvalidator.h                                      *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
