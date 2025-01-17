//
// Created by Akira Mujawar on 21/06/24.
//

#include "main_scene.hpp"

#include "camera_controller.hpp"
#include "perspective_camera.hpp"
#include "static_mesh_instance.hpp"
#include "log.hpp"
#include "opengl_line_pipeline.hpp"

#include "mesh_render_component.hpp"
#include "line_render_component.hpp"
#include "transform2d_component.hpp"
#include "transform3d_component.hpp"

#include "life_object_component.hpp"
#include "transform3d_component.hpp"
#include "box_collider_data.hpp"
#include "collider_component.hpp"

#include "rigidbody_component.hpp"
#include "entt_buffer.hpp"
#include "entt_reflection_wrapper.hpp"

#include "physx_physics.hpp"

using MeowEngine::MainScene;


using namespace MeowEngine::assets;
using namespace MeowEngine::entity;

namespace {
    MeowEngine::PerspectiveCamera CreateCamera(const MeowEngine::WindowSize& size) {
        return MeowEngine::PerspectiveCamera(static_cast<float>(size.Width), static_cast<float>(size.Height));
    }
}

// this -> type -> value
// component -> property -> property

struct MainScene::Internal {
    MeowEngine::PerspectiveCamera Camera;
    MeowEngine::CameraController CameraController;

    EnttBuffer RegistryBuffer;

    // User Input Events
    const uint8_t* KeyboardState; // SDL owns the object & will manage the lifecycle. We just keep a pointer.

    Internal(const MeowEngine::WindowSize& size)
        : Camera(::CreateCamera(size))
        , CameraController({glm::vec3(0.0f, 2.0f , -10.0f)})
        , KeyboardState(SDL_GetKeyboardState(nullptr))
        , RegistryBuffer()
    {}

    void OnWindowResized(const MeowEngine::WindowSize& size) {
        Camera = ::CreateCamera(size);
    }

    void LoadOnRenderThread(std::shared_ptr<MeowEngine::AssetManager> assetManager) {
        assetManager->LoadShaderPipelines({
                                                  assets::ShaderPipelineType::Grid,
                                                  assets::ShaderPipelineType::Default,
                                                  assets::ShaderPipelineType::Line
                                          });

        assetManager->LoadStaticMeshes({
                                               assets::StaticMeshType::Plane,
                                               assets::StaticMeshType::Cube,
                                               assets::StaticMeshType::Sphere,
                                               assets::StaticMeshType::Cylinder,
                                               assets::StaticMeshType::Cone,
                                               assets::StaticMeshType::Torus
                                       });

        assetManager->LoadTextures({
                                           assets::TextureType::Default,
                                           assets::TextureType::Pattern
                                   });

        REGISTER_ENTT_COMPONENT(LifeObjectComponent);

        REGISTER_ENTT_COMPONENT(Transform2DComponent);
        REGISTER_ENTT_COMPONENT(Transform3DComponent);

        REGISTER_ENTT_COMPONENT(ColliderComponent);
        REGISTER_ENTT_COMPONENT(RigidbodyComponent);

        REGISTER_ENTT_COMPONENT(RenderComponentBase);
        REGISTER_ENTT_COMPONENT(LineRenderComponent);
        REGISTER_ENTT_COMPONENT(MeshRenderComponent);
    }

    void AddEntitiesOnPhysicsThread(MeowEngine::simulator::Physics* inPhysics) {
        RegistryBuffer.ApplyAddRemoveOnStaging(inPhysics);
    }

