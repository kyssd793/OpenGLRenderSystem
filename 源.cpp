#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Shader.h"
#include "Model.h"
#include "SceneManager.h"  // 场景管理器头文件
#include "camera.h"
#include "Light.h"
#include <iostream>
#include "ShadowMapper.h"
#include "IBL.h"

// 窗口设置
const unsigned int SCR_WIDTH = 1280;
const unsigned int SCR_HEIGHT = 720;

// 相机设置
//使用封装好的camera
Camera* camera;
IBL* iblSystem = nullptr;  // 新增IBL系统

float deltaTime = 0.0f;
float lastFrame = 0.0f;
float brightness = 1.0f;

// MSAA帧缓冲对象
unsigned int msaaFBO;
unsigned int msaaColorBuffer;
unsigned int msaaRBO;

// 回调函数声明
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void setupMSAAFramebuffer(unsigned int width, unsigned int height);
void deleteMSAAFramebuffer();

// 全局变量区域添加
static float normalStrength = 0.8f;
static float aoStrength = 0.7f;
int main() {


    // 1. 初始化GLFW
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_SAMPLES, 4); // 启用4x MSAA

    // 2. 创建窗口
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "OpenGL Renderer", NULL, NULL);
    if (window == NULL) {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // 3. 初始化GLAD
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // 4. 配置全局OpenGL状态
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_MULTISAMPLE); // 启用多重采样

    // 创建MSAA帧缓冲
    setupMSAAFramebuffer(SCR_WIDTH, SCR_HEIGHT);

    // 5. 创建着色器
    Shader ourShader("shaders/shader.vert", "shaders/shader.frag");
    // 创建PBR着色器
    Shader pbrShader("shaders/pbr.vert", "shaders/pbr.frag");
    // 创建深度着色器
    Shader depthShader("shaders/depth.vert", "shaders/depth.frag");



    // 6. 创建场景管理器并构建场景
    SceneManager scene;

    // IBL初始化
    iblSystem = new IBL("textures/industrial_workshop_foundry_4k.hdr");
    

    // 创建阴影映射器
    ShadowMapper shadowMapper;


    //7.创建摄像机
    camera = new Camera(window, glm::vec3(0.0f, 0.0f, 5.0f));

    // 8.使用代码生成平面节点
    SceneNode& floorNode = scene.CreatePrimitiveNode("Floor", SceneManager::PrimitiveType::PLANE);
    floorNode.SetPosition(glm::vec3(0.0f, -1.5f, 0.0f));
    floorNode.SetScale(glm::vec3(5.0f, 1.0f, 5.0f)); // 放大平面



    // 9.创建主模型节点
    auto nanosuitNode = scene.CreateModelNode("Nanosuit", "models/nanosuit/nanosuit.obj");
    nanosuitNode->SetPosition(glm::vec3(0.0f, -1.0f, 0.0f));
    nanosuitNode->SetScale(glm::vec3(0.4f));

    //// 创建第二个模型节点
    //auto secondSuit = scene.CreateModelNode("SecondSuit", "models/nanosuit/nanosuit.obj");
    //secondSuit->SetPosition(glm::vec3(3.0f, -1.0f, 0.0f));
    //secondSuit->SetRotation(45.0f, glm::vec3(0.0f, 1.0f, 0.0f));
    // 创建第二个模型节点（使用PBR模型）
    auto secondSuit = scene.CreateModelNode("ToyCar", "models/ToyCar/glTF/ToyCar.gltf");
    // 修复材质参数设置：
    //secondSuit->GetMaterial().metallic = 0.05f;  // 极低金属度
    //secondSuit->GetMaterial().roughness = 0.85f; // 高粗糙度
    //secondSuit->GetMaterial().ao = 0.8f;        // 降低基础AO值
    secondSuit->SetPosition(glm::vec3(3.0f, 0.0f, 0.0f));
    secondSuit->SetRotation(90.0f, glm::vec3(1.0f, 0.0f, 0.0f));  // 绕X轴旋转90度
    secondSuit->SetScale(glm::vec3(0.01f));  // 缩小到5%
    //secondSuit->GetMaterial().isPBR = true;    // 设置材质为PBR
    //secondSuit->GetMaterial().metallic = 0.7f;
    //secondSuit->GetMaterial().roughness = 0.3f;
    //secondSuit->GetMaterial().ao = 1.0f;
        // 设置PBR材质参数（关键修改）
    Material& carMaterial = secondSuit->GetMaterial();
    carMaterial.type = Material::PBR;
    carMaterial.metallic = 0.7f;
    carMaterial.roughness = 0.3f;
    carMaterial.ao = 1.0f;
    carMaterial.useMaterialMask = true;
    carMaterial.velvetRoughness = 0.85f;
    carMaterial.velvetMetallic = 0.05f;

    // 10.添加光源
    PointLight pointLights[2] = {
        {glm::vec3(2.0f, 1.5f, 1.0f), glm::vec3(0.1f), glm::vec3(0.8f, 0.8f, 0.6f), glm::vec3(1.0f), 1.0f, 0.09f, 0.032f},
        {glm::vec3(-3.0f, 1.5f, -2.0f), glm::vec3(0.1f), glm::vec3(0.6f, 0.8f, 0.8f), glm::vec3(1.0f), 1.0f, 0.09f, 0.032f}
     };

    SpotLight spotLight = {
        camera->Position, camera->Front,
        glm::vec3(0.1f), glm::vec3(0.8f), glm::vec3(1.0f),
        glm::cos(glm::radians(12.5f)),
        glm::cos(glm::radians(17.5f)),
        1.0f, 0.09f, 0.032f
    };
     
    bool enableSoftShadows = true;
    bool softKeyPressed = false;//按键状态标志防止重复触发

    // ========== FIXED: 定义平行光方向（全局使用） ==========
    glm::vec3 dirLightDirection = glm::normalize(glm::vec3(-0.5f, -1.0f, -0.5f));

    // End. 渲染循环
    while (!glfwWindowShouldClose(window)) {
        // 计算帧时间
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;
        camera->ProcessKeyboard(deltaTime);
        //更新聚光灯的位置
        spotLight.position = camera->Position;
        spotLight.direction = camera->Front;

        // 软阴影切换快捷键（F1键）
        if (glfwGetKey(window, GLFW_KEY_F1) == GLFW_PRESS && !softKeyPressed) {
            enableSoftShadows = !enableSoftShadows;
            ourShader.setBool("enableSoftShadows", enableSoftShadows);
            softKeyPressed = true;  // 标记按键已按下
            std::cout << "Soft Shadows: " << (enableSoftShadows ? "ON" : "OFF") << std::endl;
        }
        if (glfwGetKey(window, GLFW_KEY_F1) == GLFW_RELEASE) {
            softKeyPressed = false;  // 按键释放后重置状态
        }

        // 亮度控制
        if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) {
            brightness += 0.1f;
            if (brightness > 3.0f) brightness = 3.0f;
        }
        if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) {
            brightness -= 0.1f;
            if (brightness < 0.5f) brightness = 0.5f;
        }



        // ================== 渲染深度贴图 ==================
        glViewport(0, 0, shadowMapper.SHADOW_WIDTH, shadowMapper.SHADOW_HEIGHT);
        glBindFramebuffer(GL_FRAMEBUFFER, shadowMapper.depthMapFBO);
        glClear(GL_DEPTH_BUFFER_BIT);

        // 使用平行光方向计算光源空间矩阵（关键修复）
        glm::vec3 lightPos = -dirLightDirection * 10.0f; // 从反方向10个单位处照射原点
        // 计算光源空间矩阵
        glm::mat4 lightProjection = glm::ortho(-15.0f, 15.0f, -15.0f, 15.0f, 0.1f, 30.0f);
        glm::mat4 lightView = glm::lookAt(lightPos,
            glm::vec3(0.0f),
            glm::vec3(0.0f, 1.0f, 0.0f));
        glm::mat4 lightSpaceMatrix = lightProjection * lightView;

        // 使用深度着色器渲染场景
        depthShader.use();
        depthShader.setMat4("lightSpaceMatrix", lightSpaceMatrix);
        //// 临时禁用材质纹理避免干扰
        //glDisable(GL_TEXTURE_2D);
        scene.RenderScene(depthShader);  // 注意：所有节点都会渲染深度

        // ================== 主场景渲染 ==================
        //glEnable(GL_TEXTURE_2D);

        // 绑定到MSAA帧缓冲
        glBindFramebuffer(GL_FRAMEBUFFER, msaaFBO);
        // 清除缓冲
        glClearColor(0.05f, 0.05f, 0.05f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // 设置视图/投影矩阵
        glm::mat4 view = camera->GetViewMatrix();
        glm::mat4 projection = glm::perspective(
            glm::radians(camera->Zoom),
            (float)SCR_WIDTH / (float)SCR_HEIGHT,
            0.1f, 100.0f
        );

        // 渲染循环中添加（放在相机处理之后）
        if (glfwGetKey(window, GLFW_KEY_F2) == GLFW_PRESS) {
            normalStrength = std::max(0.1f, normalStrength - 0.05f);
            std::cout << "Normal Strength: " << normalStrength << std::endl;
        }
        if (glfwGetKey(window, GLFW_KEY_F3) == GLFW_PRESS) {
            normalStrength = std::min(1.5f, normalStrength + 0.05f);
            std::cout << "Normal Strength: " << normalStrength << std::endl;
        }
        if (glfwGetKey(window, GLFW_KEY_F4) == GLFW_PRESS) {
            aoStrength = std::max(0.1f, aoStrength - 0.05f);
            std::cout << "AO Strength: " << aoStrength << std::endl;
        }
        if (glfwGetKey(window, GLFW_KEY_F5) == GLFW_PRESS) {
            aoStrength = std::min(1.2f, aoStrength + 0.05f);
            std::cout << "AO Strength: " << aoStrength << std::endl;
        }
        // 修改4：增加环境光强度，使阴影更明显
        float ambientIntensity = 0.3f * brightness; // 动态调整环境光

        // 激活着色器
        ourShader.use();


        ourShader.setMat4("projection", projection);
        ourShader.setMat4("view", view);
        ourShader.setMat4("lightSpaceMatrix", lightSpaceMatrix);  // 传递光源矩阵
        ourShader.setBool("enableSoftShadows", enableSoftShadows);
        ourShader.setFloat("brightness", brightness);

        // 绑定阴影贴图到纹理单元3
        glActiveTexture(GL_TEXTURE3);
        glBindTexture(GL_TEXTURE_2D, shadowMapper.depthMap);
        ourShader.setInt("shadowMap", 3);

        // 光源设置
        // 1. 调整平行光参数使阴影更明显
        ourShader.setVec3("dirLight.direction", dirLightDirection);
        ourShader.setVec3("dirLight.ambient", glm::vec3(ambientIntensity));
        ourShader.setVec3("dirLight.diffuse", glm::vec3(0.7f * brightness)); // 降低漫反射强度
        ourShader.setVec3("dirLight.specular", glm::vec3(0.5f)); // 降低镜面反射强度
        // 2. 设置点光源
        for (int i = 0; i < 2; i++) {
            std::string prefix = "pointLights[" + std::to_string(i) + "].";
            ourShader.setVec3(prefix + "position", pointLights[i].position);
            ourShader.setVec3(prefix + "ambient", pointLights[i].ambient);
            ourShader.setVec3(prefix + "diffuse", pointLights[i].diffuse * 0.7f);
            ourShader.setVec3(prefix + "specular", pointLights[i].specular * 0.7f);
            ourShader.setFloat(prefix + "constant", pointLights[i].constant);
            ourShader.setFloat(prefix + "linear", pointLights[i].linear);
            ourShader.setFloat(prefix + "quadratic", pointLights[i].quadratic);
        }
        // 3. 设置聚光灯
        ourShader.setVec3("spotLight.position", spotLight.position);
        ourShader.setVec3("spotLight.direction", spotLight.direction);
        ourShader.setVec3("spotLight.ambient", spotLight.ambient);
        ourShader.setVec3("spotLight.diffuse", spotLight.diffuse);
        ourShader.setVec3("spotLight.specular", spotLight.specular);
        ourShader.setFloat("spotLight.cutOff", spotLight.cutOff);
        ourShader.setFloat("spotLight.outerCutOff", spotLight.outerCutOff);
        ourShader.setFloat("spotLight.constant", spotLight.constant);
        ourShader.setFloat("spotLight.linear", spotLight.linear);
        ourShader.setFloat("spotLight.quadratic", spotLight.quadratic);

        // 渲染非PBR模型
        scene.RenderScene(ourShader); 

        // 渲染PBR模型
        pbrShader.use();
        pbrShader.setBool("useVelvet", carMaterial.useVelvet);
        carMaterial.useVelvet = true;
        carMaterial.velvetColor = glm::vec3(0.9f, 0.1f, 0.1f); // 深红色绒毛
        pbrShader.setVec3("velvetColor", carMaterial.velvetColor);
        pbrShader.setFloat("velvetStrength", carMaterial.velvetStrength);

        pbrShader.setFloat("u_NormalStrength", normalStrength);
        pbrShader.setFloat("u_AOStrength", aoStrength);
        pbrShader.setMat4("projection", projection);
        pbrShader.setMat4("view", view);
        pbrShader.setVec3("viewPos", camera->Position);

        // 绑定IBL贴图（关键新增）
        iblSystem->BindIrradianceMap(GL_TEXTURE10);
        pbrShader.setInt("irradianceMap", 10);
        iblSystem->BindPrefilterMap(GL_TEXTURE11);
        pbrShader.setInt("prefilterMap", 11);
        iblSystem->BindBRDFLUT(GL_TEXTURE12);
        pbrShader.setInt("brdfLUT", 12);

        for (int i = 0; i < 2; i++) {
            pbrShader.setVec3("lightPositions[" + std::to_string(i) + "]", pointLights[i].position);
            pbrShader.setVec3("lightColors[" + std::to_string(i) + "]", pointLights[i].diffuse * 0.8f);
        }

        // 设置材质遮罩参数
        pbrShader.setBool("useMaterialMask", carMaterial.useMaterialMask);
        pbrShader.setFloat("velvetRoughness", carMaterial.velvetRoughness);
        pbrShader.setFloat("velvetMetallic", carMaterial.velvetMetallic);
        // 渲染pbr模型
        secondSuit->Draw(pbrShader);



        // 解析MSAA到默认帧缓冲
        glBindFramebuffer(GL_READ_FRAMEBUFFER, msaaFBO);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
        glBlitFramebuffer(0, 0, SCR_WIDTH, SCR_HEIGHT, 0, 0, SCR_WIDTH, SCR_HEIGHT,
        GL_COLOR_BUFFER_BIT, GL_LINEAR); // 使用线性过滤

        // 交换缓冲并轮询事件
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // End+1. 清理资源
    deleteMSAAFramebuffer();
    glfwTerminate();
    shadowMapper.Cleanup();
    delete camera;
    delete iblSystem;  // 清理IBL系统
    return 0;
}

