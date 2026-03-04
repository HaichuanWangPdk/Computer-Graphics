#include "hw3.h"
#include "3rdparty/glad.h" // needs to be included before GLFW!
#include "3rdparty/glfw/include/GLFW/glfw3.h"
#include "hw3_scenes.h"
#include <iostream>
#include <cmath>
#include <vector>
#include <string>

using namespace hw3;

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}

void processInput(GLFWwindow* window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, true);
    }
}

static GLuint compile_shader(GLenum type, const char *src) {
    GLuint sh = glCreateShader(type);
    glShaderSource(sh, 1, &src, nullptr);
    glCompileShader(sh);
    GLint ok;
    glGetShaderiv(sh, GL_COMPILE_STATUS, &ok);
    if (!ok) {
        GLint len = 0; glGetShaderiv(sh, GL_INFO_LOG_LENGTH, &len);
        std::string log(len > 0 ? len : 1, '\0');
        glGetShaderInfoLog(sh, len, nullptr, &log[0]);
        std::cerr << "Shader compile error: " << log << std::endl;
        glDeleteShader(sh);
        return 0;
    }
    return sh;
}

static GLuint link_program(GLuint vs, GLuint fs) {
    GLuint prog = glCreateProgram();
    glAttachShader(prog, vs);
    glAttachShader(prog, fs);
    glLinkProgram(prog);
    GLint ok; glGetProgramiv(prog, GL_LINK_STATUS, &ok);
    if (!ok) {
        GLint len = 0; glGetProgramiv(prog, GL_INFO_LOG_LENGTH, &len);
        std::string log(len > 0 ? len : 1, '\0');
        glGetProgramInfoLog(prog, len, nullptr, &log[0]);
        std::cerr << "Program link error: " << log << std::endl;
        glDeleteProgram(prog);
        return 0;
    }
    return prog;
}

void hw_3_1(const std::vector<std::string> &params) {
    // HW 3.1: Open a window using GLFW
    
    if (!glfwInit()) {
        std::cout << "Failed to initialize GLFW" << std::endl;
        return;
    }
    
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    
    GLFWwindow* window = glfwCreateWindow(800, 600, "HW3_1 Window", NULL, NULL);
    if (window == NULL) {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return;
    }
    
    glfwMakeContextCurrent(window);
    
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cout << "Failed to initialize GLAD" << std::endl;
        glfwTerminate();
        return;
    }
    
    glViewport(0, 0, 800, 600);
    
    glClearColor(1.0f, 0.75f, 0.8f, 1.0f); // pink!
    
    while (!glfwWindowShouldClose(window)) {
        processInput(window);
        
        glClear(GL_COLOR_BUFFER_BIT);
        
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    
    glfwTerminate();
}

void hw_3_2(const std::vector<std::string> &params) {
    // HW 3.2: Render a single 2D triangle
    
    if (!glfwInit()) {
        std::cout << "Failed to initialize GLFW" << std::endl;
        return;
    }
    
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    
    GLFWwindow* window = glfwCreateWindow(800, 600, "HW3_2", NULL, NULL);
    if (window == NULL) {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return;
    }
    
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cout << "Failed to initialize GLAD" << std::endl;
        glfwTerminate();
        return;
    }
    
    glViewport(0, 0, 800, 600);
    
    std::vector<float> vertices = {
        -0.5f, -0.5f, 0.0f,  
         0.5f, -0.5f, 0.0f,  
         0.0f,  0.5f, 0.0f   
    };
    
    const char* vertexShaderSource = R"(
        #version 330 core
        layout (location = 0) in vec3 aPos;
        
        uniform float rotationAngle;
        
        void main() {
            // Rotation matrix for 2D rotation around Z-axis
            mat2 rotation = mat2(
                cos(rotationAngle), -sin(rotationAngle),
                sin(rotationAngle),  cos(rotationAngle)
            );
            
            // Apply rotation to x and y coordinates
            vec2 rotatedPos = rotation * vec2(aPos.x, aPos.y);
            
            gl_Position = vec4(rotatedPos, aPos.z, 1.0);
        }
    )";
    
    const char* fragmentShaderSource = R"(
        #version 330 core
        out vec4 FragColor;
        
        void main() {
            FragColor = vec4(0.8f, 0.3f, 0.8f, 1.0f);  // Purple color
        }
    )";
    
    unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);
    
    int success;
    char infoLog[512];
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
    }
    
    unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);
    
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
    }
    
    unsigned int shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);
    
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
    }
    
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
    
    unsigned int VAO, VBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    
    glBindVertexArray(VAO);
    
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);
    
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    
    int rotationLoc = glGetUniformLocation(shaderProgram, "rotationAngle");
    
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f); 
    

    while (!glfwWindowShouldClose(window)) {
        processInput(window);
        
        glClear(GL_COLOR_BUFFER_BIT);
        
        glUseProgram(shaderProgram);
        
        float time = glfwGetTime();
        float rotationAngle = time * 1.0f; 
        
        glUniform1f(rotationLoc, rotationAngle);
        
        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLES, 0, 3);
        
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteProgram(shaderProgram);
    
    glfwTerminate();
}

