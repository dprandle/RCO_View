#include <iostream>
#include <QApplication>

#include "rco_view.h"

int main(int argc, char * argv[])
{
    QApplication app(argc,argv);
    RCO_View rcov;
    rcov.show();
    app.exec();
    return 0;
}