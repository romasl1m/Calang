# Phone UI Mode Features

The Calang app now includes comprehensive phone/mobile UI optimizations for a better experience on smartphones.

## Key Phone Features

### 1. **Floating Action Buttons**
- **Quick Add Button (Left)**: Green floating button to quickly create new events
- **Menu Button (Right)**: Purple floating button to open the sidebar menu
- Both buttons include haptic feedback on supported devices

### 2. **Swipe Navigation**
- **Swipe Left**: Navigate to next week
- **Swipe Right**: Navigate to previous week
- Swipe gestures work on the calendar background (not on event cards)

### 3. **Optimized Layout**
- **Compact topbar**: Reduced height (44px) to maximize calendar space
- **Smaller time ruler**: 42px width instead of 56px for more event space
- **Touch-friendly events**: Minimum 38px height for easy tapping
- **Larger tap targets**: All interactive elements are at least 44px tall

### 4. **Responsive Calendar**
- **Reduced hour height**: 50px per hour on phone (60px on desktop)
- **Compact day headers**: Smaller fonts but still readable
- **All-day events**: Optimized strip with proper spacing

### 5. **Mobile Terminal**
- **Compact command button**: Replaces desktop terminal button with ⌘ icon
- **60% viewport height**: Terminal takes up more screen on mobile
- **Touch keyboard optimized**: Input fields sized to prevent iOS zoom

### 6. **Enhanced Modals**
- **Full-width forms**: 95% viewport width for better input experience
- **Maximum 90vh**: Scrollable content that fits on screen
- **16px font size**: Prevents automatic zoom on iOS form fields

### 7. **Sidebar Improvements**
- **Wider on mobile**: 280px instead of 224px for better touch targets
- **Full overlay**: Dark backdrop when menu is open
- **Smooth animations**: 300ms transition for opening/closing

### 8. **AI Chat Optimization**
- **Reduced height**: 180px on phone (280px on desktop)
- **Auto-hidden on small screens**: Hidden when viewport height < 700px
- **Touch-optimized input**: Easy to type and send messages

### 9. **Landscape Mode Support**
- **Compact layout**: Reduced heights for landscape orientation
- **Hidden quick-add**: More screen space in landscape
- **Smaller floating buttons**: 44px instead of 56px

### 10. **iPhone Notch Support**
- **Safe area insets**: Buttons respect iPhone notch and home indicator
- **Bottom padding**: Terminal includes safe area spacing

## Gestures Summary

| Gesture | Action |
|---------|--------|
| Tap event | Open event details (with haptic feedback) |
| Swipe left | Next week |
| Swipe right | Previous week |
| Tap green + | Create new event |
| Tap purple ☰ | Open sidebar menu |
| Tap ⌘ | Toggle terminal |

## Browser Support

- **iOS Safari**: Full support including safe areas
- **Chrome Mobile**: Full support including vibration feedback
- **Android browsers**: Full support
- **PWA mode**: Works great as a progressive web app

## Performance Optimizations

- **Hardware acceleration**: Smooth animations on modern devices
- **Touch scrolling**: Momentum scrolling on iOS with `-webkit-overflow-scrolling: touch`
- **Tap highlight**: Custom colors that match the app theme
- **Event delegation**: Efficient event handling for calendar events

## Usage Tips

1. **Create events faster**: Use the green + button instead of opening the menu
2. **Navigate weeks quickly**: Swipe left/right on the calendar
3. **Landscape mode**: Rotate device for more compact time view
4. **Terminal access**: Use the ⌘ button or swipe down for quick commands
5. **Full-screen sidebar**: Tap anywhere outside to close the menu

## Future Enhancements (Planned)

- Pull-to-refresh calendar
- Long-press event for quick actions
- Pinch-to-zoom time scale
- Shake-to-undo gesture
- Voice input for event creation
- Dark mode toggle in mobile menu

---

**Note**: All phone UI features are automatically enabled when viewport width is ≤768px. The app seamlessly switches between desktop and mobile modes based on screen size.
