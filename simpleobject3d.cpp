#include "simpleobject3d.h"
#include <QOpenGLTexture>
#include <QOpenGLFunctions>
#include <QOpenGLShaderProgram>
#include <material.h>

SimpleObject3D::SimpleObject3D() :
    m_indexBuffer(QOpenGLBuffer::IndexBuffer), m_diffuseMap(nullptr)
{
    m_scale = 1.0f;
}

SimpleObject3D::SimpleObject3D(const QVector<VertexData> &vertData, const QVector<GLuint> &indexes, Material *material) :
    m_indexBuffer(QOpenGLBuffer::IndexBuffer), m_diffuseMap(nullptr)
{
    m_scale = 1.0f;
    init(vertData, indexes, material);
}

void SimpleObject3D::init(const QVector<VertexData> &vertData, const QVector<GLuint> &indexes, Material *material)
{
    if (m_vertexBuffer.isCreated())
        m_vertexBuffer.destroy();

    if (m_indexBuffer.isCreated())
        m_indexBuffer.destroy();

    if (m_diffuseMap != nullptr)
    {
        if (m_diffuseMap->isCreated())
        {
            delete m_diffuseMap;
            m_diffuseMap = nullptr;
        }
    }

    m_vertexBuffer.create();
    m_vertexBuffer.bind();
    m_vertexBuffer.allocate(vertData.constData(), vertData.size()*sizeof(VertexData));
    m_vertexBuffer.release();

    m_indexBuffer.create();
    m_indexBuffer.bind();
    m_indexBuffer.allocate(indexes.constData(), indexes.size()*sizeof(GLuint));
    m_indexBuffer.release();

    m_material = material;

    if (m_material->isUsingDiffuseMap())
    {
        m_diffuseMap = new QOpenGLTexture(m_material->diffuseMap().mirrored());
        m_diffuseMap->setMinificationFilter(QOpenGLTexture::Nearest);
        m_diffuseMap->setMagnificationFilter(QOpenGLTexture::Linear);
        m_diffuseMap->setWrapMode(QOpenGLTexture::Repeat);
    }

    if (m_material->isUsingNormalMap())
    {
        m_normalMap = new QOpenGLTexture(m_material->normalMap().mirrored());
        m_normalMap->setMinificationFilter(QOpenGLTexture::Nearest);
        m_normalMap->setMagnificationFilter(QOpenGLTexture::Linear);
        m_normalMap->setWrapMode(QOpenGLTexture::Repeat);
    }
}

void SimpleObject3D::rotate(const QQuaternion &r)
{
    m_rotate = r * m_rotate;
}

void SimpleObject3D::draw(QOpenGLShaderProgram *program, QOpenGLFunctions *functions)
{
    if (!m_vertexBuffer.isCreated() || !m_indexBuffer.isCreated()) return;

    if (m_material->isUsingDiffuseMap())
    {
        m_diffuseMap->bind(0);
        program->setUniformValue("u_diffuseMap", 0);
    }

    if (m_material->isUsingNormalMap())
    {
        m_normalMap->bind(1);
        program->setUniformValue("u_normalMap", 1);
    }

    QMatrix4x4 modelMatrix;
    modelMatrix.setToIdentity();
    modelMatrix.translate(m_translate);
    modelMatrix.rotate(m_rotate);
    modelMatrix.scale(m_scale);
    modelMatrix = m_globalTransform * modelMatrix;

    program->setUniformValue("u_modelMatrix", modelMatrix);
    program->setUniformValue("u_materialProperty.diffuseColor", m_material->diffuseColor());
    program->setUniformValue("u_materialProperty.ambienceColor", m_material->ambienceColor());
    program->setUniformValue("u_materialProperty.specularColor", m_material->specularColor());
    program->setUniformValue("u_materialProperty.shinnes", m_material->shinnes());
    program->setUniformValue("u_isUsingDiffuseMap", m_material->isUsingDiffuseMap());
    program->setUniformValue("u_isUsingNormalMap", m_material->isUsingNormalMap());

    m_vertexBuffer.bind();

    int offset = 0;

    int vertLoc = program->attributeLocation("a_position");
    program->enableAttributeArray(vertLoc);
    program->setAttributeBuffer(vertLoc, GL_FLOAT, offset, 3, sizeof(VertexData));

    offset += sizeof(QVector3D);

    int texLoc = program->attributeLocation("a_texcoord");
    program->enableAttributeArray(texLoc);
    program->setAttributeBuffer(texLoc, GL_FLOAT, offset, 2, sizeof(VertexData));

    offset += sizeof(QVector2D);

    int normLoc = program->attributeLocation("a_normal");
    program->enableAttributeArray(normLoc);
    program->setAttributeBuffer(normLoc, GL_FLOAT, offset, 3, sizeof(VertexData));

    offset += sizeof(QVector3D);

    int tangentLoc = program->attributeLocation("a_tangent");
    program->enableAttributeArray(tangentLoc);
    program->setAttributeBuffer(tangentLoc, GL_FLOAT, offset, 3, sizeof(VertexData));

    offset += sizeof(QVector3D);

    int bitangentLoc = program->attributeLocation("a_bitangent");
    program->enableAttributeArray(bitangentLoc);
    program->setAttributeBuffer(bitangentLoc, GL_FLOAT, offset, 3, sizeof(VertexData));

    m_indexBuffer.bind();

    functions->glDrawElements(GL_TRIANGLES, m_indexBuffer.size(), GL_UNSIGNED_INT, 0);

    m_vertexBuffer.release();
    m_indexBuffer.release();
    if (m_material->isUsingDiffuseMap()) m_diffuseMap->release();
    if (m_material->isUsingNormalMap()) m_normalMap->release();
}

QOpenGLTexture *SimpleObject3D::normaleMap() const
{
    return m_normalMap;
}

void SimpleObject3D::setNormaleMap(QOpenGLTexture *normaleMap)
{
    m_normalMap = normaleMap;
}

void SimpleObject3D::translate(const QVector3D &t)
{
    m_translate += t;
}

void SimpleObject3D::scale(const float &s)
{
    m_scale *= s;
}

void SimpleObject3D::setGlobalTransform(const QMatrix4x4 &g)
{
    m_globalTransform = g;
}

SimpleObject3D::~SimpleObject3D()
{
    if (m_vertexBuffer.isCreated())
        m_vertexBuffer.destroy();

    if (m_indexBuffer.isCreated())
        m_indexBuffer.destroy();

    if (m_diffuseMap != nullptr)
    {
        if (m_diffuseMap->isCreated())
            m_diffuseMap->destroy();
    }
}
