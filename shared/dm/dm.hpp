#ifndef DM_CONVERSATION
#define DM_CONVERSATION

#include "string"

struct DMConversation  {
    std::string otherUser;
    std::string lastMessagePreview;
    std::string lastTimestamp;
};

#endif 