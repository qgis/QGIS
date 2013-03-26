/***************************************************************************
                              qgsatlascompositionwidget.h
                              ---------------------------
    begin                : October 2012
    copyright            : (C) 2012 Hugo Mercier
    email                : hugo dot mercier at oslandia dot com
 ***************************************************************************/
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "ui_qgsatlascompositionwidgetbase.h"

class QgsComposition;
class QgsMapLayer;
class QgsComposerMap;
class QgsComposerItem;

/** \ingroup MapComposer
  * Input widget for QgsAtlasComposition
  */
class QgsAtlasCompositionWidget:
      public QWidget,
      private Ui::QgsAtlasCompositionWidgetBase
{
    Q_OBJECT
  public:
    QgsAtlasCompositionWidget( QWidget* parent, QgsComposition* c );
    ~QgsAtlasCompositionWidget();

  public slots:
    void on_mUseAtlasCheckBox_stateChanged( int state );
    void on_mComposerMapComboBox_currentIndexChanged( int index );
    void on_mAtlasCoverageLayerComboBox_currentIndexChanged( int index );
    void on_mAtlasFilenamePatternEdit_textChanged( const QString& text );
    void on_mAtlasFilenameExpressionButton_clicked();
    void on_mAtlasHideCoverageCheckBox_stateChanged( int state );
    void on_mAtlasFixedScaleCheckBox_stateChanged( int state );
    void on_mAtlasSingleFileCheckBox_stateChanged( int state );

    void on_mAtlasSortFeatureCheckBox_stateChanged( int state );
    void on_mAtlasSortFeatureKeyComboBox_currentIndexChanged( int index );
    void on_mAtlasSortFeatureDirectionButton_clicked();
    void on_mAtlasFeatureFilterEdit_textChanged( const QString& text );
    void on_mAtlasFeatureFilterButton_clicked();
    void on_mAtlasFeatureFilterCheckBox_stateChanged( int state );

    // extract fields from the current coverage layer and populate the corresponding combo box
    void fillSortColumns();
  private slots:
    void onLayerRemoved( QString );
    void onLayerAdded( QgsMapLayer* );
    void onComposerMapAdded( QgsComposerMap* );
    void onItemRemoved( QgsComposerItem* );

    void updateGuiElements();

  private:
    QgsComposition* mComposition;

    void blockAllSignals( bool b );
};
