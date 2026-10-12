#pragma once  // header guard

namespace CMPUT350 {

class NotificationManager {
public:
    void Register(std::weak_ptr<GameObject> object, const std::string& key);
    void Unregister(std::weak_ptr<GameObject> object, const std::string& key);
    void Notify(const std::string& message);
private:
    std::unordered_map<std::string, std::vector<std::weak_ptr<GameObject>>> mListeners;
};

}
