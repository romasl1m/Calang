# Calang Terminal - Quick Start Guide

## Installation

```bash
# Build the project
cmake -S . -B build
cmake --build build

# Run
./build/calang
```

## Setup AI (Optional but Recommended)

```bash
# Get API key from: https://makersuite.google.com/app/apikey
export GEMINI_API_KEY="your_api_key_here"

# Or create config file:
echo "your_api_key_here" > .gemini_config
```

## Essential Commands

### Navigation
```bash
ls                          # List all groups and subgroups
cd engineering             # Navigate to group
cd engineering/frontend    # Navigate to subgroup
cd ~                       # Return to private calendar
whoami                     # Show current user
```

### View Events
```bash
cat 2026-06-15            # Events on specific date
cat 5                     # Next 5 upcoming events
grep "meeting"            # Search for events
dates                     # Show stored date variables
```

### Create Events
```bash
# Simple format
touch "Meeting" 15.06 14:00 15:00 "Team sync"

# With duration
touch "Workshop" in 15.06 09:00 length 02:00 "Training"

# With options
touch "Standup" 15.06 09:00 09:15 "Daily standup" T=daily P=high
```

**Options:**
- `P=high|medium|low` - Priority
- `T=daily|weekly|monthly|yearly` - Recurrence  
- `S=subgroup/path` - Specific subgroup

### Delete Events
```bash
rm 1234567890             # Delete event by ID
```

### AI Assistant
```bash
ai "show me today's events"
ai "create meeting tomorrow 2pm for 1 hour"
ai "switch to engineering/frontend"
ai "find all standup meetings"
```

## Common Workflows

### Morning Check
```bash
cd ~
ai "what do I have today?"
cat 5
```

### Add Quick Event
```bash
ai "remind me to call client at 3pm"
```

### Team Context
```bash
cd engineering/frontend
touch "Code Review" in 15.06 14:00 length 01:00 "Review PRs"
cat 2026-06-15
```

### Search and Delete
```bash
grep "old meeting"
# Note the ID from output
rm 1234567890
```

## Tips

1. **Use AI for natural requests** - Faster than memorizing syntax
2. **Use `cd` to set context** - Events auto-assign to current group/subgroup
3. **Use `ls` frequently** - See available groups and paths
4. **Use `$DATE` variables** - After cat/grep, dates are stored
5. **Check with `cat 5`** - Quick overview of upcoming events

## Help

```bash
help                      # Full command reference
```

**Documentation:**
- `TERMINAL_FEATURES_GUIDE.md` - Complete guide with examples
- `AI_ASSISTANT_README.md` - AI setup and usage
- `NEW_FEATURES_SUMMARY.md` - Technical overview

## Examples

```bash
# Natural workflow with AI
ai "what meetings do I have tomorrow?"
ai "add team lunch Friday at noon for 1 hour"
ai "switch to engineering group"

# Direct commands
cd engineering/backend
touch "API Review" in 16.06 14:00 length 01:00 "Review endpoints" P=high
grep "review"
cat 7

# Using date variables
grep "meeting"
dates                     # See stored dates
touch "Prep" in $DATE[0] 13:00 length 01:00 "Prepare for meeting"
```

That's it! You're ready to use Calang terminal. 🚀
