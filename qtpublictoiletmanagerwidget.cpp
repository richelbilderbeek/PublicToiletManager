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
#include "qtpublictoiletmanagerwidget.h"

#include <QDir>
#include <QFile>
#include <QFrame>
#include <QKeyEvent>
#include <QLineEdit>
#include <QMessageBox>
#include <QMouseEvent>
#include <QPainter>
#include <QProgressBar>
#include <QRandomGenerator>
#include <QStandardPaths>
#include <QStringList>
#include <QTextStream>
#include <QUrl>

#ifdef Q_OS_WASM
// Qt Multimedia is not available for WebAssembly:
// the web version plays the sounds with the browser's Audio objects
// and saves the game in the browser's local storage
#include <emscripten/em_js.h>
#include <emscripten/val.h>

// In the web version, the keys only arrive if the window's focus helper
// element has the browser's focus. Qt does not always give it the focus
// (e.g. at startup, or after clicking on an already active window),
// so do so here, also after each click. This uses the web page structure of Qt 6.11
EM_JS(void, publictoiletmanager_focus_in_browser, (), {
  function focusGame() {
    var container = document.getElementById('qt-shadow-container');
    if (!container || !container.shadowRoot) return;
    var helper = container.shadowRoot.querySelector('.qt-window-focus-helper');
    if (helper) helper.focus();
  }
  if (!window.publictoiletmanagerFocusListener) {
    window.publictoiletmanagerFocusListener = true;
    document.addEventListener('click', focusGame);
  }
  focusGame();
});
#else
#include <QSoundEffect>
#endif

namespace {

/// Borland's random(n): a random number in [0, n>
int random(const int n)
{
  return n > 0 ? QRandomGenerator::global()->bounded(n) : 0;
}

QImage loadPicture(const QString& name)
{
  QImage image(":/images/" + name);
  Q_ASSERT(!image.isNull());
  return image.convertToFormat(QImage::Format_ARGB32);
}

/// Make all pixels of a colour transparent
QImage makeTransparent(QImage image, const QRgb colour)
{
  for (int y = 0; y < image.height(); ++y)
  {
    QRgb* line = reinterpret_cast<QRgb*>(image.scanLine(y));
    for (int x = 0; x < image.width(); ++x)
    {
      if ((line[x] & 0xFFFFFF) == (colour & 0xFFFFFF)) line[x] = 0;
    }
  }
  return image;
}

/// As a VCL TImage with Transparent = true:
/// the bottom-left pixel determines the transparent colour
QImage loadTransparentPicture(const QString& name)
{
  const QImage image = loadPicture(name);
  return makeTransparent(image, image.pixel(0, image.height() - 1));
}

/// A VCL TPanel
QFrame* createPanel(QWidget* parent, const QColor& colour)
{
  QFrame* panel = new QFrame(parent);
  panel->setFrameStyle(QFrame::Panel | QFrame::Raised);
  panel->setLineWidth(1);
  QPalette palette = panel->palette();
  palette.setColor(QPalette::Window, colour);
  panel->setPalette(palette);
  panel->setAutoFillBackground(true);
  panel->setCursor(Qt::ArrowCursor);
  return panel;
}

/// A VCL .cur file, which Qt reads without its hotspot
QCursor loadCursor(const QString& name)
{
  QFile file(":/cursors/" + name);
  if (!file.open(QIODevice::ReadOnly)) return QCursor(Qt::ArrowCursor);
  const QByteArray bytes = file.readAll();
  const QPixmap pixmap = QPixmap::fromImage(QImage::fromData(bytes, "CUR"));
  // The hotspot is in the first directory entry
  int hot_x = -1;
  int hot_y = -1;
  if (bytes.size() > 13)
  {
    hot_x = static_cast<unsigned char>(bytes[10]) | (static_cast<unsigned char>(bytes[11]) << 8);
    hot_y = static_cast<unsigned char>(bytes[12]) | (static_cast<unsigned char>(bytes[13]) << 8);
  }
  if (pixmap.isNull()) return QCursor(Qt::ArrowCursor);
  return QCursor(pixmap, hot_x, hot_y);
}

} // namespace

//---------------------------------------------------------------------------
ImageButton::ImageButton(QWidget* parent) : QLabel(parent)
{
}

void ImageButton::load(const QString& name, const bool transparent)
{
  const QImage image = transparent ? loadTransparentPicture(name) : loadPicture(name);
  setPixmap(QPixmap::fromImage(image));
  resize(image.size()); //AutoSize
}

void ImageButton::mouseReleaseEvent(QMouseEvent* event)
{
  if (rect().contains(event->position().toPoint())) emit clicked();
}

void ImageButton::mouseDoubleClickEvent(QMouseEvent*)
{
  emit doubleClicked();
}

//---------------------------------------------------------------------------
void ImageList::add(const QString& name)
{
  m_images.push_back(makeTransparent(loadPicture(name), qRgb(0, 128, 0))); //clGreen
}

void ImageList::draw(QPainter& painter, const int x, const int y, const int index) const
{
  if (index < 0 || index >= static_cast<int>(m_images.size())) return;
  painter.drawImage(x, y, m_images[index]);
}

//---------------------------------------------------------------------------
QtPublicToiletManagerWidget::QtPublicToiletManagerWidget(QWidget* parent)
  : QWidget(parent)
{
  setWindowTitle("Public Toilet Manager V");
  // The client area of the original 1006 x 720 form
  setFixedSize(998, 686);
  setMouseTracking(true);
  setFocusPolicy(Qt::StrongFocus);

  loadPictures();
  createControls();
  initGame();

#ifndef Q_OS_WASM
  for (const char* const name : { "flush_0.wav", "flush_1.wav", "flush_2.wav" })
  {
    QSoundEffect* const sound = new QSoundEffect(this);
    sound->setSource(QUrl("qrc:/sounds/" + QString(name)));
    m_sounds[name] = sound;
  }
#endif

  connect(&m_central_timer, &QTimer::timeout, this, &QtPublicToiletManagerWidget::onCentralTimer);
  connect(&m_peoplemove_timer, &QTimer::timeout, this, &QtPublicToiletManagerWidget::onPeoplemoveTimer);
  m_central_timer.start(50);
  m_peoplemove_timer.start(350);

  QTimer::singleShot(100, this, &QtPublicToiletManagerWidget::focusInBrowser);
}

//---------------------------------------------------------------------------
void QtPublicToiletManagerWidget::focusInBrowser()
{
#ifdef Q_OS_WASM
  publictoiletmanager_focus_in_browser();
#endif
}

//---------------------------------------------------------------------------
void QtPublicToiletManagerWidget::showMessage(const QString& text)
{
  // Not modal using exec, as that does not work in the web version
  QMessageBox* const box = new QMessageBox(QMessageBox::Information, windowTitle(), text, QMessageBox::Ok, this);
  box->setAttribute(Qt::WA_DeleteOnClose);
  box->open();
}

