/********************************************************************************
** Form generated from reading UI file 'qgsdamengnewconnectionbase.ui'
**
** Created by: Qt User Interface Compiler version 5.15.3
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_QGSDAMENGNEWCONNECTIONBASE_H
#define UI_QGSDAMENGNEWCONNECTIONBASE_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QDialog>
#include <QtWidgets/QDialogButtonBox>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QVBoxLayout>
#include "auth/qgsauthsettingswidget.h"
#include "qgsmessagebar.h"

QT_BEGIN_NAMESPACE

class Ui_QgsDamengNewConnectionBase
{
public:
    QVBoxLayout *verticalLayout_2;
    QgsMessageBar *bar;
    QGroupBox *GroupBox1;
    QVBoxLayout *verticalLayout;
    QGridLayout *gridLayout_2;
    QLineEdit *txtPort;
    QLineEdit *txtName;
    QLabel *TextLabel1;
    QLabel *TextLabel2;
    QLabel *TextLabel1_2;
    QLineEdit *txtHost;
    QGroupBox *mAuthGroupBox;
    QGridLayout *gridLayout;
    QgsAuthSettingsWidget *mAuthSettings;
    QPushButton *btnConnect;
    QCheckBox *cb_dontResolveType;
    QCheckBox *cb_sysdbaSchemaOnly;
    QCheckBox *cb_allowGeometrylessTables;
    QCheckBox *cb_useEstimatedMetadata;
    QCheckBox *cb_projectsInDatabase;
    QSpacerItem *verticalSpacer;
    QDialogButtonBox *buttonBox;

    void setupUi(QDialog *QgsDamengNewConnectionBase)
    {
        if (QgsDamengNewConnectionBase->objectName().isEmpty())
            QgsDamengNewConnectionBase->setObjectName(QString::fromUtf8("QgsDamengNewConnectionBase"));
        QgsDamengNewConnectionBase->resize(448, 556);
        QSizePolicy sizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(QgsDamengNewConnectionBase->sizePolicy().hasHeightForWidth());
        QgsDamengNewConnectionBase->setSizePolicy(sizePolicy);
        QgsDamengNewConnectionBase->setSizeGripEnabled(true);
        QgsDamengNewConnectionBase->setModal(true);
        verticalLayout_2 = new QVBoxLayout(QgsDamengNewConnectionBase);
        verticalLayout_2->setSpacing(6);
        verticalLayout_2->setContentsMargins(11, 11, 11, 11);
        verticalLayout_2->setObjectName(QString::fromUtf8("verticalLayout_2"));
        bar = new QgsMessageBar(QgsDamengNewConnectionBase);
        bar->setObjectName(QString::fromUtf8("bar"));

        verticalLayout_2->addWidget(bar);

        GroupBox1 = new QGroupBox(QgsDamengNewConnectionBase);
        GroupBox1->setObjectName(QString::fromUtf8("GroupBox1"));
        verticalLayout = new QVBoxLayout(GroupBox1);
        verticalLayout->setSpacing(6);
        verticalLayout->setContentsMargins(11, 11, 11, 11);
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        gridLayout_2 = new QGridLayout();
        gridLayout_2->setSpacing(6);
        gridLayout_2->setObjectName(QString::fromUtf8("gridLayout_2"));
        txtPort = new QLineEdit(GroupBox1);
        txtPort->setObjectName(QString::fromUtf8("txtPort"));

        gridLayout_2->addWidget(txtPort, 2, 1, 1, 1);

        TextLabel2 = new QLabel(GroupBox1);
        TextLabel2->setObjectName(QString::fromUtf8("TextLabel2"));

        gridLayout_2->addWidget(TextLabel2, 3, 0, 1, 1);

        txtName = new QLineEdit(GroupBox1);
        txtName->setObjectName(QString::fromUtf8("txtName"));

        gridLayout_2->addWidget(txtName, 0, 1, 1, 1);

        TextLabel1 = new QLabel(GroupBox1);
        TextLabel1->setObjectName(QString::fromUtf8("TextLabel1"));

        gridLayout_2->addWidget(TextLabel1, 1, 0, 1, 1);

        TextLabel2 = new QLabel(GroupBox1);
        TextLabel2->setObjectName(QString::fromUtf8("TextLabel2"));

        gridLayout_2->addWidget(TextLabel2, 2, 0, 1, 1);

        TextLabel1_2 = new QLabel(GroupBox1);
        TextLabel1_2->setObjectName(QString::fromUtf8("TextLabel1_2"));

        gridLayout_2->addWidget(TextLabel1_2, 0, 0, 1, 1);

        txtHost = new QLineEdit(GroupBox1);
        txtHost->setObjectName(QString::fromUtf8("txtHost"));

        gridLayout_2->addWidget(txtHost, 1, 1, 1, 1);


        verticalLayout->addLayout(gridLayout_2);

        mAuthGroupBox = new QGroupBox(GroupBox1);
        mAuthGroupBox->setObjectName(QString::fromUtf8("mAuthGroupBox"));
        gridLayout = new QGridLayout(mAuthGroupBox);
        gridLayout->setSpacing(6);
        gridLayout->setContentsMargins(11, 11, 11, 11);
        gridLayout->setObjectName(QString::fromUtf8("gridLayout"));
        gridLayout->setContentsMargins(6, 6, 6, 6);
        mAuthSettings = new QgsAuthSettingsWidget(mAuthGroupBox);
        mAuthSettings->setObjectName(QString::fromUtf8("mAuthSettings"));

        gridLayout->addWidget(mAuthSettings, 0, 0, 1, 1);


        verticalLayout->addWidget(mAuthGroupBox);

        btnConnect = new QPushButton(GroupBox1);
        btnConnect->setObjectName(QString::fromUtf8("btnConnect"));

        verticalLayout->addWidget(btnConnect);

        cb_dontResolveType = new QCheckBox(GroupBox1);
        cb_dontResolveType->setObjectName(QString::fromUtf8("cb_dontResolveType"));

        verticalLayout->addWidget(cb_dontResolveType);

        cb_sysdbaSchemaOnly = new QCheckBox(GroupBox1);
        cb_sysdbaSchemaOnly->setObjectName(QString::fromUtf8("cb_sysdbaSchemaOnly"));

        verticalLayout->addWidget(cb_sysdbaSchemaOnly);

        cb_allowGeometrylessTables = new QCheckBox(GroupBox1);
        cb_allowGeometrylessTables->setObjectName(QString::fromUtf8("cb_allowGeometrylessTables"));
        cb_allowGeometrylessTables->setChecked(false);

        verticalLayout->addWidget(cb_allowGeometrylessTables);

        cb_useEstimatedMetadata = new QCheckBox(GroupBox1);
        cb_useEstimatedMetadata->setObjectName(QString::fromUtf8("cb_useEstimatedMetadata"));

        verticalLayout->addWidget(cb_useEstimatedMetadata);

        cb_projectsInDatabase = new QCheckBox(GroupBox1);
        cb_projectsInDatabase->setObjectName(QString::fromUtf8("cb_projectsInDatabase"));

        verticalLayout->addWidget(cb_projectsInDatabase);

        verticalSpacer = new QSpacerItem(20, 0, QSizePolicy::Minimum, QSizePolicy::Expanding);

        verticalLayout->addItem(verticalSpacer);


        verticalLayout_2->addWidget(GroupBox1);

        buttonBox = new QDialogButtonBox(QgsDamengNewConnectionBase);
        buttonBox->setObjectName(QString::fromUtf8("buttonBox"));
        buttonBox->setStandardButtons(QDialogButtonBox::Cancel|QDialogButtonBox::Help|QDialogButtonBox::Ok);

        verticalLayout_2->addWidget(buttonBox);

#if QT_CONFIG(shortcut)
        TextLabel1->setBuddy(txtHost);
        TextLabel2->setBuddy(txtPort);
        TextLabel1_2->setBuddy(txtName);
#endif // QT_CONFIG(shortcut)
        QWidget::setTabOrder(txtName, txtHost);
        QWidget::setTabOrder(txtHost, txtPort);
        QWidget::setTabOrder(txtPort, btnConnect);
        QWidget::setTabOrder(btnConnect, cb_dontResolveType);
        QWidget::setTabOrder(cb_dontResolveType, cb_sysdbaSchemaOnly);
        QWidget::setTabOrder(cb_sysdbaSchemaOnly, cb_allowGeometrylessTables);
        QWidget::setTabOrder(cb_allowGeometrylessTables, cb_useEstimatedMetadata);
        QWidget::setTabOrder(cb_useEstimatedMetadata, cb_projectsInDatabase);

        retranslateUi(QgsDamengNewConnectionBase);
        QObject::connect(buttonBox, SIGNAL(rejected()), QgsDamengNewConnectionBase, SLOT(reject()));
        QObject::connect(buttonBox, SIGNAL(accepted()), QgsDamengNewConnectionBase, SLOT(accept()));

        QMetaObject::connectSlotsByName(QgsDamengNewConnectionBase);
    } // setupUi

    void retranslateUi(QDialog *QgsDamengNewConnectionBase)
    {
        QgsDamengNewConnectionBase->setWindowTitle(QCoreApplication::translate("QgsDamengNewConnectionBase", "Create a New Dameng Connection", nullptr));
        GroupBox1->setTitle(QCoreApplication::translate("QgsDamengNewConnectionBase", "Connection Information", nullptr));
        txtPort->setText(QCoreApplication::translate("QgsDamengNewConnectionBase", "5236", nullptr));
#if QT_CONFIG(tooltip)
        txtName->setToolTip(QCoreApplication::translate("QgsDamengNewConnectionBase", "Name of the new connection", nullptr));
#endif // QT_CONFIG(tooltip)
        TextLabel1->setText(QCoreApplication::translate("QgsDamengNewConnectionBase", "Hos&t", nullptr));
        TextLabel2->setText(QCoreApplication::translate("QgsDamengNewConnectionBase", "Port", nullptr));
        TextLabel1_2->setText(QCoreApplication::translate("QgsDamengNewConnectionBase", "&Name", nullptr));
        mAuthGroupBox->setTitle(QCoreApplication::translate("QgsDamengNewConnectionBase", "Authentication", nullptr));
        btnConnect->setText(QCoreApplication::translate("QgsDamengNewConnectionBase", "&Test Connection", nullptr));
        cb_dontResolveType->setText(QCoreApplication::translate("QgsDamengNewConnectionBase", "Don't resolve type of unrestricted columns (GEOMETRY)", nullptr));
#if QT_CONFIG(tooltip)
        cb_sysdbaSchemaOnly->setToolTip(QCoreApplication::translate("QgsDamengNewConnectionBase", "Restrict the search to the SYSDBA schema for spatial tables not in the geometry_columns table", nullptr));
#endif // QT_CONFIG(tooltip)
#if QT_CONFIG(whatsthis)
        cb_sysdbaSchemaOnly->setWhatsThis(QCoreApplication::translate("QgsDamengNewConnectionBase", "When searching for spatial tables that are not in the geometry_columns tables, restrict the search to tables that are in the public schema (for some databases this can save lots of time)", nullptr));
#endif // QT_CONFIG(whatsthis)
        cb_sysdbaSchemaOnly->setText(QCoreApplication::translate("QgsDamengNewConnectionBase", "Only look in the 'SYSDBA' schema", nullptr));
        cb_allowGeometrylessTables->setText(QCoreApplication::translate("QgsDamengNewConnectionBase", "Also list tables with no geometry", nullptr));
#if QT_CONFIG(tooltip)
        cb_useEstimatedMetadata->setToolTip(QCoreApplication::translate("QgsDamengNewConnectionBase", "Use estimated table statistics for the layer metadata.", nullptr));
#endif // QT_CONFIG(tooltip)
#if QT_CONFIG(whatsthis)
        cb_useEstimatedMetadata->setWhatsThis(QCoreApplication::translate("QgsDamengNewConnectionBase", "<html>\n"
"<body>\n"
"<p>When the layer is setup various metadata is required for the Dameng Spatial table. This includes information such as the table row count, geometry type and spatial extents of the data in the geometry column. If the table contains a large number of rows determining this metadata is time consuming.</p>\n"
"<p>By activating this option the following fast table metadata operations are done:</p>\n"
"<p>1) Row count is determined from results of running the Dameng Analyze function on the table.</p>\n"
"<p>2) Table extents are always determined with the estimated_extent Dameng Spatial function even if a layer filter is applied.</p>\n"
"<p>3) If the table geometry type is unknown and is not exclusively taken from the geometry_columns table, then it is determined from the first 100 non-null geometry rows in the table.</p>\n"
"</body>\n"
"</html>", nullptr));
#endif // QT_CONFIG(whatsthis)
        cb_useEstimatedMetadata->setText(QCoreApplication::translate("QgsDamengNewConnectionBase", "Use estimated table metadata", nullptr));
        cb_projectsInDatabase->setText(QCoreApplication::translate("QgsDamengNewConnectionBase", "Allow saving/loading QGIS projects in the database", nullptr));
    } // retranslateUi

};

namespace Ui {
    class QgsDamengNewConnectionBase: public Ui_QgsDamengNewConnectionBase {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_QGSDAMENGNEWCONNECTIONBASE_H
