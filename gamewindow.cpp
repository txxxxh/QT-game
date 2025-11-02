#include "gamewindow.h"
#include <QLayout>
#include <QFont>
#include <QDebug>
#include "ai.h"

GameWindow::GameWindow(QWidget *parent)
    : QMainWindow(parent), gameRunning(false), gameMode(GameMode::PLAYER_VS_PLAYER), ai(nullptr)
{
    // 设置窗口大小
    gameWidth = 1200;
    gameHeight = 800;
    resize(gameWidth, gameHeight);
    setWindowTitle("2D横板射击对战游戏");

    // 确保主窗口接收所有键盘事件
    setFocusPolicy(Qt::StrongFocus);

    setupScene();
    createControls();
}

GameWindow::~GameWindow()
{
    if (ai)
        delete ai;
    delete gameTimer;
    delete itemTimer;
    delete scene;
    delete view;
}
#include <QMessageBox>
void GameWindow::setupScene()
{
    // 创建场景和视图
    scene = new QGraphicsScene(0, 0, gameWidth, gameHeight);
    view = new QGraphicsView(scene);
    view->setRenderHint(QPainter::Antialiasing);

    QPixmap backgroundPixmap("./images/vs.jpeg");
    if (!backgroundPixmap.isNull())
    {
        // 缩放图片以适应场景大小
        QPixmap scaledBackground = backgroundPixmap.scaled(gameWidth, gameHeight, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);

        // 创建背景图片项并添加到场景
        QGraphicsPixmapItem *backgroundItem = new QGraphicsPixmapItem(scaledBackground);
        backgroundItem->setPos(0, 0);
        backgroundItem->setZValue(-1000); // 确保背景在最底层
        backgroundItem->setOpacity(0.2);  // 设置背景透明度
        scene->addItem(backgroundItem);
        // QMessageBox::information(nullptr, "调试", "背景图片找到了: ");
        //  设置透明背景，让图片显示
        view->setBackgroundBrush(QBrush(Qt::transparent));
    }
    else
    {

        //  如果图片加载失败，使用默认背景色
        view->setBackgroundBrush(QBrush(QColor(30, 30, 30)));
    }
    view->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    view->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    view->setFixedSize(gameWidth, gameHeight);

    // 确保视图接收键盘事件
    view->setFocusPolicy(Qt::StrongFocus);
    view->installEventFilter(this);

    setCentralWidget(view);

    // 设置游戏定时器
    gameTimer = new QTimer(this);
    connect(gameTimer, &QTimer::timeout, this, &GameWindow::updateGame);

    // 设置物品生成定时器
    itemTimer = new QTimer(this);
    connect(itemTimer, &QTimer::timeout, this, &GameWindow::spawnItems);
}

bool GameWindow::eventFilter(QObject *obj, QEvent *event)
{
    if (obj == view)
    {
        if (event->type() == QEvent::KeyPress)
        {
            QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
            keyPressEvent(keyEvent);
            return true; // 事件已处理
        }
        else if (event->type() == QEvent::KeyRelease)
        {
            QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
            keyReleaseEvent(keyEvent);
            return true; // 事件已处理
        }
    }
    return QObject::eventFilter(obj, event);
}

void GameWindow::createControls()
{
    // 创建开始按钮 - PVP模式
    startButton = new QPushButton("玩家对战 (PVP)", this);
    startButton->setGeometry(gameWidth / 2 - 200, gameHeight / 2 - 25, 180, 50);
    startButton->setFont(QFont("Arial", 14));
    connect(startButton, &QPushButton::clicked, this, &GameWindow::startGame);

    // 创建AI对战按钮
    aiButton = new QPushButton("对战AI", this);
    aiButton->setGeometry(gameWidth / 2 + 20, gameHeight / 2 - 25, 180, 50);
    aiButton->setFont(QFont("Arial", 14));
    connect(aiButton, &QPushButton::clicked, this, &GameWindow::startAIGame);

    // 创建生命值显示
    player1HealthLabel = new QLabel(this);
    player1HealthLabel->setGeometry(20, 20, 200, 30);
    player1HealthLabel->setStyleSheet("color: white; font-size: 18px;");
    player1HealthLabel->hide();

    player2HealthLabel = new QLabel(this);
    player2HealthLabel->setGeometry(gameWidth - 220, 20, 200, 30);
    player2HealthLabel->setStyleSheet("color: white; font-size: 18px;");
    player2HealthLabel->hide();

    // 创建武器显示
    player1WeaponLabel = new QLabel(this);
    player1WeaponLabel->setGeometry(20, 50, 200, 30);
    player1WeaponLabel->setStyleSheet("color: white; font-size: 16px;");
    player1WeaponLabel->hide();

    player2WeaponLabel = new QLabel(this);
    player2WeaponLabel->setGeometry(gameWidth - 220, 50, 200, 30);
    player2WeaponLabel->setStyleSheet("color: white; font-size: 16px;");
    player2WeaponLabel->hide();

    // 创建护甲显示
    player1ArmorLabel = new QLabel(this);
    player1ArmorLabel->setGeometry(20, 80, 200, 30);
    player1ArmorLabel->setStyleSheet("color: white; font-size: 16px;");
    player1ArmorLabel->hide();

    player2ArmorLabel = new QLabel(this);
    player2ArmorLabel->setGeometry(gameWidth - 220, 80, 200, 30);
    player2ArmorLabel->setStyleSheet("color: white; font-size: 16px;");
    player2ArmorLabel->hide();

    // 创建游戏结束标签
    gameOverLabel = new QLabel(this);
    gameOverLabel->setGeometry(gameWidth / 2 - 200, gameHeight / 2 - 50, 400, 100);
    gameOverLabel->setAlignment(Qt::AlignCenter);
    gameOverLabel->setStyleSheet("color: white; font-size: 32px;");
    gameOverLabel->hide();
}

