/****************************************************************************
** Form interface generated from reading ui file 'pluginguibase.ui'
**
** Created: Mon May 10 14:07:09 2004
**      by: The User Interface Compiler ($Id$)
**
** WARNING! All changes made in this file will be lost!
****************************************************************************/

#ifndef PLUGINGUIBASE_H
#define PLUGINGUIBASE_H

#include <qvariant.h>
#include <qpixmap.h>
#include <qdialog.h>

class QVBoxLayout;
class QHBoxLayout;
class QGridLayout;
class QSpacerItem;
class QLabel;
class QFrame;
class QPushButton;
class QTextEdit;
class QTabWidget;
class QWidget;
class QGroupBox;
class QSpinBox;
class QCheckBox;

class PluginGuiBase : public QDialog
{
    Q_OBJECT

public:
    PluginGuiBase( QWidget* parent = 0, const char* name = 0, bool modal = FALSE, WFlags fl = 0 );
    ~PluginGuiBase();

    QLabel* pixmapLabel2;
    QFrame* line1;
    QPushButton* pbnCancel;
    QPushButton* pbnOK;
    QTextEdit* teInstructions_2;
    QTabWidget* tabMain;
    QWidget* tabOptions;
    QGroupBox* groupBox1;
    QLabel* textLabel1;
    QSpinBox* spinBox1;
    QCheckBox* cboEnableServer;
    QGroupBox* groupBox1_2;
    QWidget* tabLogs;
    QTextEdit* teLogs;
    QWidget* tabDebug;
    QTextEdit* teDebug;

public slots:
    virtual void pbnOK_clicked();
    virtual void pbnCancel_clicked();

protected:
    QGridLayout* PluginGuiBaseLayout;
    QHBoxLayout* layout1;
    QSpacerItem* spacer2;
    QGridLayout* tabOptionsLayout;
    QGridLayout* groupBox1Layout;
    QGridLayout* tabLogsLayout;
    QGridLayout* tabDebugLayout;

protected slots:
    virtual void languageChange();

private:
    QPixmap image0;
    QPixmap image1;

};

#endif // PLUGINGUIBASE_H