    void CreateSceneOnMainThread() {
        auto entity = RegistryBuffer.AddEntity();
        RegistryBuffer.AddComponent<entity::LifeObjectComponent>(entity, "torus");
        RegistryBuffer.AddComponent<entity::Transform3DComponent>(
                entity,
                Camera.GetProjectionMatrix() * Camera.GetViewMatrix(),
                glm::vec3{5, 0, 0},
                glm::vec3{1.0, 1.0f, 1.0f},
                glm::vec3{0.0f, 1.0f, 0.0f},
                12.0f
        );
        RegistryBuffer.AddComponent<entity::MeshRenderComponent>(
                entity,
                assets::ShaderPipelineType::Default,
                new MeowEngine::StaticMeshInstance{
                        assets::StaticMeshType::Torus,
                        assets::TextureType::Pattern
                }
        );

        const auto cubeEntity = RegistryBuffer.AddEntity();
        RegistryBuffer.AddComponent<entity::LifeObjectComponent>(cubeEntity, "cube");
        RegistryBuffer.AddComponent<entity::Transform3DComponent>(
                cubeEntity,
                Camera.GetProjectionMatrix() * Camera.GetViewMatrix(),
                glm::vec3{0.0f, 20.0f, 2},
                glm::vec3{0.5f, 0.5f,0.5f},
                glm::vec3{0.0f, 1.0f, 0.0f},
                0
        );
        RegistryBuffer.AddComponent<entity::MeshRenderComponent>(
                cubeEntity,
                assets::ShaderPipelineType::Default,
                new MeowEngine::StaticMeshInstance{
                        assets::StaticMeshType::Cube,
                        assets::TextureType::Pattern
                }
        );
        RegistryBuffer.AddComponent<entity::ColliderComponent>(
            cubeEntity,
            entity::ColliderType::BOX,
            new entity::BoxColliderData()
        );
        RegistryBuffer.AddComponent<entity::RigidbodyComponent>(
            cubeEntity
        );

        const auto cubeEntity1 = RegistryBuffer.AddEntity();
        RegistryBuffer.AddComponent<entity::LifeObjectComponent>(cubeEntity1, "cube1");
        RegistryBuffer.AddComponent<entity::Transform3DComponent>(
                cubeEntity1,
                Camera.GetProjectionMatrix() * Camera.GetViewMatrix(),
                glm::vec3{0.0f, 0.0f, 2},
                glm::vec3{0.5f, 0.5f,0.5f},
                glm::vec3{0.0f, 1.0f, 0.0f},
                0
        );
        RegistryBuffer.AddComponent<entity::MeshRenderComponent>(
                cubeEntity1,
                assets::ShaderPipelineType::Default,
                new MeowEngine::StaticMeshInstance{
                        assets::StaticMeshType::Cube,
                        assets::TextureType::Pattern
                }
        );
        RegistryBuffer.AddComponent<entity::ColliderComponent>(
                cubeEntity1,
                entity::ColliderType::BOX,
                new entity::BoxColliderData()
        );
        RegistryBuffer.AddComponent<entity::RigidbodyComponent>(
                cubeEntity1
        );

        // setup object
        // later query for all rigidbody, get the physx, get the collider and construct for physics

        const auto gridEntity = RegistryBuffer.AddEntity();
        RegistryBuffer.AddComponent<entity::LifeObjectComponent>(gridEntity, "grid");
        RegistryBuffer.AddComponent<entity::Transform3DComponent>(
                gridEntity,
                Camera.GetProjectionMatrix() * Camera.GetViewMatrix(),
                glm::vec3{0, 0, 0},
                glm::vec3{1.0, 1.0f, 1.0f},
                glm::vec3{0.0f, 1.0f, 0.0f},
                0.0f
        );
        RegistryBuffer.AddComponent<entity::RenderComponentBase>(
            gridEntity,
            assets::ShaderPipelineType::Grid
        );

        MeowEngine::Log("Creating", "Created");
    }

    void Input(const float& delta, const MeowEngine::input::InputManager& inputManager) {
        if(!inputManager.isActive) {
            return;
        }

        if(inputManager.mouseState & SDL_BUTTON_LMASK) {
            CameraController.LookAround(inputManager.mouseDeltaX, inputManager.mouseDeltaY);
        }

        if(inputManager.isMouseDown && (inputManager.mouseState & SDL_BUTTON_RMASK)) {
            const auto cubeEntity = RegistryBuffer.AddEntity();
            RegistryBuffer.AddComponent<entity::LifeObjectComponent>(cubeEntity, "cube");
            RegistryBuffer.AddComponent<entity::Transform3DComponent>(
                    cubeEntity,
                    Camera.GetProjectionMatrix() * Camera.GetViewMatrix(),
                    glm::vec3{0.0f, 20.0f, 2},
                    glm::vec3{0.5f, 0.5f,0.5f},
                    glm::vec3{0.0f, 1.0f, 0.0f},
                    0
            );
            RegistryBuffer.AddComponent<entity::MeshRenderComponent>(
                    cubeEntity,
                    assets::ShaderPipelineType::Default,
                    new MeowEngine::StaticMeshInstance{
                            assets::StaticMeshType::Cube,
                            assets::TextureType::Pattern
                    }
            );
            RegistryBuffer.AddComponent<entity::ColliderComponent>(
                    cubeEntity,
                    entity::ColliderType::BOX,
                    new entity::BoxColliderData()
            );
            RegistryBuffer.AddComponent<entity::RigidbodyComponent>(
                    cubeEntity
            );
        }

        if (KeyboardState[SDL_SCANCODE_UP] || KeyboardState[SDL_SCANCODE_W]) {
            CameraController.MoveForward(delta);
        }

        if (KeyboardState[SDL_SCANCODE_DOWN] || KeyboardState[SDL_SCANCODE_S]) {
            CameraController.MoveBackward(delta);
        }

        if (KeyboardState[SDL_SCANCODE_Q] ) {
            CameraController.MoveUp(delta);
        }

        if (KeyboardState[SDL_SCANCODE_E]) {
            CameraController.MoveDown(delta);
        }

//        if (KeyboardState[SDL_SCANCODE_LEFT] || KeyboardState[SDL_SCANCODE_A]) {
//            CameraController.TurnLeft(delta);
//        }
//
//        if (KeyboardState[SDL_SCANCODE_RIGHT] || KeyboardState[SDL_SCANCODE_D]) {
//            CameraController.TurnRight(delta);
//        }
    }