void GameWindow::startGame()
{
    // 设置游戏模式为PVP
    gameMode = GameMode::PLAYER_VS_PLAYER;

    // 重置游戏状态
    scene->clear();
    platforms.clear();
    items.clear();
    projectiles.clear();

    QPixmap backgroundPixmap("./images/vs.jpeg");
    if (!backgroundPixmap.isNull())
    {
        // 缩放图片以适应场景大小
        QPixmap scaledBackground = backgroundPixmap.scaled(gameWidth, gameHeight, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);

        // 创建背景图片项并添加到场景
        QGraphicsPixmapItem *backgroundItem = new QGraphicsPixmapItem(scaledBackground);
        backgroundItem->setPos(0, 0);
        backgroundItem->setZValue(-1000); // 确保背景在最底层
        backgroundItem->setOpacity(0.2);  // 设置背景透明度
        scene->addItem(backgroundItem);
    }

    // 移除AI
    if (ai)
    {
        delete ai;
        ai = nullptr;
    }

    // 重置按键状态
    for (int i = 0; i < 10; i++)
    {
        keys[i] = false;
    }

    // 隐藏开始按钮和游戏结束标签
    startButton->hide();
    aiButton->hide();
    gameOverLabel->hide();

    // 显示游戏信息
    player1HealthLabel->show();
    player2HealthLabel->show();
    player1WeaponLabel->show();
    player2WeaponLabel->show();
    player1ArmorLabel->show();
    player2ArmorLabel->show();

    // 设置玩家2标签
    player2HealthLabel->setText("安室透生命值: 100");
    player2WeaponLabel->setText("武器: 拳头(近战)");
    player2ArmorLabel->setText("护甲: 无");

    // 创建游戏元素
    createPlayers();
    createPlatforms();

    // 确保视图有焦点
    view->setFocus();

    // 启动游戏定时器
    gameRunning = true;
    gameTimer->start(16);   // 约60FPS
    itemTimer->start(5000); // 每5秒生成一个物品
}

void GameWindow::startAIGame()
{
    // 设置游戏模式为AI对战
    gameMode = GameMode::PLAYER_VS_AI;

    // 重置游戏状态
    scene->clear();
    platforms.clear();
    items.clear();
    projectiles.clear();

    QPixmap backgroundPixmap("./images/vs.jpeg");
    if (!backgroundPixmap.isNull())
    {
        // 缩放图片以适应场景大小
        QPixmap scaledBackground = backgroundPixmap.scaled(gameWidth, gameHeight, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);

        // 创建背景图片项并添加到场景
        QGraphicsPixmapItem *backgroundItem = new QGraphicsPixmapItem(scaledBackground);
        backgroundItem->setPos(0, 0);
        backgroundItem->setZValue(-1000); // 确保背景在最底层
        backgroundItem->setOpacity(0.2);  // 设置背景透明度
        scene->addItem(backgroundItem);
    }

    // 重置按键状态
    for (int i = 0; i < 10; i++)
    {
        keys[i] = false;
    }

    // 隐藏开始按钮和游戏结束标签
    startButton->hide();
    aiButton->hide();
    gameOverLabel->hide();

    // 显示游戏信息
    player1HealthLabel->show();
    player2HealthLabel->show();
    player1WeaponLabel->show();
    player2WeaponLabel->show();
    player1ArmorLabel->show();
    player2ArmorLabel->show();

    // 设置AI标签
    player2HealthLabel->setText("AI生命值: 100");
    player2WeaponLabel->setText("武器: 拳头(近战)");
    player2ArmorLabel->setText("护甲: 无");

    // 创建游戏元素
    createPlayers();
    createPlatforms();

    // 创建AI控制器
    ai = new AI(player2);

    // 确保视图有焦点
    view->setFocus();

    // 启动游戏定时器
    gameRunning = true;
    gameTimer->start(16);   // 约60FPS
    itemTimer->start(5000); // 每5秒生成一个物品
}

