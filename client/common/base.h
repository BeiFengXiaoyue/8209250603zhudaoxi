#ifndef BASE_H
#define BASE_H

#include <QWidget>
#include <QString>
class Base: public QWidget
{
    Q_OBJECT
public:
    virtual QString getName()=0;
    virtual void setStyleSheet()=0;
    virtual void init()=0;
    virtual void show()=0;
    virtual void refresh()=0;
    explicit Base(QWidget *parent=nullptr);
    ~Base()=default;
};

#endif // BASE_H