#include "widget.h"
#include <objectengine3d.h>
#include <QOpenGLContext>
#include <QKeyEvent>
#include <group3d.h>
#include <QtMath>
#include <camera3d.h>
#include <skybox.h>
#include <material.h>
#include <QOpenGLFramebufferObject>
#include <QOpenGLFunctions>
#include <light.h>

Widget::Widget(QWidget *parent)
    : QOpenGLWidget(parent)
{
    setMouseTracking(true);
    m_camera = new Camera3D;
    m_camera->translate(QVector3D(0.0f, 0.0f, -15.0f));
    m_fbHeight = 1024;
    m_fbWidth = 1024;

    m_selectBuffer = nullptr;

    angleObject = 0;
    angleGroup1 = 0;
    angleGroup2 = 0;
    angleMain = 0;

    m_state = Idle;

    m_projectionLightMatrix.setToIdentity();
    m_projectionLightMatrix.ortho(-40, 40, -40, 40, -40, 40);

    //m_lightRotateX = 30;
    //m_lightRotateY = 40;
    //m_shadowLightMatrix.setToIdentity();
    //m_shadowLightMatrix.rotate(m_lightRotateX, 1.0f, 0.0f, 0.0f);
    //m_shadowLightMatrix.rotate(m_lightRotateY, 0.0f, 1.0f, 0.0f);
    //
    //m_lightMatrix.setToIdentity();
    //m_lightMatrix.rotate(-m_lightRotateY, 0.0f, 1.0f, 0.0f);
    //m_lightMatrix.rotate(-m_lightRotateX, 1.0f, 0.0f, 0.0f);

    m_light[0] = new Light(Light::Directional);
    m_light[0]->setPosition(QVector4D(7.0f, 7.0f, 7.0f, 1.0f));
    m_light[0]->setDirection(QVector4D(-1.0f, -1.0f, -1.0f, 0.0f));
    m_light[0]->setDiffuseColor(QVector3D(1.0f, 0.0f, 0.0f));
    m_light[0]->setCutoff(30.0f / 180.0f * M_PI);

    m_light[1] = new Light(Light::Spot);
    m_light[1]->setPosition(QVector4D(-7.0f, 7.0f, 7.0f, 1.0f));
    m_light[1]->setDirection(QVector4D(1.0f, -1.0f, -1.0f, 0.0f));
    m_light[1]->setDiffuseColor(QVector3D(0.0f, 1.0f, 0.0f));
    m_light[1]->setCutoff(50.0f / 180.0f * M_PI);

    m_light[2] = new Light(Light::Spot);
    m_light[2]->setPosition(QVector4D(7.0f, 7.0f, -7.0f, 1.0f));
    m_light[2]->setDirection(QVector4D(-1.0f, -1.0f, 1.0f, 0.0f));
    m_light[2]->setDiffuseColor(QVector3D(0.0f, 0.0f, 1.0f));
    m_light[2]->setCutoff(35.0f / 180.0f * M_PI);
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
    } else if (event->buttons() == Qt::RightButton)
    {
        QVector3D t = screenCoordsToWorldCoords(QVector2D(event->localPos()));

        if (m_state == Idle)
        {
            m_firstPoint = t;
            m_movePoint = t;

            QImage diffuseMapCube(":/cube.png");
            QImage normalMapCube(":/cube_normal.jpg");
            initCube(1.0f, 1.0f, 1.0f, &diffuseMapCube, &normalMapCube);
            m_objects[m_objects.size() - 1]->scale(0.0f, 0.0f, 0.0f);
            m_objects[m_objects.size() - 1]->translate(t);
            m_TransformObject.append(m_objects[m_objects.size() - 1]);

            m_state = CreatePlane;
        }else if (m_state == CreateHeight)
        {
            QVector3D diff = t - m_secondPoint;
            QVector3D diffOld = m_secondPoint - m_firstPoint;

            m_objects[m_objects.size() - 1]->scale(diffOld.x(), diff.y(), diffOld.z());
            m_objects[m_objects.size() - 1]->translate(QVector3D(0.0f, (t - m_movePoint).y() / 2.0f, 0.0f));

            m_state = Idle;
        }

        update();
    } else if (event->buttons() == Qt::MiddleButton)
    {
        int res = selectObject(event->x(), event->y(), m_selectObjects);
        qDebug() << res;
    }
    event->accept();
}

