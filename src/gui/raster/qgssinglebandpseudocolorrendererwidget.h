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

#include "qgsrasterminmaxwidget.h"
#include "qgsrasterrendererwidget.h"
#include "qgscolorrampshader.h"
#include "ui_qgssinglebandpseudocolorrendererwidgetbase.h"

class GUI_EXPORT QgsSingleBandPseudoColorRendererWidget: public QgsRasterRendererWidget,
      private Ui::QgsSingleBandPseudoColorRendererWidgetBase
{
    Q_OBJECT
  public:
    enum Mode
    {
      Continuous = 1, // Using breaks from color palette
      EqualInterval = 2
    };

    QgsSingleBandPseudoColorRendererWidget( QgsRasterLayer* layer, const QgsRectangle &extent = QgsRectangle() );
    ~QgsSingleBandPseudoColorRendererWidget();

    static QgsRasterRendererWidget* create( QgsRasterLayer* layer, const QgsRectangle &theExtent ) { return new QgsSingleBandPseudoColorRendererWidget( layer, theExtent ); }
    QgsRasterRenderer* renderer() override;

    void setFromRenderer( const QgsRasterRenderer* r );

  public slots:
    void loadMinMax( int theBandNo, double theMin, double theMax, int theOrigin );

  private:
    void populateColormapTreeWidget( const QList<QgsColorRampShader::ColorRampItem>& colorRampItems );

  private slots:
    void on_mAddEntryButton_clicked();
    void on_mDeleteEntryButton_clicked();
    void on_mSortButton_clicked();
    void on_mClassifyButton_clicked();
    void on_mLoadFromBandButton_clicked();
    void on_mLoadFromFileButton_clicked();
    void on_mExportToFileButton_clicked();
    void on_mColormapTreeWidget_itemDoubleClicked( QTreeWidgetItem* item, int column );
    void on_mBandComboBox_currentIndexChanged( int index );
    void on_mMinLineEdit_textChanged( const QString & text ) { Q_UNUSED( text ); resetClassifyButton(); }
    void on_mMaxLineEdit_textChanged( const QString & text ) { Q_UNUSED( text ); resetClassifyButton(); }
    void on_mMinLineEdit_textEdited( const QString & text ) { Q_UNUSED( text ); mMinMaxOrigin = QgsRasterRenderer::MinMaxUser; showMinMaxOrigin(); }
    void on_mMaxLineEdit_textEdited( const QString & text ) { Q_UNUSED( text ); mMinMaxOrigin = QgsRasterRenderer::MinMaxUser; showMinMaxOrigin(); }
    void on_mClassificationModeComboBox_currentIndexChanged( int index );
    void on_mColorRampComboBox_currentIndexChanged( int index );

  private:
    void setLineEditValue( QLineEdit *theLineEdit, double theValue );
    double lineEditValue( const QLineEdit *theLineEdit ) const;
    void resetClassifyButton();
    void showMinMaxOrigin();
    QgsRasterMinMaxWidget * mMinMaxWidget;
    int mMinMaxOrigin;
};

#endif // QGSSINGLEBANDCOLORRENDERERWIDGET_H
