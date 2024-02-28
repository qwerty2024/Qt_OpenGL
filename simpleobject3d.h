#ifndef SIMPLEOBJECT3D_H
#define SIMPLEOBJECT3D_H

#include <QOpenGLBuffer>
#include <QMatrix4x4>
#include <QVector2D>
#include <transformational.h>

class QOpenGLTexture;
class QOpenGLFunctions;
class QOpenGLShaderProgram;
class Material;

struct VertexData
{
    VertexData() {}
    VertexData(QVector3D p, QVector2D t, QVector3D n) :
        position(p), texCoord(t), normal(n) {}

    QVector3D position;
    QVector2D texCoord;
    QVector3D normal;
    QVector3D tangent;
    QVector3D bitangent;
};

class SimpleObject3D : public Transformational
{
public:
    SimpleObject3D();
    SimpleObject3D(const QVector<VertexData> &vertData, const QVector<GLuint> &indexes, Material *material);
    ~SimpleObject3D();

    void init(const QVector<VertexData> &vertData, const QVector<GLuint> &indexes, Material *material);
    void rotate(const QQuaternion &r);
    void translate(const QVector3D &t);
    void scale(const float &s);
    void scale(const float &sx, const float &sy, const float &sz);
    void scale(const QVector3D &s);
    void setGlobalTransform(const QMatrix4x4 &g);
    void draw(QOpenGLShaderProgram *program, QOpenGLFunctions *functions);


    QOpenGLTexture *normaleMap() const;
    void setNormaleMap(QOpenGLTexture *normaleMap);

private:
    QOpenGLBuffer m_vertexBuffer;
    QOpenGLBuffer m_indexBuffer;
    QOpenGLTexture *m_diffuseMap;
    QOpenGLTexture *m_normalMap;

    QQuaternion m_rotate;
    QVector3D m_translate;
    QVector3D m_scale;
    QMatrix4x4 m_globalTransform;

    Material *m_material;
};

#endif // SIMPLEOBJECT3D_H
