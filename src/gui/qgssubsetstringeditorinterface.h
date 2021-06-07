/***************************************************************************
    qgssubsetstringeditorinterface.h
     --------------------------------------
    Date                 : 15-Nov-2020
    Copyright            : (C) 2020 by Even Rouault
    Email                : even.rouault at spatials.com
****************************************************************************/
/***************************************************************************
 *                                                                         *
 *   *
 *  
 *        *
 *                                     *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSSUBSETSTRINGEDITORINTERFACE_H
#define QGSSUBSETSTRINGEDITORINTERFACE_H

#include <QDialog>
#include <QString>
#include "qgis.h"
#include "qgis_gui.h"
#include "qgsguiutils.h"

/**
 * \ingroup gui
 * \class QgsSubsetStringEditorInterface
 * \brief Interface for a dialog that can edit subset strings
 *
 * \since QGIS 3.18
 */
class GUI_EXPORT QgsSubsetStringEditorInterface: public QDialog
{
    Q_OBJECT

  public:
    //! Constructor
    QgsSubsetStringEditorInterface( QWidget *parent SIP_TRANSFERTHIS = nullptr,
                                    Qt::WindowFlags fl = QgsGuiUtils::ModalDialogFlags );

    //! Returns the subset string entered in the dialog.
    virtual QString subsetString() const = 0;

    //! Sets a subset string into the dialog.
    virtual void setSubsetString( const QString &subsetString ) = 0;
};

#endif
