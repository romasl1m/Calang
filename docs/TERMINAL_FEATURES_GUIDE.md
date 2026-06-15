# Terminal Features Guide

This guide covers the enhanced terminal features in Calang, including subgroup navigation and AI assistance.

## Table of Contents
1. [Subgroup Navigation](#subgroup-navigation)
2. [AI Assistant](#ai-assistant)
3. [Command Reference](#command-reference)
4. [Examples](#examples)

---

## Subgroup Navigation

### Overview
You can now navigate directly to subgroups using the `cd` command, just like navigating directories in a file system.

### Syntax
```bash
cd <group_name>/<subgroup>/<nested_subgroup>
```

### Examples

**Navigate to a group:**
```bash
cd engineering
```

**Navigate to a subgroup:**
```bash
cd engineering/frontend
```

**Navigate to nested subgroups:**
```bash
cd engineering/frontend/web
```

**Return to private calendar:**
```bash
cd ~
# or
cd private
```

### List Available Groups and Subgroups
```bash
ls
```

Output example:
```
Available groups:
  Engineering (id: 1234567890)
    Subgroups:
      frontend
      frontend/web
      frontend/mobile
      backend
      backend/api
      backend/database

Use: cd <group_name>[/subgroup]
```

### How It Works
- When you `cd` into a group/subgroup, all subsequent `touch` commands automatically create events in that location
- The terminal prompt shows your current location
- Use `cat` and `grep` to view events filtered by your current location
- Events created with `touch` inherit the current group/subgroup context

### Subgroup Context
When you navigate to a subgroup, events created with `touch` automatically:
1. Belong to the group (first part of the path)
2. Are tagged with the subgroup (rest of the path)

Example:
```bash
# Navigate to frontend subgroup
cd engineering/frontend

# Create an event - it will automatically be in "engineering" group with "frontend" subgroup
touch "Code Review" in 15.06 14:00 length 01:00 "Review pull requests"
```

---

## AI Assistant

### Setup
Before using the AI assistant, you need a Gemini API key:

1. Get your API key from: https://makersuite.google.com/app/apikey
2. Set it up using one of these methods:

**Method 1: Environment Variable**
```bash
export GEMINI_API_KEY="your_api_key_here"
```

**Method 2: Config File**
```bash
echo "your_api_key_here" > .gemini_config
```

### Usage
```bash
ai "your natural language request"
```

### What the AI Can Do

#### 1. Search and View Events
```bash
ai "show me my meetings today"
ai "what do I have tomorrow?"
ai "find events with the word standup"
ai "show next 5 events"
```

The AI will:
- Convert your request to a `cat` or `grep` command
- Execute it silently if needed
- Show you the results

#### 2. Create Events
```bash
ai "create meeting tomorrow at 2pm for 1 hour"
ai "add team standup on 15.06 at 09:00 for 30 minutes with high priority"
ai "schedule dentist appointment next Monday at 14:00"
```

The AI understands:
- Natural dates: "tomorrow", "next Monday", "15.06"
- Durations: "for 1 hour", "for 30 minutes"
- Priorities: "high priority", "urgent"
- Recurrence: "daily", "weekly"

#### 3. Navigate Groups
```bash
ai "switch to engineering group"
ai "go to work/frontend"
ai "show all my groups"
ai "go back to private calendar"
```

#### 4. Delete Events
```bash
ai "delete the meeting event"
ai "remove event with id 1234567890"
```

### How It Works

1. **Context Awareness**: The AI knows:
   - Your current username
   - Current group/subgroup
   - Available commands

2. **Command Translation**: Your natural language → terminal commands

3. **Silent Execution**: Some commands run in the background:
   - Navigation (`cd`)
   - Initial searches (`grep`, `cat`)

4. **Smart Output**: Only shows relevant results, not intermediate steps

### Example Workflow

```bash
# Morning check
ai "what meetings do I have today?"

# Quick add
ai "remind me to call client at 3pm"

# Navigate and organize
ai "switch to engineering/backend"
ai "add code review tomorrow 2pm for 1 hour"

# Search and modify
ai "find all standup meetings"
# Review the output, note the IDs
rm 1234567890  # Delete specific one
```

---

## Command Reference

### Navigation Commands

| Command | Description | Example |
|---------|-------------|---------|
| `cd <group>` | Navigate to group | `cd engineering` |
| `cd <group>/<sub>` | Navigate to subgroup | `cd engineering/frontend` |
| `cd ~` or `cd private` | Return to private | `cd ~` |
| `ls` | List groups and subgroups | `ls` |
| `whoami` | Show current user | `whoami` |

### Event Commands

| Command | Description | Example |
|---------|-------------|---------|
| `cat YYYY-MM-DD` | Show events on date | `cat 2026-06-15` |
| `cat N` | Show next N events | `cat 5` |
| `grep "text"` | Search events | `grep "meeting"` |
| `rm <id>` | Delete event | `rm 1234567890` |
| `dates` | Show stored dates | `dates` |

### Create Event (touch)

**Standard format:**
```bash
touch "title" DD.MM HH:MM HH:MM "description" [options]
```

**With duration:**
```bash
touch "title" in DD.MM HH:MM length HH:MM "description" [options]
```

**Options:**
- `P=high|medium|low` - Priority
- `T=daily|weekly|monthly|yearly` - Recurrence
- `S=subgroup/path` - Specific subgroup
- `O=group_id` - Override current group

**Examples:**
```bash
# Simple event
touch "Meeting" 15.06 14:00 15:00 "Team sync"

# With duration
touch "Workshop" in 15.06 09:00 length 02:30 "Training session"

# With options
touch "Standup" 15.06 09:00 09:15 "Daily standup" T=daily P=high

# Full control
touch "Review" 15.06 14:00 15:00 "Code review" S=frontend P=high O=engineering
```

### AI Command

```bash
ai "natural language request"
```

**Examples:**
```bash
ai "show me tomorrow's schedule"
ai "create meeting next Monday 2pm 1 hour"
ai "switch to engineering/backend group"
ai "find all high priority events"
```

### Utility Commands

| Command | Description |
|---------|-------------|
| `clear` | Clear terminal screen |
| `help` | Show command help |
| `dates` | Show dates from recent searches |

### Date Variables

After using `cat` or `grep`, dates are stored in variables:
- `$DATE` - First date found
- `$DATE[0]` - First date
- `$DATE[1]` - Second date
- `$DATE[n]` - Nth date

**Example:**
```bash
# Search for events
grep "meeting"

# Use stored date to create related event
touch "Follow-up" in $DATE 15:00 length 01:00 "Follow-up meeting"
```

---

## Examples

### Example 1: Daily Workflow with Subgroups

```bash
# Morning: Check personal calendar
cd ~
cat 2026-06-15

# Switch to work context
cd engineering/frontend
ai "show today's meetings"

# Add standup
touch "Daily Standup" in 15.06 09:00 length 00:15 "Team sync" T=daily

# Switch to backend team
cd engineering/backend
touch "API Review" in 15.06 14:00 length 01:00 "Review new endpoints"

# End of day: Check what's coming
cd ~
cat 5
```

### Example 2: Using AI for Natural Interaction

```bash
# Start natural
ai "what do I have this week?"

# Quick adds
ai "remind me to submit report tomorrow at 5pm"
ai "add team lunch on Friday at noon for 1 hour"

# Navigate naturally
ai "switch to engineering group"
ai "show me frontend subgroup events"

# Complex creation
ai "create a weekly meeting on Mondays at 10am for 30 minutes called sprint planning with high priority"
```

### Example 3: Advanced Search and Filter

```bash
# Search across all events
grep "review"

# Results show dates in $DATE[0], $DATE[1], etc.
dates

# Create related event using found date
touch "Prepare for Review" in $DATE[0] 13:00 length 01:00 "Preparation"

# Find high priority items (manually check output)
grep "high"

# Navigate to specific subgroup for focused work
cd projects/alpha
cat 7  # Next 7 events in this subgroup
```

### Example 4: Group Management

```bash
# See all groups
ls

# Navigate through structure
cd engineering
ls  # Would show subgroups if implemented

cd engineering/frontend/web
touch "Bug Fix Session" in 16.06 10:00 length 02:00 "Fix critical bugs"

# Override group for cross-team event
touch "All Hands" 20.06 15:00 16:00 "Company meeting" O=private

# Back to private
cd ~
```

### Example 5: Event Lifecycle with AI

```bash
# Create with AI
ai "schedule design review next Tuesday 2pm for 2 hours"

# Later, find it
ai "find design review events"

# Note the ID from output, then delete
rm 1234567890

# Verify deletion
ai "find design review events"
```

---

## Tips and Best Practices

### For Subgroup Navigation
1. Use `ls` frequently to see available paths
2. Remember: `cd` changes context for all subsequent `touch` commands
3. Use `cd ~` to reset to private calendar
4. Subgroup paths use `/` separator, like file paths

### For AI Assistant
1. Be specific with dates and times
2. Include duration ("for 1 hour") for clarity
3. Mention priority/recurrence if important
4. Check the AI-generated command before it executes
5. Use natural language - don't try to format like commands

### General Workflow
1. Start with `cd` to set context
2. Use `ai` for quick, natural interactions
3. Use direct commands when you know exactly what you want
4. Use `$DATE` variables to chain commands efficiently
5. Regularly use `cat 5` to see what's coming up

### Debugging
- If AI generates wrong command, run it manually with corrections
- Use `help` to see all available commands
- Check `dates` to see what date variables are available
- Use `grep` to find events before modifying them

---

## Troubleshooting

### Subgroups
**Problem**: "Subgroup not found"
- Run `ls` to see available subgroups
- Check spelling and path format (use `/` not `\`)
- Ensure you're a member of the parent group

### AI Assistant
**Problem**: "API key not found"
- Set `GEMINI_API_KEY` environment variable
- Or create `.gemini_config` file
- Get key from https://makersuite.google.com/app/apikey

**Problem**: AI generates wrong command
- Check what command it tried
- Run the correct command manually
- Try rephrasing your request

### General
**Problem**: Event created in wrong group
- Check current context with `whoami` and current prompt
- Use `cd` to change context
- Or override with `O=group_id` parameter

**Problem**: Can't find events
- Try broader search with `grep`
- Check if you're in right group context
- Use `cd ~` and search all events

---

## Quick Reference Card

```
Navigation:
  cd group/subgroup    → Navigate to context
  cd ~                 → Return to private
  ls                   → List groups

Events:
  cat DATE|N           → View events
  grep "text"          → Search
  touch ...            → Create
  rm ID                → Delete

AI:
  ai "request"         → Natural language

Context:
  Current group affects 'touch' commands
  Use cd to change context
  Use O= to override

Variables:
  $DATE, $DATE[0-n]    → Dates from searches
```

---

## Getting Help

- Type `help` in terminal for command list
- Read `AI_ASSISTANT_README.md` for detailed AI guide
- Read `SUBGROUPS_FEATURE.md` for subgroup implementation details
- Check examples in this guide
