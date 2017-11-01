/***************************************************************************
                              qgscompositionwidget.h
                             ------------------------
    begin                : June 11 2008
    copyright            : (C) 2008 by Marco Hugentobler
    email                : marco dot hugentobler at karto dot baug dot ethz dot ch
 ***************************************************************************/
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "ui_qgscompositionwidgetbase.h"
#include "qgspanelwidget.h"

class QgsComposition;
class QgsComposerMap;

/**
 * \ingroup app
 * Struct to hold map composer paper properties.
 */
struct QgsCompositionPaper
{
  QgsCompositionPaper( const QString &name, double width, double height ) {mName = name; mWidth = width; mHeight = height;}
  QString mName;
  double mWidth;
  double mHeight;
};

/**
 * \ingroup app
  * Input widget for QgsComposition
  */
class QgsCompositionWidget: public QgsPanelWidget, private Ui::QgsCompositionWidgetBase
{
    Q_OBJECT
  public:
    QgsCompositionWidget( QWidget *parent, QgsComposition *c );

  public slots:
    void mPaperSizeComboBox_currentIndexChanged( const QString &text );
    void mPaperUnitsComboBox_currentIndexChanged( const QString &text );
    void mPaperOrientationComboBox_currentIndexChanged( const QString &text );
    void mPaperWidthDoubleSpinBox_editingFinished();
    void mPaperHeightDoubleSpinBox_editingFinished();
    void mNumPagesSpinBox_valueChanged( int value );
    void mPageStyleButton_clicked();
    void mResizePageButton_clicked();
    void mResolutionSpinBox_valueChanged( int value );
    void mPrintAsRasterCheckBox_toggled( bool state );
    void mGenerateWorldFileCheckBox_toggled( bool state );
    void referenceMapChanged( QgsComposerItem * );

    void mGridResolutionSpinBox_valueChanged( double d );
    void mOffsetXSpinBox_valueChanged( double d );
    void mOffsetYSpinBox_valueChanged( double d );
    void mSnapToleranceSpinBox_valueChanged( int tolerance );

    //! Sets GUI elements to width/height from composition
    void displayCompositionWidthHeight();
    //! Sets Print as raster checkbox value
    void setPrintAsRasterCheckBox( bool state );
    //! Sets number of pages spin box value
    void setNumberPages();

  signals:
    //! Is emitted when page orientation changes
    void pageOrientationChanged( const QString &orientation );

  private slots:

    //! Must be called when a data defined button changes
    void updateDataDefinedProperty();

    //! Initializes data defined buttons to current atlas coverage layer
    void populateDataDefinedButtons();

    void variablesChanged();

    void resizeMarginsChanged();

    void updateVariables();

    void updateStyleFromWidget();
    void cleanUpStyleSelector( QgsPanelWidget *container );

  private:
    QgsComposition *mComposition = nullptr;
    QMap<QString, QgsCompositionPaper> mPaperMap;

    QgsCompositionWidget(); //default constructor is forbidden
    //! Sets width/height to chosen paper format and updates paper item
    void applyCurrentPaperSettings();
    //! Applies the current width and height values
    void applyWidthHeight();
    //! Makes sure width/height values for custom paper matches the current orientation
    void adjustOrientation();
    //! Sets GUI elements to snapping distances of composition
    void displaySnappingSettings();

    void updatePageStyle();

    void createPaperEntries();
    void insertPaperEntries();

    double size( QDoubleSpinBox *spin );
    void setSize( QDoubleSpinBox *spin, double v );
    //! Blocks / unblocks the signals of all items
    void blockSignals( bool block );

    //! Sets a data defined property for the item from its current data defined button settings
    void setDataDefinedProperty( const QgsPropertyOverrideButton *ddBtn, QgsComposerObject::DataDefinedProperty property );

};
