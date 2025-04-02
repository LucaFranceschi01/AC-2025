#include "application.h"

#include "math/vec3.h"

#include "animations/pose.h"
#include "animations/skeleton.h"

Camera* Application::camera = nullptr;
Application* Application::instance;

// does not really matter
quat quat_task_quat;

void Application::init(GLFWwindow* window)
{
    instance = this;
    glfwGetFramebufferSize(window, &window_width, &window_height);

    flag_grid = true;
    flag_wireframe = false;

    // Create camera
    camera = new Camera();
    camera->look_at(vec3(0.f, 1.5f, 7.f), vec3(0.f, 0.0f, 0.f), vec3(0.f, 1.f, 0.f));
    camera->set_perspective(45.f, window_width / (float)window_height, 0.1f, 500.f);

    /* ADD ENTITIES TO THE SCENE */
    /*Entity* example = new Entity("Example");
    example->mesh = Mesh::get("res/meshes/sphere.obj");
    example->material = new FlatMaterial();
    example->set_transform(Transform());
    entity_list.push_back(example);*/

    /* ADD ENTITIES TO THE SCENE */

    // Add spheres
    Entity* dot_ent = new Entity("Dot Sphere");
    dot_ent->mesh = Mesh::get("res/meshes/sphere.obj");
    dot_ent->material = new FlatMaterial();
    dot_ent->set_transform(Transform(vec3(-3.f, 0.f, 0.f), quat(), vec3(1.f)));
    entity_list.push_back(dot_ent);

    Entity* cross_ent = new Entity("Cross Sphere");
    cross_ent->mesh = Mesh::get("res/meshes/sphere.obj");
    cross_ent->material = new NormalMaterial();
    cross_ent->set_transform(Transform(vec3(-1.f, 0.f, 0.f), quat(), vec3(1.f)));
    entity_list.push_back(cross_ent);

    Entity* quat_ent = new Entity("Quat Sphere");
    quat_ent->mesh = Mesh::get("res/meshes/sphere.obj");
    quat_ent->material = new NormalMaterial();
    quat_ent->set_transform(Transform(vec3(1.f, 0.f, 0.f), quat(), vec3(1.f)));
    entity_list.push_back(quat_ent);

    Entity* lerp_ent = new Entity("Lerp Sphere");
    lerp_ent->mesh = Mesh::get("res/meshes/sphere.obj");
    lerp_ent->material = new FlatMaterial();
    lerp_ent->set_transform(Transform(vec3(3.f, 0.f, 0.f), quat(), vec3(1.f)));
    entity_list.push_back(lerp_ent);

    // Then, add debug lines (so they are rendered on top of the previous entities)
    LineHelper* dot_l1 = new LineHelper(vec3(-3.f, 0.f, 0.f), vec3(0.f, 1.f, 0.f));
    dot_l1->color = vec4(1.f, 0.f, 0.f, 1.f);
    dot_l1->unlocked = false;
    entity_list.push_back(dot_l1);

    LineHelper* dot_l2 = new LineHelper(vec3(-3.f, 0.f, 0.f), vec3(0.f, 0.f, 1.f));
    entity_list.push_back(dot_l2);

    LineHelper* cross_l3 = new LineHelper(vec3(-1.f, 0.f, 0.f), vec3(0.f, 1.f, 0.f));
    cross_l3->color = vec4(1.f, 0.f, 0.f, 1.f);
    cross_l3->unlocked = false;
    entity_list.push_back(cross_l3);

    LineHelper* cross_l4 = new LineHelper(vec3(-1.f, 0.f, 0.f), vec3(0.f, 0.f, 1.f));
    entity_list.push_back(cross_l4);

    LineHelper* cross_l5 = new LineHelper(vec3(-1.f, 0.f, 0.f), vec3(1.f, 0.f, 0.f));
    cross_l5->color = vec4(0.f, 1.f, 0.f, 1.f);
    cross_l5->unlocked = false;
    entity_list.push_back(cross_l5);

    LineHelper* quat_l6 = new LineHelper(vec3(1.f, 0.f, 0.f), vec3(0.f, 0.f, 1.f));
    quat_l6->color = vec4(1.f, 0.f, 0.f, 1.f);
    quat_l6->unlocked = false;
    entity_list.push_back(quat_l6);

    LineHelper* quat_l7 = new LineHelper(vec3(0.f), vec3(0.f, 0.f, 1.f));
    // note that this line helper has its origin in 0.0.0 but it transformed
    // done so that the quat rotations are not weird
    quat_l7->set_transform(Transform(vec3(1.f, 0.f, 0.f), quat(), vec3(1.f)));
    entity_list.push_back(quat_l7);
}

