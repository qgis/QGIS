/***************************************************************************
    qgsattributesforminitcode.h
    ---------------------
    begin                : October 2017
    copyright            : (C) 2017 by David Signer
    email                : david at opengis dot ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSATTRIBUTESFORMINITCODE_H
#define QGSATTRIBUTESFORMINITCODE_H

// We don't want to expose this in the public API
#define SIP_NO_FILE

#include "ui_qgsattributesforminitcode.h"

#include "qgseditorconfigwidget.h"
#include "qgsvectordataprovider.h"
#include "qgshelp.h"
#include "qgis_gui.h"
#include <QWidget>

class QDialog;

/**
 * \ingroup gui
 * \class QgsAttributesFormInitCode
 */
class GUI_EXPORT QgsAttributesFormInitCode: public QDialog, private Ui::QgsAttributesFormInitCode
{
    Q_OBJECT

  public:
    explicit QgsAttributesFormInitCode();

    void setCodeSource( QgsEditFormConfig::PythonInitCodeSource initCodeSourceComboBoxIndex );
    void setInitFunction( const QString &initFunction );
    void setInitFilePath( const QString &initFilePath );
    void setInitCode( const QString &initCode );

    QgsEditFormConfig::PythonInitCodeSource codeSource() const;
    QString initFunction() const;
    QString initFilePath() const;
    QString initCode() const;

  private:
    //Ui::QgsAttributesFormInitCode *ui;

  private slots:
    void mInitCodeSourceComboBox_currentIndexChanged( int codeSource );
    void showHelp();
};

#endif // QGSATTRIBUTESFORMINITCODE_H
