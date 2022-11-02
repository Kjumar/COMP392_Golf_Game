#pragma once

#include "lve_device.hpp"
#include "lve_game_object.hpp"
#include "lve_window.hpp"
#include "lve_renderer.hpp"
#include "lve_descriptors.hpp"

#include "collision/collision.hpp"
#include "collision/collision_manager.hpp"

// std
#include <memory>
#include <vector>

namespace lve {
  class App {
  public:
    static constexpr int WIDTH = 800;
    static constexpr int HEIGHT = 600;

    App();
    ~App();

    App(const App &) = delete;
    App &operator=(const App &) = delete;

    void run();

  private:
    void loadGameObjects();
    void loadColliders(std::vector<BoxCollider> &colliderList, const char filename[], LveGameObject* parent);

    LveWindow lveWindow{WIDTH, HEIGHT, "Untitled Golf Game"};
    LveDevice lveDevice{lveWindow};
    LveRenderer lveRenderer{ lveWindow, lveDevice };

    // note: order of declarations matters
    std::unique_ptr<LveDescriptorPool> globalPool{};
    LveGameObject::Map gameObjects;
  };
}  // namespace lve