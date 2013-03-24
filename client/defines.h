#pragma once

#define CHAT_LOG_MESSAGES_PER_PAGE 20

// Any kind of loggin info
#define LOG_NETWORK_REQUEST
#define LOG_NETWORK_RESPOND_PURE
//#define LOG_NETWORK_RESPOND_TRIM
#define LOG_NETWORK_RESPOND_ERROR

// LOGS
#define LOG_CUMM_PATH "logs"
#define LOG_NETWORK_FILENAME "network.log"
#define LOG_SQLITE_FILENAME "sqlite.log"
#define LOG_XML_FILENAME "xml.log"

// Language
#define LANGUAGE_DIR QString("languages")
#define LANGUAGE_FILE_EXTENSION QString("lng")

// Stickers
#define STICKER_UNREAD_MARK_BOLD
//#define STICKER_UNREAD_MARK_ICON


// Perform fast login with demo/demo params
//#define FAST_LOGIN_DEMO_DEMO

// Fast login URL
//#define FAST_LOGIN_URL "http://192.168.0.95/cs/"
#define FAST_LOGIN_URL "http://192.168.0.90/cs/"

