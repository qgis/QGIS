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

/** \ingroup gui
 * \class QgsSingleBandPseudoColorRendererWidget
 */
class GUI_EXPORT QgsSingleBandPseudoColorRendererWidget: public QgsRasterRendererWidget,
      private Ui::QgsSingleBandPseudoColorRendererWidgetBase
{
    Q_OBJECT
  public:
    enum Mode
    {
      Continuous = 1, // Using breaks from color palette
      EqualInterval = 2,
      Quantile = 3
    };

    QgsSingleBandPseudoColorRendererWidget( QgsRasterLayer* layer, const QgsRectangle &extent = QgsRectangle() );
    ~QgsSingleBandPseudoColorRendererWidget();

    static QgsRasterRendererWidget* create( QgsRasterLayer* layer, const QgsRectangle &theExtent ) { return new QgsSingleBandPseudoColorRendererWidget( layer, theExtent ); }
    QgsRasterRenderer* renderer() override;
    void setMapCanvas( QgsMapCanvas* canvas ) override;

    void setFromRenderer( const QgsRasterRenderer* r );

  public slots:
    void loadMinMax( int theBandNo, double theMin, double theMax, int theOrigin );

  private:
    enum Column
    {
      ValueColumn = 0,
      ColorColumn = 1,
      LabelColumn = 2,
    };

    void populateColormapTreeWidget( const QList<QgsColorRampShader::ColorRampItem>& colorRampItems );
    void autoLabel();
    void setUnitFromLabels();

  private slots:
    void on_mAddEntryButton_clicked();
    void on_mDeleteEntryButton_clicked();
    void on_mNumberOfEntriesSpinBox_valueChanged();
    void on_mClassifyButton_clicked();
    void on_mLoadFromBandButton_clicked();
    void on_mLoadFromFileButton_clicked();
    void on_mExportToFileButton_clicked();
    void on_mUnitLineEdit_textEdited( const QString & text ) { Q_UNUSED( text ); autoLabel(); }
    void on_mColormapTreeWidget_itemDoubleClicked( QTreeWidgetItem* item, int column );
    void mColormapTreeWidget_itemEdited( QTreeWidgetItem* item, int column );
    void on_mBandComboBox_currentIndexChanged( int index );
    void on_mColorInterpolationComboBox_currentIndexChanged( int index );
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

/** \ingroup gui
 * Custom QTreeWidgetItem with extra signal when item is edited and numeric sorting.
 */
class GUI_EXPORT QgsTreeWidgetItem: public QObject, public QTreeWidgetItem
{
    Q_OBJECT
  public:
    /** Constructs a tree widget item of the specified type and appends it to the items in the given parent. */
    explicit QgsTreeWidgetItem( QTreeWidget * parent, int type = Type ) : QTreeWidgetItem( parent, type ) {}

    /** Sets the value for the item's column and role to the given value. */
    virtual void setData( int column, int role, const QVariant & value );
    virtual bool operator< ( const QTreeWidgetItem & other ) const;

  signals:
    /** This signal is emitted when the contents of the column in the specified item has been edited by the user. */
    void itemEdited( QTreeWidgetItem* item, int column );
};

#endif // QGSSINGLEBANDCOLORRENDERERWIDGET_H
