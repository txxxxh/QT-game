#include "platform.h"
#include <QPainter>
#include <QBrush>

// 第一个构造函数 - 用于 GameWindow::createPlatforms() 中的调用
Platform::Platform(PlatformType type, qreal width, qreal height)
    : type(type)
{
    setRect(0, 0, width, height);
    loadPlatformImage(); // 加载对应类型的图片
}

// 第二个构造函数 - 兼容其他可能的调用
Platform::Platform(qreal x, qreal y, qreal width, qreal height, PlatformType type)
    : type(type)
{
    setRect(0, 0, width, height);
    setPos(x, y);
    loadPlatformImage(); // 加载对应类型的图片
}

void Platform::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);

    if (!platformImage.isNull())
    {
        // 平铺图片填充整个平台区域
        QRectF targetRect = rect();

        // 方法1：缩放图片以适应平台大小
        QPixmap scaledImage = platformImage.scaled(targetRect.width(), targetRect.height(),
                                                   Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
        painter->drawPixmap(targetRect.toRect(), scaledImage);
    }
    else
    {
        // 如果图片加载失败，使用默认颜色
        QBrush brush;
        switch (type)
        {
        case PlatformType::GROUND:
            brush = QBrush(QColor(139, 69, 19)); // 棕色
            break;
        case PlatformType::GRASS:
            brush = QBrush(QColor(34, 139, 34)); // 绿色
            break;
        case PlatformType::ICE:
            brush = QBrush(QColor(173, 216, 230)); // 浅蓝色
            break;
        }
        painter->setBrush(brush);
        painter->drawRect(rect());
    }
}
#include <QMessageBox>
void Platform::loadPlatformImage()
{
    QString imagePath;

    switch (type)
    {
    case PlatformType::GROUND:
        imagePath = "./images/ground.png";
        break;
    case PlatformType::GRASS:
        imagePath = "./images/grass.png";
        break;
    case PlatformType::ICE:
        imagePath = "./images/ice.jpeg";
        break;
    }

    platformImage.load(imagePath);
    if (platformImage.isNull())
    {
        // QMessageBox::warning(nullptr, "错误", "平台图片未找到，请检查路径: " + imagePath);
    }
    else
    {
        // qDebug() << "平台图片加载成功:" << imagePath;
        // QMessageBox::information(nullptr, "调试", "平台图片加载成功: " + imagePath);
    }
}
