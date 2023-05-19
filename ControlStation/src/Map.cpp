#include "Map.h"

#include <QMouseEvent>

Map::Map(QWidget *parent) :
    QLabel(parent)
{
    setScaledContents(true);
    setPixmap(QPixmap::fromImage(QImage(":/map.png")));
}

void Map::mousePressEvent(QMouseEvent *event)
{
    qDebug() << "Map:" << event->pos();
}
