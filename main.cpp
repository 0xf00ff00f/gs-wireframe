#include "glwidget.h"

#include <QApplication>
#include <QLabel>
#include <QSlider>
#include <QVBoxLayout>
#include <QMouseEvent>

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
        constexpr auto kMaxThickness = 128.0f;
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
    w.resize(1200, 600);
    w.show();

    return app.exec();
}
