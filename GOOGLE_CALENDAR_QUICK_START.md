# Google Calendar Integration - Quick Start

## Setup

1. **Google Cloud Console Setup:**
   ```
   - Go to https://console.cloud.google.com
   - Create a new project or select existing
   - Enable "Google Calendar API"
   - Create OAuth 2.0 credentials (Web application)
   - Add authorized redirect URI: http://localhost:8080/auth/google/callback
   - Copy Client ID and Client Secret
   ```

2. **Set Environment Variables:**
   ```bash
   export GOOGLE_CLIENT_ID="123456789.apps.googleusercontent.com"
   export GOOGLE_CLIENT_SECRET="GOCSPX-abc123..."
   export GOOGLE_REDIRECT_URI="http://localhost:8080/auth/google/callback"
   ```

3. **Run Application:**
   ```bash
   cd /home/roman/calang/build
   ./calang
   ```

## Usage

### Login with Google

1. Navigate to: http://localhost:8080
2. Click "Login with Google"
3. Grant calendar permissions
4. You're logged in!

### Create Events via Terminal

The `touch` command now creates events in both Calang and Google Calendar:

**Syntax 1: Manual format**
```bash
touch "Meeting Title" DD.MM HH:MM HH:MM "Description" [P=priority] [T=recurrence]
```

**Example:**
```bash
touch "Team Meeting" 13.06 14:00 15:00 "Discuss Q3 goals"
```

**Syntax 2: Length format**
```bash
touch "Title" in DD.MM HH:MM length HH:MM "Description"
```

**Example:**
```bash
touch "Code Review" in 14.06 10:00 length 01:30 "Review PR #123"
```

**Syntax 3: AI format (full datetime)**
```bash
touch "Title" "YYYY-MM-DD HH:MM" "YYYY-MM-DD HH:MM" "Description"
```

**Example:**
```bash
touch "Conference Call" "2026-06-15 09:00" "2026-06-15 10:00" "Monthly sync"
```

### Output

When successful, you'll see:
```
-> Google Calendar: Event created successfully (ID: abc123xyz)
An event created!
-> Title: Team Meeting
-> Start: 2026-06-13 14:00
-> End:   2026-06-13 15:00
-> Desc:  Discuss Q3 goals
-> Type:  private
-> Prior: medium
-> Recur: none
```

### Create Events via Web Interface

1. Navigate to dashboard
2. Fill in event form
3. Click "Create Event"
4. Event is created in both Calang and Google Calendar

## Verify

Check your Google Calendar at https://calendar.google.com to see the synced event!

## Troubleshooting

**Problem: Events not syncing to Google Calendar**

1. Check if you logged in via Google (not regular login)
2. Verify environment variables are set
3. Check token file exists: `users/{your-email}/google_token.json`
4. Look for errors in terminal output

**Problem: "Invalid Credentials" error**

- Access token may have expired (tokens last ~1 hour)
- Log out and log back in via Google
- Future enhancement: automatic token refresh

**Problem: Events created locally but not in Google Calendar**

- No error - this means you don't have a Google access token
- Use Google OAuth login instead of regular login
- Check stderr output for API errors

## Key Features

✅ Automatic sync when creating events
✅ Works with terminal commands
✅ Works with web interface  
✅ Supports all event details (title, start, end, description)
✅ Non-blocking (if Google API fails, local event still created)

## Limitations

⚠️ Token expires after ~1 hour (no auto-refresh yet)
⚠️ One-way sync only (Calang → Google Calendar)
⚠️ Recurring events created as separate Google Calendar events
⚠️ Timezone defaults to UTC
⚠️ No event updates/deletions synced to Google

## Implementation Details

**Files Modified:**
- `src/google_oauth.h` - Added function declarations
- `src/google_oauth.cpp` - Implemented calendar API integration
- `src/terminal.cpp` - Added sync to touch command
- `src/api.cpp` - Added sync to web API

**New Functions:**
- `createGoogleCalendarEvent()` - Creates event in Google Calendar
- `get_user_access_token()` - Retrieves stored access token

**API Used:**
- Google Calendar API v3
- Endpoint: `POST /calendar/v3/calendars/primary/events`
- Authorization: OAuth 2.0 Bearer token

## Example Session

```bash
# Start application
./calang

# In web browser: http://localhost:8080
# Click "Login with Google"
# Grant permissions

# In terminal interface:
$ touch "Sprint Planning" 15.06 13:00 14:30 "Plan sprint 42"
-> Google Calendar: Event created successfully (ID: abcdef123)
An event created!
-> Title: Sprint Planning
-> Start: 2026-06-15 13:00
-> End:   2026-06-15 14:30
-> Desc:  Plan sprint 42
-> Type:  private
-> Prior: medium
-> Recur: none

# Check Google Calendar - event is there!
```

## Next Steps

To enhance the integration:

1. Implement token refresh
2. Add two-way sync (import from Google)
3. Sync event updates and deletions
4. Support multiple calendars
5. Add proper timezone handling
6. Batch sync operations

For detailed information, see `GOOGLE_CALENDAR_INTEGRATION.md`.
