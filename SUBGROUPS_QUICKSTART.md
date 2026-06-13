# Subgroups Quick Start Guide

## What are Subgroups?
Subgroups let you organize events within a group into categories. Think of them like folders within a folder!

## Quick Setup (3 steps)

### Step 1: Open Group Info
Right-click on any group in the left sidebar → Group Info panel opens

### Step 2: Add a Subgroup
1. Scroll to "Subgroups" section
2. Enter a name in the input field
   - Simple: `team1`
   - Nested: `team1/frontend`
3. Click the "+" button
4. Done! ✓

### Step 3: Use Your Subgroup

**Option A: Filter events by subgroup**
- In the Group Info panel, find your subgroup
- Click the eye icon (👁) next to it
- Calendar now shows ONLY events from that subgroup
- See badge at top: `📁 subgroup_name ✕`
- Click badge to clear filter

**Option B: Create event in subgroup**
- Click on the subgroup row in Group Info panel
- Event creation modal opens with group & subgroup pre-selected
- Add your event details
- Save!

**Option C: Manual selection**
- Create new event normally
- Select the group from "Add to" dropdown
- "Subgroup (optional)" dropdown appears
- Select your subgroup
- Save!

## Example: Development Team

Let's organize a software team:

1. **Create subgroups:**
   ```
   frontend
   frontend/web
   frontend/mobile
   backend
   backend/api
   backend/database
   ```

2. **Add events:**
   - "Sprint Planning" → `frontend` subgroup
   - "Code Review" → `frontend/web` subgroup
   - "API Meeting" → `backend/api` subgroup
   - "Database Migration" → `backend/database` subgroup

3. **View filtered:**
   - Click eye icon on "frontend" → see all frontend events
   - Click eye icon on "frontend/web" → see only web-specific events
   - Clear filter to see all events again

## Tips & Tricks

✨ **Nested Hierarchy**
Use `/` to create nested structure: `parent/child/grandchild`

✨ **Quick Create**
Click on a subgroup row to instantly create an event in that subgroup

✨ **Visual Organization**
Subgroups are indented to show hierarchy clearly

✨ **Optional**
You can leave events without a subgroup - just select "None"

✨ **Clear View**
The badge at the top always shows what you're viewing

## Common Patterns

### Pattern 1: Project Phases
```
project-alpha
├── planning
├── development
├── testing
└── deployment
```

### Pattern 2: Departments
```
marketing
├── content
├── social
└── analytics
```

### Pattern 3: Locations
```
company-events
├── office-ny
├── office-sf
└── remote
```

## Keyboard Shortcuts

When Group Info panel is open:
- `ESC` - Close the panel
- Type in input and press `Enter` - Add subgroup

## Troubleshooting

**Q: I don't see the "Subgroup" dropdown when creating an event?**
A: The group you selected might not have any subgroups yet. Add some first!

**Q: How do I see all events in a group after filtering?**
A: Click the `📁 subgroup_name ✕` badge at the top of the screen

**Q: Can I move events between subgroups?**
A: Yes! Edit the event and change the subgroup dropdown to a different one

**Q: Can subgroups have different permissions?**
A: Not yet - this is a future feature. All group members can see all subgroups

**Q: How deep can I nest subgroups?**
A: Technically unlimited! But for usability, we recommend 2-3 levels max

## That's It!

You're now ready to organize your group events with subgroups. Start simple with 2-3 subgroups and expand as needed! 🚀
