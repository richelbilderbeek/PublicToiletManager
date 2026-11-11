/*
 *  Public Toilet Manager 5 Beta Edition - New Art Project.
 *  Copyright (C) 2006  Joost van den Bogaart
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include <QApplication>
#include <QIcon>

#include "qtpublictoiletmanagerwidget.h"

int main(int argc, char* argv[])
{
  QApplication a(argc, argv);
  QApplication::setApplicationName("PublicToiletManager");
  QApplication::setWindowIcon(QIcon(":/PTM5icon.ico"));
  QtPublicToiletManagerWidget w;
#ifdef Q_OS_WASM
  // In the browser, the game fills the web page, without a window frame
  w.setWindowFlags(Qt::FramelessWindowHint);
  w.move(0, 0);
#endif
  w.show();
  w.activateWindow();
  w.setFocus();
  return a.exec();
}