// ---------------- hw_3_3 (Scene renderer with interactive camera) ----------------
void hw_3_3(const std::vector<std::string> &params) { 
    // HW 3.3: Render a scene 
    if (params.size() == 0) {
        std::cerr << "hw_3_3: no scene file provided.\n";
        return;
    }

    // parse scene
    Scene scene = parse_scene(params[0]);
    std::cout << scene << std::endl;

    // init GLFW / GL
    if (!glfwInit()) {
        std::cerr << "Failed to init GLFW\n"; return;
    }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    int width = scene.camera.resolution.x;
    int height = scene.camera.resolution.y;
    GLFWwindow *window = glfwCreateWindow(width, height, "HW 3.3 Renderer - WSAD to move, Mouse to look", nullptr, nullptr);
    if (!window) {
        std::cerr << "Failed to create GLFW window\n"; glfwTerminate(); return;
    }
    glfwMakeContextCurrent(window);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "Failed to initialize GLAD\n"; glfwDestroyWindow(window); glfwTerminate(); return;
    }

    glViewport(0, 0, width, height);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    // Camera interaction variables
    Matrix4x4f original_cam_to_world = scene.camera.cam_to_world;
    float camera_yaw = 0.0f;
    float camera_pitch = 0.0f;
    Vector3f camera_position = Vector3f(
        original_cam_to_world(0, 3),
        original_cam_to_world(1, 3), 
        original_cam_to_world(2, 3)
    );
    
    // Mouse state
    double last_mouse_x = 0.0, last_mouse_y = 0.0;
    bool first_mouse = true;
    bool mouse_captured = false;
    
    // Camera movement speed
    float camera_speed = 0.1f;
    float mouse_sensitivity = 0.1f;

    // Mouse callback
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    
    auto update_camera_matrix = [&]() {
        // Create rotation matrix from yaw and pitch
        float cy = cos(camera_yaw);
        float sy = sin(camera_yaw);
        float cp = cos(camera_pitch);
        float sp = sin(camera_pitch);
        
        Matrix4x4f rotation = Matrix4x4f::identity();
        rotation(0, 0) = cy;
        rotation(0, 2) = sy;
        rotation(2, 0) = -sy;
        rotation(2, 2) = cy;
        
        Matrix4x4f pitch_rotation = Matrix4x4f::identity();
        pitch_rotation(1, 1) = cp;
        pitch_rotation(1, 2) = -sp;
        pitch_rotation(2, 1) = sp;
        pitch_rotation(2, 2) = cp;
        
        // Combine rotations: yaw then pitch
        Matrix4x4f total_rotation = pitch_rotation * rotation;
        
        // Apply rotation to original camera orientation, then set position
        scene.camera.cam_to_world = original_cam_to_world * total_rotation;
        scene.camera.cam_to_world(0, 3) = camera_position.x;
        scene.camera.cam_to_world(1, 3) = camera_position.y;
        scene.camera.cam_to_world(2, 3) = camera_position.z;
    };

    // simple shader (pos + color)
    const char *vs_src = R"glsl(
        #version 330 core
        layout(location=0) in vec3 aPos;
        layout(location=1) in vec3 aColor;
        uniform mat4 uMVP;
        out vec3 vColor;
        void main() {
            gl_Position = uMVP * vec4(aPos, 1.0);
            vColor = aColor;
        }
    )glsl";

    const char *fs_src = R"glsl(
        #version 330 core
        in vec3 vColor;
        out vec4 FragColor;
        uniform vec3 uDefaultColor;
        uniform bool uUseVertexColor;
        void main() {
            vec3 c = uUseVertexColor ? vColor : uDefaultColor;
            FragColor = vec4(c, 1.0);
        }
    )glsl";

    GLuint vs = compile_shader(GL_VERTEX_SHADER, vs_src);
    GLuint fs = compile_shader(GL_FRAGMENT_SHADER, fs_src);
    if (!vs || !fs) {
        if (vs) glDeleteShader(vs);
        if (fs) glDeleteShader(fs);
        glfwDestroyWindow(window); glfwTerminate(); return;
    }
    GLuint prog = link_program(vs, fs);
    glDeleteShader(vs); glDeleteShader(fs);
    if (!prog) { glfwDestroyWindow(window); glfwTerminate(); return; }

    // prepare GL meshes
    struct GLMesh {
        GLuint vao = 0, vbo = 0, ebo = 0;
        GLsizei index_count = 0;
        bool has_vertex_colors = false;
        Matrix4x4f model; // float matrix for GL
    };

    std::vector<GLMesh> glmeshes;
    glmeshes.reserve(scene.meshes.size());

    for (const auto &mesh : scene.meshes) {
        GLMesh gm;
        gm.model = mesh.model_matrix; // Matrix4x4f in hw3_scenes.h

        bool has_colors = (mesh.vertex_colors.size() == mesh.vertices.size());
        gm.has_vertex_colors = has_colors;

        // build interleaved buffer: pos(3), color(3)
        std::vector<float> vertex_data;
        vertex_data.reserve(mesh.vertices.size() * 6);
        for (size_t i = 0; i < mesh.vertices.size(); ++i) {
            const auto &v = mesh.vertices[i];
            vertex_data.push_back(v.x);
            vertex_data.push_back(v.y);
            vertex_data.push_back(v.z);
            if (has_colors) {
                const auto &c = mesh.vertex_colors[i];
                vertex_data.push_back(c.x);
                vertex_data.push_back(c.y);
                vertex_data.push_back(c.z);
            } else {
                // fallback color baked into attribute (keeps attribute present)
                vertex_data.push_back(0.7f);
                vertex_data.push_back(0.7f);
                vertex_data.push_back(0.7f);
            }
        }

        std::vector<GLuint> indices;
        indices.reserve(mesh.faces.size() * 3);
        for (const auto &f : mesh.faces) {
            indices.push_back(static_cast<GLuint>(f.x));
            indices.push_back(static_cast<GLuint>(f.y));
            indices.push_back(static_cast<GLuint>(f.z));
        }

        glGenVertexArrays(1, &gm.vao);
        glGenBuffers(1, &gm.vbo);
        glGenBuffers(1, &gm.ebo);

        glBindVertexArray(gm.vao);

        glBindBuffer(GL_ARRAY_BUFFER, gm.vbo);
        glBufferData(GL_ARRAY_BUFFER, vertex_data.size() * sizeof(float), vertex_data.data(), GL_STATIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gm.ebo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint), indices.data(), GL_STATIC_DRAW);

        GLsizei stride = 6 * sizeof(float);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride, (void*)(3 * sizeof(float)));

        glBindVertexArray(0);

        gm.index_count = static_cast<GLsizei>(indices.size());
        glmeshes.push_back(gm);
    }

    glEnable(GL_DEPTH_TEST);

    // uniform locations
    GLint loc_uMVP = glGetUniformLocation(prog, "uMVP");
    GLint loc_uDefaultColor = glGetUniformLocation(prog, "uDefaultColor");
    GLint loc_uUseVertexColor = glGetUniformLocation(prog, "uUseVertexColor");

    // compute initial view and proj
    float aspect = static_cast<float>(width) / static_cast<float>(height);
    const float fovy_deg = 45.0f;
    const float fovy = fovy_deg * (3.14159265358979323846f / 180.0f);
    const float f = 1.0f / std::tanf(fovy * 0.5f);

    Matrix4x4f proj = Matrix4x4f::identity();
    proj(0,0) = f / aspect;
    proj(1,1) = f;
    proj(2,2) = (scene.camera.z_far + scene.camera.z_near) / (scene.camera.z_near - scene.camera.z_far);
    proj(2,3) = (2.0f * scene.camera.z_far * scene.camera.z_near) / (scene.camera.z_near - scene.camera.z_far);
    proj(3,2) = -1.0f;
    proj(3,3) = 0.0f;

    // render loop
    while (!glfwWindowShouldClose(window)) {
        // Process input for camera movement
        processInput(window);
        
        // Custom input processing for camera
        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
            // Move forward - use camera's forward vector (third column)
            camera_position.x += scene.camera.cam_to_world(0, 2) * camera_speed;
            camera_position.y += scene.camera.cam_to_world(1, 2) * camera_speed;
            camera_position.z += scene.camera.cam_to_world(2, 2) * camera_speed;
        }
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
            // Move backward
            camera_position.x -= scene.camera.cam_to_world(0, 2) * camera_speed;
            camera_position.y -= scene.camera.cam_to_world(1, 2) * camera_speed;
            camera_position.z -= scene.camera.cam_to_world(2, 2) * camera_speed;
        }
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
            // Move left - use camera's right vector (first column)
            camera_position.x -= scene.camera.cam_to_world(0, 0) * camera_speed;
            camera_position.y -= scene.camera.cam_to_world(1, 0) * camera_speed;
            camera_position.z -= scene.camera.cam_to_world(2, 0) * camera_speed;
        }
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
            // Move right
            camera_position.x += scene.camera.cam_to_world(0, 0) * camera_speed;
            camera_position.y += scene.camera.cam_to_world(1, 0) * camera_speed;
            camera_position.z += scene.camera.cam_to_world(2, 0) * camera_speed;
        }
        if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS) {
            // Move up - use camera's up vector (second column)
            camera_position.x += scene.camera.cam_to_world(0, 1) * camera_speed;
            camera_position.y += scene.camera.cam_to_world(1, 1) * camera_speed;
            camera_position.z += scene.camera.cam_to_world(2, 1) * camera_speed;
        }
        if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS) {
            // Move down
            camera_position.x -= scene.camera.cam_to_world(0, 1) * camera_speed;
            camera_position.y -= scene.camera.cam_to_world(1, 1) * camera_speed;
            camera_position.z -= scene.camera.cam_to_world(2, 1) * camera_speed;
        }
        
        // Mouse capture toggle
        if (glfwGetKey(window, GLFW_KEY_TAB) == GLFW_PRESS) {
            static bool debounce = false;
            if (!debounce) {
                mouse_captured = !mouse_captured;
                glfwSetInputMode(window, GLFW_CURSOR, mouse_captured ? GLFW_CURSOR_DISABLED : GLFW_CURSOR_NORMAL);
                first_mouse = true;
                debounce = true;
            }
        }
        if (glfwGetKey(window, GLFW_KEY_TAB) == GLFW_RELEASE) {
            static bool debounce = false;
            debounce = false;
        }
        
        // Mouse look
        if (mouse_captured) {
            double mouse_x, mouse_y;
            glfwGetCursorPos(window, &mouse_x, &mouse_y);
            
            if (first_mouse) {
                last_mouse_x = mouse_x;
                last_mouse_y = mouse_y;
                first_mouse = false;
            }
            
            double x_offset = mouse_x - last_mouse_x;
            double y_offset = last_mouse_y - mouse_y; // reversed since y-coordinates go from bottom to top
            
            last_mouse_x = mouse_x;
            last_mouse_y = mouse_y;
            
            x_offset *= mouse_sensitivity;
            y_offset *= mouse_sensitivity;
            
            camera_yaw += x_offset * 0.01f;
            camera_pitch += y_offset * 0.01f;
            
            // Constrain pitch to avoid flipping
            if (camera_pitch > 1.5f) camera_pitch = 1.5f;
            if (camera_pitch < -1.5f) camera_pitch = -1.5f;
        }
        
        // Update camera matrix
        update_camera_matrix();

        // Compute current view matrix
        Matrix4x4f view = inverse(scene.camera.cam_to_world);

        glClearColor(scene.background.x, scene.background.y, scene.background.z, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glUseProgram(prog);

        for (const auto &gm : glmeshes) {
            Matrix4x4f MVP = proj * (view * gm.model);
            glUniformMatrix4fv(loc_uMVP, 1, GL_FALSE, MVP.ptr());

            glUniform1i(loc_uUseVertexColor, gm.has_vertex_colors ? GL_TRUE : GL_FALSE);
            if (!gm.has_vertex_colors) {
                glUniform3f(loc_uDefaultColor, 0.7f, 0.7f, 0.7f);
            }

            glBindVertexArray(gm.vao);
            glDrawElements(GL_TRIANGLES, gm.index_count, GL_UNSIGNED_INT, (void*)0);
            glBindVertexArray(0);
        }

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // cleanup
    for (auto &gm : glmeshes) {
        if (gm.ebo) glDeleteBuffers(1, &gm.ebo);
        if (gm.vbo) glDeleteBuffers(1, &gm.vbo);
        if (gm.vao) glDeleteVertexArrays(1, &gm.vao);
    }
    glDeleteProgram(prog);
    glfwDestroyWindow(window);
    glfwTerminate();
}

