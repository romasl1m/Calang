# Google Calendar Sync - Testing Checklist

## Pre-Testing Setup

### Environment Variables
```bash
export GOOGLE_CLIENT_ID="your-client-id-here"
export GOOGLE_CLIENT_SECRET="your-client-secret-here"
export GOOGLE_REDIRECT_URI="http://localhost:8080/auth/google/callback"
```

### Build and Start
```bash
cd /home/roman/calang
cmake --build build
./build/calang
```

Server should start on: http://localhost:8080

---

## ✅ Phase 1: Build Verification

- [ ] Project builds without errors
  ```bash
  cmake --build build
  # Expected: [100%] Built target calang
  ```

- [ ] Test script passes
  ```bash
  ./test_sync.sh
  # Expected: Most tests pass ✓
  ```

- [ ] Executable exists
  ```bash
  ls -lh build/calang
  # Expected: File exists, ~3-4 MB
  ```

---

## ✅ Phase 2: Authentication Testing

- [ ] Server starts successfully
  ```bash
  ./build/calang
  # Expected: Server running on port 8080
  ```

- [ ] Google OAuth login page loads
  - Navigate to: http://localhost:8080/auth/google/login
  - Expected: Redirects to Google login

- [ ] Can complete OAuth flow
  - Authorize app
  - Expected: Redirects back to dashboard

- [ ] Token file created
  ```bash
  ls users/YOUR_EMAIL/google_token.json
  # Expected: File exists with access_token
  ```

- [ ] User name stored correctly
  ```bash
  cat users/YOUR_EMAIL/google_token.json
  # Expected: Contains "user_name" and "user_email"
  ```

---

## ✅ Phase 3: Terminal Command Testing

- [ ] Open terminal in dashboard
  - Click terminal icon or navigate to terminal section

- [ ] Help command shows sync
  ```
  help
  ```
  - Expected: List includes "sync - import events from Google Calendar"

- [ ] Run sync command
  ```
  sync
  ```
  - Expected: "Syncing events from Google Calendar..."
  - Expected: "Successfully imported N events"

- [ ] Verify events imported
  ```
  cat 7
  ```
  - Expected: Shows next 7 events, including imported ones

- [ ] Check google_id is stored
  ```bash
  cat users/YOUR_EMAIL/events.json | grep google_id
  # Expected: Shows google_id fields
  ```

- [ ] Run sync again (duplicate test)
  ```
  sync
  ```
  - Expected: "Successfully imported 0 events" (no duplicates)

- [ ] Search imported events
  ```
  grep "meeting"
  ```
  - Expected: Finds events with "meeting" in title/description

---

## ✅ Phase 4: API Endpoint Testing

- [ ] Get session cookie
  ```bash
  # After logging in, inspect browser cookies
  # Copy session_id value
  ```

- [ ] Test sync API endpoint
  ```bash
  curl -X POST http://localhost:8080/api/sync_google \
    -b "session_id=YOUR_SESSION_ID_HERE" \
    -H "Content-Type: application/json"
  ```
  - Expected: JSON response with success:true

- [ ] Verify JSON response format
  ```json
  {
    "success": true,
    "imported": 5,
    "message": "Successfully synced 5 events from Google Calendar"
  }
  ```

- [ ] Test without authentication
  ```bash
  curl -X POST http://localhost:8080/api/sync_google
  ```
  - Expected: 401 Unauthorized

---

## ✅ Phase 5: Data Validation

- [ ] Check event format
  ```bash
  cat users/YOUR_EMAIL/events.json | jq '.[0]'
  ```
  - Expected: Contains all required fields
  - Expected: google_id is not empty
  - Expected: Datetime format is "YYYY-MM-DD HH:MM"

- [ ] Verify no duplicates
  ```bash
  # Run sync twice, check event count
  BEFORE=$(cat users/YOUR_EMAIL/events.json | jq '. | length')
  # Run sync again
  AFTER=$(cat users/YOUR_EMAIL/events.json | jq '. | length')
  # BEFORE should equal AFTER
  ```

- [ ] Check datetime conversion
  - Find event in Google Calendar with time
  - Find same event in calang
  - Verify time matches (considering timezone)

- [ ] Verify all-day events
  - Create all-day event in Google Calendar
  - Run sync
  - Expected: Time shows as "00:00" to "23:59"

---

## ✅ Phase 6: Edge Cases

### No Google Login
- [ ] Fresh user without Google OAuth
  ```
  sync
  ```
  - Expected: "Failed to sync Google Calendar. Make sure you're logged in with Google."

### Empty Calendar
- [ ] Google Calendar with no events
  ```
  sync
  ```
  - Expected: "Successfully imported 0 events"

