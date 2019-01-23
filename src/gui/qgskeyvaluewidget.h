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

#include "qgstablewidgetbase.h"
#include "qgis_sip.h"
#include <QAbstractTableModel>
#include <QMap>
#include "qgis_gui.h"


#ifndef SIP_RUN
///@cond PRIVATE

/**
 * \ingroup gui
 * Table model to edit a QVariantMap.
 * \note not available in Python bindings
 * \since QGIS 3.0
 */
class GUI_EXPORT QgsKeyValueModel : public QAbstractTableModel
{
    Q_OBJECT
  public:

    explicit QgsKeyValueModel( QObject *parent = nullptr );
    void setMap( const QVariantMap &map );
    QVariantMap map() const;

    int rowCount( const QModelIndex &parent = QModelIndex() ) const override;
    int columnCount( const QModelIndex &parent = QModelIndex() ) const override;
    QVariant headerData( int section, Qt::Orientation orientation, int role ) const override;
    QVariant data( const QModelIndex &index, int role = Qt::DisplayRole ) const override;
    bool setData( const QModelIndex &index, const QVariant &value, int role = Qt::EditRole ) override;
    Qt::ItemFlags flags( const QModelIndex &index ) const override;
    bool insertRows( int position, int rows, const QModelIndex &parent = QModelIndex() ) override;
    bool removeRows( int position, int rows, const QModelIndex &parent = QModelIndex() ) override;

    typedef QPair<QString, QVariant> Line;

  private:
    QVector<Line> mLines;
};
///@endcond
#endif

/**
 * \ingroup gui
 * Widget allowing to edit a QVariantMap, using a table.
 * \since QGIS 3.0
 */
class GUI_EXPORT QgsKeyValueWidget: public QgsTableWidgetBase
{
    Q_OBJECT
    Q_PROPERTY( QVariantMap map READ map WRITE setMap )
  public:

    /**
     * Constructor.
     */
    explicit QgsKeyValueWidget( QWidget *parent SIP_TRANSFERTHIS = nullptr );

    /**
     * Set the initial value of the widget.
     */
    void setMap( const QVariantMap &map );

    /**
     * Gets the edit value.
     * \returns the QVariantMap
     */
    QVariantMap map() const { return mModel.map(); }

  private:
    QgsKeyValueModel mModel;
};


#endif // QGSKEYVALUEWIDGET_H
