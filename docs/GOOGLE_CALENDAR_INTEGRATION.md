# Google Calendar Integration for Calang

This document describes the Google Calendar integration added to the Calang C++ application.

## Overview

The application now supports creating events in Google Calendar when users authenticate via Google OAuth. Events created through the terminal (`touch` command) or web interface are automatically synced to the user's Google Calendar.

## Components

### 1. Google Calendar API Function

**Function:** `createGoogleCalendarEvent()`

**Location:** `src/google_oauth.cpp`

**Signature:**
```cpp
std::string createGoogleCalendarEvent(
    const std::string &access_token,
    const std::string &title,
    const std::string &start_datetime,
    const std::string &end_datetime,
    const std::string &description
);
```

**Parameters:**
- `access_token`: OAuth 2.0 access token obtained during Google login
- `title`: Event summary/title
- `start_datetime`: Start date/time in format "YYYY-MM-DD HH:MM"
- `end_datetime`: End date/time in format "YYYY-MM-DD HH:MM"
- `description`: Event description

**Returns:**
- JSON response string from Google Calendar API (contains event ID on success, error on failure)

**API Details:**
- Endpoint: `https://www.googleapis.com/calendar/v3/calendars/primary/events`
- Method: POST
- Content-Type: application/json
- Authorization: Bearer token

**Request Body Format:**
```json
{
  "summary": "Event Title",
  "description": "Event Description",
  "start": {
    "dateTime": "2026-06-13T14:00:00Z",
    "timeZone": "UTC"
  },
  "end": {
    "dateTime": "2026-06-13T15:00:00Z",
    "timeZone": "UTC"
  }
}
```

### 2. Token Management

**Function:** `get_user_access_token()`

**Location:** `src/google_oauth.cpp`

**Purpose:** Retrieves the stored Google OAuth access token for a user

**Storage:** Tokens are stored in `users/{email}/google_token.json`

**Token Storage Format:**
```json
{
    "access_token": "ya29.a0...",
    "refresh_token": "1//0g...",
    "expires_in": 3600,
    "timestamp": 1718294400
}
```

### 3. OAuth Callback Enhancement

**Location:** `src/google_oauth.cpp` - `register_google_oauth_routes()`

**Enhancement:** The OAuth callback now stores the access token and refresh token when a user logs in via Google.

**Scope:** The OAuth scope includes `https://www.googleapis.com/auth/calendar` to enable calendar access.

### 4. Terminal Integration

**Location:** `src/terminal.cpp` - `process_terminal_command()`

**Enhancement:** The `touch` command now creates events in both:
1. Local Calang database (existing functionality)
2. Google Calendar (new functionality)

**Example Terminal Command:**
```bash
touch "Team Meeting" 13.06 14:00 15:00 "Discuss project updates"
```

**Output:**
```
-> Google Calendar: Event created successfully (ID: abc123xyz)
An event created!
-> Title: Team Meeting
-> Start: 2026-06-13 14:00
-> End:   2026-06-13 15:00
-> Desc:  Discuss project updates
-> Type:  private
-> Prior: medium
-> Recur: none
```

### 5. Web API Integration

**Location:** `src/api.cpp` - `/api/new_event`

**Enhancement:** Events created via the web interface are also synced to Google Calendar.

## How It Works

1. **User Authentication:**
   - User clicks "Login with Google"
   - User is redirected to Google OAuth consent screen
   - User grants calendar permissions
   - Access token is stored in `users/{email}/google_token.json`

2. **Event Creation:**
   - User creates an event (via terminal or web interface)
   - Application checks if user has a Google access token
   - If token exists, event is created in both local database and Google Calendar
   - User receives confirmation with Google Calendar event ID

3. **Date/Time Conversion:**
   - Calang uses format: "YYYY-MM-DD HH:MM"
   - Google Calendar API requires RFC3339: "YYYY-MM-DDTHH:MM:SSZ"
   - Automatic conversion happens in `createGoogleCalendarEvent()`

## Dependencies

- **libcurl**: For HTTP requests to Google Calendar API
- **nlohmann/json**: For JSON parsing and generation
- **Crow**: Web framework (existing)

## Environment Variables

Ensure these are set for Google OAuth to work:

```bash
export GOOGLE_CLIENT_ID="your_client_id.apps.googleusercontent.com"
export GOOGLE_CLIENT_SECRET="your_client_secret"
export GOOGLE_REDIRECT_URI="http://localhost:8080/auth/google/callback"
```

## OAuth Scope

The application requests the following scope:
```
https://www.googleapis.com/auth/calendar
```

This provides full access to the user's Google Calendar.

## Error Handling

- If access token is missing, event is created locally only (no error)
- If Google Calendar API returns an error, it's logged but doesn't prevent local event creation
- Network errors are logged via stderr

## Future Enhancements

Potential improvements:

1. **Token Refresh:** Implement automatic token refresh when access token expires
2. **Two-way Sync:** Import events from Google Calendar to Calang
3. **Calendar Selection:** Allow users to choose which Google Calendar to use
4. **Batch Operations:** Sync multiple events at once
5. **Event Updates:** Sync event modifications and deletions
6. **Recurring Events:** Better handling of recurring events in Google Calendar
7. **Timezone Support:** Proper timezone handling instead of defaulting to UTC

## Testing

To test the integration:

1. Set up Google OAuth credentials in Google Cloud Console
2. Enable Google Calendar API
3. Set environment variables
4. Run the application
5. Login via Google
6. Create an event using terminal or web interface
7. Check your Google Calendar to verify the event appears

## Code Structure

```
src/
├── google_oauth.h          # Function declarations
├── google_oauth.cpp        # OAuth + Calendar implementation
├── terminal.cpp            # Terminal command integration
├── api.cpp                 # Web API integration
└── functions.cpp           # Core event management
```

## API Reference

### Google Calendar API v3

**Documentation:** https://developers.google.com/calendar/api/v3/reference

**Create Event Endpoint:**
- URL: `POST https://www.googleapis.com/calendar/v3/calendars/{calendarId}/events`
- Calendar ID: `primary` (user's primary calendar)
- Auth: Bearer token in Authorization header

**Response Format (Success):**
```json
{
  "kind": "calendar#event",
  "id": "abc123xyz",
  "status": "confirmed",
  "htmlLink": "https://www.google.com/calendar/event?eid=...",
  "created": "2026-06-13T12:00:00.000Z",
  "updated": "2026-06-13T12:00:00.000Z",
  "summary": "Event Title",
  "description": "Event Description",
  "start": {
    "dateTime": "2026-06-13T14:00:00Z",
    "timeZone": "UTC"
  },
  "end": {
    "dateTime": "2026-06-13T15:00:00Z",
    "timeZone": "UTC"
  }
}
```

**Response Format (Error):**
```json
{
  "error": {
    "errors": [
      {
        "domain": "global",
        "reason": "authError",
        "message": "Invalid Credentials"
      }
    ],
    "code": 401,
    "message": "Invalid Credentials"
  }
}
```

## Security Considerations

- Access tokens are stored in plain text files (consider encryption for production)
- Tokens have limited lifetime (typically 1 hour)
- Refresh tokens can be used to obtain new access tokens
- Never commit token files to version control
- Add `users/*/google_token.json` to `.gitignore`

## Building

No changes to build process required. The integration uses existing dependencies.

```bash
cd build
cmake ..
make
./calang
```
