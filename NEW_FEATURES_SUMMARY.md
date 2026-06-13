# New Features Summary

## Overview
This document summarizes the two major features added to Calang terminal:

1. **Subgroup Navigation** - Navigate to subgroups like directories
2. **AI Assistant** - Natural language interface using Gemini API

---

## Feature 1: Subgroup Navigation

### What Changed
Previously, you could use `cd <group>` to switch to a group, but subgroups weren't directly accessible in the terminal. Now you can navigate the full hierarchy.

### New Capabilities

#### Navigate to Subgroups
```bash
cd engineering/frontend
cd engineering/frontend/web
```

#### List Groups and Subgroups
```bash
ls
```

Output shows full structure:
```
Available groups:
  Engineering (id: 1234567890)
    Subgroups:
      frontend
      frontend/web
      frontend/mobile
      backend
```

#### Automatic Context
When you navigate to a subgroup, events created with `touch` automatically:
- Belong to the parent group
- Are tagged with the subgroup path

Example:
```bash
cd engineering/frontend
touch "Meeting" in 15.06 14:00 length 01:00 "Team sync"
# This event is now in "engineering" group with "frontend" subgroup
```

### Technical Implementation

**Files Modified:**
- `src/terminal.cpp` - Added subgroup path parsing in `cd` command
- `src/terminal.cpp` - Modified `touch` command to parse currentgroup for subgroup context
- `src/terminal.cpp` - Added `ls` command to display groups and subgroups

**Key Functions:**
- Modified `cd` command to handle paths with `/` separator
- Added validation to check if subgroup exists before navigation
- Modified `touch` command to extract subgroup from currentgroup context
- Uses existing `get_subgroups()` function from `functions.cpp`

---

## Feature 2: AI Assistant

### What Changed
Added natural language processing capability using Google's Gemini API. Users can now interact with the calendar using everyday language instead of memorizing command syntax.

### New Capabilities

#### Natural Language Commands
```bash
ai "show me my meetings today"
ai "create meeting tomorrow at 2pm for 1 hour"
ai "switch to engineering/frontend"
ai "find events about standup"
```

#### Smart Command Translation
The AI:
1. Understands context (current user, group, subgroup)
2. Converts natural language to terminal commands
3. Executes commands automatically
4. Shows clean output without intermediate steps

#### Silent Command Execution
When searching or navigating:
- `grep` runs silently to find events
- `cd` runs silently to change context
- Only relevant results shown to user

### Technical Implementation

**New Files:**
- `src/ai_assistant.h` - Header file with AI function declarations
- `src/ai_assistant.cpp` - AI integration implementation
- `AI_ASSISTANT_README.md` - Complete AI feature documentation
- `.gemini_config.example` - Example configuration file

**Files Modified:**
- `src/terminal.cpp` - Added `ai` command handler
- `CMakeLists.txt` - Added ai_assistant.cpp to build
- `.gitignore` - Added .gemini_config

**Key Components:**

1. **API Integration** (`ai_assistant.cpp`):
   - `call_gemini_api()` - Makes HTTP requests to Gemini API
   - `get_gemini_api_key()` - Reads API key from env or config file
   - `build_ai_prompt()` - Creates context-aware prompt for AI
   - `extract_command_from_response()` - Parses AI response for commands
   - `process_ai_command()` - Main entry point, orchestrates the flow

2. **Dependencies**:
   - Uses existing `curl` library for HTTP requests
   - Uses existing `nlohmann/json` for JSON parsing
   - Integrates with existing `process_terminal_command()` function

3. **Configuration**:
   - Environment variable: `GEMINI_API_KEY`
   - Or config file: `.gemini_config`
   - Get key from: https://makersuite.google.com/app/apikey

### How It Works

```
User Input: "show me meetings tomorrow"
     ↓
Build Prompt: Add context (user, group, available commands)
     ↓
Call Gemini API: Send prompt, get response
     ↓
Parse Response: Extract terminal command from AI response
     ↓
Execute: Run command(s) through process_terminal_command()
     ↓
Output: Show results to user
```

---

## Additional Improvements

### New Commands

#### `ls` - List Groups and Subgroups
Shows all groups you're a member of and their subgroup structure.

#### `rm <id>` - Delete Event
Delete an event by its ID. Finds the event and removes it from the correct group.

#### `ai "request"` - AI Assistant
Process natural language requests and execute appropriate commands.

### Enhanced Help
Updated `help` command to show:
- Subgroup navigation syntax
- AI command usage
- New ls and rm commands

