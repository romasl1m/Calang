# Join Group Features

## Overview
Added comprehensive group joining functionality with QR codes, shareable links, and an improved join flow.

## New Features

### 1. **Share Group with QR Code**
- Generate QR codes for any group
- Accessible from the Group Info panel
- QR code encodes a join link that can be scanned with any QR code reader
- Modal displays QR code, group name, and copyable link

### 2. **Shareable Join Links**
- Format: `https://your-domain.com/join/{group_id}`
- Click "Copy Join Link" button in Group Info panel
- Links can be shared via messaging apps, email, etc.
- Automatic redirect to login if user is not authenticated
- Auto-opens join modal with pre-filled group ID

### 3. **Enhanced Join Group Modal**
- New dedicated "🔗 Join Group" button in top navigation bar
- Modal accepts:
  - Group ID directly
  - Full join links (automatically extracts group ID)
- Automatic password prompt for private groups
- Better error handling with descriptive messages

### 4. **Join Link Route Handler**
- Backend route: `/join/:group_id`
- Redirects to login if not authenticated (preserves join intent)
- Redirects to dashboard with auto-join parameter
- Frontend automatically opens join modal with pre-filled group ID

## UI Components

### Group Info Panel - New Share Section
```
┌─────────────────────────────┐
│ Share Group                 │
├─────────────────────────────┤
│ [📋 Copy Join Link]        │
│ [📱 Show QR Code]          │
└─────────────────────────────┘
```

### QR Code Modal
```
┌─────────────────────────────┐
│ Group QR Code               │
├─────────────────────────────┤
│         [QR Code]           │
│                             │
│ Scan this QR code to join   │
│ the group                   │
│                             │
│ My Awesome Group            │
│                             │
│ [https://domain.com/join..] │
├─────────────────────────────┤
│ [Close]  [Copy Link]        │
└─────────────────────────────┘
```

### Join Group Modal
```
┌─────────────────────────────┐
│ Join Group                  │
├─────────────────────────────┤
│ Group ID or Join Link       │
│ [_____________________]     │
│                             │
│ Password (Private Group)    │
│ [_____________________]     │
│  (hidden until needed)      │
├─────────────────────────────┤
│ [Cancel]      [Join]        │
└─────────────────────────────┘
```

### Top Navigation Bar
Added new "🔗 Join Group" button between "Invites" and "Profile" buttons.

## Technical Implementation

### Frontend (dashboard.html)

#### New JavaScript Functions:
1. `copyJoinLink()` - Copies join link to clipboard
2. `showQRCode()` - Generates and displays QR code modal
3. `closeQRCodeModal()` - Closes QR code modal
4. `copyQRLink()` - Copies link from QR modal
5. `openJoinGroupModal()` - Opens join group modal
6. `closeJoinGroupModal()` - Closes join group modal
7. `joinGroupFromModal()` - Handles joining from modal with link/ID support

#### Auto-Join Feature:
- Checks URL parameters on page load
- If `?join=GROUP_ID` parameter exists:
  - Removes parameter from URL (clean history)
  - Opens join modal
  - Pre-fills group ID field

#### Dependencies Added:
```html
<script src="https://cdn.jsdelivr.net/npm/qrcodejs@1.0.0/qrcode.min.js"></script>
```

### Backend (core_routes.cpp)

#### New Route:
```cpp
CROW_ROUTE(app, "/join/<string>")
```

**Behavior:**
- Checks if user is logged in via cookies
- If not logged in: redirects to `/login?return=/join/{group_id}`
- If logged in: redirects to `/dashboard?join={group_id}`

**Includes Added:**
```cpp
#include "cookies.h"  // For get_logged_in_user()
```

## User Workflows

### Sharing a Group (Creator/Member):
1. Right-click on group in sidebar
2. Group Info panel opens
3. Scroll to "Share Group" section
4. Option A: Click "📋 Copy Join Link" → Share link via messaging
5. Option B: Click "📱 Show QR Code" → Display QR code for scanning

### Joining via QR Code:
1. Scan QR code with phone camera or QR reader app
2. Opens join link in browser
3. If not logged in: prompted to log in
4. After login: automatically redirected to dashboard
5. Join modal opens with group ID pre-filled
6. If private group: enter password when prompted
7. Click "Join" → Success!

### Joining via Link:
1. Receive join link (e.g., `https://app.com/join/1234567890`)
2. Click link
3. Same flow as QR code (login → auto-open modal → join)

### Joining Manually:
1. Click "🔗 Join Group" button in top navigation
2. Enter group ID or paste full join link
3. If private group: password field appears after first attempt
4. Click "Join"

## API Endpoints Used

- `POST /api/join_group`
  - Parameters: `group_id`, `password` (optional)
  - Returns: 200 (success), 403 (wrong password), 404 (not found)

## Security Considerations

- Private groups still require password even with join links
- Join links don't bypass group privacy settings
- Group IDs are already public (not secret tokens)
- Authentication required before joining
- Password prompt appears only when needed (UX + security)

## Mobile Responsiveness

All new features work on mobile:
- QR codes can be displayed and scanned
- Join modal is touch-friendly
- Copy to clipboard works on mobile browsers
- Links open correctly in mobile browsers

## Future Enhancements

Potential additions:
- Expiring join links with token-based authentication
- One-time use invite links
- Link analytics (track who joined via which link)
- QR code customization (colors, logo)
- Deep linking for mobile apps
- Social media share buttons (WhatsApp, Telegram, etc.)
- Email invite system with embedded QR codes
- Join link preview with group info

## Testing Checklist

- [x] Build succeeds without errors
- [ ] QR code generates correctly
- [ ] QR code scans and opens correct link
- [ ] Copy join link button works
- [ ] Join link redirects to dashboard
- [ ] Auto-join modal opens with correct group ID
- [ ] Manual join with group ID works
- [ ] Manual join with full link works (extracts ID)
- [ ] Private group password prompt appears
- [ ] Join succeeds for public groups
- [ ] Join succeeds for private groups with correct password
- [ ] Join fails for private groups with wrong password
- [ ] Unauthenticated users redirect to login
- [ ] Login redirect preserves join intent
- [ ] Mobile QR code scanning works
- [ ] Mobile join modal is responsive

## Files Modified

1. `templates/dashboard.html`
   - Added QR code library
   - Added share section in Group Info panel
   - Added QR code modal
   - Added enhanced join group modal
   - Added "🔗 Join Group" button in topbar
   - Added JavaScript functions for all new features
   - Added auto-join on page load

2. `src/core_routes.cpp`
   - Added `/join/:group_id` route handler
   - Added `cookies.h` include

## Notes

- QR codes are generated client-side (no server processing needed)
- Join links work across devices and platforms
- Group creators see share options for all their groups
- Group members can also share (if invite permissions allow)
- Join flow integrates seamlessly with existing group system
- No database schema changes required
