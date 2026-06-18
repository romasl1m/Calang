# Group Management Features

This document describes all the group management features available in Calang.

## 🎯 Features Overview

### 1. **Group Settings**
- **Max Members Limit**: Set a maximum number of members (0 = unlimited)
- **Private Groups**: Make groups private with password protection
- **Edit Settings**: Only the group creator can modify settings

### 2. **Member Management**
- **View Members**: See all group members with creator badge
- **Kick Members**: Remove members from the group (creator only)
- **Member List**: Shows avatars and roles

### 3. **Join Request System**
- **Send Join Request**: Users can request to join private groups
- **Approve/Reject Requests**: Creators can manage join requests
- **Request Notifications**: Creators see pending requests in group info panel

### 4. **Invite System**
- **Send Invites**: Group members can invite others to join
- **Invite Notifications**: Users see pending invites with a badge in topbar
- **Accept/Reject Invites**: Users can accept or decline group invites
- **Invite Counter**: Visual indicator showing number of pending invites

### 5. **Delete Group**
- **Full Deletion**: Remove entire group including all events, subgroups, and members
- **Creator Only**: Only the group creator can delete the group
- **Confirmation Dialog**: Warns before permanent deletion

### 6. **Subgroup Management**
- **Nested Subgroups**: Support for hierarchical subgroup structure (e.g., team1/subteam1)
- **Add Subgroups**: Add new subgroups from the group info panel
- **Filter by Subgroup**: View events for specific subgroups only
- **Visual Tree**: Expandable tree view in sidebar

---

## 📋 API Endpoints

### Group Settings
- `POST /api/update_group_settings` - Update max members, privacy, and password
  - Parameters: `group_id`, `max_people`, `is_private`, `password`
  - Returns: `{success: true}` or error

### Member Management
- `POST /api/kick_member` - Remove a member from the group
  - Parameters: `group_id`, `username`
  - Returns: `{success: true}` or error

### Join Requests
- `POST /api/send_join_request` - Send a request to join a group
  - Parameters: `group_id`
  - Returns: `{success: true, message: "Join request sent"}`

- `POST /api/approve_join_request` - Approve a join request
  - Parameters: `group_id`, `username`
  - Returns: `{success: true}` or error

- `POST /api/reject_join_request` - Reject a join request
  - Parameters: `group_id`, `username`
  - Returns: `{success: true}` or error

### Invites
- `POST /api/send_invite` - Invite a user to join the group
  - Parameters: `group_id`, `invitee`
  - Returns: `{success: true, message: "Invite sent"}`

- `GET /api/my_invites` - Get all pending invites for current user
  - Returns: `[{group_id, group_name}, ...]`

- `POST /api/accept_invite` - Accept a group invite
  - Parameters: `group_id`
  - Returns: `{success: true}` or error

- `POST /api/reject_invite` - Reject a group invite
  - Parameters: `group_id`
  - Returns: `{success: true}` or error

### Group Info
- `GET /api/group_info?id=<group_id>` - Get detailed group information
  - Returns: Group details including settings, members, join requests, subgroups

### Delete Group
- `POST /api/delete_group` - Delete a group permanently
  - Parameters: `group_id`
  - Returns: `{success: true, message: "Group deleted successfully"}`

---

## 🎨 UI Components

### Group Info Panel
Located on the right side when right-clicking a group. Contains:

1. **Header**
   - Group name and ID
   - Close button

2. **Group ID Section**
   - Copy ID button for sharing

3. **Settings Section** (Creator only)
   - Max members display
   - Privacy status (Public/Private)
   - "Edit Settings" button

4. **Owner Section**
   - Shows group creator with badge

5. **Members Section**
   - List of all members with avatars
   - Creator badge for owner
   - Kick button for other members (creator only)

6. **Join Requests Section** (Creator only)
   - Shows pending join requests
   - Approve/Reject buttons for each request

7. **Invite Section** (Creator only)
   - Input field to enter username
   - "Invite" button to send invite

8. **Subgroups Section**
   - List of all subgroups with hierarchy
   - "Add Subgroup" input and button

9. **Delete Section** (Creator only)
   - Red "Delete Group" button