### Documentation
Created comprehensive documentation:
1. **TERMINAL_FEATURES_GUIDE.md** - Complete user guide with examples
2. **AI_ASSISTANT_README.md** - Detailed AI setup and usage
3. **NEW_FEATURES_SUMMARY.md** - This file
4. **.gemini_config.example** - Configuration template

---

## Usage Examples

### Example 1: Morning Workflow
```bash
# Check today's schedule
ai "what do I have today?"

# Quick reminder
ai "remind me to call client at 3pm"

# Navigate to team context
cd engineering/frontend

# Add recurring standup
touch "Daily Standup" in 15.06 09:00 length 00:15 "Team sync" T=daily
```

### Example 2: Cross-team Meeting
```bash
# Navigate to backend team
cd engineering/backend

# Create event
ai "schedule API review tomorrow 2pm for 1 hour with high priority"

# Later, search across all events
cd ~
grep "review"
```

### Example 3: Natural Planning
```bash
# Ask AI to help
ai "show next week's meetings"

# Add event naturally
ai "add team lunch Friday noon for 1 hour"

# Navigate and organize
ai "go to projects/alpha"
ai "create sprint planning for Monday 10am weekly"
```

---

## Setup Instructions

### For Subgroup Navigation
No setup required! Just use `cd` with paths:
```bash
ls                          # See available groups
cd engineering/frontend     # Navigate
```

### For AI Assistant

**Step 1: Get API Key**
1. Visit https://makersuite.google.com/app/apikey
2. Sign in with Google account
3. Create API key

**Step 2: Configure**

Option A - Environment Variable (Recommended):
```bash
export GEMINI_API_KEY="your_key_here"
# Add to ~/.bashrc for persistence
echo 'export GEMINI_API_KEY="your_key_here"' >> ~/.bashrc
```

Option B - Config File:
```bash
echo "your_key_here" > .gemini_config
```

**Step 3: Test**
```bash
ai "show me today's events"
```

---

## Build Instructions

### Prerequisites
- C++17 compiler
- CMake 3.16+
- curl library
- nlohmann/json library

### Build Steps
```bash
# From project root
mkdir -p build
cmake -S . -B build
cmake --build build

# Run
./build/calang
```

### Build Configuration
All necessary changes already in `CMakeLists.txt`:
- Added `src/ai_assistant.cpp` to sources
- `curl` library already linked
- No additional dependencies needed

---

## File Changes Summary

### New Files
```
src/ai_assistant.h              - AI assistant header
src/ai_assistant.cpp            - AI assistant implementation
AI_ASSISTANT_README.md          - AI documentation
TERMINAL_FEATURES_GUIDE.md      - Complete features guide
NEW_FEATURES_SUMMARY.md         - This file
.gemini_config.example          - Configuration template
```

### Modified Files
```
src/terminal.cpp                - Added: ls, ai, rm commands
                                - Modified: cd, touch for subgroups
CMakeLists.txt                  - Added: ai_assistant.cpp
.gitignore                      - Added: .gemini_config
```

### No Changes Needed
```
src/functions.cpp               - Uses existing functions
src/functions.h                 - Uses existing declarations
All other source files          - No modifications
```

---

## API Usage and Costs

### Gemini API
- **Model**: gemini-pro
- **Endpoint**: Google's Generative Language API
- **Free Tier**: Available for development/testing
- **Pricing**: Check https://ai.google.dev/pricing for current rates

### Typical Request
Each AI command makes one API request:
- Input: ~500-800 tokens (context + user query)
- Output: ~50-200 tokens (generated commands)
- Very cost-effective for this use case

### Privacy
- Only command context sent to API (username, current group)
- No event data sent unless needed for command generation
- Calendar data stays local
- API key stored locally only

---

## Testing

### Test Subgroup Navigation
```bash
# Create test structure (if not exists)
# Through web UI: Create group "TestGroup" with subgroups "sub1" and "sub1/nested"

# Test navigation
ls                      # Should show TestGroup with subgroups
cd TestGroup           # Navigate to group
cd TestGroup/sub1      # Navigate to subgroup
cd TestGroup/sub1/nested  # Navigate to nested

# Test event creation
cd TestGroup/sub1
touch "Test Event" in 15.06 10:00 length 01:00 "Testing subgroup context"
# Verify: Event should be in TestGroup group with sub1 subgroup

# Test error handling
cd NonExistent         # Should show error
cd TestGroup/wrong     # Should show available subgroups
```

