/****************************************************************************
** Form interface generated from reading ui file 'pluginguibase.ui'
**
** Created: Tue Mar 15 00:43:08 2005
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
class QPushButton;
class QLabel;
class QTextEdit;
class QLineEdit;

class PluginGuiBase : public QDialog
{
    Q_OBJECT

public:
    PluginGuiBase( QWidget* parent = 0, const char* name = 0, bool modal = FALSE, WFlags fl = 0 );
    ~PluginGuiBase();

    QFrame* line1;
    QPushButton* pbnOK;
    QPushButton* pbnCancel;
    QLabel* txtHeading;
    QTextEdit* teInstructions_2;
    QLabel* textLabel3;
    QLabel* textLabel1;
    QLabel* textLabel2_2;
    QLineEdit* leLongitude;
    QLabel* textLabel1_4;
    QLineEdit* leEmail;
    QLabel* textLabel1_3;
    QLineEdit* leName;
    QLineEdit* leCountry;
    QLabel* textLabel1_2;
    QLineEdit* leImageUrl;
    QLineEdit* leHomeUrl;
    QLabel* textLabel2;
    QLabel* textLabel3_2;
    QLineEdit* leLatitude;
    QLineEdit* lePlaceDescription;
    QLabel* pixmapLabel2;
    QPushButton* pbnGetCoords;

public slots:
    virtual void pbnOK_clicked();
    virtual void pbnCancel_clicked();
    virtual void pbnGetCoords_clicked();

protected:
    QGridLayout* PluginGuiBaseLayout;
    QHBoxLayout* layout73;
    QSpacerItem* spacer2;
    QGridLayout* layout3;

protected slots:
    virtual void languageChange();

private:
    QPixmap image0;
    QPixmap image1;

};

#endif // PLUGINGUIBASE_H
