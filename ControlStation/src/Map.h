#ifndef MAP_H
#define MAP_H

#include <QLabel>

class Map : public QLabel
{
public:
    Map(QWidget *parent = nullptr);

protected:
    void mousePressEvent(QMouseEvent *event) override;
};

#endif // MAP_H
