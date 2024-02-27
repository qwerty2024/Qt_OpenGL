#ifndef WIDGET_H
#define WIDGET_H

#include <QOpenGLWidget>
#include <QMatrix4x4>
#include <QOpenGLShaderProgram>
#include <QOpenGLTexture>
#include <QOpenGLBuffer>
#include <QMouseEvent>
#include <QBasicTimer>

class QOpenGLFramebufferObject;
class Transformational;
class Group3D;
class Camera3D;
class SkyBox;
class ObjectEngine3D;
class Light;

class Widget : public QOpenGLWidget
{
    Q_OBJECT

public:
    Widget(QWidget *parent = nullptr);
    ~Widget();

protected:
    void initializeGL() override;
    void resizeGL(int w, int h) override;
    void paintGL() override;
    void initShaders();
    void initCube(float width, float height, float depth, QImage *diffuseMap = nullptr, QImage *normalMap = nullptr);

    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;
    void timerEvent(QTimerEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;

private:
    QMatrix4x4 m_projectionMatrix;
    QMatrix4x4 m_projectionLightMatrix;
    //QMatrix4x4 m_lightMatrix;
    //QMatrix4x4 m_shadowLightMatrix;

    //float m_lightRotateX;
    //float m_lightRotateY;

    QOpenGLShaderProgram m_program;
    QOpenGLShaderProgram m_programSkybox;
    QOpenGLShaderProgram m_programDepth;

    QVector2D m_mousePosition;

    QVector<ObjectEngine3D *> m_objects;
    QVector<Transformational *> m_TransformObject;
    QVector<Group3D *> m_groups;

    QBasicTimer m_timer;

    float angleObject;
    float angleGroup1;
    float angleGroup2;
    float angleMain;

    Camera3D *m_camera;
    SkyBox *m_skybox;

    QOpenGLFramebufferObject *m_depthBuffer;
    quint32 m_fbHeight;
    quint32 m_fbWidth;

    Light *m_light[3];
};
#endif // WIDGET_H





























