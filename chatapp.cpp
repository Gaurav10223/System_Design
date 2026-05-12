#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <ctime>

using namespace std;

enum class ConversationType {
    DIRECT,
    GROUP
};

enum class MessageStatus {
    SENT,
    DELIVERED,
    READ
};

// I keep the user model small.
// I only store the fields that matter for chat flow.
class User {
private:
    string userId;
    string name;
    bool online;

public:
    User() : online(false) {}

    User(string id, string userName) {
        userId = id;
        name = userName;
        online = false;
    }

    string getUserId() const {
        return userId;
    }

    string getName() const {
        return name;
    }

    bool isOnline() const {
        return online;
    }

    void setOnline(bool status) {
        online = status;
    }
};

// I keep the message object focused.
// It should carry only the data needed for storage and delivery.
class Message {
private:
    string messageId;
    string conversationId;
    string senderId;
    string content;
    long timestamp;
    MessageStatus status;

public:
    Message() : timestamp(0), status(MessageStatus::SENT) {}

    Message(string msgId, string convId, string sender, string text, long timeStampValue) {
        messageId = msgId;
        conversationId = convId;
        senderId = sender;
        content = text;
        timestamp = timeStampValue;
        status = MessageStatus::SENT;
    }

    string getMessageId() const {
        return messageId;
    }

    string getConversationId() const {
        return conversationId;
    }

    string getSenderId() const {
        return senderId;
    }

    string getContent() const {
        return content;
    }

    long getTimestamp() const {
        return timestamp;
    }

    MessageStatus getStatus() const {
        return status;
    }

    void setStatus(MessageStatus newStatus) {
        status = newStatus;
    }
};

// I model the conversation as the boundary for ordering.
// That keeps the message timeline simple and consistent.
class Conversation {
private:
    string conversationId;
    ConversationType type;
    vector<string> participantIds;
    vector<Message> messages;

public:
    Conversation() : type(ConversationType::DIRECT) {}

    Conversation(string id, ConversationType conversationType, vector<string> participants) {
        conversationId = id;
        type = conversationType;
        participantIds = participants;
    }

    string getConversationId() const {
        return conversationId;
    }

    ConversationType getType() const {
        return type;
    }

    vector<string> getParticipants() const {
        return participantIds;
    }

    bool hasParticipant(const string& userId) const {
        for (const string& participant : participantIds) {
            if (participant == userId) {
                return true;
            }
        }
        return false;
    }

    void addMessage(const Message& message) {
        messages.push_back(message);
    }

    vector<Message> getMessages() const {
        return messages;
    }
};

// I use a listener so the chat flow can notify other parts cleanly.
// That keeps the core service open for extension.
class ChatEventListener {
public:
    virtual void onNewMessage(const Message& message) = 0;
    virtual void onDeliveryUpdate(const Message& message) = 0;
    virtual ~ChatEventListener() {}
};

// I keep message storage simple for the interview version.
// In production, this would become a database-backed store.
class MessageStore {
private:
    map<string, vector<Message> > conversationMessages;

public:
    void saveMessage(const Message& message) {
        conversationMessages[message.getConversationId()].push_back(message);
    }

    vector<Message> getMessages(const string& conversationId) {
        if (conversationMessages.find(conversationId) == conversationMessages.end()) {
            return vector<Message>();
        }
        return conversationMessages[conversationId];
    }
};

// I keep conversation storage separate from messages.
// That gives me cleaner ownership and better extensibility.
class ConversationStore {
private:
    map<string, Conversation> conversations;

public:
    void saveConversation(const Conversation& conversation) {
        conversations[conversation.getConversationId()] = conversation;
    }

    Conversation* getConversation(const string& conversationId) {
        if (conversations.find(conversationId) == conversations.end()) {
            return NULL;
        }
        return &conversations[conversationId];
    }
};

// I keep delivery logic separate from chat orchestration.
// That makes it easier to swap transport later.
class DeliveryService {
public:
    void deliver(const Message& message, const Conversation& conversation) {
        vector<string> participants = conversation.getParticipants();

        cout << "Delivering message '" << message.getMessageId() << "' to participants: ";
        for (int i = 0; i < (int)participants.size(); i++) {
            if (participants[i] != message.getSenderId()) {
                cout << participants[i];
                if (i != (int)participants.size() - 1) {
                    cout << ", ";
                }
            }
        }
        cout << endl;
    }
};

// I keep the main service as the orchestrator.
// It coordinates validation, persistence, and delivery.
class ChatService {
private:
    map<string, User> users;
    MessageStore messageStore;
    ConversationStore conversationStore;
    DeliveryService deliveryService;
    vector<ChatEventListener*> listeners;
    int conversationCounter;
    int messageCounter;

    string generateConversationId() {
        conversationCounter++;
        return "conv_" + to_string(conversationCounter);
    }

    string generateMessageId() {
        messageCounter++;
        return "msg_" + to_string(messageCounter);
    }

