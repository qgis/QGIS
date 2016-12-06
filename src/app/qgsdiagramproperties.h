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
#include <ui_qgsdiagrampropertiesbase.h>
#include <QItemDelegate>
#include <QStyledItemDelegate>
#include "qgis_app.h"

class QgsVectorLayer;
class QgsMapCanvas;


/**
 * The QgsDiagramProperties class
 * \ingroup app
 * \note added in QGIS 3.0
 */
class APP_EXPORT QgsDiagramProperties : public QWidget, private Ui::QgsDiagramPropertiesBase, private QgsExpressionContextGenerator
{
    Q_OBJECT

  public:
    QgsDiagramProperties( QgsVectorLayer *layer, QWidget *parent, QgsMapCanvas *canvas );

    ~QgsDiagramProperties();

    //! Adds an attribute from the list of available attributes to the assigned attributes with a random color.
    void addAttribute( QTreeWidgetItem *item );

  public slots:
    void apply();
    void on_mDiagramTypeComboBox_currentIndexChanged( int index );
    void on_mAddCategoryPushButton_clicked();
    void on_mAttributesTreeWidget_itemDoubleClicked( QTreeWidgetItem *item, int column );
    void on_mFindMaximumValueButton_clicked();
    void on_mRemoveCategoryPushButton_clicked();
    void on_mDiagramFontButton_clicked();
    void on_mDiagramAttributesTreeWidget_itemDoubleClicked( QTreeWidgetItem *item, int column );
    void on_mEngineSettingsButton_clicked();
    void showAddAttributeExpressionDialog();
    void on_mDiagramStackedWidget_currentChanged( int index );
    void on_mPlacementComboBox_currentIndexChanged( int index );
    void on_mButtonSizeLegendSymbol_clicked();
    void scalingTypeChanged();

  protected:
    QFont mDiagramFont;

    QgsVectorLayer *mLayer = nullptr;

  private:

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
    std::unique_ptr< QgsMarkerSymbol > mSizeLegendSymbol;

    QString guessLegendText( const QString &expression );
    QgsMapCanvas *mMapCanvas = nullptr;

    QgsExpressionContext createExpressionContext() const override;

    void registerDataDefinedButton( QgsPropertyOverrideButton *button, QgsDiagramLayerSettings::Property key );

  private slots:

    void updateProperty();
    void on_mAddButton_clicked();
    void on_mRemoveButton_clicked();
};

class EditBlockerDelegate: public QStyledItemDelegate
{
  public:
    EditBlockerDelegate( QObject *parent = nullptr )
      : QStyledItemDelegate( parent )
    {}

    virtual QWidget *createEditor( QWidget *, const QStyleOptionViewItem &, const QModelIndex & ) const override
    {
      return nullptr;
    }
};

class SizeRuleItemDelegate: public QItemDelegate
{
  public:
    SizeRuleItemDelegate( QObject *parent = nullptr )
      : QItemDelegate( parent )
    {
      mValidator = new QDoubleValidator( this );
    }

    virtual QWidget *createEditor( QWidget *parent, const QStyleOptionViewItem &, const QModelIndex & ) const override
    {
      QLineEdit *edit = new QLineEdit( parent );
      edit->setValidator( mValidator );
      return edit;
    }
  private:
    const QValidator *mValidator;
};

#endif // QGSDIAGRAMPROPERTIES_H