// ---------------- hw_3_4 (Scene renderer with lighting) ----------------
void hw_3_4(const std::vector<std::string> &params) {
    // HW 3.4: Render a scene with basic Phong lighting
    if (params.size() == 0) {
        std::cerr << "hw_3_4: no scene file provided.\n";
        return;
    }

    Scene scene = parse_scene(params[0]);
    std::cout << scene << std::endl;

    // --- Init GLFW / GL (same pattern as hw_3_3) ---
    if (!glfwInit()) {
        std::cerr << "Failed to init GLFW\n"; return;
    }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    int width = scene.camera.resolution.x;
    int height = scene.camera.resolution.y;
    GLFWwindow *window = glfwCreateWindow(width, height, "HW 3.4 Lighting", nullptr, nullptr);
    if (!window) { std::cerr << "Failed to create GLFW window\n"; glfwTerminate(); return; }
    glfwMakeContextCurrent(window);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "Failed to initialize GLAD\n"; glfwDestroyWindow(window); glfwTerminate(); return;
    }
    glViewport(0, 0, width, height);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    // --- Camera interaction variables (same design as hw_3_3) ---
    Matrix4x4f original_cam_to_world = scene.camera.cam_to_world;
    float camera_yaw = 0.0f;
    float camera_pitch = 0.0f;
    Vector3f camera_position = Vector3f(
        original_cam_to_world(0, 3),
        original_cam_to_world(1, 3),
        original_cam_to_world(2, 3)
    );

    // Mouse & control state
    double last_mouse_x = 0.0, last_mouse_y = 0.0;
    bool first_mouse = true;
    bool mouse_captured = false;
    float camera_speed = 0.1f;
    float mouse_sensitivity = 0.1f;

    // Start with cursor free; TAB toggles capture
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);

    auto update_camera_matrix = [&]() {
        // Build yaw (around world up) and pitch (around camera right) rotations
        float cy = cos(camera_yaw);
        float sy = sin(camera_yaw);
        float cp = cos(camera_pitch);
        float sp = sin(camera_pitch);

        // yaw rotation (Y axis)
        Matrix4x4f yawR = Matrix4x4f::identity();
        yawR(0,0) = cy; yawR(0,2) = sy;
        yawR(2,0) = -sy; yawR(2,2) = cy;

        // pitch rotation (X axis)
        Matrix4x4f pitchR = Matrix4x4f::identity();
        pitchR(1,1) = cp; pitchR(1,2) = -sp;
        pitchR(2,1) = sp; pitchR(2,2) = cp;

        Matrix4x4f totalR = pitchR * yawR; // yaw then pitch

        // apply rotation to original orientation, then set position
        scene.camera.cam_to_world = original_cam_to_world * totalR;
        scene.camera.cam_to_world(0,3) = camera_position.x;
        scene.camera.cam_to_world(1,3) = camera_position.y;
        scene.camera.cam_to_world(2,3) = camera_position.z;
    };

    // --- Shaders (vertex + fragment with normals & lighting) ---
    const char *vs_src = R"glsl(
        #version 330 core
        layout(location=0) in vec3 aPos;
        layout(location=1) in vec3 aNormal;
        layout(location=2) in vec3 aColor;

        uniform mat4 model;
        uniform mat4 view;
        uniform mat4 projection;
        uniform mat3 normalMatrix; // mat3 to transform normals into world space

        out vec3 FragPos;   // world-space position
        out vec3 Normal;    // world-space normal
        out vec3 vColor;

        void main() {
            vec4 worldPos = model * vec4(aPos, 1.0);
            FragPos = worldPos.xyz;
            Normal = normalize(normalMatrix * aNormal);
            vColor = aColor;
            gl_Position = projection * view * worldPos;
        }
    )glsl";

    const char *fs_src = R"glsl(
        #version 330 core
        in vec3 FragPos;
        in vec3 Normal;
        in vec3 vColor;

        out vec4 FragColor;

        // lighting parameters
        uniform vec3 lightDir; // directional light (assumed normalized)
        uniform vec3 viewPos;  // camera world position
        uniform float ambientStrength;
        uniform float specularStrength;
        uniform bool useVertexColor; // whether to use per-vertex color; otherwise use uObjectColor
        uniform vec3 uObjectColor;

        void main() {
            vec3 objectColor = useVertexColor ? vColor : uObjectColor;

            // ambient
            vec3 ambient = ambientStrength * vec3(1.0);

            // diffuse
            vec3 norm = normalize(Normal);
            // for directional light, lightDir is direction FROM which light comes (pointing toward scene)
            vec3 L = normalize(lightDir); // already normalized, but keep formal
            float diff = max(dot(norm, L), 0.0);
            vec3 diffuse = diff * vec3(1.0);

            // specular (Blinn-Phong style could also be used; we do reflect + pow)
            vec3 viewDir = normalize(viewPos - FragPos);
            vec3 reflectDir = reflect(-L, norm);
            float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32.0);
            vec3 specular = specularStrength * spec * vec3(1.0);

            vec3 result = (ambient + diffuse + specular) * objectColor;
            FragColor = vec4(result, 1.0);
        }
    )glsl";

    GLuint vs = compile_shader(GL_VERTEX_SHADER, vs_src);
    GLuint fs = compile_shader(GL_FRAGMENT_SHADER, fs_src);
    if (!vs || !fs) {
        if (vs) glDeleteShader(vs);
        if (fs) glDeleteShader(fs);
        glfwDestroyWindow(window); glfwTerminate(); return;
    }
    GLuint prog = link_program(vs, fs);
    glDeleteShader(vs); glDeleteShader(fs);
    if (!prog) { glfwDestroyWindow(window); glfwTerminate(); return; }

    // --- Prepare GL meshes (pos, normal, color interleaved) ---
    struct GLMesh {
        GLuint vao = 0, vbo = 0, ebo = 0;
        GLsizei index_count = 0;
        Matrix4x4f model;
        bool has_vertex_color = false;
    };
    std::vector<GLMesh> glmeshes;
    glmeshes.reserve(scene.meshes.size());

    for (const auto &mesh : scene.meshes) {
        // Copy mesh to mutable for computing normals if necessary
        TriangleMesh local = mesh;

        // If vertex normals missing, compute per-vertex normals by averaging face normals
        if (local.vertex_normals.empty()) {
            local.vertex_normals.resize(local.vertices.size(), Vector3f{0,0,0});
            for (const auto &f : local.faces) {
                Vector3f v0 = local.vertices[f.x];
                Vector3f v1 = local.vertices[f.y];
                Vector3f v2 = local.vertices[f.z];
                Vector3f face_normal = normalize(cross(v1 - v0, v2 - v0));
                local.vertex_normals[f.x] += face_normal;
                local.vertex_normals[f.y] += face_normal;
                local.vertex_normals[f.z] += face_normal;
            }
            for (size_t i = 0; i < local.vertex_normals.size(); ++i) {
                local.vertex_normals[i] = normalize(local.vertex_normals[i]);
            }
        }

        bool has_colors = (local.vertex_colors.size() == local.vertices.size());

        // Build interleaved vertex buffer: pos(3), normal(3), color(3)
        std::vector<float> vdata;
        vdata.reserve(local.vertices.size() * 9);
        for (size_t i = 0; i < local.vertices.size(); ++i) {
            const auto &p = local.vertices[i];
            const auto &n = local.vertex_normals[i];
            Vector3f c = has_colors ? local.vertex_colors[i] : Vector3f{0.7f, 0.7f, 0.7f};
            vdata.push_back(p.x); vdata.push_back(p.y); vdata.push_back(p.z);
            vdata.push_back(n.x); vdata.push_back(n.y); vdata.push_back(n.z);
            vdata.push_back(c.x); vdata.push_back(c.y); vdata.push_back(c.z);
        }

        std::vector<GLuint> indices;
        indices.reserve(local.faces.size() * 3);
        for (const auto &f : local.faces) {
            indices.push_back(static_cast<GLuint>(f.x));
            indices.push_back(static_cast<GLuint>(f.y));
            indices.push_back(static_cast<GLuint>(f.z));
        }

        GLMesh gm;
        gm.model = local.model_matrix;
        gm.has_vertex_color = has_colors;

        glGenVertexArrays(1, &gm.vao);
        glGenBuffers(1, &gm.vbo);
        glGenBuffers(1, &gm.ebo);

        glBindVertexArray(gm.vao);

        glBindBuffer(GL_ARRAY_BUFFER, gm.vbo);
        glBufferData(GL_ARRAY_BUFFER, vdata.size() * sizeof(float), vdata.data(), GL_STATIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gm.ebo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint), indices.data(), GL_STATIC_DRAW);

        GLsizei stride = 9 * sizeof(float);
        // position
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*)0);
        // normal
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride, (void*)(3 * sizeof(float)));
        // color
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, stride, (void*)(6 * sizeof(float)));

        glBindVertexArray(0);

        gm.index_count = static_cast<GLsizei>(indices.size());
        glmeshes.push_back(gm);
    }

    glEnable(GL_DEPTH_TEST);

    // --- Uniform locations ---
    GLint loc_model = glGetUniformLocation(prog, "model");
    GLint loc_view = glGetUniformLocation(prog, "view");
    GLint loc_proj = glGetUniformLocation(prog, "projection");
    GLint loc_normalMatrix = glGetUniformLocation(prog, "normalMatrix");
    GLint loc_lightDir = glGetUniformLocation(prog, "lightDir");
    GLint loc_viewPos = glGetUniformLocation(prog, "viewPos");
    GLint loc_ambientStrength = glGetUniformLocation(prog, "ambientStrength");
    GLint loc_specularStrength = glGetUniformLocation(prog, "specularStrength");
    GLint loc_useVertexColor = glGetUniformLocation(prog, "useVertexColor");
    GLint loc_uObjectColor = glGetUniformLocation(prog, "uObjectColor");

    // Fixed lighting constants
    Vector3f lightDirVec = Vector3f{1.0f, 1.0f, 1.0f};
    // normalize
    {
        float mag = std::sqrt(lightDirVec.x*lightDirVec.x + lightDirVec.y*lightDirVec.y + lightDirVec.z*lightDirVec.z);
        lightDirVec.x /= mag; lightDirVec.y /= mag; lightDirVec.z /= mag;
    }
    const float ambientStrength = 0.1f;
    const float specularStrength = 0.5f;

    // Projection matrix (perspective)
    float aspect = static_cast<float>(width) / static_cast<float>(height);
    const float fovy_deg = 45.0f;
    const float fovy = fovy_deg * (3.14159265358979323846f / 180.0f);
    const float f = 1.0f / std::tanf(fovy * 0.5f);

    Matrix4x4f proj = Matrix4x4f::identity();
    proj(0,0) = f / aspect;
    proj(1,1) = f;
    proj(2,2) = (scene.camera.z_far + scene.camera.z_near) / (scene.camera.z_near - scene.camera.z_far);
    proj(2,3) = (2.0f * scene.camera.z_far * scene.camera.z_near) / (scene.camera.z_near - scene.camera.z_far);
    proj(3,2) = -1.0f;
    proj(3,3) = 0.0f;

    // --- Render loop ---
    while (!glfwWindowShouldClose(window)) {
        // Process system-level input (escape)
        processInput(window);

        // --- Camera movement (WSAD + QE) - using cam_to_world columns like hw_3_3 ---
        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
            camera_position.x += scene.camera.cam_to_world(0,2) * camera_speed;
            camera_position.y += scene.camera.cam_to_world(1,2) * camera_speed;
            camera_position.z += scene.camera.cam_to_world(2,2) * camera_speed;
        }
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
            camera_position.x -= scene.camera.cam_to_world(0,2) * camera_speed;
            camera_position.y -= scene.camera.cam_to_world(1,2) * camera_speed;
            camera_position.z -= scene.camera.cam_to_world(2,2) * camera_speed;
        }
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
            camera_position.x -= scene.camera.cam_to_world(0,0) * camera_speed;
            camera_position.y -= scene.camera.cam_to_world(1,0) * camera_speed;
            camera_position.z -= scene.camera.cam_to_world(2,0) * camera_speed;
        }
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
            camera_position.x += scene.camera.cam_to_world(0,0) * camera_speed;
            camera_position.y += scene.camera.cam_to_world(1,0) * camera_speed;
            camera_position.z += scene.camera.cam_to_world(2,0) * camera_speed;
        }
        if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS) {
            camera_position.x += scene.camera.cam_to_world(0,1) * camera_speed;
            camera_position.y += scene.camera.cam_to_world(1,1) * camera_speed;
            camera_position.z += scene.camera.cam_to_world(2,1) * camera_speed;
        }
        if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS) {
            camera_position.x -= scene.camera.cam_to_world(0,1) * camera_speed;
            camera_position.y -= scene.camera.cam_to_world(1,1) * camera_speed;
            camera_position.z -= scene.camera.cam_to_world(2,1) * camera_speed;
        }

        // TAB toggles mouse capture (debounced)
        static bool tab_debounce = false;
        if (glfwGetKey(window, GLFW_KEY_TAB) == GLFW_PRESS) {
            if (!tab_debounce) {
                mouse_captured = !mouse_captured;
                glfwSetInputMode(window, GLFW_CURSOR, mouse_captured ? GLFW_CURSOR_DISABLED : GLFW_CURSOR_NORMAL);
                first_mouse = true;
                tab_debounce = true;
            }
        }
        if (glfwGetKey(window, GLFW_KEY_TAB) == GLFW_RELEASE) tab_debounce = false;

        // Mouse look when captured
        if (mouse_captured) {
            double mouse_x, mouse_y;
            glfwGetCursorPos(window, &mouse_x, &mouse_y);
            if (first_mouse) {
                last_mouse_x = mouse_x;
                last_mouse_y = mouse_y;
                first_mouse = false;
            }
            double x_offset = mouse_x - last_mouse_x;
            double y_offset = last_mouse_y - mouse_y;
            last_mouse_x = mouse_x;
            last_mouse_y = mouse_y;
            x_offset *= mouse_sensitivity;
            y_offset *= mouse_sensitivity;
            camera_yaw += static_cast<float>(x_offset * 0.01);
            camera_pitch += static_cast<float>(y_offset * 0.01);
            // constrain pitch
            if (camera_pitch > 1.5f) camera_pitch = 1.5f;
            if (camera_pitch < -1.5f) camera_pitch = -1.5f;
        }

        // Apply yaw/pitch to camera transform & update scene.camera.cam_to_world
        update_camera_matrix();

        // Compute view & view pos
        Matrix4x4f view = inverse(scene.camera.cam_to_world);
        Vector3f camPos = Vector3f(
            scene.camera.cam_to_world(0,3),
            scene.camera.cam_to_world(1,3),
            scene.camera.cam_to_world(2,3)
        );

        // Clear
        glClearColor(scene.background.x, scene.background.y, scene.background.z, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glUseProgram(prog);

        // set global lighting uniforms (directional)
        glUniform3f(loc_lightDir, lightDirVec.x, lightDirVec.y, lightDirVec.z);
        glUniform1f(loc_ambientStrength, ambientStrength);
        glUniform1f(loc_specularStrength, specularStrength);
        glUniform3f(loc_viewPos, camPos.x, camPos.y, camPos.z);
        glUniformMatrix4fv(loc_view, 1, GL_FALSE, view.ptr());
        glUniformMatrix4fv(loc_proj, 1, GL_FALSE, proj.ptr());
        // default object color if no vertex colors present
        glUniform3f(loc_uObjectColor, 0.7f, 0.7f, 0.7f);

        // Draw meshes
        for (const auto &gm : glmeshes) {
            // model matrix (Matrix4x4f)
            Matrix4x4f model = gm.model;
            glUniformMatrix4fv(loc_model, 1, GL_FALSE, model.ptr());

            // compute normal matrix (3x3) = transpose(inverse(upper-left 3x3 of model))
            Matrix3x3f upper;
            for (int r = 0; r < 3; ++r)
                for (int c = 0; c < 3; ++c)
                    upper(r,c) = model(r,c);

            Matrix3x3f inv_upper = inverse(upper);
            // transpose inv_upper to get normal matrix
            float normalMat[9];
            for (int r = 0; r < 3; ++r) {
                for (int c = 0; c < 3; ++c) {
                    // normalMatrix should be mat3(transpose(inverse(...)))
                    normalMat[c*3 + r] = inv_upper(r,c); // column-major store of transpose(inv)
                }
            }
            glUniformMatrix3fv(loc_normalMatrix, 1, GL_FALSE, normalMat);

            // whether to use vertex color buffer or uniform color
            glUniform1i(loc_useVertexColor, gm.has_vertex_color ? GL_TRUE : GL_FALSE);

            glBindVertexArray(gm.vao);
            glDrawElements(GL_TRIANGLES, gm.index_count, GL_UNSIGNED_INT, (void*)0);
            glBindVertexArray(0);
        }

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // cleanup
    for (auto &gm : glmeshes) {
        if (gm.ebo) glDeleteBuffers(1, &gm.ebo);
        if (gm.vbo) glDeleteBuffers(1, &gm.vbo);
        if (gm.vao) glDeleteVertexArrays(1, &gm.vao);
    }
    glDeleteProgram(prog);
    glfwDestroyWindow(window);
    glfwTerminate();
}
