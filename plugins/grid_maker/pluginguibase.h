/****************************************************************************
** Form interface generated from reading ui file 'pluginguibase.ui'
**
** Created: Tue Apr 6 10:12:02 2004
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
class QTextEdit;
class QPushButton;
class QSpinBox;
class QLineEdit;
class QFrame;

class PluginGuiBase : public QDialog
{
    Q_OBJECT

public:
    PluginGuiBase( QWidget* parent = 0, const char* name = 0, bool modal = FALSE, WFlags fl = 0 );
    ~PluginGuiBase();

    QLabel* txtHeading;
    QTextEdit* teInstructions;
    QLabel* lblOutputFile;
    QLabel* lblMinTimeGap;
    QLabel* lblMinDistanceGap;
    QPushButton* pbnSelectOutputFile;
    QSpinBox* spinLatInterval;
    QSpinBox* spinLongInterval;
    QPushButton* pbnCancel;
    QPushButton* pbnOK;
    QLineEdit* leOutputShapeFile;
    QLabel* pixmapLabel1;
    QFrame* line1;

public slots:
    virtual void pbnOK_clicked();
    virtual void pbnSelectInputFile_clicked();
    virtual void pbnSelectOutputFile_clicked();
    virtual void pbnCancel_clicked();
    virtual void leInputFile_textChanged( const QString & theQString );
    virtual void leOutputShapeFile_textChanged( const QString & theQString );

protected:
    QGridLayout* PluginGuiBaseLayout;
    QSpacerItem* spacer2;

protected slots:
    virtual void languageChange();

private:
    QPixmap image0;
    QPixmap image1;

};

#endif // PLUGINGUIBASE_H