//---------------------------------------------------------------------------
void QtPublicToiletManagerWidget::loadPictures()
{
  //Cursors inladen
  m_cursor_build = loadCursor("build.cur");
  m_cursor_clean = loadCursor("clean.cur");
  setCursor(m_cursor_clean);

  //plaatjes inladen toiletten
  m_background = loadPicture("Background.bmp");
  m_front_layer = loadTransparentPicture("FrontLayer1.bmp");
  //in list
  m_toilet_list.add("WCLEEG.bmp"); //0
  WCA = 1;
  for (const char* const s : { "00", "01", "02M", "03M", "04M", "05M", "06M", "02K", "03K", "04K", "05K", "06K", "02W", "03W", "04W", "05W", "06W" })
  {
    m_toilet_list.add(QString("WCA") + s + ".bmp"); //1-17
  }
  WCB = 18;
  for (const char* const s : { "00", "01", "02M", "03M", "04M", "05M", "06M", "02K", "03K", "04K", "05K", "06K", "02W", "03W", "04W", "05W", "06W" })
  {
    m_toilet_list.add(QString("WCB") + s + ".bmp"); //18-34
  }
  WCC = 35;
  for (const char* const s : { "00", "01", "02M", "03M", "04M", "05M", "06M", "02K", "03K", "04K", "05K", "06K", "02W", "03W", "04W", "05W", "06W" })
  {
    m_toilet_list.add(QString("WCC") + s + ".bmp"); //35-51
  }
  WCU = 52;
  for (const char* const s : { "00", "01", "02M", "03M", "04M", "05M", "06M", "02K", "03K", "04K", "05K", "06K" })
  {
    m_toilet_list.add(QString("WCU") + s + ".bmp"); //52-63
  }
  WCUR = 64;
  for (const char* const s : { "10", "11", "12M", "13M", "14M", "15M", "16M", "12K", "13K", "14K", "15K", "16K" })
  {
    m_toilet_list.add(QString("WCU") + s + ".bmp"); //64-75
  }
  WCS = 76;
  for (const char* const s : { "10", "11", "12M", "13M", "14M", "15M", "16M", "12K", "13K", "14K", "15K", "16K", "12W", "13W", "14W", "15W", "16W" })
  {
    m_toilet_list.add(QString("WCS") + s + ".bmp"); //76-92
  }

  //plaatjes inladen mensen:
  for (const char* const s : {
    "MEMPTY",
    "MK1L", "MK1R", "MK2L", "MK2R", "MKWL", "MKWR", //kinderen 1-6
    "MM1L", "MM1R", "MM2L", "MM2R", "MMWL", "MMWR", //mannen 7-12
    "MW1L", "MW1R", "MW2L", "MW2R", "MWWL", "MWWR", //vrouwen 13-
    "MKWZ", "MMWZ", "MWWZ" //Zenuwachtigen 19-
  })
  {
    m_people_list.add(QString(s) + ".bmp");
  }

  //plaatjes inladen stock
  m_stock_list.add("BoxToiletPaper.bmp");
  m_stock_list.add("BoxSoap.bmp");
  m_stock_list.add("BoxTowel.bmp");

  //cleanlist inladen
  for (const char* const s : {
    "Logo",
    "CleanSimpel1", "CleanSimpel2", "CleanSimpel3", "CleanSimpel4",
    "CleanStandard1", "CleanStandard2", "CleanStandard3", "CleanStandard4",
    "CleanLuxe1", "CleanLuxe2", "CleanLuxe3", "CleanLuxe4",
    "CleanUrinal1", "CleanUrinal2", "CleanUrinal3", "CleanUrinal4", //16
    "CleanSink1", "Clean5", "CleanUrinal5", "CleanSink5"
  })
  {
    m_clean_list.add(QString(s) + ".bmp");
  }

  //Upgrades
  for (int i = 1; i <= 4; ++i) m_upgrade_list.add(QString("Upgrade%1.bmp").arg(i));

  //Happycons
  for (int i = 0; i <= 4; ++i) m_happycon_list.add(QString("HappyIcon%1.bmp").arg(i));

  //overig
  m_clean_nes = loadTransparentPicture("Clean.bmp");
  m_dirt_list.add("Dirt1.bmp");
  m_dirt_list.add("Dirt2.bmp");
  m_dirt_list.add("Dirt3.bmp");
  m_fence = loadTransparentPicture("Fence.bmp");

  //pijl inladen
  m_arrow = loadTransparentPicture("Arrow.bmp");

  m_clean_layer = QImage(167, 252, QImage::Format_ARGB32);
  m_clean_layer.fill(Qt::white);
}

//---------------------------------------------------------------------------
void QtPublicToiletManagerWidget::createControls()
{
  QFont bold_font = font();
  bold_font.setPixelSize(11);
  bold_font.setBold(true);

  //NamePanel
  QLabel* const name_label = new QLabel("Joost van den Bogaart, 2006", this);
  name_label->setGeometry(form_width - 245, form_height - 70, 217, 25);
  name_label->setFrameStyle(QFrame::Panel | QFrame::Raised);
  name_label->setAlignment(Qt::AlignCenter);
  name_label->setAutoFillBackground(true);
  {
    QPalette palette = name_label->palette();
    palette.setColor(QPalette::Window, Qt::white);
    name_label->setPalette(palette);
    QFont f = bold_font;
    f.setPixelSize(13);
    name_label->setFont(f);
  }
  name_label->setCursor(Qt::ArrowCursor);

  //statuspanel2
  QFrame* const status_panel_2 = createPanel(this, Qt::white);
  m_clean_image = new QLabel(status_panel_2);
  m_clean_image->setGeometry(4, 4, 167, 252);
  m_progress_bar_1 = new QProgressBar(status_panel_2);
  m_progress_bar_2 = new QProgressBar(status_panel_2);
  for (QProgressBar* const bar : { m_progress_bar_1, m_progress_bar_2 })
  {
    bar->setRange(0, 100);
    bar->setTextVisible(false);
    bar->setEnabled(false);
    bar->hide();
  }
  m_progress_bar_1->setGeometry(24, 252 + 8, 145, 18);
  m_progress_bar_2->setGeometry(24, 252 + 18 + 12, 145, 18);
  status_panel_2->setGeometry(8, 8, 167 + 8, 252 + 16 + (18 * 2));
  m_clean_s_image = new ImageButton(status_panel_2);
  m_clean_s_image->load("IconCleanextrasmall.bmp", true);
  m_clean_s_image->move(3, m_progress_bar_1->y());
  m_soap_s_image = new ImageButton(status_panel_2);
  m_soap_s_image->load("IconSoapextrasmall.bmp", true);
  m_soap_s_image->move(3, m_progress_bar_1->y());
  m_paper_s_image = new ImageButton(status_panel_2);
  m_paper_s_image->load("IconPaperextrasmall.bmp", true);
  m_paper_s_image->move(3, m_progress_bar_2->y());
  m_towel_s_image = new ImageButton(status_panel_2);
  m_towel_s_image->load("IconTowelsextrasmall.bmp", true);
  m_towel_s_image->move(3, m_progress_bar_2->y());
  for (ImageButton* const b : { m_clean_s_image, m_soap_s_image, m_paper_s_image, m_towel_s_image }) b->hide();

  //StatusPanel1
  QFrame* const status_panel_1 = createPanel(this, Qt::white);
  status_panel_1->setGeometry(status_panel_2->width() + 16, 8, 561, 41);
  const auto add_icon = [status_panel_1](const QString& name, const int x)
  {
    ImageButton* const b = new ImageButton(status_panel_1);
    b->load(name, true);
    b->move(x, 8);
    return b;
  };
  add_icon("IconMoneysmall.bmp", 8);
  ImageButton* const paper_icon_image = add_icon("IconPapersmall.bmp", 120);
  ImageButton* const towel_icon_image = add_icon("IconTowelssmall.bmp", 200);
  ImageButton* const soap_icon_image = add_icon("IconSoapsmall.bmp", 278);
  ImageButton* const people_icon_image = add_icon("IconPeoplesmall.bmp", 443);
  ImageButton* const happy_icon_image = add_icon("IconHappysmall.bmp", 358);
  const auto add_edit = [status_panel_1, bold_font](const QString& hint, const int x, const int width)
  {
    QLineEdit* const e = new QLineEdit("0", status_panel_1);
    e->setGeometry(x, 11, width, 21);
    e->setToolTip(hint);
    e->setEnabled(false);
    e->setFont(bold_font);
    return e;
  };
  m_money_edit = add_edit("Money", 36, 77);
  m_paper_edit = add_edit("Paper Stock", 149, 44);
  m_soap_edit = add_edit("Soap Stock", 308, 45);
  m_towel_edit = add_edit("Towel Stock", 229, 44);
  m_rating_edit = add_edit("Rating", 390, 49);
  m_klant_edit = add_edit("Number of Customers", 475, 78);

  // The cheat
  connect(people_icon_image, &ImageButton::clicked, this, [this]() { dev = 1; });
  connect(paper_icon_image, &ImageButton::doubleClicked, this, [this]() { if (dev == 1) dev = 2; });
  connect(soap_icon_image, &ImageButton::clicked, this, [this]() { if (dev == 2) dev = 3; });
  connect(towel_icon_image, &ImageButton::clicked, this, [this]() { if (dev == 3) dev = 4; });
  connect(happy_icon_image, &ImageButton::clicked, this, [this]() { if (dev == 4) money += 10000; });

  //mainpanel
  QFrame* const main_panel = createPanel(this, Qt::white);
  main_panel->setGeometry(8, form_height - 35 - 40, 209, 35);
  const auto add_button = [main_panel](const QString& name, const QString& hint, const int x, const int y)
  {
    ImageButton* const b = new ImageButton(main_panel);
    b->load(name, true);
    b->move(x, y);
    b->setToolTip(hint);
    return b;
  };
  ImageButton* const build_btn = add_button("IconBuildsmall.bmp", "Open Build Panel", 4, 5);
  ImageButton* const order_btn = add_button("IconOrdersmall.bmp", "Show Order Panel", 35, 5);
  m_speed_image = add_button("IconSteady.bmp", "Speed", 107, 5);
  ImageButton* const load_image = add_button("IconOpen.bmp", "Load", 148, 4);
  ImageButton* const save_image = add_button("IconSave.bmp", "Save", 177, 5);
  m_close_button = add_button("IconClose.bmp", "Open or close toilets", 75, 5);
  connect(build_btn, &ImageButton::clicked, this, &QtPublicToiletManagerWidget::onBuildBtnClick);
  connect(order_btn, &ImageButton::clicked, this, &QtPublicToiletManagerWidget::onOrderBtnClick);
  connect(m_speed_image, &ImageButton::clicked, this, &QtPublicToiletManagerWidget::onSpeedImageClick);
  connect(load_image, &ImageButton::clicked, this, &QtPublicToiletManagerWidget::onLoadImageClick);
  connect(save_image, &ImageButton::clicked, this, &QtPublicToiletManagerWidget::onSaveImageClick);
  connect(m_close_button, &ImageButton::clicked, this, &QtPublicToiletManagerWidget::onCloseButtonClick);

  const QColor navy(0, 0, 128); //clNavy

  //StockPanel
  m_stock_panel = createPanel(this, navy);
  m_stock_panel->setGeometry(8, form_height - 35 - 145 - 48, 369, 145);
  const auto add_stock = [this](const QString& name, const QString& hint, const int x)
  {
    ImageButton* const b = new ImageButton(m_stock_panel);
    b->load(name, false);
    b->move(x, 8);
    b->setToolTip(hint);
    return b;
  };
  ImageButton* const paper_image = add_stock("IconPaper.bmp", "Buy toilet paper", 8);
  ImageButton* const towel_image = add_stock("IconTowels.bmp", "Buy towels", 128);
  ImageButton* const soap_image = add_stock("IconSoap.bmp", "Buy soap", 248);
  connect(paper_image, &ImageButton::clicked, this, &QtPublicToiletManagerWidget::onPaperImageClick);
  connect(towel_image, &ImageButton::clicked, this, &QtPublicToiletManagerWidget::onTowelImageClick);
  connect(soap_image, &ImageButton::clicked, this, &QtPublicToiletManagerWidget::onSoapImageClick);
  m_stock_panel->hide();

  //Bouwpanel
  m_bouw_panel = createPanel(this, navy);
  m_bouw_panel->setGeometry(8, form_height - 35 - 321 - 48, 369, 321);
  const auto add_bouw = [this](const QString& name, const QString& hint, const int x, const int y)
  {
    ImageButton* const b = new ImageButton(m_bouw_panel);
    b->load(name, false);
    b->move(x, y);
    b->setToolTip(hint);
    return b;
  };
  ImageButton* const wca_image = add_bouw("ToiletBasic.bmp", "Build basic toilet", 8, 8);
  ImageButton* const sloop_image = add_bouw("ToiletDelete.bmp", "Delete", 248, 184);
  ImageButton* const wcb_image = add_bouw("ToiletStandard.bmp", "Build standard toilet", 128, 8);
  ImageButton* const wcc_image = add_bouw("ToiletLuxery.bmp", "Build luxury toilet", 248, 8);
  ImageButton* const wcu_image = add_bouw("ToiletUrinal.bmp", "Build urinal", 8, 184);
  ImageButton* const wcs_image = add_bouw("ToiletSink.bmp", "Build sink", 128, 184);
  // A disabled VCL control lets the clicks through
  m_select_image = new ImageButton(m_bouw_panel);
  m_select_image->setAttribute(Qt::WA_TransparentForMouseEvents);
  m_select_image->hide();
  connect(wca_image, &ImageButton::clicked, this, [this, wca_image]() { onBouwButton(WCA, "ToiletSelect.bmp", wca_image); });
  connect(wcb_image, &ImageButton::clicked, this, [this, wcb_image]() { onBouwButton(WCB, "ToiletSelect.bmp", wcb_image); });
  connect(wcc_image, &ImageButton::clicked, this, [this, wcc_image]() { onBouwButton(WCC, "ToiletSelect.bmp", wcc_image); });
  connect(wcu_image, &ImageButton::clicked, this, [this, wcu_image]() { onBouwButton(WCU, "ToiletSelect2.bmp", wcu_image); });
  connect(wcs_image, &ImageButton::clicked, this, [this, wcs_image]() { onBouwButton(WCS, "ToiletSelect2.bmp", wcs_image); });
  connect(sloop_image, &ImageButton::clicked, this, [this, sloop_image]() { onSloopImageClick(sloop_image); });
  m_bouw_panel->hide();
}

