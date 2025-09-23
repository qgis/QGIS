// MIT License
//
// Copyright (c) 2018-2025 Jakub Melka and Contributors
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

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
    virtual QString getPluginMenuName() const = 0;

protected:
    IPluginDataExchange* m_dataExchangeInterface;
    PDFWidget* m_widget;
    PDFCMSManager* m_cmsManager;
    PDFDocument* m_document;
};

}   // namespace pdf

#endif // PDFPLUGIN_H
