# Google OAuth Quick Start

Get Google login working in 5 minutes.

## 1. Get Google Credentials

Visit [Google Cloud Console](https://console.cloud.google.com/):

1. Create/select project
2. **APIs & Services** > **Credentials**
3. **Create Credentials** > **OAuth 2.0 Client ID**
4. Choose **Web application**
5. Add authorized redirect URI: `http://localhost:8080/auth/google/callback`
6. Copy your **Client ID** and **Client Secret**

## 2. Configure Environment

Create `.env` file:

```bash
GOOGLE_CLIENT_ID=your_client_id.apps.googleusercontent.com
GOOGLE_CLIENT_SECRET=your_secret_here
GOOGLE_REDIRECT_URI=http://localhost:8080/auth/google/callback
PORT=8080
```

## 3. Build & Run

```bash
# Export variables
export $(cat .env | xargs)

# Build
cd build
cmake ..
make

# Run
cd ..
./build/calang
```

## 4. Test

1. Open `http://localhost:8080/login_page`
2. Click **Continue with Google**
3. Sign in with Google
4. Redirected to dashboard ✓

## Troubleshooting

**"redirect_uri_mismatch"**: URI in `.env` must exactly match Google Console setting

**"Missing environment variables"**: Run `export $(cat .env | xargs)` before starting app

**OAuth consent screen error**: Add yourself as a test user in Google Console

## What Happens

- First login: Account auto-created with your Google email
- Session cookie created automatically
- Works with existing username/password login
- No database changes needed

## Production

For production deployment:
- Use HTTPS
- Update redirect URI to your domain
- Add `Secure` flag to cookies
- Publish OAuth consent screen

See [GOOGLE_OAUTH_SETUP.md](GOOGLE_OAUTH_SETUP.md) for complete documentation.