//---------------------------------------------------------------------------
void QtPublicToiletManagerWidget::initGame()
{
  //Sanitairarray vullen
  for (int tel = 0; tel < 32; tel++)
  {
    //0 = X:, 1 = Y;
    //tot 12 damestoiletten,
    if (tel < 8)
    {
      SAN[tel][0] = 314 + (tel * 38);
      SAN[tel][1] = 74 + (tel * 38);
    }
    else if (tel < 16) //verder herentoiletten
    {
      SAN[tel][0] = 314 + ((tel + 4) * 38);
      SAN[tel][1] = 74 + ((tel + 4) * 38);
    }
    else if (tel < 24) //dameswasbakken
    {
      SAN[tel][0] = 314 + ((tel - 16) * 38) - 103;
      SAN[tel][1] = 74 + ((tel - 16) * 38) + 103;
    }
    else //verder herendingen
    {
      SAN[tel][0] = 314 + ((tel + 4 - 16) * 38) - 103;
      SAN[tel][1] = 74 + ((tel + 4 - 16) * 38) + 103;
    }
    //2 = wat laten zien, begin is niks
    SAN[tel][2] = 0;
    //3 = welke persoon en hoe lang het duurt
    SAN[tel][3] = -1;
    //4 = schoonheid of zeep
    SAN[tel][4] = 100;
    //5 = papier of towels
    SAN[tel][5] = 100;
  }

  //mensenarray vullen
  for (int tel = 0; tel < maxmensen; tel++)
  {
    //0 = X; 1 = Y;
    HUM[tel][0] = -36;
    HUM[tel][1] = 1228;
    //2 = leeftijd, welk plaatje
    //3 = geslacht
    //4 = indexnummer: 0 is beschikbaar
    HUM[tel][4] = 0;
    //5 = plek in rij: leeg = 0;
    HUM[tel][5] = 0;
    //6 is welk toilet,
    HUM[tel][6] = -1;
    //7 is tevredenheid, bij begin 10, betaald tevredenheid "letterlijk" uit, max 100
    //8 bepaald Happycon
  }

  //VoorraadArray vullen:
  VRD[0] = 60; //papier
  VRD[2] = 20; //zeep
  VRD[4] = 20; //towels
  for (int vrd = 0; vrd < 4; vrd++)
  {
    VRDP[vrd][0] = 760; VRDP[vrd][1] = 376 - vrd * 19;
    VRDP[vrd + 4][0] = 784; VRDP[vrd + 4][1] = 400 - vrd * 19;
    VRDP[vrd + 8][0] = 808; VRDP[vrd + 8][1] = 424 - vrd * 19;
    VRDS[vrd][0] = 832; VRDS[vrd][1] = 448 - vrd * 19;
    VRDT[vrd][0] = 856; VRDT[vrd][1] = 472 - vrd * 19;
  }
}

//---------------------------------------------------------------------------
void QtPublicToiletManagerWidget::playSound(const QString& name)
{
#ifdef Q_OS_WASM
  // The .wav files are in the 'sounds' folder next to the web page
  using emscripten::val;
  static val playing_sound = val::null();
  if (!playing_sound.isNull()) playing_sound.call<void>("pause");
  playing_sound = val::global("Audio").new_(val(("sounds/" + name).toStdString()));
  playing_sound.call<val>("play");
#else
  for (QSoundEffect* const sound : std::as_const(m_sounds)) sound->stop();
  if (m_sounds.contains(name)) m_sounds[name]->play();
#endif
}

//---------------------------------------------------------------------------
void QtPublicToiletManagerWidget::welkegekozen(ImageButton* wat_image)
{
  if (watbouwen == 0)
  {
    m_select_image->hide();
  }
  else
  {
    m_select_image->move(wat_image->pos());
    m_select_image->show();
    m_select_image->raise();
  }
}

//---------------------------------------------------------------------------
void QtPublicToiletManagerWidget::walkto(const int tel, const int DestX, const int DestY)
{
  //bewegen
  if (HUM[tel][0] < DestX) HUM[tel][0] += 19;
  if (HUM[tel][0] > DestX) HUM[tel][0] -= 19;
  if (HUM[tel][1] < DestY) HUM[tel][1] += 19;
  if (HUM[tel][1] > DestY) HUM[tel][1] -= 19;
  //lopen
  switch (HUM[tel][2])
  {
    case 1: HUM[tel][2] = 3; break;
    case 2: HUM[tel][2] = 4; break;
    case 3: HUM[tel][2] = 1; break;
    case 4: HUM[tel][2] = 2; break;
    case 7: HUM[tel][2] = 9; break;
    case 8: HUM[tel][2] = 10; break;
    case 9: HUM[tel][2] = 7; break;
    case 10: HUM[tel][2] = 8; break;
    case 13: HUM[tel][2] = 15; break;
    case 14: HUM[tel][2] = 16; break;
    case 15: HUM[tel][2] = 13; break;
    case 16: HUM[tel][2] = 14; break;
  }
}

