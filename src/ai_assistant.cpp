#include "ai_assistant.h"
#include "terminal.h"
#include "functions.h"
#include <iostream>
#include <sstream>
#include <cstdlib>
#include <fstream>
#include <nlohmann/json.hpp>
#include <curl/curl.h>

using namespace std;
using json = nlohmann::json;

// Callback for curl to write response data
static size_t WriteCallback(void *contents, size_t size, size_t nmemb, void *userp) {
    ((string *)userp)->append((char *)contents, size * nmemb);
    return size * nmemb;
}

string get_gemini_api_key() {
    // Try environment variable first
    const char *env_key = getenv("GEMINI_API_KEY");
    if (env_key) {
        return string(env_key);
    }

    // Try reading from config file
    ifstream config_file(".gemini_config");
    if (config_file.is_open()) {
        string key;
        getline(config_file, key);
        config_file.close();
        if (!key.empty()) {
            return key;
        }
    }

    return "";
}

string call_gemini_api(const string &prompt, const string &api_key) {
    CURL *curl;
    CURLcode res;
    string readBuffer;

    curl = curl_easy_init();
    if (!curl) {
        return "Error: Failed to initialize CURL";
    }

    // Prepare JSON payload
    json payload = {
        {"contents", {{{"parts", {{{"text", prompt}}}}}}}};

    string json_str = payload.dump();
    string url = "https://generativelanguage.googleapis.com/v1beta/models/gemini-pro:generateContent?key=" + api_key;

    struct curl_slist *headers = NULL;
    headers = curl_slist_append(headers, "Content-Type: application/json");

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, json_str.c_str());
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);

    res = curl_easy_perform(curl);

    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);

    if (res != CURLE_OK) {
        return "Error: " + string(curl_easy_strerror(res));
    }

    return readBuffer;
}

string extract_command_from_response(const string &response) {
    try {
        json j = json::parse(response);

        if (j.contains("candidates") && !j["candidates"].empty()) {
            auto candidate = j["candidates"][0];
            if (candidate.contains("content") && candidate["content"].contains("parts")) {
                auto parts = candidate["content"]["parts"];
                if (!parts.empty() && parts[0].contains("text")) {
                    string text = parts[0]["text"];

                    // Extract command between ```bash and ```
                    size_t start = text.find("```bash");
                    if (start == string::npos) {
                        start = text.find("```");
                    }

                    if (start != string::npos) {
                        start = text.find('\n', start) + 1;
                        size_t end = text.find("```", start);
                        if (end != string::npos) {
                            string command = text.substr(start, end - start);
                            // Trim whitespace
                            command.erase(0, command.find_first_not_of(" \t\n\r"));
                            command.erase(command.find_last_not_of(" \t\n\r") + 1);
                            return command;
                        }
                    }

                    // If no code block, return the whole text
                    return text;
                }
            }
        }
    } catch (const exception &e) {
        return "Error parsing AI response: " + string(e.what());
    }

    return "No valid response from AI";
}

string build_ai_prompt(const string &userInput, const string &currentUsername, const string &currentgroup) {
    stringstream prompt;

    prompt << "You are a terminal assistant for a calendar application called 'calang'. ";
    prompt << "Convert the user's natural language request into terminal commands.\n\n";

    prompt << "Current context:\n";
    prompt << "- Username: " << currentUsername << "\n";
    prompt << "- Current group: " << (currentgroup.empty() ? "private" : currentgroup) << "\n\n";

    prompt << "Available commands:\n";
    prompt << "1. cat YYYY-MM-DD - show events on a specific date\n";
    prompt << "2. cat N - show next N upcoming events\n";
    prompt << "3. grep \"text\" - search for events containing text (case-insensitive)\n";
    prompt << "4. touch \"title\" DD.MM HH:MM HH:MM \"description\" - create an event\n";
    prompt << "5. touch \"title\" in DD.MM HH:MM length HH:MM \"description\" - create event with duration\n";
    prompt << "6. cd group_name - change to a group\n";
    prompt << "7. cd group_name/subgroup - navigate to a subgroup\n";
    prompt << "8. ls - list available groups and subgroups\n";
    prompt << "9. rm event_id - delete an event by ID\n";
    prompt << "10. dates - show stored dates from recent searches\n\n";

    prompt << "Additional parameters for touch command:\n";
    prompt << "- P=high|medium|low (priority)\n";
    prompt << "- T=daily|weekly|monthly|yearly (recurrence)\n";
    prompt << "- S=subgroup_path (specify subgroup)\n\n";

    prompt << "Important notes:\n";
    prompt << "- For editing events: First use grep to find the event, then tell user the ID to use with rm or manual edit\n";
    prompt << "- For deleting: First grep to find event ID, then use rm command\n";
    prompt << "- Use cat with date format YYYY-MM-DD (convert dates like 'tomorrow' to actual dates)\n";
    prompt << "- When creating events, if duration is mentioned (e.g., 'for 1 hour'), use the 'in...length' format\n\n";

    prompt << "User request: " << userInput << "\n\n";
    prompt << "Respond ONLY with the command(s) to execute, wrapped in ```bash code blocks.\n";
    prompt << "If multiple commands are needed, put each on a new line.\n";
    prompt << "Do NOT include explanations outside the code block.\n";

    return prompt.str();
}

string process_ai_command(const string &userInput, const string &currentUsername, string &currentgroup) {
    string api_key = get_gemini_api_key();

    if (api_key.empty()) {
        return "Error: Gemini API key not found.\n"
               "Set GEMINI_API_KEY environment variable or create .gemini_config file with your API key.\n"
               "Get your API key from: https://makersuite.google.com/app/apikey\n";
    }

    stringstream output;
    output << "AI: Processing request...\n";

    // Build the prompt
    string prompt = build_ai_prompt(userInput, currentUsername, currentgroup);

    // Call Gemini API
    string response = call_gemini_api(prompt, api_key);

    // Extract command from response
    string command = extract_command_from_response(response);

    output << "AI: Executing: " << command << "\n\n";

    // Split multiple commands and execute them
    stringstream commandStream(command);
    string line;

    while (getline(commandStream, line)) {
        if (line.empty() || line[0] == '#')
            continue;

        // Process each command
        string result = process_terminal_command(line, currentUsername, currentgroup);
        output << result;

        if (result.find("CLEAR_SIGNAL") != string::npos) {
            return "CLEAR_SIGNAL";
        }
    }

    return output.str();
}
