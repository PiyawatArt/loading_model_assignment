#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <learnopengl/filesystem.h>
#include <learnopengl/shader_m.h>
#include <learnopengl/camera.h>
#include <learnopengl/model.h>

#include <iostream>
#include <vector>
#include <cstdlib>
#include <ctime>

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow* window);
void renderCube();
void renderSphere();

// settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

// camera
Camera camera(glm::vec3(0.0f, 25.0f, 35.0f));
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;
float cameraDistance = 35.0f;
float cameraHeight = 25.0f;

// timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;

// ตัวแปรควบคุมรถ
glm::vec3 carPosition = glm::vec3(0.0f, 0.0f, 0.0f);
float carRotationY = 0.0f;
float carSpeed = 0.0f;
const float maxSpeed = 25.0f;
const float acceleration = 30.0f;
const float deceleration = 15.0f;
const float turnSpeed = 180.0f;

// คะแนน
int playerScore = 0;

// โครงสร้างไอเทม
struct Item {
    glm::vec3 position;
    glm::vec3 color;
    bool active;
};

std::vector<Item> items;
const int MAX_ITEMS = 12; // เพิ่มเป็น 12 ลูก
const float ITEM_COLLECT_RADIUS = 5.0f; // เพิ่มรัศมีให้ใหญ่ขึ้น
const float GROUND_SIZE = 500.0f;
const float ITEM_SPAWN_RANGE = 35.0f; // ลดระยะลงให้ใกล้รถมากขึ้น (เห็นชัดเจน)

// VAO สำหรับ primitive shapes
unsigned int cubeVAO = 0;
unsigned int cubeVBO = 0;
unsigned int sphereVAO = 0;
unsigned int sphereVBO = 0;

// ฟังก์ชันสุ่มตำแหน่งไอเทม (สุ่มรอบๆรถ ไม่ไกลเกินไป)
glm::vec3 randomItemPosition() {
    float offsetX = ((rand() % 200) - 100) / 100.0f * ITEM_SPAWN_RANGE;
    float offsetZ = ((rand() % 200) - 100) / 100.0f * ITEM_SPAWN_RANGE;
    return glm::vec3(carPosition.x + offsetX, 1.5f, carPosition.z + offsetZ); // ลดความสูงลง
}

// ฟังก์ชันสุ่มสี
glm::vec3 randomColor() {
    glm::vec3 colors[] = {
        glm::vec3(1.0f, 0.0f, 0.0f),  // แดง
        glm::vec3(0.0f, 1.0f, 0.0f),  // เขียว
        glm::vec3(0.0f, 0.0f, 1.0f),  // น้ำเงิน
        glm::vec3(1.0f, 1.0f, 0.0f),  // เหลือง
        glm::vec3(1.0f, 0.0f, 1.0f),  // ม่วง
        glm::vec3(0.0f, 1.0f, 1.0f),  // ฟ้า
        glm::vec3(1.0f, 0.5f, 0.0f),  // ส้ม
        glm::vec3(1.0f, 0.75f, 0.8f)  // ชมพู
    };
    return colors[rand() % 8];
}

// สร้างไอเทมใหม่
void spawnItem(int index) {
    items[index].position = randomItemPosition();
    items[index].color = randomColor();
    items[index].active = true;
    std::cout << "Item " << index << " spawned at: "
        << items[index].position.x << ", "
        << items[index].position.y << ", "
        << items[index].position.z << std::endl;
}

// ตรวจสอบการชนไอเทม
void checkItemCollision() {
    for (int i = 0; i < MAX_ITEMS; i++) {
        if (items[i].active) {
            // คำนวณระยะในแนว XZ เท่านั้น (ไม่สนใจความสูง Y)
            glm::vec3 carPos2D = glm::vec3(carPosition.x, 0.0f, carPosition.z);
            glm::vec3 itemPos2D = glm::vec3(items[i].position.x, 0.0f, items[i].position.z);
            float distance = glm::length(carPos2D - itemPos2D);

            // Debug: แสดงระยะห่าง
            if (distance < 10.0f) {
                std::cout << "Distance to item " << i << ": " << distance << std::endl;
            }

            if (distance < ITEM_COLLECT_RADIUS) {
                // ลูกบอลหายไป
                items[i].active = false;

                // เพิ่มคะแนน
                playerScore++;
                std::cout << "*** ITEM COLLECTED! Score: " << playerScore << " ***" << std::endl;

                // สร้างลูกบอลใหม่ทันที
                spawnItem(i);
            }
        }
    }
}

