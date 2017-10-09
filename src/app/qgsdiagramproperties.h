/***************************************************************************
  qgsdiagramproperties.h
  Properties for diagram layers
  -------------------
         begin                : August 2012
         copyright            : (C) Matthias Kuhn
         email                : matthias at opengis dot ch

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
#include "qgsdiagramrenderer.h"
#include "ui_qgsdiagrampropertiesbase.h"
#include <QStyledItemDelegate>
#include "qgis_app.h"

class QgsVectorLayer;
class QgsMapCanvas;

class APP_EXPORT QgsDiagramProperties : public QWidget, private Ui::QgsDiagramPropertiesBase, private QgsExpressionContextGenerator
{
    Q_OBJECT

  public:
    QgsDiagramProperties( QgsVectorLayer *layer, QWidget *parent, QgsMapCanvas *canvas );

    ~QgsDiagramProperties();

    //! Adds an attribute from the list of available attributes to the assigned attributes with a random color.
    void addAttribute( QTreeWidgetItem *item );

  signals:

    void auxiliaryFieldCreated();

  public slots:
    void apply();
    void mDiagramTypeComboBox_currentIndexChanged( int index );
    void mAddCategoryPushButton_clicked();
    void mAttributesTreeWidget_itemDoubleClicked( QTreeWidgetItem *item, int column );
    void mFindMaximumValueButton_clicked();
    void mRemoveCategoryPushButton_clicked();
    void mDiagramAttributesTreeWidget_itemDoubleClicked( QTreeWidgetItem *item, int column );
    void mEngineSettingsButton_clicked();
    void showAddAttributeExpressionDialog();
    void mDiagramStackedWidget_currentChanged( int index );
    void updatePlacementWidgets();
    void scalingTypeChanged();
    void showSizeLegendDialog();

  private:

    QgsVectorLayer *mLayer = nullptr;
    //! Point placement button group
    QButtonGroup *mPlacePointBtnGrp = nullptr;
    //! Line placement button group
    QButtonGroup *mPlaceLineBtnGrp = nullptr;
    //! Polygon placement button group
    QButtonGroup *mPlacePolygonBtnGrp = nullptr;

    enum Columns
    {
      ColumnAttributeExpression = 0,
      ColumnColor,
      ColumnLegendText,
    };

    enum Roles
    {
      RoleAttributeExpression = Qt::UserRole,
    };

    QString showExpressionBuilder( const QString &initialExpression );

    QgsPropertyCollection mDataDefinedProperties;

    // Keeps track of the diagram type to properly save / restore settings when the diagram type combo box is set to no diagram.
    QString mDiagramType;
    std::unique_ptr< QgsDataDefinedSizeLegend > mSizeLegend;

    QString guessLegendText( const QString &expression );
    QgsMapCanvas *mMapCanvas = nullptr;

    QgsExpressionContext createExpressionContext() const override;

    void registerDataDefinedButton( QgsPropertyOverrideButton *button, QgsDiagramLayerSettings::Property key );

  private slots:

    void updateProperty();
    void showHelp();

    void createAuxiliaryField();
};

class EditBlockerDelegate: public QStyledItemDelegate
{
    Q_OBJECT

  public:
    EditBlockerDelegate( QObject *parent = nullptr )
      : QStyledItemDelegate( parent )
    {}

    virtual QWidget *createEditor( QWidget *, const QStyleOptionViewItem &, const QModelIndex & ) const override
    {
      return nullptr;
    }
};


#endif // QGSDIAGRAMPROPERTIES_H
