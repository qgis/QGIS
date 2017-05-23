/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/qgsfieldvalidator.h                                          *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/




class QgsFieldValidator : QValidator
{

%TypeHeaderCode
#include "qgsfieldvalidator.h"
%End
  public:
    QgsFieldValidator( QObject *parent, const QgsField &field, const QString &defaultValue, const QString &dateFormat = "yyyy-MM-dd" );
    ~QgsFieldValidator();

    virtual State validate( QString &SIP_CONSTRAINED, int &/In,Out/ ) const;
    virtual void fixup( QString & ) const;

    QString dateFormat() const;
%Docstring
 :rtype: str
%End

};

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/qgsfieldvalidator.h                                          *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
