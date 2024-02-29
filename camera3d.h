#ifndef CAMERA3D_H
#define CAMERA3D_H

#include <transformational.h>
#include <QVector>
#include <QQuaternion>
#include <QMatrix4x4>

class Camera3D : public Transformational
{
public:
    Camera3D();

    void rotate(const QQuaternion &r);
    void rotateX(const QQuaternion &r);
    void rotateY(const QQuaternion &r);
    void translate(const QVector3D &t);
    void scale(const float &s);
    void setGlobalTransform(const QMatrix4x4 &g);
    void draw(QOpenGLShaderProgram *program, QOpenGLFunctions *functions = nullptr, bool usingTextures = true);

    const QMatrix4x4 &getViewMatrix() const;

    void updateViewMatrix();

private:
    QQuaternion m_rotate;
    QQuaternion m_rotateX;
    QQuaternion m_rotateY;
    QVector3D m_translate;
    float m_scale;
    QMatrix4x4 m_globalTransform;
    QMatrix4x4 m_viewMatrix;
};

#endif // CAMERA3D_H
