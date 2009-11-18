// $Id$
//////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2008 by Mateusz Loskot <mateusz@loskot.net>
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License,
// or (at your option) any later version.
//
//////////////////////////////////////////////////////////////////////////////
#ifndef QGIS_PLUGIN_OGRCONV_DIALOG_H_INCLUDED
#define QGIS_PLUGIN_OGRCONV_DIALOG_H_INCLUDED

// qgis::plugin::ogrconv
#include "format.h"
#include <ui_ogrconverterguibase.h>
// Qt4
#include <QDialog>
#include "qgscontexthelp.h"

/**
@author Mateusz Loskot
*/
class Dialog : public QDialog, private Ui::OgrConverterGuiBase
{
    Q_OBJECT

  public:

    Dialog( QWidget* parent = 0, Qt::WFlags fl = 0 );
    ~Dialog();

  private:
    FormatsRegistry mFrmts;
    Format mSrcFormat;
    Format mDstFormat;

    void resetSrcUi();
    void resetDstUi();
    void setButtonState( QPushButton* btn, bool isProtocol );

    void populateFormats();
    void populateLayers( QString const& url );
    bool testConnection( QString const& url );
    QString openFile();
    QString openDirectory();

  private slots:

    void on_buttonBox_accepted();
    void on_buttonBox_rejected();
    void on_buttonBox_helpRequested() { QgsContextHelp::run( metaObject()->className() ); }
    void on_radioSrcFile_toggled( bool checked );
    void on_radioSrcDirectory_toggled( bool checked );
    void on_radioSrcProtocol_toggled( bool checked );
    void on_buttonSelectSrc_clicked();
    void on_buttonSelectDst_clicked();
    void on_comboSrcFormats_currentIndexChanged( int index );
    void on_comboDstFormats_currentIndexChanged( int index );
};

#endif // QGIS_PLUGIN_OGRCONV_DIALOG_H_INCLUDED