//---------------------------------------------------------------------------
void QtPublicToiletManagerWidget::onCentralTimer()
{
  //BOUWEN:
  //op gevoelige cel eventueel iets tijdelijks tekenen...
  //Maar eerst alle tijdelijke cellen op 0:
  if (watbouwen > 0)
  {
    for (int tel = 0; tel < 32; tel++)
    {
      if (SAN[tel][2] == -1 || SAN[tel][2] == WCA || SAN[tel][2] == WCB || SAN[tel][2] == WCC || SAN[tel][2] == WCU || SAN[tel][2] == WCS || SAN[tel][2] == WCUR)
      {
        SAN[tel][2] = 0;
      }
      //anders misschien iets laten zien
      else if (tel == welkecel && SAN[tel][2] == 0)
      {
        //links wasbakken, rechts toiletten, urinoir goed
        if (watbouwen == WCA || watbouwen == WCB || watbouwen == WCC)
        {
          if (tel < 16) SAN[tel][2] = watbouwen;
        }
        if (watbouwen == WCS)
        {
          if (tel > 15) SAN[tel][2] = watbouwen;
        }
        if (watbouwen == WCU)
        {
          if (tel > 7 && tel < 16) SAN[tel][2] = watbouwen;
          if (tel > 23) SAN[tel][2] = WCUR;
        }
      }
    }
  }

  //Voorraad, 2e positie array is altijd 1/10 van 1e
  for (int tel = 0; tel < 6; tel += 2)
  {
    VRD[tel + 1] = VRD[tel] / 10;
  }
  //en voorraad maximaliseren
  if (VRD[1] > 12) VRD[1] = 12;
  if (VRD[3] > 4) VRD[3] = 4;
  if (VRD[5] > 4) VRD[5] = 4;

  //statuspanel updaten
  m_money_edit->setText(QString::number(money));
  m_paper_edit->setText(QString::number(VRD[0]));
  m_soap_edit->setText(QString::number(VRD[2]));
  m_towel_edit->setText(QString::number(VRD[4]));

  //Buffer, tekenen
  m_canvas_layer = m_background;
  QPainter p(&m_canvas_layer);
  //sanitair 1 tot 16:
  for (int tel = 0; tel < 16; tel++)
  {
    if (SAN[tel][4] < 10) //dirt inladen?
    {
      m_dirt_list.draw(p, SAN[tel][0], SAN[tel][1], 0);
      if (SAN[tel][2] < WCU && SAN[tel][2] > WCA)
        m_dirt_list.draw(p, SAN[tel][0], SAN[tel][1], 2);
    }
    m_toilet_list.draw(p, SAN[tel][0], SAN[tel][1], SAN[tel][2]);
  }
  //mensen
  for (int tel = 0; tel < maxmensen; tel++)
  {
    m_people_list.draw(p, HUM[tel][0], HUM[tel][1], HUM[tel][2]);
  }

  //sanitair 17 tot 32:
  for (int tel = 16; tel < 32; tel++)
  {
    if (SAN[tel][4] < 10 && SAN[tel][2] > WCUR && SAN[tel][2] < WCS) //dirt inladen?
    {
      m_dirt_list.draw(p, SAN[tel][0], SAN[tel][1], 1);
    }
    m_toilet_list.draw(p, SAN[tel][0], SAN[tel][1], SAN[tel][2]);
  }

  //stock: toiletpapier, hoeveelheid dozen staat in oneven vrd
  for (int tel = 0; tel < VRD[1]; tel++)
  {
    m_stock_list.draw(p, VRDP[tel][0], VRDP[tel][1], 0);
  }
  //stock: soap
  for (int tel = 0; tel < VRD[3]; tel++)
  {
    m_stock_list.draw(p, VRDS[tel][0], VRDS[tel][1], 1);
  }
  //stock: towels
  for (int tel = 0; tel < VRD[5]; tel++)
  {
    m_stock_list.draw(p, VRDT[tel][0], VRDT[tel][1], 2);
  }

  //Eventueel hek, als gesloten en geen mensen aanwezig
  if (close_state == 0)
  {
    int controle = 0;
    for (int tel = 0; tel < maxmensen; tel++)
    {
      if (HUM[tel][4] > 0) controle = 1;
    }
    if (controle == 0)
    {
      p.drawImage(463, 671, m_fence);
    }
  }

  //Frontlayer
  p.drawImage(0, 0, m_front_layer);
  //Happycons
  for (int tel = 0; tel < maxmensen; tel++)
  {
    if (HUM[tel][2] != 0) //als mensen zichtbaar
      m_happycon_list.draw(p, HUM[tel][0] + 4, HUM[tel][1] - 14, HUM[tel][8]);
  }
  //laten zien als onderhoud nodig is...
  for (int tel = 0; tel < 32; tel++)
  {
    int A = 90;
    int B = -40;
    if (tel > 15)
    {
      A = 15;
      B = 20;
    }
    if (SAN[tel][4] < 10 || (SAN[tel][5] < 10 && SAN[tel][2] < WCU) || (SAN[tel][5] < 10 && SAN[tel][2] > WCS))
      p.drawImage(SAN[tel][0] + A, SAN[tel][1] + B, m_clean_nes);
  }

  //Arrow
  if (welkecel < 16 && welkecel > -1)
  {
    p.drawImage(SAN[welkecel][0] + 70, SAN[welkecel][1], m_arrow);
  }
  else if (welkecel > 15)
  {
    p.drawImage(SAN[welkecel][0] + 11, SAN[welkecel][1] + 52, m_arrow);
  }

  //Upgrades:
  m_upgrade_list.draw(p, -4, 566, Upgrade);
  p.end();

  //Op canvas
  update();
}

//---------------------------------------------------------------------------
void QtPublicToiletManagerWidget::paintEvent(QPaintEvent*)
{
  QPainter p(this);
  p.setClipRect(0, 0, canvas_width, canvas_height);
  p.drawImage(MX, MY, m_canvas_layer);
}

//---------------------------------------------------------------------------
void QtPublicToiletManagerWidget::keyPressEvent(QKeyEvent* event)
{
  //bewegen van veld, Beinvloed waar hij de Canvasimage tekent.
  switch (event->key())
  {
    case Qt::Key_Down:
      if (MY > canvas_height - m_background.height()) MY -= 26;
      break;
    case Qt::Key_Up:
      if (MY < 0) MY += 26;
      break;
    case Qt::Key_Right:
      if (MX > canvas_width - m_background.width()) MX -= 26;
      break;
    case Qt::Key_Left:
      if (MX < 0) MX += 26;
      break;
    default:
      QWidget::keyPressEvent(event);
  }
}

