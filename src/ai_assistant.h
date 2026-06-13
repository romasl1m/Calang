#pragma once
#include <string>

// Process natural language command using Gemini API
std::string process_ai_command(const std::string &userInput, const std::string &currentUsername, std::string &currentgroup);

// Get Gemini API key from environment or config
std::string get_gemini_api_key();
