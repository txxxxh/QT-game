#include "item.h"
#include <QPainter>
#include <QBrush>

Item::Item(qreal x, qreal y, ItemType type)
    : type(type), yVelocity(0), onGround(false)
{
    setRect(0, 0, 30, 30);
    setPos(x, y);
}

void Item::applyGravity()
{
    if (!onGround) {
        yVelocity += GRAVITY;
    }
}

void Item::move()
{
    setPos(x(), y() + yVelocity);
}

void Item::checkPlatformCollision(Platform *platform)
{
    if (collidesWithItem(platform)) {
        QRectF itemRect = sceneBoundingRect();
        QRectF platformRect = platform->sceneBoundingRect();

        // 检查是否从上方着陆
        if (itemRect.bottom() >= platformRect.top() &&
            itemRect.bottom() - yVelocity <= platformRect.top()) {
            // 着陆在平台上
            setY(platformRect.top() - rect().height());
            yVelocity = 0;
            onGround = true;
        }
    }
}

void Item::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);

    QColor color;
    QString label;

    switch (type) {
    case ItemType::KNIFE:
        color = QColor(200, 200, 200);
        label = "小刀";
        break;
    case ItemType::BALL:
        color = QColor(50, 50, 50);
        label = "实心球";
        break;
    case ItemType::RIFLE:
        color = QColor(100, 100, 100);
        label = "步枪";
        break;
    case ItemType::SNIPER:
        color = QColor(70, 70, 70);
        label = "狙击枪";
        break;
    case ItemType::BANDAGE:
        color = QColor(255, 255, 255);
        label = "绷带+25";
        break;
    case ItemType::MEDKIT:
        color = QColor(255, 0, 0);
        label = "医疗箱+100";
        break;
    case ItemType::ADRENALINE:
        color = QColor(255, 255, 0);
        label = "肾上腺素";
        break;
    case ItemType::LIGHT_ARMOR:
        color = QColor(100, 100, 255);
        label = "轻甲";
        break;
    case ItemType::BULLETPROOF_VEST:
        color = QColor(50, 150, 50);
        label = "防弹衣";
        break;
    }

    painter->setBrush(QBrush(color));
    painter->drawRect(rect());

    // 绘制边框使物品更加明显
    painter->setPen(QPen(Qt::black, 2));
    painter->drawRect(rect());

    // 在物品上绘制文字
    painter->setPen(Qt::black);
    // 使用更小的字体以适应文本
    QFont font = painter->font();
    font.setPointSize(6);
    painter->setFont(font);

    // 为不同类型的物品绘制不同的图标和文本
    switch (type) {
    case ItemType::KNIFE:
    case ItemType::BALL:
    case ItemType::RIFLE:
    case ItemType::SNIPER:
    {
        // 武器类物品
        painter->drawText(rect(), Qt::AlignCenter, label);
        break;
    }
    case ItemType::BANDAGE:
    case ItemType::MEDKIT:
    {
        // 医疗类物品，绘制红十字
        painter->setBrush(Qt::red);
        painter->drawRect(rect().width()/2-2, rect().height()/4, 4, rect().height()/2);
        painter->drawRect(rect().width()/4, rect().height()/2-2, rect().width()/2, 4);
        painter->setPen(Qt::black);
        painter->drawText(rect(), Qt::AlignBottom | Qt::AlignCenter, label);
        break;
    }
    case ItemType::ADRENALINE:
    {
        // 肾上腺素，绘制闪电图标
        painter->setPen(QPen(Qt::black, 2));
        QPointF points[6] = {
            QPointF(rect().width()/2, 5),
            QPointF(rect().width()/2-5, rect().height()/2),
            QPointF(rect().width()/2+2, rect().height()/2),
            QPointF(rect().width()/2-3, rect().height()-5),
            QPointF(rect().width()/2+3, rect().height()/2+2),
            QPointF(rect().width()/2-4, rect().height()/2+2)
        };
        painter->drawPolyline(points, 6);
        painter->drawText(rect(), Qt::AlignBottom | Qt::AlignCenter, label);
        break;
    }
    case ItemType::LIGHT_ARMOR:
    {
        // 轻甲，绘制蓝色盾牌图标
        painter->setBrush(QColor(100, 100, 255, 180));
        painter->drawEllipse(rect().width()/4, rect().height()/4, rect().width()/2, rect().height()/2);
        painter->setPen(Qt::black);
        painter->drawText(rect(), Qt::AlignBottom | Qt::AlignCenter, label);
        break;
    }
    case ItemType::BULLETPROOF_VEST:
    {
        // 防弹衣，绘制绿色盾牌图标
        painter->setBrush(QColor(50, 150, 50, 180));
        QPointF shieldPoints[8] = {
            QPointF(rect().width()/2, rect().height()/5),
            QPointF(rect().width()/4, rect().height()/3),
            QPointF(rect().width()/4, rect().height()*2/3),
            QPointF(rect().width()/2, rect().height()*4/5),
            QPointF(rect().width()*3/4, rect().height()*2/3),
            QPointF(rect().width()*3/4, rect().height()/3),
            QPointF(rect().width()/2, rect().height()/5)
        };
        painter->drawPolygon(shieldPoints, 7);
        painter->setPen(Qt::black);
        painter->drawText(rect(), Qt::AlignBottom | Qt::AlignCenter, label);
        break;
    }
    }
}
