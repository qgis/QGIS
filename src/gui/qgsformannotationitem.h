/***************************************************************************
                              qgsformannotationitem.h
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

#ifndef QGSFORMANNOTATIONITEM_H
#define QGSFORMANNOTATIONITEM_H

#include "qgsannotationitem.h"
#include "qgsfeature.h"
#include <QObject>

class QGraphicsProxyWidget;

/** \ingroup gui
 * An annotation item that embedds a designer form showing the feature attribute*/
class GUI_EXPORT QgsFormAnnotationItem: public QObject, public QgsAnnotationItem
{
    Q_OBJECT
  public:
    QgsFormAnnotationItem( QgsMapCanvas* canvas, QgsVectorLayer* vlayer = nullptr, bool hasFeature = false, int feature = 0 );
    ~QgsFormAnnotationItem();

    void paint( QPainter * painter ) override;

    //! paint function called by map canvas
    void paint( QPainter * painter, const QStyleOptionGraphicsItem * option, QWidget * widget = nullptr ) override;

    QSizeF minimumFrameSize() const override;
    /** Returns the optimal frame size*/
    QSizeF preferredFrameSize() const;

    /** Reimplemented from QgsAnnotationItem*/
    void setMapPosition( const QgsPoint& pos ) override;

    void setDesignerForm( const QString& uiFile );
    QString designerForm() const { return mDesignerForm; }

    void writeXML( QDomDocument& doc ) const override;
    void readXML( const QDomDocument& doc, const QDomElement& itemElem ) override;

    QgsVectorLayer* vectorLayer() const { return mVectorLayer; }

  private slots:
    /** Sets a feature for the current map position and updates the dialog*/
    void setFeatureForMapPosition();
    /** Sets visibility status based on mVectorLayer visibility*/
    void updateVisibility();

  private:
    QGraphicsProxyWidget* mWidgetContainer;
    QWidget* mDesignerWidget;
    /** Associated vectorlayer (or 0 if attributes are not supposed to be replaced)*/
    QgsVectorLayer* mVectorLayer;
    /** True if the item is related to a vector feature*/
    bool mHasAssociatedFeature;
    /** Associated feature*/
    QgsFeatureId mFeature;
    /** Path to (and including) the .ui file*/
    QString mDesignerForm;

    QWidget* createDesignerWidget( const QString& filePath );
};

#endif // QGSFORMANNOTATIONITEM_H
