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

#include "qgsvectordataprovider.h"
#include <qcombobox.h>
#include <qfiledialog.h>

/**A file dialog which lets the user select the prefered encoding type for a data provider*/
class QgsEncodingFileDialog: public QFileDialog
{
    Q_OBJECT
 public:
    QgsEncodingFileDialog(const QString & dirName, const QString& filter, QWidget * parent, const char * name);
    ~QgsEncodingFileDialog();
    QgsVectorDataProvider::Encoding encoding() const;
 private:
    /**Box to choose the encoding type*/
    QComboBox* mEncodingComboBox;
};

#endif
