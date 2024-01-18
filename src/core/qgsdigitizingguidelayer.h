/***************************************************************************
    qgsdigitizingguidelayer.h
    ----------------------
    begin                : August 2023
    copyright            : (C) Denis Rouzaud
    email                : denis@opengis.ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


#ifndef QGSDIGITIZINGGUIDELAYER_H
#define QGSDIGITIZINGGUIDELAYER_H

#include "qgis_core.h"
#include "qgsannotationlayer.h"
#include "qgsgeometry.h"

#define SIP_NO_FILE

class QgsDigitizingGuideModel;
class QgsAnnotationMarkerItem;
class QgsAnnotationLineItem;
class QgsAnnotationPointTextItem;
class QgsMarkerSymbol;
class QgsLineSymbol;

/**
 * \ingroup core
 * @brief The QgsDigitizingGuideLayer class holds map guides information saved in the project file.
 *
 * \since QGIS 3.36
 */
class CORE_EXPORT QgsDigitizingGuideLayer : public QgsAnnotationLayer
{
    Q_OBJECT
  public:

    //! Constructor
    QgsDigitizingGuideLayer( const QString &name, const QgsAnnotationLayer::LayerOptions &options );

    //! Enables or disabled the guides
    void setEnabled( bool enabled );

    //! Adds a point guide
    void addPointGuide( const QgsPoint &point, const QString &title, QList<QgsAnnotationItem *> details = QList<QgsAnnotationItem *>(), const QDateTime &creation = QDateTime::currentDateTime() );

    //! Adds a line guide
    void addLineGuide( QgsCurve *curve SIP_TRANSFER, const QString &title, QList<QgsAnnotationItem *> details = QList<QgsAnnotationItem *>(), const QDateTime &creation = QDateTime::currentDateTime() );

    //! Return the guides
    std::pair<QList<QgsPointXY>, QList<const QgsCurve *> > guides() const;

    //! Returns the model
    QgsDigitizingGuideModel *model() const {return mModel;}

    /**
     * Creates an additional point guide without adding it to the annotation layer
     * Additional items are not shown by default on the map
     */
    QgsAnnotationItem *createDetailsPoint( const QgsPoint &point );

    /**
     * Creates an additional line guide without adding it to the annotation layer
     * Additional items are not shown by default on the map
     */
    QgsAnnotationItem *createDetailsLine( QgsCurve *curve );

    //! Creates an additional text line item without adding to the annotation layer
    QgsAnnotationItem *createDetailsPointTextGuide( const QString &text, const QgsPoint &point, double angle );

    //! Highlights or removes highlight of a guide in the map
    void setGuideHighlight( const QString &guideId = QString() );

    virtual void clear() override;

  protected:
    virtual bool readXml( const QDomNode &node, QgsReadWriteContext &context ) override;
    virtual bool writeXml( QDomNode &layer_node, QDomDocument &doc, const QgsReadWriteContext &context ) const override;

  private:
    QStringList addDetails( QList<QgsAnnotationItem *> details );

    QgsMarkerSymbol *pointGuideSymbol() const;
    QgsMarkerSymbol *highlightedPointGuideSymbol() const;
    QgsMarkerSymbol *detailsPointSymbol() const;

    QgsLineSymbol *lineGuideSymbol() const;
    QgsLineSymbol *highlightedLineGuideSymbol() const;
    QgsLineSymbol *detailsLineSymbol() const;

    QgsDigitizingGuideModel *mModel = nullptr;
    QString mHighlightItemId;

};

#endif // QGSDIGITIZINGGUIDELAYER_H