int main()
{
    srand(time(NULL));

    // glfw: initialize and configure
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // glfw window creation
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Car Game - Score: 0", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);

    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    stbi_set_flip_vertically_on_load(false);

    glEnable(GL_DEPTH_TEST);

    // build and compile shaders
    Shader ourShader("1.model_loading.vs", "1.model_loading.fs");

    // load car model
    Model ourModel(FileSystem::getPath("resources/objects/test_model3/ElderStreetOldCar03.obj"));

    // สร้างไอเทมเริ่มต้น
    items.resize(MAX_ITEMS);
    for (int i = 0; i < MAX_ITEMS; i++) {
        spawnItem(i);
    }

    // render loop
    while (!glfwWindowShouldClose(window))
    {
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        processInput(window);

        // อัพเดทการเคลื่อนที่ของรถ
        if (carSpeed != 0.0f) {
            float radians = glm::radians(carRotationY);
            carPosition.x += sin(radians) * carSpeed * deltaTime;
            carPosition.z += cos(radians) * carSpeed * deltaTime;

            if (carSpeed > 0.0f) {
                carSpeed -= deceleration * deltaTime;
                if (carSpeed < 0.0f) carSpeed = 0.0f;
            }
            else if (carSpeed < 0.0f) {
                carSpeed += deceleration * deltaTime;
                if (carSpeed > 0.0f) carSpeed = 0.0f;
            }
        }

        // ตรวจสอบการชนไอเทม
        checkItemCollision();

        // อัพเดทตำแหน่งกล้องให้ตามรถ
        glm::vec3 cameraTarget = carPosition;
        float radians = glm::radians(carRotationY);
        glm::vec3 cameraPos = carPosition;
        cameraPos.x -= sin(radians) * cameraDistance;
        cameraPos.z -= cos(radians) * cameraDistance;
        cameraPos.y = cameraHeight;

        camera.Position = cameraPos;
        camera.Front = glm::normalize(cameraTarget - cameraPos);

        // อัพเดทชื่อ window ให้แสดงคะแนน
        std::string title = "Car Game - Score: " + std::to_string(playerScore);
        glfwSetWindowTitle(window, title.c_str());

        // render
        glClearColor(0.1f, 0.15f, 0.2f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 1000.0f);
        glm::mat4 view = camera.GetViewMatrix();

        // วาดพื้นถนน (ใหญ่มาก ไม่มีที่สิ้นสุด)
        ourShader.use();
        ourShader.setMat4("projection", projection);
        ourShader.setMat4("view", view);
        ourShader.setInt("useTexture", 0);  // 0 = false

        // วาดพื้นหลายชิ้นรอบๆรถ
        for (int x = -2; x <= 2; x++) {
            for (int z = -2; z <= 2; z++) {
                glm::mat4 groundModel = glm::mat4(1.0f);
                groundModel = glm::translate(groundModel, glm::vec3(
                    floor(carPosition.x / GROUND_SIZE) * GROUND_SIZE + x * GROUND_SIZE,
                    -0.01f,
                    floor(carPosition.z / GROUND_SIZE) * GROUND_SIZE + z * GROUND_SIZE
                ));
                groundModel = glm::scale(groundModel, glm::vec3(GROUND_SIZE, 0.1f, GROUND_SIZE));
                ourShader.setMat4("model", groundModel);
                ourShader.setVec3("objectColor", glm::vec3(0.3f, 0.3f, 0.35f));
                renderCube();
            }
        }

        // วาดเส้นถนน (เยอะมาก)
        for (int i = -50; i <= 50; i++) {
            if (i == 0) continue;
            glm::mat4 lineModel = glm::mat4(1.0f);
            lineModel = glm::translate(lineModel, glm::vec3(i * 5.0f, 0.0f, carPosition.z));
            lineModel = glm::scale(lineModel, glm::vec3(0.3f, 0.05f, GROUND_SIZE * 2));
            ourShader.setMat4("model", lineModel);
            ourShader.setVec3("objectColor", glm::vec3(0.9f, 0.9f, 0.9f));
            renderCube();
        }

        // วาดไอเทม (เฉพาะที่ active เท่านั้น)
        for (int i = 0; i < MAX_ITEMS; i++) {
            if (items[i].active) {  // วาดเฉพาะลูกบอลที่ยังไม่โดนเก็บ
                glm::mat4 itemModel = glm::mat4(1.0f);
                itemModel = glm::translate(itemModel, items[i].position);

                // หมุนไอเทมให้ดูสวย + ขึ้นลงเล็กน้อย
                float rotation = currentFrame * 80.0f;
                float bounce = sin(currentFrame * 2.0f) * 0.5f;
                itemModel = glm::translate(itemModel, glm::vec3(0, bounce, 0));
                itemModel = glm::rotate(itemModel, glm::radians(rotation), glm::vec3(0, 1, 0));

                // ขยายให้ใหญ่มากๆ
                itemModel = glm::scale(itemModel, glm::vec3(3.0f));
                ourShader.setMat4("model", itemModel);
                ourShader.setVec3("objectColor", items[i].color);
                renderSphere();
            }
        }

        // วาดรถ
        ourShader.use();
        ourShader.setBool("useTexture", true);
        ourShader.setMat4("projection", projection);
        ourShader.setMat4("view", view);

        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, carPosition);
        model = glm::rotate(model, glm::radians(carRotationY), glm::vec3(0, 1, 0));
        model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(1, 0, 0));
        model = glm::rotate(model, glm::radians(90.0f), glm::vec3(0, 0, 1));
        model = glm::rotate(model, glm::radians(180.0f), glm::vec3(0, 1, 0));
        model = glm::rotate(model, glm::radians(180.0f), glm::vec3(1, 0, 0));
        model = glm::scale(model, glm::vec3(0.6f));

        ourShader.setMat4("model", model);
        ourModel.Draw(ourShader);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}

