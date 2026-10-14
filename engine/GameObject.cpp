#include "GameObject.h"

namespace CMPUT350 {

void GameObject::Initialize(GameContext* context) { return; }
void GameObject::Update(GameContext* context) { return; }
void GameObject::LateUpdate(GameContext* context) { return; }
void GameObject::RenderUI(GameContext* contextrender) { return; }
void GameObject::Kill() { mAlive = false; }
bool GameObject::IsAlive() const { return mAlive; }
void GameObject::ReceiveNotification(const std::string& key) { return; }
}  // namespace CMPUT350
