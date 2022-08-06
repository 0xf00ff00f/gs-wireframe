#pragma once

#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include <QOpenGLVertexArrayObject>
#include <QOpenGLBuffer>
#include <QMatrix4x4>

class Camera;
class QOpenGLShaderProgram;

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
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;

private:
    void initProgram();
    void initBuffer();

    QOpenGLShaderProgram *m_program = nullptr;
    QOpenGLVertexArrayObject m_vao;
    QOpenGLBuffer m_vbo;
    Camera *m_camera;
    QMatrix4x4 m_model;
    QMatrix4x4 m_projection;
    int m_mvpUniform = -1;
    int m_viewportSizeUniform = -1;
    int m_thicknessUniform = -1;
    std::vector<QVector3D> m_wireframeVertices;
    float m_thickness = 4.0f;
    QPoint m_lastMousePos;
};
