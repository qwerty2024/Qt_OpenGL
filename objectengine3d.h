#ifndef OBJECTENGINE3D_H
#define OBJECTENGINE3D_H

#include <QVector>
#include <simpleobject3d.h>
#include <materiallibrary.h>

class ObjectEngine3D : public Transformational
{
public:
    ObjectEngine3D();
    void loadObjectFromFile(const QString &filename);
    void addObject(SimpleObject3D *object);
    SimpleObject3D *getObject(quint32 index);
    void calculateTBN(QVector<VertexData> &vertData);
    void rotate(const QQuaternion &r);
    void translate(const QVector3D &t);
    void scale(const float &s);
    void scale(const float &sx, const float &sy, const float &sz);
    void scale(const QVector3D &s);
    void setGlobalTransform(const QMatrix4x4 &g);
    void draw(QOpenGLShaderProgram *program, QOpenGLFunctions *functions, bool usingTextures = true);

private:
    QVector<SimpleObject3D *> m_objects;
    MaterialLibrary m_materialLibrary;

};

#endif // OBJECTENGINE3D_H
