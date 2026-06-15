# 📋 Phone UI Testing Checklist

## Pre-Testing Setup

- [ ] Build the app: `make`
- [ ] Run the app: `./calang`
- [ ] Open in browser on desktop
- [ ] Enable device emulation (F12 → Device Toolbar)
- [ ] Test on actual phone if available

## 1. Visual Layout (Portrait)

### Viewport ≤768px
- [ ] Floating action buttons appear (green + and purple ☰)
- [ ] Desktop sidebar is hidden
- [ ] Topbar is 44px height
- [ ] Toolbar is compact
- [ ] Calendar takes full width
- [ ] Time ruler is 42px wide
- [ ] Day headers are compact
- [ ] Week badge displays correctly
- [ ] No horizontal scrolling

### Elements Visibility
- [ ] Mobile terminal button (⌘) visible
- [ ] Desktop terminal button hidden
- [ ] Desktop "New event" button hidden
- [ ] Floating buttons don't overlap content
- [ ] All touch targets ≥44px

## 2. Floating Action Buttons

### Green + Button (Bottom Left)
- [ ] Visible and positioned correctly
- [ ] Opens new event modal on tap
- [ ] Smooth press animation
- [ ] Doesn't interfere with calendar scrolling
- [ ] Safe area spacing on iPhone

### Purple ☰ Button (Bottom Right)
- [ ] Visible and positioned correctly
- [ ] Opens sidebar on tap
- [ ] Smooth press animation
- [ ] Changes to ✕ when sidebar open
- [ ] Closes sidebar when tapped again

## 3. Swipe Gestures

### Calendar Swipe
- [ ] Swipe left navigates to next week
- [ ] Swipe right navigates to previous week
- [ ] Minimum 50px swipe distance
- [ ] Ignores vertical scrolling
- [ ] Doesn't trigger on event cards
- [ ] Smooth page transition
- [ ] URL updates with new date
- [ ] Works consistently

### Gesture Edge Cases
- [ ] Short swipes ignored
- [ ] Vertical swipes don't trigger navigation
- [ ] Diagonal swipes handled correctly
- [ ] Rapid swipes work
- [ ] Swipe during scroll doesn't break

## 4. Touch Interactions

### Event Cards
- [ ] Tap opens event detail modal
- [ ] Haptic feedback on tap (if supported)
- [ ] No delay in response
- [ ] Long press doesn't break (for future)
- [ ] Overlapping events tappable
- [ ] All-day events tappable

### Buttons and Controls
- [ ] All buttons respond to first tap
- [ ] No accidental double-taps
- [ ] Active states visible
- [ ] Ripple/press effects work
- [ ] Form inputs focusable

## 5. Modal Behavior

### New Event Modal
- [ ] Opens from green + button
- [ ] Full width (95vw)
- [ ] Scrollable content
- [ ] Form inputs don't zoom (16px font)
- [ ] Date picker works on mobile
- [ ] Time picker works on mobile
- [ ] Save button accessible
- [ ] Close on backdrop tap
- [ ] Close on cancel button

### Event Detail Modal
- [ ] Opens from event tap
- [ ] Full width (95vw)
- [ ] Scrollable content
- [ ] Edit mode works
- [ ] Delete button accessible
- [ ] Close on backdrop tap

## 6. Sidebar (Mobile Menu)

### Opening/Closing
- [ ] Opens with purple ☰ button
- [ ] Smooth slide-in animation (300ms)
- [ ] Dark overlay appears
- [ ] Close on overlay tap
- [ ] Close on button tap (becomes ✕)
- [ ] Width is 280px
- [ ] Covers full height

### Sidebar Content
- [ ] All menu items visible
- [ ] Group tree accessible
- [ ] AI chat functional
- [ ] Forms usable
- [ ] Scrollable if content overflows
- [ ] All buttons tappable

## 7. Terminal

### Access
- [ ] Toolbar ⌘ button visible
- [ ] Opens terminal on tap
- [ ] Takes 60vh height
- [ ] Positioned at bottom
- [ ] Safe area padding on iPhone

### Functionality
- [ ] Input field accessible
- [ ] Commands execute
- [ ] Output displays correctly
- [ ] Scrollable output
- [ ] Close button works
- [ ] Keyboard appears properly

## 8. AI Chat

### Display
- [ ] Height is 180px on phone
- [ ] Visible in sidebar
- [ ] Hidden if viewport height <700px
- [ ] Scrollable messages
- [ ] Input field accessible

