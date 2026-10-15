#pragma once  // header guard

#include "GameObject.h"

// inherits GameObject because it needs to be able to added to the engine
class Galaga : public CMPUT350::GameObject {
    public:
        // Using enum for state
        enum State {
            Waiting,
            Startup,
            Waves,
            Gameplay,
            GameOver
        };

    private:
        State mState{Waiting};  // start at waiting state
};
