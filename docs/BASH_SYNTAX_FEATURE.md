# Bash-Like Command Synchronization

## Overview
Added bash-like command synchronization and conditional logic to the Calang terminal, allowing users and the AI to chain commands with logical operators.

## Features Added

### 1. Command Chaining Operators

#### `&&` (AND operator)
- Executes the second command **only if** the first command succeeds
- Example: `cat 10 && touch "Meeting" in 16.06 14:00 length 01:00 "Discussion"`
- Use case: Create an event only if you successfully fetch existing events

#### `||` (OR operator)
- Executes the second command **only if** the first command fails
- Example: `grep "project" || cat 5`
- Use case: Fallback behavior - show next 5 events if search returns nothing

#### `;` (Sequential operator)
- Executes all commands **regardless** of success/failure
- Example: `cat 2026-06-20 ; dates ; ls`
- Use case: Run multiple independent queries in sequence

### 2. Conditional Statements

#### `if-then-else` syntax
```bash
if <condition_command> then <success_command> else <failure_command>
```

Example:
```bash
if grep "important" then cat $DATE else cat 5
```

The `else` clause is optional:
```bash
if cat 10 then dates
```

### 3. Test Commands

#### `test` or `[` command
- `test -n "string"` - Test if string is not empty (exits successfully if true)
- `test -z "string"` - Test if string is empty (exits successfully if true)
- Can use `[` as alias: `[ -n "string" ]`

Example:
```bash
if test -n "$DATE" then cat $DATE else cat 5
```

## Success/Failure Detection

A command is considered **failed** if its output contains any of:
- "Error:"
- "not found"
- "Syntax error"  
- "Unknown command"
- "No events found" (for cat/grep commands)

Otherwise, the command is considered **successful**.

## Complex Examples

### 1. Chain multiple operations
```bash
cat 5 && dates && touch "New Event" in 17.06 10:00 length 02:00 "Workshop"
```
Fetch 5 events, show dates, then create a new event (only if previous commands succeed)

### 2. Search with fallback
```bash
grep "important" || cat 10
```
Search for "important" events, or show next 10 events if search fails

### 3. Multiple independent queries
```bash
cat 2026-06-20 ; ls ; dates
```
Show events on specific date, list groups, show stored dates (all run regardless)

### 4. Conditional creation
```bash
if grep "meeting" then rm event_id_here else touch "New Meeting" in 20.06 14:00 length 01:00 "Weekly sync"
```

### 5. Complex AI usage
The AI can generate sophisticated command chains:
```bash
ai "show next 10 events and if any are labeled important, display their dates"
```

The AI will generate:
```bash
cat 10 && grep "important" && dates
```

## Implementation Details

### Files Modified
1. **src/terminal.cpp**
   - Added `execute_single_command()` - handles individual command execution
   - Modified `process_terminal_command()` - parses and executes chained commands
   - Added `if` command handler for conditional logic
   - Added `test`/`[` command handler for boolean tests
   - Updated help text with new operators

2. **src/ai_assistant.cpp**
   - Updated AI prompt to teach it about bash-like operators
   - Added examples of command chaining
   - Instructed AI to use logical operators when appropriate

### Parser Logic
- Respects quoted strings (won't split `&&` inside quotes)
- Properly handles whitespace trimming
- Maintains command execution order
- Short-circuits evaluation (doesn't execute unnecessary commands)

### Special Signals
The system still respects special signals like:
- `CLEAR_SIGNAL` - clears the terminal
- `RELOAD_CALENDAR` - refreshes calendar display

## Usage for AI Assistant

The AI can now generate more intelligent command sequences:

**Before:**
```
cat 10
grep "important"
```
(Two separate commands, user has to run each)

**After:**
```bash
cat 10 && grep "important" || cat 5
```
(Fetch 10 events, search for important ones, or show next 5 if none found)

## Benefits

1. **Reduced user interaction** - Multiple operations in one command
2. **Intelligent fallbacks** - Automatic error handling with `||`
3. **Conditional logic** - Smart branching with `if-then-else`
4. **Bash familiarity** - Users already know these operators
5. **AI efficiency** - AI can generate more sophisticated command chains

## Testing

Build the project:
```bash
cmake -B build .
cmake --build build
```

Test examples:
```bash
# Test AND operator
cat 10 && dates

# Test OR operator  
grep "nonexistent" || cat 5

# Test sequential
cat 2026-06-20 ; ls ; dates

# Test conditional
if grep "meeting" then cat $DATE else cat 5
```

## Future Enhancements

Possible additions:
- Loop constructs (`while`, `for`)
- Variables (`VAR=value`, `echo $VAR`)
- Functions (`function name() { ... }`)
- Piping (`cat 10 | grep "text"`)
- Redirects (`cat 10 > output.txt`)
- Background jobs (`cat 10 &`)
