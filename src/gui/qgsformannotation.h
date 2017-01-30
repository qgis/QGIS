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
#include <QWidget>
#include "qgis_gui.h"

/** \ingroup gui
 * An annotation item that embedds a designer form showing the feature attribute*/
class GUI_EXPORT QgsFormAnnotation: public QgsAnnotation
{
    Q_OBJECT
  public:

    /**
     * Constructor for QgsFormAnnotation.
     */
    QgsFormAnnotation( QObject* parent = nullptr );

    QSizeF minimumFrameSize() const override;
    //! Returns the optimal frame size
    QSizeF preferredFrameSize() const;

    /**
     * Sets the path to the Qt Designer UI file to show in the annotation.
     * @see designerForm()
     */
    void setDesignerForm( const QString& uiFile );

    /**
     * Returns the path to the Qt Designer UI file to show in the annotation.
     * @see setDesignerForm()
     */
    QString designerForm() const { return mDesignerForm; }

    virtual void writeXml( QDomElement& elem, QDomDocument & doc ) const override;
    virtual void readXml( const QDomElement& itemElem, const QDomDocument& doc ) override;

    void setAssociatedFeature( const QgsFeature& feature ) override;

    /**
     * Returns a new QgsFormAnnotation object.
     */
    static QgsFormAnnotation* create() { return new QgsFormAnnotation(); }

  protected:

    void renderAnnotation( QgsRenderContext& context, QSizeF size ) const override;

  private:

    QScopedPointer<QWidget> mDesignerWidget;
    QSize mMinimumSize;
    //! Path to (and including) the .ui file
    QString mDesignerForm;

    QWidget* createDesignerWidget( const QString& filePath );
};

#endif // QGSFORMANNOTATION_H