void Widget::mouseReleaseEvent(QMouseEvent *event)
{
    if (m_state == CreatePlane)
    {
        QVector3D t = screenCoordsToWorldCoords(QVector2D(event->localPos()));
        QVector3D diff = t - m_firstPoint;

        m_objects[m_objects.size() - 1]->scale(diff.x(), 0.0f, diff.z());
        m_objects[m_objects.size() - 1]->translate((t - m_movePoint) / 2.0f);

        m_movePoint = t;
        m_secondPoint = t;

        m_state = CreateHeight;
    }
}

void Widget::mouseMoveEvent(QMouseEvent *event)
{
    //if (event->buttons() != Qt::LeftButton && event->buttons() != Qt::RightButton) return;

    if (event->buttons() == Qt::LeftButton)
    {
        QVector2D diff = QVector2D(event->localPos()) - m_mousePosition;
        m_mousePosition = QVector2D(event->localPos());

        //float angle = diff.length() / 2.0;
        //QVector3D axis = QVector3D(diff.y(), diff.x(), 0.0);
        //m_camera->rotate(QQuaternion::fromAxisAndAngle(axis, angle));

        float angelX = diff.y() / 2.0f;
        float angelY = diff.x() / 2.0f;
        m_camera->rotateX(QQuaternion::fromAxisAndAngle(1.0f, 0.0f, 0.0f, angelX));
        m_camera->rotateY(QQuaternion::fromAxisAndAngle(0.0f, 1.0f, 0.0f, angelY));
    } else if (event->buttons() == Qt::RightButton)
    {
        QVector3D t = screenCoordsToWorldCoords(QVector2D(event->localPos()));

        if (m_state == CreatePlane)
        {
            QVector3D diff = t - m_firstPoint;

            m_objects[m_objects.size() - 1]->scale(diff.x(), 0.0f, diff.z());
            m_objects[m_objects.size() - 1]->translate((t - m_movePoint) / 2.0f);

            m_movePoint = t;
        }
    } else if (event->buttons() == Qt::NoButton)
    {
        QVector3D t = screenCoordsToWorldCoords(QVector2D(event->localPos()));

        if (m_state == CreateHeight)
        {
            QVector3D diff = t - m_secondPoint;
            QVector3D diffOld = m_secondPoint - m_firstPoint;

            m_objects[m_objects.size() - 1]->scale(diffOld.x(), diff.y(), diffOld.z());
            m_objects[m_objects.size() - 1]->translate(QVector3D(0.0f, (t - m_movePoint).y() / 2.0f, 0.0f));

            m_movePoint = t;
        }
    }


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

    for(int i = 0; i < m_objects.size() - 3; i++)
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

int Widget::selectObject(int x, int y, QVector<Transformational *> &objs)
{
    m_selectBuffer->bind();

    glViewport(0, 0, width(), height());
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glEnable(GL_DEPTH_TEST);

    m_programSelect.bind();
    m_programSelect.setUniformValue("u_projectionMatrix", m_projectionMatrix);

    m_camera->draw(&m_programSelect);

    for (int i = 0; i < objs.size(); i++)
    {
        m_programSelect.setUniformValue("u_code", (float)((i + 1) * 64));
        objs[i]->draw(&m_programSelect, context()->functions(), false);
    }
    m_programSelect.release();

    GLint viewport[4];
    glGetIntegerv(GL_VIEWPORT, viewport);
    unsigned char res[4];
    glReadPixels(x, viewport[3] - y, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, &res[0]);

    glDisable(GL_DEPTH_TEST);

    m_selectBuffer->toImage().save("123.bmp");

    m_selectBuffer->release();

    return res[0];
}

QVector3D Widget::screenCoordsToWorldCoords(const QVector2D &mousePos)
{
    QVector4D tmp(2.0f * mousePos.x() / width() - 1.0f,
                  -2.0f * mousePos.y() / height() + 1.0f,
                  -1.0f,
                  1.0f);

    QVector4D iTmp((m_projectionMatrix.inverted() * tmp).toVector2D(), -1.0f, 0.0f);

    QVector3D Direction((m_camera->getViewMatrix().inverted() * iTmp).toVector3D().normalized());

    QVector3D camPos((m_camera->getViewMatrix().inverted() * QVector4D(0.0f, 0.0f, 0.0f, 1.0f)).toVector3D());

    // Ax + By + Cz + D = 0   уравнение плоскости
    // Norm = (A, B, C)
    // P * N - P0 * N = 0 уравнение плоскости в векторном виде
    // O + D * t
    // (O + D * t) * N - P0 * N = 0
    // t = (P0 * N - O * N) / (D * N)
    // result = O + D * t

    QVector3D N;
    QVector3D P0;
    if (m_state == CreatePlane || m_state == Idle)
    {
        N = QVector3D(0.0f, 1.0f, 0.0f);
        P0 = QVector3D(0.0f, 0.0f, 0.0f);
    } else if (m_state == CreateHeight)
    {
        N = QVector3D(0.0f, 0.0f, 1.0f);
        P0 = m_secondPoint;
    }
    float t = (QVector3D::dotProduct(P0, N) - QVector3D::dotProduct(camPos, N)) / QVector3D::dotProduct(Direction, N);
    QVector3D result = camPos + Direction * t;

    return result;
}

void Widget::initializeGL()
{
    glClearColor(0.3f, 0.3f, 0.3f, 1.0f);

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);

    initShaders();

    m_groups.append(new Group3D);

    m_objects.append(new ObjectEngine3D);
    m_objects[m_objects.size() - 1]->loadObjectFromFile(":/model/bibika.obj");
    m_objects[m_objects.size() - 1]->scale(3.0f);

    m_groups[m_groups.size() - 1]->addObject(m_objects[m_objects.size() - 1]);
    m_selectObjects.append(m_objects[m_objects.size() - 1]);

    m_objects.append(new ObjectEngine3D);
    m_objects[m_objects.size() - 1]->loadObjectFromFile(":/model/bibika.obj");
    m_objects[m_objects.size() - 1]->scale(3.0f);

    m_groups.append(new Group3D);
    m_groups[m_groups.size() - 1]->addObject(m_objects[m_objects.size() - 1]);
    m_selectObjects.append(m_objects[m_objects.size() - 1]);

    m_groups[0]->translate(QVector3D(-8.0f, 0.0f, 0.0f));
    m_groups[1]->translate(QVector3D(8.0f, 0.0f, 0.0f));

    m_groups.append(new Group3D);
    m_groups[2]->addObject(m_groups[0]);
    m_groups[2]->addObject(m_groups[1]);

    m_TransformObject.append(m_groups[2]);

    m_groups.append(new Group3D);
    QImage diffuseMapCube(":/cube.png");
    QImage normalMapCube(":/cube_normal.jpg");
    initCube(2.0f, 2.0f, 2.0f, &diffuseMapCube, &normalMapCube);
    m_groups[m_groups.size() - 1]->addObject(m_objects[m_objects.size() - 1]);
    m_selectObjects.append(m_objects[m_objects.size() - 1]);

    m_groups[m_groups.size() - 1]->addObject(m_camera);

    m_TransformObject.append(m_groups[3]);

    QImage diffuseMap(":/ground.jpg");
    initCube(40.0f, 2.0f, 40.0f, &diffuseMap);
    m_objects[m_objects.size() - 1]->translate(QVector3D(0.0f, -2.0f, 0.0f));
    m_TransformObject.append(m_objects[m_objects.size() - 1]);

    m_skybox = new SkyBox(100, QImage(":/sky.png"));

    m_depthBuffer = new QOpenGLFramebufferObject(m_fbWidth, m_fbHeight, QOpenGLFramebufferObject::Depth);
    m_selectBuffer = new QOpenGLFramebufferObject(width(), height());
    //m_timer.start(30, this);
}

