#pragma once
class Player {
private:
    enum cameraModes { FPP, TPP };
    float lightLevels[3]{ .5, 1.25, 2 };

    Model obj;
    PerspectiveCamera tpp = PerspectiveCamera(
        glm::vec3(0, 0, 1.5),
        glm::vec3(0),
        glm::vec3(0, 1, 0),
        true);
    PerspectiveCamera fpp = PerspectiveCamera(
        glm::vec3(0, 0, 1),
        glm::vec3(0),
        glm::vec3(0, 1, 0),
        false);
    PointLight flashlight;

    // User control
    cameraModes activeCamera = TPP;
    int lightLevel = 0;
    bool lookMode = false;
    double cursorX, cursorY;

    void repositionLight() {
        glm::vec3 newPos = obj.getPos();
        float facing = obj.getRotation().x;
        newPos += glm::vec3(
            0.5 * sin(glm::radians(facing)),
            0,
            0.5 * cos(glm::radians(facing)));
        flashlight.setPos(newPos);
    }

public:
    Player() {}

    Player(std::string objPath, std::string texPath,
        glm::vec3 pos, float size, glm::vec3 rot,
        int colorMode = GL_RGBA)
    {
        obj = Model(objPath.c_str(), texPath.c_str(),
            pos, size, rot,
            colorMode);
        obj.loadNorm("3D/brickwall_normal.jpg", GL_RGB);
        obj.setPivotObject();

        flashlight = PointLight(
            glm::vec3(0), glm::vec3(1), // Pos to be overriden
            .1f, glm::vec3(1),
            1.f, 32.f
        );
        flashlight.setIntensity(lightLevels[lightLevel]);
        repositionLight();
    }

    Model getPlayer() {
        return obj;
    }

    PerspectiveCamera getActiveCamera() {
        if (activeCamera == FPP)
            return fpp;
        else
            return tpp;
    }

    PointLight getFlashlight() {
        return flashlight;
    }

    // TODO: Move light along with model
    void parseKey(int key, int action) {
        // Register only key press and hold
        if (action == GLFW_RELEASE)
            return;

        static float speed = 0.05;
        switch (key) {
            // Change active camera
            case GLFW_KEY_1:
                if (activeCamera == FPP)
                    activeCamera = TPP;
                else
                    activeCamera = FPP;
                break;

            // Movement
            // WS - main object z position
            // QE - main object y position
            // AD - main object x rotation
            case GLFW_KEY_Q:
                if (!(obj.getPos().y > 0)) {
                    obj.modPos(glm::vec3(0, 0.1f, 0));
                    cout << "Current Depth: " << obj.getPos().y << endl;
                }
                break;
            case GLFW_KEY_W:
                glm::vec3 rotation_w = obj.getRotation();
                obj.modPos(glm::vec3(
                    speed * sin(glm::radians(rotation_w.x)),
                    0,
                    speed * cos(glm::radians(rotation_w.x))));
                break;
            case GLFW_KEY_E:
                obj.modPos(glm::vec3(0, -0.1f, 0));
                cout << "Current Depth: " << obj.getPos().y << endl;
                break;
            case GLFW_KEY_A:
                obj.adjustRotate(glm::vec3(1.f, 0, 0));
                break;
            case GLFW_KEY_S:
                glm::vec3 rotation_s = obj.getRotation();
                obj.modPos(glm::vec3(
                    -speed * sin(glm::radians(rotation_s.x)),
                    0,
                    -speed * cos(glm::radians(rotation_s.x))));
                break;
            case GLFW_KEY_D:
                obj.adjustRotate(glm::vec3(-1.f, 0, 0));
                break;

            // Change flashlight brightness
            case GLFW_KEY_F:
                lightLevel++;
                flashlight.setIntensity(lightLevels[lightLevel]);
                if (lightLevel > 2)
                    lightLevel = 0;
                break;
        }

        // Update camera positions
        tpp.adjustCameraTpp(obj.getPos(), obj.getRotation());
        fpp.adjustCameraFpp(obj.getPos(), obj.getRotation());

        // Move player light to front of player
        repositionLight();
    }

    void parseCursor(GLFWwindow* window, double xpos, double ypos) {
        // Exit if camera is in orthographic view
        if (activeCamera == FPP)
            return;

        // X degrees per pixel
        static float sensitivity = 0.25;

        // Unlock freelook on mouse press, lock on release
        int mouseButton = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT);
        if (mouseButton == GLFW_PRESS && !lookMode) {
            lookMode = true;

            // Set starting position for cursor
            glfwGetCursorPos(window, &cursorX, &cursorY);
        }
        else if (mouseButton == GLFW_RELEASE) {
            tpp.adjustCameraTpp(obj.getPos(), obj.getRotation());
            tpp.setYawPitch();
            lookMode = false;
        }

        // Proceed if freelook is unlocked
        if (!lookMode) 
            return;

        // Move pitch and yaw depending on how far cursor moves
        double oldX = cursorX;
        double oldY = cursorY;
        glfwGetCursorPos(window, &cursorX, &cursorY);

        

        tpp.revolve(sensitivity * (cursorX - oldX), sensitivity * (oldY - cursorY), obj.getPos());
    }

    void cleanup() {
        obj.cleanup();
    }
};