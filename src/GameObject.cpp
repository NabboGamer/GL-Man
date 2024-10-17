#include "GameObject.hpp"


GameObject::GameObject() : position(0.0f, 0.0f), direction(0.0f), sprite() { }

GameObject::GameObject(glm::vec2 pos, glm::vec2 size, glm::vec2 direction, Texture2D sprite) : position(pos), size(size), direction(velocity), Sprite(sprite) { }

void GameObject::Draw(SpriteRenderer &renderer) {
    renderer.DrawSprite(this->Sprite, this->Position, this->Size, this->Rotation, this->Color);
}