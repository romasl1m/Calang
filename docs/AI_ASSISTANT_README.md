# AI Assistant for Calang Terminal

## Overview
The AI Assistant uses Google's Gemini API to interpret natural language commands and convert them into terminal commands. This allows users to interact with the calendar application using everyday language instead of memorizing command syntax.

## Setup

### 1. Get Gemini API Key
1. Visit: https://makersuite.google.com/app/apikey
2. Sign in with your Google account
3. Create a new API key

### 2. Configure API Key

**Option A: Environment Variable (Recommended)**
```bash
export GEMINI_API_KEY="your_api_key_here"
```

Add to your `~/.bashrc` or `~/.zshrc` to make it permanent:
```bash
echo 'export GEMINI_API_KEY="your_api_key_here"' >> ~/.bashrc
```

**Option B: Config File**
Create a `.gemini_config` file in the calang directory:
```bash
echo "your_api_key_here" > .gemini_config
```

## Usage

### Basic Syntax
```bash
ai "your natural language request"
```

### Examples

#### View Events
```bash
ai "show me my events today"
ai "what meetings do I have tomorrow?"
ai "show next 5 events"
ai "find events with word meeting"
```

#### Create Events
```bash
ai "create meeting tomorrow at 2pm for 1 hour"
ai "add team standup on 15.06 at 09:00 for 30 minutes"
ai "schedule dentist appointment on June 20 at 14:00"
```

#### Navigate Groups
```bash
ai "switch to work group"
ai "go to engineering/frontend group"
ai "list all my groups"
```

#### Modify Events
```bash
ai "change the description of meeting event"
ai "delete the event with id 123456"
```

## How It Works

1. **User Input**: You provide a natural language command
2. **AI Processing**: The request is sent to Gemini API with context about:
   - Current username
   - Current group/subgroup
   - Available commands and syntax
3. **Command Generation**: Gemini converts your request to terminal commands
4. **Execution**: Commands are automatically executed
5. **Output**: Results are displayed to you

## Features

### Silent Command Execution
The AI can run `cat` and `cd` commands silently when needed:
- Finding events doesn't clutter output
- Group navigation happens in the background
- Only relevant results are shown

### Context Awareness
The AI knows:
- What group you're currently in
- Your username
- Available groups and subgroups
- All terminal command syntax

### Smart Search
When you ask to "change the description of meeting":
1. AI searches for "meeting" using `grep`
2. Finds the matching event
3. Prepares the edit command
4. Shows you the event to confirm

## Supported Operations

### Search & View
- Show events by date
- Find upcoming events
- Search by keyword
- List groups and subgroups

### Create
- Create events with natural dates
- Set priorities (high/medium/low)
- Add recurrence (daily/weekly/monthly/yearly)
- Assign to subgroups

### Navigate
- Change to groups by name
- Navigate to subgroups using path notation
- Return to private calendar

### Delete
- Remove events by ID
- Confirm before deletion

## Tips

1. **Be Specific**: "tomorrow at 2pm" is better than "soon"
2. **Use Natural Language**: "next Monday" works as well as "17.06"
3. **Check Output**: The AI shows what command it's running
4. **Error Recovery**: If AI makes a mistake, you can run the command manually

## Command Reference

The AI can generate these terminal commands:

| Command | Purpose | Example |
|---------|---------|---------|
| `cat YYYY-MM-DD` | Show events on date | `cat 2026-06-15` |
| `cat N` | Show next N events | `cat 5` |
| `grep "text"` | Search events | `grep "meeting"` |
| `touch` | Create event | See touch syntax below |
| `cd group` | Change group | `cd work` |
| `cd group/sub` | Navigate subgroup | `cd work/frontend` |
| `ls` | List groups | `ls` |
| `rm id` | Delete event | `rm 1234567890` |

### Touch Command Syntax

**Standard Format:**
```bash
touch "title" DD.MM HH:MM HH:MM "description" [P=priority] [T=recurrence] [S=subgroup]
```

**Length Format:**
```bash
touch "title" in DD.MM HH:MM length HH:MM "description" [P=priority] [T=recurrence] [S=subgroup]
```

**AI Format (full timestamps):**
```bash
touch "title" "YYYY-MM-DD HH:MM" "YYYY-MM-DD HH:MM" "description" [P=priority] [T=recurrence]
```

## Troubleshooting

### "API key not found"
- Make sure `GEMINI_API_KEY` is set in environment
- Or create `.gemini_config` file with your key
- Verify key is valid at https://makersuite.google.com

### "AI generated invalid command"
- Check the command it tried to run
- Run the correct command manually
- The AI learns from context, so try rephrasing

### "No response from AI"
- Check internet connection
- Verify API key is valid and has quota
- Try again in a few moments

## Privacy & Security

- Your API key should be kept private
- Don't commit `.gemini_config` to version control
- Add to `.gitignore`: `echo ".gemini_config" >> .gitignore`
- Calendar data is only sent to Gemini for command interpretation
- No calendar data is stored by Google

## Future Enhancements

Planned features:
- Edit existing events through AI
- Bulk operations ("delete all events next week")
- Smart scheduling ("find free time tomorrow")
- Natural language date parsing improvements
- Multi-step operations with confirmation
- Learning from user corrections

## Examples Gallery

### Morning Routine
```bash
# Check what's coming up
ai "what do I have today"

# Add quick reminder
ai "remind me to call mom at 5pm"

# Check work calendar
cd work
ai "show me this week's meetings"
```

### Event Management
```bash
# Search and modify
ai "find team standup events"
ai "show all high priority events"

# Clean up
ai "show events from last month"
# Then manually review and delete old ones
```

### Group Organization
```bash
# List structure
ai "show all my groups"

# Navigate and create
cd engineering/backend
ai "add code review meeting tomorrow 3pm 30 minutes"
```

## Support

For issues or feature requests:
- Check this documentation first
- Review terminal command syntax with `help`
- Test commands manually to verify they work
- Report bugs with example AI input that failed