### Test AI Assistant
```bash
# Without API key - should show error message
ai "test"

# Set API key
export GEMINI_API_KEY="your_key"

# Test basic queries
ai "show me today's events"
ai "list all my groups"

# Test event creation
ai "create test meeting tomorrow at 2pm for 1 hour"
cat 5  # Verify event created

# Test navigation
ai "switch to TestGroup"
ai "go to TestGroup/sub1"

# Test search
ai "find all test events"
grep "test"  # Compare results
```

### Test Integration
```bash
# Navigate with cd
cd TestGroup/sub1

# Create with AI (should use current context)
ai "add team meeting tomorrow 10am 1 hour"
# Verify event is in TestGroup/sub1

# Search with AI
ai "show meetings in this group"

# Delete with AI
ai "find team meeting"
# Note the ID, then:
rm <event_id>
```

---

## Troubleshooting

### Build Issues
**Problem**: `curl/curl.h: No such file or directory`
```bash
# Install curl development library
sudo apt-get install libcurl4-openssl-dev  # Ubuntu/Debian
sudo yum install libcurl-devel              # CentOS/RHEL
```

**Problem**: `nlohmann/json.hpp not found`
- Verify `external/nlohmann/json.hpp` exists in project
- Should already be present, included in project

### Runtime Issues
**Problem**: AI command doesn't work
1. Check API key is set: `echo $GEMINI_API_KEY`
2. Or check config file exists: `cat .gemini_config`
3. Verify API key is valid at https://makersuite.google.com

**Problem**: Subgroups not showing
1. Verify subgroups exist: Check `groups/<group_id>/info.json`
2. Verify you're a member of the group
3. Try `ls` to see all available groups

**Problem**: Events created in wrong subgroup
1. Check current context (shown in prompt or via `whoami`)
2. Use `cd` to navigate to correct subgroup first
3. Or use explicit `S=subgroup` parameter in touch command

---

## Future Enhancements

### Possible Improvements
1. **Edit Command**: Modify existing events from terminal
2. **AI Learning**: Remember user preferences and patterns
3. **Batch Operations**: Bulk delete, modify multiple events
4. **Smart Scheduling**: AI finds free time slots
5. **Natural Date Parsing**: Better handling of "next Tuesday", "in 2 weeks"
6. **Voice Input**: Integration with speech recognition
7. **Shortcuts**: Aliases for common commands
8. **History**: Command history with up/down arrows
9. **Tab Completion**: Auto-complete group/subgroup names
10. **Export/Import**: Terminal commands to backup/restore

### Community Contributions
If you'd like to contribute:
1. Follow existing code style
2. Test thoroughly with various scenarios
3. Update documentation
4. Consider edge cases (empty groups, special characters, etc.)

---

## Credits

### Technologies Used
- **Gemini API** by Google - Natural language processing
- **nlohmann/json** - JSON parsing
- **libcurl** - HTTP requests
- **C++17** - Modern C++ features

### Implementation
- Subgroup navigation: Extends existing subgroup system
- AI integration: New module with minimal changes to existing code
- Documentation: Comprehensive guides and examples

---

## Support

### Documentation
- `TERMINAL_FEATURES_GUIDE.md` - Complete user guide
- `AI_ASSISTANT_README.md` - AI-specific documentation
- `SUBGROUPS_FEATURE.md` - Subgroup system details
- Type `help` in terminal - Quick reference

### Getting Help
1. Read the documentation
2. Try `help` command in terminal
3. Check troubleshooting sections
4. Review examples in guides
5. Test with simple cases first

### Reporting Issues
When reporting problems, include:
- What command you ran
- Expected behavior
- Actual behavior
- Error messages
- Context (current group, etc.)

---

## Quick Start

### 1. Build the Project
```bash
cmake -S . -B build && cmake --build build
```

### 2. Setup AI (Optional)
```bash
export GEMINI_API_KEY="your_key_here"
```

### 3. Run and Test
```bash
./build/calang
# In the terminal:
ls                                      # See groups
cd engineering/frontend                 # Navigate
touch "Test" in 15.06 14:00 length 01:00 "Testing"  # Create event
ai "show me today's events"             # Use AI
```

---

## Conclusion

These features make Calang terminal significantly more powerful and user-friendly:

✅ **Subgroup Navigation**: Intuitive hierarchical navigation like file systems
✅ **AI Assistant**: Natural language interface for quick interactions  
✅ **Better UX**: Less memorization, more productivity
✅ **Extensible**: Easy to add more features on top of this foundation
✅ **Well Documented**: Comprehensive guides and examples

The implementation maintains backward compatibility while adding significant new capabilities. All existing commands work as before, with enhancements to support the new features.