void Widget::resizeGL(int w, int h)
{
    float aspect = w / (h ? (float)h : 1);
    m_projectionMatrix.setToIdentity();
    m_projectionMatrix.perspective(45, aspect, 0.01f, 1000.0f);

    if (m_selectBuffer != 0) delete m_selectBuffer;
    m_selectBuffer = new QOpenGLFramebufferObject(w, h);
}

void Widget::paintGL()
{
    // Отрисовка во фрейм буфер
    m_depthBuffer->bind();

    glViewport(0, 0, m_fbWidth, m_fbHeight);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    m_programDepth.bind();
    m_programDepth.setUniformValue("u_projectionLightMatrix", m_projectionLightMatrix);
    m_programDepth.setUniformValue("u_shadowLightMatrix", m_light[0]->getLightMatrix());

    for(int i = 0; i < m_TransformObject.size(); i++)
    {
        m_TransformObject[i]->draw(&m_programDepth, context()->functions());
    }

    m_depthBuffer->release();

    GLuint texture = m_depthBuffer->texture();

    context()->functions()->glActiveTexture(GL_TEXTURE4);
    context()->functions()->glBindTexture(GL_TEXTURE_2D, texture);

    // Отрисовка на экран
    glViewport(0, 0, width(), height());
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    m_programSkybox.bind();
    m_programSkybox.setUniformValue("u_projectionMatrix", m_projectionMatrix);

    m_camera->draw(&m_programSkybox);
    m_skybox->draw(&m_programSkybox, context()->functions());
    m_programSkybox.release();

    m_program.bind();
    m_program.setUniformValue("u_shadowMap", GL_TEXTURE4 - GL_TEXTURE0);
    m_program.setUniformValue("u_projectionMatrix", m_projectionMatrix);
//    m_program.setUniformValue("u_lightDirection", QVector4D(0.0, 0.0, -1.0, 0.0));
    m_program.setUniformValue("u_projectionLightMatrix", m_projectionLightMatrix);
    m_program.setUniformValue("u_shadowLightMatrix", m_light[0]->getLightMatrix());
//    m_program.setUniformValue("u_lightMatrix", m_lightMatrix);
    m_program.setUniformValue("u_lightPower", 1.0f);
    for (int i = 0; i < 3; i++)
    {
        m_program.setUniformValue(QString("u_lightProperty[%1].ambienceColor").arg(i).toLatin1().data(), m_light[i]->getAmbienceColor());
        m_program.setUniformValue(QString("u_lightProperty[%1].diffuseColor").arg(i).toLatin1().data(), m_light[i]->getDiffuseColor());
        m_program.setUniformValue(QString("u_lightProperty[%1].specularColor").arg(i).toLatin1().data(), m_light[i]->getSpecularColor());
        m_program.setUniformValue(QString("u_lightProperty[%1].position").arg(i).toLatin1().data(), m_light[i]->getPosition());
        m_program.setUniformValue(QString("u_lightProperty[%1].direction").arg(i).toLatin1().data(), m_light[i]->getDirection());
        m_program.setUniformValue(QString("u_lightProperty[%1].cutoff").arg(i).toLatin1().data(), m_light[i]->getCutoff());
        m_program.setUniformValue(QString("u_lightProperty[%1].type").arg(i).toLatin1().data(), m_light[i]->getType());
    }
    m_program.setUniformValue("u_countLights", 3);
    m_program.setUniformValue("u_indexLightForShadow", 0);


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

    if (!m_programDepth.addShaderFromSourceFile(QOpenGLShader::Vertex, ":/depth.vsh"))
        close();
    if (!m_programDepth.addShaderFromSourceFile(QOpenGLShader::Fragment, ":/depth.fsh"))
        close();
    if (!m_programDepth.link())
        close();

    if (!m_programSelect.addShaderFromSourceFile(QOpenGLShader::Vertex, ":/select.vsh"))
        close();
    if (!m_programSelect.addShaderFromSourceFile(QOpenGLShader::Fragment, ":/select.fsh"))
        close();
    if (!m_programSelect.link())
        close();
}

