/****************************************************************************
** Form interface generated from reading ui file 'layerselectorbase.ui'
**
** Created: Thu Feb 17 01:10:15 2005
**      by: The User Interface Compiler ($Id$)
**
** WARNING! All changes made in this file will be lost!
****************************************************************************/

#ifndef LAYERSELECTORBASE_H
#define LAYERSELECTORBASE_H

#include <qvariant.h>
#include <qpixmap.h>
#include <qdialog.h>

class QVBoxLayout;
class QHBoxLayout;
class QGridLayout;
class QSpacerItem;
class QLabel;
class QPushButton;
class QListView;
class QListViewItem;

class LayerSelectorBase : public QDialog
{
    Q_OBJECT

public:
    LayerSelectorBase( QWidget* parent = 0, const char* name = 0, bool modal = FALSE, WFlags fl = 0 );
    ~LayerSelectorBase();

    QLabel* lblBaseDir;
    QPushButton* pbnDirectorySelector;
    QListView* listFileTree;
    QPushButton* pbnOK;
    QPushButton* pbnCancel;

public slots:
    virtual void pbnDirectorySelector_clicked();

protected:
    QGridLayout* LayerSelectorBaseLayout;
    QSpacerItem* spacer19;

protected slots:
    virtual void languageChange();

private:
    QPixmap image0;

};

#endif // LAYERSELECTORBASE_H
