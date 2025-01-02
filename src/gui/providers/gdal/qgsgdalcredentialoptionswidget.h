/***************************************************************************
                          qgsgdalcredentialoptionswidget.h
                             -------------------
    begin                : June 2024
    copyright            : (C) 2024 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGGDALCREDENTIALOPTIONSWIDGET_H
#define QGGDALCREDENTIALOPTIONSWIDGET_H

#include "ui_qgsgdalcredentialoptionswidgetbase.h"
#include "qgis_gui.h"
#include "qgis_sip.h"
#include "qgsgdalutils.h"

#include <QAbstractItemModel>
#include <QStyledItemDelegate>

#ifndef SIP_RUN
///@cond PRIVATE

class QgsGdalCredentialOptionsModel : public QAbstractItemModel
{
    Q_OBJECT

  public:
    enum Column
    {
      Key = 0,
      Value = 1,
      Actions = 2
    };

    QgsGdalCredentialOptionsModel( QObject *parent );
    int columnCount( const QModelIndex &parent = QModelIndex() ) const override;
    int rowCount( const QModelIndex &parent = QModelIndex() ) const override;
    QModelIndex index( int row, int column, const QModelIndex &parent = QModelIndex() ) const override;
    QModelIndex parent( const QModelIndex &child ) const override;
    Qt::ItemFlags flags( const QModelIndex &index ) const override;
    QVariant data( const QModelIndex &index, int role ) const override;
    bool setData( const QModelIndex &index, const QVariant &value, int role ) override;
    bool insertRows( int position, int rows, const QModelIndex &parent = QModelIndex() ) override;
    bool removeRows( int position, int rows, const QModelIndex &parent = QModelIndex() ) override;

    void setOptions( const QList<QPair<QString, QString>> &options );
    void setAvailableOptions( const QList<QgsGdalOption> &options );
    QStringList availableKeys() const { return mAvailableKeys; }
    QgsGdalOption option( const QString &key ) const;
    QList<QPair<QString, QString>> credentialOptions() const { return mCredentialOptions; }
    void setCredentialOptions( const QList<QPair<QString, QString>> &options );

  signals:

    void optionsChanged();

  private:
    QList<QPair<QString, QString>> mCredentialOptions;
    QList<QgsGdalOption> mAvailableOptions;
    QStringList mAvailableKeys;
    QMap<QString, QString> mDescriptions;
};


class QgsGdalCredentialOptionsDelegate : public QStyledItemDelegate
{
    Q_OBJECT

  public:
    QgsGdalCredentialOptionsDelegate( QObject *parent );

  protected:
    QWidget *createEditor( QWidget *parent, const QStyleOptionViewItem & /*option*/, const QModelIndex &index ) const override;
    void setEditorData( QWidget *editor, const QModelIndex &index ) const override;
    void setModelData( QWidget *editor, QAbstractItemModel *model, const QModelIndex &index ) const override;
};

class QgsGdalCredentialOptionsRemoveOptionDelegate : public QStyledItemDelegate
{
    Q_OBJECT

  public:
    QgsGdalCredentialOptionsRemoveOptionDelegate( QObject *parent );
    bool eventFilter( QObject *obj, QEvent *event ) override;

  protected:
    void paint( QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index ) const override;

  private:
    void setHoveredIndex( const QModelIndex &index );

    QModelIndex mHoveredIndex;
};

///@endcond PRIVATE
#endif

/**
 * \class QgsGdalCredentialOptionsWidget
 * \brief A widget for configuring GDAL credential options.
 *
 * \since QGIS 3.40
 */
class GUI_EXPORT QgsGdalCredentialOptionsWidget : public QWidget, private Ui::QgsGdalCredentialOptionsWidgetBase
{
    Q_OBJECT

  public:
    /**
     * Constructor for QgsGdalCredentialOptionsWidget, with the specified \a parent widget.
     */
    QgsGdalCredentialOptionsWidget( QWidget *parent = nullptr );

    /**
     * Sets the corresponding \a handler.
     *
     * This should match a GDAL VSI handler, eg "vsis3".
     */
    void setHandler( const QString &handler );

    /**
     * Returns the credential options configured in the widget.
     *
     * \see setCredentialOptions()
     */
    QVariantMap credentialOptions() const;

    /**
     * Sets the credential \a options to show in the widget.
     *
     * \see credentialOptions()
     */
    void setCredentialOptions( const QVariantMap &options );

  signals:

    /**
     * Emitted when the credential options are changed in the widget.
     */
    void optionsChanged();

  private slots:

    void modelOptionsChanged();

  private:
    QString mHandler;
    QgsGdalCredentialOptionsModel *mModel = nullptr;
    QgsGdalCredentialOptionsDelegate *mDelegate = nullptr;
};

#endif // QGGDALCREDENTIALOPTIONSWIDGET_H
