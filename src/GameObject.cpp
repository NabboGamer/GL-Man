#include "GameObject.hpp"


GameObject::GameObject(glm::vec3 position, glm::vec3 direction, float scale, Shader* shader, std::vector<float>& mesh, Texture2D* texture, ModelRenderer* renderer)
          : position(position), direction(direction), scale(scale), shader(shader), mesh(mesh), texture(texture), renderer(renderer) {
    modelIndex = renderer->InitModel(shader, mesh);
}

void GameObject::Draw() {
    renderer->DrawModel(modelIndex, position, direction, scale, texture);
}