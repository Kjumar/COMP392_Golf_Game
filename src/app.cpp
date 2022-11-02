#include "app.hpp"

#include "lve_camera.hpp"
#include "lve_buffer.hpp"
#include "rendersystems/simple_render_system.hpp"
#include "rendersystems/point_light_system.hpp"
#include "rendersystems/transparent_render_system.hpp"
#include "rendersystems/outline_render_system.hpp"
#include "rendersystems/wireframe_render_system.hpp"
#include "rendersystems/golfturf_render_system.hpp"

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
#include <thread>
#include <cassert>
#include <stdexcept>
#include <iostream>
#include <algorithm>
#include <stdlib.h>
#include <time.h>

namespace lve {

    App::App() {
        globalPool = LveDescriptorPool::Builder(lveDevice)
            .setMaxSets(LveSwapChain::MAX_FRAMES_IN_FLIGHT)
            .addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, LveSwapChain::MAX_FRAMES_IN_FLIGHT)
            .build();
        loadGameObjects();
    }

    App::~App() {}

    void App::run() {
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
        GolfturfRenderSystem golfTurfRenderSystem{
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

        // setting up the game objects for each course
        LveGameObject courses[9] = { LveGameObject::createGameObject(),
            LveGameObject::createGameObject(),
            LveGameObject::createGameObject(),
            LveGameObject::createGameObject(),
            LveGameObject::createGameObject(),
            LveGameObject::createGameObject(),
            LveGameObject::createGameObject(),
            LveGameObject::createGameObject(),
            LveGameObject::createGameObject()};
        courses[0].tag = "c1";
        courses[1].tag = "c2";
        courses[2].tag = "c3";
        courses[3].tag = "c4";
        courses[4].tag = "c5";
        courses[5].tag = "c6";
        courses[6].tag = "c7";
        courses[7].tag = "c8";
        courses[8].tag = "c9";

        CollisionManager collisionManager{};
        std::vector<BoxCollider> colliders = CollisionManager::readCollidersFromFile("models/collision/hole1_colliders.boxc");
        for (BoxCollider bc : colliders)
        {
            bc.gameObject = &courses[0];
        }

        loadColliders(colliders, "models/collision/hole2_colliders.boxc", &courses[1]);
        loadColliders(colliders, "models/collision/hole3_colliders.boxc", &courses[2]);
        loadColliders(colliders, "models/collision/hole4_colliders.boxc", &courses[3]);
        loadColliders(colliders, "models/collision/hole5_colliders.boxc", &courses[4]);
        loadColliders(colliders, "models/collision/hole6_colliders.boxc", &courses[5]);
        loadColliders(colliders, "models/collision/hole7_colliders.boxc", &courses[6]);
        loadColliders(colliders, "models/collision/hole8_colliders.boxc", &courses[7]);

        bool showCollisionDebug = false;
        std::vector<LveModel*> staticColliderWireframes;

        for (BoxCollider collider : colliders)
        {
            collisionManager.InsertStaticCollider(new BoxCollider(collider));

            staticColliderWireframes.push_back(collider.GetWireFrame(lveDevice, {0.0f, 1.0f, 0.0f}));
        }
        collisionManager.buildStaticTree();

        // triggers for each golf hole. They're not passed to the collision manager since we'll be handling these ourselves
        BoxCollider goals[9] = {
            BoxCollider({14.0f, 0.5f, 0.0f}, {1.0f, 0.0f, 0.0f, 0.15f}, {0.0f, 0.0f, 1.0f, 0.15f}, {0.0f, 1.0f, 0.0f, 0.25f}),
            BoxCollider({22.0f, 0.5f, 8.0f}, {1.0f, 0.0f, 0.0f, 0.15f}, {0.0f, 0.0f, 1.0f, 0.15f}, {0.0f, 1.0f, 0.0f, 0.25f}),
            BoxCollider({28.0f, -1.5f, -4.0f}, {1.0f, 0.0f, 0.0f, 0.15f}, {0.0f, 0.0f, 1.0f, 0.15f}, {0.0f, 1.0f, 0.0f, 0.25f}),
            BoxCollider({28.0f, 0.5f, -22.0f}, {1.0f, 0.0f, 0.0f, 0.15f}, {0.0f, 0.0f, 1.0f, 0.15f}, {0.0f, 1.0f, 0.0f, 0.25f}),
            BoxCollider({20.0f, 0.5f, -42.0f}, {1.0f, 0.0f, 0.0f, 0.15f}, {0.0f, 0.0f, 1.0f, 0.15f}, {0.0f, 1.0f, 0.0f, 0.25f}),
            BoxCollider({1.5f, 0.5f, -46.0f}, {1.0f, 0.0f, 0.0f, 0.15f}, {0.0f, 0.0f, 1.0f, 0.15f}, {0.0f, 1.0f, 0.0f, 0.25f}),
            BoxCollider({-10.0f, 0.5f, -31.5f}, {1.0f, 0.0f, 0.0f, 0.15f}, {0.0f, 0.0f, 1.0f, 0.15f}, {0.0f, 1.0f, 0.0f, 0.25f}),
            BoxCollider({-20.0f, 0.0f, -20.0f}, {1.0f, 0.0f, 0.0f, 0.15f}, {0.0f, 0.0f, 1.0f, 0.15f}, {0.0f, 1.0f, 0.0f, 0.25f}),
            BoxCollider({10.5f, 0.5f, -24.0f}, {1.0f, 0.0f, 0.0f, 0.15f}, {0.0f, 0.0f, 1.0f, 0.15f}, {0.0f, 1.0f, 0.0f, 0.25f})
        };

        for (BoxCollider collider : goals)
        {
            staticColliderWireframes.push_back(collider.GetWireFrame(lveDevice, {1.0f, 1.0f, 0.0f}));
        }

        // direct refereces to objects I want to control
        LveGameObject* playerBall = &(gameObjects.find(10)->second);
        GolfBallController ballController{*playerBall, 0.1f};

        LveGameObject* ballAim = &(gameObjects.find(11)->second);

        LveGameObject* ballPower = &(gameObjects.find(12)->second);

        std::vector<LveGameObject*> transparentObjects{};

        // keep track of the old keystate for the C key (to help simulate "OnKeyDown" events)
        bool keyStateC = false;
        bool keyStateKPAdd = false;

        // starting location for each track
        glm::vec3 tees[] = {
            { 0.0f, -0.11f, 0.0f },
            { 14.0f, -0.11f, 6.0f },
            { 28.0f, -0.11f, 6.0f },
            { 34.0f, -3.11f, -8.0f },
            { 26.0f, -0.11f, -28.0f },
            { 16.0f, -0.11f, -49.0f },
            { -2.0f, -0.11f, -40.0f },
            { -16.0f, -0.11f, -30.0f },
            { -18.0f, -0.11f, -10.0f }};
        int current_tee = 0;

        auto currentTime = std::chrono::high_resolution_clock::now();
        float totalTime = 0;

        std::cout << "Controls: \n  WASD - pivot the camera\n";
        std::cout << "  ARROW KEYS - rotate the camera\n  QE   - move up or down\n";
        std::cout << "  C    - switch between free camera and golf ball controls\n";
        std::cout << "Golf Ball Controls: \n  ARROW KEYS - rotate the camera\n";
        std::cout << "  SPACE - hold to charge your swing. While charging, use the ARROW KEYS to aim the ball and increase/decrease power\n";

        while (!lveWindow.shouldClose()) {
            glfwPollEvents();

            auto newTime = std::chrono::high_resolution_clock::now();
            // clamping frameTime to a maximum 0.1 seconds between frames to prevent some extreme edge cases in my collision system
            float frameTime = std::min(std::chrono::duration<float, std::chrono::seconds::period>(newTime - currentTime).count(), 0.1f);
            totalTime += frameTime;
            currentTime = newTime;

            if (playerBall->transform.translation.y > 0.2f)
            {
                // ball reset plane
                if (playerBall->transform.translation.y > 10)
                {
                    ballController.resetBall();
                }
                // else if (!ballController.isMoving())
                // {
                //     current_tee = (current_tee + 1) % 8;
                //     ballController.nextHole(tees[current_tee]);
                // }
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
            collisionManager.GetCollisions(sc, &GolfBallController::ForwardOnCollision, &ballController);

            // check if the ball is in the goal
            if (!ballController.isMoving())
            {
                if (goals[current_tee].CollidesWith(sc))
                {
                    current_tee = (current_tee + 1) % 8;
                    ballController.nextHole(tees[current_tee]);
                }
            }

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
                simpleRenderSystem.renderGameObjects(frameInfo, 0);
                golfTurfRenderSystem.renderGameObjects(frameInfo, 1, totalTime);
                outlineRenderer.renderGameObjects(frameInfo);
                //PointLightSystem.render(frameInfo);
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

    void App::loadGameObjects() {
        std::shared_ptr<LveModel> lveModel;

        lveModel = LveModel::createModelFromFile(lveDevice, "models/course/course1r_h1.obj");
        auto h1 = LveGameObject::createGameObject();
        h1.model = lveModel;
        h1.transform.translation = { 0.0f, 0.0f, 0.0f };
        h1.transform.scale = { 1.0f, 1.0f, 1.0f };
        h1.materialId = 1;
        gameObjects.emplace(h1.getId(), std::move(h1));

        lveModel = LveModel::createModelFromFile(lveDevice, "models/course/course1r_bumpers.obj");
        h1 = LveGameObject::createGameObject();
        h1.model = lveModel;
        h1.transform.translation = { 0.0f, 0.0f, 0.0f };
        h1.transform.scale = { 1.0f, 1.0f, 1.0f };
        h1.materialId = 0;
        h1.outline = true;
        gameObjects.emplace(h1.getId(), std::move(h1));

        lveModel = LveModel::createModelFromFile(lveDevice, "models/course/course1r_h2.obj");
        auto h2 = LveGameObject::createGameObject();
        h2.model = lveModel;
        h2.transform.translation = { 0.0f, 0.0f, 0.0f };
        h2.transform.scale = { 1.0f, 1.0f, 1.0f };
        h2.materialId = 1;
        gameObjects.emplace(h2.getId(), std::move(h2));

        lveModel = LveModel::createModelFromFile(lveDevice, "models/course/course1r_h3.obj");
        auto h3 = LveGameObject::createGameObject();
        h3.model = lveModel;
        h3.transform.translation = { 0.0f, 0.0f, 0.0f };
        h3.transform.scale = { 1.0f, 1.0f, 1.0f };
        h3.materialId = 1;
        gameObjects.emplace(h3.getId(), std::move(h3));

        lveModel = LveModel::createModelFromFile(lveDevice, "models/course/course1r_h4.obj");
        auto h4 = LveGameObject::createGameObject();
        h4.model = lveModel;
        h4.transform.translation = { 0.0f, 0.0f, 0.0f };
        h4.transform.scale = { 1.0f, 1.0f, 1.0f };
        h4.materialId = 1;
        gameObjects.emplace(h4.getId(), std::move(h4));

        lveModel = LveModel::createModelFromFile(lveDevice, "models/course/course1r_h5.obj");
        auto h5 = LveGameObject::createGameObject();
        h5.model = lveModel;
        h5.transform.translation = { 0.0f, 0.0f, 0.0f };
        h5.transform.scale = { 1.0f, 1.0f, 1.0f };
        h5.materialId = 1;
        gameObjects.emplace(h5.getId(), std::move(h5));

        lveModel = LveModel::createModelFromFile(lveDevice, "models/course/course1r_h6.obj");
        auto h6 = LveGameObject::createGameObject();
        h6.model = lveModel;
        h6.transform.translation = { 0.0f, 0.0f, 0.0f };
        h6.transform.scale = { 1.0f, 1.0f, 1.0f };
        h6.materialId = 1;
        gameObjects.emplace(h6.getId(), std::move(h6));

        lveModel = LveModel::createModelFromFile(lveDevice, "models/course/course1r_h7.obj");
        auto h7 = LveGameObject::createGameObject();
        h7.model = lveModel;
        h7.transform.translation = { 0.0f, 0.0f, 0.0f };
        h7.transform.scale = { 1.0f, 1.0f, 1.0f };
        h7.materialId = 1;
        gameObjects.emplace(h7.getId(), std::move(h7));

        lveModel = LveModel::createModelFromFile(lveDevice, "models/course/course1r_h8.obj");
        auto h8 = LveGameObject::createGameObject();
        h8.model = lveModel;
        h8.transform.translation = { 0.0f, 0.0f, 0.0f };
        h8.transform.scale = { 1.0f, 1.0f, 1.0f };
        h8.materialId = 1;
        gameObjects.emplace(h8.getId(), std::move(h8));

        lveModel = LveModel::createModelFromFile(lveDevice, "models/course/course1r_h9.obj");
        auto h9 = LveGameObject::createGameObject();
        h9.model = lveModel;
        h9.transform.translation = { 0.0f, 0.0f, 0.0f };
        h9.transform.scale = { 1.0f, 1.0f, 1.0f };
        h9.materialId = 1;
        gameObjects.emplace(h9.getId(), std::move(h9));

        lveModel = LveModel::createModelFromFile(lveDevice,
            "models/ball.obj");
        auto golfBall = LveGameObject::createGameObject();
        golfBall.model = lveModel;
        golfBall.transform.translation = {0.0f, -0.11f, 0.0f};
        golfBall.transform.scale = {0.6f, 0.6f, 0.6f};
        golfBall.specular = 1.0f;
        golfBall.outline = true;
        // id = 9
        gameObjects.emplace(golfBall.getId(), std::move(golfBall));

        lveModel = LveModel::createModelFromFile(lveDevice,
            "models/ball_hold.obj");
        auto ballAim = LveGameObject::createGameObject();
        ballAim.model = lveModel;
        ballAim.transform.translation = {0.0f, -0.1f, 0.0f};
        ballAim.transform.scale = {0.12f, 0.1f, 0.12f};
        // id = 10
        gameObjects.emplace(ballAim.getId(), std::move(ballAim));

        lveModel = LveModel::createModelFromFile(lveDevice,
            "models/ball_power.obj");
        auto ballPow = LveGameObject::createGameObject();
        ballPow.model = lveModel;
        ballPow.transform.translation = {0.0f, -0.1f, 0.1f};
        ballPow.transform.scale = {0.12f, 0.1f, 0.12f};
        // id = 11
        gameObjects.emplace(ballPow.getId(), std::move(ballPow));

        lveModel = LveModel::createModelFromFile(lveDevice,
            "models/THE_CUBES.obj");
        auto cubes = LveGameObject::createGameObject();
        cubes.model = lveModel;
        cubes.transform.translation = {0.0f, 1.0f, 0.0f};
        cubes.transform.scale = {1.0f, 1.0f, 1.0f};
        cubes.specular = 1.0f;
        cubes.outline = true;
        gameObjects.emplace(cubes.getId(), std::move(cubes));

        lveModel = LveModel::createModelFromFile(lveDevice, "models/ballhole.obj");
        auto go = LveGameObject::createGameObject();
        go.model = lveModel;
        go.transform.translation = { 14.0f, 0.0f, 0.0f };
        go.transform.scale = { 1.0f, 1.0f, 1.0f };
        gameObjects.emplace(go.getId(), std::move(go));

        go = LveGameObject::createGameObject();
        go.model = lveModel;
        go.transform.translation = { 22.0f, 0.0f, 8.0f };
        go.transform.scale = { 1.0f, 1.0f, 1.0f };
        gameObjects.emplace(go.getId(), std::move(go));

        go = LveGameObject::createGameObject();
        go.model = lveModel;
        go.transform.translation = { 28.0f, -2.0f, -4.0f };
        go.transform.scale = { 1.0f, 1.0f, 1.0f };
        gameObjects.emplace(go.getId(), std::move(go));

        go = LveGameObject::createGameObject();
        go.model = lveModel;
        go.transform.translation = { 28.0f, 0.0f, -22.0f };
        go.transform.scale = { 1.0f, 1.0f, 1.0f };
        gameObjects.emplace(go.getId(), std::move(go));

        go = LveGameObject::createGameObject();
        go.model = lveModel;
        go.transform.translation = { 20.0f, 0.0f, -42.0f };
        go.transform.scale = { 1.0f, 1.0f, 1.0f };
        gameObjects.emplace(go.getId(), std::move(go));

        go = LveGameObject::createGameObject();
        go.model = lveModel;
        go.transform.translation = { 1.5f, 0.0f, -46.0f };
        go.transform.scale = { 1.0f, 1.0f, 1.0f };
        gameObjects.emplace(go.getId(), std::move(go));

        go = LveGameObject::createGameObject();
        go.model = lveModel;
        go.transform.translation = { -10.0f, 0.0f, -31.5f };
        go.transform.scale = { 1.0f, 1.0f, 1.0f };
        gameObjects.emplace(go.getId(), std::move(go));

        go = LveGameObject::createGameObject();
        go.model = lveModel;
        go.transform.translation = { -20.0f, -0.5f, -20.0f };
        go.transform.scale = { 1.0f, 1.0f, 1.0f };
        gameObjects.emplace(go.getId(), std::move(go));

        go = LveGameObject::createGameObject();
        go.model = lveModel;
        go.transform.translation = { 10.5f, 0.0f, -24.0f };
        go.transform.scale = { 1.0f, 1.0f, 1.0f };
        gameObjects.emplace(go.getId(), std::move(go));

        // ==============================================
        //              Let there be trees!
        // ==============================================
        lveModel = LveModel::createModelFromFile(lveDevice,
            "models/tree.obj");

        for (int i = 0; i < 0; i++)
        {
            auto tree = LveGameObject::createGameObject();
            tree.model = lveModel;
            tree.transform.translation = { 5.0f * i, 0.0f, 17.5f + (rand() % 100) / 20.0f };
            tree.transform.scale = {1.0f, 1.0f, 1.0f};
            gameObjects.emplace(tree.getId(), std::move(tree));
        }

        for (int i = 0; i < 0; i++)
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

    void App::loadColliders(std::vector<BoxCollider> &colliderList, const char filename[], LveGameObject* parent)
    {
        std::vector<BoxCollider> colliders2 = CollisionManager::readCollidersFromFile(filename);
        for (BoxCollider bc : colliders2)
        {
            bc.gameObject = parent;
            colliderList.push_back(bc);
        }
    }
}  // namespace lve