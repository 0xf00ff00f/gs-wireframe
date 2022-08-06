#include "mesh.h"

#include <QFile>
#include <QRegularExpression>

std::optional<Mesh> loadMesh(const QString &name)
{
    QFile file(name);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return std::nullopt;

    Mesh mesh;

    while (!file.atEnd())
    {
        const QString line = file.readLine();
        const QStringList tokens = line.split(QRegularExpression("\\s+"), Qt::SkipEmptyParts);
        if (tokens.empty())
            continue;
        if (tokens.front() == "v")
        {
            assert(tokens.size() == 4);
            mesh.positions.emplace_back(tokens[1].toFloat(), tokens[2].toFloat(), tokens[3].toFloat());
        }
        else if (tokens.front() == "vn")
        {
            assert(tokens.size() == 4);
            mesh.normals.emplace_back(tokens[1].toFloat(), tokens[2].toFloat(), tokens[3].toFloat());
        }
        else if (tokens.front() == "f")
        {
            Mesh::Face f;
            for (auto it = std::next(tokens.begin()); it != tokens.end(); ++it)
            {
                const QStringList components = it->split(QLatin1Char('/'));
                int position = components[0].toInt() - 1;
                int normal = -1;
                if (components.size() == 3)
                    normal = components[2].toInt() - 1;
                f.push_back({position, normal});
            }
            mesh.faces.push_back(f);
        }
    }

    return mesh;
}
