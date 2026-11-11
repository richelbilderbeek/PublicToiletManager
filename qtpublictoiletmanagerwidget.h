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
// Qt port of the C++Builder/VCL version (PTM5.*).
// The game logic is kept as close to the original as possible,
// including the original (Dutch) variable names
#ifndef QTPUBLICTOILETMANAGERWIDGET_H
#define QTPUBLICTOILETMANAGERWIDGET_H

#include <QCursor>
#include <QHash>
#include <QImage>
#include <QLabel>
#include <QString>
#include <QTimer>
#include <QWidget>
#include <vector>

class QFrame;
class QLineEdit;
class QProgressBar;
class QSoundEffect;

/// The equivalent of a VCL TImage with AutoSize = true,
/// that can be clicked
class ImageButton : public QLabel
{
  Q_OBJECT
public:
  explicit ImageButton(QWidget* parent = nullptr);

  /// Show a picture from the resources, e.g. 'IconGo.bmp'.
  /// If transparent, the colour of the bottom-left pixel
  /// becomes transparent, as with a VCL TImage with Transparent = true
  void load(const QString& name, bool transparent);

signals:
  void clicked();
  void doubleClicked();

protected:
  void mouseReleaseEvent(QMouseEvent* event) override;
  void mouseDoubleClickEvent(QMouseEvent* event) override;
};

/// The equivalent of a VCL TImageList,
/// to which pictures are added with AddMasked(bitmap, clGreen)
class ImageList
{
public:
  void add(const QString& name);
  /// Draws nothing if the index is out of range
  void draw(QPainter& painter, int x, int y, int index) const;

private:
  std::vector<QImage> m_images;
};

class QtPublicToiletManagerWidget : public QWidget
{
  Q_OBJECT

public:
  explicit QtPublicToiletManagerWidget(QWidget* parent = nullptr);

protected:
  void keyPressEvent(QKeyEvent* event) override;
  void mouseMoveEvent(QMouseEvent* event) override;
  void mousePressEvent(QMouseEvent* event) override;
  void paintEvent(QPaintEvent* event) override;

private:
  // The size of the original form, used in the layout calculations
  static constexpr int form_width = 1006;
  static constexpr int form_height = 720;
  // The size of CanvasImage
  static constexpr int canvas_width = 1000;
  static constexpr int canvas_height = 700;

  int MX = 0; //voor bewegen van scherm
  int MY = 0; //Tekent hij op CanvasImage, plaatjes kunnen op de canvaslayer...

  //De sanitair-array:
  int SAN[32][6] = {};

  //De mensen-array:
  int HUM[40][9] = {};
  static constexpr int maxmensen = 40; //omvang array
  int rowman = 0;
  int rowwoman = 0;
  int welkgeslacht = 0;

  //Wat hij neerzet:
  int watbouwen = 0;
  int WCA = 0;
  int WCB = 0;
  int WCC = 0;
  int WCU = 0;
  int WCS = 0;
  int WCUR = 0;

  //Integer die bepaald welke cel aangeraaakt wordt
  int welkecel = -1;
  //en een die aangeeft wat aangeraakt wordt: plee, Sink, Urinoir...
  int watte = 0;

  int money = 5000;
  int VRD[6] = {}; //bevat voorraad, 2 posities per artikel, 2e voor hoeveelheid dozen
  int VRDP[12][2] = {}; //bevat info voor voorraadplaatjes
  int VRDS[4][2] = {};
  int VRDT[4][2] = {};

  //Array met happyness, alsmede teller voor eerste 20 klanten
  int klantteller = 0;
  int Happy[20] = {};
  int Rating = 0; //rating
  int Upgrade = 0;

  //clean: welk plaatje op statuspaneel, clean2 is om te voorkomen dat de boel onrustig wordt...
  int clean = 666;
  int clean2 = 0;

  int speed = 1;
  int dev = 0; //onbelangrijke developmentinteger
  int close_state = 0; //0 is gesloten, 1 is open, activeerd mensengenerator

  // Pictures
  QImage m_background;
  QImage m_front_layer;
  QImage m_fence;
  QImage m_clean_nes;
  QImage m_arrow;
  ImageList m_toilet_list;
  ImageList m_people_list;
  ImageList m_stock_list;
  ImageList m_clean_list;
  ImageList m_upgrade_list;
  ImageList m_happycon_list;
  ImageList m_dirt_list;

  /// CanvasLayer: the whole playing field, of which a part is shown
  QImage m_canvas_layer;
  /// CleanLayer: the picture on the left status panel
  QImage m_clean_layer;

  QCursor m_cursor_build;
  QCursor m_cursor_clean;

  QTimer m_central_timer;
  QTimer m_peoplemove_timer;

  // Controls
  QFrame* m_bouw_panel = nullptr;
  QFrame* m_stock_panel = nullptr;
  ImageButton* m_select_image = nullptr;
  ImageButton* m_speed_image = nullptr;
  ImageButton* m_close_button = nullptr;
  QLabel* m_clean_image = nullptr;
  QProgressBar* m_progress_bar_1 = nullptr;
  QProgressBar* m_progress_bar_2 = nullptr;
  ImageButton* m_clean_s_image = nullptr;
  ImageButton* m_paper_s_image = nullptr;
  ImageButton* m_soap_s_image = nullptr;
  ImageButton* m_towel_s_image = nullptr;
  QLineEdit* m_money_edit = nullptr;
  QLineEdit* m_paper_edit = nullptr;
  QLineEdit* m_soap_edit = nullptr;
  QLineEdit* m_towel_edit = nullptr;
  QLineEdit* m_rating_edit = nullptr;
  QLineEdit* m_klant_edit = nullptr;

  QHash<QString, QSoundEffect*> m_sounds;

  void createControls();
  void loadPictures();
  void initGame();

  /// Like the Win32 PlaySound with SND_ASYNC: stops the sound being played
  void playSound(const QString& name);

  /// For the red square around the chosen picture
  void welkegekozen(ImageButton* wat_image);
  void walkto(int tel, int DestX, int DestY);

  void onCentralTimer();
  void onPeoplemoveTimer();

  void onBouwButton(int wat, const QString& select_picture, ImageButton* wat_image);
  void onSloopImageClick(ImageButton* sloop_image);
  void onPaperImageClick();
  void onTowelImageClick();
  void onSoapImageClick();
  void onOrderBtnClick();
  void onBuildBtnClick();
  void onSpeedImageClick();
  void onSaveImageClick();
  void onLoadImageClick();
  void onCloseButtonClick();

  /// The file the game is saved to
  static QString saveFilename();
};

#endif // QTPUBLICTOILETMANAGERWIDGET_H
