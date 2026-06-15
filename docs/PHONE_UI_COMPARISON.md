# Phone UI vs Desktop UI Comparison

## Layout Comparison

### Desktop View (>768px)
```
┌─────────────────────────────────────────────────────────┐
│ [Logo] Calang                                    [User] │ 52px topbar
├───────┬─────────────────────────────────────────────────┤
│       │ [+ New Event] [Terminal] [Week Badge]           │ 48px toolbar
│ Grps  ├─────────────────────────────────────────────────┤
│       │ ┌─ Week Navigation ─────────────────────────┐   │
│ Side  │ │ Mon | Tue | Wed | Thu | Fri | Sat | Sun  │   │
│ bar   │ ├─────────────────────────────────────────┤   │
│ 224px │ │ [Time]  [Events displayed here...]      │   │ 60px/hour
│       │ │         Scrollable calendar area         │   │
│ AI    │ │                                          │   │
│ Chat  │ │                                          │   │
│       │ └──────────────────────────────────────────┘   │
│ Forms │                                                 │
└───────┴─────────────────────────────────────────────────┘
```

### Phone View (≤768px)
```
┌─────────────────────────────┐
│ [User Icon]           [⚲]   │ 44px topbar
├─────────────────────────────┤
│ [⌘ Terminal] [Week Badge]   │ 40px toolbar
├─────────────────────────────┤
│ ┌─ Week Navigation ───────┐ │
│ │ Mo Tu We Th Fr Sa Su    │ │
│ ├─────────────────────────┤ │
│ │[T] [Events here...]     │ │ 50px/hour
│ │    ← Swipe to navigate→ │ │ 42px ruler
│ │    Tap events to view   │ │
│ │                         │ │
│ │    Scrollable area      │ │
│ │                         │ │
│ └─────────────────────────┘ │
│                             │
│   [+]                 [☰]   │ Floating buttons
└─────────────────────────────┘
   Green                Purple
```

## Feature Comparison Table

| Feature | Desktop | Phone |
|---------|---------|-------|
| **Navigation** | Prev/Next buttons | Swipe gestures + buttons |
| **New Event** | Toolbar button | Green floating + button |
| **Menu Access** | Always visible sidebar | Purple floating ☰ button |
| **Terminal** | Toolbar button | Toolbar ⌘ button |
| **Event Interaction** | Click | Tap with haptic feedback |
| **Week Navigation** | Button clicks | Swipe left/right |
| **Sidebar** | 224px fixed | 280px slide-in overlay |
| **Time Ruler** | 56px | 42px |
| **Hour Height** | 60px | 50px |
| **Touch Targets** | 36px+ | 44px+ minimum |
| **AI Chat** | 280px height | 180px height |
| **Modal Width** | 430px | 95vw (full width) |

## Size Optimizations

### Topbar
```
Desktop:  ████████████████████████████ 52px
Phone:    ██████████████████████ 44px
```

### Time Ruler Width
```
Desktop:  ████████████ 56px
Phone:    ██████████ 42px
```

### Hour Height
```
Desktop:  ████████████████ 60px
Phone:    ██████████████ 50px
```

### Sidebar Width
```
Desktop:  ████████████████████ 224px
Phone:    ██████████████████████████ 280px
```

## Button Placement

### Desktop
```
Top Bar: [Create] [Terminal]
Sidebar: Always visible with AI chat and forms
```

### Phone
```
Toolbar: [⌘ Terminal] [Week Badge]
Floating:
  - Bottom Left: [+] Create event
  - Bottom Right: [☰] Menu
```

## Responsive Breakpoints

| Breakpoint | Layout |
|------------|--------|
| 0-768px | Phone mode (portrait) |
| 769-915px (landscape) | Phone mode (landscape) |
| 769-1024px | Tablet mode |
| 1025px+ | Desktop mode |

## Space Efficiency

### Desktop Calendar Area
- Sidebar: 224px (left)
- Calendar: Remaining width
- Total ruler: 56px

### Phone Calendar Area
- Sidebar: Hidden (slide-in)
- Calendar: Full width
- Total ruler: 42px
- **Result**: ~30px more event space per column

## Gesture Support

### Desktop
- Mouse clicks
- Hover effects
- Keyboard shortcuts

### Phone
- Tap (with haptic feedback)
- Swipe left/right (week navigation)
- Long press (future feature)
- Pinch-to-zoom (future feature)

## Modal Behavior

### Desktop
- 430px fixed width
- Centered on screen
- Backdrop blur

### Phone
- 95vw width (near full-width)
- Centered with margins
- Backdrop blur
- 16px font (prevents iOS zoom)
- Maximum 90vh height

## AI Chat Comparison

### Desktop
```
┌─────────────┐
│ AI Chat     │
│ 280px       │
│ height      │
│             │
│ [Messages]  │
│             │
│ [Input]     │
└─────────────┘
```

### Phone
```
┌─────────┐
│ AI Chat │
│ 180px   │
│ height  │
│ [Msgs]  │
│ [Input] │
└─────────┘
```

## Terminal Display

### Desktop
```
Terminal at bottom:
┌────────────────────────────────┐
│ user@calang:~$ command         │
│ Output                         │
│ More output                    │
│ [Input]                        │
└────────────────────────────────┘
```

### Phone (60vh)
```
Terminal (larger):
┌──────────────────┐
│ user@calang:~$   │
│ command          │
│ Output           │
│ More output      │
│ [Input]          │
│ Safe area        │
└──────────────────┘
```

## Performance Metrics

| Metric | Desktop | Phone |
|--------|---------|-------|
| Layout shift | Minimal | Zero |
| Touch response | N/A | <16ms |
| Scroll FPS | 60fps | 60fps |
| Gesture recognition | N/A | <50ms |
| Animation duration | 200ms | 300ms |

## Accessibility

### Desktop
- Full keyboard navigation
- Mouse hover states
- Click targets 36px+

### Phone
- Touch targets 44px+
- Haptic feedback
- Large tap areas
- No hover required
- Voice input ready (future)

## Battery/Resource Usage

### Desktop
- Full animations
- Hover effects
- Larger canvas

### Phone
- Optimized animations
- Passive event listeners
- Hardware acceleration
- Smaller render area

---

## Summary

**Desktop**: Full-featured traditional UI with always-visible sidebar
**Phone**: Optimized touch-first UI with gestures and floating actions

Both modes share the same core functionality but optimized for their respective input methods and screen sizes.
