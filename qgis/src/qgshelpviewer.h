/****************************************************************************
** Form interface generated from reading ui file 'qgshelpviewer.ui'
**
** Created: Fre Jan 30 07:57:36 2004
**      by: The User Interface Compiler ($Id$)
**
** WARNING! All changes made in this file will be lost!
****************************************************************************/

#ifndef QGSHELPVIEWER_H
#define QGSHELPVIEWER_H

#include <qvariant.h>
#include <qdialog.h>

class QVBoxLayout;
class QHBoxLayout;
class QGridLayout;
class QSpacerItem;
class QPushButton;
class QTextBrowser;

class QgsHelpViewer : public QDialog
{
    Q_OBJECT

public:
    QgsHelpViewer( QWidget* parent = 0, const char* name = 0, bool modal = FALSE, WFlags fl = 0 );
    ~QgsHelpViewer();

    QPushButton* buttonCancel;
    QTextBrowser* textBrowser;

protected:
    QGridLayout* QgsHelpViewerLayout;
    QHBoxLayout* Layout1;
    QSpacerItem* Horizontal_Spacing2;

protected slots:
    virtual void languageChange();

};

#endif // QGSHELPVIEWER_H
