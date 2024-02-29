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

    enum CreateState
    {
        Idle,
        CreatePlane,
        CreateHeight
    };

protected:
    void initializeGL() override;
    void resizeGL(int w, int h) override;
    void paintGL() override;
    void initShaders();
    void initCube(float width, float height, float depth, QImage *diffuseMap = nullptr, QImage *normalMap = nullptr);

    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;
    void timerEvent(QTimerEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;

    int selectObject(int x, int y, QVector<Transformational *> &objs);

    QVector3D screenCoordsToWorldCoords(const QVector2D &mousePos);

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
    QOpenGLShaderProgram m_programSelect;

    QVector2D m_mousePosition;

    QVector<ObjectEngine3D *> m_objects;
    QVector<Transformational *> m_TransformObject;
    QVector<Transformational *> m_selectObjects;
    QVector<Group3D *> m_groups;

    QBasicTimer m_timer;

    float angleObject;
    float angleGroup1;
    float angleGroup2;
    float angleMain;

    Camera3D *m_camera;
    SkyBox *m_skybox;

    QOpenGLFramebufferObject *m_depthBuffer;
    QOpenGLFramebufferObject *m_selectBuffer;
    quint32 m_fbHeight;
    quint32 m_fbWidth;

    Light *m_light[3];

    CreateState m_state;
    QVector3D m_firstPoint;
    QVector3D m_secondPoint;
    QVector3D m_movePoint;
};
#endif // WIDGET_H





























