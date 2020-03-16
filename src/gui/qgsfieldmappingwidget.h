/***************************************************************************
  qgsfieldmappingwidget.h - QgsFieldMappingWidget

 ---------------------
 begin                : 16.3.2020
 copyright            : (C) 2020 by Alessandro Pasotti
 email                : elpaso at itopen dot it
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSFIELDMAPPINGWIDGET_H
#define QGSFIELDMAPPINGWIDGET_H

#include <QWidget>
#include <QAbstractTableModel>
#include <QStyledItemDelegate>

#include "qgis_gui.h"
#include "qgsfields.h"
#include "qgsexpression.h"
#include "ui_qgsfieldmappingwidget.h"
#include "qgsexpressioncontextgenerator.h"

/**
 * \ingroup gui
 * The QgsFieldMappingWidget class allows to define a map from one set of QgsFields to another,
 * for each set of "destination" fields an expression defines how to obtain the values of the
 * "destination" fields.
 * \since QGIS 3.14
 */
class GUI_EXPORT QgsFieldMappingWidget : public QWidget, private Ui::QgsFieldMappingWidget
{
    Q_OBJECT

  public:

    explicit QgsFieldMappingWidget(const QgsFields &sourceFields,
                                   const QgsFields &destinationFields,
                                   const QMap<QString, QgsExpression> &expressions = QMap<QString, QgsExpression>(),
                                   QWidget *parent = nullptr);

    QMap<QString, QgsExpression> expressions() const;

  signals:

  private:

    QgsFields mSourceFields;
    QgsFields mDestinationFields;
    QMap<QString, QgsExpression> mExpressions;
    QAbstractTableModel* mModel;

    class ExpressionDelegate: public QStyledItemDelegate
    {

     public:

        ExpressionDelegate(QObject *parent = nullptr);

        // QAbstractItemDelegate interface
        QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const override;
        void setEditorData(QWidget* editor, const QModelIndex& index) const override;
        void setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const override;
    };


};


/**
 * \ingroup gui
 * The QgsFieldMappingModel holds mapping information for mapping from one set of QgsFields to another,
 * for each set of "destination" fields an expression defines how to obtain the values of the
 * "destination" fields.
 * \since QGIS 3.14
 */
class GUI_EXPORT QgsFieldMappingModel: public QAbstractTableModel
{

    Q_OBJECT

  public:

    QgsFieldMappingModel(const QgsFields &sourceFields,
                         const QgsFields &destinationFields,
                         const QMap<QString, QgsExpression> &expressions = QMap<QString, QgsExpression>(),
                         QObject* parent = nullptr);

    QgsExpressionContextGenerator* contextGenerator() const;

    // QAbstractItemModel interface
    int rowCount(const QModelIndex& parent) const override;
    int columnCount(const QModelIndex& parent) const override;
    QVariant data(const QModelIndex& index, int role) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
    Qt::ItemFlags flags(const QModelIndex& index) const override;
    QgsFields sourceFields() const;
    bool setData(const QModelIndex& index, const QVariant& value, int role) override;



  private:

    struct Field
    {
      QString name;
      QVariant type;
      int length;
      int precision;
      QgsExpression expression;
    };

    QList<Field> mMapping;

    class ExpressionContextGenerator: public QgsExpressionContextGenerator
    {

      public:

       ExpressionContextGenerator( const QgsFields* sourceFields );

        // QgsExpressionContextGenerator interface
        QgsExpressionContext createExpressionContext() const override;

      private:

        const QgsFields* mSourceFields;

    };

    QgsFields mSourceFields;
    QgsFields mDestinationFields;
    std::unique_ptr<ExpressionContextGenerator> mExpressionContextGenerator;

};














#endif // QGSFIELDMAPPINGWIDGET_H
