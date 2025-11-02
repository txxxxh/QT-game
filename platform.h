#ifndef PLATFORM_H
#define PLATFORM_H

#include <QGraphicsRectItem>
#include <QColor>

enum class PlatformType
{
    GROUND,
    GRASS,
    ICE
};

class Platform : public QGraphicsRectItem
{
public:
    // 两个构造函数，兼容不同调用方式
    Platform(PlatformType type, qreal width, qreal height);
    Platform(qreal x, qreal y, qreal width, qreal height, PlatformType type);

    PlatformType getType() const { return type; }

    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;

private:
    PlatformType type;
    QPixmap platformImage;
    void loadPlatformImage();
};

#endif // PLATFORM_H
