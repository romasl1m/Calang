# Command Synchronization Test Examples

## Basic Command Chaining

### Using && (AND operator)
Executes the second command only if the first succeeds:
```bash
cat 10 && touch "Meeting" in 16.06 14:00 length 01:00 "Review session"
```

### Using || (OR operator)
Executes the second command only if the first fails:
```bash
grep "nonexistent" || echo "Fallback: No events found"
```

### Using ; (Sequential execution)
Always executes both commands regardless of success:
```bash
cat 2026-06-20 ; dates
```

## Conditional Statements

### Simple if-then-else
```bash
if grep "important" then cat $DATE else cat 5
```

### Test commands for conditions
```bash
test -n "some string"
```

## Complex Examples

### Chain multiple commands
```bash
cat 5 && dates && touch "New Event" in 17.06 10:00 length 02:00 "Workshop"
```

### AI can use these operators
```bash
ai "show me next 10 events and if there are any important ones, show them"
```

## How AI Should Use These

The AI assistant can now generate bash-like commands:
- Use `&&` to chain dependent operations (e.g., find events first, then create based on results)
- Use `||` for fallback behavior (e.g., search for events, if none found, create new)
- Use `;` to run multiple independent queries
- Use `if/then/else` for conditional logic based on command success

## Command Success/Failure

A command is considered **failed** if its output contains:
- "Error:"
- "not found"
- "Syntax error"
- "Unknown command"
- "No events found"

Otherwise, it's considered **successful**.
