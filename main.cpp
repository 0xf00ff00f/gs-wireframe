#include "mesh.h"

#include <QApplication>
#include <QLabel>
#include <QOpenGLBuffer>
#include <QOpenGLFunctions>
#include <QOpenGLShaderProgram>
#include <QOpenGLVertexArrayObject>
#include <QOpenGLWidget>
#include <QSlider>
#include <QVBoxLayout>

#include <set>

class GLWidget : public QOpenGLWidget, protected QOpenGLFunctions
{
public:
    explicit GLWidget(QWidget *parent = nullptr);
    ~GLWidget();

    float thickness() const { return m_thickness; }
    void setThickness(float thickness);

protected:
    void resizeGL(int w, int h) override;
    void paintGL() override;
    void initializeGL() override;

private:
    void initProgram();
    void initBuffer();

    struct Vertex
    {
        QVector3D from;
        QVector3D to;
        QVector2D offset;
        QVector2D texCoord;
    };
    static_assert(sizeof(QVector3D) == 3 * sizeof(float));
    static_assert(sizeof(QVector2D) == 2 * sizeof(float));
    static_assert(sizeof(Vertex) == (3 + 3 + 2 + 2) * sizeof(float));

    QOpenGLShaderProgram m_program;
    QOpenGLVertexArrayObject m_vao;
    QOpenGLBuffer m_vbo;
    QMatrix4x4 m_model;
    QMatrix4x4 m_view;
    QMatrix4x4 m_projection;
    int m_mvpUniformSolid = -1;
    int m_mvpUniformLine = -1;
    int m_viewportSizeUniform = -1;
    int m_thicknessUniform = -1;
    std::vector<Vertex> m_wireframeVertices;
    float m_thickness = 4.0f;
};

GLWidget::GLWidget(QWidget *parent)
    : QOpenGLWidget(parent)
{
    Q_ASSERT(m_model.isIdentity());
    Q_ASSERT(m_view.isIdentity());
    Q_ASSERT(m_projection.isIdentity());

    const auto eye = 1.5 * QVector3D(3, 3, 8);
    const auto center = QVector3D(0, 0, 0);
    const auto up = QVector3D(0, 1, 0);
    m_view.lookAt(eye, center, up);
}

GLWidget::~GLWidget() = default;

