/***************************************************************************
                              qgstextannotation.h
                              -------------------
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

#ifndef QGSTEXTANNOTATION_H
#define QGSTEXTANNOTATION_H

#include "qgsannotation.h"
#include <QTextDocument>
#include "qgis_core.h"

/**
 * \class QgsTextAnnotation
 * \ingroup core
 * An annotation item that displays formatted text from a QTextDocument document.
 * \since QGIS 3.0
*/
class CORE_EXPORT QgsTextAnnotation: public QgsAnnotation
{
    Q_OBJECT

  public:

    /**
     * Constructor for QgsTextAnnotation.
     */
    QgsTextAnnotation( QObject *parent SIP_TRANSFERTHIS = nullptr );

    QgsTextAnnotation *clone() const override SIP_FACTORY;

    /**
     * Returns the text document which will be rendered
     * within the annotation.
     * \see setDocument()
     */
    const QTextDocument *document() const;

    /**
     * Sets the text document which will be rendered
     * within the annotation. Ownership is not transferred.
     * \see document()
     */
    void setDocument( const QTextDocument *doc );

    void writeXml( QDomElement &elem, QDomDocument &doc, const QgsReadWriteContext &context ) const override;
    void readXml( const QDomElement &itemElem, const QgsReadWriteContext &context ) override;

    /**
     * Returns a new QgsTextAnnotation object.
     */
    static QgsTextAnnotation *create() SIP_FACTORY { return new QgsTextAnnotation(); }

  protected:

    void renderAnnotation( QgsRenderContext &context, QSizeF size ) const override;

  private:
    std::unique_ptr< QTextDocument > mDocument;
};

#endif // QGSTEXTANNOTATION_H