    // We can perform -> culling, input detection
    void Update(const float& deltaTime) {
        Camera.Configure(CameraController.GetPosition(), CameraController.GetUp(), CameraController.GetDirection());

        const glm::mat4 cameraMatrix {Camera.GetProjectionMatrix() * Camera.GetViewMatrix()};

//        for(auto& lifeObject : LifeObjects) {
//            lifeObject.TransformComponent->Update(cameraMatrix);
//        }

//        MeowEngine::Log("Camera", std::to_string(Camera.GetPosition().z));

        auto view = RegistryBuffer.GetCurrent().view<entity::Transform3DComponent>();
        for(auto entity: view) {
            auto& transform = view.get<entity::Transform3DComponent>(entity);
            transform.Update(deltaTime);
            transform.CalculateTransformMatrix(cameraMatrix);
        }

        //        auto view = registry.view<MeowEngine::core::component::Transform3DComponent>();
//        for(auto entity: view)
//        {
//            auto transform = view.get<MeowEngine::core::component::Transform3DComponent>(entity);
//
//        }

//        auto view = registry.view<entity::Transform3dComponent>();
//        registry.view<MeowEngine::entity::Transform3dComponent>().each([](auto entity, auto &MeowEngine::entity::Transform3dComponent) {
//            // ...
//        });
//
//        for(auto &&[entt::entity, MeowEngine::entity::Transform3dComponent]: registry.view<entity::Transform3dComponent>().each()) {
//            // ...
//        }
//        for(auto &&[entity,renderComponent, transform]: registry.view<entity::MeshRenderComponent, entity::Transform3DComponent>().each())
//        {
//            AssetManager->GetShaderPipeline<OpenGLMeshPipeline>(ShaderPipelineType::Default)->Render(
//                    *AssetManager,
//                    &renderComponent,
//                    &transform
//            );
//        }
//
//        for(auto &&[entity,renderComponent, transform]: registry.view<entity::RenderComponentBase, entity::Transform3DComponent>().each())
//        {
//            AssetManager->GetShaderPipeline<OpenGLGridPipeline>(ShaderPipelineType::Grid)->Render(
//                    *AssetManager,
//                    &renderComponent,
//                    &transform,
//                    cameraObject
//            );
//        }
    }

    void RenderGameView(MeowEngine::Renderer& renderer) {
        // This is important for now - we can come to this later for optimization
        // Current goal is to have full control on render as individual objects
        // as we will have elements like UI, Static Meshes, Post Processing, Camera Culling, Editor Tools
//        for(auto& lifeObject : LifeObjects) {
//            renderer.Render(&Camera, &lifeObject);
//        }
        renderer.RenderGameView(&Camera, RegistryBuffer.GetFinal());
    }

    void RenderUserInterface(MeowEngine::Renderer& renderer, unsigned int frameBufferId, const double fps) {
        renderer.RenderUserInterface(RegistryBuffer.GetFinal(), RegistryBuffer.GetPropertyChangeQueue() , frameBufferId, fps);
    }

    void SwapMainAndRenderBufferOnMainThread() {
        RegistryBuffer.Swap();
    }