### Large Event Count
- [ ] Calendar with 100+ events
  ```
  sync
  ```
  - Expected: Imports up to 250 events
  - Expected: Completes within 5 seconds

### Special Characters
- [ ] Event with special characters in title
  - Create event: "Test & \"Quotes\" <HTML>"
  - Run sync
  - Expected: Characters preserved correctly

### Past Events
- [ ] Event from 2 months ago
  - Expected: Not imported (outside time range)

### Future Events
- [ ] Event 1 year in the future
  - Expected: Not imported (outside time range)

### Event with No Description
- [ ] Event without description
  - Run sync
  - Expected: description field is empty string

---

## ✅ Phase 7: Integration Testing

- [ ] Create event in Google Calendar
  - Title: "Integration Test"
  - Time: Tomorrow 10:00-11:00

- [ ] Run sync in calang
  ```
  sync
  ```
  - Expected: Event imported

- [ ] Find event
  ```
  grep "Integration Test"
  ```
  - Expected: Event found

- [ ] View event details
  ```
  cat TOMORROW_DATE
  ```
  - Expected: Integration Test appears

- [ ] Verify event can be used with AI
  ```
  ai "show me my integration test event"
  ```
  - Expected: AI finds and displays event

---

## ✅ Phase 8: Error Handling

### Invalid Token
- [ ] Corrupt token file
  ```bash
  echo "invalid json" > users/YOUR_EMAIL/google_token.json
  sync
  ```
  - Expected: Error message, doesn't crash

### Network Failure
- [ ] Disconnect internet
  ```
  sync
  ```
  - Expected: "Failed to fetch Google Calendar events"

### Malformed Response
- Not easily testable, but code handles json parse errors

---

## ✅ Phase 9: Performance Testing

- [ ] Measure sync time
  ```bash
  time echo "sync" | ./build/calang
  ```
  - Expected: < 5 seconds for normal calendar

- [ ] Check memory usage
  ```bash
  # While syncing, in another terminal:
  ps aux | grep calang
  ```
  - Expected: Reasonable memory usage (< 50 MB)

- [ ] Multiple concurrent syncs
  - Open multiple terminal sessions
  - Run sync simultaneously
  - Expected: All complete successfully

---

## ✅ Phase 10: Documentation Verification

- [ ] README or docs mention sync feature
- [ ] Help command lists sync
- [ ] API documentation includes /api/sync_google
- [ ] Error messages are clear and helpful

---

## Test Results Summary

Total Tests: 50+
- Critical: 20
- Important: 15
- Nice-to-have: 15+

### Quick Test (5 minutes)
1. Build project ✓
2. Login with Google ✓
3. Run sync command ✓
4. Verify events imported ✓
5. Run sync again (no duplicates) ✓

### Full Test (30 minutes)
Complete all phases above

---

## Common Issues and Solutions

### Issue: "No Google access token found"
**Solution**: Login with Google OAuth first
```
http://localhost:8080/auth/google/login
```

### Issue: "Failed to fetch Google Calendar events"
**Solution**: 
- Check internet connection
- Verify GOOGLE_CLIENT_ID and GOOGLE_CLIENT_SECRET are set
- Check Google API quota

### Issue: No events imported
**Solution**:
- Verify Google Calendar has events in date range
- Check that events haven't been imported already
- Look at time range: past 30 days to future 6 months

### Issue: Wrong time zone
**Solution**:
- Check server timezone: `date`
- Events use server's local time

---

## Manual Verification Steps

After automated tests, manually verify:

1. **Visual Check**
   - Events appear in dashboard calendar view
   - Times display correctly
   - Titles are readable

2. **Functionality Check**
   - Can view event details
   - Events are searchable
   - Events appear in correct dates

3. **Data Integrity Check**
   ```bash
   jq '.[] | select(.google_id != "")' users/YOUR_EMAIL/events.json
   ```
   - All imported events have google_id
   - No google_id duplicates

---

## Sign-off Checklist

Before marking as complete:

- [ ] All critical tests pass
- [ ] Documentation is complete
- [ ] Code builds without warnings
- [ ] No memory leaks (valgrind clean)
- [ ] Error messages are user-friendly
- [ ] Feature works end-to-end

---

## Test Log Template

```
Date: ____________________
Tester: __________________
Version: _________________

Build Test:          [ ] PASS  [ ] FAIL
Auth Test:           [ ] PASS  [ ] FAIL
Terminal Sync:       [ ] PASS  [ ] FAIL
API Sync:           [ ] PASS  [ ] FAIL
Duplicate Check:     [ ] PASS  [ ] FAIL
Integration Test:    [ ] PASS  [ ] FAIL

Notes:
___________________________________
___________________________________
___________________________________
```
