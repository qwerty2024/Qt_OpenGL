#include "widget.h"
#include <simpleobject3d.h>
#include <QOpenGLContext>
#include <QKeyEvent>
#include <group3d.h>
#include <QtMath>
#include <camera3d.h>
#include <skybox.h>

Widget::Widget(QWidget *parent)
    : QOpenGLWidget(parent)
{
    m_camera = new Camera3D;
    m_camera->translate(QVector3D(0.0f, 0.0f, -15.0f));
}

Widget::~Widget()
{
    delete m_camera;
    for (int i = 0; i < m_objects.size(); i++)
        delete m_objects[i];

    for (int i = 0; i < m_groups.size(); i++)
        delete m_groups[i];
}

void Widget::mousePressEvent(QMouseEvent *event)
{
    if (event->buttons() == Qt::LeftButton)
    {
        m_mousePosition = QVector2D(event->localPos());
    }
    event->accept();
}

void Widget::mouseMoveEvent(QMouseEvent *event)
{
    if (event->buttons() != Qt::LeftButton) return;

    QVector2D diff = QVector2D(event->localPos()) - m_mousePosition;
    m_mousePosition = QVector2D(event->localPos());

    float angle = diff.length() / 2.0;

    QVector3D axis = QVector3D(diff.y(), diff.x(), 0.0);

    m_camera->rotate(QQuaternion::fromAxisAndAngle(axis, angle));

    update();
}

void Widget::wheelEvent(QWheelEvent *event)
{
    if (event->delta() > 0)
    {
        m_camera->translate(QVector3D(0.0f, 0.0f, 0.25f));
    } else if (event->delta() < 0)
    {
        m_camera->translate(QVector3D(0.0f, 0.0f, -0.25f));
    }

    update();
}

void Widget::timerEvent(QTimerEvent *event)
{
    Q_UNUSED(event);

    for(int i = 0; i < m_objects.size() - 1; i++)
    {
        if (i % 2 == 0)
        {
            m_objects[i]->rotate(QQuaternion::fromAxisAndAngle(1.0f, 0.0f, 0.0f, qSin(angleObject)));
            m_objects[i]->rotate(QQuaternion::fromAxisAndAngle(0.0f, 1.0f, 0.0f, qCos(angleObject)));
        } else
        {
            m_objects[i]->rotate(QQuaternion::fromAxisAndAngle(0.0f, 1.0f, 0.0f, qSin(angleObject)));
            m_objects[i]->rotate(QQuaternion::fromAxisAndAngle(1.0f, 0.0f, 0.0f, qCos(angleObject)));
        }
    }

    m_groups[0]->rotate(QQuaternion::fromAxisAndAngle(0.0f, 0.0f, 1.0f, qSin(angleGroup1)));
    m_groups[0]->rotate(QQuaternion::fromAxisAndAngle(0.0f, 1.0f, 0.0f, -qSin(angleGroup1)));

    m_groups[1]->rotate(QQuaternion::fromAxisAndAngle(1.0f, 0.0f, 0.0f, qCos(angleGroup2)));
    m_groups[1]->rotate(QQuaternion::fromAxisAndAngle(0.0f, 1.0f, 0.0f, -qCos(angleGroup2)));

    m_groups[2]->rotate(QQuaternion::fromAxisAndAngle(1.0f, 0.0f, 0.0f, qSin(angleMain)));
    m_groups[2]->rotate(QQuaternion::fromAxisAndAngle(0.0f, 1.0f, 0.0f, qCos(angleMain)));

    angleObject += M_PI / 180.0f;
    angleGroup1 += M_PI / 360.0f;
    angleGroup2 += M_PI / 360.0f;
    angleMain += M_PI / 720.0f;

    update();
}

void Widget::keyPressEvent(QKeyEvent *event)
{
    switch (event->key())
    {
        case Qt::Key_Left:
            m_groups[0]->delObject(m_camera);
            m_groups[1]->addObject(m_camera);
            break;
        case Qt::Key_Right:
            m_groups[1]->delObject(m_camera);
            m_groups[0]->addObject(m_camera);
            break;
        case Qt::Key_Down:
            m_groups[0]->delObject(m_camera);
            m_groups[1]->delObject(m_camera);
            break;
        case Qt::Key_Up:
            m_groups[0]->delObject(m_camera);
            m_groups[1]->delObject(m_camera);
            QMatrix4x4 tmp;
            tmp.setToIdentity();
            m_camera->setGlobalTransform(tmp);
            break;
    }
    update();
}

