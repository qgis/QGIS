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
#include "qgscomposeritem.h"

class QgsComposition;
class QgsComposerMap;
class QgsDataDefinedButton;

/** \ingroup MapComposer
 * Struct to hold map composer paper properties.
 */
struct QgsCompositionPaper
{
  QgsCompositionPaper( QString name, double width, double height ) {mName = name; mWidth = width; mHeight = height;}
  QString mName;
  double mWidth;
  double mHeight;
};

/** \ingroup MapComposer
  * Input widget for QgsComposition
  */
class QgsCompositionWidget: public QWidget, private Ui::QgsCompositionWidgetBase
{
    Q_OBJECT
  public:
    QgsCompositionWidget( QWidget* parent, QgsComposition* c );
    ~QgsCompositionWidget();

  public slots:
    void on_mPaperSizeComboBox_currentIndexChanged( const QString& text );
    void on_mPaperUnitsComboBox_currentIndexChanged( const QString& text );
    void on_mPaperOrientationComboBox_currentIndexChanged( const QString& text );
    void on_mPaperWidthDoubleSpinBox_editingFinished();
    void on_mPaperHeightDoubleSpinBox_editingFinished();
    void on_mNumPagesSpinBox_valueChanged( int value );
    void on_mPageStyleButton_clicked();
    void on_mResolutionSpinBox_valueChanged( const int value );
    void on_mPrintAsRasterCheckBox_toggled( bool state );
    void on_mGenerateWorldFileCheckBox_toggled( bool state );
    void on_mWorldFileMapComboBox_currentIndexChanged( int index );

    void on_mGridResolutionSpinBox_valueChanged( double d );
    void on_mOffsetXSpinBox_valueChanged( double d );
    void on_mOffsetYSpinBox_valueChanged( double d );
    void on_mSnapToleranceSpinBox_valueChanged( int tolerance );

    /** Sets GUI elements to width/height from composition*/
    void displayCompositionWidthHeight();
    /** Sets Print as raster checkbox value*/
    void setPrintAsRasterCheckBox( bool state );
    /** Sets number of pages spin box value*/
    void setNumberPages();

  signals:
    /** Is emitted when page orientation changes*/
    void pageOrientationChanged( QString orientation );

  private slots:
    /* when a new map is added */
    void onComposerMapAdded( QgsComposerMap* );
    /* when a map is deleted */
    void onItemRemoved( QgsComposerItem* );

    /** Must be called when a data defined button changes*/
    void updateDataDefinedProperty();

    /** Initializes data defined buttons to current atlas coverage layer*/
    void populateDataDefinedButtons();

    void variablesChanged();

  private:
    QgsComposition* mComposition;
    QMap<QString, QgsCompositionPaper> mPaperMap;

    QgsCompositionWidget(); //default constructor is forbidden
    /** Sets width/height to chosen paper format and updates paper item*/
    void applyCurrentPaperSettings();
    /** Applies the current width and height values*/
    void applyWidthHeight();
    /** Makes sure width/height values for custom paper matches the current orientation*/
    void adjustOrientation();
    /** Sets GUI elements to snaping distances of composition*/
    void displaySnapingSettings();

    void updatePageStyle();

    void createPaperEntries();
    void insertPaperEntries();

    double size( QDoubleSpinBox *spin );
    void setSize( QDoubleSpinBox *spin, double v );
    /** Blocks / unblocks the signals of all items*/
    void blockSignals( bool block );

    /** Sets a data defined property for the item from its current data defined button settings*/
    void setDataDefinedProperty( const QgsDataDefinedButton *ddBtn, QgsComposerObject::DataDefinedProperty property );

    /** Returns the data defined property corresponding to a data defined button widget*/
    virtual QgsComposerObject::DataDefinedProperty ddPropertyForWidget( QgsDataDefinedButton* widget );

};
