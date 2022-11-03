/***************************************************************************
                         qgscolorrampshaderwidget.h
                         --------------------------
    begin                : Jun 2018
    copyright            : (C) 2018 by Peter Petrik
    email                : zilolv at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSCOLORRAMPSHADERWIDGET_H
#define QGSCOLORRAMPSHADERWIDGET_H

#include <QStyledItemDelegate>

#include "qgis_sip.h"
#include "qgscolorrampshader.h"
#include "qgsrasterrenderer.h"
#include "ui_qgscolorrampshaderwidgetbase.h"
#include "qgis_gui.h"
#include "qgsrasterrendererwidget.h"
#include "qgscolorramplegendnodesettings.h"

class QgsRasterDataProvider;
class QgsLocaleAwareNumericLineEditDelegate;

/**
 * \ingroup gui
 * \class QgsColorRampShaderWidget
 *
 * \brief It has 2 ways how to use it. For raster layers, raster data provider and band is assigned and
 * the Quantile classification mode can be used and the LoadFromBandButton is visible.
 *
 * The other mode is used to style mesh layer contours (scalar datasets)
 *
 * \since QGIS 3.4
 */
class GUI_EXPORT QgsColorRampShaderWidget: public QWidget, protected Ui::QgsColorRampShaderWidgetBase
{

    Q_OBJECT

  public:

    //! Creates new color ramp shader widget
    QgsColorRampShaderWidget( QWidget *parent = nullptr );

    //! Allows quantile classification mode for raster layers
    void initializeForUseWithRasterLayer();

    //! Associates raster with the widget, only when used for raster layer
    void setRasterDataProvider( QgsRasterDataProvider *dp );

    //! Sets raster band, only when used for raster layer
    void setRasterBand( int band );

    //! Sets extent, only when used for raster layer
    void setExtent( const QgsRectangle &extent );

    //! Sets min max and classify color tree
    void setMinimumMaximumAndClassify( double minimum, double maximum );

    //! Sets min max
    void setMinimumMaximum( double minimum, double maximum );

    //! Gets min value
    double minimum() const;

    //! Gets max value
    double maximum() const;

    //! Returns shared function used in the renderer
    QgsColorRampShader shader() const;

    //! Sets widget state from the color ramp shader
    void setFromShader( const QgsColorRampShader &colorRampShader );

  signals:
    //! Color ramp tree has changed
    void minimumMaximumChangedFromTree( double minimum, double maximum );

    //! Widget changed
    void widgetChanged();

    //! Classification mode changed
    void classificationModeChanged( QgsColorRampShader::ClassificationMode mode );

  public slots:

    /**
     * Executes the single band pseudo raster classification
     */
    void classify();

    //! Loads min and max values from color ramp tree
    void loadMinimumMaximumFromTree();

  protected:

    //! Populates color ramp tree from ramp items
    void populateColormapTreeWidget( const QList<QgsColorRampShader::ColorRampItem> &colorRampItems );

  private:

    enum Column
    {
      ValueColumn = 0,
      ColorColumn = 1,
      LabelColumn = 2,
    };

    /**
     * Generate labels from the values in the color map.
     * Skip labels which were manually edited (black text).
     * Text of generated labels is made gray
     */
    void autoLabel();

    //! Extracts the minimal and maximal value from the colormapTreeWidget
    bool colormapMinMax( double &min, double &max ) const;

    //! Extract the unit out of the current labels and set the unit field.
    void setUnitFromLabels();

    QMenu *contextMenu = nullptr;

  private slots:

    void applyColorRamp();
    void updateColorRamp();
    void mAddEntryButton_clicked();
    void mDeleteEntryButton_clicked();
    void mLoadFromBandButton_clicked();
    void mLoadFromFileButton_clicked();
    void mExportToFileButton_clicked();
    void mUnitLineEdit_textEdited( const QString &text ) { Q_UNUSED( text ) autoLabel(); }
    void mColormapTreeWidget_itemDoubleClicked( QTreeWidgetItem *item, int column );
    void mColormapTreeWidget_itemEdited( QTreeWidgetItem *item, int column );
    void mColorInterpolationComboBox_currentIndexChanged( int index );
    void mClassificationModeComboBox_currentIndexChanged( int index );
    void changeColor();
    void changeOpacity();
    void showLegendSettings();

  private:
    void setLineEditValue( QLineEdit *lineEdit, double value );
    double lineEditValue( const QLineEdit *lineEdit ) const;
    void resetClassifyButton();

    QString createLabel( QTreeWidgetItem *item, int row, const QString unit );

#ifdef QGISDEBUG
    //! Dump all the classes for debugging purposes
    void dumpClasses();
#endif

    double mMin = std::numeric_limits<double>::quiet_NaN();
    double mMax = std::numeric_limits<double>::quiet_NaN();

    // For mode with raster layer
    QPointer< QgsRasterDataProvider > mRasterDataProvider;
    int mBand = -1;
    QgsRectangle mExtent;
    QgsLocaleAwareNumericLineEditDelegate *mValueDelegate = nullptr;
    QgsColorRampLegendNodeSettings mLegendSettings;


};

#endif // QGSCOLORRAMPSHADERWIDGET_H
