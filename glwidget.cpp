#include "glwidget.h"

#include "camera.h"
#include "mesh.h"

#include <QOpenGLShaderProgram>
#include <QMouseEvent>

#include <set>

GLWidget::GLWidget(QWidget *parent)
    : QOpenGLWidget(parent)
    , m_camera(new Camera(this))
{
    Q_ASSERT(m_model.isIdentity());
    Q_ASSERT(m_projection.isIdentity());

    connect(m_camera, &Camera::viewMatrixChanged, this, [this] { update(); });
}

GLWidget::~GLWidget()
{
    makeCurrent();
    m_vbo.destroy();
    m_vao.destroy();
    delete m_program;
    doneCurrent();
}

void GLWidget::paintGL()
{
    glClearColor(0, 0, 0, 1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glDepthMask(GL_FALSE); // don't write to the depth buffer
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    QMatrix4x4 mvp = m_projection * m_camera->viewMatrix() * m_model;

    m_program->bind();
    m_program->setUniformValue(m_mvpUniform, mvp);
    m_program->setUniformValue(m_viewportSizeUniform, QVector2D(width(), height()));
    m_program->setUniformValue(m_thicknessUniform, m_thickness);

    {
        QOpenGLVertexArrayObject::Binder vaoBinder(&m_vao);
        glDrawArrays(GL_LINES, 0, m_wireframeVertices.size());
    }
}

void GLWidget::setThickness(float thickness)
{
    if (thickness == m_thickness)
        return;
    m_thickness = thickness;
    update();
}

void GLWidget::initializeGL()
{
    initializeOpenGLFunctions();

    initProgram();
    initBuffer();
}

void GLWidget::mousePressEvent(QMouseEvent *event)
{
    m_lastMousePos = event->pos();
    m_cameraCommand = [event] {
        switch (event->button())
        {
        case Qt::LeftButton:
            return CameraCommand::Pan;
        case Qt::RightButton:
            return CameraCommand::Rotate;
        default:
            return CameraCommand::None;
        }
    }();
    event->accept();
}

void GLWidget::mouseMoveEvent(QMouseEvent *event)
{
    const auto offset = QVector2D(event->pos() - m_lastMousePos);

    switch (m_cameraCommand)
    {
    case CameraCommand::Rotate: {
        constexpr auto RotateSpeed = 0.2f;
        m_camera->panAboutViewCenter(-RotateSpeed * offset.x());
        m_camera->tiltAboutViewCenter(RotateSpeed * offset.y());
        break;
    }
    case CameraCommand::Pan: {
        constexpr auto PanSpeed = 0.01f;
        const auto l = (m_camera->viewCenter() - m_camera->position()).length();
        m_camera->translate(PanSpeed * QVector3D(-offset.x(), offset.y(), 0));
        break;
    }
    default:
        break;
    }

    m_lastMousePos = event->pos();
    event->accept();
}

void GLWidget::wheelEvent(QWheelEvent *event)
{
    constexpr auto ZoomSpeed = 0.01f;
    const auto dz = ZoomSpeed * event->angleDelta().y();
    m_camera->zoom(dz);
}

void GLWidget::initProgram()
{
    m_program = new QOpenGLShaderProgram;
    if (!m_program->addShaderFromSourceFile(QOpenGLShader::Vertex, ":/line.vert"))
        qWarning() << "Failed to add vertex shader:" << m_program->log();
    if (!m_program->addShaderFromSourceFile(QOpenGLShader::Geometry, ":/line.geom"))
        qWarning() << "Failed to add vertex shader:" << m_program->log();
    if (!m_program->addShaderFromSourceFile(QOpenGLShader::Fragment, ":/line.frag"))
        qWarning() << "Failed to add fragment shader:" << m_program->log();
    if (!m_program->link())
        qWarning() << "Failed to link program";
    m_mvpUniform = m_program->uniformLocation("mvp");
    m_viewportSizeUniform = m_program->uniformLocation("viewportSize");
    m_thicknessUniform = m_program->uniformLocation("thickness");
}

void GLWidget::initBuffer()
{
    auto mesh = loadMesh(":/cow.obj");
    if (!mesh)
        qWarning() << "Failed to load mesh";
    qDebug("%lu faces, %lu vertices", mesh->faces.size(), mesh->positions.size());

    const auto &positions = mesh->positions;
    QVector3D center = std::accumulate(positions.begin(), positions.end(), QVector3D(0, 0, 0));
    center *= 1.0f / static_cast<float>(positions.size());

    // wireframe

    std::set<std::tuple<int, int>> seen;

    for (const auto &face : mesh->faces)
    {
        for (size_t i = 0, count = face.size(); i < count; ++i)
        {
            const auto j = (i + 1) % count;
            const auto fromIndex = face[i].positionIndex;
            const auto toIndex = face[j].positionIndex;

            if (seen.count({fromIndex, toIndex}) || seen.count({toIndex, fromIndex}))
                continue;
            seen.insert({fromIndex, toIndex});

            const auto from = positions[fromIndex] - center;
            m_wireframeVertices.push_back(from);

            const auto to = positions[toIndex] - center;
            m_wireframeVertices.push_back(to);
        }
    }

    m_vao.create();
    QOpenGLVertexArrayObject::Binder vaoBinder(&m_vao);

    m_vbo.create();
    m_vbo.bind();
    m_vbo.allocate(m_wireframeVertices.data(), m_wireframeVertices.size() * sizeof(QVector3D));

    m_program->enableAttributeArray(0); // position
    m_program->setAttributeBuffer(0, GL_FLOAT, 0, 3, sizeof(QVector3D));
    m_vbo.release();
}

void GLWidget::resizeGL(int w, int h)
{
    m_projection.setToIdentity();
    m_projection.perspective(45.f, static_cast<qreal>(w) / static_cast<qreal>(h), 0.1f, 100.0f);
}
