#include "camera.h"

Camera::Camera(QObject *parent)
    : QObject(parent)
{
    updateViewMatrix();
}

Camera::~Camera() = default;

void Camera::setPosition(const QVector3D &position)
{
    if (position == m_position)
        return;
    m_position = position;
    updateViewMatrix();
}

void Camera::setUpVector(const QVector3D &upVector)
{
    if (upVector == m_upVector)
        return;
    m_upVector = upVector;
    updateViewMatrix();
}

void Camera::setViewCenter(const QVector3D &viewCenter)
{
    if (viewCenter == m_viewCenter)
        return;
    m_viewCenter = viewCenter;
    updateViewMatrix();
}

void Camera::panAboutViewCenter(float angle)
{
    rotate(angle, m_upVector);
}

void Camera::tiltAboutViewCenter(float angle)
{
    const auto viewVector = (m_viewCenter - m_position).normalized();
    const auto axis = QVector3D::crossProduct(m_upVector, viewVector).normalized();
    rotate(angle, axis);
}

void Camera::rotate(float angle, const QVector3D &axis)
{
    const auto q = QQuaternion::fromAxisAndAngle(axis, angle);
    setUpVector(q * m_upVector);
    const auto viewVector = m_viewCenter - m_position;
    const auto cameraToCenter = q * viewVector;
    setPosition(viewCenter() - cameraToCenter);
    setViewCenter(position() + cameraToCenter);
}

void Camera::updateViewMatrix()
{
    m_viewMatrix.setToIdentity();
    m_viewMatrix.lookAt(m_position, m_viewCenter, m_upVector);
    emit viewMatrixChanged(m_viewMatrix);
}
