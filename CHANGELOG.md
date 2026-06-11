# Calang Updates - June 2026

## Summary
Fixed AI integration issues, improved terminal functionality, and added mobile support.

## Changes Made

### 1. ✅ Fixed AI Event Creation
**Issue**: Events created by AI weren't appearing in calendar until manual refresh.

**Solution**: 
- Modified AI chat handler to detect event creation commands
- Added automatic calendar reload when AI executes `touch` commands
- Calendar now refreshes after AI creates/modifies events

**Files Changed**: `templates/dashboard.html` (lines ~2876-2890)

---

### 2. ✅ Enhanced Find/Search Functionality
**Issue**: Find event functionality wasn't well documented.

**Solution**:
- Updated help command to clearly show `grep` for searching
- Added examples in AI prompts
- Improved `grep` output formatting

**Files Changed**: `src/terminal.cpp` (help section)

---

### 3. ✅ Replaced AI_DATE with Vector Storage
**Issue**: Single AI_DATE variable could only store one date at a time.

**Solution**:
- Replaced `static string AI_DATE` with `static vector<string> DATE_VECTOR`
- Now stores multiple dates from `cat`, `grep`, and other searches
- Added syntax `$DATE[n]` to access specific dates (e.g., `$DATE[0]`, `$DATE[1]`)
- `$DATE` without brackets uses first date
- Added `dates` command to view all stored dates

**Example Usage**:
```bash
grep "meeting"           # Stores all meeting dates
dates                    # Shows: [0] 2026-06-15, [1] 2026-06-20
touch "Review" $DATE[1] 14:00 15:00 "Review the second meeting date"
```

**Files Changed**: 
- `src/terminal.cpp` (global variable, command processing, cat/grep/dates commands)
- `src/api.cpp` (updated AI prompts)

---

### 4. ✅ Fixed AI Command Execution
**Issue**: AI couldn't execute terminal commands properly.

**Solution**:
- Fixed user and group context extraction from cookies in `/api/ai_chat`
- AI now properly retrieves logged-in user from session
- Commands execute with correct permissions and context

**Files Changed**: `src/api.cpp` (lines ~425-436)

---

### 5. ✅ Improved UI & Added Mobile Support
**Issue**: UI wasn't optimized for mobile devices.

**Solution**:

#### Mobile Responsive Design:
- Added responsive breakpoints for phones (≤768px) and tablets (769-1024px)
- Sidebar becomes sliding drawer on mobile
- Added floating action button (FAB) for menu toggle
- Mobile overlay for sidebar backdrop
- Improved touch targets (minimum 44px)
- Prevented zoom on input focus (iOS)

#### UI Improvements:
- Better spacing and font sizes on small screens
- Terminal takes 60% of viewport height on mobile
- Modals are 95% width on mobile
- Calendar events have larger touch areas
- Week badge moved to top on mobile
- Improved form control sizes

#### Mobile Gestures:
- Tap FAB to open/close sidebar
- Tap overlay to close sidebar
- Touch-optimized event cards
- Better scrolling experience

**Files Changed**: `templates/dashboard.html`
- Added mobile CSS media queries
- Added mobile menu button and overlay HTML
- Added JavaScript for sidebar toggle functions
- Added mobile meta tags for PWA-like experience

---

## Testing Recommendations

### Terminal Commands:
1. Test `dates` command to view stored dates
2. Test `grep "keyword"` and verify dates are stored
3. Test `cat 5` and check date vector population
4. Test `$DATE` and `$DATE[0]`, `$DATE[1]` in commands

### AI Chat:
1. Ask AI to "show my next 3 events"
2. Ask AI to "find meetings"
3. Ask AI to "create an event tomorrow at 2pm"
4. Verify calendar refreshes automatically

### Mobile:
1. Open on phone/tablet
2. Test sidebar toggle with FAB
3. Test calendar scrolling
4. Test event creation form
5. Test terminal on mobile

---

## API Endpoints Modified
- `/api/ai_chat` - Fixed user context, improved command execution
- `/api/terminal` - Enhanced with vector date storage

## Breaking Changes
None - all changes are backward compatible. Old `$DATE` syntax still works (uses first date in vector).

## Future Improvements
- Add swipe gestures for mobile navigation
- Add dark/light theme toggle
- Add calendar export functionality
- Add push notifications for mobile
- Add offline support (PWA)
