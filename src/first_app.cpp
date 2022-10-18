#include "first_app.hpp"

#include "lve_camera.hpp"
#include "lve_buffer.hpp"
#include "systems/simple_render_system.hpp"
#include "systems/point_light_system.hpp"
#include "systems/transparent_render_system.hpp"
#include "systems/outline_render_system.hpp"
#include "systems/wireframe_render_system.hpp"

#include "collision/collision.hpp"
#include "collision/collision_manager.hpp"

//#include "controllers/orbit_controller.hpp"
#include "controllers/keyboard_movement_controller.hpp"
#include "controllers/golf_ball_controller.hpp"

// libs
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

// std
#include <array>
#include <chrono>
#include <cassert>
#include <stdexcept>
#include <iostream>
#include <algorithm>
#include <stdlib.h>
#include <time.h>

namespace lve {

    FirstApp::FirstApp() {
        globalPool = LveDescriptorPool::Builder(lveDevice)
            .setMaxSets(LveSwapChain::MAX_FRAMES_IN_FLIGHT)
            .addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, LveSwapChain::MAX_FRAMES_IN_FLIGHT)
            .build();
        loadGameObjects();
    }

    FirstApp::~FirstApp() {}

    void FirstApp::run() {
        srand(time(NULL));
        std::vector<std::unique_ptr<LveBuffer>> uboBuffers(LveSwapChain::MAX_FRAMES_IN_FLIGHT);
        for (int i = 0; i < uboBuffers.size(); i++)
        {
            uboBuffers[i] = std::make_unique<LveBuffer>(
                lveDevice,
                sizeof(GlobalUbo),
                1,
                VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
            uboBuffers[i]->map();
        }

        auto globalSetLayout = LveDescriptorSetLayout::Builder(lveDevice)
            .addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS)
            .build();

        std::vector<VkDescriptorSet> globalDescriptorSets(LveSwapChain::MAX_FRAMES_IN_FLIGHT);
        for (int i = 0; i < globalDescriptorSets.size(); i++)
        {
            auto bufferInfo = uboBuffers[i]->descriptorInfo();
            LveDescriptorWriter(*globalSetLayout, *globalPool)
                .writeBuffer(0, &bufferInfo)
                .build(globalDescriptorSets[i]);
        }

        // ================================
        //   Instantiating render systems
        // ================================
        SimpleRenderSystem simpleRenderSystem{
            lveDevice, lveRenderer.getSwapChainRenderPass(), globalSetLayout->getDescriptorSetLayout() };
        PointLightSystem PointLightSystem{
            lveDevice, lveRenderer.getSwapChainRenderPass(), globalSetLayout->getDescriptorSetLayout() };
        TransparentRenderSystem transparentRender{
            lveDevice, lveRenderer.getSwapChainRenderPass(), globalSetLayout->getDescriptorSetLayout()
        };
        OutlineRenderSystem outlineRenderer{
            lveDevice, lveRenderer.getSwapChainRenderPass(), globalSetLayout->getDescriptorSetLayout()
        };
        WireframeRenderSystem wireframeRenderer{
            lveDevice, lveRenderer.getSwapChainRenderPass(), globalSetLayout->getDescriptorSetLayout()
        };
        LveCamera camera{};
        camera.setViewTarget(glm::vec3(-1.0f, -2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 2.5f));

        auto viewerObject = LveGameObject::createGameObject();
        viewerObject.transform.translation.z = -2.5f;
        KeyboardMovementController cameraController{};

        CollisionManager collisionManager{};
        std::vector<BoxCollider> colliders = collisionManager.readCollidersFromFile("models/hole2_colliders.boxc");

        bool showCollisionDebug = false;
        std::vector<LveModel*> staticColliderWireframes;

        for (BoxCollider collider : colliders)
        {
            collisionManager.InsertStaticCollider(new BoxCollider(collider));

            staticColliderWireframes.push_back(collider.GetWireFrame(lveDevice, {0.0f, 1.0f, 0.0f}));
        }
        collisionManager.buildStaticTree();

        // direct refereces to objects I want to control
        LveGameObject* playerBall = &(gameObjects.find(1)->second);
        GolfBallController ballController{*playerBall, 0.09f};

        // LveGameObject* ballOutline = &(gameObjects.find(8)->second);

        LveGameObject* ballAim = &(gameObjects.find(2)->second);

        LveGameObject* ballPower = &(gameObjects.find(3)->second);

        std::vector<LveGameObject*> transparentObjects{};
        transparentObjects.push_back(&(gameObjects.find(7)->second));

        // keep track of the old keystate for the C key (to help simulate "OnKeyDown" events)
        bool keyStateC = false;
        bool keyStateKPAdd = false;

        // starting location for each track
        glm::vec3 tees[] = {
            { 0.0f, -0.11f, 0.0f },
            { 18.0f, -0.11f, 7.0f }};
        int current_tee = 0;

        auto currentTime = std::chrono::high_resolution_clock::now();

        std::cout << "Controls: \n  WASD - pivot the camera\n";
        std::cout << "  ARROW KEYS - rotate the camera\n  QE   - move up or down\n";
        std::cout << "  C    - switch between free camera and golf ball controls\n";
        std::cout << "Golf Ball Controls: \n  ARROW KEYS - rotate the camera\n";
        std::cout << "  SPACE - hold to charge your swing. While charging, use the ARROW KEYS to aim the ball and increase/decrease power\n";

        while (!lveWindow.shouldClose()) {
            glfwPollEvents();

            auto newTime = std::chrono::high_resolution_clock::now();
            // clamping frameTime to a minimum 0.1 seconds between frames to prevent some extreme edge cases in my collision system
            float frameTime = std::min(std::chrono::duration<float, std::chrono::seconds::period>(newTime - currentTime).count(), 0.1f);
            currentTime = newTime;

            if (playerBall->transform.translation.y > 0.2f)
            {
                // ball reset plane
                if (playerBall->transform.translation.y > 10)
                {
                    ballController.resetBall(tees[current_tee]);
                }
                else if (!ballController.isMoving())
                {
                    current_tee = (current_tee + 1) % 2;
                    ballController.resetBall(tees[current_tee]);
                }
            }

            // ============================
            // if the user pressed the C key, swap camera controls
            // ============================
            if (glfwGetKey(lveWindow.getGLFWwindow(), GLFW_KEY_C) == GLFW_PRESS)
            {
                if (!keyStateC)
                {
                    ballController.isAttached = !ballController.isAttached;
                    keyStateC = true;
                }
            }
            else
            {
                keyStateC = false;
            }
            ballController.update(lveWindow.getGLFWwindow(), frameTime);

            if (glfwGetKey(lveWindow.getGLFWwindow(), GLFW_KEY_KP_ADD) == GLFW_PRESS)
            {
                if (!keyStateKPAdd)
                {
                    showCollisionDebug = !showCollisionDebug;
                    keyStateKPAdd = true;
                }
            }
            else
            {
                keyStateKPAdd = false;
            }

            // ====================================
            // Collision phase of the loop
            //      for this game, I'm only checking collision between the ball and each box collider
            // ====================================
            SphereCollider& sc = ballController.getCollider();
            collisionManager.GetCollisions(sc, &GolfBallController::ForawrdOnCollision, &ballController);

            // if the player is currently focusing on the ball, pivot the camera around the ball
            // This could probably use its own controller, but as its behaviour is very specific to this case in the program
            // and it requires a lot of references held only by this Run function, I left it in here
            if (ballController.isAttached)
            {
                float yaw = ballController.aimRotation.y;
                float roll = ballController.aimRotation.x;
                glm::vec3 aimingDir{sin(yaw), 0.0f, cos(yaw)};
                glm::vec3 forwardDir{ aimingDir.x * cos(roll), -sin(roll), cos(roll) * aimingDir.z };
                viewerObject.transform.translation = playerBall->transform.translation - (forwardDir * 3.0f);
                camera.setViewTarget(viewerObject.transform.translation, playerBall->transform.translation);

                if (ballController.showReticle())
                {
                    ballAim->isVisible = true;
                    ballAim->transform.translation = playerBall->transform.translation;
                    ballAim->transform.rotation.y = ballController.aimRotation.y;
                    
                    ballPower->isVisible = true;
                    ballPower->transform.translation = playerBall->transform.translation + (aimingDir * 0.15f);
                    ballPower->transform.rotation.y = ballController.aimRotation.y;
                    ballPower->transform.scale.z = ballController.getPowerRatio() * 0.2f;
                }
                else
                {
                    ballAim->isVisible = false;
                    ballPower->isVisible = false;
                }
            }
            else
            {
                // otherwise we use default "flying" camera controls
                cameraController.moveInPlaneXZ(lveWindow.getGLFWwindow(), frameTime, viewerObject);
                camera.setViewYXZ(viewerObject.transform.translation, viewerObject.transform.rotation);
            }

            float aspect = lveRenderer.getAspectRatio();
            camera.setPerspectiveProjection(glm::radians(50.0f), aspect, 0.1f, 100.0f);
            
            if (auto commandBuffer = lveRenderer.beginFrame())
            {
                int frameIndex = lveRenderer.getFrameIndex();
                FrameInfo frameInfo{
                    frameIndex,
                    frameTime,
                    commandBuffer,
                    camera,
                    globalDescriptorSets[frameIndex],
                    gameObjects
                };

                // update
                GlobalUbo ubo{};
                ubo.projection = camera.getProjection();
                ubo.view = camera.getView();
                ubo.inverseView = camera.getInverseView();
                PointLightSystem.update(frameInfo, ubo);
                uboBuffers[frameIndex]->writeToBuffer(&ubo);
                uboBuffers[frameIndex]->flush();
                
                // render
                lveRenderer.beginSwapChainRenderPass(commandBuffer);
                simpleRenderSystem.renderGameObjects(frameInfo);
                outlineRenderer.renderGameObjects(frameInfo);
                PointLightSystem.render(frameInfo);
                transparentRender.renderGameObjects(frameInfo, transparentObjects);
                if (showCollisionDebug)
                {
                    wireframeRenderer.renderGameObjects(frameInfo, staticColliderWireframes);
                }
                lveRenderer.endSwapChainRenderPass(commandBuffer);
                lveRenderer.endFrame();
            }
        }

        vkDeviceWaitIdle(lveDevice.device());
    }

    void FirstApp::loadGameObjects() {
        std::shared_ptr<LveModel> lveModel;

        lveModel = LveModel::createModelFromFile(lveDevice,
            "models/hole1.obj");
        auto golfCourse = LveGameObject::createGameObject();
        golfCourse.model = lveModel;
        golfCourse.transform.translation = {0.0f, 0.0f, 0.0f};
        golfCourse.transform.scale = {1.0f, 1.0f, 1.0f};
        golfCourse.outline = true;
        gameObjects.emplace(golfCourse.getId(), std::move(golfCourse));

        lveModel = LveModel::createModelFromFile(lveDevice,
            "models/ball.obj");
        auto golfBall = LveGameObject::createGameObject();
        golfBall.model = lveModel;
        golfBall.transform.translation = {0.0f, -0.11f, 0.0f};
        golfBall.transform.scale = {1.0f, 1.0f, 1.0f};
        golfBall.specular = 1.0f;
        golfBall.outline = true;
        // id = 1
        gameObjects.emplace(golfBall.getId(), std::move(golfBall));

        lveModel = LveModel::createModelFromFile(lveDevice,
            "models/ball_hold.obj");
        auto ballAim = LveGameObject::createGameObject();
        ballAim.model = lveModel;
        ballAim.transform.translation = {0.0f, -0.1f, 0.0f};
        ballAim.transform.scale = {0.12f, 0.1f, 0.12f};
        // id = 2
        gameObjects.emplace(ballAim.getId(), std::move(ballAim));

        lveModel = LveModel::createModelFromFile(lveDevice,
            "models/ball_power.obj");
        auto ballPow = LveGameObject::createGameObject();
        ballPow.model = lveModel;
        ballPow.transform.translation = {0.0f, -0.1f, 0.1f};
        ballPow.transform.scale = {0.12f, 0.1f, 0.12f};
        // id = 3
        gameObjects.emplace(ballPow.getId(), std::move(ballPow));

        lveModel = LveModel::createModelFromFile(lveDevice,
            "models/terrain.obj");
        auto ter = LveGameObject::createGameObject();
        ter.model = lveModel;
        ter.transform.translation = {0.0f, 0.0f, 0.0f};
        ter.transform.scale = {1.0f, 1.0f, 1.0f};
        gameObjects.emplace(ter.getId(), std::move(ter));

        lveModel = LveModel::createModelFromFile(lveDevice,
            "models/rock_path.obj");
        auto rocks = LveGameObject::createGameObject();
        rocks.model = lveModel;
        rocks.transform.translation = {0.0f, 0.0f, 0.0f};
        rocks.transform.scale = {1.0f, 1.0f, 1.0f};
        rocks.specular = 0.3f;
        gameObjects.emplace(rocks.getId(), std::move(rocks));

        lveModel = LveModel::createModelFromFile(lveDevice,
            "models/table.obj");
        auto table = LveGameObject::createGameObject();
        table.model = lveModel;
        table.transform.translation = {2.0f, 0.0f, 5.0f};
        table.transform.scale = {1.0f, 1.0f, 1.0f};
        table.specular = 1.0f;
        table.outline = true;
        gameObjects.emplace(table.getId(), std::move(table));

        lveModel = LveModel::createModelFromFile(lveDevice,
            "models/table_glass.obj");
        auto ttop = LveGameObject::createGameObject();
        ttop.model = lveModel;
        ttop.transform.translation = {2.0f, 0.0f, 5.0f};
        ttop.transform.scale = {1.0f, 1.0f, 1.0f};
        ttop.alpha = 0.4f;
        // id = 7
        gameObjects.emplace(ttop.getId(), std::move(ttop));

        lveModel = LveModel::createModelFromFile(lveDevice,
            "models/hole2.obj");
        auto hole2 = LveGameObject::createGameObject();
        hole2.model = lveModel;
        hole2.transform.translation = {18.0f, 0.0f, 7.0f};
        hole2.transform.scale = {1.0f, 1.0f, 1.0f};
        hole2.outline = true;
        gameObjects.emplace(hole2.getId(), std::move(hole2));

        lveModel = LveModel::createModelFromFile(lveDevice,
            "models/river.obj");
        auto river = LveGameObject::createGameObject();
        river.model = lveModel;
        river.transform.translation = {0.0f, 0.0f, 0.0f};
        river.transform.scale = {1.0f, 1.0f, 1.0f};
        gameObjects.emplace(river.getId(), std::move(river));

        lveModel = LveModel::createModelFromFile(lveDevice,
            "models/hole3.obj");
        auto hole3 = LveGameObject::createGameObject();
        hole3.model = lveModel;
        hole3.transform.translation = {0.0f, 0.0f, 0.0f};
        hole3.transform.scale = {1.0f, 1.0f, 1.0f};
        hole3.outline = true;
        gameObjects.emplace(hole3.getId(), std::move(hole3));

        lveModel = LveModel::createModelFromFile(lveDevice,
            "models/THE_CUBES.obj");
        auto cubes = LveGameObject::createGameObject();
        cubes.model = lveModel;
        cubes.transform.translation = {0.0f, 1.0f, 0.0f};
        cubes.transform.scale = {1.0f, 1.0f, 1.0f};
        cubes.specular = 1.0f;
        cubes.outline = true;
        gameObjects.emplace(cubes.getId(), std::move(cubes));

        lveModel = LveModel::createModelFromFile(lveDevice,
            "models/lamp.obj");
        auto lamp = LveGameObject::createGameObject();
        lamp.model = lveModel;
        lamp.transform.translation = {5.0f, 0.0f, 2.0f};
        lamp.transform.scale = {1.0f, 1.0f, 1.0f};
        lamp.specular = 0.6f;
        gameObjects.emplace(lamp.getId(), std::move(lamp));

        lamp = LveGameObject::createGameObject();
        lamp.model = lveModel;
        lamp.transform.translation = { 22.0f, 0.0f, 11.0f };
        lamp.transform.scale = {1.0f, 1.0f, 1.0f};
        lamp.specular = 0.6f;
        gameObjects.emplace(lamp.getId(), std::move(lamp));

        lamp = LveGameObject::createGameObject();
        lamp.model = lveModel;
        lamp.transform.translation = { 24.0f, 0.0f, 6.0f };
        lamp.transform.scale = {1.0f, 1.0f, 1.0f};
        lamp.specular = 0.6f;
        gameObjects.emplace(lamp.getId(), std::move(lamp));

        lamp = LveGameObject::createGameObject();
        lamp.model = lveModel;
        lamp.transform.translation = { 34.75f, 0.0f, -2.0f };
        lamp.transform.scale = {1.0f, 1.0f, 1.0f};
        lamp.specular = 0.6f;
        gameObjects.emplace(lamp.getId(), std::move(lamp));

        lamp = LveGameObject::createGameObject();
        lamp.model = lveModel;
        lamp.transform.translation = { 37.25f, 0.0f, -2.0f };
        lamp.transform.scale = {1.0f, 1.0f, 1.0f};
        lamp.specular = 0.6f;
        gameObjects.emplace(lamp.getId(), std::move(lamp));

        lamp = LveGameObject::createGameObject();
        lamp.model = lveModel;
        lamp.transform.translation = { 34.75f, 0.0f, -10.0f };
        lamp.transform.scale = {1.0f, 1.0f, 1.0f};
        lamp.specular = 0.6f;
        gameObjects.emplace(lamp.getId(), std::move(lamp));

        lamp = LveGameObject::createGameObject();
        lamp.model = lveModel;
        lamp.transform.translation = { 37.25f, 0.0f, -10.0f };
        lamp.transform.scale = {1.0f, 1.0f, 1.0f};
        lamp.specular = 0.6f;
        gameObjects.emplace(lamp.getId(), std::move(lamp));

        // ==============================================
        //              Let there be trees!
        // ==============================================
        lveModel = LveModel::createModelFromFile(lveDevice,
            "models/tree.obj");

        for (int i = 0; i < 10; i++)
        {
            auto tree = LveGameObject::createGameObject();
            tree.model = lveModel;
            tree.transform.translation = { 5.0f * i, 0.0f, 17.5f + (rand() % 100) / 20.0f };
            tree.transform.scale = {1.0f, 1.0f, 1.0f};
            gameObjects.emplace(tree.getId(), std::move(tree));
        }

        for (int i = 0; i < 6; i++)
        {
            auto tree = LveGameObject::createGameObject();
            tree.model = lveModel;
            tree.transform.translation = { -12.5f + (rand() % 100) / 20.0f, 0.0f, 5.0f - (5.0f * i)};
            tree.transform.scale = {1.0f, 1.0f, 1.0f};
            gameObjects.emplace(tree.getId(), std::move(tree));
        }

        // ================================================
        //          Point lights created below
        // ================================================
        auto pointLight = LveGameObject::makePointLight();
        pointLight.color = { 1.0f, 1.0f, 1.0f };
        pointLight.transform.translation = { 5.0f, -3.4f, 2.0f };
        gameObjects.emplace(pointLight.getId(), std::move(pointLight));

        pointLight = LveGameObject::makePointLight();
        pointLight.color = { 1.0f, 0.0f, 1.0f };
        pointLight.transform.translation = { 22.0f, -3.4f, 11.0f };
        gameObjects.emplace(pointLight.getId(), std::move(pointLight));

        pointLight = LveGameObject::makePointLight();
        pointLight.color = { 1.0f, 0.0f, 1.0f };
        pointLight.transform.translation = { 24.0f, -3.4f, 6.0f };
        gameObjects.emplace(pointLight.getId(), std::move(pointLight));

        pointLight = LveGameObject::makePointLight();
        pointLight.color = { 0.0f, 1.0f, 1.0f };
        pointLight.transform.translation = { 34.75f, -3.4f, -2.0f };
        gameObjects.emplace(pointLight.getId(), std::move(pointLight));

        pointLight = LveGameObject::makePointLight();
        pointLight.color = { 0.0f, 1.0f, 1.0f };
        pointLight.transform.translation = { 37.25f, -3.4f, -2.0f };
        gameObjects.emplace(pointLight.getId(), std::move(pointLight));

        pointLight = LveGameObject::makePointLight();
        pointLight.color = { 0.0f, 1.0f, 1.0f };
        pointLight.transform.translation = { 34.75f, -3.4f, -10.0f };
        gameObjects.emplace(pointLight.getId(), std::move(pointLight));

        pointLight = LveGameObject::makePointLight();
        pointLight.color = { 0.0f, 1.0f, 1.0f };
        pointLight.transform.translation = { 37.25f, -3.4f, -10.0f };
        gameObjects.emplace(pointLight.getId(), std::move(pointLight));
    }
}  // namespace lve