### Group Settings Modal
Appears when clicking "Edit Settings":
- Max members input field
- Private group checkbox
- Password field (shown when private is checked)
- Save/Cancel buttons

### Invites Notification
Located in top-right corner:
- Badge in topbar showing invite count
- Dropdown panel with pending invites
- Accept/Reject buttons for each invite
- Shows group name for each invite

---

## 🔒 Security & Permissions

### Group Creator Permissions
- Edit group settings (max members, privacy, password)
- Kick members
- Approve/reject join requests
- Delete group
- Send invites

### Group Member Permissions
- View group info
- View members
- Send invites
- Leave group (by being kicked or leaving)

### Non-Member Permissions
- Request to join public groups
- Request to join private groups (requires password)
- Accept invites

---

## 💡 Usage Examples

### Creating a Private Group
1. Click "+ New Group" in sidebar
2. Enter group name
3. Check "Private group" checkbox
4. Enter a password
5. Set max members if desired (optional)
6. Click "Create"

### Managing Join Requests
1. Right-click on your group
2. Scroll to "Join Requests" section
3. Click ✓ to approve or ✕ to reject

### Inviting Users
1. Right-click on your group
2. Scroll to "Invite User" section
3. Enter username
4. Click "Invite"

### Accepting Invites
1. Look for the "Invites" badge in topbar (appears when you have invites)
2. Click the badge to open invites panel
3. Click "Accept" on desired invite
4. Group will appear in your sidebar

### Kicking Members
1. Right-click on your group
2. Scroll to "Members" section
3. Click ✕ button next to member's name
4. Confirm the action

### Editing Group Settings
1. Right-click on your group
2. Click "Edit Settings" button
3. Modify max members or privacy settings
4. Enter new password if making group private
5. Click "Save"

---

## 🗄️ Data Storage

### Group Directory Structure
```
groups/
  <group_id>/
    info.json           # Group metadata (name, settings, subgroups)
    members.json        # Array of member usernames
    events.json         # Group events
    join_requests.json  # Array of usernames requesting to join
```

### User Directory Structure
```
users/
  <username>/
    events.json         # User's private events
    invites.json        # Array of pending group invites
```

### Group Info JSON Structure
```json
{
  "id": "1234567890",
  "name": "My Group",
  "max_people": 10,
  "is_private": true,
  "password": "secret",
  "subgroups": {
    "team1": {
      "subteam1": {}
    },
    "team2": {}
  }
}
```

### Invite JSON Structure
```json
[
  {
    "group_id": "1234567890",
    "inviter": "john_doe",
    "timestamp": 1718750000
  }
]
```

---

## 🔧 Implementation Details

### Backend Functions (functions.cpp)
- `kick_member()` - Remove member from group
- `update_group_settings()` - Update group configuration
- `send_join_request()` - Create join request
- `approve_join_request()` - Add user and remove request
- `reject_join_request()` - Remove request without adding user
- `get_join_requests()` - Fetch pending requests
- `send_invite()` - Create invite for user
- `get_user_invites()` - Fetch user's invites
- `accept_invite()` - Add user to group and remove invite
- `reject_invite()` - Remove invite without joining

### Frontend Functions (dashboard.html)
- `displayMembersList()` - Render members with kick buttons
- `displayJoinRequests()` - Render pending requests
- `kickMember()` - API call to remove member
- `approveJoinRequest()` / `rejectJoinRequest()` - Manage requests
- `sendInviteToUser()` - Send invite via API
- `loadUserInvites()` - Load and display invites badge
- `acceptInvite()` / `rejectInvite()` - Respond to invites
- `openGroupSettingsModal()` - Show settings editor
- `saveGroupSettings()` - Update group settings via API

---

## 🎯 Future Enhancements

Potential additions:
- Member roles (admin, moderator, member)
- Group categories/tags
- Group search functionality
- Member activity history
- Bulk invite system
- Email notifications for invites
- Group transfer ownership
- Member permissions system
- Group templates

---

## 📝 Notes

- All group operations require authentication
- Group creator is determined by first member in members.json
- Private groups can be joined with password OR via invite
- Kicked members can rejoin if invited again
- Deleting a group is permanent and cannot be undone
- Maximum members limit is enforced when joining
- Invites expire after 30 days (can be configured)