//---------------------------------------------------------------------------
void QtPublicToiletManagerWidget::mouseMoveEvent(QMouseEvent* event)
{
  const int X = event->position().toPoint().x();
  const int Y = event->position().toPoint().y();

  //om te voorkomen dat cleanplaatje flikkert, niet bij begin!
  clean2 = clean;
  if (clean == 666) clean = 0;

  //alle toiletten langsgaan, anders alle wasbakken
  for (int tel = 0; tel < 33; tel++)
  {
    //gevoelig veld, tellen tot 33 omdat misschien niks actief
    //op WC-deurjes
    if ((tel < 16 && X - MX > SAN[tel][0] + 43 && X - MX < SAN[tel][0] + 76 && Y - MY > SAN[tel][1] + 52 && Y - MY < SAN[tel][1] + 149)
      || (tel < 16 && X - MX > SAN[tel][0] + 78 && X - MX < SAN[tel][0] + 111 && Y - MY > SAN[tel][1] + 1 && Y - MY < SAN[tel][1] + 85))
    {
      welkecel = tel;
      tel = 33;
    }
    else if (tel < 32 && X - MX > SAN[tel][0] + 9 && X - MX < SAN[tel][0] + 38 && Y - MY > SAN[tel][1] + 94 && Y - MY < SAN[tel][1] + 195) //anders wasbakachterwanden
    {
      welkecel = tel;
      tel = 33;
    }
    else if (tel == 32) //niks actief
    {
      welkecel = -1;
    }
  }

  const auto show_status = [this](const bool clean_s, const bool paper_s, const bool soap_s, const bool towel_s)
  {
    m_clean_s_image->setVisible(clean_s);
    m_paper_s_image->setVisible(paper_s);
    m_soap_s_image->setVisible(soap_s);
    m_towel_s_image->setVisible(towel_s);
  };

  //statuspanel 2 updaten, als cel actief
  //en WC vrij, slaat wat op in watte
  if (welkecel > -1) //toiletten
  {
    const int s = SAN[welkecel][2];
    if (s == WCA + 1 || s == WCB + 1 || s == WCC + 1 || s == WCS + 1)
    {
      m_progress_bar_1->setValue(qBound(0, SAN[welkecel][4], 100));
      m_progress_bar_2->setValue(qBound(0, SAN[welkecel][5], 100));
      m_progress_bar_1->show();
      m_progress_bar_2->show();
      watte = 1; //wc
      //plaatjes
      if (s == WCS + 1)
      {
        watte = 2; //sink
        clean = 17; //logo laten zien
        show_status(false, false, true, true);
      }
      if (s == WCA + 1)
      {
        clean = 1; //wc1 laten zien
        show_status(true, true, false, false);
      }
      if (s == WCB + 1)
      {
        clean = 5; //wc2 laten zien
        show_status(true, true, false, false);
      }
      if (s == WCC + 1)
      {
        clean = 9; //wc3 laten zien
        show_status(true, true, false, false);
      }
    }
    else if (s == WCU + 1 || s == WCUR + 1)
    {
      m_progress_bar_1->setValue(qBound(0, SAN[welkecel][4], 100));
      m_progress_bar_1->show();
      m_progress_bar_2->hide();
      watte = 3; //urinoir
      clean = 13;
      show_status(true, false, false, false);
    }
    //bezette WC?
    else if ((s > WCA && s < WCB) || (s > WCB && s < WCC) || (s > WCC && s < WCU))
    {
      m_progress_bar_1->hide();
      m_progress_bar_2->hide();
      watte = 2;
      clean = 18;
      show_status(false, false, false, false);
    }
    //bezet urinoir?
    else if (s > WCU && s < WCS && s != WCUR)
    {
      m_progress_bar_1->hide();
      m_progress_bar_2->hide();
      watte = 2;
      clean = 19;
      show_status(false, false, false, false);
    }
    //bezette sink?
    else if (s > WCS)
    {
      m_progress_bar_1->hide();
      m_progress_bar_2->hide();
      watte = 2;
      clean = 20;
      show_status(false, false, false, false);
    }
  }
  else //geen cel actief
  {
    m_progress_bar_1->hide();
    m_progress_bar_2->hide();
    watte = 0;
    clean = 0;
    show_status(false, false, false, false);
  }

  //Bij clean wat optellen als toilet vies, geen sink, wat geselecteerd
  if (clean > 0 && clean < 17 && welkecel > -1 && watte != 2)
  {
    if (SAN[welkecel][4] < 26) clean++;
    if (SAN[welkecel][4] < 51) clean++;
    if (SAN[welkecel][4] < 76) clean++;
  }

  //Updaten CleanImage
  if (clean != clean2)
  {
    QPainter p(&m_clean_layer);
    m_clean_list.draw(p, 0, 0, clean);
    p.end();
    m_clean_image->setPixmap(QPixmap::fromImage(m_clean_layer));
  }

  //bewegen scherm?
  if (Y > form_height - 100)
  {
    if (MY > canvas_height - m_background.height()) MY -= 26;
  }
  if (Y < 100)
  {
    if (MY < 0) MY += 26;
  }
  if (X > form_width - 100)
  {
    if (MX > canvas_width - m_background.width()) MX -= 26;
  }
  if (X < 100)
  {
    if (MX < 0) MX += 26;
  }
}

//---------------------------------------------------------------------------
void QtPublicToiletManagerWidget::mousePressEvent(QMouseEvent*)
{
  //Alleen als bouwen uit staat en cel actief: onderhoud
  if (watbouwen == 0 && welkecel > -1)
  {
    switch (watte)
    {
      case 1: //wc schoonmaak en papier als nodig
        SAN[welkecel][4] = 100; //schoonmaak
        //evt papier als nodig en op voorraad:
        if (SAN[welkecel][5] < 41 && VRD[0] > 0)
        {
          SAN[welkecel][5] = 100;
          VRD[0] -= 1; //papier op maken
        }
        money -= 10;
        break;
      case 2: //wasbak bijvullen als nodig
        //evt zeep als op voorraad:
        if (VRD[2] > 0)
        {
          SAN[welkecel][4] = 100;
          VRD[2] -= 1; //zeep op maken
        }
        //evt towels als op voorraad:
        if (VRD[4] > 0)
        {
          SAN[welkecel][5] = 100;
          VRD[4] -= 1; //towels op maken
        }
        money -= 5;
        break;
      case 3: //urinoir schoonmaak
        SAN[welkecel][4] = 100; //schoonmaak
        money -= 5;
        break;
    }
  }

  //BOUWEN
  //Bij Muisclick moet hij -misschien- iets tijdelijks iets echts maken:
  if (watbouwen > 0) //bouwbutton ingedrukt
  {
    if (welkecel > -1) //cel geselecteerd
    {
      if (SAN[welkecel][2] == 0 || SAN[welkecel][2] == watbouwen || SAN[welkecel][2] == WCUR) //iets tijdelijks
      {
        //Toiletten rechts en geld:
        if ((watbouwen == WCA && money > 799) || (watbouwen == WCB && money > 1999) || (watbouwen == WCC && money > 3999))
        {
          if (welkecel < 16)
          {
            SAN[welkecel][2] = watbouwen + 1;
            if (watbouwen == WCA) money -= 800;
            if (watbouwen == WCB) money -= 2000;
            if (watbouwen == WCC) money -= 4000;
            //papier en schoonheid
            SAN[welkecel][4] = 100;
            SAN[welkecel][5] = 100;
          }
        }
        //wastafels links
        if (watbouwen == WCS && money > 499)
        {
          if (welkecel > 15)
          {
            SAN[welkecel][2] = watbouwen + 1;
            money -= 500;
            //towels en zeep
            SAN[welkecel][4] = 100;
            SAN[welkecel][5] = 100;
          }
        }
        //Urinoirs bij heren:
        if (watbouwen == WCU && money > 499)
        {
          if (welkecel > 7 && welkecel < 16)
          {
            SAN[welkecel][2] = watbouwen + 1;
          }
          else if (welkecel > 23)
          {
            SAN[welkecel][2] = WCUR + 1;
          }
          money -= 500;
          //schoonheid, geen papier
          SAN[welkecel][4] = 100;
          SAN[welkecel][5] = 9999;
        }
      }
    }
  }
  //SLOPEN
  if (watbouwen == -1) //sloopbutton ingedrukt
  {
    if (welkecel > -1) //cel geselecteerd
    {
      const int s = SAN[welkecel][2];
      if (s == WCA + 1 || s == WCB + 1 || s == WCC + 1 || s == WCU + 1 || s == WCUR + 1 || s == WCS + 1) //iets onbezets
      {
        SAN[welkecel][2] = 0; //slopen
        SAN[welkecel][3] = -1; //cel vrijhouden
        //mens dat gekozen had naar huis:
        for (int tel = 0; tel < maxmensen; tel++)
        {
          if (HUM[tel][6] == welkecel)
          {
            //even staan
            if (HUM[tel][2] < 7) HUM[tel][2] = 5; //kind
            else if (HUM[tel][2] < 13) HUM[tel][2] = 11; //man
            else HUM[tel][2] = 17; //vrouw
            //naar huis
            HUM[tel][4] = 99;
          }
        }
        //hier hoeveel het oplevert
        money += 100;
        SAN[welkecel][4] = 100;
        SAN[welkecel][5] = 100;
      }
    }
  }
}

//---------------------------------------------------------------------------
void QtPublicToiletManagerWidget::onBouwButton(const int wat, const QString& select_picture, ImageButton* wat_image)
{
  if (watbouwen != wat) watbouwen = wat;
  else watbouwen = 0; //anders bouwen uitzetten

  //voor rood vakje om gekozen plaatje
  m_select_image->load(select_picture, true);
  welkegekozen(wat_image);
  //voorkomen dat spel hangt
  welkecel = -1;
}

void QtPublicToiletManagerWidget::onSloopImageClick(ImageButton* sloop_image)
{
  //Slopen
  watbouwen = -1;
  //voor rood vakje om gekozen plaatje
  m_select_image->load("ToiletSelect2.bmp", true);
  welkegekozen(sloop_image);
  //voorkomen dat spel hangt
  welkecel = -1;
}

void QtPublicToiletManagerWidget::onPaperImageClick()
{
  //papier inkopen
  if (VRD[0] < 111)
  {
    VRD[0] += 10;
    money -= 100;
  }
  welkecel = -1;
}

void QtPublicToiletManagerWidget::onTowelImageClick()
{
  //towels inkopen
  if (VRD[4] < 31)
  {
    VRD[4] += 10;
    money -= 200;
  }
  welkecel = -1;
}

void QtPublicToiletManagerWidget::onSoapImageClick()
{
  //soap inkopen
  if (VRD[2] < 31)
  {
    VRD[2] += 10;
    money -= 200;
  }
  welkecel = -1;
}

