/***************************************************************************
    qgslistwidget.h
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

#ifndef QGSLISTWIDGET_H
#define QGSLISTWIDGET_H

#include "qgstablewidgetbase.h"
#include <QAbstractTableModel>
#include <QVariant>
#include "qgis_gui.h"


#ifndef SIP_RUN
///@cond PRIVATE

/**
 * \ingroup gui
 * \brief Table model to edit a QVariantList.
 * \note not available in Python bindings
 */
class GUI_EXPORT QgsListModel : public QAbstractTableModel
{
    Q_OBJECT
  public:

    explicit QgsListModel( QVariant::Type subType, QObject *parent = nullptr );
    void setList( const QVariantList &list );
    QVariantList list() const;
    bool valid() const;

    int rowCount( const QModelIndex &parent = QModelIndex() ) const override;
    int columnCount( const QModelIndex &parent = QModelIndex() ) const override;
    QVariant headerData( int section, Qt::Orientation orientation, int role ) const override;
    QVariant data( const QModelIndex &index, int role = Qt::DisplayRole ) const override;
    bool setData( const QModelIndex &index, const QVariant &value, int role = Qt::EditRole ) override;
    Qt::ItemFlags flags( const QModelIndex &index ) const override;
    bool insertRows( int position, int rows, const QModelIndex &parent = QModelIndex() ) override;
    bool removeRows( int position, int rows, const QModelIndex &parent = QModelIndex() ) override;
    void setReadOnly( bool readOnly );
  private:
    bool mReadOnly = false;
    QVariantList mLines;
    QVariant::Type mSubType;
};
///@endcond
#endif


/**
 * \ingroup gui
 * \brief Widget allowing to edit a QVariantList, using a table.
 */
class GUI_EXPORT QgsListWidget: public QgsTableWidgetBase
{
    Q_OBJECT
    Q_PROPERTY( QVariantList list READ list WRITE setList )
  public:

    /**
     * Constructor.
     */
    explicit QgsListWidget( QVariant::Type subType, QWidget *parent = nullptr );

    /**
     * Set the initial value of the widget.
     */
    void setList( const QVariantList &list );

    /**
     * Gets the edit value.
     * \returns the QVariantList
     */
    QVariantList list() const { return mModel.list(); }

    /**
     * Check the content is valid
     * \returns TRUE if valid
     */
    bool valid() const { return mModel.valid(); }

  public slots:

    void setReadOnly( bool readOnly ) override;

  private:
    QgsListModel mModel;
    QVariant::Type mSubType;
};


#endif // QGSKEYVALUEWIDGET_H
