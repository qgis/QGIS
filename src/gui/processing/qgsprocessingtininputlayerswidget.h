/***************************************************************************
  qgsprocessingtininputlayerswidget.h
  ---------------------
  Date                 : August 2020
  Copyright            : (C) 2020 by Vincent Cloarec
  Email                : vcloarec at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSPROCESSINGTININPUTLAYERSWIDGET_H
#define QGSPROCESSINGTININPUTLAYERSWIDGET_H

#define SIP_NO_FILE

#include <QAbstractTableModel>
#include <QStyledItemDelegate>

#include "qgsprocessingwidgetwrapper.h"
#include "qgsprocessingparametertininputlayers.h"
#include "ui_qgsprocessingtinmeshdatawidgetbase.h"

/// @cond PRIVATE

class QgsProcessingTinInputLayersModel: public QAbstractTableModel
{
    Q_OBJECT
  public:
    enum Roles
    {
      Type = Qt::UserRole
    };

    QgsProcessingTinInputLayersModel( QgsProject *project );

    int rowCount( const QModelIndex &parent ) const override;
    int columnCount( const QModelIndex &parent ) const override;
    QVariant data( const QModelIndex &index, int role ) const override;
    bool setData( const QModelIndex &index, const QVariant &value, int role ) override;
    Qt::ItemFlags flags( const QModelIndex &index ) const override;
    QVariant headerData( int section, Qt::Orientation orientation, int role ) const override;

    void addLayer( QgsProcessingParameterTinInputLayers::InputLayer &layer );
    void removeLayer( int index );
    void clear();

    QList<QgsProcessingParameterTinInputLayers::InputLayer> layers() const;

    void setProject( QgsProject *project );

  private:
    QList<QgsProcessingParameterTinInputLayers::InputLayer> mInputLayers;
    QgsProject *mProject = nullptr;
};

class QgsProcessingTinInputLayersDelegate: public QStyledItemDelegate
{
    Q_OBJECT
  public:
    QgsProcessingTinInputLayersDelegate( QObject *parent ): QStyledItemDelegate( parent ) {}

    QWidget *createEditor( QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index ) const override;
    void setEditorData( QWidget *editor, const QModelIndex &index ) const override;
    void setModelData( QWidget *editor, QAbstractItemModel *model, const QModelIndex &index ) const override;
};


class GUI_EXPORT QgsProcessingTinInputLayersWidget: public QWidget, private Ui::QgsProcessingTinInputLayersWidgetBase
{
    Q_OBJECT
  public:
    QgsProcessingTinInputLayersWidget( QgsProject *project );

    QVariant value() const;
    void setValue( const QVariant &value );
    void setProject( QgsProject *project );

  signals:
    void changed();

  private slots:
    void onLayerChanged( QgsMapLayer *layer );
    void onCurrentLayerAdded();
    void onLayersRemove();

  private:

    QgsProcessingTinInputLayersModel mInputLayersModel;
};


class GUI_EXPORT QgsProcessingTinInputLayersWidgetWrapper  : public QgsAbstractProcessingParameterWidgetWrapper, public QgsProcessingParameterWidgetFactoryInterface
{
    Q_OBJECT

  public:
    QgsProcessingTinInputLayersWidgetWrapper( const QgsProcessingParameterDefinition *parameter = nullptr,
        QgsProcessingGui::WidgetType type = QgsProcessingGui::Standard, QWidget *parent = nullptr );

    QString parameterType() const override;
    QgsAbstractProcessingParameterWidgetWrapper *createWidgetWrapper( const QgsProcessingParameterDefinition *parameter, QgsProcessingGui::WidgetType type ) override;

  protected:
    QStringList compatibleParameterTypes() const override;
    QStringList compatibleOutputTypes() const override;
    QWidget *createWidget() override SIP_FACTORY;
    void setWidgetValue( const QVariant &value, QgsProcessingContext &context ) override;
    QVariant widgetValue() const override;

  private:
    QgsProcessingTinInputLayersWidget *mWidget = nullptr;

    friend class TestProcessingGui;
};

///@endcond

#endif // QGSPROCESSINGTININPUTLAYERSWIDGET_H
