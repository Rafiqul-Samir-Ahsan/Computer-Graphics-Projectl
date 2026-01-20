#include <windows.h>
#include <GL/glut.h>
#include <GL/glu.h>
#include <cmath>

const int W = 800;
const int H = 600;
const float PI = 3.1415926f;

int currentScene = 1;

// Scene 1
const float SCENE1_XR1 = 728.0f, SCENE1_YR1 = 0.0f;
const float SCENE1_XR2 = 454.6f, SCENE1_YR2 = 313.7f;

const float SCENE1_XL1_INIT = 720.0f, SCENE1_YL1 = 0.0f;
const float SCENE1_XL2_INIT = 440.0f, SCENE1_YL2 = 300.0f;

float scene1_shift = 0.0f;
float scene1_maxShift = 60.0f;
float scene1_speed = 140.0f;
bool scene1_movingLeft = true;

int scene1_lastMs = 0;

float scene1_roadLaneOffset = 0.0f;
float scene1_moveProgress = 0.0f;
bool scene1_headlightOn = false;

void initGL() {
    glClearColor(0.5f, 0.8f, 1.0f, 1.0f);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(0.0, W, 0.0, H);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

void scene1_drawCircle(float cx, float cy, float radius, float r, float g, float b) {
    glColor3f(r, g, b);
    glBegin(GL_TRIANGLE_FAN);
    glVertex2f(cx, cy);
    for (int i = 0; i <= 360; i += 10) {
        float angle = i * 3.14159f / 180.0f;
        glVertex2f(cx + cos(angle) * radius, cy + sin(angle) * radius);
    }
    glEnd();
}

void scene1_drawEnvironment() {
    scene1_drawCircle(120, 510, 36, 1.0f, 1.0f, 0.0f);

    glBegin(GL_TRIANGLES);
    glColor3f(0.4f, 0.3f, 0.2f);
    glVertex2i(0, 300);
    glVertex2i(240, 480);
    glVertex2i(480, 300);

    glColor3f(0.3f, 0.25f, 0.15f);
    glVertex2i(320, 300);
    glVertex2i(640, 540);
    glVertex2i(800, 300);
    glEnd();

    glBegin(GL_QUADS);
    glColor3f(0.1f, 0.6f, 0.1f);
    glVertex2i(0, 0);
    glVertex2i(400, 0);
    glVertex2i(400, 300);
    glVertex2i(0, 300);

    glColor3f(0.8f, 0.7f, 0.1f);
    glVertex2i(400, 0);
    glVertex2i(800, 0);
    glVertex2i(800, 300);
    glVertex2i(400, 300);
    glEnd();
}

void scene1_drawRoad() {
    glColor3f(0.2f, 0.2f, 0.2f);
    glBegin(GL_QUADS);
    glVertex2i(80, 0);
    glVertex2i(720, 0);
    glVertex2i(440, 300);
    glVertex2i(360, 300);
    glEnd();

    if (scene1_headlightOn) {
        glColor3f(0.5f, 0.5f, 0.5f);
        glBegin(GL_QUADS);
        glVertex2i(384, 300);
        glVertex2i(416, 300);
        glVertex2i(640, 0);
        glVertex2i(160, 0);
        glEnd();

        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glColor4f(1.0f, 1.0f, 0.8f, 0.2f);
        glBegin(GL_TRIANGLES);
        glVertex2i(400, 0);
        glVertex2i(240, 300);
        glVertex2i(560, 300);
        glEnd();
        glDisable(GL_BLEND);
    }

    glColor3f(1.0f, 1.0f, 1.0f);
    for (int i = 0; i < 6; i++) {
        float t = static_cast<float>(i) / 6.0f + scene1_roadLaneOffset;
        if (t > 1.0f) t -= 1.0f;

        float yPos = 300.0f - (t * 300.0f);
        float scale = (300.0f - yPos) / 300.0f;
        float w = 4.0f + (scale * 16.0f);
        float h = 6.0f + (scale * 36.0f);

        if (yPos < 300 && yPos > 0) {
            glBegin(GL_QUADS);
            glVertex2f(400 - w, yPos);
            glVertex2f(400 + w, yPos);
            glVertex2f(400 + w, yPos - h);
            glVertex2f(400 - w, yPos - h);
            glEnd();
        }
    }
}

void scene1_drawMovingQuad() {
    float XL1 = SCENE1_XL1_INIT - scene1_shift;
    float XL2 = SCENE1_XL2_INIT - scene1_shift;

    glColor3f(0.15f, 0.60f, 0.85f);
    glBegin(GL_QUADS);
        glVertex2f(XL1, SCENE1_YL1);
        glVertex2f(XL2, SCENE1_YL2);
        glVertex2f(SCENE1_XR2, SCENE1_YR2);
        glVertex2f(SCENE1_XR1, SCENE1_YR1);
    glEnd();
}

void scene1_drawTree(float cx, float cy, float s) {
    float w = 15.0f * s;
    float h = 60.0f * s;
    glColor3f(0.4f, 0.2f, 0.1f);
    glBegin(GL_QUADS);
    glVertex2f(cx - w, cy);       glVertex2f(cx + w, cy);
    glVertex2f(cx + w, cy + h);   glVertex2f(cx - w, cy + h);
    glEnd();
    glColor3f(0.0f, 0.5f, 0.0f);
    glBegin(GL_TRIANGLES);
    glVertex2f(cx - (w * 4), cy + h);
    glVertex2f(cx + (w * 4), cy + h);
    glVertex2f(cx, cy + (h * 3));
    glEnd();
}

void scene1_drawRock(float cx, float cy, float s) {
    float w = 20.0f * s;
    float h = 15.0f * s;
    glColor3f(0.5f, 0.5f, 0.5f);
    glBegin(GL_POLYGON);
    glVertex2f(cx - w, cy);
    glVertex2f(cx - (w / 2), cy + h);
    glVertex2f(cx + (w / 2), cy + (h * 0.8f));
    glVertex2f(cx + w, cy);
    glEnd();
}

void scene1_drawScarecrow(float cx, float cy, float s) {
    float h = 80.0f * s;
    float armW = 30.0f * s;
    glColor3f(0.6f, 0.4f, 0.2f);
    glLineWidth(3.0f);
    glBegin(GL_LINES);
    glVertex2f(cx, cy); glVertex2f(cx, cy + h);
    glVertex2f(cx - armW, cy + (h * 0.7f)); glVertex2f(cx + armW, cy + (h * 0.7f));
    glEnd();
    scene1_drawCircle(cx, cy + h, 15.0f * s, 1.0f, 0.6f, 0.0f);
    glColor3f(0.0f, 0.0f, 0.8f);
    glBegin(GL_TRIANGLES);
    glVertex2f(cx - (armW / 2), cy + (h * 0.4f));
    glVertex2f(cx + (armW / 2), cy + (h * 0.4f));
    glVertex2f(cx, cy + (h * 0.7f));
    glEnd();
}

void scene1_drawAnimatedSprites() {
    float t = scene1_moveProgress;
    float scale = 0.2f + (t * 1.5f);
    float yPos = 300.0f - (t * 300.0f);

    float leftX = 336.0f - (t * 336.0f);
    if (yPos > 0) {
        scene1_drawTree(leftX, yPos, scale);
        scene1_drawRock(leftX - (30 * scale), yPos, scale);
    }

    float rightX = 464.0f + (t * 336.0f);
    if (yPos > 0) {
        scene1_drawScarecrow(rightX, yPos, scale);
    }
}

void scene1_drawBikePOV() {
    glColor3f(0.1f, 0.1f, 0.1f);
    glBegin(GL_POLYGON);
    glVertex2i(160, 0);
    glVertex2i(160, 90);
    glVertex2i(280, 150);
    glVertex2i(520, 150);
    glVertex2i(640, 90);
    glVertex2i(640, 0);
    glEnd();

    glColor3f(0.2f, 0.4f, 0.6f);
    glBegin(GL_POLYGON);
    glVertex2i(280, 150);
    glVertex2i(520, 150);
    glVertex2i(480, 210);
    glVertex2i(320, 210);
    glEnd();

    scene1_drawCircle(280, 90, 36, 0.2f, 0.2f, 0.2f);
    scene1_drawCircle(280, 90, 30, 0.9f, 0.9f, 0.9f);

    glColor3f(1.0f, 0.0f, 0.0f);
    glLineWidth(3.0f);
    glBegin(GL_LINES);
    glVertex2i(280, 90);
    glVertex2i(304, 108);
    glEnd();

    scene1_drawCircle(520, 90, 36, 0.2f, 0.2f, 0.2f);
    scene1_drawCircle(520, 90, 30, 0.9f, 0.9f, 0.9f);

    glBegin(GL_LINES);
    glVertex2i(520, 90);
    glVertex2i(544, 102);
    glEnd();

    glLineWidth(8.0f);
    glColor3f(0.4f, 0.4f, 0.4f);
    glBegin(GL_LINES);
    glVertex2i(200, 60); glVertex2i(40, 90);
    glVertex2i(600, 60); glVertex2i(760, 90);
    glEnd();

    glColor3f(0.8f, 0.8f, 0.9f);
    glBegin(GL_QUADS);
    glVertex2i(16, 108); glVertex2i(64, 108); glVertex2i(64, 138); glVertex2i(16, 138);
    glEnd();
    glBegin(GL_QUADS);
    glVertex2i(736, 108); glVertex2i(784, 108); glVertex2i(784, 138); glVertex2i(736, 138);
    glEnd();

    if (scene1_headlightOn) {
        scene1_drawCircle(400, 90, 5, 0.0f, 0.8f, 1.0f);
    }
}

void scene1_display() {
    scene1_drawEnvironment();
    scene1_drawRoad();
    scene1_drawAnimatedSprites();
    scene1_drawBikePOV();
}

void scene1_update() {
    scene1_roadLaneOffset += 0.015f;
    if (scene1_roadLaneOffset > 1.0f) scene1_roadLaneOffset = 0.0f;

    scene1_moveProgress += 0.005f;
    if (scene1_moveProgress > 1.0f) scene1_moveProgress = 0.0f;

    int now = glutGet(GLUT_ELAPSED_TIME);
    if (scene1_lastMs == 0) scene1_lastMs = now;
    float dt = (now - scene1_lastMs) / 1000.0f;
    scene1_lastMs = now;

    if (scene1_movingLeft) {
        scene1_shift += scene1_speed * dt;
        if (scene1_shift >= scene1_maxShift) {
            scene1_shift = 0.0f;
        }
    }
}

// Scene 2
const float SCENE2_XR1 = 728.0f, SCENE2_YR1 = 0.0f;
const float SCENE2_XR2 = 454.6f, SCENE2_YR2 = 313.7f;
const float SCENE2_XL1_INIT = 720.0f, SCENE2_YL1 = 0.0f;
const float SCENE2_XL2_INIT = 440.0f, SCENE2_YL2 = 300.0f;
float scene2_shift = 0.0f;
float scene2_maxShift = 25.0f;
const float scene2_shiftStep = scene2_maxShift / 120.0f;

float scene2_roadOffset = 0.0f;
const float SCENE2_SUN_RADIUS_MIN = 0.11f * (W * 0.5f);
const float SCENE2_SUN_RADIUS_MAX = 0.16f * (W * 0.5f);
const float SCENE2_SUN_RADIUS_STEP = 0.6f;
float scene2_sunRadius = SCENE2_SUN_RADIUS_MIN;
const float SCENE2_BALLOON_START_X = 660.0f;
float scene2_balloonX = SCENE2_BALLOON_START_X;
const float SCENE2_BALLOON_SPEED_DEFAULT = 0.6f;
float scene2_balloonSpeed = SCENE2_BALLOON_SPEED_DEFAULT;
const float SCENE2_BALLOON_SPEED_STEP = 0.1f;
const float SCENE2_BALLOON_SPEED_MAX = 10.0f;
float scene2_birdX = (-1.30f + 1.0f) * (W * 0.5f);
float scene2_birdSpeed = 0.0060f * (W * 0.5f);

float scene2_redBallX = 120.0f;
float scene2_redBallY = 300.0f;
float scene2_redBallSpeed = 2.0f;

void scene2_drawEllipsePx(float x, float y, float rx, float ry, float r, float g, float b)
{
    glColor3f(r, g, b);
    glBegin(GL_TRIANGLE_FAN);
        glVertex2f(x, y);
        for (int i = 0; i <= 60; i++) {
            float a = 2.0f * 3.14159f * i / 60.0f;
            glVertex2f(x + cos(a) * rx, y + sin(a) * ry);
        }
    glEnd();
}

void scene2_drawFilledCircle(float cx, float cy, float r)
{
    glBegin(GL_TRIANGLE_FAN);
        glVertex2f(cx, cy);
        for (int i = 0; i <= 360; i++)
        {
            float a = i * PI / 180.0f;
            glVertex2f(cx + cos(a) * r, cy + sin(a) * r);
        }
    glEnd();
}

void scene2_wave()
{
    float xL1 = SCENE2_XL1_INIT - scene2_shift;
    float xL2 = SCENE2_XL2_INIT - scene2_shift;

    glColor3f(0.15f, 0.60f, 0.85f);
    glBegin(GL_QUADS);
        glVertex2f(xL1, 0.0f);
        glVertex2f(xL2, 300.0f);
        glVertex2f(454.6f, 300.7f);
        glVertex2f(799.0f, 0.0f);
    glEnd();
}

void scene2_drawSunPulsing()
{
    float sunCX = 712.0f;
    float sunCY = 534.0f;
    scene2_drawEllipsePx(sunCX, sunCY, scene2_sunRadius, scene2_sunRadius, 0.95f, 0.78f, 0.10f);
}

void scene2_drawHandlebar()
{
    glColor3f(0.2f, 0.2f, 0.2f);
    glBegin(GL_QUADS);
        glVertex2i(185, 166);
        glVertex2i(615, 166);
        glVertex2i(615, 150);
        glVertex2i(185, 150);
    glEnd();

    glBegin(GL_QUADS);
        glVertex2i(185, 150);
        glVertex2i(201, 150);
        glVertex2i(211, 78);
        glVertex2i(195, 78);

        glVertex2i(599, 150);
        glVertex2i(615, 150);
        glVertex2i(605, 78);
        glVertex2i(589, 78);
    glEnd();

    glColor3f(0.55f, 0.35f, 0.15f);
    glBegin(GL_QUADS);
        glVertex2i(240, 172);
        glVertex2i(300, 172);
        glVertex2i(300, 144);
        glVertex2i(240, 144);

        glVertex2i(500, 172);
        glVertex2i(560, 172);
        glVertex2i(560, 144);
        glVertex2i(500, 144);
    glEnd();

    glColor3f(0.55f, 0.35f, 0.15f);

    glBegin(GL_QUADS);
        glVertex2i(230, 150);
        glVertex2i(260, 150);
        glVertex2i(260, -20);
        glVertex2i(230, -20);

        glVertex2i(540, 150);
        glVertex2i(570, 150);
        glVertex2i(570, -20);
        glVertex2i(540, -20);
    glEnd();

    glColor3f(0.12f, 0.12f, 0.12f);
    glBegin(GL_POLYGON);
        glVertex2i(365, 158);
        glVertex2i(435, 158);
        glVertex2i(415, -2);
        glVertex2i(385, -2);
    glEnd();

    glColor3f(0.75f, 0.75f, 0.75f);
    scene2_drawFilledCircle(203.0f, 78, 8);
    scene2_drawFilledCircle(597.0f, 78, 8);
}

void scene2_drawBalloon(float x, float y)
{
    scene2_drawEllipsePx(x, y, 34, 34, 0.95f, 0.15f, 0.15f);

    glColor3f(0.40f, 0.22f, 0.08f);
    glLineWidth(2);
    glBegin(GL_LINES);
        glVertex2f(x - 16, y - 16);
        glVertex2f(x - 7, y - 48);

        glVertex2f(x + 16, y - 16);
        glVertex2f(x + 7, y - 48);
    glEnd();
    glLineWidth(1);

    glColor3f(0.55f, 0.30f, 0.10f);
    glBegin(GL_QUADS);
        glVertex2f(x - 12, y - 48);
        glVertex2f(x + 12, y - 48);
        glVertex2f(x + 12, y - 70);
        glVertex2f(x - 12, y - 70);
    glEnd();
}

void scene2_drawBird(float x, float y)
{
    glColor3f(0, 0, 0);
    glLineWidth(2);
    glBegin(GL_LINES);
        glVertex2f(x, y);
        glVertex2f(x + 14, y + 5);

        glVertex2f(x + 14, y + 5);
        glVertex2f(x + 28, y);
    glEnd();
    glLineWidth(1);
}

void scene2_drawTree()
{
    glColor3f(0.35f, 0.20f, 0.08f);
    glBegin(GL_QUADS);
        glVertex2f(32, 282);
        glVertex2f(48, 282);
        glVertex2f(48, 362);
        glVertex2f(32, 362);
    glEnd();

    glColor3f(0.05f, 0.45f, 0.10f);
    glBegin(GL_TRIANGLE_FAN);
        glVertex2f(40, 392);
        for (int i = 0; i <= 60; i++) {
            float a = 2.0f * 3.14159f * i / 60.0f;
            glVertex2f(40 + cos(a) * 28, 392 + sin(a) * 28);
        }
    glEnd();

    glColor3f(0.35f, 0.20f, 0.08f);
    glBegin(GL_QUADS);
        glVertex2f(72, 282);
        glVertex2f(88, 282);
        glVertex2f(88, 362);
        glVertex2f(72, 362);
    glEnd();

    glColor3f(0.05f, 0.45f, 0.10f);
    glBegin(GL_TRIANGLE_FAN);
        glVertex2f(80, 392);
        for (int i = 0; i <= 60; i++) {
            float a = 2.0f * 3.14159f * i / 60.0f;
            glVertex2f(80 + cos(a) * 28, 392 + sin(a) * 28);
        }
    glEnd();

    glColor3f(0.05f, 0.45f, 0.10f);
    glBegin(GL_TRIANGLE_FAN);
        glVertex2f(60, 402);
        for (int i = 0; i <= 60; i++) {
            float a = 2.0f * 3.14159f * i / 60.0f;
            glVertex2f(60 + cos(a) * 40, 402 + sin(a) * 40);
        }
    glEnd();
}

void scene2_drawHorizonTrees()
{
    scene2_drawTree();
}

void scene2_drawBackground()
{
    float birdGap = 48.0f;
    float birdY  = 426.0f;
    float bird2Y = 414.0f;
    float balloonY = 510.0f;

    glColor3f(0.95f, 0.85f, 0.10f);
    glBegin(GL_QUADS);
        glVertex2f(0.0f, 0.0f);
        glVertex2f(400.0f, 0.0f);
        glVertex2f(400.0f, 300.0f);
        glVertex2f(0.0f, 300.0f);
    glEnd();

    glColor3f(0.15f, 0.60f, 0.85f);
    glBegin(GL_QUADS);
        glVertex2f(400.0f, 0.0f);
        glVertex2f(800.0f, 0.0f);
        glVertex2f(800.0f, 300.0f);
        glVertex2f(400.0f, 300.0f);
    glEnd();

    scene2_drawBalloon(scene2_balloonX, balloonY);

    scene2_drawBird(scene2_birdX,           birdY);
    scene2_drawBird(scene2_birdX + birdGap, bird2Y);
}

void scene2_drawRoad()
{
    glColor3f(0.90f, 0.80f, 0.15f);
    glBegin(GL_QUADS);
        glVertex2f(0.0f, 0.0f);
        glVertex2f(800.0f, 0.0f);
        glVertex2f(440.0f, 300.0f);
        glVertex2f(360.0f, 300.0f);
    glEnd();

    glColor3f(0.30f, 0.20f, 0.05f);
    for (float i = -1.0f; i <= -0.20f; i += 0.2f)
    {
        float pos = i + scene2_roadOffset;
        if (pos > -0.05f) continue;

        glLineWidth(2.0f);
        glBegin(GL_LINES);
            glVertex2f((pos * 0.1f + 1.0f) * 400.0f, (pos + 1.0f) * 300.0f);
            glVertex2f(((pos + 0.1f) * 0.1f + 1.0f) * 400.0f, ((pos + 0.1f) + 1.0f) * 300.0f);
        glEnd();
    }
    glLineWidth(1);
}

void scene2_drawRedBallLeft()
{
    float yNdc = (scene2_redBallY / (H * 0.5f)) - 1.0f;
    float xNdc = -1.0f + 0.9f * (yNdc + 1.0f);
    float xPix = (xNdc + 1.0f) * (W * 0.5f);

    glColor3f(1.0f, 0.0f, 0.0f);
    glBegin(GL_TRIANGLE_FAN);
        glVertex2f(xPix, scene2_redBallY);
        for (int i = 0; i <= 60; i++) {
            float a = 2.0f * 3.14159f * i / 60.0f;
            glVertex2f(xPix + cos(a) * 14, scene2_redBallY + sin(a) * 14);
        }
    glEnd();
}

void scene2_display()
{
    scene2_drawBackground();
    scene2_drawSunPulsing();
    scene2_drawHorizonTrees();
    scene2_drawRoad();
    scene2_wave();
    scene2_drawRedBallLeft();
    scene2_drawHandlebar();
}

void scene2_update()
{
    scene2_roadOffset += 0.015f;
    if (scene2_roadOffset > 0.2f) scene2_roadOffset = 0.0f;
    if (scene2_sunRadius >= SCENE2_SUN_RADIUS_MAX)
    {
        scene2_sunRadius = SCENE2_SUN_RADIUS_MIN;
    }
    else
    {
        scene2_sunRadius += SCENE2_SUN_RADIUS_STEP;
    }

    scene2_balloonX -= scene2_balloonSpeed;
    if (scene2_balloonX < -50.0f) scene2_balloonX = SCENE2_BALLOON_START_X;

    scene2_birdX += scene2_birdSpeed;
    if (scene2_birdX > W) scene2_birdX = 0.0f;

    scene2_redBallY -= scene2_redBallSpeed;
    if (scene2_redBallY < 0.0f) scene2_redBallY = 300.0f;

    scene2_shift += scene2_shiftStep;
    if (scene2_shift >= scene2_maxShift)
    {
        scene2_shift = 0.0f;
    }
}

void display()
{
    glClear(GL_COLOR_BUFFER_BIT);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    if (currentScene == 2) {
        scene2_display();
    } else {
        scene1_display();
    }

    glutSwapBuffers();
}

void update(int)
{
    if (currentScene == 2) {
        scene2_update();
    } else if (currentScene == 1) {
        scene1_update();
    }

    glutPostRedisplay();
    glutTimerFunc(16, update, 0);
}

void handleKeypress(unsigned char key, int, int)
{
    if (key == 13) {
        if (currentScene == 0) {
            currentScene = 1;
        } else if (currentScene == 1) {
            currentScene = 2;
        } else {
            currentScene = 1;
        }
    }

    if (currentScene == 1 && (key == 'h' || key == 'H')) {
        scene1_headlightOn = !scene1_headlightOn;
    }

    glutPostRedisplay();
}

void handleMouse(int button, int state, int, int)
{
    if (state != GLUT_DOWN) return;

    if (currentScene != 2) {
        return;
    }

    if (button == GLUT_RIGHT_BUTTON)
    {
        scene2_balloonSpeed += SCENE2_BALLOON_SPEED_STEP;
        if (scene2_balloonSpeed > SCENE2_BALLOON_SPEED_MAX) scene2_balloonSpeed = SCENE2_BALLOON_SPEED_MAX;
    }
    else if (button == GLUT_LEFT_BUTTON)
    {
        scene2_balloonSpeed = 0.0f;
    }

    glutPostRedisplay();
}

int main(int argc, char** argv)
{
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
    glutInitWindowSize(W, H);
    glutCreateWindow("Two Scenes (Enter to Switch)");

    initGL();
    glutDisplayFunc(display);
    glutTimerFunc(0, update, 0);
    glutKeyboardFunc(handleKeypress);
    glutMouseFunc(handleMouse);

    glutMainLoop();
    return 0;
}
