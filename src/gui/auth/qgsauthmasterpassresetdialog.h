/***************************************************************************
    qgsauthmasterpassresetdialog.h
    ---------------------
    begin                : September 10, 2015
    copyright            : (C) 2015 by Boundless Spatial, Inc. USA
    author               : Larry Shaffer
    email                : lshaffer at boundlessgeo dot com
 ***************************************************************************
 *                                                                         *
 *   *
 *  
 *        *
 *                                     *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSAUTHMASTERPASSWORDRESETDIALOG_H
#define QGSAUTHMASTERPASSWORDRESETDIALOG_H

#include <QDialog>

#include "ui_qgsauthmasterpassresetdialog.h"
#include "qgis_gui.h"

#define SIP_NO_FILE

class QLabel;
class QVBoxLayout;
class QgsMessageBar;


/**
 * \ingroup gui
 * \brief Dialog to verify current master password and initiate reset of
 * authentication database with a new password
 * \note not available in Python bindings
 */
class GUI_EXPORT QgsMasterPasswordResetDialog : public QDialog, private Ui::QgsMasterPasswordResetDialog
{
    Q_OBJECT

  public:
    explicit QgsMasterPasswordResetDialog( QWidget *parent = nullptr );

    bool requestMasterPasswordReset( QString *newpass, QString *oldpass, bool *keepbackup );

  private slots:
    void leMasterPassCurrent_textChanged( const QString &pass );
    void leMasterPassNew_textChanged( const QString &pass );

  private:
    void validatePasswords();

    bool mPassCurOk = false;
    bool mPassNewOk = false;
    QVBoxLayout *mAuthNotifyLayout = nullptr;
    QLabel *mAuthNotify = nullptr;
};

#endif // QGSAUTHMASTERPASSWORDRESETDIALOG_H
