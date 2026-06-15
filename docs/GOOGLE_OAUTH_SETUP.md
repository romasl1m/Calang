# Google OAuth2 Setup Guide

This guide explains how to set up Google OAuth2 login for your Crow C++ application.

## Features

- **OAuth2 Authorization Code Flow**: Secure authentication with Google
- **Automatic Account Creation**: Users are automatically created if their email doesn't exist
- **Session Integration**: Uses existing cookie-based session system
- **State Token Validation**: CSRF protection with state parameter validation

## Endpoints

- `GET /auth/google/login` - Initiates OAuth flow, redirects to Google login
- `GET /auth/google/callback` - Handles OAuth callback and creates session

## Setup Instructions

### 1. Create Google OAuth2 Credentials

1. Go to [Google Cloud Console](https://console.cloud.google.com/)
2. Create a new project or select an existing one
3. Navigate to **APIs & Services** > **Credentials**
4. Click **Create Credentials** > **OAuth 2.0 Client ID**
5. Configure the OAuth consent screen if prompted:
   - User Type: External (for testing) or Internal (for organization)
   - Add required information (app name, user support email, etc.)
   - Add scopes: `openid`, `email`, `profile`
6. Create OAuth 2.0 Client ID:
   - Application type: **Web application**
   - Name: Your app name (e.g., "Calang")
   - Authorized redirect URIs:
     - For local development: `http://localhost:8080/auth/google/callback`
     - For production: `https://yourdomain.com/auth/google/callback`
7. Save your **Client ID** and **Client Secret**

### 2. Configure Environment Variables

Copy the example environment file:

```bash
cp .env.example .env
```

Edit `.env` and add your credentials:

```bash
GOOGLE_CLIENT_ID=your_client_id_here.apps.googleusercontent.com
GOOGLE_CLIENT_SECRET=your_client_secret_here
GOOGLE_REDIRECT_URI=http://localhost:8080/auth/google/callback
PORT=8080
```

**Important**: Never commit `.env` file to version control. Add it to `.gitignore`.

### 3. Load Environment Variables

Before running your application, export the environment variables:

```bash
# Export environment variables
export $(cat .env | xargs)

# Or source them
source .env
```

Alternatively, use a tool like `dotenv` or modify your startup script.

### 4. Build and Run

```bash
mkdir -p build
cd build
cmake ..
make
cd ..
./build/calang
```

### 5. Test the Integration

1. Navigate to `http://localhost:8080/login_page`
2. Click **Continue with Google**
3. Sign in with your Google account
4. You'll be redirected back to `/dashboard` with an active session

## How It Works

### Authentication Flow

1. User clicks "Continue with Google" on the login page
2. App generates a random state token and stores it in a cookie
3. User is redirected to Google's OAuth consent screen
4. User approves access and Google redirects back with an authorization code
5. App validates the state token to prevent CSRF attacks
6. App exchanges the authorization code for an access token
7. App retrieves user information (email, name) using the access token
8. If user doesn't exist, app creates a new account automatically
9. App creates a session and sets the session cookie
10. User is redirected to the dashboard

### Security Features

- **State Token Validation**: Prevents CSRF attacks
- **HttpOnly Cookies**: Session cookies are not accessible via JavaScript
- **Secure Token Exchange**: Authorization code is exchanged server-side
- **Environment Variables**: Sensitive credentials never hardcoded

### Auto-Registration

When a user logs in with Google for the first time:
- A new user account is created with their email as the username
- A random temporary password is generated (not exposed to user)
- User directory and events file are created automatically
- User can immediately access the application

### Data Stored

For each new Google user:
- `users.json`: Email and temporary password hash
- `users/{email}/`: User directory
- `users/{email}/events.json`: Empty events array

## Troubleshooting

### "OAuth configuration error: Missing environment variables"

Make sure all three environment variables are set:
```bash
echo $GOOGLE_CLIENT_ID
echo $GOOGLE_CLIENT_SECRET
echo $GOOGLE_REDIRECT_URI
```

### "redirect_uri_mismatch" error

The redirect URI in your `.env` file must exactly match one of the URIs configured in Google Cloud Console. Check:
- Protocol (http vs https)
- Domain (localhost vs IP address)
- Port number
- Path (`/auth/google/callback`)

### "Access blocked: Authorization Error"

Your OAuth consent screen might not be configured or published:
1. Go to **APIs & Services** > **OAuth consent screen**
2. Complete all required fields
3. Add your email as a test user (for External apps in testing mode)

### Session not persisting

Check that:
- Cookies are enabled in your browser
- The `active_sessions` map is working correctly
- The session cookie is being set with the correct path (`Path=/`)

## Dependencies

The implementation uses:
- **libcurl**: For HTTP requests to Google's OAuth endpoints
- **nlohmann/json**: For JSON parsing
- **Crow**: Web framework with cookie support
- **Standard C++17**: For filesystem operations and random number generation

## Production Considerations

When deploying to production:

1. **Use HTTPS**: Update redirect URI to use `https://`
2. **Secure Cookies**: Add `Secure` flag to cookies:
   ```cpp
   res.add_header("Set-Cookie", "session_id=" + session_id + "; Path=/; HttpOnly; Secure; SameSite=Lax");
   ```
3. **Environment Variables**: Use proper secret management (e.g., AWS Secrets Manager, HashiCorp Vault)
4. **OAuth Consent Screen**: Publish your OAuth consent screen for public access
5. **Error Handling**: Implement proper logging and error reporting
6. **Rate Limiting**: Add rate limiting to prevent abuse
7. **Database**: Consider migrating from JSON files to a proper database

## API Reference

### User Info Response (from Google)

```json
{
  "id": "123456789",
  "email": "user@example.com",
  "verified_email": true,
  "name": "John Doe",
  "given_name": "John",
  "family_name": "Doe",
  "picture": "https://lh3.googleusercontent.com/..."
}
```

Currently, only the `email` field is used for account creation and session management.

## Further Reading

- [Google OAuth2 Documentation](https://developers.google.com/identity/protocols/oauth2)
- [OAuth2 Authorization Code Flow](https://www.rfc-editor.org/rfc/rfc6749#section-4.1)
- [Google OAuth2 Playground](https://developers.google.com/oauthplayground/) - Test API calls

## Support

For issues or questions:
1. Check the troubleshooting section above
2. Review Google Cloud Console error messages
3. Check application logs for detailed error information
4. Verify all environment variables are correctly set
