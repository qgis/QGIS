/***************************************************************************
                         qgssinglebandpseudocolorrendererwidget.h
                         ----------------------------------------
    begin                : February 2012
    copyright            : (C) 2012 by Marco Hugentobler
    email                : marco at sourcepole dot ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSSINGLEBANDCOLORRENDERERWIDGET_H
#define QGSSINGLEBANDCOLORRENDERERWIDGET_H

#include "qgsrasterrendererwidget.h"
#include "qgis_sip.h"
#include "qgscolorrampshader.h"
#include "qgsrasterrenderer.h"
#include "ui_qgssinglebandpseudocolorrendererwidgetbase.h"
#include "qgis_gui.h"

class QgsRasterMinMaxWidget;

/**
 * \ingroup gui
 * \class QgsSingleBandPseudoColorRendererWidget
 */
class GUI_EXPORT QgsSingleBandPseudoColorRendererWidget: public QgsRasterRendererWidget, private Ui::QgsSingleBandPseudoColorRendererWidgetBase
{

    Q_OBJECT

  public:

    QgsSingleBandPseudoColorRendererWidget( QgsRasterLayer *layer, const QgsRectangle &extent = QgsRectangle() );

    static QgsRasterRendererWidget *create( QgsRasterLayer *layer, const QgsRectangle &extent ) SIP_FACTORY { return new QgsSingleBandPseudoColorRendererWidget( layer, extent ); }
    QgsRasterRenderer *renderer() override;
    void setMapCanvas( QgsMapCanvas *canvas ) override;
    void doComputations() override;
    QgsRasterMinMaxWidget *minMaxWidget() override { return mMinMaxWidget; }

    void setFromRenderer( const QgsRasterRenderer *r );

  public slots:

    /**
     * Executes the single band pseudo raster classficiation
     */
    void classify();
    //! called when new min/max values are loaded
    void loadMinMax( int bandNo, double min, double max );

  private:

    enum Column
    {
      ValueColumn = 0,
      ColorColumn = 1,
      LabelColumn = 2,
    };

    void populateColormapTreeWidget( const QList<QgsColorRampShader::ColorRampItem> &colorRampItems );

    /**
     * Generate labels from the values in the color map.
     *  Skip labels which were manually edited (black text).
     *  Text of generated labels is made gray
     */
    void autoLabel();

    //! Extract the unit out of the current labels and set the unit field.
    void setUnitFromLabels();

    QMenu *contextMenu = nullptr;

  private slots:

    void applyColorRamp();
    void mAddEntryButton_clicked();
    void mDeleteEntryButton_clicked();
    void mLoadFromBandButton_clicked();
    void mLoadFromFileButton_clicked();
    void mExportToFileButton_clicked();
    void mUnitLineEdit_textEdited( const QString &text ) { Q_UNUSED( text ); autoLabel(); }
    void mColormapTreeWidget_itemDoubleClicked( QTreeWidgetItem *item, int column );
    void mColormapTreeWidget_itemEdited( QTreeWidgetItem *item, int column );
    void bandChanged();
    void mColorInterpolationComboBox_currentIndexChanged( int index );
    void mMinLineEdit_textChanged( const QString & ) { resetClassifyButton(); }
    void mMaxLineEdit_textChanged( const QString & ) { resetClassifyButton(); }
    void mMinLineEdit_textEdited( const QString &text );
    void mMaxLineEdit_textEdited( const QString &text );
    void mClassificationModeComboBox_currentIndexChanged( int index );
    void changeColor();
    void changeOpacity();

  private:

    void setLineEditValue( QLineEdit *lineEdit, double value );
    double lineEditValue( const QLineEdit *lineEdit ) const;
    void resetClassifyButton();
    QgsRasterMinMaxWidget *mMinMaxWidget = nullptr;
    bool mDisableMinMaxWidgetRefresh;
    int mMinMaxOrigin;

    void minMaxModified();
};


#endif // QGSSINGLEBANDCOLORRENDERERWIDGET_H
