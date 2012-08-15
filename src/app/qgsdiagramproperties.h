/***************************************************************************
  qgsdiagramproperties.h
  Properties for diagram layers
  -------------------
         begin                : August 2012
         copyright            : (C) Matthias Kuhn
         email                : matthias dot kuhn at gmx dot ch

 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSDIAGRAMPROPERTIES_H
#define QGSDIAGRAMPROPERTIES_H

#include <QDialog>
#include <ui_qgsdiagrampropertiesbase.h>

class QgsVectorLayer;

class QgsDiagramProperties : public QWidget, private Ui::QgsDiagramPropertiesBase
{
    Q_OBJECT

  public:
    QgsDiagramProperties( QgsVectorLayer* layer, QWidget* parent );

     //void handleAttributeDoubleClicked( QTreeWidgetItem * item, int column );

  public slots:
    void apply();
    void on_mDiagramTypeComboBox_currentIndexChanged( const QString& itemtext );
    void on_mTransparencySlider_valueChanged( int value );
    void on_mAddCategoryPushButton_clicked();
    void on_mBackgroundColorButton_clicked();
    void on_mFindMaximumValueButton_clicked();
    void on_mDiagramPenColorButton_clicked();
    void on_mDisplayDiagramsGroupBox_toggled( bool checked );
    void on_mRemoveCategoryPushButton_clicked();
    void on_mDiagramFontButton_clicked();
    void on_mDiagramAttributesTreeWidget_itemDoubleClicked( QTreeWidgetItem * item, int column );

  protected:
    QFont mDiagramFont;

    QgsVectorLayer* mLayer;

  private:
};

#endif // QGSDIAGRAMPROPERTIES_H
