#include "clicklabel.h"

ClickLabel::ClickLabel(QWidget *parent):
    QLabel(parent)
{
    setMaximumSize(QSize(16,16));
    setMaximumWidth(25);
}

ClickLabel::~ClickLabel()
{

}

void ClickLabel::mousePressEvent(QMouseEvent *event)
{
    if(event->button() == Qt::LeftButton)
    {
            emit clicked();
    }

}

void ClickLabel::setIcon(const QIcon &icon)
{
    QPixmap img = icon.pixmap(icon.actualSize(QSize(16, 16)));
    this->setPixmap(img);
}
