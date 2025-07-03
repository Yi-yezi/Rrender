#include "utils/Time.h"

namespace utils {

    void Time::Update(double currentTime) {
        s_LastTime = s_CurrentTime;
        s_CurrentTime = currentTime;
        s_DeltaTime = s_CurrentTime - s_LastTime;
    }

    float Time::GetDeltaTime() {
        return static_cast<float>(s_DeltaTime);
    }

    float Time::GetTime() {
        return static_cast<float>(s_CurrentTime);
    }

}