void GameWindow::createPlayers()
{
    // 创建玩家1 - 初始位置改为平台上方
    player1 = new Player(250, gameHeight - 260, QColor(0, 0, 255), 1); // 放在左侧草地平台上
    scene->addItem(player1);

    // 创建玩家2 - 初始位置改为平台上方
    player2 = new Player(850, gameHeight - 260, QColor(255, 0, 0), 2); // 放在右侧冰面平台上
    scene->addItem(player2);

    // 初始化UI显示
    player1WeaponLabel->setText("武器: 拳头(近战)");
    player1ArmorLabel->setText("护甲: 无");
    player1HealthLabel->setText("赤井秀一生命值: 100");
    // 设置玩家图片
    player2->setPlayerImage("./images/anshi.jpeg");
    player1->setPlayerImage("./images/chijing.jpeg");
}

void GameWindow::createPlatforms()
{
    // 创建地面
    Platform *ground = new Platform(0, gameHeight - 50, gameWidth, 50, PlatformType::GROUND);
    platforms.append(ground);
    scene->addItem(ground);

    // 创建草地平台（二层平台）
    Platform *grassPlatform = new Platform(200, gameHeight - 200, 300, 30, PlatformType::GRASS);
    platforms.append(grassPlatform);
    scene->addItem(grassPlatform);

    // 创建冰面平台（二层平台）
    Platform *icePlatform = new Platform(700, gameHeight - 200, 300, 30, PlatformType::ICE);
    platforms.append(icePlatform);
    scene->addItem(icePlatform);

    // 创建高层平台（三层平台）
    Platform *highPlatform1 = new Platform(150, gameHeight - 350, 200, 30, PlatformType::GROUND);
    platforms.append(highPlatform1);
    scene->addItem(highPlatform1);

    Platform *highPlatform2 = new Platform(850, gameHeight - 350, 200, 30, PlatformType::GROUND);
    platforms.append(highPlatform2);
    scene->addItem(highPlatform2);

    // 创建中间平台（三层平台）
    Platform *middlePlatform = new Platform(gameWidth / 2 - 150, gameHeight - 500, 300, 30, PlatformType::GROUND);
    platforms.append(middlePlatform);
    scene->addItem(middlePlatform);
}

void GameWindow::spawnItem()
{
    // 随机生成物品类型
    int itemTypeRand = QRandomGenerator::global()->bounded(100);
    ItemType type;

    if (itemTypeRand < 10)
    {
        type = ItemType::KNIFE;
    }
    else if (itemTypeRand < 20)
    {
        type = ItemType::BALL;
    }
    else if (itemTypeRand < 30)
    {
        type = ItemType::RIFLE;
    }
    else if (itemTypeRand < 40)
    {
        type = ItemType::SNIPER;
    }
    else if (itemTypeRand < 55)
    {
        type = ItemType::BANDAGE;
    }
    else if (itemTypeRand < 65)
    {
        type = ItemType::MEDKIT;
    }
    else if (itemTypeRand < 75)
    {
        type = ItemType::ADRENALINE;
    }
    else if (itemTypeRand < 87)
    {
        type = ItemType::LIGHT_ARMOR;
    }
    else
    {
        type = ItemType::BULLETPROOF_VEST;
    }

    // 在随机位置生成物品
    int x = QRandomGenerator::global()->bounded(100, gameWidth - 100);
    Item *item = new Item(x, 0, type);
    items.append(item);
    scene->addItem(item);
}

void GameWindow::spawnItems()
{
    if (!gameRunning)
        return;
    spawnItem();

    // 限制物品数量，防止过多
    if (items.size() > 15)
    {
        scene->removeItem(items.first());
        delete items.first();
        items.removeFirst();
    }
}

