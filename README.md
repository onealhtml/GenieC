# GenieC - Gemini Client in C
An intelligent assistant in C language that integrates Google Gemini AI API with advanced chat features and real-time weather information.

## 📋 Features

### 🤖 AI Assistant
- Colorful and intuitive command-line interface
- Communication with Google Gemini API using cURL
- **Conversation history**: Maintains context between questions
- **Real-time search**: Google Search integration via Gemini API
- Custom system prompt for responses in English

### 🌤️ Weather Information
- **Real-time weather data** via OpenWeather API
- Display of temperature and weather conditions for user's city
- Automatic encoding of city names for URLs

### 💬 Interactive Commands
- `clear` - Clears conversation history and restarts
- `history` - Displays complete current conversation history
- `help` - Shows detailed help and usage tips
- `0` - Exit the program

### 🎨 Advanced Interface
- Colorful GenieC ASCII art
- Different colors for better user experience
- Loading animation during AI queries
- Main menu with weather information

## 🛠️ Prerequisites

- **C Compiler**: 
  - MinGW-w64
  - GCC
  - Visual Studio
- **CMake** (version 3.21 or higher)
- **Package Manager**:
  - MSYS2/pacman
  - vcpkg
- **Libraries**:
  - cURL (for HTTP requests)
  - cJSON (for JSON parsing)

## 🔑 API Keys Configuration

**IMPORTANT**: To use this project, you need to configure two API keys.

### 1. Google Gemini API Key
1. Access [Google AI Studio](https://aistudio.google.com/app/apikey)
2. Log in with your Google account
3. Click "Create API Key"
4. Copy the generated key

### 2. OpenWeather API Key
1. Access [OpenWeatherMap](https://openweathermap.org/api)
2. Create a free account
3. Get your API key

### 3. Configure API Keys in the project
Open the `api_key.example.h` file, rename it to `api_key.h` and configure both keys:

```c
#ifndef API_KEY_H
#define API_KEY_H

// Google Gemini API Key
#define API_KEY "YOUR_GEMINI_API_KEY_HERE"

// OpenWeather API Key
#define API_KEY_WEATHER "YOUR_OPENWEATHER_API_KEY_HERE"

#endif
```

## 💻 How to Use

### First Run
1. Run the program
2. Enter your city name to get weather information
3. GenieC will display the main menu with weather data

### Interacting with the Assistant
- **Ask natural questions**: "How to make a chocolate cake?"
- **Use special commands**: Type `help` to see all options
- **Maintain context**: GenieC remembers previous conversations
- **Search current information**: "What's the latest news about technology?"

### Usage example:
```
🌍 Enter your city name to get weather information: Santa Cruz do Sul

╔═════════════════════════════════════════════════════════════════════════════╗
║                ██████╗ ███████╗███╗   ██╗██╗███████╗ ██████╗                ║
║               ██╔════╝ ██╔════╝████╗  ██║██║██╔════╝██╔════╝                ║
║               ██║  ███╗█████╗  ██╔██╗ ██║██║█████╗  ██║                     ║
║               ██║   ██║██╔══╝  ██║╚██╗██║██║██╔══╝  ██║                     ║
║               ╚██████╔╝███████╗██║ ╚████║██║███████╗╚██████╗                ║
║                ╚═════╝ ╚══════╝╚═╝  ╚═══╝╚═╝╚══════╝ ╚═════╝                ║
╚═════════════════════════════════════════════════════════════════════════════╝

🤖 Welcome to GenieC - Your Intelligent Gemini Assistant! 🤖

🌤️ Current weather in Santa Cruz do Sul: 22.5°C - partly cloudy

You: Who is the current Pope?
GenieC: The current Pope of the Catholic Church is Leo XIV. He was elected on May 8, 2025, succeeding Pope Francis.

You: 0
Terminating the program...
```

## 📁 Project Structure

```
GenieC/
├── GenieC-en.c       # Main code (English version)
├── api_key.h         # API keys configuration file
├── limpar_tela.h     # Screen clearing function
├── dormir.h          # Sleep function
├── CMakeLists.txt    # CMake configuration
├── build/            # Build directory
│   └── GenieC.exe    # Generated executable
└── README.md         # This file
```

## 🔧 Main Functions

### System Core
- `main()`: Main program loop
- `show_initial_art()`: Displays GenieC ASCII art
- `menu_with_weather()`: Shows main menu with weather information
- `show_help()`: Displays detailed help and tips
- `credits()`: Displays project credits

### API Communication
- `make_http_request()`: Performs HTTP requests using cURL
- `create_json_payload_with_history()`: Creates JSON payload with conversation history
- `extract_text_from_response()`: Extracts text from Gemini JSON response
- `get_weather_data()`: Gets weather data via OpenWeather API

### History Management
- `initialize_chat_history()`: Initializes history system
- `add_turn()`: Adds new message to history
- `display_history()`: Shows complete conversation history
- `free_chat_history()`: Frees history memory

### Utilities
- `url_encode()`: Encodes strings for URLs
- `show_loading()`: Displays loading animation
- `WriteMemoryCallback()`: Callback to store HTTP responses

## 🌟 Advanced Features

### Intelligent History System
- Maintains complete conversation context
- Allows references to previous messages
- Automatic memory management
- Configurable turn limit (50 by default)

### Google Search Integration
- Real-time search via Gemini API
- Responses with updated information
- Support for location-specific queries

### Custom System Prompt
- Responses in English
- Specific instructions for international context
- Guidelines for concise and useful responses

## 📄 License

This project is for educational purposes. Use responsibly and respect the terms of use of Google Gemini and OpenWeather APIs.

## 👥 Credits

This project was developed by:
- [Lorenzo Farias](https://github.com/onealhtml)

## ⚠️ Legal Notice

This project uses Google Gemini and OpenWeather APIs. Make sure to comply with the terms of use of both APIs and use them responsibly. The authors are not responsible for improper use of the APIs or possible costs arising from excessive use.
