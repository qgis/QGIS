/***************************************************************************
                              qgsformannotation.h
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

#ifndef QGSFORMANNOTATION_H
#define QGSFORMANNOTATION_H

#include "qgsannotation.h"
#include "qgsfeature.h"
#include "qgis_gui.h"

class QGraphicsProxyWidget;

/** \ingroup gui
 * An annotation item that embedds a designer form showing the feature attribute*/
class GUI_EXPORT QgsFormAnnotation: public QgsAnnotation
{
    Q_OBJECT
  public:
    QgsFormAnnotation( QgsVectorLayer* vlayer = nullptr, bool hasFeature = false, int feature = 0 );
    ~QgsFormAnnotation();

    QSizeF minimumFrameSize() const override;
    //! Returns the optimal frame size
    QSizeF preferredFrameSize() const;
#if 0
    void setMapPosition( const QgsPoint& pos ) override;
#endif
    void setDesignerForm( const QString& uiFile );
    QString designerForm() const { return mDesignerForm; }

    virtual void writeXml( QDomElement& elem, QDomDocument & doc ) const override;
    virtual void readXml( const QDomElement& itemElem, const QDomDocument& doc ) override;

    QgsVectorLayer* vectorLayer() const { return mVectorLayer; }

  protected:

    void renderAnnotation( QgsRenderContext& context, QSizeF size ) const override;

  private slots:
    //! Sets a feature for the current map position and updates the dialog
    void setFeatureForMapPosition();
    //! Sets visibility status based on mVectorLayer visibility
    void updateVisibility();

  private:

    QWidget* mDesignerWidget;
    QSize mMinimumSize;
    //! Associated vectorlayer (or 0 if attributes are not supposed to be replaced)
    QgsVectorLayer* mVectorLayer;
    //! True if the item is related to a vector feature
    bool mHasAssociatedFeature;
    //! Associated feature
    QgsFeatureId mFeature;
    //! Path to (and including) the .ui file
    QString mDesignerForm;

    QWidget* createDesignerWidget( const QString& filePath );
};

#endif // QGSFORMANNOTATION_H
