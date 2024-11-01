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

#include "qgis_gui.h"
#include <QWidget>

class QDialog;

/**
 * \ingroup gui
 * \class QgsAttributesFormInitCode
 * \brief A dialog for configuring the Python init code handling for attribute forms.
 */
class GUI_EXPORT QgsAttributesFormInitCode : public QDialog, private Ui::QgsAttributesFormInitCode
{
    Q_OBJECT

  public:
    explicit QgsAttributesFormInitCode();

    /**
     * Sets the Python init code \a source.
     *
     * \see codeSource()
     */
    void setCodeSource( Qgis::AttributeFormPythonInitCodeSource source );

    /**
     * Sets the name of the init function.
     *
     * \see initFunction()
     */
    void setInitFunction( const QString &initFunction );

    /**
     * Sets the file path for the file containing the init code.
     *
     * \see initFilePath()
     */
    void setInitFilePath( const QString &initFilePath );

    /**
     * Sets the init code contents.
     *
     * \see initCode()
     */
    void setInitCode( const QString &initCode );

    /**
     * Returns the Python init code source.
     *
     * \see setCodeSource()
     */
    Qgis::AttributeFormPythonInitCodeSource codeSource() const;

    /**
     * Returns the name of the init function.
     *
     * \see setInitFunction()
     */
    QString initFunction() const;

    /**
     * Returns the file path for the file containing the init code.
     *
     * \see setInitFilePath()
     */
    QString initFilePath() const;

    /**
     * Returns the init code contents.
     *
     * \see setInitCode()
     */
    QString initCode() const;

  private:
    //Ui::QgsAttributesFormInitCode *ui;

  private slots:
    void mInitCodeSourceComboBox_currentIndexChanged( int codeSource );
    void showHelp();
};

#endif // QGSATTRIBUTESFORMINITCODE_H
