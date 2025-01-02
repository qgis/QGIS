/***************************************************************************
    qgssensorthingssourcewidget.h
     --------------------------------------
    Date                 : December 2023
    Copyright            : (C) 2023 by Nyall Dawson
    Email                : nyall dot dawson at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSSENSORTHINGSSOURCEWIDGET_H
#define QGSSENSORTHINGSSOURCEWIDGET_H

#include "qgsprovidersourcewidget.h"
#include "qgssensorthingsutils.h"
#include "qgis.h"
#include "ui_qgssensorthingssourcewidgetbase.h"
#include <QDialog>
#include <QVariantMap>
#include <QPointer>
#include <QStyledItemDelegate>

class QgsExtentWidget;
class QgsSensorThingsConnectionPropertiesTask;
class QTableView;

///@cond PRIVATE
#define SIP_NO_FILE


class QgsSensorThingsExpansionsModel : public QAbstractItemModel
{
    Q_OBJECT

  public:
    enum Column
    {
      Entity = 0,
      Limit = 1,
      OrderBy = 2,
      SortOrder = 3,
      Filter = 4,
      Actions = 5,
    };

    QgsSensorThingsExpansionsModel( QObject *parent );
    int columnCount( const QModelIndex &parent = QModelIndex() ) const override;
    int rowCount( const QModelIndex &parent = QModelIndex() ) const override;
    QModelIndex index( int row, int column, const QModelIndex &parent = QModelIndex() ) const override;
    QModelIndex parent( const QModelIndex &child ) const override;
    Qt::ItemFlags flags( const QModelIndex &index ) const override;
    QVariant data( const QModelIndex &index, int role ) const override;
    QVariant headerData( int section, Qt::Orientation orientation, int role = Qt::DisplayRole ) const override;
    bool setData( const QModelIndex &index, const QVariant &value, int role ) override;
    bool insertRows( int position, int rows, const QModelIndex &parent = QModelIndex() ) override;
    bool removeRows( int position, int rows, const QModelIndex &parent = QModelIndex() ) override;

    void setExpansions( const QList<QgsSensorThingsExpansionDefinition> &expansions );
    QList<QgsSensorThingsExpansionDefinition> expansions() const { return mExpansions; }

  private:
    QList<QgsSensorThingsExpansionDefinition> mExpansions;
};

class QgsSensorThingsFilterWidget : public QWidget
{
    Q_OBJECT

  public:
    QgsSensorThingsFilterWidget( QWidget *parent, Qgis::SensorThingsEntity entity );
    void setFilter( const QString &filter );
    QString filter() const;

  signals:
    void filterChanged();

  private slots:

    void setQuery();

  private:
    QString mFilter;
    Qgis::SensorThingsEntity mEntity = Qgis::SensorThingsEntity::Invalid;
};


class QgsSensorThingsExpansionsDelegate : public QStyledItemDelegate
{
    Q_OBJECT

  public:
    QgsSensorThingsExpansionsDelegate( QObject *parent );
    void setBaseEntityType( Qgis::SensorThingsEntity type );

  protected:
    QWidget *createEditor( QWidget *parent, const QStyleOptionViewItem & /*option*/, const QModelIndex &index ) const override;
    void setEditorData( QWidget *editor, const QModelIndex &index ) const override;
    void setModelData( QWidget *editor, QAbstractItemModel *model, const QModelIndex &index ) const override;

  private:
    Qgis::SensorThingsEntity mBaseEntityType = Qgis::SensorThingsEntity::Invalid;
};


class QgsSensorThingsRemoveExpansionDelegate : public QStyledItemDelegate SIP_SKIP
{
    Q_OBJECT

  public:
    QgsSensorThingsRemoveExpansionDelegate( QObject *parent );
    bool eventFilter( QObject *obj, QEvent *event ) override;

  protected:
    void paint( QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index ) const override;

  private:
    void setHoveredIndex( const QModelIndex &index );

    QModelIndex mHoveredIndex;
};


class QgsSensorThingsSourceWidget : public QgsProviderSourceWidget, protected Ui::QgsSensorThingsSourceWidgetBase
{
    Q_OBJECT

  public:
    QgsSensorThingsSourceWidget( QWidget *parent = nullptr );
    ~QgsSensorThingsSourceWidget() override;

    void setSourceUri( const QString &uri ) override;
    QString sourceUri() const override;
    QString groupTitle() const override;
    void setMapCanvas( QgsMapCanvas *mapCanvas ) override;
    Qgis::SensorThingsEntity currentEntityType() const;

    /**
     * Updates a connection uri with the layer specific URI settings defined in the widget.
     */
    QString updateUriFromGui( const QString &connectionUri ) const;

    bool isValid() const { return mIsValid; }

  private slots:

    void entityTypeChanged();
    void validate();
    void retrieveTypes();
    void connectionPropertiesTaskCompleted();

  private:
    void setCurrentEntityType( Qgis::SensorThingsEntity type );
    void setCurrentGeometryTypeFromString( const QString &geometryType );

    QgsExtentWidget *mExtentWidget = nullptr;
    QgsSensorThingsExpansionsModel *mExpansionsModel = nullptr;
    QgsSensorThingsExpansionsDelegate *mExpansionsTableDelegate = nullptr;
    QVariantMap mSourceParts;
    bool mIsValid = false;
    QPointer<QgsSensorThingsConnectionPropertiesTask> mPropertiesTask;
};

///@endcond
#endif // QGSSENSORTHINGSSOURCEWIDGET_H
