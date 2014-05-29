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
#include "qgsvectorlayer.h"

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
    void changeCoverageLayer( QgsMapLayer* layer );
    void on_mAtlasFilenamePatternEdit_editingFinished();
    void on_mAtlasFilenameExpressionButton_clicked();
    void on_mAtlasHideCoverageCheckBox_stateChanged( int state );
    void on_mAtlasSingleFileCheckBox_stateChanged( int state );

    void on_mAtlasSortFeatureCheckBox_stateChanged( int state );
    void changesSortFeatureField( QString fieldName );
    void on_mAtlasSortFeatureDirectionButton_clicked();
    void on_mAtlasFeatureFilterEdit_editingFinished();
    void on_mAtlasFeatureFilterButton_clicked();
    void on_mAtlasFeatureFilterCheckBox_stateChanged( int state );

  private slots:
    void updateGuiElements();

    void updateAtlasFeatures();

  private:
    QgsComposition* mComposition;

    void blockAllSignals( bool b );
    void checkLayerType( QgsVectorLayer *layer );
};
