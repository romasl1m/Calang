# Google Calendar Sync Implementation

## Overview
Added complete Google Calendar event import synchronization to calang. The implementation allows users to fetch events from their primary Google Calendar and import them into their local calang calendar.

## Features Implemented

### 1. Core Sync Function: `syncGoogleEvents(username)`
**Location**: `src/google_oauth.cpp`

Fetches events from Google Calendar (past 30 days to future 6 months) and imports them into local storage.

**Key features**:
- Fetches up to 250 events from primary calendar
- Time range: 30 days in the past to 6 months in the future
- Converts RFC3339 datetime format to calang's local format
- Stores `google_id` for each imported event
- Skips events that already exist (checks by `google_id`)
- Handles both timed events and all-day events
- Returns count of imported events

### 2. Supporting Functions

#### `fetchGoogleCalendarEvents(access_token)`
Makes API call to Google Calendar API v3 to retrieve events.

**Parameters**:
- Uses `singleEvents=true` to expand recurring events
- Orders by start time
- Filters by time range

#### `convertRFC3339ToLocal(rfc3339_time)`
Converts Google's RFC3339 datetime format to calang's format:
- From: `2024-06-14T10:00:00Z` or `2024-06-14T10:00:00+02:00`
- To: `2024-06-14 10:00`

Handles:
- Timezone removal
- Seconds removal
- Date-only events (adds `00:00`)

#### `eventExistsByGoogleId(google_id, events_file)`
Checks if an event with a given `google_id` already exists in the events file to prevent duplicates.

### 3. Enhanced User Authentication
**Modified**: Google OAuth callback in `src/google_oauth.cpp`

Now stores additional user information:
- `user_name` - User's full name from Google profile
- `user_email` - User's email address

Stored in `users/{email}/google_token.json` along with access tokens.

### 4. Terminal Command: `sync`
**Location**: `src/terminal.cpp`

Adds a new terminal command that users can run:
```bash
sync
```

**Behavior**:
- Calls `syncGoogleEvents(currentUsername)`
- Displays import count
- Triggers calendar reload via `RELOAD_CALENDAR`
- Shows error message if sync fails

**Added to help menu** with description: "sync - import events from Google Calendar"

### 5. API Endpoint: `/api/sync_google`
**Location**: `src/api.cpp`

POST endpoint for web interface synchronization.

**Request**: POST with authenticated session cookie

**Response**:
```json
{
  "success": true,
  "imported": 5,
  "message": "Successfully synced 5 events from Google Calendar"
}
```

Or on failure:
```json
{
  "success": false,
  "message": "Failed to sync Google Calendar events"
}
```

### 6. Event Schema Enhancement
**Already present in**: `src/event.h`

The Event struct already included `google_id` field:
```cpp
std::string google_id;
```

This field is:
- Serialized to/from JSON
- Stored in events.json
- Used for duplicate detection

## Usage

### Via Terminal
1. User logs in with Google OAuth
2. Opens terminal in calang
3. Runs: `sync`
4. Events are imported and calendar refreshes

### Via API (for web interface)
```javascript
fetch('/api/sync_google', {
  method: 'POST',
  credentials: 'include'
})
.then(response => response.json())
.then(data => {
  console.log(`Imported ${data.imported} events`);
  // Reload calendar view
});
```

## Data Flow

1. **Authentication Check**
   - Retrieves access token from `users/{email}/google_token.json`
   - Returns error if no token found

2. **Fetch from Google**
   - Calls Google Calendar API v3
   - Gets events from primary calendar
   - Filters by time range (past 30 days to future 6 months)

3. **Process Each Event**
   - Checks if `google_id` already exists in local storage
   - Skips if duplicate
   - Converts datetime formats
   - Extracts title, description, times
   - Handles all-day events

4. **Save to Local Storage**
   - Appends new events to `users/{email}/events.json`
   - Each event includes `google_id` field
   - Maintains all calang event properties

## File Structure

```
users/
  {email}/
    events.json          # Local events with google_id field
    google_token.json    # Access token, user name, email
```

## Event JSON Format

```json
{
  "id": "gcal_1718370000_0",
  "title": "Team Meeting",
  "start": "2024-06-14 10:00",
  "end": "2024-06-14 11:00",
  "user": "user@example.com",
  "description": "Weekly team sync",
  "origin": "private",
  "recurrence": "none",
  "recurrence_id": "",
  "priority": "medium",
  "subgroup": "",
  "google_id": "abc123xyz_20240614T100000Z"
}
```

## Error Handling

- **No access token**: Returns -1, displays error message
- **API fetch fails**: Returns -1, logs curl error
- **JSON parse error**: Returns -1, logs parse error
- **No items in response**: Returns 0 (no events to import)
- **File write error**: Returns -1, error message

## Testing Checklist

- [x] Build succeeds without errors
- [ ] User can log in with Google OAuth
- [ ] Access token is stored correctly
- [ ] Sync command runs without crashing
- [ ] Events are imported with google_id
- [ ] Duplicate events are skipped
- [ ] Calendar reloads after sync
- [ ] API endpoint returns correct JSON
- [ ] All-day events are handled correctly
- [ ] Datetime conversion works for different timezones

## Future Enhancements

1. **Bi-directional Sync**: Update Google Calendar when local events change
2. **Incremental Sync**: Use `updatedMin` parameter to fetch only changed events
3. **Recurring Events**: Better handling of recurring event series
4. **Calendar Selection**: Allow syncing from multiple calendars (not just primary)
5. **Conflict Resolution**: Handle when same event exists with different data
6. **Token Refresh**: Implement refresh token logic for expired access tokens
7. **Background Sync**: Automatic periodic synchronization
8. **Sync Status**: Show last sync time and status in UI

## Dependencies

- **libcurl**: HTTP requests to Google Calendar API
- **nlohmann/json**: JSON parsing and serialization
- **Crow**: Web framework for API endpoints
- Existing Google OAuth implementation

## API Documentation

### Google Calendar API v3
- **Endpoint**: `GET https://www.googleapis.com/calendar/v3/calendars/primary/events`
- **Parameters**:
  - `timeMin`: RFC3339 timestamp (start of range)
  - `timeMax`: RFC3339 timestamp (end of range)
  - `singleEvents`: `true` (expand recurring events)
  - `orderBy`: `startTime`
  - `maxResults`: `250`
- **Authentication**: Bearer token in Authorization header

## Files Modified

1. `src/google_oauth.h` - Added `syncGoogleEvents` declaration
2. `src/google_oauth.cpp` - Implemented sync functions
3. `src/terminal.cpp` - Added `sync` command
4. `src/api.cpp` - Added `/api/sync_google` endpoint

## Files Unchanged (Already Compatible)

1. `src/event.h` - Already has `google_id` field
2. `src/functions.cpp` - Works with google_id automatically

## Build Instructions

```bash
cd /home/roman/calang
cmake --build build
./build/calang
```

## Configuration

Requires environment variables (already set for OAuth):
- `GOOGLE_CLIENT_ID`
- `GOOGLE_CLIENT_SECRET`
- `GOOGLE_REDIRECT_URI`

OAuth scope must include:
- `https://www.googleapis.com/auth/calendar` (already configured)
