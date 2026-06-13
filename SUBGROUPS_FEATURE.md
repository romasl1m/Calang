# Subgroups Feature Documentation

## Overview
The subgroups feature allows you to organize events within a group into hierarchical categories. This is useful for teams with multiple sub-teams, projects with different phases, or any scenario where you want to organize group events into smaller, manageable categories.

## Key Features

### 1. **Hierarchical Structure**
- Subgroups can be nested using forward slashes (e.g., `team1/frontend`, `team1/backend`)
- Create multiple levels of hierarchy for complex organizational structures
- Visual indentation in the UI shows the hierarchy clearly

### 2. **Adding Subgroups**
You can add subgroups in two ways:

**Method 1: Through Group Info Panel**
1. Right-click on a group in the sidebar to open the Group Info panel
2. Scroll to the "Subgroups" section
3. Enter a subgroup name (e.g., `team1` or `team1/frontend`)
4. Click the "+" button

**Method 2: Programmatically through API**
```bash
POST /api/add_subgroup
group_id=<group_id>&name=<subgroup_path>
```

### 3. **Viewing Events by Subgroup**
There are multiple ways to filter events by subgroup:

**Option 1: Click the Eye Icon (👁)**
- Open the Group Info panel (right-click on a group)
- Find the subgroup you want to view
- Click the eye icon next to the subgroup name
- This will filter the calendar to show only events from that subgroup

**Option 2: Click on Subgroup Row**
- Clicking anywhere on a subgroup row (except the eye icon) will:
  - Open the "New Event" modal
  - Pre-select the group and subgroup
  - Let you quickly create an event in that subgroup

**Option 3: Select During Event Creation**
- When creating or editing an event
- Select a group in the "Add to" dropdown
- If the group has subgroups, a "Subgroup (optional)" dropdown will appear
- Select the desired subgroup from the list

### 4. **Subgroup Filter Badge**
When viewing events filtered by a subgroup:
- A badge appears in the toolbar showing: `📁 subgroup_name ✕`
- Click the badge to clear the filter and see all group events
- The badge helps you understand what you're currently viewing

### 5. **Subgroup in Event Display**
Events assigned to subgroups show the subgroup information:
- In the calendar grid (data attribute)
- In the event detail modal
- When editing events

## Use Cases

### 1. **Development Teams**
```
Engineering Group
├── frontend
│   ├── web
│   └── mobile
└── backend
    ├── api
    └── database
```

### 2. **Project Phases**
```
Project Alpha
├── planning
├── development
├── testing
└── deployment
```

### 3. **Department Subdivisions**
```
Marketing
├── social-media
├── content
└── analytics
```

## Technical Implementation

### Backend (C++)

**New Functions in `functions.cpp`:**
- `add_subgroup(group_id, subgroup_path)` - Adds a subgroup to a group
- `get_subgroups(group_id)` - Returns flattened list of all subgroups
- `flatten_subgroups(node, prefix, result)` - Helper to flatten nested structure

**Data Structure:**
Subgroups are stored in `groups/{group_id}/info.json` as a nested JSON object:
```json
{
  "name": "Engineering",
  "id": "1234567890",
  "subgroups": {
    "frontend": {
      "web": {},
      "mobile": {}
    },
    "backend": {
      "api": {},
      "database": {}
    }
  }
}
```

**Event Model:**
Each event has a `subgroup` field (string) storing the full path (e.g., `"frontend/web"`)

### Frontend (JavaScript)

**New Functions:**
- `displaySubgroupsInPanel(subgroups)` - Renders subgroups in the Group Info panel
- `addSubgroupToCurrentGroup()` - Adds a new subgroup via the UI
- `filterEventsBySubgroup(groupId, subgroup)` - Filters calendar by subgroup
- `clearSubgroupFilter()` - Removes subgroup filter
- `updateSubgroupFilterBadge()` - Updates the filter badge display
- `updateSubgroupOptions(selectId, groupId)` - Populates subgroup dropdowns
- `openNewModalForSubgroup(groupId, subgroup)` - Opens event modal with pre-selected subgroup

### API Endpoints

**Add Subgroup:**
```
POST /api/add_subgroup
Content-Type: application/x-www-form-urlencoded

group_id=<group_id>&name=<subgroup_path>
```

**Get Group Info (includes subgroups):**
```
GET /api/group_info?id=<group_id>
```

Response includes:
```json
{
  "id": "...",
  "name": "...",
  "subgroups": { ... },
  "members": [...],
  "member_count": 5,
  "event_count": 42,
  "creator": "username"
}
```

## Dashboard Filtering

The dashboard now supports URL parameter `?subgroup=<path>` to filter events:
- Example: `?view=1234567890&subgroup=frontend/web`
- This shows only events in the "frontend/web" subgroup of group 1234567890
- The filter respects the exact subgroup path (not recursive by default)

## UI/UX Considerations

1. **Visual Hierarchy**: Subgroups are displayed with indentation to show nesting levels
2. **Quick Actions**: Multiple ways to interact with subgroups (view, create event)
3. **Clear Feedback**: Badge shows current filter and allows easy clearing
4. **Optional Assignment**: Events can optionally have no subgroup (None)
5. **Mobile Friendly**: All subgroup features work on mobile devices

## Future Enhancements

Potential improvements:
- Recursive subgroup filtering (show all events in child subgroups)
- Subgroup permissions/access control
- Subgroup colors/icons for visual distinction
- Drag-and-drop to move events between subgroups
- Subgroup statistics and analytics
- Bulk operations on subgroup events
- Subgroup templates for common structures