    void SyncPhysicsBufferOnMainThread(bool inIsPhysicsThreadWorking) {
        auto currentView = RegistryBuffer.GetCurrent().view<MeowEngine::entity::Transform3DComponent, MeowEngine::entity::RigidbodyComponent>();
        auto stagingView = RegistryBuffer.GetStaging().view<MeowEngine::entity::Transform3DComponent, MeowEngine::entity::RigidbodyComponent>();
        auto finalView = RegistryBuffer.GetFinal().view<MeowEngine::entity::Transform3DComponent, MeowEngine::entity::RigidbodyComponent>();

        if(inIsPhysicsThreadWorking) {
            // since physics is working on its buffer (staging) we cache the main thread updates
            for(entt::entity entity : stagingView) {

                auto& staging = stagingView.get<MeowEngine::entity::RigidbodyComponent>(entity);
                auto final = finalView.get<MeowEngine::entity::Transform3DComponent>(entity);
                auto current = currentView.get<MeowEngine::entity::Transform3DComponent>(entity);

                staging.CacheDelta(current.Position - final.Position);
            }
        }
        else {
            // since physics is not working, we can update rigidbody in physics thread
            for (entt::entity entity: stagingView) {
                auto staging = stagingView.get<MeowEngine::entity::Transform3DComponent>(entity);
                auto &rigidbody = stagingView.get<MeowEngine::entity::RigidbodyComponent>(entity);
                auto &final = finalView.get<MeowEngine::entity::Transform3DComponent>(entity);

                auto &current = currentView.get<MeowEngine::entity::Transform3DComponent>(entity);

                rigidbody.AddDelta(current.Position - final.Position);
                current.Position = staging.Position;
            }
        }
    }

    void SyncRenderBufferOnMainThread() {
        // Add a template method to apply changes
        // Each component will have a apply data method
        // We will have data and component methods
        // Data will always my permanant with no apply data method
        // Components will always have apply method
        // Sync Transform Component
        auto currentView = RegistryBuffer.GetCurrent().view<MeowEngine::entity::Transform3DComponent>();
        auto finalView = RegistryBuffer.GetFinal().view<MeowEngine::entity::Transform3DComponent>();

        for(entt::entity entity : currentView) {
            auto current = currentView.get<MeowEngine::entity::Transform3DComponent>(entity);
            auto& final = finalView.get<MeowEngine::entity::Transform3DComponent>(entity);

            final.Position = current.Position;
        }

        // Apply UI inputs to render and main buffers
        // Push UI inputs for physics buffer (which gets processed in physics thread)
        RegistryBuffer.ApplyPropertyChange();
    }

    void SyncPhysicsBufferOnPhysicsThread() {
        // Apply update physics transform to entities
        auto view = RegistryBuffer.GetStaging().view<entity::Transform3DComponent, entity::RigidbodyComponent>();
        for(auto entity: view)
        {
            auto& transform = view.get<entity::Transform3DComponent>(entity);
            auto& rigidbody = view.get<entity::RigidbodyComponent>(entity);

            rigidbody.UpdateTransform(transform);
        }

        // Apply UI inputs to physics components
        RegistryBuffer.ApplyPropertyChangeOnStaging();
    }
};

MainScene::MainScene(const MeowEngine::WindowSize& size)
    : InternalPointer(MeowEngine::make_internal_ptr<Internal>(size)){}

void MainScene::OnWindowResized(const MeowEngine::WindowSize &size) {
    InternalPointer->OnWindowResized(size);
}

void MainScene::LoadOnRenderThread(std::shared_ptr<MeowEngine::AssetManager> assetManager) {
    InternalPointer->LoadOnRenderThread(assetManager);
}
void MainScene::CreateSceneOnMainThread() {
    InternalPointer->CreateSceneOnMainThread();
}

void MainScene::AddEntitiesOnPhysicsThread(MeowEngine::simulator::Physics* inPhysics) {
    InternalPointer->AddEntitiesOnPhysicsThread(inPhysics);
}

void MainScene::Input(const float &deltaTime, const MeowEngine::input::InputManager& inputManager) {
    InternalPointer->Input(deltaTime, inputManager);
}

void MainScene::Update(const float &deltaTime) {
    InternalPointer->Update(deltaTime);
}

void MainScene::RenderGameView(MeowEngine::Renderer &renderer) {
    InternalPointer->RenderGameView(renderer);
}

void MainScene::RenderUserInterface(MeowEngine::Renderer &renderer, unsigned int frameBufferId, const double fps) {
    InternalPointer->RenderUserInterface(renderer, frameBufferId, fps);
}

void MainScene::SwapMainAndRenderBufferOnMainThread() {
    InternalPointer->SwapMainAndRenderBufferOnMainThread();
}

void MainScene::SyncPhysicsBufferOnMainThread(bool inIsPhysicsThreadWorking) {
    InternalPointer->SyncPhysicsBufferOnMainThread(inIsPhysicsThreadWorking);
}

void MainScene::SyncRenderBufferOnMainThread() {
    InternalPointer->SyncRenderBufferOnMainThread();
}

void MainScene::SyncPhysicsBufferOnPhysicsThread() {
    InternalPointer->SyncPhysicsBufferOnPhysicsThread();
}











