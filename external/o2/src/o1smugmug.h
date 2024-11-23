#ifndef O1SMUGMUG_H
#define O1SMUGMUG_H

#include "o0export.h"
#include "o1.h"

#if QT_VERSION >= 0x050000
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
    Q_ENUMS(Access)
    Q_ENUMS(Permissions)

public:
    enum Access {
        AccessPublic,
        AccessFull
    };

    enum Permissions {
        PermissionsRead,
        PermissionsAdd,
        PermissionsModify
    };

    Q_INVOKABLE void initAuthorizationUrl(Access access, Permissions permissions);

#if QT_VERSION >= 0x050000
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
#endif // QT_VERSION >= 0x050000

    explicit O1SmugMug(QObject *parent = 0, QNetworkAccessManager *manager = 0, O0AbstractStore *store = 0);
};

#endif // O1SMUGMUG_H
