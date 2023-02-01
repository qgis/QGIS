/***************************************************************************
                         qgsaddattrdialog.h  -  description
                             -------------------
    begin                : January 2005
    copyright            : (C) 2005 by Marco Hugentobler
    email                : marco.hugentobler@autoform.ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSADDATTRDIALOG_H
#define QGSADDATTRDIALOG_H

// We don't want to expose this in the public API
#define SIP_NO_FILE

#include "ui_qgsaddattrdialogbase.h"
#include "qgsguiutils.h"
#include "qgis_gui.h"
#include <QSet>

class QgsVectorLayer;
class QgsField;

/**
 * \ingroup gui
 * \brief Dialog to add a source field attribute
 *
 * \note This class is not a part of public API
 * \since QGIS 3.14
 */
class GUI_EXPORT QgsAddAttrDialog: public QDialog, private Ui::QgsAddAttrDialogBase
{
    Q_OBJECT
  public:
    //! constructor
    QgsAddAttrDialog( QgsVectorLayer *vlayer,
                      QWidget *parent = nullptr, Qt::WindowFlags fl = QgsGuiUtils::ModalDialogFlags );
    //! constructor
    QgsAddAttrDialog( const std::list<QString> &typelist,
                      QWidget *parent = nullptr, Qt::WindowFlags fl = QgsGuiUtils::ModalDialogFlags );

    /**
     * Sets a list of field \a names which are considered illegal and
     * should not be accepted by the dialog.
     *
     * \since QGIS 3.30
     */
    void setIllegalFieldNames( const QSet< QString> &names );

    //! Returns a field for the configured attribute
    QgsField field() const;

  private slots:
    void mTypeBox_currentIndexChanged( int idx );
    void mLength_editingFinished();
    void accept() override;

  private:
    bool mIsShapeFile = false;
    QSet< QString > mIllegalFieldNames;

    void setPrecisionMinMax();
};

#endif
