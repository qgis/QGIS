/********************************************************************************
** Form generated from reading UI file 'qgsdamengprojectstoragedialog.ui'
**
** Created by: Qt User Interface Compiler version 5.15.3
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_QGSDAMENGPROJECTSTORAGEDIALOG_H
#define UI_QGSDAMENGPROJECTSTORAGEDIALOG_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QDialog>
#include <QtWidgets/QDialogButtonBox>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QVBoxLayout>

QT_BEGIN_NAMESPACE

class Ui_QgsDamengProjectStorageDialog
{
  public:
    QVBoxLayout *verticalLayout;
    QGridLayout *gridLayout;
    QLabel *label;
    QComboBox *mCboConnection;
    QLabel *label_2;
    QComboBox *mCboSchema;
    QLabel *label_3;
    QComboBox *mCboProject;
    QLabel *mLblProjectsNotAllowed;
    QSpacerItem *verticalSpacer;
    QDialogButtonBox *buttonBox;

    void setupUi( QDialog *QgsDamengProjectStorageDialog )
    {
      if ( QgsDamengProjectStorageDialog->objectName().isEmpty() )
        QgsDamengProjectStorageDialog->setObjectName( QString::fromUtf8( "QgsDamengProjectStorageDialog" ) );
      QgsDamengProjectStorageDialog->resize( 552, 442 );
      verticalLayout = new QVBoxLayout( QgsDamengProjectStorageDialog );
      verticalLayout->setObjectName( QString::fromUtf8( "verticalLayout" ) );
      gridLayout = new QGridLayout();
      gridLayout->setObjectName( QString::fromUtf8( "gridLayout" ) );
      label = new QLabel( QgsDamengProjectStorageDialog );
      label->setObjectName( QString::fromUtf8( "label" ) );

      gridLayout->addWidget( label, 0, 0, 1, 1 );

      mCboConnection = new QComboBox( QgsDamengProjectStorageDialog );
      mCboConnection->setObjectName( QString::fromUtf8( "mCboConnection" ) );

      gridLayout->addWidget( mCboConnection, 0, 1, 1, 1 );

      label_2 = new QLabel( QgsDamengProjectStorageDialog );
      label_2->setObjectName( QString::fromUtf8( "label_2" ) );

      gridLayout->addWidget( label_2, 1, 0, 1, 1 );

      mCboSchema = new QComboBox( QgsDamengProjectStorageDialog );
      mCboSchema->setObjectName( QString::fromUtf8( "mCboSchema" ) );

      gridLayout->addWidget( mCboSchema, 1, 1, 1, 1 );

      label_3 = new QLabel( QgsDamengProjectStorageDialog );
      label_3->setObjectName( QString::fromUtf8( "label_3" ) );

      gridLayout->addWidget( label_3, 2, 0, 1, 1 );

      mCboProject = new QComboBox( QgsDamengProjectStorageDialog );
      mCboProject->setObjectName( QString::fromUtf8( "mCboProject" ) );

      gridLayout->addWidget( mCboProject, 2, 1, 1, 1 );


      verticalLayout->addLayout( gridLayout );

      mLblProjectsNotAllowed = new QLabel( QgsDamengProjectStorageDialog );
      mLblProjectsNotAllowed->setObjectName( QString::fromUtf8( "mLblProjectsNotAllowed" ) );
      mLblProjectsNotAllowed->setAlignment( Qt::AlignCenter );
      mLblProjectsNotAllowed->setWordWrap( true );

      verticalLayout->addWidget( mLblProjectsNotAllowed );

      verticalSpacer = new QSpacerItem( 20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding );

      verticalLayout->addItem( verticalSpacer );

      buttonBox = new QDialogButtonBox( QgsDamengProjectStorageDialog );
      buttonBox->setObjectName( QString::fromUtf8( "buttonBox" ) );
      buttonBox->setOrientation( Qt::Horizontal );
      buttonBox->setStandardButtons( QDialogButtonBox::Cancel | QDialogButtonBox::Ok );

      verticalLayout->addWidget( buttonBox );


      retranslateUi( QgsDamengProjectStorageDialog );
      QObject::connect( buttonBox, SIGNAL( rejected() ), QgsDamengProjectStorageDialog, SLOT( reject() ) );

      QMetaObject::connectSlotsByName( QgsDamengProjectStorageDialog );
    } // setupUi

    void retranslateUi( QDialog *QgsDamengProjectStorageDialog )
    {
      label->setText( QCoreApplication::translate( "QgsDamengProjectStorageDialog", "Connection", nullptr ) );
      label_2->setText( QCoreApplication::translate( "QgsDamengProjectStorageDialog", "Schema", nullptr ) );
      label_3->setText( QCoreApplication::translate( "QgsDamengProjectStorageDialog", "Project", nullptr ) );
      mLblProjectsNotAllowed->setText( QCoreApplication::translate( "QgsDamengProjectStorageDialog", "Storage of QGIS projects is not enabled for this database connection.", nullptr ) );
      ( void ) QgsDamengProjectStorageDialog;
    } // retranslateUi
};

namespace Ui
{
  class QgsDamengProjectStorageDialog : public Ui_QgsDamengProjectStorageDialog
  {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_QGSDAMENGPROJECTSTORAGEDIALOG_H
