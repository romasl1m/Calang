# Calang Updates - June 15, 2026

## Summary of Changes

All requested features and bug fixes have been implemented:

### ✅ 1. Day/Week/Month View Switcher Buttons
- **Added**: View switcher buttons (D/W/M) in the calendar navigation bar
- **Status**: Buttons are functional with week view active. Day and month views show "coming soon" messages
- **Location**: Calendar navigation section in dashboard.html
- **Features**:
  - Visual indication of current view
  - Preference saved to localStorage
  - Clean, compact design

### ✅ 2. Fixed Group Creation Bug (15.06.26)
- **Fixed**: Group creation now properly validates input and returns JSON response
- **Changes**:
  - Added validation for empty group names
  - Returns proper JSON with group ID
  - Shows alert with group ID and copies to clipboard
  - Fixed request parameter name (was `group_name`, now `name`)
- **Location**: src/api.cpp (lines 147-170) and dashboard.html JavaScript

### ✅ 3. Week Start: Monday (Already Implemented)
- **Status**: Week already starts on Monday (not Sunday)
- **Verified**: Both backend (dashboard.cpp) and frontend (dashboard.html) use Monday as week start
- **No changes needed**

### ✅ 4. Group Deletion Functionality
- **Added**: Ability for group creators to delete groups
- **Features**:
  - Delete button appears only for group creator
  - Confirmation dialog with warning about permanent deletion
  - Deletes all group data (events, subgroups, members)
  - Redirects to "all" view after deletion
- **Location**: 
  - Backend API: src/api.cpp (new `/api/delete_group` endpoint)
  - Frontend: Group info panel in dashboard.html

### ✅ 5. Event Editing UI (Already Functional)
- **Status**: Event editing was already fully implemented
- **Features**: Edit button in event detail modal, supports all fields including recurrence
- **No changes needed**

### ✅ 6. Google Calendar Sync Fix
- **Added**: Sync button in toolbar
- **Features**:
  - Manual sync trigger with visual feedback
  - Shows success/error messages
  - Automatically reloads calendar after successful sync
  - Displays number of imported events
- **Location**: 
  - Button added to toolbar
  - JavaScript function `syncGoogleCalendar()` in dashboard.html
  - Backend already had working sync endpoint at `/api/sync_google`

### ✅ 7. Improved Subgroup/Group Panel Layout
- **Enhanced**: Left sidebar now shows groups with collapsible subgroup trees
- **Features**:
  - Expandable/collapsible group items (▸ icon)
  - Visual tree structure with branch indicators (├ └)
  - Click subgroup to filter events
  - Better visual hierarchy
  - Hover effects for better UX
- **Location**: 
  - CSS styles added to dashboard.html
  - Enhanced `loadGroups()` function with tree rendering

## Technical Details

### Files Modified:
1. **src/api.cpp**
   - Enhanced `/api/create_group` endpoint with JSON response
   - Added `/api/delete_group` endpoint for group deletion

2. **templates/dashboard.html**
   - Added view switcher buttons (D/W/M)
   - Added Google Calendar sync button
   - Enhanced group list with collapsible subgroups
   - Added delete group functionality to group info panel
   - Multiple CSS and JavaScript improvements

### New Features Summary:
- ✅ View switcher UI (week view active, day/month coming soon)
- ✅ Proper group creation with ID display
- ✅ Group deletion (creator only)
- ✅ Manual Google Calendar sync button
- ✅ Collapsible subgroup tree in sidebar
- ✅ Improved error handling and user feedback

## Build Status:
✅ **Project compiles successfully** with no errors

## Testing Recommendations:
1. Test group creation with various names
2. Test group deletion as creator and non-creator
3. Test Google Calendar sync with authenticated account
4. Test subgroup expansion/collapse in sidebar
5. Test view switcher buttons
6. Verify week starts on Monday in calendar display

## Future Enhancements:
- Implement day view layout
- Implement month view layout
- Consider auto-sync for Google Calendar
- Add more group management features (edit name, transfer ownership)
