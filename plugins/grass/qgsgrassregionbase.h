/****************************************************************************
** Form interface generated from reading ui file 'qgsgrassregionbase.ui'
**
** Created: Sat Aug 14 10:43:40 2004
**      by: The User Interface Compiler ($Id$)
**
** WARNING! All changes made in this file will be lost!
****************************************************************************/

#ifndef QGSGRASSREGIONBASE_H
#define QGSGRASSREGIONBASE_H

#include <qvariant.h>
#include <qwidget.h>

class QVBoxLayout;
class QHBoxLayout;
class QGridLayout;
class QSpacerItem;
class QFrame;
class QLabel;
class QLineEdit;
class QRadioButton;
class QPushButton;
class QSpinBox;

class QgsGrassRegionBase : public QWidget
{
    Q_OBJECT

public:
    QgsGrassRegionBase( QWidget* parent = 0, const char* name = 0, WFlags fl = 0 );
    ~QgsGrassRegionBase();

    QFrame* frame4;
    QLabel* textLabel1;
    QLineEdit* mNorth;
    QFrame* frame19;
    QLabel* textLabel2;
    QLineEdit* mWest;
    QLabel* textLabel3;
    QLineEdit* mEast;
    QFrame* frame20;
    QLabel* textLabel4;
    QLineEdit* mSouth;
    QFrame* frame5;
    QRadioButton* mNSResRadio;
    QLineEdit* mNSRes;
    QRadioButton* mRowsRadio;
    QLineEdit* mRows;
    QRadioButton* mColsRadio;
    QLineEdit* mEWRes;
    QLineEdit* mCols;
    QRadioButton* mEWResRadio;
    QFrame* frame6;
    QLabel* textLabel7;
    QPushButton* mColorButton;
    QLabel* textLabel6;
    QSpinBox* mWidthSpinBox;
    QFrame* frame8;
    QPushButton* acceptButton;
    QPushButton* rejectButton;

public slots:
    virtual void accept();
    virtual void reject();

protected:
    QGridLayout* QgsGrassRegionBaseLayout;
    QHBoxLayout* frame4Layout;
    QSpacerItem* spacer1;
    QSpacerItem* spacer2;
    QHBoxLayout* frame19Layout;
    QSpacerItem* spacer3;
    QHBoxLayout* frame20Layout;
    QSpacerItem* spacer4;
    QSpacerItem* spacer5;
    QGridLayout* frame5Layout;
    QHBoxLayout* frame6Layout;
    QSpacerItem* spacer6;
    QHBoxLayout* frame8Layout;
    QSpacerItem* spacer7;
    QSpacerItem* spacer8;
    QSpacerItem* spacer9;

protected slots:
    virtual void languageChange();

};

#endif // QGSGRASSREGIONBASE_H
