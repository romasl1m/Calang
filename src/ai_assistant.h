#pragma once
#include <string>

// Process natural language command using configured AI (Gemini or local model)
std::string process_ai_command(const std::string &userInput, const std::string &currentUsername, std::string &currentgroup);

// Get Gemini API key from environment or config
std::string get_gemini_api_key();

// Get AI model configuration (gemini or qwen)
std::string get_ai_model_type();

// Call local Qwen model
std::string call_local_qwen(const std::string &prompt);