void Widget::initializeGL()
{
    glClearColor(0.3f, 0.3f, 0.3f, 1.0f);

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);

    initShaders();

    float step = 2.0f;

    m_groups.append(new Group3D);
    for (float x = -step; x <= step; x+= step)
    {
        for (float y = -step; y <= step; y+= step)
        {
            for (float z = -step; z <= step; z+= step)
            {
                initCube(1.0f);
                m_objects[m_objects.size() - 1]->translate(QVector3D(x, y, z));
                m_groups[m_groups.size() - 1]->addObject(m_objects[m_objects.size() - 1]);
            }
        }
    }

    m_groups[0]->translate(QVector3D(-8.0f, 0.0f, 0.0f));


    m_groups.append(new Group3D);
    for (float x = -step; x <= step; x+= step)
    {
        for (float y = -step; y <= step; y+= step)
        {
            for (float z = -step; z <= step; z+= step)
            {
                initCube(1.0f);
                m_objects[m_objects.size() - 1]->translate(QVector3D(x, y, z));
                m_groups[m_groups.size() - 1]->addObject(m_objects[m_objects.size() - 1]);
            }
        }
    }

    m_groups[1]->translate(QVector3D(8.0f, 0.0f, 0.0f));

    m_groups.append(new Group3D);
    m_groups[2]->addObject(m_groups[0]);
    m_groups[2]->addObject(m_groups[1]);

    m_TransformObject.append(m_groups[2]);

    loadObj(":/model/my_obj.obj");
    m_TransformObject.append(m_objects[m_objects.size() - 1]);

    m_groups[0]->addObject(m_camera);

    m_skybox = new SkyBox(100, QImage(":/sky.png"));

    m_timer.start(30, this);
}

void Widget::resizeGL(int w, int h)
{
    float aspect = w / (h ? (float)h : 1);
    m_projectionMatrix.setToIdentity();
    m_projectionMatrix.perspective(45, aspect, 0.01f, 1000.0f);
}

void Widget::paintGL()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    m_programSkybox.bind();
    m_programSkybox.setUniformValue("u_projectionMatrix", m_projectionMatrix);

    m_camera->draw(&m_programSkybox);
    m_skybox->draw(&m_programSkybox, context()->functions());
    m_programSkybox.release();

    m_program.bind();
    m_program.setUniformValue("u_projectionMatrix", m_projectionMatrix);
    m_program.setUniformValue("u_lightPosition", QVector4D(0.0, 0.0, 0.0, 1.0));
    m_program.setUniformValue("u_lightPower", 1.0f);

    m_camera->draw(&m_program);

    for(int i = 0; i < m_TransformObject.size(); i++)
    {
        m_TransformObject[i]->draw(&m_program, context()->functions());
    }
    m_program.release();
}

void Widget::initShaders()
{
    if (!m_programSkybox.addShaderFromSourceFile(QOpenGLShader::Vertex, ":/skybox.vsh"))
        close();

    if (!m_programSkybox.addShaderFromSourceFile(QOpenGLShader::Fragment, ":/skybox.fsh"))
        close();

    if (!m_programSkybox.link())
        close();

    if (!m_program.addShaderFromSourceFile(QOpenGLShader::Vertex, ":/vshader.vsh"))
        close();

    if (!m_program.addShaderFromSourceFile(QOpenGLShader::Fragment, ":/fshader.fsh"))
        close();

    if (!m_program.link())
        close();
}