void Widget::initCube(float width, float height, float depth, QImage *diffuseMap, QImage *normalMap)
{
    float width_div_2 = width / 2.0f;
    float height_div_2 = height / 2.0f;
    float depth_div_2 = depth / 2.0f;

    QVector<VertexData> vertexes;
    vertexes.append(VertexData(QVector3D(-width_div_2, height_div_2, depth_div_2), QVector2D(0.0f, 1.0f), QVector3D(0.0f, 0.0f, 1.0f)));
    vertexes.append(VertexData(QVector3D(-width_div_2, -height_div_2, depth_div_2), QVector2D(0.0f, 0.0f), QVector3D(0.0f, 0.0f, 1.0f)));
    vertexes.append(VertexData(QVector3D(width_div_2, height_div_2, depth_div_2), QVector2D(1.0f, 1.0f), QVector3D(0.0f, 0.0f, 1.0f)));
    vertexes.append(VertexData(QVector3D(width_div_2, height_div_2, depth_div_2), QVector2D(1.0f, 1.0f), QVector3D(0.0f, 0.0f, 1.0f)));
    vertexes.append(VertexData(QVector3D(-width_div_2, -height_div_2, depth_div_2), QVector2D(0.0f, 0.0f), QVector3D(0.0f, 0.0f, 1.0f)));
    vertexes.append(VertexData(QVector3D(width_div_2, -height_div_2, depth_div_2), QVector2D(1.0f, 0.0f), QVector3D(0.0f, 0.0f, 1.0f)));

    vertexes.append(VertexData(QVector3D(width_div_2, height_div_2, depth_div_2), QVector2D(0.0f, 1.0f), QVector3D(1.0f, 0.0f, 0.0f)));
    vertexes.append(VertexData(QVector3D(width_div_2, -height_div_2, depth_div_2), QVector2D(0.0f, 0.0f), QVector3D(1.0f, 0.0f, 0.0f)));
    vertexes.append(VertexData(QVector3D(width_div_2, height_div_2, -depth_div_2), QVector2D(1.0f, 1.0f), QVector3D(1.0f, 0.0f, 0.0f)));
    vertexes.append(VertexData(QVector3D(width_div_2, height_div_2, -depth_div_2), QVector2D(1.0f, 1.0f), QVector3D(1.0f, 0.0f, 0.0f)));
    vertexes.append(VertexData(QVector3D(width_div_2, -height_div_2, depth_div_2), QVector2D(0.0f, 0.0f), QVector3D(1.0f, 0.0f, 0.0f)));
    vertexes.append(VertexData(QVector3D(width_div_2, -height_div_2, -depth_div_2), QVector2D(1.0f, 0.0f), QVector3D(1.0f, 0.0f, 0.0f)));

    vertexes.append(VertexData(QVector3D(width_div_2, height_div_2, depth_div_2), QVector2D(0.0f, 1.0f), QVector3D(0.0f, 1.0f, 0.0f)));
    vertexes.append(VertexData(QVector3D(width_div_2, height_div_2, -depth_div_2), QVector2D(0.0f, 0.0f), QVector3D(0.0f, 1.0f, 0.0f)));
    vertexes.append(VertexData(QVector3D(-width_div_2, height_div_2, depth_div_2), QVector2D(1.0f, 1.0f), QVector3D(0.0f, 1.0f, 0.0f)));
    vertexes.append(VertexData(QVector3D(-width_div_2, height_div_2, depth_div_2), QVector2D(1.0f, 1.0f), QVector3D(0.0f, 1.0f, 0.0f)));
    vertexes.append(VertexData(QVector3D(width_div_2, height_div_2, -depth_div_2), QVector2D(0.0f, 0.0f), QVector3D(0.0f, 1.0f, 0.0f)));
    vertexes.append(VertexData(QVector3D(-width_div_2, height_div_2, -depth_div_2), QVector2D(1.0f, 0.0f), QVector3D(0.0f, 1.0f, 0.0f)));

    vertexes.append(VertexData(QVector3D(width_div_2, height_div_2, -depth_div_2), QVector2D(0.0f, 1.0f), QVector3D(0.0f, 0.0f, -1.0f)));
    vertexes.append(VertexData(QVector3D(width_div_2, -height_div_2, -depth_div_2), QVector2D(0.0f, 0.0f), QVector3D(0.0f, 0.0f, -1.0f)));
    vertexes.append(VertexData(QVector3D(-width_div_2, height_div_2, -depth_div_2), QVector2D(1.0f, 1.0f), QVector3D(0.0f, 0.0f, -1.0f)));
    vertexes.append(VertexData(QVector3D(-width_div_2, height_div_2, -depth_div_2), QVector2D(1.0f, 1.0f), QVector3D(0.0f, 0.0f, -1.0f)));
    vertexes.append(VertexData(QVector3D(width_div_2, -height_div_2, -depth_div_2), QVector2D(0.0f, 0.0f), QVector3D(0.0f, 0.0f, -1.0f)));
    vertexes.append(VertexData(QVector3D(-width_div_2, -height_div_2, -depth_div_2), QVector2D(1.0f, 0.0f), QVector3D(0.0f, 0.0f, -1.0f)));

    vertexes.append(VertexData(QVector3D(-width_div_2, height_div_2, depth_div_2), QVector2D(0.0f, 1.0f), QVector3D(-1.0f, 0.0f, 0.0f)));
    vertexes.append(VertexData(QVector3D(-width_div_2, height_div_2, -depth_div_2), QVector2D(0.0f, 0.0f), QVector3D(-1.0f, 0.0f, 0.0f)));
    vertexes.append(VertexData(QVector3D(-width_div_2, -height_div_2, depth_div_2), QVector2D(1.0f, 1.0f), QVector3D(-1.0f, 0.0f, 0.0f)));
    vertexes.append(VertexData(QVector3D(-width_div_2, -height_div_2, depth_div_2), QVector2D(1.0f, 1.0f), QVector3D(-1.0f, 0.0f, 0.0f)));
    vertexes.append(VertexData(QVector3D(-width_div_2, height_div_2, -depth_div_2), QVector2D(0.0f, 0.0f), QVector3D(-1.0f, 0.0f, 0.0f)));
    vertexes.append(VertexData(QVector3D(-width_div_2, -height_div_2, -depth_div_2), QVector2D(1.0f, 0.0f), QVector3D(-1.0f, 0.0f, 0.0f)));

    vertexes.append(VertexData(QVector3D(-width_div_2, -height_div_2, depth_div_2), QVector2D(0.0f, 1.0f), QVector3D(0.0f, -1.0f, 0.0f)));
    vertexes.append(VertexData(QVector3D(-width_div_2, -height_div_2, -depth_div_2), QVector2D(0.0f, 0.0f), QVector3D(0.0f, -1.0f, 0.0f)));
    vertexes.append(VertexData(QVector3D(width_div_2, -height_div_2, depth_div_2), QVector2D(1.0f, 1.0f), QVector3D(0.0f, -1.0f, 0.0f)));
    vertexes.append(VertexData(QVector3D(width_div_2, -height_div_2, depth_div_2), QVector2D(1.0f, 1.0f), QVector3D(0.0f, -1.0f, 0.0f)));
    vertexes.append(VertexData(QVector3D(-width_div_2, -height_div_2, -depth_div_2), QVector2D(0.0f, 0.0f), QVector3D(0.0f, -1.0f, 0.0f)));
    vertexes.append(VertexData(QVector3D(width_div_2, -height_div_2, -depth_div_2), QVector2D(1.0f, 0.0f), QVector3D(0.0f, -1.0f, 0.0f)));

    QVector<GLuint> indexes;
    for(GLuint i = 0; i < 36; ++i) indexes.append(i);

    Material *newMtl = new Material;
    if (diffuseMap)
        newMtl->setDiffuseMap(*diffuseMap);
    if (normalMap)
        newMtl->setNormalMap(*normalMap);
    newMtl->setShinnes(96);
    newMtl->setDiffuseColor(QVector3D(1.0f, 1.0f, 1.0f));
    newMtl->setAmbienceColor(QVector3D(1.0f, 1.0f, 1.0f));
    newMtl->setSpecularColor(QVector3D(1.0f, 1.0f, 1.0f));

    ObjectEngine3D *newObj = new ObjectEngine3D;
    newObj->calculateTBN(vertexes);
    newObj->addObject(new SimpleObject3D(vertexes, indexes, newMtl));

    m_objects.append(newObj);
}





























