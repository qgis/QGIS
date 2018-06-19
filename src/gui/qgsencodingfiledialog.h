/***************************************************************************
    qgsencodingfiledialog.h - File dialog which queries the encoding type
     --------------------------------------
    Date                 : 16-Feb-2005
    Copyright            : (C) 2005 by Marco Hugentobler
    email                : marco.hugentobler@autoform.ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSENCODINGFILEDIALOG_H
#define QGSENCODINGFILEDIALOG_H

#include <QFileDialog>
#include "qgis_gui.h"
#include "qgis.h"

class QComboBox;
class QPushButton;

/**
 * \ingroup gui
 * A file dialog which lets the user select the preferred encoding type for a data provider.
 **/
class GUI_EXPORT QgsEncodingFileDialog: public QFileDialog
{
    Q_OBJECT
  public:

    /**
     * Constructor for QgsEncodingFileDialog
     */
    QgsEncodingFileDialog( QWidget *parent SIP_TRANSFERTHIS = nullptr,
                           const QString &caption = QString(), const QString &directory = QString(),
                           const QString &filter = QString(), const QString &encoding = QString() );
    //! Returns a string describing the chosen encoding
    QString encoding() const;
    //! Adds a 'Cancel All' button for the user to click
    void addCancelAll();
    //! Returns true if the user clicked 'Cancel All'
    bool cancelAll();

  public slots:
    void saveUsedEncoding();

    void pbnCancelAll_clicked();

  private:
    //! Box to choose the encoding type
    QComboBox *mEncodingComboBox = nullptr;

    /* The button to click */
    QPushButton *mCancelAllButton = nullptr;

    /* Set if user clicked 'Cancel All' */
    bool mCancelAll;
};

/**
 * \ingroup gui
 * A dialog which presents the user with a choice of file encodings.
 * \since QGIS 3.0
 */
class GUI_EXPORT QgsEncodingSelectionDialog: public QDialog
{
    Q_OBJECT

  public:

    /**
     * Constructor for QgsEncodingSelectionDialog.
     *
     * If \a caption is set, it will be used as the caption within the dialog.
     *
     * The \a encoding argument can be used to specify the encoding initially selected in the dialog.
     */
    QgsEncodingSelectionDialog( QWidget *parent SIP_TRANSFERTHIS = nullptr,
                                const QString &caption = QString(), const QString &encoding = QString(),
                                Qt::WindowFlags flags = Qt::WindowFlags() );

    /**
     * Returns the encoding selected within the dialog.
     * \see setEncoding()
     */
    QString encoding() const;

    /**
     * Sets the \a encoding selected within the dialog.
     * see encoding()
     */
    void setEncoding( const QString &encoding );

  private:

    QComboBox *mEncodingComboBox = nullptr;

};


#endif
