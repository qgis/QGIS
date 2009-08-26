/********************************************************************************
** Form generated from reading ui file 'qgsrasterterrainanalysisdialogbase.ui'
**
** Created: Fri Aug 21 16:16:26 2009
**      by: Qt User Interface Compiler version 4.5.0
**
** WARNING! All changes made in this file will be lost when recompiling ui file!
********************************************************************************/

#ifndef UI_QGSRASTERTERRAINANALYSISDIALOGBASE_H
#define UI_QGSRASTERTERRAINANALYSISDIALOGBASE_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QCheckBox>
#include <QtGui/QComboBox>
#include <QtGui/QDialog>
#include <QtGui/QDialogButtonBox>
#include <QtGui/QGridLayout>
#include <QtGui/QHBoxLayout>
#include <QtGui/QHeaderView>
#include <QtGui/QLabel>
#include <QtGui/QLineEdit>
#include <QtGui/QPushButton>

QT_BEGIN_NAMESPACE

class Ui_QgsRasterTerrainAnalysisDialogBase
{
  public:
    QGridLayout *gridLayout;
    QHBoxLayout *horizontalLayout;
    QLabel *mAnalysisLabel;
    QComboBox *mAnalysisComboBox;
    QLabel *mInputLayerLabel;
    QComboBox *mInputLayerComboBox;
    QLabel *mOutputLayerLabel;
    QHBoxLayout *horizontalLayout_2;
    QLineEdit *mOutputLayerLineEdit;
    QPushButton *mOutputLayerPushButton;
    QLabel *mOutputFormatLabel;
    QComboBox *mOutputFormatComboBox;
    QCheckBox *mAddResultToProjectCheckBox;
    QDialogButtonBox *mButtonBox;

    void setupUi( QDialog *QgsRasterTerrainAnalysisDialogBase )
    {
      if ( QgsRasterTerrainAnalysisDialogBase->objectName().isEmpty() )
        QgsRasterTerrainAnalysisDialogBase->setObjectName( QString::fromUtf8( "QgsRasterTerrainAnalysisDialogBase" ) );
      QgsRasterTerrainAnalysisDialogBase->resize( 355, 318 );
      gridLayout = new QGridLayout( QgsRasterTerrainAnalysisDialogBase );
      gridLayout->setObjectName( QString::fromUtf8( "gridLayout" ) );
      horizontalLayout = new QHBoxLayout();
      horizontalLayout->setObjectName( QString::fromUtf8( "horizontalLayout" ) );
      mAnalysisLabel = new QLabel( QgsRasterTerrainAnalysisDialogBase );
      mAnalysisLabel->setObjectName( QString::fromUtf8( "mAnalysisLabel" ) );

      horizontalLayout->addWidget( mAnalysisLabel );

      mAnalysisComboBox = new QComboBox( QgsRasterTerrainAnalysisDialogBase );
      mAnalysisComboBox->setObjectName( QString::fromUtf8( "mAnalysisComboBox" ) );
      QSizePolicy sizePolicy( QSizePolicy::Expanding, QSizePolicy::Fixed );
      sizePolicy.setHorizontalStretch( 0 );
      sizePolicy.setVerticalStretch( 0 );
      sizePolicy.setHeightForWidth( mAnalysisComboBox->sizePolicy().hasHeightForWidth() );
      mAnalysisComboBox->setSizePolicy( sizePolicy );

      horizontalLayout->addWidget( mAnalysisComboBox );


      gridLayout->addLayout( horizontalLayout, 0, 0, 1, 1 );

      mInputLayerLabel = new QLabel( QgsRasterTerrainAnalysisDialogBase );
      mInputLayerLabel->setObjectName( QString::fromUtf8( "mInputLayerLabel" ) );

      gridLayout->addWidget( mInputLayerLabel, 1, 0, 1, 1 );

      mInputLayerComboBox = new QComboBox( QgsRasterTerrainAnalysisDialogBase );
      mInputLayerComboBox->setObjectName( QString::fromUtf8( "mInputLayerComboBox" ) );

      gridLayout->addWidget( mInputLayerComboBox, 2, 0, 1, 1 );

      mOutputLayerLabel = new QLabel( QgsRasterTerrainAnalysisDialogBase );
      mOutputLayerLabel->setObjectName( QString::fromUtf8( "mOutputLayerLabel" ) );

      gridLayout->addWidget( mOutputLayerLabel, 3, 0, 1, 1 );

      horizontalLayout_2 = new QHBoxLayout();
      horizontalLayout_2->setObjectName( QString::fromUtf8( "horizontalLayout_2" ) );
      mOutputLayerLineEdit = new QLineEdit( QgsRasterTerrainAnalysisDialogBase );
      mOutputLayerLineEdit->setObjectName( QString::fromUtf8( "mOutputLayerLineEdit" ) );

      horizontalLayout_2->addWidget( mOutputLayerLineEdit );

      mOutputLayerPushButton = new QPushButton( QgsRasterTerrainAnalysisDialogBase );
      mOutputLayerPushButton->setObjectName( QString::fromUtf8( "mOutputLayerPushButton" ) );
      QSizePolicy sizePolicy1( QSizePolicy::Preferred, QSizePolicy::Fixed );
      sizePolicy1.setHorizontalStretch( 0 );
      sizePolicy1.setVerticalStretch( 0 );
      sizePolicy1.setHeightForWidth( mOutputLayerPushButton->sizePolicy().hasHeightForWidth() );
      mOutputLayerPushButton->setSizePolicy( sizePolicy1 );
      mOutputLayerPushButton->setMinimumSize( QSize( 20, 0 ) );

      horizontalLayout_2->addWidget( mOutputLayerPushButton );


      gridLayout->addLayout( horizontalLayout_2, 4, 0, 1, 1 );

      mOutputFormatLabel = new QLabel( QgsRasterTerrainAnalysisDialogBase );
      mOutputFormatLabel->setObjectName( QString::fromUtf8( "mOutputFormatLabel" ) );

      gridLayout->addWidget( mOutputFormatLabel, 5, 0, 1, 1 );

      mOutputFormatComboBox = new QComboBox( QgsRasterTerrainAnalysisDialogBase );
      mOutputFormatComboBox->setObjectName( QString::fromUtf8( "mOutputFormatComboBox" ) );

      gridLayout->addWidget( mOutputFormatComboBox, 6, 0, 1, 1 );

      mAddResultToProjectCheckBox = new QCheckBox( QgsRasterTerrainAnalysisDialogBase );
      mAddResultToProjectCheckBox->setObjectName( QString::fromUtf8( "mAddResultToProjectCheckBox" ) );

      gridLayout->addWidget( mAddResultToProjectCheckBox, 7, 0, 1, 1 );

      mButtonBox = new QDialogButtonBox( QgsRasterTerrainAnalysisDialogBase );
      mButtonBox->setObjectName( QString::fromUtf8( "mButtonBox" ) );
      mButtonBox->setOrientation( Qt::Horizontal );
      mButtonBox->setStandardButtons( QDialogButtonBox::Cancel | QDialogButtonBox::Ok );

      gridLayout->addWidget( mButtonBox, 8, 0, 1, 1 );


      retranslateUi( QgsRasterTerrainAnalysisDialogBase );
      QObject::connect( mButtonBox, SIGNAL( accepted() ), QgsRasterTerrainAnalysisDialogBase, SLOT( accept() ) );
      QObject::connect( mButtonBox, SIGNAL( rejected() ), QgsRasterTerrainAnalysisDialogBase, SLOT( reject() ) );

      QMetaObject::connectSlotsByName( QgsRasterTerrainAnalysisDialogBase );
    } // setupUi

