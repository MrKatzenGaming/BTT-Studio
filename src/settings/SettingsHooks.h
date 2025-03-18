#pragma once

namespace btt {

class SettingsHooks {
public:
    static void installSettingsHooks();
    int calls = 0;
};

} // namespace btt