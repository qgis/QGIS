/****************************************************************************
** Form interface generated from reading ui file 'pluginguibase.ui'
**
** Created: Fri Mar 5 13:08:43 2004
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
class QFrame;
class QLabel;
class QTextEdit;
class QPushButton;
class QSpinBox;
class QLineEdit;

class PluginGuiBase : public QDialog
{
    Q_OBJECT

public:
    PluginGuiBase( QWidget* parent = 0, const char* name = 0, bool modal = FALSE, WFlags fl = 0 );
    ~PluginGuiBase();

    QFrame* line1;
    QLabel* pixmapLabel1;
    QLabel* txtHeading;
    QTextEdit* teInstructions;
    QLabel* lblInputFile;
    QLabel* lblOutputFile;
    QLabel* lblMinTimeGap;
    QLabel* lblMinDistanceGap;
    QPushButton* pbnSelectInputFile;
    QPushButton* pbnSelectOutputFile;
    QSpinBox* spinMinTimeGap;
    QSpinBox* spinMinDistanceGap;
    QPushButton* pbnCancel;
    QLineEdit* leInputFile;
    QLineEdit* leOutputShapeFile;
    QPushButton* pbnOK;

public slots:
    virtual void pbnOK_clicked();
    virtual void pbnSelectInputFile_clicked();
    virtual void pbnSelectOutputFile_clicked();
    virtual void pbnCancel_clicked();

protected:

protected slots:
    virtual void languageChange();

private:
    QPixmap image0;
    QPixmap image1;

};

#endif // PLUGINGUIBASE_H
