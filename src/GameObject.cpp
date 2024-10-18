#include "GameObject.hpp"


GameObject::GameObject(glm::vec3 position, glm::vec3 direction, float scale, ModelRenderer* renderer, std::vector<float>& mesh, Texture2D* texture) 
          : position(position), direction(direction), scale(scale), renderer(renderer), mesh(mesh), texture(texture) {
    modelIndex = renderer->InitModel(mesh);
}

void GameObject::Draw() {
    renderer->DrawModel(modelIndex, position, direction, scale, texture);
}