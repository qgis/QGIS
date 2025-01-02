#ifndef O0GLOBALS_H
#define O0GLOBALS_H

// Common constants
const char O2_ENCRYPTION_KEY[] = "12345678";
const char O2_CALLBACK_URL[] = "http://127.0.0.1:%1/";
const char O2_MIME_TYPE_XFORM[] = "application/x-www-form-urlencoded";
const char O2_MIME_TYPE_JSON[] = "application/json";

// QSettings key names
const char O2_KEY_TOKEN[] = "token.%1";
const char O2_KEY_TOKEN_SECRET[] = "tokensecret.%1";
const char O2_KEY_CODE[] = "code.%1";
const char O2_KEY_EXPIRES[] = "expires.%1";
const char O2_KEY_REFRESH_TOKEN[] = "refreshtoken.%1";
const char O2_KEY_LINKED[] = "linked.%1";
const char O2_KEY_EXTRA_TOKENS[] = "extratokens.%1";

// OAuth 1/1.1 Request Parameters
const char O2_OAUTH_CALLBACK[] = "oauth_callback";
const char O2_OAUTH_CONSUMER_KEY[] = "oauth_consumer_key";
const char O2_OAUTH_NONCE[] = "oauth_nonce";
const char O2_OAUTH_SIGNATURE[] = "oauth_signature";
const char O2_OAUTH_SIGNATURE_METHOD[] = "oauth_signature_method";
const char O2_OAUTH_TIMESTAMP[] = "oauth_timestamp";
const char O2_OAUTH_VERSION[] = "oauth_version";

// OAuth 1/1.1 Response Parameters
const char O2_OAUTH_TOKEN[] = "oauth_token";
const char O2_OAUTH_TOKEN_SECRET[] = "oauth_token_secret";
const char O2_OAUTH_CALLBACK_CONFIRMED[] = "oauth_callback_confirmed";
const char O2_OAUTH_VERFIER[] = "oauth_verifier";

// OAuth 2 Request Parameters
const char O2_OAUTH2_RESPONSE_TYPE[] = "response_type";
const char O2_OAUTH2_CLIENT_ID[] = "client_id";
const char O2_OAUTH2_CLIENT_SECRET[] = "client_secret";
const char O2_OAUTH2_USERNAME[] = "username";
const char O2_OAUTH2_PASSWORD[] = "password";
const char O2_OAUTH2_REDIRECT_URI[] = "redirect_uri";
const char O2_OAUTH2_SCOPE[] = "scope";
const char O2_OAUTH2_GRANT_TYPE_CODE[] = "code";
const char O2_OAUTH2_GRANT_TYPE_TOKEN[] = "token";
const char O2_OAUTH2_GRANT_TYPE_PASSWORD[] = "password";
const char O2_OAUTH2_GRANT_TYPE_DEVICE[] = "urn:ietf:params:oauth:grant-type:device_code";
const char O2_OAUTH2_GRANT_TYPE[] = "grant_type";
const char O2_OAUTH2_API_KEY[] = "api_key";
const char O2_OAUTH2_STATE[] = "state";
const char O2_OAUTH2_CODE[] = "code";

// OAuth 2 Response Parameters
const char O2_OAUTH2_ACCESS_TOKEN[] = "access_token";
const char O2_OAUTH2_REFRESH_TOKEN[] = "refresh_token";
const char O2_OAUTH2_EXPIRES_IN[] = "expires_in";
const char O2_OAUTH2_DEVICE_CODE[] = "device_code";
const char O2_OAUTH2_USER_CODE[] = "user_code";
const char O2_OAUTH2_VERIFICATION_URI[] = "verification_uri";
const char O2_OAUTH2_VERIFICATION_URL[] = "verification_url"; // Google sign-in
const char O2_OAUTH2_VERIFICATION_URI_COMPLETE[] = "verification_uri_complete";
const char O2_OAUTH2_INTERVAL[] = "interval";

// OAuth signature types
const char O2_SIGNATURE_TYPE_HMAC_SHA1[] = "HMAC-SHA1";
const char O2_SIGNATURE_TYPE_HMAC_SHA256[] = "HMAC-SHA256";
const char O2_SIGNATURE_TYPE_PLAINTEXT[] = "PLAINTEXT";

// Parameter values
const char O2_AUTHORIZATION_CODE[] = "authorization_code";

// Standard HTTP headers
const char O2_HTTP_HTTP_HEADER[] = "HTTP";
const char O2_HTTP_AUTHORIZATION_HEADER[] = "Authorization";

// PKCE parameters
const char O2_OAUTH2_PKCE_CODE_VERIFIER_PARAM[] = "code_verifier";
const char O2_OAUTH2_PKCE_CODE_CHALLENGE_PARAM[] = "code_challenge";
const char O2_OAUTH2_PKCE_CODE_CHALLENGE_METHOD_S256[] = "S256";
const char O2_OAUTH2_PKCE_CODE_CHALLENGE_METHOD_PARAM[] = "code_challenge_method";

#endif // O0GLOBALS_H
