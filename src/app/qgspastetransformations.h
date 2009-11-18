/***************************************************************************
    qgspastetransformations.h - set up how source fields are transformed to
                                destination fields in copy/paste operations
                             -------------------
    begin                : 8 July 2005
    copyright            : (C) 2005 by Brendan Morley
    email                : morb at ozemail dot com dot au
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
/* $Id$ */
#ifndef QGSPASTETRANSFORMATIONS_H
#define QGSPASTETRANSFORMATIONS_H
#include "ui_qgspastetransformationsbase.h"

#include "qgsmaplayer.h"
#include "qgscontexthelp.h"

class QPushButton;

/*!
 * \brief Dialog to allow the user to set up how source fields are transformed to destination fields in copy/paste operations
 */
class QgsPasteTransformations : public QDialog, private Ui::QgsPasteTransformationsBase
{
    Q_OBJECT

  public:
    //! Constructor
    QgsPasteTransformations();

    //! Destructor
    ~QgsPasteTransformations();

    /**
       Returns the destination field in destinationLayerName that
       should be chosen for pastes from sourceLayerName & sourceFieldName.

       Returns the sourceFieldName if there is no saved preference.

       @note  This non-GUI function is a bonus for this class.  OO purists may insist that this function should be in its own class.  If so, let them separate it.
     */
    QString pasteTo( const QString& sourceLayerName,
                     const QString& destinationLayerName,
                     const QString& sourceFieldName );

  public slots:
    virtual void accept();
    virtual void addNewTransfer();
    virtual void sourceChanged( const QString& layerName );
    virtual void destinationChanged( const QString& layerName );

    void on_buttonBox_helpRequested() { QgsContextHelp::run( metaObject()->className() ); }

  private:

    void addTransfer( const QString& sourceLayerName      = QString::null,
                      const QString& destinationLayerName = QString::null );

    //! Common subfunction to sourceChanged() and destinationChanged()
    void layerChanged( const QString& layerName, std::vector<QString>* fields );

    void restoreTransfers( const QString& sourceSelectedFieldName,
                           const QString& destinationSelectedFieldName );

    std::map<QString, QgsMapLayer*> mMapNameLookup;
    std::vector<QString> mSourceFields;
    std::vector<QString> mDestinationFields;
    std::vector<QComboBox*> mSourceTransfers;
    std::vector<QComboBox*> mDestinationTransfers;
    QPushButton * mAddTransferButton;
};

#endif //  QGSPASTETRANSFORMATIONS_H