void QtPublicToiletManagerWidget::onOrderBtnClick()
{
  //OrderPaneel openen
  if (!m_stock_panel->isVisible())
  {
    m_bouw_panel->hide();
    m_stock_panel->show();
    watbouwen = 0;
    setCursor(m_cursor_clean);
  }
  else
  {
    m_stock_panel->hide();
  }
  welkecel = -1;
}

void QtPublicToiletManagerWidget::onBuildBtnClick()
{
  //BouwPaneel openen
  if (!m_bouw_panel->isVisible())
  {
    m_bouw_panel->show();
    m_stock_panel->hide();
    watbouwen = 0;
    setCursor(m_cursor_build);
  }
  else
  {
    m_bouw_panel->hide();
    watbouwen = 0;
    setCursor(m_cursor_clean);
  }
  welkecel = -1;
}

//---------------------------------------------------------------------------
void QtPublicToiletManagerWidget::onPeoplemoveTimer()
{
  //mensen genereren. Bepaald geslacht en leeftijd, geeft dan indexnummer 1;
  //randomizator, afhankelijk reputatie, alleen als open:
  if (close_state == 1 && random(2000) < (Rating * 4 + 100))
  {
    //mensenarray doorlopen, eerste beschikbare positie wordt het:
    for (int tel = 0; tel < maxmensen; tel++)
    {
      //als indexnummer 0 is:
      if (HUM[tel][4] == 0)
      {
        //gelijke hoeveelheid mannen en vrouwen uitbraken
        switch (welkgeslacht)
        {
          case 0: //vrouw
            HUM[tel][3] = welkgeslacht; //3 bevat geslacht
            HUM[tel][2] = 13; //2 bevat welk plaatje
            //rij
            HUM[tel][5] = rowwoman;
            rowwoman++;
            welkgeslacht = 1;
            break;
          case 1: //man
            HUM[tel][3] = welkgeslacht; //3 bevat geslacht
            HUM[tel][2] = 7; //2 bevat welk plaatje
            //rij
            HUM[tel][5] = rowman;
            rowman++;
            welkgeslacht = 0;
            break;
        }
        //als kind:
        if (random(2) == 1)
        {
          HUM[tel][2] = 1; //2 bevat welk plaatje
        }
        //in index
        HUM[tel][4] = 1;
        //tevredenheid begin
        HUM[tel][7] = 10;
        //uit forloop
        tel = maxmensen + 1;
      }
    }
  }

  //loopgrid. Voor mannen en vrouwen hetzelfde, maar mannen staan in andere rij,
  //en gebruiken 6-11 en >18;
  for (int tel = 0; tel < maxmensen; tel++)
  {
    switch (HUM[tel][4])
    {
      case 0: //niet tekenen
        HUM[tel][2] = 0;
        HUM[tel][0] = -36;
        HUM[tel][1] = 1228;
        break;
      case 1: //naar een positie uit beeld...
        HUM[tel][0] = -36 + HUM[tel][3] * 38; //3 is voor mannen, die staan 1 positie schijnonderder
        HUM[tel][1] = 1228 + HUM[tel][3] * 38;
        HUM[tel][4] = 2; //1 op bij index
        break;
      case 2:
      {
        //bestemming bepalen, in rij gaan staan:
        const int DestX = 648 + HUM[tel][3] * 38 - HUM[tel][5] * 19;
        const int DestY = 544 + HUM[tel][3] * 38 + HUM[tel][5] * 19;
        walkto(tel, DestX, DestY);
        //als je er bent index doorschruiven;
        if (HUM[tel][1] < DestY + 1)
        {
          HUM[tel][4] = 3; //1 op bij index
        }
        break;
      }
      case 3:
      {
        //gewoon in rij staan, dus sta-plaatje
        switch (HUM[tel][2])
        {
          case 1: case 3: HUM[tel][2] = 5; break;
          case 7: case 9: HUM[tel][2] = 11; break;
          case 13: case 15: HUM[tel][2] = 17; break;
          case 19: HUM[tel][2] = 5; break;
          case 20: HUM[tel][2] = 11; break;
          case 21: HUM[tel][2] = 17; break;
        }
        //zenuwachtig? laten zien + tevredenheid
        if (random(20) == 0)
        {
          if (HUM[tel][7] > 0) HUM[tel][7]--;
          switch (HUM[tel][2])
          {
            case 5: HUM[tel][2] = 19; break;
            case 11: HUM[tel][2] = 20; break;
            case 17: HUM[tel][2] = 21; break;
          }
        }
        else //deze else dient om te voorkomen dat geslacht verspringt
        if (HUM[tel][5] == 0) //als vooraan in rij
          HUM[tel][4] = 4; //1 op bij index
        break;
      }
      case 4:
      {
        //eerst vrouwen
        if (HUM[tel][3] == 0)
        {
          //alle vrouwentoiletten af
          for (int count = 0; count < 20; count++)
          {
            const int welke = random(8); //toilet kiezen
            //kijken of het er een is
            if (SAN[welke][2] == WCA + 1 || SAN[welke][2] == WCB + 1 || SAN[welke][2] == WCC + 1)
            {
              //dat vrij is
              if (SAN[welke][3] == -1)
              {
                //in HUM opslaan welk toilet(6)
                HUM[tel][6] = welke;
                //in SAN opslaan welke persoon
                SAN[welke][3] = tel;
                HUM[tel][4] = 5; //index
                //wisselen van hoe je loopt
                HUM[tel][2] -= 1;
                //rij door laten lopen
                for (int tel2 = 0; tel2 < maxmensen; tel2++)
                {
                  if (HUM[tel2][3] == 0 && tel2 != tel && HUM[tel2][6] < 0 && HUM[tel2][4] > 0)
                  {
                    HUM[tel2][5]--; //rij 1 opschuiven
                    if (HUM[tel2][5] < 0) HUM[tel2][5] = 0;
                    HUM[tel2][4] = 2; //index terug
                    if (HUM[tel2][2] == 5 || HUM[tel2][2] == 11 || HUM[tel2][2] == 17) HUM[tel2][2] -= 2;
                  }
                }
                rowwoman--; if (rowwoman < 0) rowwoman = 0;
                //uit count
                count = 20;
              }
            }
          }
        } //einde vrouwen
        //dan mannen
        if (HUM[tel][3] == 1)
        {
          //alle mannentoiletten af
          for (int count = 0; count < 20; count++)
          {
            const int welke = random(24) + 8; //toilet kiezen
            //kijken of het er een is
            if (SAN[welke][2] == WCA + 1 || SAN[welke][2] == WCB + 1 || SAN[welke][2] == WCC + 1 || SAN[welke][2] == WCU + 1 || SAN[welke][2] == WCUR + 1)
            {
              //dat vrij is
              if (SAN[welke][3] == -1)
              {
                //in HUM opslaan welk toilet(6)
                HUM[tel][6] = welke;
                //in SAN opslaan dat bezet
                SAN[welke][3] = 0;
                HUM[tel][4] = 5; //index
                //wisselen van hoe je loopt
                HUM[tel][2] -= 1;
                //rij door laten lopen
                for (int tel2 = 0; tel2 < maxmensen; tel2++)
                {
                  //man? niet betr persoon? //niet bezig? //wel actief?
                  if (HUM[tel2][3] == 1 && tel2 != tel && HUM[tel2][6] < 0 && HUM[tel2][4] > 0)
                  {
                    HUM[tel2][5]--; //rij 1 opschuiven
                    if (HUM[tel2][5] < 0) HUM[tel2][5] = 0;
                    HUM[tel2][4] = 2; //index terug
                  }
                }
                rowman--; if (rowman < 0) rowman = 0;
                //uit count
                count = 20;
              }
            }
          }
        } //einde mannen
        break;
      }
      case 5:
      case 21:
      {
        //5: bepalen en naar goede plek lopen
        //21: Naar wasbak toelopen
        int welke = HUM[tel][6];
        //voor urinoirs/wasbakken:
        if (welke > 15) welke -= 16;
        int DestX = 0;
        int DestY = 0;
        if (welke < 8)
        {
          DestX = 306 + welke * 38;
          DestY = 220 + welke * 38;
        }
        if (welke > 7 && welke < 16)
        {
          DestX = 458 + welke * 38;
          DestY = 372 + welke * 38;
        }
        walkto(tel, DestX, DestY);
        //als je er bent 1 verzetten
        const int margin = HUM[tel][4] == 5 ? 9 : 15;
        if (HUM[tel][0] - DestX < margin && HUM[tel][0] - DestX > -margin) HUM[tel][4]++;
        break;
      }
      case 6:
      case 22:
      {
        //toilet openen, of naar wasbak toegaan
        const int welke = HUM[tel][6];
        if (HUM[tel][2] < 7) SAN[welke][2] += 6; //kind
        else if (HUM[tel][2] < 13) SAN[welke][2] += 1; //man
        else SAN[welke][2] += 11; //vrouw
        HUM[tel][2] = 0; //verdwijnen
        HUM[tel][4]++; //naar case 7 of 23;
        break;
      }
      case 7:
      case 23:
      case 24:
      {
        //toilet inlopen, naar wasbak toegaan, wassen1
        const int welke = HUM[tel][6];
        SAN[welke][2]++;
        HUM[tel][4]++;
        break;
      }
      case 8:
      {
        //toilet verder inlopen
        const int welke = HUM[tel][6];
        SAN[welke][2]++; //04
        //tevredenheid type plee
        if (SAN[welke][2] == WCC + 4 || SAN[welke][2] == WCC + 9 || SAN[welke][2] == WCC + 14) HUM[tel][7] += 30;
        if (SAN[welke][2] == WCB + 4 || SAN[welke][2] == WCB + 9 || SAN[welke][2] == WCB + 14) HUM[tel][7] += 15;
        HUM[tel][4]++; //naar case 9;
        break;
      }
      case 9:
      {
        //Pee or Poo?
        int Wat = 1; //1=pee, 0=poo
        if (random(3) == 2) Wat = 0; //meer kans pee

        const int welke = HUM[tel][6];
        SAN[welke][2] += Wat + 1; //datgene doen
        //schoonmaak [4] en tevredenheid
        HUM[tel][7] += SAN[welke][4] - 45; //tevredenheid schoonmaak
        SAN[welke][4] -= random(15) + 5; //vies worden
        if (SAN[welke][4] < 0) SAN[welke][4] = 0;
        //papier [5] en tevredenheid
        if (SAN[welke][5] != 9999) SAN[welke][5] -= (random(10) + 12 - (Wat * 12)); //op maken
        if (SAN[welke][5] < 0)
        {
          // The original has 'HUM[tel][7]-50;', which has no effect
          SAN[welke][5] = 0;
        }
        HUM[tel][4] += Wat + 1; //naar case 10(Poo) of 11(Pee);
        break;
      }
      case 10:
      {
        const int welke = HUM[tel][6];
        //Poo, duurt lang tenzij urinoir
        //Slaat hij op in San[welke][3]:
        if (SAN[welke][2] < WCU + 1) SAN[welke][3] = random(20) + 20;
        else SAN[welke][3] = random(10) + 10;
        HUM[tel][4] = 12;
        break;
      }
      case 11:
      {
        //Pee, duurt korter...
        //Slaat hij op in San[welke][3]:
        const int welke = HUM[tel][6];
        SAN[welke][3] += random(10) + 10;
        HUM[tel][4] = 13;
        break;
      }
      case 12:
      {
        //Afmaken Poo
        const int welke = HUM[tel][6];
        SAN[welke][3]--;
        if (SAN[welke][3] == 0) //klaar?
        {
          HUM[tel][4] = 14;
          SAN[welke][2] -= 1;
        }
        break;
      }
      case 13:
      {
        //Afmaken Pee
        const int welke = HUM[tel][6];
        SAN[welke][3]--;
        if (SAN[welke][3] == 0) //klaar?
        {
          HUM[tel][4] = 14;
          SAN[welke][2] -= 2;
        }
        break;
      }
      case 14:
      {
        //geluid afspelen bij toiletten:
        HUM[tel][4] = 16;
        const int welke = HUM[tel][6];
        if (SAN[welke][2] > WCA && SAN[welke][2] < WCB) playSound("flush_0.wav");
        else if (SAN[welke][2] > WCB && SAN[welke][2] < WCC) playSound("flush_1.wav");
        else if (SAN[welke][2] > WCC && SAN[welke][2] < WCU) playSound("flush_2.wav");
        break;
      }
      case 16:
      case 17:
      {
        //uitlopen
        const int welke = HUM[tel][6];
        SAN[welke][2]--;
        HUM[tel][4]++;
        break;
      }
      case 18:
      {
        const int welke = HUM[tel][6];
        const int s = SAN[welke][2];
        //mens zichtbaar maken
        if (s == WCA + 2 || s == WCB + 2 || s == WCC + 2 || s == WCU + 2 || s == WCUR + 2)
        { //man
          HUM[tel][2] = 11;
        }
        else if (s == WCA + 7 || s == WCB + 7 || s == WCC + 7 || s == WCU + 7 || s == WCUR + 7)
        { //kind
          HUM[tel][2] = 5;
        }
        else if (s == WCA + 12 || s == WCB + 12 || s == WCC + 12)
        { //vrouw
          HUM[tel][2] = 17;
        }
        //toilet sluiten
        if (s < WCB) SAN[welke][2] = WCA + 1; //WCA
        else if (s < WCC) SAN[welke][2] = WCB + 1; //WCB
        else if (s < WCU) SAN[welke][2] = WCC + 1; //WCC
        else if (s < WCUR) SAN[welke][2] = WCU + 1; //WCU
        else SAN[welke][2] = WCUR + 1; //WCUR

        SAN[welke][3] = -1; //vrijgeven toilet
        HUM[tel][4]++;
        break;
      }
      case 19:
      {
        //Altijd weg tenzij wasbak
        HUM[tel][4] = 98;
        //kans op basis geslacht en leeftijd:
        //kinderen 1op4, vrouwen 2op3 en mannen 1op3
        switch (HUM[tel][2])
        {
          case 5: //kinderen
            if (random(4) == 0) HUM[tel][4] = 20;
            break;
          case 11: //mannen
            if (random(3) == 0) HUM[tel][4] = 20;
            break;
          case 17: //Vrouwen
            if (random(3) < 2) HUM[tel][4] = 20;
            break;
        }
        break;
      }
      case 20:
      {
        //wassen: kijken of wasbak beschikbaar
        for (int count = 0; count < 30; count++)
        {
          //als count 29 is, dan is er geen gevonden: naar huis
          if (count > 28) HUM[tel][4] = 98;
          const int welke = random(8) + 16 + HUM[tel][3] * 8; //wasbak kiezen, hum[tel][3] voor mannen
          //kijken of het er een is, dat vrij is
          if (SAN[welke][2] == WCS + 1 && SAN[welke][3] == -1)
          {
            //in HUM opslaan welk toilet(6)
            HUM[tel][6] = welke;
            //in SAN opslaan welke persoon
            SAN[welke][3] = tel;
            HUM[tel][4]++; //index
            //uit for-loop
            count = 30;
            //draaien
            HUM[tel][2] -= 3;
          }
        }
        break;
      }
      case 25:
      {
        HUM[tel][4]++; //naar case 26;
        //tevredenheid voor wasbak
        const int welke = HUM[tel][6];
        HUM[tel][7] += 25;
        //soap [4] en tevredenheid
        SAN[welke][4] -= (random(10) + 5); //op maken
        if (SAN[welke][4] < 0)
        {
          // The original has 'HUM[tel][7]-10;', which has no effect
          SAN[welke][4] = 0;
        }
        //towels [5] en tevredenheid
        SAN[welke][5] -= (random(10) + 8); //op maken
        if (SAN[welke][5] < 0)
        {
          // The original has 'HUM[tel][7]-10;', which has no effect
          SAN[welke][5] = 0;
        }
        break;
      }
      case 26:
      case 28:
        HUM[tel][4]++;
        break;
      case 27:
      case 29:
      {
        //wassen2, wassen3
        const int welke = HUM[tel][6];
        SAN[welke][2]++;
        HUM[tel][4]++;
        break;
      }
      case 30:
      case 31:
      case 32:
        if (random(3) == 0) HUM[tel][4]++; //naar case 31, 32, 33;
        break;
      case 33:
      {
        //weggaan
        const int welke = HUM[tel][6];
        SAN[welke][2] -= 3;
        HUM[tel][4]++; //naar case 34;
        break;
      }
      case 34:
      {
        //weggaan
        const int welke = HUM[tel][6];
        SAN[welke][2]--;
        HUM[tel][4] = 40; //naar case 40;
        break;
      }
      case 40:
      {
        const int welke = HUM[tel][6];
        //mens zichtbaar maken
        if (SAN[welke][2] == WCS + 2) HUM[tel][2] = 11; //man
        else if (SAN[welke][2] == WCS + 7) HUM[tel][2] = 5; //kind
        else if (SAN[welke][2] == WCS + 12) HUM[tel][2] = 17; //vrouw
        //wasbak normaal sluiten
        SAN[welke][2] = WCS + 1;
        SAN[welke][3] = -1; //vrijgeven wasbak
        HUM[tel][4] = 98;
        break;
      }
      case 98:
      {
        //betalen
        if (HUM[tel][7] > 5)
        {
          if (HUM[tel][7] < 101) money += HUM[tel][7];
          else money += 120;
        }
        else money += 5;

        HUM[tel][4]++;
        klantteller++; //klantteller 1 opvoeren
        //happyness in grid, rating berekenen:
        if (klantteller < 20)
        {
          Happy[klantteller] = HUM[tel][7];
        }
        else
        {
          for (int i = 0; i < 19; i++) Happy[i] = Happy[i + 1];
          Happy[19] = HUM[tel][7];
        }
        //rating berekenen
        Rating = 0;
        for (int i = 0; i < 20; i++) Rating += Happy[i];
        Rating = Rating / 20;
        m_rating_edit->setText(QString::number(Rating));
        m_klant_edit->setText(QString::number(klantteller));
        //upgrade bepalen
        if (Rating > 60 && Upgrade == 0) Upgrade = 1;
        else if (Rating > 70 && Upgrade == 1) Upgrade = 2;
        else if (Rating > 80 && Upgrade == 2) Upgrade = 3;
        //Downgrade
        if (Rating < 70 && Upgrade == 3) Upgrade = 2;
        else if (Rating < 60 && Upgrade == 2) Upgrade = 1;
        else if (Rating < 50 && Upgrade == 1) Upgrade = 0;
        break;
      }
      case 99:
        //weggaan, eerst naar uitgang
        HUM[tel][2] -= 3;
        HUM[tel][4]++;
        break;
      case 100:
      {
        const int DestX = 648 + HUM[tel][3] * 38;
        const int DestY = 544 + HUM[tel][3] * 38;
        walkto(tel, DestX, DestY);
        //als bij uitgang
        if (DestY - HUM[tel][1] < 15 && DestY - HUM[tel][1] > -15)
        {
          HUM[tel][2]--; //mens draaien
          HUM[tel][4]++;
        }
        break;
      }
      case 101:
      {
        //uit beeld
        const int DestX = -36 + HUM[tel][3] * 38;
        const int DestY = 1228 + HUM[tel][3] * 38;
        walkto(tel, DestX, DestY);
        //cijfers op 0: [4]=0,[2]=0,[5]=0,[6]=-1
        if (DestY - HUM[tel][1] < 14 && DestY - HUM[tel][1] > -14)
        {
          HUM[tel][2] = 0;
          HUM[tel][4] = 0;
          HUM[tel][5] = 0;
          HUM[tel][6] = -1;
        }
        break;
      }
    } //einde walkloop
  }

  //Happyconloop
  for (int tel = 0; tel < maxmensen; tel++)
  {
    HUM[tel][8] = 0;
    if (HUM[tel][7] < 11) HUM[tel][8] = 1;
    if (HUM[tel][7] > 10) HUM[tel][8] = 2;
    if (HUM[tel][7] > 50) HUM[tel][8] = 3;
    if (HUM[tel][7] > 75) HUM[tel][8] = 4;
  }
}