### Functionality
- [ ] Send button works
- [ ] Enter key sends
- [ ] Messages display
- [ ] Auto-scrolls to new messages
- [ ] Loading state shows

## 9. Landscape Mode (Width ≤915px)

### Layout Changes
- [ ] Quick-add button hidden
- [ ] Menu button smaller (44px)
- [ ] Topbar height reduced to 40px
- [ ] Toolbar height reduced to 40px
- [ ] Hour height reduced to 40px
- [ ] Day headers more compact
- [ ] Calendar readable

### Functionality
- [ ] All features still work
- [ ] Swipe gestures work
- [ ] Modals fit properly
- [ ] Sidebar still accessible
- [ ] Terminal still usable

## 10. iPhone Specific

### Safe Areas
- [ ] Buttons respect notch area
- [ ] Bottom buttons above home indicator
- [ ] Terminal has bottom padding
- [ ] No UI hidden by notch
- [ ] Full-screen mode works

### iOS Safari
- [ ] No zoom on input focus
- [ ] Smooth scrolling
- [ ] Haptic feedback works
- [ ] PWA mode functional
- [ ] Status bar styled correctly

## 11. Performance

### Animations
- [ ] Smooth transitions (60fps)
- [ ] No jank on swipe
- [ ] Button press instant
- [ ] Modal open/close smooth
- [ ] Sidebar slide smooth

### Responsiveness
- [ ] Tap response <16ms
- [ ] Gesture recognition <50ms
- [ ] No input lag
- [ ] Scroll momentum works
- [ ] No memory leaks

## 12. Edge Cases

### Small Screens (<375px width)
- [ ] Layout doesn't break
- [ ] Buttons still accessible
- [ ] Text readable
- [ ] Modals fit

### Very Tall Screens
- [ ] Calendar scrolls properly
- [ ] Buttons stay fixed
- [ ] No content cutoff

### No JavaScript
- [ ] Graceful degradation
- [ ] Core functionality hint
- [ ] No console errors

### Slow Connection
- [ ] Loading states show
- [ ] No broken UI
- [ ] Buttons disabled when needed

## 13. Cross-Browser Testing

### Chrome Mobile
- [ ] All features work
- [ ] Vibration API works
- [ ] Smooth rendering

### Firefox Mobile
- [ ] All features work
- [ ] Fallbacks working
- [ ] Smooth rendering

### Safari iOS
- [ ] All features work
- [ ] Safe areas respected
- [ ] Smooth rendering

### Samsung Internet
- [ ] All features work
- [ ] Fallbacks working
- [ ] Smooth rendering

## 14. Accessibility

### Touch Targets
- [ ] All buttons ≥44x44px
- [ ] Easy to tap accurately
- [ ] No tiny targets
- [ ] Proper spacing

### Visual Feedback
- [ ] Active states clear
- [ ] Loading states visible
- [ ] Error states obvious
- [ ] Success confirmations

### Text
- [ ] Readable font sizes
- [ ] Sufficient contrast
- [ ] No text cutoff
- [ ] Labels clear

## 15. Desktop Compatibility

### Viewport >768px
- [ ] Floating buttons hidden
- [ ] Desktop sidebar visible
- [ ] Original layout intact
- [ ] No mobile styles applied
- [ ] Swipe gestures disabled
- [ ] All desktop features work

## Bug Reporting Template

```
**Bug**: [Brief description]
**Device**: [iPhone 13, Pixel 6, etc.]
**Browser**: [Safari 15, Chrome 96, etc.]
**Viewport**: [375x667, etc.]
**Orientation**: [Portrait/Landscape]
**Steps**:
1. [Step 1]
2. [Step 2]
**Expected**: [What should happen]
**Actual**: [What happened]
**Screenshot**: [If applicable]
```

---

## Testing Sign-Off

- [ ] All critical features tested
- [ ] All blockers resolved
- [ ] Performance acceptable
- [ ] Ready for production

**Tester**: _______________
**Date**: _______________
**Browser/Device**: _______________
**Pass/Fail**: _______________

---

## Quick Test (5 minutes)

For rapid verification:
1. [ ] Resize browser to phone width
2. [ ] See floating buttons
3. [ ] Tap green + → creates event
4. [ ] Swipe left → next week
5. [ ] Tap purple ☰ → opens menu
6. [ ] All looks good visually

**Status**: ⬜ Pass ⬜ Fail ⬜ Needs work