    long currentTimestamp() {
        return (long)time(NULL);
    }

public:
    ChatService() {
        conversationCounter = 0;
        messageCounter = 0;
    }

    void addUser(const User& user) {
        users[user.getUserId()] = user;
    }

    bool setUserOnline(const string& userId, bool status) {
        if (users.find(userId) == users.end()) {
            return false;
        }
        users[userId].setOnline(status);
        return true;
    }

    void registerListener(ChatEventListener* listener) {
        listeners.push_back(listener);
    }

    // I create the conversation first.
    // That gives me a stable container for all future messages.
    string createConversation(ConversationType type, vector<string> participants) {
        for (int i = 0; i < (int)participants.size(); i++) {
            if (users.find(participants[i]) == users.end()) {
                cout << "Invalid participant: " << participants[i] << endl;
                return "";
            }
        }

        string conversationId = generateConversationId();
        Conversation conversation(conversationId, type, participants);
        conversationStore.saveConversation(conversation);

        cout << "Conversation created: " << conversationId << endl;
        return conversationId;
    }

    // I validate the sender and the conversation before storing anything.
    // That keeps bad requests out of the system early.
    bool sendMessage(const string& conversationId, const string& senderId, const string& content) {
        Conversation* conversation = conversationStore.getConversation(conversationId);
        if (conversation == NULL) {
            cout << "Conversation not found." << endl;
            return false;
        }

        if (!conversation->hasParticipant(senderId)) {
            cout << "Sender is not part of the conversation." << endl;
            return false;
        }

        string messageId = generateMessageId();
        Message message(messageId, conversationId, senderId, content, currentTimestamp());

        // I save first so the system does not lose the message.
        // Delivery should always come after durability.
        messageStore.saveMessage(message);
        conversation->addMessage(message);

        // I notify listeners so external concerns stay decoupled.
        for (int i = 0; i < (int)listeners.size(); i++) {
            listeners[i]->onNewMessage(message);
        }

        // I push the message to participants after persistence.
        // Offline users can catch up later from storage.
        deliveryService.deliver(message, *conversation);

        return true;
    }

    vector<Message> getConversationHistory(const string& conversationId) {
        return messageStore.getMessages(conversationId);
    }

    bool markDelivered(const string& messageId, const string& conversationId) {
        vector<Message> messages = messageStore.getMessages(conversationId);

        for (int i = 0; i < (int)messages.size(); i++) {
            if (messages[i].getMessageId() == messageId) {
                messages[i].setStatus(MessageStatus::DELIVERED);

                for (int j = 0; j < (int)listeners.size(); j++) {
                    listeners[j]->onDeliveryUpdate(messages[i]);
                }

                cout << "Message marked as delivered: " << messageId << endl;
                return true;
            }
        }

        return false;
    }

    bool markRead(const string& messageId, const string& conversationId) {
        vector<Message> messages = messageStore.getMessages(conversationId);

        for (int i = 0; i < (int)messages.size(); i++) {
            if (messages[i].getMessageId() == messageId) {
                messages[i].setStatus(MessageStatus::READ);

                for (int j = 0; j < (int)listeners.size(); j++) {
                    listeners[j]->onDeliveryUpdate(messages[i]);
                }

                cout << "Message marked as read: " << messageId << endl;
                return true;
            }
        }

        return false;
    }
};

// I use a simple listener to show how notifications can plug in.
// This keeps the design open for future extensions.
class ConsoleChatListener : public ChatEventListener {
public:
    void onNewMessage(const Message& message) {
        cout << "Listener: new message arrived -> " << message.getMessageId() << endl;
    }

    void onDeliveryUpdate(const Message& message) {
        cout << "Listener: status updated -> " << message.getMessageId() << endl;
    }
};

int main() {
    ChatService chatService;
    ConsoleChatListener listener;

    // I start by registering the users.
    // That gives the system valid participants for conversations.
    User user1("u1", "Aman");
    User user2("u2", "Riya");
    User user3("u3", "Kabir");

    chatService.addUser(user1);
    chatService.addUser(user2);
    chatService.addUser(user3);

    chatService.registerListener(&listener);

    // I create a group conversation as a realistic chat use case.
    vector<string> participants;
    participants.push_back("u1");
    participants.push_back("u2");
    participants.push_back("u3");

    string conversationId = chatService.createConversation(ConversationType::GROUP, participants);

    if (conversationId != "") {
        chatService.sendMessage(conversationId, "u1", "Hello everyone!");
        chatService.sendMessage(conversationId, "u2", "Hi Aman!");
        chatService.markDelivered("msg_1", conversationId);
        chatService.markRead("msg_1", conversationId);

        cout << "\nConversation history:" << endl;
        vector<Message> history = chatService.getConversationHistory(conversationId);
        for (int i = 0; i < (int)history.size(); i++) {
            cout << history[i].getMessageId() << " | "
                 << history[i].getSenderId() << " | "
                 << history[i].getContent() << endl;
        }
    }

    return 0;
}