void Widget::initCube(float width)
{
    float width_div_2 = width / 2.0f;
    QVector<VertexData> vertexes;

    vertexes.append(VertexData(QVector3D(-width_div_2, width_div_2, width_div_2), QVector2D(0.0, 1.0), QVector3D(0.0, 0.0, 1.0)));
    vertexes.append(VertexData(QVector3D(-width_div_2, -width_div_2, width_div_2), QVector2D(0.0, 0.0), QVector3D(0.0, 0.0, 1.0)));
    vertexes.append(VertexData(QVector3D(width_div_2, width_div_2, width_div_2), QVector2D(1.0, 1.0), QVector3D(0.0, 0.0, 1.0)));
    vertexes.append(VertexData(QVector3D(width_div_2, -width_div_2, width_div_2), QVector2D(1.0, 0.0), QVector3D(0.0, 0.0, 1.0)));

    vertexes.append(VertexData(QVector3D(width_div_2, width_div_2, width_div_2), QVector2D(0.0, 1.0), QVector3D(1.0, 0.0, 0.0)));
    vertexes.append(VertexData(QVector3D(width_div_2, -width_div_2, width_div_2), QVector2D(0.0, 0.0), QVector3D(1.0, 0.0, 0.0)));
    vertexes.append(VertexData(QVector3D(width_div_2, width_div_2, -width_div_2), QVector2D(1.0, 1.0), QVector3D(1.0, 0.0, 0.0)));
    vertexes.append(VertexData(QVector3D(width_div_2, -width_div_2, -width_div_2), QVector2D(1.0, 0.0), QVector3D(1.0, 0.0, 0.0)));

    vertexes.append(VertexData(QVector3D(width_div_2, width_div_2, width_div_2), QVector2D(0.0, 1.0), QVector3D(0.0, 1.0, 0.0)));
    vertexes.append(VertexData(QVector3D(width_div_2, width_div_2, -width_div_2), QVector2D(0.0, 0.0), QVector3D(0.0, 1.0, 0.0)));
    vertexes.append(VertexData(QVector3D(-width_div_2, width_div_2, width_div_2), QVector2D(1.0, 1.0), QVector3D(0.0, 1.0, 0.0)));
    vertexes.append(VertexData(QVector3D(-width_div_2, width_div_2, -width_div_2), QVector2D(1.0, 0.0), QVector3D(0.0, 1.0, 0.0)));


    vertexes.append(VertexData(QVector3D(width_div_2, width_div_2, -width_div_2), QVector2D(0.0, 1.0), QVector3D(0.0, 0.0, -1.0)));
    vertexes.append(VertexData(QVector3D(width_div_2, -width_div_2, -width_div_2), QVector2D(0.0, 0.0), QVector3D(0.0, 0.0, -1.0)));
    vertexes.append(VertexData(QVector3D(-width_div_2, width_div_2, -width_div_2), QVector2D(1.0, 1.0), QVector3D(0.0, 0.0, -1.0)));
    vertexes.append(VertexData(QVector3D(-width_div_2, -width_div_2, -width_div_2), QVector2D(1.0, 0.0), QVector3D(0.0, 0.0, -1.0)));

    vertexes.append(VertexData(QVector3D(-width_div_2, width_div_2, width_div_2), QVector2D(0.0, 1.0), QVector3D(-1.0, 0.0, 0.0)));
    vertexes.append(VertexData(QVector3D(-width_div_2, width_div_2, -width_div_2), QVector2D(0.0, 0.0), QVector3D(-1.0, 0.0, 0.0)));
    vertexes.append(VertexData(QVector3D(-width_div_2, -width_div_2, width_div_2), QVector2D(1.0, 1.0), QVector3D(-1.0, 0.0, 0.0)));
    vertexes.append(VertexData(QVector3D(-width_div_2, -width_div_2, -width_div_2), QVector2D(1.0, 0.0), QVector3D(-1.0, 0.0, 0.0)));

    vertexes.append(VertexData(QVector3D(-width_div_2, -width_div_2, width_div_2), QVector2D(0.0, 1.0), QVector3D(0.0, -1.0, 0.0)));
    vertexes.append(VertexData(QVector3D(-width_div_2, -width_div_2, -width_div_2), QVector2D(0.0, 0.0), QVector3D(0.0, -1.0, 0.0)));
    vertexes.append(VertexData(QVector3D(width_div_2, -width_div_2, width_div_2), QVector2D(1.0, 1.0), QVector3D(0.0, -1.0, 0.0)));
    vertexes.append(VertexData(QVector3D(width_div_2, -width_div_2, -width_div_2), QVector2D(1.0, 0.0), QVector3D(0.0, -1.0, 0.0)));

    QVector<GLuint> indexes;
    for (int i = 0; i < 24; i += 4)
    {
        indexes.append(i + 0);
        indexes.append(i + 1);
        indexes.append(i + 2);
        indexes.append(i + 2);
        indexes.append(i + 1);
        indexes.append(i + 3);
    }

    m_objects.append(new SimpleObject3D(vertexes, indexes, QImage(":/cube.png")));
}

void Widget::loadObj(const QString &path)
{
    QFile objFile(path);
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
            qDebug() << "This is material: " << str;
            continue;

        } else if (list[0] == "v")
        {
            coords.append(QVector3D(list[1].toFloat(), list[2].toFloat(), list[3].toFloat()) / 10.0f); // временно уменьшил размер в 10 раз
            continue;

        } else if (list[0] == "vt")
        {
            //texCoord.append(QVector2D(list[1].toFloat(), list[2].toFloat()));
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
                //vertexes.append(VertexData(coords[vert[0].toLong() - 1], texCoord[vert[0].toLong() - 1], normals[vert[2].toLong() - 1]));
                vertexes.append(VertexData(coords[vert[0].toLong() - 1], QVector2D(1.0f, 1.0f), normals[vert[2].toLong() - 1])); // пока без текстурных координат
                indexes.append(indexes.size());
            }
            continue;
        }
    }

    objFile.close();

    m_objects.append(new SimpleObject3D(vertexes, indexes, QImage(":/cube.png")));
}































