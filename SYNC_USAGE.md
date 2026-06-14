# Google Calendar Sync - Usage Guide

## Quick Start

### 1. Login with Google
Visit your calang instance and click "Login with Google" to authenticate.

### 2. Sync Your Events

#### Using Terminal
```bash
# Open terminal in calang
sync
```

Output:
```
Syncing events from Google Calendar...
Successfully imported 12 events from Google Calendar
```

#### Using API (JavaScript)
```javascript
fetch('/api/sync_google', {
  method: 'POST',
  credentials: 'include'
})
.then(res => res.json())
.then(data => {
  console.log(data.message);
  // Reload your calendar view
  location.reload();
});
```

## What Gets Synced?

- **Time Range**: Past 30 days to 6 months in the future
- **Calendar**: Primary calendar only
- **Event Limit**: Up to 250 events per sync
- **Event Types**: Both timed events and all-day events
- **Duplicate Prevention**: Events with the same `google_id` are skipped

## Event Details Imported

Each imported event includes:
- Title
- Description
- Start time
- End time
- Google Calendar ID (for duplicate detection)

## Where Events Are Stored

Imported events appear in your **private calendar** (`origin: "private"`).

## Re-syncing

Running `sync` multiple times is safe:
- Existing events are not duplicated
- Only new events are imported
- Same Google Calendar events are recognized by their `google_id`

## Sync Frequency

Currently manual only. Run `sync` command whenever you want to update:
- After adding events in Google Calendar
- When starting a new session
- Periodically to stay up-to-date

## Troubleshooting

### "Failed to sync Google Calendar"
**Cause**: No Google access token found

**Solution**:
1. Make sure you're logged in via Google OAuth
2. Check that `users/{your-email}/google_token.json` exists
3. Re-authenticate if token expired

### No events imported (imported: 0)
**Possible reasons**:
- All events already synced
- No events in selected time range
- Calendar is empty

### Events appear with wrong timezone
The system converts to local time automatically. If times are incorrect:
- Check your server's timezone settings
- Verify Google Calendar timezone matches

## API Response Examples

### Success
```json
{
  "success": true,
  "imported": 5,
  "message": "Successfully synced 5 events from Google Calendar"
}
```

### No new events
```json
{
  "success": true,
  "imported": 0,
  "message": "Successfully synced 0 events from Google Calendar"
}
```

### Error
```json
{
  "success": false,
  "message": "Failed to sync Google Calendar events"
}
```

## Example Workflow

1. **Morning routine**:
   ```bash
   # Check calendar
   cat 7  # Next 7 events
   
   # Sync with Google
   sync
   
   # View updated calendar
   cat 7
   ```

2. **After team meeting** (where events were created in Google Calendar):
   ```bash
   sync
   grep "project"  # Find project-related events
   ```

3. **Planning your week**:
   ```bash
   sync
   cat 2024-06-17  # Monday
   cat 2024-06-18  # Tuesday
   # ... etc
   ```

## Advanced: Checking Sync Status

View your Google token info:
```bash
cat users/{your-email}/google_token.json
```

Check if events have Google IDs:
```bash
# In your events.json, look for:
"google_id": "abc123xyz_20240614T100000Z"
```

## Integration with Calang Features

Synced events work with all calang features:
- **Search**: `grep "meeting"`
- **View by date**: `cat 2024-06-14`
- **Terminal navigation**: `cd`, `ls`
- **AI commands**: `ai "show me events next week"`

## Notes

- Synced events are **one-way** (Google → Calang)
- Changes in calang won't sync back to Google Calendar
- Deleting synced events in calang won't delete them in Google
- Re-syncing will re-import deleted events

## Future Features

Coming soon:
- Two-way sync
- Automatic background sync
- Sync multiple calendars
- Selective calendar sync
- Conflict resolution
