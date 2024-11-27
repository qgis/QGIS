#ifndef O1SMUGMUG_H
#define O1SMUGMUG_H

#include "o0export.h"
#include "o1.h"

#if QT_VERSION >= QT_VERSION_CHECK(5,0,0)
#include <QUrlQuery>
#endif

/// SmugMug OAuth 1.0 client
///
/// Simple usage:
/// @code
/// o1_ = new O1SmugMug(this);
/// o1_->initAuthorizationUrl(O1SmugMug::AccessFull, O1SmugMug::PermissionsAdd);
/// @endcode
///
/// Advanced usage (Qt 5.0 or later):
/// @code
/// o1_ = new O1SmugMug(this, 0, secureStore);
/// O1SmugMug::AuthorizationUrlBuilder builder;
/// builder.setAccess(O1SmugMug::AccessFull);
/// builder.setShowSignUpButton(false);
/// builder.setPrepopulatedUsername(lastUsername_);
/// o1_->initAuthorizationUrl(builder);
/// @endcode
class O0_EXPORT O1SmugMug: public O1 {
    Q_OBJECT

public:
    enum Access {
        AccessPublic,
        AccessFull
    };
    Q_ENUM(Access)

    enum Permissions {
        PermissionsRead,
        PermissionsAdd,
        PermissionsModify
    };
    Q_ENUM(Permissions)

    Q_INVOKABLE void initAuthorizationUrl(O1SmugMug::Access access, O1SmugMug::Permissions permissions);

#if QT_VERSION >= QT_VERSION_CHECK(5,0,0)
    class AuthorizationUrlBuilder {
    public:
        void setAccess(Access value);
        void setPermissions(Permissions value);
        void setAllowThirdPartyLogin(bool value);
        void setShowSignUpButton(bool value);
        void setPrepopulatedUsername(const QString &value);
        void setViewportScale(double value);

        QUrl url() const;

    private:
        QUrlQuery query_;
    };

    void initAuthorizationUrl(const AuthorizationUrlBuilder &builder);
#endif // QT_VERSION >= QT_VERSION_CHECK(5,0,0)

    explicit O1SmugMug(QObject *parent = nullptr, QNetworkAccessManager *manager = nullptr, O0AbstractStore *store = nullptr);
};

#endif // O1SMUGMUG_H
