/****************************************************************************
** Form interface generated from reading ui file 'pluginguibase.ui'
**
** Created: Wed Apr 27 14:43:25 2005
**      by: The User Interface Compiler ($Id$)
**
** WARNING! All changes made in this file will be lost!
****************************************************************************/

#ifndef QGSCOMMUNITYREGPLUGINGUIBASE_H
#define QGSCOMMUNITYREGPLUGINGUIBASE_H

#include <qvariant.h>
#include <qpixmap.h>
#include <qdialog.h>
//Added by qt3to4:
#include <Q3VBoxLayout>
#include <Q3HBoxLayout>
#include <QLabel>
#include <Q3GridLayout>
#include <Q3Frame>

class Q3VBoxLayout;
class Q3HBoxLayout;
class Q3GridLayout;
class QSpacerItem;
class Q3Frame;
class QPushButton;
class QLabel;
class Q3TextEdit;
class QLineEdit;

class QgsCommunityRegPluginGuiBase : public QDialog
{
    Q_OBJECT

public:
    QgsCommunityRegPluginGuiBase( QWidget* parent = 0, const char* name = 0, bool modal = FALSE, Qt::WFlags fl = 0 );
    ~QgsCommunityRegPluginGuiBase();

    Q3Frame* line1;
    QPushButton* pbnOK;
    QPushButton* pbnCancel;
    QLabel* txtHeading;
    Q3TextEdit* teInstructions_2;
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
    Q3GridLayout* QgsCommunityRegPluginGuiBaseLayout;
    Q3HBoxLayout* layout73;
    QSpacerItem* spacer2;
    Q3GridLayout* layout3;

protected slots:
    virtual void languageChange();

private:
    QPixmap image0;
    QPixmap image1;

};

#endif // QGSCOMMUNITYREGPLUGINGUIBASE_H
