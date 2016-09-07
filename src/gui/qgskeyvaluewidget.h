/***************************************************************************
    qgskeyvaluewidget.h
     --------------------------------------
    Date                 : 08.2016
    Copyright            : (C) 2016 Patrick Valsecchi
    Email                : patrick.valsecchi@camptocamp.com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSKEYVALUEWIDGET_H
#define QGSKEYVALUEWIDGET_H

#include "ui_qgskeyvaluewidgetbase.h"
#include <QAbstractTableModel>
#include <QMap>

///@cond PRIVATE
/** @ingroup gui
 * Table model to edit a QVariantMap.
 * @note added in QGIS 3.0
 * @note not available in Python bindings
 */
class GUI_EXPORT QgsKeyValueModel : public QAbstractTableModel
{
    Q_OBJECT
  public:

    explicit QgsKeyValueModel( QObject *parent = 0 );
    void setMap( const QVariantMap& map );
    QVariantMap map() const;

    int rowCount( const QModelIndex& parent = QModelIndex() ) const override;
    int columnCount( const QModelIndex& parent = QModelIndex() ) const override;
    QVariant headerData( int section, Qt::Orientation orientation, int role ) const override;
    QVariant data( const QModelIndex& index, int role = Qt::DisplayRole ) const override;
    bool setData( const QModelIndex & index, const QVariant & value, int role = Qt::EditRole ) override;
    Qt::ItemFlags flags( const QModelIndex &index ) const override;
    bool insertRows( int position, int rows, const QModelIndex & parent =  QModelIndex() ) override;
    bool removeRows( int position, int rows, const QModelIndex &parent =  QModelIndex() ) override;

    typedef QPair<QString, QVariant> Line;

  private:
    QVector<Line> mLines;
};
///@endcond


/** \ingroup gui
 * Widget allowing to edit a QVariantMap, using a table.
 * @note added in QGIS 3.0
 */
class GUI_EXPORT QgsKeyValueWidget: public QWidget, public Ui::QgsKeyValueWidgetBase
{
    Q_OBJECT
    Q_PROPERTY( QVariantMap map READ map WRITE setMap )
  public:
    /**
     * Constructor.
     */
    explicit QgsKeyValueWidget( QWidget* parent = nullptr );

    /**
     * Set the initial value of the widget.
     */
    void setMap( const QVariantMap& map );

    /**
     * Get the edit value.
     * @return the QVariantMap
     */
    QVariantMap map() const { return mModel.map(); }

  signals:
    /**
     * Emitted each time a key or a value is changed.
     * @param value the new value
     */
    void valueChanged( const QVariant& value );

  private slots:
    void on_addButton_clicked();
    void on_removeButton_clicked();

    /**
     * Called when the selection is changed to enable/disable the delete button.
     */
    void onSelectionChanged();

    void onValueChanged();

  private:
    QgsKeyValueModel mModel;
};


#endif // QGSKEYVALUEWIDGET_H