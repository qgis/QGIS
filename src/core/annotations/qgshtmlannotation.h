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
 * \note added in QGIS 3.0
*/

class CORE_EXPORT QgsHtmlAnnotation: public QgsAnnotation
{
    Q_OBJECT
  public:

    /**
     * Constructor for QgsHtmlAnnotation.
     */
    QgsHtmlAnnotation( QObject* parent = nullptr, QgsVectorLayer* vlayer = nullptr, bool hasFeature = false, int feature = 0 );

    ~QgsHtmlAnnotation();

    QSizeF minimumFrameSize() const override;
#if 0
    void setMapPosition( const QgsPoint& pos ) override;
#endif

    /**
     * Sets the file path for the source HTML file.
     * @see sourceFile()
     */
    void setSourceFile( const QString& htmlFile );

    /**
     * Returns the file path for the source HTML file.
     * @see setSourceFile()
     */
    QString sourceFile() const { return mHtmlFile; }

    virtual void writeXml( QDomElement& elem, QDomDocument & doc ) const override;
    virtual void readXml( const QDomElement& itemElem, const QDomDocument& doc ) override;

    /**
     * Returns the vector layer associated with the annotation.
     */
    QgsVectorLayer* vectorLayer() const { return mVectorLayer; }

  protected:

    void renderAnnotation( QgsRenderContext& context, QSizeF size ) const override;

  private slots:
    //! Sets a feature for the current map position and updates the dialog
    void setFeatureForMapPosition();
    //! Sets visibility status based on mVectorLayer visibility
    void updateVisibility();

    void javascript();

  private:
    QgsWebPage* mWebPage;
    //! Associated vectorlayer (or 0 if attributes are not supposed to be replaced)
    QgsVectorLayer* mVectorLayer;
    //! True if the item is related to a vector feature
    bool mHasAssociatedFeature;
    //! Associated feature
    QgsFeatureId mFeatureId;
    QgsFeature mFeature;
    QString mHtmlFile;
    QString mHtmlSource;

    QString replaceText( QString displayText, QgsVectorLayer *layer, QgsFeature &feat );
};

#endif // QGSHTMLANNOTATION_H
