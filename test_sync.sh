#!/bin/bash

# Test script for Google Calendar Sync
# This script tests the sync functionality

echo "=== Google Calendar Sync Test ==="
echo ""

# Check if calang is built
if [ ! -f "./build/calang" ]; then
    echo "❌ Error: calang executable not found"
    echo "Run: cmake --build build"
    exit 1
fi

echo "✓ calang executable found"
echo ""

# Check if required env vars are set
if [ -z "$GOOGLE_CLIENT_ID" ] || [ -z "$GOOGLE_CLIENT_SECRET" ]; then
    echo "⚠ Warning: Google OAuth environment variables not set"
    echo "Required: GOOGLE_CLIENT_ID, GOOGLE_CLIENT_SECRET, GOOGLE_REDIRECT_URI"
    echo ""
fi

# Test 1: Check that sync command is in help
echo "Test 1: Checking if 'sync' command is in help..."
echo "help" | timeout 2 ./build/calang 2>/dev/null | grep -q "sync - import events"
if [ $? -eq 0 ]; then
    echo "✓ 'sync' command found in help"
else
    echo "❌ 'sync' command NOT found in help"
fi
echo ""

# Test 2: Check header files
echo "Test 2: Checking header declarations..."
if grep -q "syncGoogleEvents" src/google_oauth.h; then
    echo "✓ syncGoogleEvents declared in google_oauth.h"
else
    echo "❌ syncGoogleEvents NOT found in header"
fi
echo ""

# Test 3: Check implementation exists
echo "Test 3: Checking implementation..."
if grep -q "int syncGoogleEvents" src/google_oauth.cpp; then
    echo "✓ syncGoogleEvents implemented in google_oauth.cpp"
else
    echo "❌ syncGoogleEvents NOT implemented"
fi
echo ""

# Test 4: Check terminal command
echo "Test 4: Checking terminal command handler..."
if grep -q '"sync"' src/terminal.cpp; then
    echo "✓ sync command handler found in terminal.cpp"
else
    echo "❌ sync command handler NOT found"
fi
echo ""

# Test 5: Check API endpoint
echo "Test 5: Checking API endpoint..."
if grep -q "/api/sync_google" src/api.cpp; then
    echo "✓ /api/sync_google endpoint found in api.cpp"
else
    echo "❌ API endpoint NOT found"
fi
echo ""

# Test 6: Check Event struct has google_id
echo "Test 6: Checking Event struct..."
if grep -q "google_id" src/event.h; then
    echo "✓ google_id field found in Event struct"
else
    echo "❌ google_id field NOT found"
fi
echo ""

# Test 7: Check helper functions
echo "Test 7: Checking helper functions..."
helpers_found=0
if grep -q "fetchGoogleCalendarEvents" src/google_oauth.cpp; then
    echo "  ✓ fetchGoogleCalendarEvents"
    ((helpers_found++))
fi
if grep -q "convertRFC3339ToLocal" src/google_oauth.cpp; then
    echo "  ✓ convertRFC3339ToLocal"
    ((helpers_found++))
fi
if grep -q "eventExistsByGoogleId" src/google_oauth.cpp; then
    echo "  ✓ eventExistsByGoogleId"
    ((helpers_found++))
fi

if [ $helpers_found -eq 3 ]; then
    echo "✓ All helper functions found"
else
    echo "⚠ Only $helpers_found/3 helper functions found"
fi
echo ""

# Test 8: Check build success
echo "Test 8: Verifying build..."
cd /home/roman/calang
cmake --build build >/dev/null 2>&1
if [ $? -eq 0 ]; then
    echo "✓ Project builds successfully"
else
    echo "❌ Build failed"
    echo "Run: cmake --build build"
fi
echo ""

# Summary
echo "=== Test Summary ==="
echo "Basic integration tests completed."
echo ""
echo "To test sync functionality:"
echo "1. Start calang: ./build/calang"
echo "2. Login with Google OAuth: http://localhost:8080/auth/google/login"
echo "3. Open terminal and run: sync"
echo ""
echo "Or test API endpoint:"
echo "curl -X POST http://localhost:8080/api/sync_google -b 'session_id=YOUR_SESSION'"
echo ""
