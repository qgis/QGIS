/***************************************************************************
                              qgshtmlannotation.h
                              ------------------------
  begin                : February 9, 2010
  copyright            : (C) 2010 by Marco Hugentobler
  email                : marco dot hugentobler at hugis dot net
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSHTMLANNOTATION_H
#define QGSHTMLANNOTATION_H

#include "qgsannotation.h"
#include "qgsfeature.h"

#include "qgis_core.h"

class QgsWebPage;

/**
 * \class QgsHtmlAnnotation
 * \ingroup core
 * An annotation item that embeds HTML content.
 * \since QGIS 3.0
*/

class CORE_EXPORT QgsHtmlAnnotation: public QgsAnnotation
{
    Q_OBJECT
  public:

    /**
     * Constructor for QgsHtmlAnnotation.
     */
    QgsHtmlAnnotation( QObject *parent SIP_TRANSFERTHIS = nullptr );

    QgsHtmlAnnotation *clone() const override SIP_FACTORY;

    QSizeF minimumFrameSize() const override;

    /**
     * Sets the file path for the source HTML file.
     * \see sourceFile()
     */
    void setSourceFile( const QString &htmlFile );

    /**
     * Returns the file path for the source HTML file.
     * \see setSourceFile()
     */
    QString sourceFile() const { return mHtmlFile; }

    void writeXml( QDomElement &elem, QDomDocument &doc, const QgsReadWriteContext &context ) const override;
    void readXml( const QDomElement &itemElem, const QgsReadWriteContext &context ) override;

    void setAssociatedFeature( const QgsFeature &feature ) override;

    /**
     * Returns a new QgsHtmlAnnotation object.
     */
    static QgsHtmlAnnotation *create() SIP_FACTORY { return new QgsHtmlAnnotation(); }

  protected:

    void renderAnnotation( QgsRenderContext &context, QSizeF size ) const override;

  private slots:

    void javascript();

  private:
    QgsWebPage *mWebPage = nullptr;
    QString mHtmlFile;
    QString mHtmlSource;

};

#endif // QGSHTMLANNOTATION_H
