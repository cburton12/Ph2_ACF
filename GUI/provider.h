#pragma once
#include <QWidget>
#include <memory>

namespace GUI{

    class Settings;

    class Provider final
    {
    public:
        Provider();
        static Settings& getSettingsAsSingleton();

    private:

        static std::unique_ptr<Settings> m_instanceSettings;
        explicit Provider(const Provider& rhs) = delete;
        Provider& operator = (const Provider& rhs) = delete;
    };
}



