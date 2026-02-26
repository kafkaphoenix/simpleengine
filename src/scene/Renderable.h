#pragma once

#include "../assets/Material.h"
#include "Transform.h"

class Mesh;

struct Renderable {
    Mesh* mesh = nullptr;
    MaterialHandle material;
    Transform transform;
};
