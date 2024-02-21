#include "objectengine3d.h"
#include <QFile>
#include <QFileInfo>

ObjectEngine3D::ObjectEngine3D()
{

}

void ObjectEngine3D::loadObjectFromFile(const QString &filename)
{
    QFile objFile(filename);
    if (!objFile.exists())
    {
        qDebug() << "File not found!";
        return;
    }

    objFile.open(QIODevice::ReadOnly);
    QTextStream input(&objFile);

    QVector<QVector3D> coords;
    QVector<QVector2D> texCoord;
    QVector<QVector3D> normals;

    QVector<VertexData> vertexes;
    QVector<GLuint> indexes;
    SimpleObject3D *object = nullptr;
    QString mtlName;

    while (!input.atEnd())
    {
        QString str = input.readLine();
        QStringList list = str.split(" ");

        if (list[0] == "#")
        {
            qDebug() << "This is comment: " << str;
            continue;
        } else if (list[0] == "mtllib")
        {
            QFileInfo info(filename);
            m_materialLibrary.loadMaterialsFromFile(QString("%1/%2").arg(info.absolutePath()).arg(list[1]));
            qDebug() << "This is material: " << str;
            continue;

        } else if (list[0] == "v")
        {
            coords.append(QVector3D(list[1].toFloat(), list[2].toFloat(), list[3].toFloat()));
            continue;

        } else if (list[0] == "vt")
        {
            texCoord.append(QVector2D(list[1].toFloat(), list[2].toFloat()));
            continue;

        } else if (list[0] == "vn")
        {
            normals.append(QVector3D(list[1].toFloat(), list[2].toFloat(), list[3].toFloat()));
            continue;

        } else if (list[0] == "f")
        {
            for (int i = 1; i <= 3; i++)
            {
                QStringList vert = list[i].split("/");
                vertexes.append(VertexData(coords[vert[0].toLong() - 1], texCoord[vert[1].toLong() - 1], normals[vert[2].toLong() - 1]));
                //vertexes.append(VertexData(coords[vert[0].toLong() - 1], QVector2D(1.0f, 1.0f), normals[vert[2].toLong() - 1])); // пока без текстурных координат
                indexes.append(indexes.size());
            }
            continue;
        } else if (list[0] == "usemtl")
        {
            if (object)
            {
                object->init(vertexes, indexes, m_materialLibrary.getMaterial(mtlName));
            }
            mtlName = list[1];
            addObject(object);
            object = new SimpleObject3D;
            vertexes.clear();
            indexes.clear();
        }
    }

    object->init(vertexes, indexes, m_materialLibrary.getMaterial(mtlName));
    addObject(object);
    objFile.close();
}

void ObjectEngine3D::addObject(SimpleObject3D *object)
{
    if (!object)
        return;

    for (int i = 0; i < m_objects.size(); i++)
        if (m_objects[i] == object)
            return;

    m_objects.append(object);
}

SimpleObject3D *ObjectEngine3D::getObject(quint32 index)
{
    if (index < (quint32)m_objects.size())
        return m_objects[index];
    else
        return nullptr;
}

void ObjectEngine3D::rotate(const QQuaternion &r)
{
    for (int i = 0; i < m_objects.size(); i++)
        m_objects[i]->rotate(r);
}

void ObjectEngine3D::translate(const QVector3D &t)
{
    for (int i = 0; i < m_objects.size(); i++)
        m_objects[i]->translate(t);
}

void ObjectEngine3D::scale(const float &s)
{
    for (int i = 0; i < m_objects.size(); i++)
        m_objects[i]->scale(s);
}

void ObjectEngine3D::setGlobalTransform(const QMatrix4x4 &g)
{
    for (int i = 0; i < m_objects.size(); i++)
        m_objects[i]->setGlobalTransform(g);
}

void ObjectEngine3D::draw(QOpenGLShaderProgram *program, QOpenGLFunctions *functions)
{
    for (int i = 0; i < m_objects.size(); i++)
        m_objects[i]->draw(program, functions);
}
