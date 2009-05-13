
#ifndef QGSSTYLEV2MANAGERDIALOG_H
#define QGSSTYLEV2MANAGERDIALOG_H

#include <QDialog>

#include "ui_qgsstylev2managerdialogbase.h"

class QgsStyleV2;

class QgsStyleV2ManagerDialog : public QDialog, private Ui::QgsStyleV2ManagerDialogBase
{
  Q_OBJECT
  
public:
  QgsStyleV2ManagerDialog(QgsStyleV2* style, QString styleFilename, QWidget* parent = NULL);
  
public slots:
  void addItem();
  void editItem();
  void removeItem();
  //! adds symbols of some type to list
  void populateList();
  
  //! called when the dialog is going to be closed
  void onFinished();

protected:
  
  //! populate combo box with known style items (symbols, color ramps)
  void populateTypes();
  
  //! populate list view with symbols of specified type
  void populateSymbols(int type);
  //! populate list view with color ramps
  void populateColorRamps();
  
  int currentItemType();
  QString currentItemName();
  
  //! add a new symbol to style
  bool addSymbol();
  //! add a new color ramp to style
  bool addColorRamp();
  
  bool editSymbol();
  bool editColorRamp();
  
  bool removeSymbol();
  bool removeColorRamp();
  
  QgsStyleV2* mStyle;

  QString mStyleFilename;
};

#endif