void GameWindow::keyPressEvent(QKeyEvent *event)
{
    if (!gameRunning)
    {
        QMainWindow::keyPressEvent(event);
        return;
    }

    // 调试信息，帮助查看实际按键值
    qDebug() << "Key pressed: " << event->key();

    // 玩家1控制
    if (event->key() == Qt::Key_A)
        keys[0] = true;
    if (event->key() == Qt::Key_D)
        keys[1] = true;
    if (event->key() == Qt::Key_W)
        keys[2] = true;
    if (event->key() == Qt::Key_S)
        keys[3] = true;
    if (event->key() == Qt::Key_Space)
        keys[4] = true;

    // 玩家2控制 - 仅在PVP模式下处理
    if (gameMode == GameMode::PLAYER_VS_PLAYER)
    {
        if (event->key() == 0x01000012)
            keys[5] = true; // Qt::Key_Left
        if (event->key() == 0x01000014)
            keys[6] = true; // Qt::Key_Right
        if (event->key() == 0x01000013)
            keys[7] = true; // Qt::Key_Up
        if (event->key() == 0x01000015)
            keys[8] = true; // Qt::Key_Down
        if (event->key() == Qt::Key_Return)
            keys[9] = true;
    }

    event->accept();
}

void GameWindow::keyReleaseEvent(QKeyEvent *event)
{
    if (!gameRunning)
    {
        QMainWindow::keyReleaseEvent(event);
        return;
    }

    // 玩家1控制
    if (event->key() == Qt::Key_A)
    {
        keys[0] = false;
        player1->stopMoving();
    }
    if (event->key() == Qt::Key_D)
    {
        keys[1] = false;
        player1->stopMoving();
    }
    if (event->key() == Qt::Key_W)
        keys[2] = false;
    if (event->key() == Qt::Key_S)
        keys[3] = false;
    if (event->key() == Qt::Key_Space)
        keys[4] = false;

    // 玩家2控制 - 仅在PVP模式下处理
    if (gameMode == GameMode::PLAYER_VS_PLAYER)
    {
        if (event->key() == 0x01000012)
        { // Qt::Key_Left
            keys[5] = false;
            player2->stopMoving();
        }
        if (event->key() == 0x01000014)
        { // Qt::Key_Right
            keys[6] = false;
            player2->stopMoving();
        }
        if (event->key() == 0x01000013)
            keys[7] = false; // Qt::Key_Up
        if (event->key() == 0x01000015)
            keys[8] = false; // Qt::Key_Down
        if (event->key() == Qt::Key_Return)
            keys[9] = false;
    }

    event->accept();
}

void GameWindow::updateGame()
{
    if (!gameRunning)
        return;

    // 更新玩家1状态
    if (keys[0])
        player1->moveLeft();
    if (keys[1])
        player1->moveRight();
    if (keys[2])
        player1->jump();
    if (keys[3])
        player1->crouch(true);
    else
        player1->crouch(false);
    if (keys[4])
        player1->fire(projectiles, scene);

    // 更新玩家2状态 - PVP模式
    if (gameMode == GameMode::PLAYER_VS_PLAYER)
    {
        if (keys[5])
            player2->moveLeft();
        if (keys[6])
            player2->moveRight();
        if (keys[7])
            player2->jump();
        if (keys[8])
            player2->crouch(true);
        else
            player2->crouch(false);
        if (keys[9])
            player2->fire(projectiles, scene);
    }
    // AI模式 - 更新AI
    else if (gameMode == GameMode::PLAYER_VS_AI && ai)
    {
        ai->update(player1, items, platforms, projectiles, scene);
    }

    // 应用重力和移动
    player1->applyGravity();
    player2->applyGravity();

    player1->move();
    player2->move();

    player1->updateEffects();
    player2->updateEffects();

    // 在检查碰撞前重置地面状态
    player1->setOnGround(false);
    player2->setOnGround(false);

    // 保存上一帧位置，用于碰撞检测
    QPointF player1Pos = player1->pos();
    QPointF player2Pos = player2->pos();

    // 检查玩家与平台的碰撞
    for (Platform *platform : platforms)
    {
        player1->checkPlatformCollision(platform);
        player2->checkPlatformCollision(platform);
        // 更新物品与平台的碰撞
        for (Item *item : items)
        {
            item->applyGravity();
            item->move();
            item->checkPlatformCollision(platform);
        }
    }

    // 更新投射物
    for (int i = 0; i < projectiles.size(); i++)
    {
        projectiles[i]->move();

        // 检查投射物边界
        if (projectiles[i]->x() < 0 || projectiles[i]->x() > gameWidth ||
            projectiles[i]->y() < 0 || projectiles[i]->y() > gameHeight)
        {
            scene->removeItem(projectiles[i]);
            delete projectiles[i];
            projectiles.removeAt(i);
            i--;
            continue;
        }
    }

    // 检查物品拾取和其他碰撞
    checkCollisions();

    // 更新玩家状态效果
    player1->updateEffects();
    player2->updateEffects();

    // 更新界面信息
    renderInfo();
}

