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
class QComboBox;
class QPushButton;

/** \ingroup gui
 * A file dialog which lets the user select the preferred encoding type for a data provider.
 **/
class GUI_EXPORT QgsEncodingFileDialog: public QFileDialog
{
    Q_OBJECT
  public:
    QgsEncodingFileDialog( QWidget * parent = nullptr,
                           const QString & caption = QString(), const QString & directory = QString(),
                           const QString & filter = QString(), const QString & encoding = QString() );
    ~QgsEncodingFileDialog();
    /** Returns a string describing the chosen encoding*/
    QString encoding() const;
    /** Adds a 'Cancel All' button for the user to click */
    void addCancelAll();
    /** Returns true if the user clicked 'Cancel All' */
    bool cancelAll();

  public slots:
    void saveUsedEncoding();

    void pbnCancelAll_clicked();

  private:
    /** Box to choose the encoding type*/
    QComboBox* mEncodingComboBox;

    /* The button to click */
    QPushButton *mCancelAllButton;

    /* Set if user clicked 'Cancel All' */
    bool mCancelAll;
};

#endif
