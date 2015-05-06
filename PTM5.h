//---------------------------------------------------------------------------
/*
 *  Public Toilet Manager 5 Beta Edition - New Art Project.
 *  Copyright (C) 2006  Joost van den Bogaart
 *
 *  This program is free software: you can redistribute it and/or modify *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful, *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
//---------------------------------------------------------------------------
#ifndef PTM5H
#define PTM5H
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <ExtCtrls.hpp>
#include <Graphics.hpp>
#include <ImgList.hpp>
#include <Buttons.hpp>
#include <Grids.hpp>
#include <ComCtrls.hpp>
//---------------------------------------------------------------------------
class TForm1 : public TForm
{
__published:	// IDE-managed Components
        TImage *BackGround;
        TImage *CanvasImage;
        TTimer *CentralTimer;
        TImage *DumboImage;
        TImageList *ToiletList;
        TImage *WCAImage;
        TPanel *BouwPanel;
        TImage *SloopImage;
        TImage *WCBImage;
        TImage *WCCImage;
        TImage *WCUImage;
        TImage *WCSImage;
        TImageList *PeopleList;
        TImage *CanvasLayer;
        TImage *FrontLayer;
        TImageList *StockList;
        TPanel *StockPanel;
        TImage *PaperImage;
        TImage *TowelImage;
        TImage *SoapImage;
        TPanel *StatusPanel1;
        TEdit *MoneyEdit;
        TEdit *PaperEdit;
        TEdit *SoapEdit;
        TEdit *TowelEdit;
        TPanel *StatusPanel2;
        TProgressBar *ProgressBar1;
        TProgressBar *ProgressBar2;
        TPanel *MainPanel;
        TImage *BuildBtn;
        TImage *OrderBtn;
        TImage *CleanImage;
        TImageList *CleanList;
        TImage *CleanLayer;
        TImage *MoneyImage;
        TImage *PaperIconImage;
        TImage *TowelIconImage;
        TImage *SoapIconImage;
        TTimer *PeoplemoveTimer;
        TImageList *UpgradeList;
        TEdit *RatingEdit;
        TEdit *KlantEdit;
        TImageList *HappyconList;
        TImage *PeopleIconImage;
        TImage *HappyIconImage;
        TImage *CleanNesImage;
        TImageList *DirtList;
        TImage *SpeedImage;
        TImage *LoadImage;
        TImage *SaveImage;
        TMemo *OpslaMemo;
        TImage *CloseButton;
        TImage *FenceImage;
        TPanel *NameLabel;
        TImage *CleanSImage;
        TImage *PaperSImage;
        TImage *SoapSImage;
        TImage *TowelSImage;
        TImage *SelectImage;
        void __fastcall FormCreate(TObject *Sender);
        void __fastcall CentralTimerTimer(TObject *Sender);
        void __fastcall FormKeyDown(TObject *Sender, WORD &Key,
          TShiftState Shift);
        void __fastcall WCAImageClick(TObject *Sender);
        void __fastcall FormMouseMove(TObject *Sender, TShiftState Shift,
          int X, int Y);
        void __fastcall FormMouseDown(TObject *Sender, TMouseButton Button,
          TShiftState Shift, int X, int Y);
        void __fastcall SloopImageClick(TObject *Sender);
        void __fastcall WCBImageClick(TObject *Sender);
        void __fastcall WCCImageClick(TObject *Sender);
        void __fastcall WCUImageClick(TObject *Sender);
        void __fastcall WCSImageClick(TObject *Sender);
        void __fastcall PaperImageClick(TObject *Sender);
        void __fastcall TowelImageClick(TObject *Sender);
        void __fastcall SoapImageClick(TObject *Sender);
        void __fastcall OrderBtnClick(TObject *Sender);
        void __fastcall BuildBtnClick(TObject *Sender);
        void __fastcall PeoplemoveTimerTimer(TObject *Sender);
        void __fastcall SpeedImageClick(TObject *Sender);
        void __fastcall SaveImageClick(TObject *Sender);
        void __fastcall LoadImageClick(TObject *Sender);
        void __fastcall PaperIconImageDblClick(TObject *Sender);
        void __fastcall SoapIconImageClick(TObject *Sender);
        void __fastcall TowelIconImageClick(TObject *Sender);
        void __fastcall PeopleIconImageClick(TObject *Sender);
        void __fastcall HappyIconImageClick(TObject *Sender);
        void __fastcall CloseButtonClick(TObject *Sender);
private:	// User declarations
public:		// User declarations
        __fastcall TForm1(TComponent* Owner);
};
//---------------------------------------------------------------------------
extern PACKAGE TForm1 *Form1;
//---------------------------------------------------------------------------
#endif


 void plaatjeladen(TImage*DumboImage, AnsiString(MyString), TImageList*AList)
        {
         DumboImage->Picture->LoadFromFile(MyString);
         AList->AddMasked(DumboImage->Picture->Bitmap,clGreen);
        }

 void welkegekozen(int watbouwen, TImage*SelectImage, TImage*WatImage)
        {
        if (watbouwen==0)
             {
             SelectImage->Visible=false;
             }
        else
             {
             SelectImage->Left=WatImage->Left;
             SelectImage->Top=WatImage->Top;
             SelectImage->Visible=true;
             }
        }

 void walkto(int HUM[40][9],int tel,int DestX,int DestY)
        {
        //bewegen
        if (HUM[tel][0]<DestX) HUM[tel][0]+=19;
        if (HUM[tel][0]>DestX) HUM[tel][0]-=19;
        if (HUM[tel][1]<DestY) HUM[tel][1]+=19;
        if (HUM[tel][1]>DestY) HUM[tel][1]-=19;
        //lopen
        switch (HUM[tel][2])
           {
           case 1:
              HUM[tel][2]=3;
              break;
           case 2:
              HUM[tel][2]=4;
              break;
           case 3:
              HUM[tel][2]=1;
              break;
           case 4:
              HUM[tel][2]=2;
              break;
           case 7:
              HUM[tel][2]=9;
              break;
           case 8:
              HUM[tel][2]=10;
              break;
           case 9:
              HUM[tel][2]=7;
              break;
           case 10:
              HUM[tel][2]=8;
              break;
           case 13:
              HUM[tel][2]=15;
              break;
           case 14:
              HUM[tel][2]=16;
              break;
           case 15:
              HUM[tel][2]=13;
              break;
           case 16:
              HUM[tel][2]=14;
              break;
           }
        }