void GLWidget::paintGL()
{
    glClearColor(0, 0, 0, 1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glDepthMask(GL_FALSE); // don't write to the depth buffer
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    QMatrix4x4 mvp = m_projection * m_view * m_model;

    m_program.bind();
    m_program.setUniformValue(m_mvpUniformLine, mvp);
    m_program.setUniformValue(m_viewportSizeUniform, QVector2D(width(), height()));
    m_program.setUniformValue(m_thicknessUniform, m_thickness);

    QOpenGLVertexArrayObject::Binder vaoBinder(&m_vao);

    glDrawArrays(GL_TRIANGLES, 0, m_wireframeVertices.size());

    m_model.rotate(0.1, QVector3D(0, 1, 0));
    update();
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

void GLWidget::initProgram()
{
    if (!m_program.addShaderFromSourceFile(QOpenGLShader::Vertex, ":/line.vert"))
        qWarning() << "Failed to add vertex shader:" << m_program.log();
    if (!m_program.addShaderFromSourceFile(QOpenGLShader::Fragment, ":/line.frag"))
        qWarning() << "Failed to add fragment shader:" << m_program.log();
    if (!m_program.link())
        qWarning() << "Failed to link program";
    m_mvpUniformLine = m_program.uniformLocation("mvp");
    m_viewportSizeUniform = m_program.uniformLocation("viewportSize");
    m_thicknessUniform = m_program.uniformLocation("thickness");
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

            if (seen.count(std::tuple(fromIndex, toIndex)) || seen.count(std::tuple(toIndex, fromIndex)))
                continue;

            seen.insert(std::tuple(fromIndex, toIndex));

            const auto from = positions[fromIndex] - center;
            const auto to = positions[toIndex] - center;

            const auto v0 = Vertex{from, to, QVector2D(-1, -1), QVector2D(0, 0)};
            const auto v1 = Vertex{from, to, QVector2D(-1, 1), QVector2D(0, 1)};

            const auto v2 = Vertex{from, to, QVector2D(0, -1), QVector2D(0.5, 0)};
            const auto v3 = Vertex{from, to, QVector2D(0, 1), QVector2D(0.5, 1)};

            const auto v4 = Vertex{to, from, QVector2D(0, 1), QVector2D(0.5, 0)};
            const auto v5 = Vertex{to, from, QVector2D(0, -1), QVector2D(0.5, 1)};

            const auto v6 = Vertex{to, from, QVector2D(-1, 1), QVector2D(1, 0)};
            const auto v7 = Vertex{to, from, QVector2D(-1, -1), QVector2D(1, 1)};

            m_wireframeVertices.push_back(v0);
            m_wireframeVertices.push_back(v1);
            m_wireframeVertices.push_back(v3);

            m_wireframeVertices.push_back(v3);
            m_wireframeVertices.push_back(v2);
            m_wireframeVertices.push_back(v0);

            m_wireframeVertices.push_back(v2);
            m_wireframeVertices.push_back(v3);
            m_wireframeVertices.push_back(v5);

            m_wireframeVertices.push_back(v5);
            m_wireframeVertices.push_back(v4);
            m_wireframeVertices.push_back(v2);

            m_wireframeVertices.push_back(v4);
            m_wireframeVertices.push_back(v5);
            m_wireframeVertices.push_back(v7);

            m_wireframeVertices.push_back(v7);
            m_wireframeVertices.push_back(v6);
            m_wireframeVertices.push_back(v4);
        }
    }

    m_vao.create();
    QOpenGLVertexArrayObject::Binder vaoBinder(&m_vao);

    m_vbo.create();
    m_vbo.bind();
    m_vbo.allocate(m_wireframeVertices.data(), m_wireframeVertices.size() * sizeof(Vertex));

    m_program.enableAttributeArray(0); // from
    m_program.enableAttributeArray(1); // to
    m_program.enableAttributeArray(2); // shift
    m_program.enableAttributeArray(3); // texCoord

    m_program.setAttributeBuffer(0, GL_FLOAT, offsetof(Vertex, from), 3, sizeof(Vertex));
    m_program.setAttributeBuffer(1, GL_FLOAT, offsetof(Vertex, to), 3, sizeof(Vertex));
    m_program.setAttributeBuffer(2, GL_FLOAT, offsetof(Vertex, offset), 2, sizeof(Vertex));
    m_program.setAttributeBuffer(3, GL_FLOAT, offsetof(Vertex, texCoord), 2, sizeof(Vertex));
    m_vbo.release();
}

void GLWidget::resizeGL(int w, int h)
{
    m_projection.setToIdentity();
    m_projection.perspective(45.f, static_cast<qreal>(w) / static_cast<qreal>(h), 0.1f, 100.0f);
}

class MainWindow : public QWidget
{
public:
    explicit MainWindow(QWidget *parent = nullptr);
};

MainWindow::MainWindow(QWidget *parent)
    : QWidget(parent)
{
    auto *layout = new QVBoxLayout(this);

    auto *glWidget = new GLWidget(this);
    layout->addWidget(glWidget, 1);

    auto *controlContainer = new QWidget(this);
    layout->addWidget(controlContainer);

    auto *controlLayout = new QHBoxLayout(controlContainer);
    controlLayout->addWidget(new QLabel(tr("Thickness")));

    auto *thicknessSlider = new QSlider(controlContainer);
    controlLayout->addWidget(thicknessSlider);

    thicknessSlider->setOrientation(Qt::Horizontal);
    thicknessSlider->setMinimum(0);
    thicknessSlider->setMaximum(1000);
    connect(thicknessSlider, &QAbstractSlider::valueChanged, this, [this, thicknessSlider, glWidget](int value) {
        constexpr auto kMinThickness = 0.5f;
        constexpr auto kMaxThickness = 32.0f;
        const auto t = static_cast<float>(value - thicknessSlider->minimum()) /
                       static_cast<float>(thicknessSlider->maximum() - thicknessSlider->minimum());
        const auto thickness = kMinThickness + t * (kMaxThickness - kMinThickness);
        glWidget->setThickness(thickness);
    });

    controlLayout->addStretch();
}

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    MainWindow w;
    w.resize(800, 400);
    w.show();

    return app.exec();
}
