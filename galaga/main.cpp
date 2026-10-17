#include <memory>

#include "Galaga.h"
#include "GameEngine.h"

// Galaga creates the player, the stars, and the enemies
int main() {
    CMPUT350::GameEngine engine(768, 1024, "Galaga");
    engine.AddGameObject(std::make_shared<Galaga>(768, 1024));
    engine.Run();
    return 0;
}