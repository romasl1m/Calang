# Google Calendar Sync - Architecture Diagram

## System Architecture

```
┌─────────────────────────────────────────────────────────────────┐
│                        User Interface                            │
├──────────────────────┬──────────────────────────────────────────┤
│   Terminal Interface │         Web Interface                    │
│   (terminal.cpp)     │         (HTML + JS)                      │
└──────────┬───────────┴──────────────┬───────────────────────────┘
           │                          │
           │ sync command             │ POST /api/sync_google
           │                          │
           ▼                          ▼
┌─────────────────────────────────────────────────────────────────┐
│                   Sync Controller Layer                          │
│                                                                   │
│  ┌────────────────────────────────────────────────────────┐     │
│  │  syncGoogleEvents(username)                            │     │
│  │  - Get access token                                    │     │
│  │  - Fetch from Google Calendar                          │     │
│  │  - Process and import events                           │     │
│  │  - Return import count                                 │     │
│  └────────────────────────────────────────────────────────┘     │
│                  (google_oauth.cpp)                              │
└─────────┬────────────────────────────────────┬───────────────────┘
          │                                    │
          │ Uses helper functions              │ Calls Google API
          ▼                                    ▼
┌──────────────────────────────┐   ┌─────────────────────────────┐
│   Helper Functions           │   │  Google Calendar API v3     │
│                              │   │                             │
│ • fetchGoogleCalendarEvents()│   │ GET /calendars/primary/     │
│   - Build API request        │◄──┤      events                 │
│   - Handle response          │   │                             │
│                              │   │ Auth: Bearer {token}        │
│ • convertRFC3339ToLocal()    │   │                             │
│   - Parse RFC3339            │   │ Response: JSON event list   │
│   - Convert to local format  │   └─────────────────────────────┘
│                              │
│ • eventExistsByGoogleId()    │
│   - Check duplicates         │
│   - Read from events.json    │
│                              │
└──────────────┬───────────────┘
               │
               │ Read/Write
               ▼
┌─────────────────────────────────────────────────────────────────┐
│                     Data Storage Layer                           │
│                                                                   │
│  users/{email}/                                                  │
│  ├── events.json           ← Events with google_id              │
│  └── google_token.json     ← Access token, user name            │
│                                                                   │
└─────────────────────────────────────────────────────────────────┘
```

## Component Interactions

### 1. User Authentication Flow
```
User
 │
 ├─→ Click "Login with Google"
 │
 ├─→ Google OAuth Page
 │    └─→ User authorizes
 │
 └─→ Callback: /auth/google/callback
      │
      ├─→ Exchange code for token
      ├─→ Get user info (name, email)
      └─→ Store in google_token.json
```

### 2. Sync Execution Flow
```
Terminal: sync               Web: POST /api/sync_google
    │                                    │
    └──────────┬─────────────────────────┘
               │
               ▼
    syncGoogleEvents(username)
               │
               ├─→ 1. get_user_access_token(username)
               │        └─→ Read google_token.json
               │
               ├─→ 2. fetchGoogleCalendarEvents(token)
               │        ├─→ Build request (timeMin, timeMax)
               │        ├─→ curl to Google API
               │        └─→ Return JSON response
               │
               ├─→ 3. Parse JSON response
               │        └─→ json::parse(response)
               │
               ├─→ 4. For each event:
               │        ├─→ Check eventExistsByGoogleId()
               │        │        └─→ Skip if exists
               │        ├─→ convertRFC3339ToLocal()
               │        │        └─→ Convert datetime
               │        ├─→ Create Event object
               │        └─→ Add to events array
               │
               └─→ 5. Save to events.json
                        └─→ Write updated JSON file
```

### 3. Data Transformation Flow
```
Google Calendar Event (RFC3339)
    │
    │  {
    │    "id": "abc123",
    │    "summary": "Meeting",
    │    "start": {"dateTime": "2024-06-14T10:00:00Z"},
    │    "end": {"dateTime": "2024-06-14T11:00:00Z"}
    │  }
    │
    ▼
convertRFC3339ToLocal()
    │
    │  "2024-06-14T10:00:00Z" → "2024-06-14 10:00"
    │
    ▼
Calang Event Object
    │
    │  {
    │    "id": "gcal_1718370000_0",
    │    "title": "Meeting",
    │    "start": "2024-06-14 10:00",
    │    "end": "2024-06-14 11:00",
    │    "google_id": "abc123",
    │    "user": "user@example.com",
    │    ...
    │  }
    │
    ▼
Stored in events.json
```

## Function Call Hierarchy

