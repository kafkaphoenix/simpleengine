#include "Config.h"

#include <SimpleIni.h>

#include <cctype>
#include <cerrno>
#include <cstdlib>
#include <stdexcept>
#include <string>

namespace {
std::string formatKey(const char* section, const char* key) {
    return std::string("[") + section + "] " + key;
}

void throwConfigError(const std::string& message) {
    throw std::runtime_error("Config error: " + message);
}

bool isAllWhitespace(const char* value) {
    for (const char* it = value; *it != '\0'; ++it) {
        if (!std::isspace(static_cast<unsigned char>(*it))) {
            return false;
        }
    }
    return true;
}

bool parseBoolToken(const char* value, bool& out) {
    std::string token(value);
    for (auto& ch : token) {
        ch = static_cast<char>(std::tolower(static_cast<unsigned char>(ch)));
    }

    if (token == "1" || token == "true" || token == "yes" || token == "on") {
        out = true;
        return true;
    }
    if (token == "0" || token == "false" || token == "no" || token == "off") {
        out = false;
        return true;
    }
    return false;
}

bool parseIntToken(const char* value, int& out) {
    errno = 0;
    char* end = nullptr;
    long result = std::strtol(value, &end, 10);
    if (end == value || errno != 0 || (end && *end != '\0')) {
        return false;
    }
    out = static_cast<int>(result);
    return true;
}

bool parseFloatToken(const char* value, float& out) {
    errno = 0;
    char* end = nullptr;
    double result = std::strtod(value, &end);
    if (end == value || errno != 0 || (end && *end != '\0')) {
        return false;
    }
    out = static_cast<float>(result);
    return true;
}
}

const char* Config::requireValue(const CSimpleIniA& ini, const char* section, const char* key) {
    const char* value = ini.GetValue(section, key, nullptr);
    if (!value || isAllWhitespace(value)) {
        throwConfigError("missing key " + formatKey(section, key));
    }
    return value;
}

std::string Config::readString(const CSimpleIniA& ini, const char* section, const char* key) {
    return requireValue(ini, section, key);
}

int Config::readInt(const CSimpleIniA& ini, const char* section, const char* key) {
    const char* value = requireValue(ini, section, key);
    int parsed = 0;
    if (!parseIntToken(value, parsed)) {
        throwConfigError("invalid int for " + formatKey(section, key));
    }
    return parsed;
}

float Config::readFloat(const CSimpleIniA& ini, const char* section, const char* key) {
    const char* value = requireValue(ini, section, key);
    float parsed = 0.0f;
    if (!parseFloatToken(value, parsed)) {
        throwConfigError("invalid float for " + formatKey(section, key));
    }
    return parsed;
}

bool Config::readBool(const CSimpleIniA& ini, const char* section, const char* key) {
    const char* value = requireValue(ini, section, key);
    bool parsed = false;
    if (!parseBoolToken(value, parsed)) {
        throwConfigError("invalid bool for " + formatKey(section, key));
    }
    return parsed;
}

void Config::readWindow(const CSimpleIniA& ini, Window& window) {
    window.title = readString(ini, "window", "title");
    window.width = readInt(ini, "window", "width");
    window.height = readInt(ini, "window", "height");
    window.vsync = readBool(ini, "window", "vsync");
    window.startFullscreen = readBool(ini, "window", "startFullscreen");
    window.glDebugNotifications = readBool(ini, "window", "glDebugNotifications");

    if (window.width <= 0 || window.height <= 0) {
        throwConfigError("[window] width/height must be > 0");
    }
}

void Config::readInput(const CSimpleIniA& ini, Input& input) {
    input.mouseSmoothAlpha = readFloat(ini, "input", "mouseSmoothAlpha");
    input.mouseSensitivity = readFloat(ini, "input", "mouseSensitivity");
    input.fixedStep = readFloat(ini, "input", "fixedStep");

    if (input.mouseSmoothAlpha < 0.0f || input.mouseSmoothAlpha > 1.0f) {
        throwConfigError("[input] mouseSmoothAlpha must be in [0, 1]");
    }
    if (input.mouseSensitivity <= 0.0f) {
        throwConfigError("[input] mouseSensitivity must be > 0");
    }
    if (input.fixedStep <= 0.0f) {
        throwConfigError("[input] fixedStep must be > 0");
    }
}

void Config::readCamera(const CSimpleIniA& ini, Camera& camera) {
    camera.moveSpeed = readFloat(ini, "camera", "moveSpeed");
    camera.fov = readFloat(ini, "camera", "fov");
    camera.nearPlane = readFloat(ini, "camera", "nearPlane");
    camera.farPlane = readFloat(ini, "camera", "farPlane");
    camera.startPosX = readFloat(ini, "camera", "startPosX");
    camera.startPosY = readFloat(ini, "camera", "startPosY");
    camera.startPosZ = readFloat(ini, "camera", "startPosZ");

    if (camera.moveSpeed <= 0.0f) {
        throwConfigError("[camera] moveSpeed must be > 0");
    }
    if (camera.fov <= 1.0f || camera.fov >= 179.0f) {
        throwConfigError("[camera] fov must be in (1, 179)");
    }
    if (camera.nearPlane <= 0.0f) {
        throwConfigError("[camera] nearPlane must be > 0");
    }
    if (camera.farPlane <= camera.nearPlane) {
        throwConfigError("[camera] farPlane must be > nearPlane");
    }
}

void Config::readStats(const CSimpleIniA& ini, Stats& stats) {
    stats.showStats = readBool(ini, "stats", "showStats");
    stats.interval = readFloat(ini, "stats", "interval");

    if (stats.interval <= 0.0f) {
        throwConfigError("[stats] interval must be > 0");
    }
}

Config Config::load(const std::string& path) {
    Config config;
    CSimpleIniA ini;
    ini.SetUnicode();
    if (ini.LoadFile(path.c_str()) < 0) {
        throwConfigError("failed to load " + path);
    }
    readWindow(ini, config.m_Window);
    readInput(ini, config.m_Input);
    readCamera(ini, config.m_Camera);
    readStats(ini, config.m_Stats);

    return config;
}
