/********************************************************************************
** Form generated from reading UI file 'qgsexpressionaddfunctionfiledialogbase.ui'
**
** Created by: Qt User Interface Compiler version 5.15.3
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef QGSEXPRESSIONADDFUNCTIONFILEDIALOGBASE_H
#define QGSEXPRESSIONADDFUNCTIONFILEDIALOGBASE_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QDialog>
#include <QtWidgets/QDialogButtonBox>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>

QT_BEGIN_NAMESPACE

class Ui_QgsAddFunctionFileDialogBase
{
  public:
    QGridLayout *gridLayout;
    QLabel *label;
    QComboBox *cboFileOptions;
    QLabel *lblNewFileName;
    QDialogButtonBox *buttonBox;
    QLineEdit *txtNewFileName;

    void setupUi( QDialog *QgsAddFunctionFileDialogBase )
    {
      if ( QgsAddFunctionFileDialogBase->objectName().isEmpty() )
        QgsAddFunctionFileDialogBase->setObjectName( QString::fromUtf8( "QgsAddFunctionFileDialogBase" ) );
      QgsAddFunctionFileDialogBase->resize( 277, 132 );
      gridLayout = new QGridLayout( QgsAddFunctionFileDialogBase );
      gridLayout->setObjectName( QString::fromUtf8( "gridLayout" ) );
      label = new QLabel( QgsAddFunctionFileDialogBase );
      label->setObjectName( QString::fromUtf8( "label" ) );

      gridLayout->addWidget( label, 0, 0, 1, 1 );

      cboFileOptions = new QComboBox( QgsAddFunctionFileDialogBase );
      cboFileOptions->addItem( QString() );
      cboFileOptions->addItem( QString() );
      cboFileOptions->setObjectName( QString::fromUtf8( "cboFileOptions" ) );

      gridLayout->addWidget( cboFileOptions, 0, 1, 1, 1 );

      lblNewFileName = new QLabel( QgsAddFunctionFileDialogBase );
      lblNewFileName->setObjectName( QString::fromUtf8( "lblNewFileName" ) );

      gridLayout->addWidget( lblNewFileName, 1, 0, 1, 1 );

      buttonBox = new QDialogButtonBox( QgsAddFunctionFileDialogBase );
      buttonBox->setObjectName( QString::fromUtf8( "buttonBox" ) );
      buttonBox->setOrientation( Qt::Horizontal );
      buttonBox->setStandardButtons( QDialogButtonBox::Cancel | QDialogButtonBox::Ok );

      gridLayout->addWidget( buttonBox, 2, 0, 1, 2 );

      txtNewFileName = new QLineEdit( QgsAddFunctionFileDialogBase );
      txtNewFileName->setObjectName( QString::fromUtf8( "txtNewFileName" ) );

      gridLayout->addWidget( txtNewFileName, 1, 1, 1, 1 );


      retranslateUi( QgsAddFunctionFileDialogBase );
      QObject::connect( buttonBox, SIGNAL( accepted() ), QgsAddFunctionFileDialogBase, SLOT( accept() ) );
      QObject::connect( buttonBox, SIGNAL( rejected() ), QgsAddFunctionFileDialogBase, SLOT( reject() ) );

      QMetaObject::connectSlotsByName( QgsAddFunctionFileDialogBase );
    } // setupUi

    void retranslateUi( QDialog *QgsAddFunctionFileDialogBase )
    {
      QgsAddFunctionFileDialogBase->setWindowTitle( QCoreApplication::translate( "QgsAddFunctionFileDialogBase", "Add Function File", nullptr ) );
      label->setText( QCoreApplication::translate( "QgsAddFunctionFileDialogBase", "Create", nullptr ) );
      cboFileOptions->setItemText( 0, QCoreApplication::translate( "QgsAddFunctionFileDialogBase", "Function file", nullptr ) );
      cboFileOptions->setItemText( 1, QCoreApplication::translate( "QgsAddFunctionFileDialogBase", "Project functions", nullptr ) );

      lblNewFileName->setText( QCoreApplication::translate( "QgsAddFunctionFileDialogBase", "File name", nullptr ) );
    } // retranslateUi

};

namespace Ui
{
  class QgsAddFunctionFileDialogBase: public Ui_QgsAddFunctionFileDialogBase {};
} // namespace Ui

QT_END_NAMESPACE

#endif // QGSEXPRESSIONADDFUNCTIONFILEDIALOGBASE_H