```
main()
 │
 └─→ Crow App
      │
      ├─→ API Routes (api.cpp)
      │    └─→ POST /api/sync_google
      │         └─→ syncGoogleEvents(user) ──┐
      │                                       │
      └─→ Terminal (terminal.cpp)            │
           └─→ process_terminal_command()    │
                └─→ "sync" command           │
                     └─→ syncGoogleEvents() ─┤
                                              │
                    ┌─────────────────────────┘
                    │
                    ▼
          syncGoogleEvents(username)
           │
           ├─→ get_user_access_token()
           │    └─→ Read google_token.json
           │
           ├─→ fetchGoogleCalendarEvents()
           │    ├─→ curl_easy_init()
           │    ├─→ curl_easy_setopt()
           │    └─→ curl_easy_perform()
           │
           ├─→ json::parse()
           │
           ├─→ For each event:
           │    ├─→ eventExistsByGoogleId()
           │    │    └─→ Read events.json
           │    │
           │    └─→ convertRFC3339ToLocal()
           │         ├─→ Parse timestamp
           │         └─→ Format local time
           │
           └─→ Write events.json
```

## Data Model

### Google Token Storage
```json
{
  "access_token": "ya29.a0...",
  "refresh_token": "1//0...",
  "expires_in": 3600,
  "timestamp": 1718370000,
  "user_name": "John Doe",
  "user_email": "john@example.com"
}
```

### Event with Google ID
```json
{
  "id": "gcal_1718370000_0",
  "title": "Team Meeting",
  "start": "2024-06-14 10:00",
  "end": "2024-06-14 11:00",
  "user": "john@example.com",
  "description": "Weekly sync",
  "origin": "private",
  "recurrence": "none",
  "recurrence_id": "",
  "priority": "medium",
  "subgroup": "",
  "google_id": "abc123xyz_20240614T100000Z"  ← Unique Google ID
}
```

## API Request/Response Flow

### Request to Google Calendar API
```
GET https://www.googleapis.com/calendar/v3/calendars/primary/events
    ?timeMin=2024-05-15T00:00:00Z
    &timeMax=2024-12-15T00:00:00Z
    &singleEvents=true
    &orderBy=startTime
    &maxResults=250

Headers:
  Authorization: Bearer ya29.a0...
```

### Response from Google
```json
{
  "kind": "calendar#events",
  "items": [
    {
      "id": "abc123xyz",
      "summary": "Team Meeting",
      "start": {
        "dateTime": "2024-06-14T10:00:00Z"
      },
      "end": {
        "dateTime": "2024-06-14T11:00:00Z"
      },
      "description": "Weekly team sync"
    }
  ]
}
```

### Response from Sync API
```json
{
  "success": true,
  "imported": 12,
  "message": "Successfully synced 12 events from Google Calendar"
}
```

## Error Handling Flow

```
syncGoogleEvents()
 │
 ├─→ No access token found
 │    └─→ Return -1 (Error)
 │         └─→ Display: "No Google access token found"
 │
 ├─→ API fetch fails
 │    └─→ curl error
 │         └─→ Return -1 (Error)
 │              └─→ Display: "Failed to fetch"
 │
 ├─→ JSON parse error
 │    └─→ catch(exception)
 │         └─→ Return -1 (Error)
 │              └─→ Display: "Failed to parse response"
 │
 └─→ Success
      └─→ Return count (0+)
           └─→ Display: "Successfully imported N events"
```

## Thread Safety

Currently single-threaded operation:
- Each request processed sequentially
- File I/O protected by Crow's request handling
- No concurrent sync operations

For multi-user concurrent access:
- Crow handles requests in separate threads
- Each user has separate files (users/{email}/)
- No shared state between users

## Performance Characteristics

- **Sync duration**: 1-3 seconds (network dependent)
- **Memory usage**: ~1-2 MB per sync operation
- **Disk I/O**: 2 reads + 1 write per sync
- **Network**: 1 API call to Google
- **Scaling**: Linear with event count (up to 250)

## Security Model

```
User Request
    │
    ├─→ Session Cookie Verification
    │    └─→ get_logged_in_user(cookie)
    │         └─→ Returns username or empty
    │
    ├─→ Access Token Retrieval
    │    └─→ Read from user's private directory
    │         └─→ users/{email}/google_token.json
    │
    └─→ Google API Call
         └─→ Bearer token authentication
              └─→ Scoped to user's calendar only
```

---

This architecture enables:
- ✅ Secure, per-user synchronization
- ✅ Efficient data import
- ✅ Duplicate prevention
- ✅ Clear error handling
- ✅ Extensible design
