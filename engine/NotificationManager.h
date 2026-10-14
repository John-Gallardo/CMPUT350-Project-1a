#pragma once  // header guard

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

namespace CMPUT350 {

class GameObject;

class NotificationManager {
public:
    void Register(std::weak_ptr<GameObject> object, const std::string& key);
    void Unregister(std::weak_ptr<GameObject> object, const std::string& key);
    void Notify(const std::string& message);

private:
    std::unordered_map<std::string, std::vector<std::weak_ptr<GameObject>>> mListeners;
};

}  // namespace CMPUT350
