#pragma once

#include <QString>
#include <QVector3D>

#include <vector>
#include <optional>

struct Mesh
{
    std::vector<QVector3D> positions;
    std::vector<QVector3D> normals;
    struct Vertex
    {
        int positionIndex;
        int normalIndex;
    };
    using Face = std::vector<Vertex>;
    std::vector<Face> faces;
};

std::optional<Mesh> loadMesh(const QString &name);