    void retranslateUi( QDialog *QgsRasterTerrainAnalysisDialogBase )
    {
      QgsRasterTerrainAnalysisDialogBase->setWindowTitle( QApplication::translate( "QgsRasterTerrainAnalysisDialogBase", "Raster based terrain analysis", 0, QApplication::UnicodeUTF8 ) );
      mAnalysisLabel->setText( QApplication::translate( "QgsRasterTerrainAnalysisDialogBase", "Analysis:", 0, QApplication::UnicodeUTF8 ) );
      mInputLayerLabel->setText( QApplication::translate( "QgsRasterTerrainAnalysisDialogBase", "Input layer:", 0, QApplication::UnicodeUTF8 ) );
      mOutputLayerLabel->setText( QApplication::translate( "QgsRasterTerrainAnalysisDialogBase", "Output layer:", 0, QApplication::UnicodeUTF8 ) );
      mOutputLayerPushButton->setText( QApplication::translate( "QgsRasterTerrainAnalysisDialogBase", "...", 0, QApplication::UnicodeUTF8 ) );
      mOutputFormatLabel->setText( QApplication::translate( "QgsRasterTerrainAnalysisDialogBase", "Output format:", 0, QApplication::UnicodeUTF8 ) );
      mAddResultToProjectCheckBox->setText( QApplication::translate( "QgsRasterTerrainAnalysisDialogBase", "Add result to project", 0, QApplication::UnicodeUTF8 ) );
      Q_UNUSED( QgsRasterTerrainAnalysisDialogBase );
    } // retranslateUi

};

namespace Ui
{
  class QgsRasterTerrainAnalysisDialogBase: public Ui_QgsRasterTerrainAnalysisDialogBase {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_QGSRASTERTERRAINANALYSISDIALOGBASE_H
