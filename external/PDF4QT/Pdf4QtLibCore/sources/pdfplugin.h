//    Copyright (C) 2020-2021 Jakub Melka
//
//    This file is part of PDF4QT.
//
//    PDF4QT is free software: you can redistribute it and/or modify
//    it under the terms of the GNU Lesser General Public License as published by
//    the Free Software Foundation, either version 3 of the License, or
//    with the written consent of the copyright owner, any later version.
//
//    PDF4QT is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU Lesser General Public License for more details.
//
//    You should have received a copy of the GNU Lesser General Public License
//    along with PDF4QT.  If not, see <https://www.gnu.org/licenses/>.

#ifndef PDFPLUGIN_H
#define PDFPLUGIN_H

#include "pdfdocument.h"
#include "pdftextlayout.h"

#include <QObject>
#include <QJsonObject>

#include <vector>

class QAction;
class QMainWindow;

namespace pdf
{
class PDFWidget;
class PDFCMSManager;

struct PDF4QTLIBCORESHARED_EXPORT PDFPluginInfo
{
    QString name;
    QString author;
    QString version;
    QString license;
    QString description;
    QString pluginFile;
    QString pluginFileWithPath;

    static PDFPluginInfo loadFromJson(const QJsonObject* json);
};
using PDFPluginInfos = std::vector<PDFPluginInfo>;

class IPluginDataExchange
{
public:
    explicit IPluginDataExchange() = default;
    virtual ~IPluginDataExchange() = default;

    struct VoiceSettings
    {
        QString directory;
        QString voiceName;
        double volume = 1.0;
        double rate = 0.0;
        double pitch = 0.0;
    };

    virtual QString getOriginalFileName() const = 0;
    virtual pdf::PDFTextSelection getSelectedText() const = 0;
    virtual QMainWindow* getMainWindow() const = 0;
    virtual VoiceSettings getVoiceSettings() const = 0;
};

class PDF4QTLIBCORESHARED_EXPORT PDFPlugin : public QObject
{
    Q_OBJECT

public:
    explicit PDFPlugin(QObject* parent);

    virtual void setDataExchangeInterface(IPluginDataExchange* dataExchangeInterface);
    virtual void setWidget(PDFWidget* widget);
    virtual void setCMSManager(PDFCMSManager* manager);
    virtual void setDocument(const PDFModifiedDocument& document);
    virtual std::vector<QAction*> getActions() const;

protected:
    IPluginDataExchange* m_dataExchangeInterface;
    PDFWidget* m_widget;
    PDFCMSManager* m_cmsManager;
    PDFDocument* m_document;
};

}   // namespace pdf

#endif // PDFPLUGIN_H
