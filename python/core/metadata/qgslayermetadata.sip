/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/metadata/qgslayermetadata.h                                 *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/




class QgsLayerMetadata
{

%TypeHeaderCode
#include "qgslayermetadata.h"
%End
  public:

    QgsLayerMetadata();

    virtual ~QgsLayerMetadata();


    QString identifier() const;
%Docstring
 :rtype: str
%End
    void setIdentifier( const QString &identifier );

    QString parentIdentifier() const;
%Docstring
 Returns an empty string if no parent identifier is set.
 :rtype: str
%End


    void setParentIdentifier( const QString &parentIdentifier );

    QString type() const;
%Docstring
 :rtype: str
%End
    void setType( const QString &type );

    QString title() const;
%Docstring
 :rtype: str
%End
    void setTitle( const QString &title );

    QString abstract() const;
%Docstring
 :rtype: str
%End
    void setAbstract( const QString &abstract );

    QString fees() const;
%Docstring
 Returns an empty string if no fees are set.
 :rtype: str
%End
    void setFees( const QString &fees );

    QStringList constraints() const;
%Docstring
 :rtype: list of str
%End
    void setConstraints( const QStringList &constraints );

    QStringList rights() const;
%Docstring
 :rtype: list of str
%End
    void setRights( const QStringList &rights );

    QString encoding() const;
%Docstring
 Returns an empty string if no encoding is set.
 :rtype: str
%End
    void setEncoding( const QString &encoding );

};

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/metadata/qgslayermetadata.h                                 *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
