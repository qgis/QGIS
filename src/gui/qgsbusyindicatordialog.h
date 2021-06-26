/***************************************************************************
                          qgsbusyindicatordialog.h
                          ------------------------
    begin                : Mar 27, 2013
    copyright            : (C) 2013 by Larry Shaffer
    email                : larrys at dakcarto dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   *
 *  
 *        *
 *                                     *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSBUSYINDICATORDIALOG_H
#define QGSBUSYINDICATORDIALOG_H

#include "qgsguiutils.h"

#include <QDialog>
#include <QLabel>
#include "qgis_gui.h"
#include "qgis_sip.h"


/**
 * \ingroup gui
 * \class QgsBusyIndicatorDialog
 * A simple dialog to show an indeterminate busy progress indicator.
 */
class GUI_EXPORT QgsBusyIndicatorDialog : public QDialog
{
    Q_OBJECT
  public:

    /**
     * Constructor
     * Modal busy indicator dialog with no buttons.
     * \param message Text to show above busy progress indicator.
     * \param parent parent object (owner)
     * \param fl widget flags
     */
    QgsBusyIndicatorDialog( const QString &message = QString(), QWidget *parent SIP_TRANSFERTHIS = nullptr, Qt::WindowFlags fl = QgsGuiUtils::ModalDialogFlags );

    QString message() const { return mMessage; }
    void setMessage( const QString &message );

  private:
    QString mMessage;
    QLabel *mMsgLabel = nullptr;
};

// clazy:excludeall=qstring-allocations

#endif // QGSBUSYINDICATORDIALOG_H