// 设置MSAA帧缓冲
void setupMSAAFramebuffer(unsigned int width, unsigned int height) {
    // 创建帧缓冲
    glGenFramebuffers(1, &msaaFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, msaaFBO);

    // 创建多重采样颜色纹理
    glGenTextures(1, &msaaColorBuffer);
    glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, msaaColorBuffer);
    glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, 4, GL_RGB, width, height, GL_TRUE);
    glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE, msaaColorBuffer, 0);

    // 创建渲染缓冲对象（深度和模板）
    glGenRenderbuffers(1, &msaaRBO);
    glBindRenderbuffer(GL_RENDERBUFFER, msaaRBO);
    glRenderbufferStorageMultisample(GL_RENDERBUFFER, 4, GL_DEPTH24_STENCIL8, width, height);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, msaaRBO);

    // 检查完整性
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        std::cout << "ERROR::MSAA Framebuffer not complete!" << std::endl;
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

// 删除MSAA帧缓冲资源
void deleteMSAAFramebuffer() {
    glDeleteFramebuffers(1, &msaaFBO);
    glDeleteTextures(1, &msaaColorBuffer);
    glDeleteRenderbuffers(1, &msaaRBO);
}

// 窗口大小调整回调
void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}

// 鼠标移动回调
void mouse_callback(GLFWwindow* window, double xposIn, double yposIn) {

    static float lastX = SCR_WIDTH / 2.0f;
    static float lastY = SCR_HEIGHT / 2.0f;
    float xpos = static_cast<float>(xposIn);
    float ypos = static_cast<float>(yposIn);
    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; // 注意y轴反转
    lastX = xpos;
    lastY = ypos;
    // 使用相机类处理鼠标移动
    camera->ProcessMouseMovement(xoffset, yoffset);
}

// 鼠标滚轮回调
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
    camera->ProcessMouseScroll(static_cast<float>(yoffset));
}