void GameWindow::checkCollisions()
{
    // 检查玩家与物品碰撞（拾取）
    for (int i = 0; i < items.size(); i++)
    {
        // 只有在下蹲状态且与物品碰撞时才拾取
        if (player1->isCrouching() && player1->collidesWithItem(items[i]))
        {
            player1->pickupItem(items[i]);
            scene->removeItem(items[i]);
            delete items[i];
            items.removeAt(i);
            i--;
            continue;
        }

        if (player2->isCrouching() && player2->collidesWithItem(items[i]))
        {
            player2->pickupItem(items[i]);
            scene->removeItem(items[i]);
            delete items[i];
            items.removeAt(i);
            i--;
            continue;
        }
    }

    // 检查投射物与玩家碰撞
    for (int i = 0; i < projectiles.size(); i++)
    {
        // 检查是否击中玩家1（排除自己发射的投射物）
        if (projectiles[i]->getOwnerID() != 1 && player1->collidesWithItem(projectiles[i]))
        {
            player1->takeDamage(projectiles[i]->getDamage(), projectiles[i]->getType());
            scene->removeItem(projectiles[i]);
            delete projectiles[i];
            projectiles.removeAt(i);
            i--;

            // 检查玩家1是否已死亡
            if (player1->getHealth() <= 0)
            {
                gameOver(player2);
                return;
            }
            continue;
        }

        // 检查是否击中玩家2（排除自己发射的投射物）
        if (projectiles[i]->getOwnerID() != 2 && player2->collidesWithItem(projectiles[i]))
        {
            player2->takeDamage(projectiles[i]->getDamage(), projectiles[i]->getType());
            scene->removeItem(projectiles[i]);
            delete projectiles[i];
            projectiles.removeAt(i);
            i--;

            // 检查玩家2是否已死亡
            if (player2->getHealth() <= 0)
            {
                gameOver(player1);
                return;
            }
            continue;
        }

        // 检查投射物与平台的碰撞
        for (Platform *platform : platforms)
        {
            if (i < projectiles.size() && projectiles[i] && projectiles[i]->collidesWithItem(platform))
            {
                // 只有子弹和球才会与平台碰撞消失
                if (projectiles[i]->getType() != ProjectileType::MELEE)
                {
                    scene->removeItem(projectiles[i]);
                    delete projectiles[i];
                    projectiles.removeAt(i);
                    i--;
                    break;
                }
            }
        }
    }
}

void GameWindow::renderInfo()
{
    // 更新生命值显示
    player1HealthLabel->setText(QString("赤井秀一生命值: %1").arg(player1->getHealth()));

    if (gameMode == GameMode::PLAYER_VS_PLAYER)
    {
        player2HealthLabel->setText(QString("安室透生命值: %1").arg(player2->getHealth()));
    }
    else
    {
        player2HealthLabel->setText(QString("AI生命值: %1").arg(player2->getHealth()));
    }

    // 更新武器显示
    player1WeaponLabel->setText(QString("武器: %1").arg(player1->getWeaponName()));
    player2WeaponLabel->setText(QString("武器: %1").arg(player2->getWeaponName()));

    // 更新护甲显示
    player1ArmorLabel->setText(QString("护甲: %1").arg(player1->getArmorName()));
    player2ArmorLabel->setText(QString("护甲: %1").arg(player2->getArmorName()));
}

void GameWindow::gameOver(Player *winner)
{
    gameRunning = false;
    gameTimer->stop();
    itemTimer->stop();

    // 显示游戏结束信息
    gameOverLabel->setGeometry(gameWidth / 2 - 200, gameHeight / 2 - 150, 400, 100);
    if (winner->getPlayerID() == 1)
    {
        gameOverLabel->setText("游戏结束！\n赤井秀一胜利！");
    }
    else
    {
        if (gameMode == GameMode::PLAYER_VS_PLAYER)
        {
            gameOverLabel->setText("游戏结束！\n安室透胜利！");
        }
        else
        {
            gameOverLabel->setText("游戏结束！\nAI胜利！");
        }
    }
    gameOverLabel->show();

    // 显示开始按钮，可以重新开始游戏
    startButton->setText("玩家对战 (PVP)");
    startButton->setGeometry(gameWidth / 2 - 200, gameHeight / 2 + 50, 180, 50);
    startButton->show();

    aiButton->setText("对战AI");
    aiButton->setGeometry(gameWidth / 2 + 20, gameHeight / 2 + 50, 180, 50);
    aiButton->show();
}
