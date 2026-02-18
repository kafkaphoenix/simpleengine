#pragma once
#include "Camera.h"

class Input;

class Player {
   public:
    Player(float aspectRatio) : m_Camera(aspectRatio) {}
    void update(float deltaTime, const Input& input);
    void setMouseSmoothing(float alpha);
    void setFixedStep(float stepSeconds);
    Camera& getCamera() { return m_Camera; }
    const Camera& getCamera() const { return m_Camera; }

   private:
    Camera m_Camera;
    float m_MouseSmoothAlpha = 0.5f;
    float m_SmoothedDx = 0.0f;
    float m_SmoothedDy = 0.0f;
    float m_MoveAccumulator = 0.0f;
    float m_FixedStep = 1.0f / 120.0f;

    void updateMouseLook(const Input& input);
    void updateKeyboardMovement(float deltaTime, const Input& input);
    void applyKeyboardStep(float stepSeconds, const Input& input);
};