//---------------------------------------------------------------------------
void QtPublicToiletManagerWidget::onSpeedImageClick()
{
  welkecel = -1;

  //bepaald snelheid in speed, 0=pauze, 1=normaal, 2=fast
  switch (speed)
  {
    case 0:
      m_speed_image->load("IconSteady.bmp", true);
      m_peoplemove_timer.start(350);
      speed = 1;
      break;
    case 1:
      m_speed_image->load("IconFast.bmp", true);
      m_peoplemove_timer.start(200);
      speed = 2;
      break;
    case 2:
      m_speed_image->load("IconPause.bmp", true);
      m_peoplemove_timer.stop(); //A VCL timer with interval 0
      speed = 0;
      break;
  }
}

//---------------------------------------------------------------------------
#ifdef Q_OS_WASM
namespace {
const char* const local_storage_key = "PublicToiletManager_SaveGame";
}
#endif

QString QtPublicToiletManagerWidget::saveFilename()
{
  const QString folder = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
  QDir().mkpath(folder);
  return folder + "/SaveGame.sav";
}

bool QtPublicToiletManagerWidget::writeSaveGame(const QString& text)
{
#ifdef Q_OS_WASM
  emscripten::val::global("localStorage").call<void>("setItem", std::string(local_storage_key), text.toStdString());
  return true;
#else
  QFile file(saveFilename());
  if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) return false;
  QTextStream out(&file);
  out << text;
  return true;
