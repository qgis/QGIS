#ifndef WINTOASTLIB_H
#define WINTOASTLIB_H
#include <Windows.h>
#include <sdkddkver.h>
#include <WinUser.h>
#include <ShObjIdl.h>
#include <wrl/implements.h>
#include <wrl/event.h>
#include <windows.ui.notifications.h>
#include <strsafe.h>
#include <Psapi.h>
#include <ShlObj.h>
#include <roapi.h>
#include <propvarutil.h>
#include <functiondiscoverykeys.h>
#include <iostream>
#include <winstring.h>
#include <string.h>
#include <vector>
#include <map>
using namespace Microsoft::WRL;
using namespace ABI::Windows::Data::Xml::Dom;
using namespace ABI::Windows::Foundation;
using namespace ABI::Windows::UI::Notifications;
using namespace Windows::Foundation;

#define DEFAULT_SHELL_LINKS_PATH	L"\\Microsoft\\Windows\\Start Menu\\Programs\\"
#define DEFAULT_LINK_FORMAT			L".lnk"
namespace WinToastLib {

    class IWinToastHandler {
    public:
        enum WinToastDismissalReason {
            UserCanceled = ToastDismissalReason::ToastDismissalReason_UserCanceled,
            ApplicationHidden = ToastDismissalReason::ToastDismissalReason_ApplicationHidden,
            TimedOut = ToastDismissalReason::ToastDismissalReason_TimedOut
        };
        virtual ~IWinToastHandler() = default;
        virtual void toastActivated() const = 0;
        virtual void toastActivated(int actionIndex) const = 0;
        virtual void toastDismissed(WinToastDismissalReason state) const = 0;
        virtual void toastFailed() const = 0;
    };

    class WinToastTemplate {
    public:
        enum Duration { System, Short, Long };
        enum AudioOption { Default = 0, Silent = 1, Loop = 2 };
        enum TextField { FirstLine = 0, SecondLine, ThirdLine };
        enum WinToastTemplateType {
            ImageAndText01 = ToastTemplateType::ToastTemplateType_ToastImageAndText01,
            ImageAndText02 = ToastTemplateType::ToastTemplateType_ToastImageAndText02,
            ImageAndText03 = ToastTemplateType::ToastTemplateType_ToastImageAndText03,
            ImageAndText04 = ToastTemplateType::ToastTemplateType_ToastImageAndText04,
            Text01 = ToastTemplateType::ToastTemplateType_ToastText01,
            Text02 = ToastTemplateType::ToastTemplateType_ToastText02,
            Text03 = ToastTemplateType::ToastTemplateType_ToastText03,
            Text04 = ToastTemplateType::ToastTemplateType_ToastText04,
            WinToastTemplateTypeCount
        };

        WinToastTemplate(_In_ WinToastTemplateType type = WinToastTemplateType::ImageAndText02);
        ~WinToastTemplate();

        void setTextField(_In_ const std::wstring& txt, _In_ TextField pos);
        void setImagePath(_In_ const std::wstring& imgPath);
        void setAudioPath(_In_ const std::wstring& audioPath);
        void setAttributionText(_In_ const std::wstring & attributionText);
        void addAction(_In_ const std::wstring& label);
        void setAudioOption(_In_ WinToastTemplate::AudioOption audioOption);
        void setDuration(_In_ Duration duration);
        void setExpiration(_In_ INT64 millisecondsFromNow);
        std::size_t textFieldsCount() const;
        std::size_t actionsCount() const;
        bool hasImage() const;
        const std::vector<std::wstring>& textFields() const;
        const std::wstring& textField(_In_ TextField pos) const;
        const std::wstring& actionLabel(_In_ int pos) const;
        const std::wstring& imagePath() const;
        const std::wstring& audioPath() const;
        const std::wstring& attributionText() const;
        INT64 expiration() const;
        WinToastTemplateType type() const;
        WinToastTemplate::AudioOption audioOption() const;
        Duration duration() const;
    private:
        std::vector<std::wstring>			_textFields;
        std::vector<std::wstring>           _actions;
        std::wstring                        _imagePath = L"";
        std::wstring                        _audioPath = L"";
        std::wstring                        _attributionText = L"";
        INT64                               _expiration = 0;
        AudioOption                         _audioOption = WinToastTemplate::AudioOption::Default;
        WinToastTemplateType                _type = WinToastTemplateType::Text01;
        Duration                            _duration = Duration::System;
    };

    class WinToast {
    public:
        enum WinToastError {
            NoError = 0,
            NotInitialized,
            SystemNotSupported,
            ShellLinkNotCreated,
            InvalidAppUserModelID,
            InvalidParameters,
            InvalidHandler,
            NotDisplayed,
            UnknownError
        };

        enum ShortcutResult {
            SHORTCUT_UNCHANGED = 0,
            SHORTCUT_WAS_CHANGED = 1,
            SHORTCUT_WAS_CREATED = 2,

            SHORTCUT_MISSING_PARAMETERS = -1,
            SHORTCUT_INCOMPATIBLE_OS = -2,
            SHORTCUT_COM_INIT_FAILURE = -3,
            SHORTCUT_CREATE_FAILED = -4
        };

        WinToast(void);
        virtual ~WinToast();
        static WinToast* instance();
        static bool isCompatible();
		static bool	isSupportingModernFeatures();
		static std::wstring configureAUMI(_In_ const std::wstring& companyName,
                                                    _In_ const std::wstring& productName,
                                                    _In_ const std::wstring& subProduct = std::wstring(),
                                                    _In_ const std::wstring& versionInformation = std::wstring()
                                                    );
        virtual bool initialize(_Out_ WinToastError* error = nullptr);
        virtual bool isInitialized() const;
        virtual bool hideToast(_In_ INT64 id);
        virtual INT64 showToast(_In_ const WinToastTemplate& toast, _In_ IWinToastHandler* handler, _Out_ WinToastError* error = nullptr);
        virtual void clear();
        virtual enum ShortcutResult createShortcut();

        const std::wstring& appName() const;
        const std::wstring& appUserModelId() const;
        void setAppUserModelId(_In_ const std::wstring& appName);
        void setAppName(_In_ const std::wstring& appName);

    protected:
        bool											_isInitialized;
        bool                                            _hasCoInitialized;
        std::wstring                                    _appName;
        std::wstring                                    _aumi;
        std::map<INT64, ComPtr<IToastNotification>>     _buffer;

        HRESULT validateShellLinkHelper(_Out_ bool& wasChanged);
        HRESULT createShellLinkHelper();
        HRESULT setImageFieldHelper(_In_ IXmlDocument *xml, _In_ const std::wstring& path);
        HRESULT setAudioFieldHelper(_In_ IXmlDocument *xml, _In_ const std::wstring& path, _In_opt_ WinToastTemplate::AudioOption option = WinToastTemplate::AudioOption::Default);
        HRESULT setTextFieldHelper(_In_ IXmlDocument *xml, _In_ const std::wstring& text, _In_ int pos);
        HRESULT setAttributionTextFieldHelper(_In_ IXmlDocument *xml, _In_ const std::wstring& text);
        HRESULT addActionHelper(_In_ IXmlDocument *xml, _In_ const std::wstring& action, _In_ const std::wstring& arguments);
        HRESULT addDurationHelper(_In_ IXmlDocument *xml, _In_ const std::wstring& duration);
        ComPtr<IToastNotifier> notifier(_In_ bool* succeded) const;
        void setError(_Out_ WinToastError* error, _In_ WinToastError value);
    };
}
#endif // WINTOASTLIB_H
