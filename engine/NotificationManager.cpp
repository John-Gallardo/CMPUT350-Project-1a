#include "NotificationManager.h"

#include <memory>
#include <string>

#include "GameObject.h"

namespace CMPUT350 {

// Register a GameObject to receive notifications for a specific key
void NotificationManager::Register(std::weak_ptr<GameObject> object, const std::string& key) {
    auto& listeners = mListeners[key];
    for (const auto& listener : listeners) {
        if (!listener.owner_before(object) && !object.owner_before(listener)) return;
    }
    listeners.push_back(object);
}

// Unregister a GameObject from receiving notifications for a specific key
void NotificationManager::Unregister(std::weak_ptr<GameObject> object, const std::string& key) {
    auto it = mListeners.find(key);
    if (it == mListeners.end()) return;
    std::erase_if(it->second, [&](const std::weak_ptr<GameObject>& listener) {
        return !listener.owner_before(object) && !object.owner_before(listener);
    });
}

// Notify all registered GameObjects for a specific message key
void NotificationManager::Notify(const std::string& message) {
    auto it = mListeners.find(message);
    if (it == mListeners.end()) return;
    std::erase_if(it->second, [](const std::weak_ptr<GameObject>& listener) {
        auto object = listener.lock();
        return !object || !object->IsAlive();
    });
    // Notify all alive listeners
    std::vector<std::weak_ptr<GameObject>> listeners = it->second;
    for (const auto& listener : listeners) {
        if (auto object = listener.lock()) object->ReceiveNotification(message);
    }
}
}  // namespace CMPUT350
