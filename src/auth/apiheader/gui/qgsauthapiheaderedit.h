/***************************************************************************
    qgsauthapiheaderedit.h
    ----------------------
    begin                : October 2021
    copyright            : (C) 2021 by Tom Cummins
    author               : Tom Cummins
    email                : tom cumminsc9 at googlemail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSAUTHAPIHEADEREDIT_H
#define QGSAUTHAPIHEADEREDIT_H

#include <QWidget>
#include "qgsauthmethodedit.h"
#include "ui_qgsauthapiheaderedit.h"

#include "qgsauthconfig.h"


class QgsAuthApiHeaderEdit : public QgsAuthMethodEdit, private Ui::QgsAuthApiHeaderEdit
{
    Q_OBJECT

  public:
    explicit QgsAuthApiHeaderEdit( QWidget *parent = nullptr );

    bool validateConfig() override;

    QgsStringMap configMap() const override;

  public slots:
    void loadConfig( const QgsStringMap &configmap ) override;

    void resetConfig() override;

    void clearConfig() override;

  private slots:
    void addHeaderPair();

    void removeHeaderPair();

    void clearHeaderPairs();

    void populateHeaderPairs( const QgsStringMap &headerpairs, bool append = false );

    void headerTableSelectionChanged();

    void headerTableCellChanged( const int row, const int column );

  private:
    QgsStringMap mConfigMap;
    bool mValid = false;

    bool emptyHeadersKeysPresent();

    void addHeaderPairRow( const QString &key, const QString &val );

    QgsStringMap headerPairs() const;
};

#endif // QGSAUTHAPIHEADEREDIT_H
