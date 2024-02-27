/***************************************************************************
    qgssensorthingssubseteditor.h
     --------------------------------------
    Date                 : February 2024
    Copyright            : (C) 2024 by Nyall Dawson
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
#ifndef QGSSENSORTHINGSSUBSETEDITOR_H
#define QGSSENSORTHINGSSUBSETEDITOR_H

#include "qgis.h"
#include "ui_qgssensorthingssubseteditorbase.h"
#include "qgssubsetstringeditorinterface.h"
#include "qgsfields.h"
#include <QVariantMap>
#include <QPointer>

class QgsVectorLayer;
class QgsCodeEditor;
class QgsFieldProxyModel;

///@cond PRIVATE
#define SIP_NO_FILE

class QgsSensorThingsSubsetEditor : public QgsSubsetStringEditorInterface, protected Ui::QgsSensorThingsSubsetEditorBase
{
    Q_OBJECT

  public:
    QgsSensorThingsSubsetEditor( QgsVectorLayer *layer = nullptr,
                                 const QgsFields &fields = QgsFields(),
                                 QWidget *parent SIP_TRANSFERTHIS = nullptr,
                                 Qt::WindowFlags fl = QgsGuiUtils::ModalDialogFlags );
    QString subsetString() const override;
    void setSubsetString( const QString &subsetString ) override;

  private slots:
    void accept() override;
    void reset();
    void lstFieldsDoubleClicked( const QModelIndex &index );
  private:

    QgsCodeEditor *mSubsetEditor = nullptr;

    QPointer< QgsVectorLayer > mLayer;
    QgsFields mFields;

    QgsFieldProxyModel *mModelFields = nullptr;
};

///@endcond
#endif // QGSSENSORTHINGSSUBSETEDITOR_H