#endif
}

QString QtPublicToiletManagerWidget::readSaveGame()
{
#ifdef Q_OS_WASM
  const emscripten::val item = emscripten::val::global("localStorage").call<emscripten::val>("getItem", std::string(local_storage_key));
  if (item.isNull() || item.isUndefined()) return QString();
  return QString::fromStdString(item.as<std::string>());
#else
  QFile file(saveFilename());
  if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) return QString();
  return QTextStream(&file).readAll();
#endif
}

void QtPublicToiletManagerWidget::onSaveImageClick()
{
  welkecel = -1;

  //Opslaan, op volgorde van declaratie, in hetzelfde formaat als het origineel
  QStringList lines;
  const auto add = [&lines](const int i) { lines << QString::number(i); };
  add(MX);
  add(MY);
  //De sanitair-array:
  for (int tel = 0; tel < 32; tel++) for (int tel2 = 0; tel2 < 6; tel2++) add(SAN[tel][tel2]);
  //De mensen-array:
  for (int tel = 0; tel < maxmensen; tel++) for (int tel2 = 0; tel2 < 9; tel2++) add(HUM[tel][tel2]);
  add(rowman);
  add(rowwoman);
  add(welkgeslacht);
  add(money);
  for (int tel = 0; tel < 6; tel++) add(VRD[tel]);
  for (int tel = 0; tel < 12; tel++) for (int tel2 = 0; tel2 < 2; tel2++) add(VRDP[tel][tel2]);
  for (int tel = 0; tel < 4; tel++) for (int tel2 = 0; tel2 < 2; tel2++) add(VRDS[tel][tel2]);
  for (int tel = 0; tel < 4; tel++) for (int tel2 = 0; tel2 < 2; tel2++) add(VRDT[tel][tel2]);
  add(klantteller);
  for (int tel = 0; tel < 20; tel++) add(Happy[tel]);
  add(Rating);
  add(Upgrade);
  //Controleregel
  lines << "Game Loaded";

  if (!writeSaveGame(lines.join("\r\n") + "\r\n"))
  {
    showMessage("Could not save the game to " + saveFilename());
    return;
  }
  showMessage("Game Saved");
}

void QtPublicToiletManagerWidget::onLoadImageClick()
{
  welkecel = -1;

  const QString text = readSaveGame();
  if (text.isEmpty())
  {
    showMessage("There is no saved game");
    return;
  }
  QStringList lines;
  for (const QString& line : text.split('\n')) lines << line.trimmed();

  const int n_numbers = 2 + 32 * 6 + maxmensen * 9 + 3 + 1 + 6 + 12 * 2 + 4 * 2 + 4 * 2 + 1 + 20 + 2;
  if (lines.size() <= n_numbers)
  {
    showMessage("The saved game is invalid");
    return;
  }

  //niet alles opslaan dus niet alles laden:
  clean = 666;
  clean2 = 0;
  watbouwen = 0;
  m_bouw_panel->hide();
  m_stock_panel->hide();
  m_speed_image->load("IconSteady.bmp", true);
  m_peoplemove_timer.start(350);
  speed = 1;
  close_state = 1;
  m_close_button->load("IconGo.bmp", true);

  //laden
  int R = 0;
  const auto next = [&lines, &R]() { return lines[R++].toInt(); };
  MX = next();
  MY = next();
  for (int tel = 0; tel < 32; tel++) for (int tel2 = 0; tel2 < 6; tel2++) SAN[tel][tel2] = next();
  for (int tel = 0; tel < maxmensen; tel++) for (int tel2 = 0; tel2 < 9; tel2++) HUM[tel][tel2] = next();
  rowman = next();
  rowwoman = next();
  welkgeslacht = next();
  money = next();
  for (int tel = 0; tel < 6; tel++) VRD[tel] = next();
  for (int tel = 0; tel < 12; tel++) for (int tel2 = 0; tel2 < 2; tel2++) VRDP[tel][tel2] = next();
  for (int tel = 0; tel < 4; tel++) for (int tel2 = 0; tel2 < 2; tel2++) VRDS[tel][tel2] = next();
  for (int tel = 0; tel < 4; tel++) for (int tel2 = 0; tel2 < 2; tel2++) VRDT[tel][tel2] = next();
  klantteller = next();
  for (int tel = 0; tel < 20; tel++) Happy[tel] = next();
  Rating = next();
  Upgrade = next();
  //controleregel laten zien
  showMessage(lines[R]);
}

//---------------------------------------------------------------------------
void QtPublicToiletManagerWidget::onCloseButtonClick()
{
  welkecel = -1;

  if (close_state == 0) //open gooien
  {
    close_state = 1;
    m_close_button->load("IconGo.bmp", true);
  }
  else if (close_state == 1) //dicht gooien
  {
    close_state = 0;
    m_close_button->load("IconClose.bmp", true);
  }
}
