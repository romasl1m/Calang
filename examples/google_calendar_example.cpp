/**
 * Google Calendar Integration Example
 *
 * This example demonstrates how to use the Google Calendar API integration
 * in the Calang application.
 */

#include <iostream>
#include <string>
#include "../src/google_oauth.h"

using namespace std;

int main() {
    // Example 1: Create a simple event
    cout << "=== Example 1: Create Simple Event ===" << endl;

    string access_token = "ya29.a0..."; // Your actual access token
    string title = "Team Meeting";
    string start = "2026-06-13 14:00";
    string end = "2026-06-13 15:00";
    string description = "Discuss Q3 roadmap and sprint planning";

    string response = createGoogleCalendarEvent(
        access_token,
        title,
        start,
        end,
        description
    );

    cout << "Response: " << response << endl << endl;


    // Example 2: Create an all-day event
    cout << "=== Example 2: All-Day Event ===" << endl;

    response = createGoogleCalendarEvent(
        access_token,
        "Company Holiday",
        "2026-12-25 00:00",
        "2026-12-25 23:59",
        "Christmas Day - Office Closed"
    );

    cout << "Response: " << response << endl << endl;


    // Example 3: Create a short meeting
    cout << "=== Example 3: Short Meeting (30 min) ===" << endl;

    response = createGoogleCalendarEvent(
        access_token,
        "Quick Standup",
        "2026-06-14 09:00",
        "2026-06-14 09:30",
        "Daily standup - 3 questions: What did you do? What will you do? Any blockers?"
    );

    cout << "Response: " << response << endl << endl;


    // Example 4: Get user's access token
    cout << "=== Example 4: Retrieve Stored Token ===" << endl;

    string user_email = "user@example.com";
    string stored_token = get_user_access_token(user_email);

    if (!stored_token.empty()) {
        cout << "Found token for " << user_email << endl;
        cout << "Token: " << stored_token.substr(0, 20) << "..." << endl;
    } else {
        cout << "No token found for " << user_email << endl;
        cout << "User needs to login via Google OAuth first." << endl;
    }


    // Example 5: Error handling
    cout << "\n=== Example 5: Error Handling ===" << endl;

    response = createGoogleCalendarEvent(
        "invalid_token",
        "Test Event",
        "2026-06-15 10:00",
        "2026-06-15 11:00",
        "This should fail due to invalid token"
    );

    if (response.empty()) {
        cout << "Request failed (empty response)" << endl;
    } else {
        cout << "Response: " << response << endl;
    }

    return 0;
}

/**
 * Expected Output:
 *
 * === Example 1: Create Simple Event ===
 * Response: {
 *   "kind": "calendar#event",
 *   "id": "abc123",
 *   "status": "confirmed",
 *   "summary": "Team Meeting",
 *   "description": "Discuss Q3 roadmap and sprint planning",
 *   "start": {
 *     "dateTime": "2026-06-13T14:00:00Z",
 *     "timeZone": "UTC"
 *   },
 *   "end": {
 *     "dateTime": "2026-06-13T15:00:00Z",
 *     "timeZone": "UTC"
 *   }
 * }
 *
 * === Example 2: All-Day Event ===
 * Response: {...}
 *
 * === Example 3: Short Meeting (30 min) ===
 * Response: {...}
 *
 * === Example 4: Retrieve Stored Token ===
 * Found token for user@example.com
 * Token: ya29.a0AfH6SMBj1234...
 *
 * === Example 5: Error Handling ===
 * Response: {
 *   "error": {
 *     "code": 401,
 *     "message": "Invalid Credentials",
 *     "errors": [...]
 *   }
 * }
 */

/**
 * Integration with Terminal Commands:
 *
 * When a user runs a touch command like:
 *   touch "Meeting" 13.06 14:00 15:00 "Discuss project"
 *
 * The system will:
 * 1. Parse the command arguments
 * 2. Create the event locally in Calang database
 * 3. Check if user has Google access token
 * 4. If yes, call createGoogleCalendarEvent()
 * 5. Display success/error message to user
 *
 * Code flow:
 *   terminal.cpp:process_terminal_command()
 *   -> functions.cpp:add_new_event()
 *   -> google_oauth.cpp:get_user_access_token()
 *   -> google_oauth.cpp:createGoogleCalendarEvent()
 */

/**
 * Date/Time Format Conversion:
 *
 * Input format (Calang):     "2026-06-13 14:00"
 * Output format (Google):    "2026-06-13T14:00:00Z"
 *
 * Conversion happens in createGoogleCalendarEvent():
 * 1. Replace space with 'T'
 * 2. Add seconds ":00"
 * 3. Add timezone "Z" (UTC)
 */

/**
 * Token Storage Format:
 *
 * File: users/{email}/google_token.json
 *
 * {
 *   "access_token": "ya29.a0AfH6SMBj...",
 *   "refresh_token": "1//0gLxYz...",
 *   "expires_in": 3600,
 *   "timestamp": 1718294400
 * }
 *
 * Note: Currently only access_token is used.
 * Future: Implement automatic refresh using refresh_token.
 */

/**
 * API Endpoint Details:
 *
 * URL: https://www.googleapis.com/calendar/v3/calendars/primary/events
 * Method: POST
 * Headers:
 *   - Content-Type: application/json
 *   - Authorization: Bearer {access_token}
 *
 * Request Body:
 * {
 *   "summary": "Event title",
 *   "description": "Event description",
 *   "start": {
 *     "dateTime": "2026-06-13T14:00:00Z",
 *     "timeZone": "UTC"
 *   },
 *   "end": {
 *     "dateTime": "2026-06-13T15:00:00Z",
 *     "timeZone": "UTC"
 *   }
 * }
 */

/**
 * Testing Checklist:
 *
 * [ ] Set up Google Cloud Console project
 * [ ] Enable Google Calendar API
 * [ ] Create OAuth 2.0 credentials
 * [ ] Set environment variables (CLIENT_ID, CLIENT_SECRET, REDIRECT_URI)
 * [ ] Build and run application
 * [ ] Login via Google OAuth
 * [ ] Verify token file created in users/{email}/google_token.json
 * [ ] Create event via terminal (touch command)
 * [ ] Verify event appears in local Calang database
 * [ ] Verify event appears in Google Calendar (calendar.google.com)
 * [ ] Create event via web interface
 * [ ] Verify sync works for web-created events too
 * [ ] Test error handling with invalid token
 * [ ] Test with user who hasn't logged in via Google
 */