void processInput(GLFWwindow* window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    // W = เดินหน้า
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
        carSpeed += acceleration * deltaTime;
        if (carSpeed > maxSpeed) carSpeed = maxSpeed;
    }

    // S = ถอยหลัง
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
        carSpeed -= acceleration * deltaTime;
        if (carSpeed < -maxSpeed) carSpeed = -maxSpeed;
    }

    // A = เลี้ยวซ้าย
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS && carSpeed != 0.0f) {
        if (carSpeed > 0) {
            carRotationY += turnSpeed * deltaTime;
        }
        else {
            carRotationY -= turnSpeed * deltaTime;
        }
    }

    // D = เลี้ยวขวา
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS && carSpeed != 0.0f) {
        if (carSpeed > 0) {
            carRotationY -= turnSpeed * deltaTime;
        }
        else {
            carRotationY += turnSpeed * deltaTime;
        }
    }
}

// ฟังก์ชันวาด Cube
void renderCube()
{
    if (cubeVAO == 0)
    {
        float vertices[] = {
            -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
             0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
             0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
             0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
            -0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
            -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,

            -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
             0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
             0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
             0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
            -0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
            -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,

            -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,
            -0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
            -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
            -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
            -0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f,
            -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,

             0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,
             0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
             0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
             0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
             0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,
             0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,

            -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,
             0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,
             0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
             0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
            -0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
            -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,

            -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,
             0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,
             0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
             0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
            -0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
            -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f
        };
        glGenVertexArrays(1, &cubeVAO);
        glGenBuffers(1, &cubeVBO);
        glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
        glBindVertexArray(cubeVAO);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
    }
    glBindVertexArray(cubeVAO);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glBindVertexArray(0);
}

// ฟังก์ชันวาด Sphere
void renderSphere()
{
    if (sphereVAO == 0)
    {
        std::vector<float> vertices;
        const int stacks = 20;
        const int slices = 20;
        const float PI = 3.14159265359f;

        for (int i = 0; i <= stacks; ++i) {
            float phi = PI * float(i) / float(stacks);
            for (int j = 0; j <= slices; ++j) {
                float theta = 2.0f * PI * float(j) / float(slices);

                float x = sin(phi) * cos(theta);
                float y = cos(phi);
                float z = sin(phi) * sin(theta);

                vertices.push_back(x * 0.5f);
                vertices.push_back(y * 0.5f);
                vertices.push_back(z * 0.5f);
                vertices.push_back(x);
                vertices.push_back(y);
                vertices.push_back(z);
            }
        }

        glGenVertexArrays(1, &sphereVAO);
        glGenBuffers(1, &sphereVBO);
        glBindBuffer(GL_ARRAY_BUFFER, sphereVBO);
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), &vertices[0], GL_STATIC_DRAW);
        glBindVertexArray(sphereVAO);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    }

    glBindVertexArray(sphereVAO);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, (20 + 1) * (20 + 1));
    glBindVertexArray(0);
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}

void mouse_callback(GLFWwindow* window, double xposIn, double yposIn)
{
    float xpos = static_cast<float>(xposIn);
    float ypos = static_cast<float>(yposIn);

    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos;

    lastX = xpos;
    lastY = ypos;

    camera.ProcessMouseMovement(xoffset, yoffset);
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    camera.ProcessMouseScroll(static_cast<float>(yoffset));
}