void Application::update(float dt)
{
    float curr_time = glfwGetTime();

    // Update entities of the scene
    for (unsigned int i = 0; i < entity_list.size(); i++) {
        entity_list[i]->update(dt);
    }

    // Mouse update
    vec2 delta = last_mouse_position - mouse_position;
    if (orbiting) {
        camera->orbit(-delta.x * dt, delta.y * dt);
    }
    if (moving_2D) {
        camera->move(vec2(delta.x * dt, -delta.y * dt));
    }
    last_mouse_position = mouse_position;

    // inside entity_list there are the line helpers
    // the first two are related to dot product
    // the next three are related to cross product
    // the last two are related to quat

    // iterate entity_list and save directions
    std::vector<vec3> directions;
    for (Entity* entity : entity_list) {
        LineHelper* line_helper = entity->as<LineHelper>();

        if (!line_helper) continue;

        directions.push_back(line_helper->end); // line direction relative to its origin
    }

    { // DOT PRODUCT TASK 
        vec3 color_xyz(1.f); // we cannot get it from the material since it can be 0, will be always black

        color_xyz = color_xyz * dot(directions[0], directions[1]);

        entity_list[0]->set_color(color_xyz); // hardcoded entity[0] is Dot Product sphere
    }
    
    { // CROSS PRODUCT TASK
        entity_list[1]->follow_front(directions[3]); // hardcoded entity[1] is Cross Product sphere
    }

    { // QUAT TASK
        // we change the rotation according to quat_task_quat (awesome name)
        Transform t = entity_list[10]->get_transform(); // entity[10] is last LineHelper
        t.rotation = quat_task_quat;
        entity_list[10]->set_transform(t);

        // which can be seen in the sphere as
        entity_list[2]->follow_front(transform_vector(t, directions[6])); // entity[2] is Quat sphere

        // this does not work because "origin" and "end" are always fixed, they do not change according to the transform
        //entity_list[2]->follow_front(entity_list[10]->as<LineHelper>()->end);
    }
}

void Application::render()
{
    // set the clear color (the background color)
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);

    // Clear the window and the depth buffer
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // set OpenGL flags
    glEnable(GL_CULL_FACE); // render both sides of every triangle
    glEnable(GL_DEPTH_TEST); // check the occlusions using the Z buffer

    for (unsigned int i = 0; i < entity_list.size(); i++)
    {         
        entity_list[i]->render(camera);        
    }

    // Draw the floor grid
    if (flag_grid) draw_grid();
}

void Application::render_gui()
{
    if (ImGui::TreeNodeEx("Scene", ImGuiTreeNodeFlags_DefaultOpen))
    {
        if (ImGui::TreeNode("Camera")) {
            camera->render_gui();
            ImGui::TreePop();
        }

        unsigned int count = 0;
        std::stringstream ss;
        for (auto& node : entity_list) {
            ss << count;
            if (ImGui::TreeNode(node->name.c_str())) {
                node->render_gui();
                ImGui::TreePop();
            }
        }
        
        if (ImGui::TreeNode("Quat task")) {
            ImGui::DragFloat4("Quaternion", quat_task_quat.v, 0.1);
            ImGui::TreePop();
        }

        ImGui::TreePop();

    }
}

void Application::shut_down() { }

// keycodes: https://www.glfw.org/docs/3.3/group__keys.html
void Application::on_key_down(int key, int scancode)
{
    switch (key) {
    case GLFW_KEY_ESCAPE: // quit
        close = true;
        break;
    case GLFW_KEY_R:
        Shader::reload_all();
        break;
    }
}

// keycodes: https://www.glfw.org/docs/3.3/group__keys.html
void Application::on_key_up(int key, int scancode)
{
    switch (key) {
    case GLFW_KEY_T:
        std::cout << "T released" << std::endl;
        break;
    }
}

void Application::on_right_mouse_down()
{
    moving_2D = true;
    last_mouse_position = mouse_position;
}

void Application::on_right_mouse_up()
{
    moving_2D = false;
    last_mouse_position = mouse_position;
}

void Application::on_left_mouse_down()
{
    orbiting = true;
    last_mouse_position = mouse_position;
}

void Application::on_left_mouse_up()
{
    orbiting = false;
    last_mouse_position = mouse_position;
}

void Application::on_middle_mouse_down() { }

void Application::on_middle_mouse_up() { }

void Application::on_mouse_position(double xpos, double ypos) { }

void Application::on_scroll(double xOffset, double yOffset)
{
    int min = camera->min_fov;
    int max = camera->max_fov;

    if (yOffset < 0) {
        camera->fov += 4.f;
        if (camera->fov > max) {
            camera->fov = max;
        }
    }
    else {
        camera->fov -= 4.f;
        if (camera->fov < min) {
            camera->fov = min;
        }
    }
    camera->update_projection_matrix();
}