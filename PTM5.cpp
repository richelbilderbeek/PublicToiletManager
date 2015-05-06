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
#include <vcl.h>
#pragma hdrstop
#include <dos.h>
#include <mmsystem.h>

#include "PTM5.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TForm1 *Form1;

int MX=0;  //voor bewegen van scherm
int MY=0;  //Tekent hij op CanvasImage, plaatjes kunnen op de canvaslayer...

//De sanitair-array:
int SAN[32][6];

//De mensen-array:
int HUM[40][9];
int maxmensen=40; //omvang array
int rowman=0;
int rowwoman=0;
int welkgeslacht=0;

//Array die bepaald wat hij neerzet:
int watbouwen=0;
int WCA, WCB, WCC, WCU, WCS, WCUR;

//Integer die bepaald welke cel aangeraaakt wordt
int welkecel=-1;
//en een die aangeeft wat aangeraakt wordt: plee, Sink, Urinoir...
int watte=0;

//Array met geld
//Array met voorraad
int money=5000;
int VRD[6]; //bevat voorraad, 2 posities per artikel, 2e voor hoeveelheid dozen
int VRDP[12][2];        //bevat info voor voorraadplaatjes
int VRDS[4][2];
int VRDT[4][2];

//Array met happyness, alsmede teller voor eerste 20 klanten
int klantteller=0;
int Happy[20];
int Rating=0; //rating
int Upgrade=0;

//clean: welk plaatje op statuspaneel, clean2 is om te voorkomen dat de boel onrustig wordt...
int clean=666;
int clean2=0;

//speed
int speed=1;
int dev=0; //onbelangrijke developmentinteger
int close=0; //0 is gesloten, 1 is open, activeerd mensengenerator
//---------------------------------------------------------------------------
__fastcall TForm1::TForm1(TComponent* Owner)
        : TForm(Owner)
{
}
//---------------------------------------------------------------------------

void __fastcall TForm1::FormCreate(TObject *Sender)
{
//positie
Form1->Left=10;
Form1->Top=10;
Form1->Height=720;
Form1->Width=1006;
CanvasImage->Height=700;
CanvasImage->Width=1000;

//panelen
  //NamePanel
      NameLabel->Left=Form1->Width-245;
      NameLabel->Top=Form1->Height-70;

  //statuspanel2
      StatusPanel2->Top=8;
      StatusPanel2->Left=8;
      CleanImage->Height=252; CleanImage->Width=167;
      CleanImage->Left=4; CleanImage->Top=4;
      CleanLayer->Height=252; CleanLayer->Width=167;
      CleanLayer->Left=4; CleanLayer->Top=4;
      ProgressBar1->Left=24; ProgressBar1->Top=CleanImage->Height+8;
      ProgressBar2->Left=24; ProgressBar2->Top=CleanImage->Height+ProgressBar1->Height+12;
      StatusPanel2->Width=CleanImage->Width+8;
      StatusPanel2->Height=CleanImage->Height+16+(ProgressBar1->Height*2);
      CleanSImage->Top=ProgressBar1->Top;
      SoapSImage->Top=ProgressBar1->Top;
      TowelSImage->Top=ProgressBar2->Top;
      PaperSImage->Top=ProgressBar2->Top;
  //StatusPanel1
      StatusPanel1->Top=8;
      StatusPanel1->Left=StatusPanel2->Width+16;

  //mainpanel
      MainPanel->Height=35;
      //MainPanel->Width=200;
      MainPanel->Left=8;
      MainPanel->Top=Form1->Height-MainPanel->Height-40;
  //StockPanel
      StockPanel->Height=145;
      StockPanel->Width=369;
      StockPanel->Top=Form1->Height-MainPanel->Height-StockPanel->Height-48;
      StockPanel->Left=8;
      PaperImage->Top=8; TowelImage->Top=8; SoapImage->Top=8;
      PaperImage->Left=8; SoapImage->Left=248; TowelImage->Left=128;
  //Bouwpanel
     BouwPanel->Height=321;
     BouwPanel->Width=369;
     BouwPanel->Top=Form1->Height-MainPanel->Height-BouwPanel->Height-48;
     BouwPanel->Left=8;
     WCAImage->Top=8;
     WCBImage->Top=WCAImage->Top;
     WCCImage->Top=WCAImage->Top;
     WCUImage->Top=184;
     WCSImage->Top=WCUImage->Top;
     SloopImage->Top=WCUImage->Top;
     WCAImage->Left=8;
     WCUImage->Left=WCAImage->Left;
     WCBImage->Left=128;
     WCSImage->Left=WCBImage->Left;
     WCCImage->Left=248;
     SloopImage->Left=WCCImage->Left;

//Cursors inladen
     Screen->Cursors[30]=LoadCursorFromFile("build.cur");
     Screen->Cursors[31]=LoadCursorFromFile("clean.cur");
     Form1->Cursor=31;
     
//knoppen inladen
     WCAImage->Picture->LoadFromFile("ToiletBasic.bmp");
     WCBImage->Picture->LoadFromFile("ToiletStandard.bmp");
     WCCImage->Picture->LoadFromFile("ToiletLuxery.bmp");
     WCUImage->Picture->LoadFromFile("ToiletUrinal.bmp");
     WCSImage->Picture->LoadFromFile("ToiletSink.bmp");
     SloopImage->Picture->LoadFromFile("ToiletDelete.bmp");
     PaperImage->Picture->LoadFromFile("IconPaper.bmp");
     SoapImage->Picture->LoadFromFile("IconSoap.bmp");
     TowelImage->Picture->LoadFromFile("IconTowels.bmp");

//plaatjes inladen toiletten
   BackGround->Picture->LoadFromFile("BackGround.bmp");
   FrontLayer->Picture->LoadFromFile("FrontLayer1.bmp");
//in list
   plaatjeladen(DumboImage,"WCLEEG.bmp",ToiletList); //0
 WCA=1;
   plaatjeladen(DumboImage,"WCA00.bmp",ToiletList);  //1
   plaatjeladen(DumboImage,"WCA01.bmp",ToiletList);  //2
   plaatjeladen(DumboImage,"WCA02M.bmp",ToiletList); //3
   plaatjeladen(DumboImage,"WCA03M.bmp",ToiletList); //4
   plaatjeladen(DumboImage,"WCA04M.bmp",ToiletList); //5
   plaatjeladen(DumboImage,"WCA05M.bmp",ToiletList);
   plaatjeladen(DumboImage,"WCA06M.bmp",ToiletList);
   plaatjeladen(DumboImage,"WCA02K.bmp",ToiletList);
   plaatjeladen(DumboImage,"WCA03K.bmp",ToiletList);
   plaatjeladen(DumboImage,"WCA04K.bmp",ToiletList); //10
   plaatjeladen(DumboImage,"WCA05K.bmp",ToiletList);
   plaatjeladen(DumboImage,"WCA06K.bmp",ToiletList);
   plaatjeladen(DumboImage,"WCA02W.bmp",ToiletList);
   plaatjeladen(DumboImage,"WCA03W.bmp",ToiletList);
   plaatjeladen(DumboImage,"WCA04W.bmp",ToiletList); //15
   plaatjeladen(DumboImage,"WCA05W.bmp",ToiletList);
   plaatjeladen(DumboImage,"WCA06W.bmp",ToiletList); //17
  WCB=18;
   plaatjeladen(DumboImage,"WCB00.bmp",ToiletList);  //18
   plaatjeladen(DumboImage,"WCB01.bmp",ToiletList);  //19
   plaatjeladen(DumboImage,"WCB02M.bmp",ToiletList); //20
   plaatjeladen(DumboImage,"WCB03M.bmp",ToiletList); //21
   plaatjeladen(DumboImage,"WCB04M.bmp",ToiletList); //22
   plaatjeladen(DumboImage,"WCB05M.bmp",ToiletList);
   plaatjeladen(DumboImage,"WCB06M.bmp",ToiletList);
   plaatjeladen(DumboImage,"WCB02K.bmp",ToiletList);
   plaatjeladen(DumboImage,"WCB03K.bmp",ToiletList);
   plaatjeladen(DumboImage,"WCB04K.bmp",ToiletList); //27
   plaatjeladen(DumboImage,"WCB05K.bmp",ToiletList);
   plaatjeladen(DumboImage,"WCB06K.bmp",ToiletList);
   plaatjeladen(DumboImage,"WCB02W.bmp",ToiletList);
   plaatjeladen(DumboImage,"WCB03W.bmp",ToiletList);
   plaatjeladen(DumboImage,"WCB04W.bmp",ToiletList); //32
   plaatjeladen(DumboImage,"WCB05W.bmp",ToiletList);
   plaatjeladen(DumboImage,"WCB06W.bmp",ToiletList); //34
  WCC=35;
   plaatjeladen(DumboImage,"WCC00.bmp",ToiletList);  //35
   plaatjeladen(DumboImage,"WCC01.bmp",ToiletList);  //36
   plaatjeladen(DumboImage,"WCC02M.bmp",ToiletList); //37
   plaatjeladen(DumboImage,"WCC03M.bmp",ToiletList); //38
   plaatjeladen(DumboImage,"WCC04M.bmp",ToiletList); //39
   plaatjeladen(DumboImage,"WCC05M.bmp",ToiletList);
   plaatjeladen(DumboImage,"WCC06M.bmp",ToiletList);
   plaatjeladen(DumboImage,"WCC02K.bmp",ToiletList);
   plaatjeladen(DumboImage,"WCC03K.bmp",ToiletList);
   plaatjeladen(DumboImage,"WCC04K.bmp",ToiletList); //44
   plaatjeladen(DumboImage,"WCC05K.bmp",ToiletList);
   plaatjeladen(DumboImage,"WCC06K.bmp",ToiletList);
   plaatjeladen(DumboImage,"WCC02W.bmp",ToiletList);
   plaatjeladen(DumboImage,"WCC03W.bmp",ToiletList);
   plaatjeladen(DumboImage,"WCC04W.bmp",ToiletList); //49
   plaatjeladen(DumboImage,"WCC05W.bmp",ToiletList);
   plaatjeladen(DumboImage,"WCC06W.bmp",ToiletList); //51
  WCU=52;
   plaatjeladen(DumboImage,"WCU00.bmp",ToiletList);  //52
   plaatjeladen(DumboImage,"WCU01.bmp",ToiletList);  //53
   plaatjeladen(DumboImage,"WCU02M.bmp",ToiletList); //54
   plaatjeladen(DumboImage,"WCU03M.bmp",ToiletList); //55
   plaatjeladen(DumboImage,"WCU04M.bmp",ToiletList);
   plaatjeladen(DumboImage,"WCU05M.bmp",ToiletList);
   plaatjeladen(DumboImage,"WCU06M.bmp",ToiletList); //58
   plaatjeladen(DumboImage,"WCU02K.bmp",ToiletList);
   plaatjeladen(DumboImage,"WCU03K.bmp",ToiletList);
   plaatjeladen(DumboImage,"WCU04K.bmp",ToiletList);
   plaatjeladen(DumboImage,"WCU05K.bmp",ToiletList);
   plaatjeladen(DumboImage,"WCU06K.bmp",ToiletList); //63
  WCUR=64;
   plaatjeladen(DumboImage,"WCU10.bmp",ToiletList);  //64
   plaatjeladen(DumboImage,"WCU11.bmp",ToiletList);  //65
   plaatjeladen(DumboImage,"WCU12M.bmp",ToiletList); //66
   plaatjeladen(DumboImage,"WCU13M.bmp",ToiletList); //67
   plaatjeladen(DumboImage,"WCU14M.bmp",ToiletList);
   plaatjeladen(DumboImage,"WCU15M.bmp",ToiletList);
   plaatjeladen(DumboImage,"WCU16M.bmp",ToiletList); //70
   plaatjeladen(DumboImage,"WCU12K.bmp",ToiletList);
   plaatjeladen(DumboImage,"WCU13K.bmp",ToiletList);
   plaatjeladen(DumboImage,"WCU14K.bmp",ToiletList);
   plaatjeladen(DumboImage,"WCU15K.bmp",ToiletList);
   plaatjeladen(DumboImage,"WCU16K.bmp",ToiletList); //75
  WCS=76;
   plaatjeladen(DumboImage,"WCS10.bmp",ToiletList);  //76
   plaatjeladen(DumboImage,"WCS11.bmp",ToiletList);
   plaatjeladen(DumboImage,"WCS12M.bmp",ToiletList); //78
   plaatjeladen(DumboImage,"WCS13M.bmp",ToiletList); //79
   plaatjeladen(DumboImage,"WCS14M.bmp",ToiletList); //80
   plaatjeladen(DumboImage,"WCS15M.bmp",ToiletList);
   plaatjeladen(DumboImage,"WCS16M.bmp",ToiletList);
   plaatjeladen(DumboImage,"WCS12K.bmp",ToiletList);
   plaatjeladen(DumboImage,"WCS13K.bmp",ToiletList);
   plaatjeladen(DumboImage,"WCS14K.bmp",ToiletList); //85
   plaatjeladen(DumboImage,"WCS15K.bmp",ToiletList);
   plaatjeladen(DumboImage,"WCS16K.bmp",ToiletList);
   plaatjeladen(DumboImage,"WCS12W.bmp",ToiletList);
   plaatjeladen(DumboImage,"WCS13W.bmp",ToiletList);
   plaatjeladen(DumboImage,"WCS14W.bmp",ToiletList); //90
   plaatjeladen(DumboImage,"WCS15W.bmp",ToiletList);
   plaatjeladen(DumboImage,"WCS16W.bmp",ToiletList);


//plaatjes inladen mensen:
   plaatjeladen(DumboImage,"MEMPTY.bmp",PeopleList);
  //kinderen 1-6
   plaatjeladen(DumboImage,"MK1L.bmp",PeopleList);
   plaatjeladen(DumboImage,"MK1R.bmp",PeopleList);
   plaatjeladen(DumboImage,"MK2L.bmp",PeopleList);
   plaatjeladen(DumboImage,"MK2R.bmp",PeopleList);
   plaatjeladen(DumboImage,"MKWL.bmp",PeopleList);
   plaatjeladen(DumboImage,"MKWR.bmp",PeopleList);
  //mannen 7-12
   plaatjeladen(DumboImage,"MM1L.bmp",PeopleList);
   plaatjeladen(DumboImage,"MM1R.bmp",PeopleList);
   plaatjeladen(DumboImage,"MM2L.bmp",PeopleList);
   plaatjeladen(DumboImage,"MM2R.bmp",PeopleList);
   plaatjeladen(DumboImage,"MMWL.bmp",PeopleList);
   plaatjeladen(DumboImage,"MMWR.bmp",PeopleList);
  //vrouwen 13-
   plaatjeladen(DumboImage,"MW1L.bmp",PeopleList);
   plaatjeladen(DumboImage,"MW1R.bmp",PeopleList);
   plaatjeladen(DumboImage,"MW2L.bmp",PeopleList);
   plaatjeladen(DumboImage,"MW2R.bmp",PeopleList);
   plaatjeladen(DumboImage,"MWWL.bmp",PeopleList);
   plaatjeladen(DumboImage,"MWWR.bmp",PeopleList);
 //Zenuwachtigen   19-
   plaatjeladen(DumboImage,"MKWZ.bmp",PeopleList);
   plaatjeladen(DumboImage,"MMWZ.bmp",PeopleList);
   plaatjeladen(DumboImage,"MWWZ.bmp",PeopleList);

//plaatjes inladen stock
   plaatjeladen(DumboImage,"BoxToiletPaper.bmp",StockList);
   plaatjeladen(DumboImage,"BoxSoap.bmp",StockList);
   plaatjeladen(DumboImage,"BoxTowel.bmp",StockList);

//cleanlist inladen
   plaatjeladen(DumboImage,"Logo.bmp",CleanList);
   plaatjeladen(DumboImage,"CleanSimpel1.bmp",CleanList);
   plaatjeladen(DumboImage,"CleanSimpel2.bmp",CleanList);
   plaatjeladen(DumboImage,"CleanSimpel3.bmp",CleanList);
   plaatjeladen(DumboImage,"CleanSimpel4.bmp",CleanList);
   plaatjeladen(DumboImage,"CleanStandard1.bmp",CleanList);
   plaatjeladen(DumboImage,"CleanStandard2.bmp",CleanList);
   plaatjeladen(DumboImage,"CleanStandard3.bmp",CleanList);
   plaatjeladen(DumboImage,"CleanStandard4.bmp",CleanList);
   plaatjeladen(DumboImage,"CleanLuxe1.bmp",CleanList);
   plaatjeladen(DumboImage,"CleanLuxe2.bmp",CleanList);
   plaatjeladen(DumboImage,"CleanLuxe3.bmp",CleanList);
   plaatjeladen(DumboImage,"CleanLuxe4.bmp",CleanList);
   plaatjeladen(DumboImage,"CleanUrinal1.bmp",CleanList);
   plaatjeladen(DumboImage,"CleanUrinal2.bmp",CleanList);
   plaatjeladen(DumboImage,"CleanUrinal3.bmp",CleanList);
   plaatjeladen(DumboImage,"CleanUrinal4.bmp",CleanList); //16
   plaatjeladen(DumboImage,"CleanSink1.bmp",CleanList);
   plaatjeladen(DumboImage,"Clean5.bmp",CleanList);
   plaatjeladen(DumboImage,"CleanUrinal5.bmp",CleanList);
   plaatjeladen(DumboImage,"CleanSink5.bmp",CleanList);
   //plaatjeladen(DumboImage,"Clean6.bmp",CleanList);
//Upgrades
   plaatjeladen(DumboImage,"Upgrade1.bmp",UpgradeList);
   plaatjeladen(DumboImage,"Upgrade2.bmp",UpgradeList);
   plaatjeladen(DumboImage,"Upgrade3.bmp",UpgradeList);
   plaatjeladen(DumboImage,"Upgrade4.bmp",UpgradeList);

//Happycons
   plaatjeladen(DumboImage,"Happyicon0.bmp",HappyconList);
   plaatjeladen(DumboImage,"Happyicon1.bmp",HappyconList);
   plaatjeladen(DumboImage,"Happyicon2.bmp",HappyconList);
   plaatjeladen(DumboImage,"Happyicon3.bmp",HappyconList);
   plaatjeladen(DumboImage,"Happyicon4.bmp",HappyconList);

//overig
   CleanNesImage->Picture->LoadFromFile("Clean.bmp");
   plaatjeladen(DumboImage,"Dirt1.bmp",DirtList);
   plaatjeladen(DumboImage,"Dirt2.bmp",DirtList);
   plaatjeladen(DumboImage,"Dirt3.bmp",DirtList);
   FenceImage->Picture->LoadFromFile("Fence.bmp");

   //Sanitairarray vullen
for (int tel=0; tel<32; tel++)
        {
        //0 = X:, 1 = Y;
        //tot 12 damestoiletten,
        if (tel<8)
           {
           SAN[tel][0]=314+(tel*38);
           SAN[tel][1]=74+(tel*38);
           }
        else if (tel<16)//verder herentoiletten
           {
           SAN[tel][0]=314+((tel+4)*38);
           SAN[tel][1]=74+((tel+4)*38);
           }
        else if (tel<24)//dameswasbakken
           {
           SAN[tel][0]=314+((tel-16)*38)-103;
           SAN[tel][1]=74+((tel-16)*38)+103;
           }
        else //verder herendingen
           {
           SAN[tel][0]=314+((tel+4-16)*38)-103;
           SAN[tel][1]=74+((tel+4-16)*38)+103;
           }
        //2 = wat laten zien, begin is niks
        SAN[tel][2]=0;
        //3 = welke persoon en hoe lang het duurt
        SAN[tel][3]=-1;
        //4 = schoonheid of zeep
        SAN[tel][4]=100;
        //5 = papier of towels
        SAN[tel][5]=100;
        }

//mensenarray vullen
for (int tel=0; tel<maxmensen; tel++)
        {
        HUM[tel][0]=-36;
        HUM[tel][1]=1228;
        //0 = X; 1 = Y;
        //2 = leeftijd, welk plaatje; niet inladen
        //3 = geslacht; niet inladen
        //4 = indexnummer: 0 is beschikbaar
        HUM[tel][4]=0;
        //5 = plek in rij: leeg = 0;
        HUM[tel][5]=0;
        //6 is welk toilet,
        HUM[tel][6]=-1;
        //7 is tevredenheid, bij begin 10,
        //vervolgens:
        //gaat er af als lang in rij
        //schoon toilet levert 50 op, vies kost 50
        //geen papier kost 50
        //basis toilet: bonus is 10
        //luxe toilet: bonus is 25
        //sink: levert 25 op
        //geen towels en zeep kost elk 10
        //betaald tevredenheid "letterlijk" uit, max 100
        //tevredenheid kan negatief zijn, betaald dan 5

        //9 bepaald Happycon
        }

//VoorraadArray vullen:
VRD[0]=60;//papier
VRD[2]=20;//zeep
VRD[4]=20;//towels
for (int vrd=0; vrd<4; vrd++)
        {
        VRDP[vrd][0]=760;  VRDP[vrd][1]=376-vrd*19;
        VRDP[vrd+4][0]=784;  VRDP[vrd+4][1]=400-vrd*19;
        VRDP[vrd+8][0]=808;  VRDP[vrd+8][1]=424-vrd*19;
        VRDS[vrd][0]=832;  VRDS[vrd][1]=448-vrd*19;
        VRDT[vrd][0]=856;  VRDT[vrd][1]=472-vrd*19;
        }

//pijl inladen
DumboImage->Picture->LoadFromFile("Arrow.bmp");

}
//---------------------------------------------------------------------------

void __fastcall TForm1::CentralTimerTimer(TObject *Sender)
{
//BOUWEN:
//op gevoelige cel eventueel iets tijdelijks tekenen...
   //Maar eerst alle tijdelijke cellen op 0:
if (watbouwen>0)
   {
   for (int tel=0; tel<32; tel++)
        {
        if (SAN[tel][2]==-1||SAN[tel][2]==WCA||SAN[tel][2]==WCB||SAN[tel][2]==WCC||SAN[tel][2]==WCU||SAN[tel][2]==WCS||SAN[tel][2]==WCUR)
           {
           SAN[tel][2]=0;
           }
        //anders misschien iets laten zien
        else
        if (tel==welkecel&&SAN[tel][2]==0)
           {
           //links wasbakken, rechts toiletten, urinoir goed
           if (watbouwen==WCA||watbouwen==WCB||watbouwen==WCC)
           if (tel<16)
                {
                SAN[tel][2]=watbouwen;
                }
           if (watbouwen==WCS)
           if (tel>15)
                {
                SAN[tel][2]=watbouwen;
                }
           if (watbouwen==WCU)
             {
             if (tel>7&&tel<16)
                {
                SAN[tel][2]=watbouwen;
                }
               if (tel>23)
                {
                SAN[tel][2]=WCUR;
                }
             }
           }
        }
     }

//Voorraad, 2e positie array is altijd 1/10 van 1e
for (int tel=0; tel<6; tel+=2)
        {
        VRD[tel+1]=VRD[tel]/10;
        }
//en voorraad maximaliseren
if (VRD[1]>12) VRD[1]=12;
if (VRD[3]>4) VRD[3]=4;
if (VRD[5]>4) VRD[5]=4;

//statuspanel updaten
MoneyEdit->Text=money;
PaperEdit->Text=VRD[0];
SoapEdit->Text=VRD[2];
TowelEdit->Text=VRD[4];

//Buffer, tekenen
        CanvasLayer->Picture=BackGround->Picture;
        //sanitair 1 tot 16:
        for(int tel=0; tel<16; tel++)
                {
                if (SAN[tel][4]<10) //dirt inladen?
                    {
                    DirtList->Draw(CanvasLayer->Canvas,SAN[tel][0],SAN[tel][1],0);
                    if (SAN[tel][2]<WCU&&SAN[tel][2]>WCA)
                    DirtList->Draw(CanvasLayer->Canvas,SAN[tel][0],SAN[tel][1],2);
                    }
                ToiletList->Draw(CanvasLayer->Canvas,SAN[tel][0],SAN[tel][1],SAN[tel][2]);
                }
        //mensen
        for(int tel=0; tel<maxmensen; tel++)
                {
                PeopleList->Draw(CanvasLayer->Canvas,HUM[tel][0],HUM[tel][1],HUM[tel][2]);
                }


        //sanitair 17 tot 32:
        for(int tel=16; tel<32; tel++)
                {
                if (SAN[tel][4]<10&&SAN[tel][2]>WCUR&&SAN[tel][2]<WCS) //dirt inladen?
                    {
                    DirtList->Draw(CanvasLayer->Canvas,SAN[tel][0],SAN[tel][1],1);
                    }
                ToiletList->Draw(CanvasLayer->Canvas,SAN[tel][0],SAN[tel][1],SAN[tel][2]);
                }


        //stock: toiletpapier, hoeveelheid dozen staat in oneven vrd
        for(int tel=0; tel<VRD[1]; tel++)
                {
                StockList->Draw(CanvasLayer->Canvas,VRDP[tel][0],VRDP[tel][1],0);
        //stock: soap
                }
        for(int tel=0; tel<VRD[3]; tel++)
                {
                StockList->Draw(CanvasLayer->Canvas,VRDS[tel][0],VRDS[tel][1],1);
                }
        //stock: paper
        for(int tel=0; tel<VRD[5]; tel++)
                {
                StockList->Draw(CanvasLayer->Canvas,VRDT[tel][0],VRDT[tel][1],2);
                }

        //Eventueel hek, als gesloten en geen mensen aanwezig
        if (close==0)
                {
                int controle=0;
                for (int tel=0; tel<maxmensen; tel++)
                    {
                    if (HUM[tel][4]>0)
                        {
                        controle=1;
                        }
                    }
                if (controle==0)
                    {
                    CanvasLayer->Canvas->Draw(463,671,FenceImage->Picture->Graphic);
                    }
                }

        //Frontlayer
        CanvasLayer->Canvas->Draw(0,0,FrontLayer->Picture->Graphic);
       //Happycons
        for(int tel=0; tel<maxmensen; tel++)
                {
                if (HUM[tel][2]!=0) //als mensen zichtbaar
                HappyconList->Draw(CanvasLayer->Canvas,HUM[tel][0]+4,HUM[tel][1]-14,HUM[tel][8]);
                }
       //laten zien als onderhoud nodig is...
       for(int tel=0; tel<32; tel++)
             {
             int A=90;
             int B=-40;
             if (tel>15)
                 {
                 A=15;
                 B=20;
                 }
             if (SAN[tel][4]<10||SAN[tel][5]<10&&SAN[tel][2]<WCU||SAN[tel][5]<10&&SAN[tel][2]>WCS) CanvasLayer->Canvas->Draw(SAN[tel][0]+A,SAN[tel][1]+B,CleanNesImage->Picture->Graphic);
             }

        //Arrow
        if (welkecel<16&&welkecel>-1)
                {
                CanvasLayer->Canvas->Draw(SAN[welkecel][0]+70,SAN[welkecel][1],DumboImage->Picture->Graphic);
                }
        else if (welkecel>15)
                {
                CanvasLayer->Canvas->Draw(SAN[welkecel][0]+11,SAN[welkecel][1]+52,DumboImage->Picture->Graphic);
                }

        //Upgrades:
        UpgradeList->Draw(CanvasLayer->Canvas,-4,566,Upgrade);

        //Op canvas
        CanvasImage->Canvas->Draw(MX,MY,CanvasLayer->Picture->Graphic);
        Form1->Canvas->Draw(0,0,CanvasImage->Picture->Graphic);


}
//---------------------------------------------------------------------------
void __fastcall TForm1::FormKeyDown(TObject *Sender, WORD &Key,
      TShiftState Shift)
{
//bewegen van veld, Beinvloed waar hij de Canvasimage tekent.

switch (Key)
        {
        case VK_DOWN:
             if (MY>CanvasImage->Height-BackGround->Height)
                {
                MY-=26;
                }
             break;
        case VK_UP:
             if (MY<0)
                {
                MY+=26;
                }
             break;
        case VK_RIGHT:
             if (MX>CanvasImage->Width-BackGround->Width)
                {
                MX-=26;
                }
             break;
        case VK_LEFT:
             if (MX<0)
                {
                MX+=26;
                }
             break;

        }
}
//---------------------------------------------------------------------------

void __fastcall TForm1::WCAImageClick(TObject *Sender)
{
 if (watbouwen!=WCA) //simpel toilet bouwen
        {
        watbouwen=WCA;
        }
else watbouwen=0; //anders bouwen uitzetten

//voor rood vakje om gekozen plaatje
SelectImage->Picture->LoadFromFile("ToiletSelect.bmp");
welkegekozen(watbouwen,SelectImage,WCAImage);
//voorkomen dat spel hangt
welkecel=-1;
}
//---------------------------------------------------------------------------

void __fastcall TForm1::FormMouseMove(TObject *Sender, TShiftState Shift,
      int X, int Y)
{
//om te voorkomen dat claenplaatje flikkert, niet bij begin!
clean2=clean;
if (clean==666) clean=0;

//alle toiletten langsgaan, anders alle wasbakken
for(int tel=0; tel<33; tel++)
        {
        //gevoelig veld, tellen tot 17 omdat misschien niks actief
        //if (tel<16&&X>SAN[tel][0]+89&&X<SAN[tel][0]+121&&Y>SAN[tel][1]+4&&Y<SAN[tel][1]+54)
        //op WC-deurjes
        if (tel<16&&X-MX>SAN[tel][0]+43&&X-MX<SAN[tel][0]+76&&Y-MY>SAN[tel][1]+52&&Y-MY<SAN[tel][1]+149||tel<16&&X-MX>SAN[tel][0]+78&&X-MX<SAN[tel][0]+111&&Y-MY>SAN[tel][1]+1&&Y-MY<SAN[tel][1]+85)
           {
           welkecel=tel;
           tel=33;
           }
        else if (tel<32&&X-MX>SAN[tel][0]+9&&X-MX<SAN[tel][0]+38&&Y-MY>SAN[tel][1]+94&&Y-MY<SAN[tel][1]+195) //anders wasbakachterwanden
           {
           welkecel=tel;
           tel=33;
           }
        else if (tel==32) //niks actief
           {
           welkecel=-1;
           }
        }

//statuspanel 2 updaten, als cel actief
//en WC vrij, slaat wat op in watte
if (welkecel>-1) //toiletten
   {
   if (SAN[welkecel][2]==WCA+1||SAN[welkecel][2]==WCB+1||SAN[welkecel][2]==WCC+1||SAN[welkecel][2]==WCS+1)
        {
        ProgressBar1->Position=SAN[welkecel][4];
        ProgressBar2->Position=SAN[welkecel][5];
        ProgressBar1->Show();
        ProgressBar2->Show();
        watte=1;   //wc
        //plaatjes
        if (SAN[welkecel][2]==WCS+1)
             {
             watte=2; //sink
             clean=17; //logo laten zien
             SoapSImage->Show();
             TowelSImage->Show();
             CleanSImage->Hide();
             PaperSImage->Hide();
             }
        if (SAN[welkecel][2]==WCA+1)
             {
             clean=1; //wc1 laten zien
             CleanSImage->Show();
             PaperSImage->Show();
             SoapSImage->Hide();
             TowelSImage->Hide();
             }
        if (SAN[welkecel][2]==WCB+1)
             {
             clean=5; //wc2 laten zien
             CleanSImage->Show();
             PaperSImage->Show();
             SoapSImage->Hide();
             TowelSImage->Hide();
             }
        if (SAN[welkecel][2]==WCC+1)
             {
             clean=9; //wc3 laten zien
             CleanSImage->Show();
             PaperSImage->Show();
             SoapSImage->Hide();
             TowelSImage->Hide();
             }
        }
    else if (SAN[welkecel][2]==WCU+1||SAN[welkecel][2]==WCUR+1)
        {
        ProgressBar1->Position=SAN[welkecel][4];
        ProgressBar1->Show();
        ProgressBar2->Hide();
        watte=3;   //urinoir
        clean=13;
        CleanSImage->Show();
        PaperSImage->Hide();
        SoapSImage->Hide();
        TowelSImage->Hide();
        }
    else //bezette WC?
    if (SAN[welkecel][2]>WCA&&SAN[welkecel][2]<WCB||SAN[welkecel][2]>WCB&&SAN[welkecel][2]<WCC||SAN[welkecel][2]>WCC&&SAN[welkecel][2]<WCU)
        {
        ProgressBar1->Hide();
        ProgressBar2->Hide();
        watte=2;
        clean=18;
        //if (random(80)==20) clean = 21;
        CleanSImage->Hide();
        PaperSImage->Hide();
        SoapSImage->Hide();
        TowelSImage->Hide();
        }
    else //bezet urinoir?
    if (SAN[welkecel][2]>WCU&&SAN[welkecel][2]<WCS&&SAN[welkecel][2]!=WCUR)
        {
        ProgressBar1->Hide();
        ProgressBar2->Hide();
        watte=2;
        clean=19;
        CleanSImage->Hide();
        PaperSImage->Hide();
        SoapSImage->Hide();
        TowelSImage->Hide();
        }
    else //bezette sink?
    if (SAN[welkecel][2]>WCS)
        {
        ProgressBar1->Hide();
        ProgressBar2->Hide();
        watte=2;
        clean=20;
        CleanSImage->Hide();
        PaperSImage->Hide();
        SoapSImage->Hide();
        TowelSImage->Hide();
        }
    }
else //geen cel actief
        {
        ProgressBar1->Hide();
        ProgressBar2->Hide();
        watte=0;
        clean=0;
        CleanSImage->Hide();
        PaperSImage->Hide();
        SoapSImage->Hide();
        TowelSImage->Hide();
        }

//Bij clean wat optellen als toilet vies, geen sink, wat geselecteerd
if (clean>0&&clean<17&&welkecel>-1&&watte!=2)
    {
    if (SAN[welkecel][4]<26) clean++;
    if (SAN[welkecel][4]<51) clean++;
    if (SAN[welkecel][4]<76) clean++;
    }

//Updaten CleanImage
if (clean!=clean2)
    {
    CleanList->Draw(CleanLayer->Canvas,0,0,clean);
    CleanImage->Canvas->Draw(0,0,CleanLayer->Picture->Graphic);
    }

//bewegen scherm?
if (Y>Form1->Height-100)
             if (MY>CanvasImage->Height-BackGround->Height)
                {
                MY-=26;
                }
if (Y<100)
             if (MY<0)
                {
                MY+=26;
                }
if (X>Form1->Width-100)
             if (MX>CanvasImage->Width-BackGround->Width)
                {
                MX-=26;
                }
if (X<100)
             if (MX<0)
                {
                MX+=26;
                }

}
//---------------------------------------------------------------------------


void __fastcall TForm1::FormMouseDown(TObject *Sender, TMouseButton Button,
      TShiftState Shift, int X, int Y)
{
//Alleen als bouwen uit staat en cel actief: onderhoud
if (watbouwen==0&&welkecel>-1)
        {
        switch (watte)
            {
            case 1://wc schoonmaak en papier als nodig
                 {
                 SAN[welkecel][4]=100;  //schoonmaak
                 //evt papier als nodig en op voorraad:
                 if (SAN[welkecel][5]<41&&VRD[0]>0)
                    {
                    SAN[welkecel][5]=100;
                    VRD[0]-=1;      //papier op maken
                    }
                 money-=10;
                 break;
                 }
            case 2://wasbak bijvullen als nodig
                 {
                 //evt zeep als op voorraad:
                 if (VRD[2]>0)
                    {
                    SAN[welkecel][4]=100;
                    VRD[2]-=1;      //zeep op maken
                    }
                 //evt towels als op voorraad:
                 if (VRD[4]>0)
                    {
                    SAN[welkecel][5]=100;
                    VRD[4]-=1;      //towels op maken
                    }
                 money-=5;
                 break;
                 }
            case 3://urinoir schoonmaak
                 {
                 SAN[welkecel][4]=100;  //schoonmaak
                 money-=5;
                 break;
                 }

            }
        }

//BOUWEN
//Bij Muisclick moet hij -misschien- iets tijdelijks iets echts maken:
if (watbouwen>0)  //bouwbutton ingedrukt
    {
    if (welkecel>-1)  //cel geselecteerd
        {
        if (SAN[welkecel][2]==0||SAN[welkecel][2]==watbouwen||SAN[welkecel][2]==WCUR) //iets tijdelijks
            {
            //Toiletten rechts en geld:
            if (watbouwen==WCA&&money>799||watbouwen==WCB&&money>1999||watbouwen==WCC&&money>3999)
            if (welkecel<16)
                {
                    SAN[welkecel][2]=watbouwen+1;
                    if (watbouwen==WCA) money-=800;
                    if (watbouwen==WCB) money-=2000;
                    if (watbouwen==WCC) money-=4000;

                    //papier en schoonheid
                    SAN[welkecel][4]=100;
                    SAN[welkecel][5]=100;
                }
            //wastafels links
            if (watbouwen==WCS&&money>499)
            if (welkecel>15)
                {
                SAN[welkecel][2]=watbouwen+1;
                money-=500;
                     //towels en zeep
                    SAN[welkecel][4]=100;
                    SAN[welkecel][5]=100;
                }
            //Urinoirs bij heren:
            if (watbouwen==WCU&&money>499)
                {
                if (welkecel>7&&welkecel<16)
                  {
                  SAN[welkecel][2]=watbouwen+1;
                  }
                else if (welkecel>23)
                  {
                  SAN[welkecel][2]=WCUR+1;
                  }
                money-=500;
                //schoonheid, geen papier
                 SAN[welkecel][4]=100;
                 SAN[welkecel][5]=9999;
                }

            /*
            //tenzij urinoir bij dames
            if (welkecel>7&&watbouwen==WCU||watbouwen!=WCU)
                {
                SAN[welkecel][2]=watbouwen+1; //definitief maken
                //hier hoeveel het kost
                }
            //Wastafels en Linker Urinoirs:
            */
            }
        }
    }
//SLOPEN
if (watbouwen==-1)  //sloopbutton ingedrukt
    {
    if (welkecel>-1)  //cel geselecteerd
        {
        if (SAN[welkecel][2]==WCA+1||SAN[welkecel][2]==WCB+1||SAN[welkecel][2]==WCC+1||SAN[welkecel][2]==WCU+1||SAN[welkecel][2]==WCUR+1||SAN[welkecel][2]==WCS+1) //iets onbezets
            {
            SAN[welkecel][2]=0; //slopen
            SAN[welkecel][3]=-1;//cel vrijhouden
            //mens dat gekozen had naar huis:
            for(int tel=0; tel<maxmensen; tel++)
               {
               if (HUM[tel][6]==welkecel)
                  {
                    //even staan
                    if (HUM[tel][2]<7) HUM[tel][2]=5; //kind
                    else
                    if (HUM[tel][2]<13) HUM[tel][2]=11; //man
                    else HUM[tel][2]=17; //vrouw
                    //naar huis
                    HUM[tel][4]=99;
                  }
               }
            //hier hoeveel het oplevert
            money+=100;
            SAN[welkecel][4]=100;
            SAN[welkecel][5]=100;
            }
        }
    }
}
//---------------------------------------------------------------------------

void __fastcall TForm1::SloopImageClick(TObject *Sender)
{
//Slopen
watbouwen=-1;
//voor rood vakje om gekozen plaatje
SelectImage->Picture->LoadFromFile("ToiletSelect2.bmp");
welkegekozen(watbouwen,SelectImage,SloopImage);
//voorkomen dat spel hangt
welkecel=-1;

}
//---------------------------------------------------------------------------

void __fastcall TForm1::WCBImageClick(TObject *Sender)
{
 if (watbouwen!=WCB) //Normaal toilet bouwen
        {
        watbouwen=WCB;
        }
else watbouwen=0; //anders bouwen uitzetten
//voor rood vakje om gekozen plaatje
SelectImage->Picture->LoadFromFile("ToiletSelect.bmp");
welkegekozen(watbouwen,SelectImage,WCBImage);
//voorkomen dat spel hangt
welkecel=-1;
}
//---------------------------------------------------------------------------

void __fastcall TForm1::WCCImageClick(TObject *Sender)
{
 if (watbouwen!=WCC) //Luxe toilet bouwen
        {
        watbouwen=WCC;
        }
else watbouwen=0; //anders bouwen uitzetten
//voor rood vakje om gekozen plaatje
SelectImage->Picture->LoadFromFile("ToiletSelect.bmp");
welkegekozen(watbouwen,SelectImage,WCCImage);
//voorkomen dat spel hangt
welkecel=-1;
}
//---------------------------------------------------------------------------

void __fastcall TForm1::WCUImageClick(TObject *Sender)
{
 if (watbouwen!=WCU) //Urinoir bouwen
        {
        watbouwen=WCU;
        }
else watbouwen=0; //anders bouwen uitzetten
//voor rood vakje om gekozen plaatje
SelectImage->Picture->LoadFromFile("ToiletSelect2.bmp");
welkegekozen(watbouwen,SelectImage,WCUImage);
//voorkomen dat spel hangt
welkecel=-1;
}
//---------------------------------------------------------------------------


void __fastcall TForm1::WCSImageClick(TObject *Sender)
{
 if (watbouwen!=WCS) //Sink bouwen
        {
        watbouwen=WCS;
        }
else watbouwen=0; //anders bouwen uitzetten
//voor rood vakje om gekozen plaatje
SelectImage->Picture->LoadFromFile("ToiletSelect2.bmp");
welkegekozen(watbouwen,SelectImage,WCSImage);
//voorkomen dat spel hangt
welkecel=-1;
}
//---------------------------------------------------------------------------


void __fastcall TForm1::PaperImageClick(TObject *Sender)
{
//papier inkopen
if (VRD[0]<111)
        {
        VRD[0]+=10;
        money-=100;
        }

welkecel=-1;
}
//---------------------------------------------------------------------------

void __fastcall TForm1::TowelImageClick(TObject *Sender)
{
//towels inkopen
if (VRD[4]<31)
        {
        VRD[4]+=10;
        money-=200;
        }

welkecel=-1;
}
//---------------------------------------------------------------------------

void __fastcall TForm1::SoapImageClick(TObject *Sender)
{
//soap inkopen
if (VRD[2]<31)
        {
        VRD[2]+=10;
        money-=200;
        }

welkecel=-1;

}
//---------------------------------------------------------------------------


void __fastcall TForm1::OrderBtnClick(TObject *Sender)
{
//OrderPaneel openen
if (StockPanel->Visible==false)
     {
     BouwPanel->Hide();
     StockPanel->Show();
     watbouwen=0;
     Form1->Cursor=31;
     }
else
     {
     StockPanel->Hide();
     }

welkecel=-1;
}
//---------------------------------------------------------------------------

void __fastcall TForm1::BuildBtnClick(TObject *Sender)
{
//BouwPaneel openen
if (BouwPanel->Visible==false)
     {
     BouwPanel->Show();
     StockPanel->Hide();
     watbouwen=0;
     Form1->Cursor=30;
     }
else
     {
     BouwPanel->Hide();
     watbouwen=0;
     Form1->Cursor=31;
     }

welkecel=-1;     
}
//---------------------------------------------------------------------------


void __fastcall TForm1::PeoplemoveTimerTimer(TObject *Sender)
{
 //mensen genereren. Bepaald geslacht en leeftijd, geeft dan indexnummer 1;
  //randomizator, afhankelijk reputatie, alleen als open:
  if (close==1)
  if (random(2000)<(Rating*4+100))
      {
      //mensenarray doorlopen, eerste beschikbare positie wordt het:
        for (int tel=0; tel<maxmensen; tel++)
            {
              //als indexnummer 0 is:
              if (HUM[tel][4]==0)
                 {
                  //gelijke hoeveelheid mannen en vrouwen uitbraken
                     switch (welkgeslacht)
                        {
                        case 0:  //vrouw
                           HUM[tel][3]=welkgeslacht; //3 bevat geslacht
                           HUM[tel][2]=13; //2 bevat welk plaatje
                           //rij
                           HUM[tel][5]=rowwoman;
                           rowwoman++;
                           welkgeslacht=1;
                           break;
                        case 1:  //man
                           HUM[tel][3]=welkgeslacht; //3 bevat geslacht
                           HUM[tel][2]=7; //2 bevat welk plaatje
                           //rij
                           HUM[tel][5]=rowman;
                           rowman++;
                           welkgeslacht=0;
                           break;
                        }
                     //als kind:
                     if (random(2)==1)
                        {
                        HUM[tel][2]=1; //2 bevat welk plaatje
                        }
                  //in index
                  HUM[tel][4]=1;
                  //tevredenheid begin
                  HUM[tel][7]=10;
                  //uit forloop
                  tel=maxmensen+1;
                 }
            }
      }
   /*
for (int gr=0;gr<10;gr++)
   {
   for (int gr2=0; gr2<24; gr2++)
      {
       StringGrid1->Cells[gr][gr2]=HUM[gr2][gr];
      }

   }    */

//loopgrid. Voor mannen en vrouwen hetzelfde, maar mannen staan in andere rij,
//en gebruiken 6-11 en >18;
   for (int tel=0; tel<maxmensen; tel++)
      {
       switch (HUM[tel][4])
           {
           case 0: //niet tekenen
              {
              HUM[tel][2]=0;
              HUM[tel][0]=-36;
              HUM[tel][1]=1228;
              }
              break;
           case 1: //naar een positie uit beeld...
              {
              HUM[tel][0]=-36+HUM[tel][3]*38; //3 is voor mannen, die staan 1 positie schijnonderder
              HUM[tel][1]=1228+HUM[tel][3]*38;
              HUM[tel][4]=2; //1 op bij index
              }
              break;
           case 2:
              {
              //bestemming bepalen, in rij gaan staan:
              int DestX=648+HUM[tel][3]*38-HUM[tel][5]*19;
              int DestY=544+HUM[tel][3]*38+HUM[tel][5]*19;
              walkto(HUM,tel,DestX,DestY);
              //als je er bent index doorschruiven;
              if (HUM[tel][1]<DestY+1)
                 {
                 HUM[tel][4]=3; //1 op bij index
                 }
              }
              break;
           case 3:
              {
              //gewoon in rij staan, dus sta-plaatje
              switch (HUM[tel][2])
                  {
                  case 1:
                  case 3:
                     HUM[tel][2]=5;
                     break;
                  case 7:
                  case 9:
                     HUM[tel][2]=11;
                     break;
                  case 13:
                  case 15:
                     HUM[tel][2]=17;
                     break;
                  case 19:
                     HUM[tel][2]=5;
                     break;
                  case 20:
                     HUM[tel][2]=11;
                     break;
                  case 21:
                     HUM[tel][2]=17;
                     break;
                  }
              //zenuwachtig? laten zien + tevredenheid
              if (random(20)==0)
                       {
                       if (HUM[tel][7]>0) HUM[tel][7]--;
                       switch (HUM[tel][2])
                           {
                           case 5:
                               HUM[tel][2]=19;
                               break;
                           case 11:
                               HUM[tel][2]=20;
                               break;
                           case 17:
                               HUM[tel][2]=21;
                               break;
                           }
                       }
              else //deze else dient om te voorkomen dat geslacht verspringt
              if (HUM[tel][5]==0) //als vooraan in rij
              HUM[tel][4]=4; //1 op bij index
              }
              break;
           case 4:
              {
              //eerst vrouwen
              if (HUM[tel][3]==0)
                  {
                  int welke=-1;
                  //alle vrouwentoiletten af
                  for (int count=0; count<20; count++)
                      {
                      welke=random(8); //toilet kiezen
                      //kijken of het er een is
                      if (SAN[welke][2]==WCA+1||SAN[welke][2]==WCB+1||SAN[welke][2]==WCC+1)
                      //dat vrij is
                      if (SAN[welke][3]==-1)
                            {
                            //in HUM opslaan welk toilet(6)
                            HUM[tel][6]=welke;
                            //in SAN opslaan welke persoon
                            SAN[welke][3]=tel;
                            HUM[tel][4]=5;  //index
                            //wisselen van hoe je loopt
                            HUM[tel][2]-=1;
                            //rij door laten lopen
                            for (int tel2=0; tel2<maxmensen; tel2++)
                                {
                                if (HUM[tel2][3]==0&&tel2!=tel&&HUM[tel2][6]<0&&HUM[tel2][4]>0)
                                      {
                                      HUM[tel2][5]--;//rij 1 opschuiven
                                      if (HUM[tel2][5]<0) HUM[tel2][5]=0;
                                      HUM[tel2][4]=2;//index terug
                                      if(HUM[tel2][2]==5||HUM[tel2][2]==11||HUM[tel2][2]==17) HUM[tel2][2]-=2;
                                      }
                                }
                            rowwoman--;  if (rowwoman<0) rowwoman=0;
                            //uit count
                            count=20;
                            //ShowMessage (HUM[tel][4]);
                            }
                      }
                   }//einde vrouwen
              //dan mannen
              if (HUM[tel][3]==1)
                  {
                  int welke=-1;
                  //alle mannentoiletten af
                  for (int count=0; count<20; count++)
                      {
                      welke=random(24)+8; //toilet kiezen
                      //kijken of het er een is
                      if (SAN[welke][2]==WCA+1||SAN[welke][2]==WCB+1||SAN[welke][2]==WCC+1||SAN[welke][2]==WCU+1||SAN[welke][2]==WCUR+1)
                      //dat vrij is
                      if (SAN[welke][3]==-1)
                            {
                            //in HUM opslaan welk toilet(6)
                            HUM[tel][6]=welke;
                            //in SAN opslaan dat bezet
                            SAN[welke][3]=0;
                            HUM[tel][4]=5;  //index
                            //wisselen van hoe je loopt
                            HUM[tel][2]-=1;
                            //rij door laten lopen
                            for (int tel2=0; tel2<maxmensen; tel2++)
                                {
                                //man? niet betr persoon? //niet bezig? //wel actief?
                                if (HUM[tel2][3]==1&&tel2!=tel&&HUM[tel2][6]<0&&HUM[tel2][4]>0)
                                      {
                                      HUM[tel2][5]--;//rij 1 opschuiven
                                      if (HUM[tel2][5]<0) HUM[tel2][5]=0;
                                      HUM[tel2][4]=2;//index terug
                                      }
                                }
                            rowman--;  if (rowman<0) rowman=0;
                            //uit count
                            count=20;
                            //ShowMessage (HUM[tel][4]);
                            }
                      }
                   }//einde mannen
              }//einde case 3
              break;
           case 5:
              {
              //bepalen en naar goede plek lopen
              int welke=HUM[tel][6];
              //voor urinoirs/wasbakken:
              if (welke>15) welke-=16;
              int DestX=0;
              int DestY=0;
              if (welke<8)
                   {
                   DestX=306+welke*38;
                   DestY=220+welke*38;
                   }
              if (welke>7&&welke<16)
                   {
                   DestX=458+welke*38;
                   DestY=372+welke*38;
                   }
              walkto(HUM,tel,DestX,DestY);
              //als je er bent 1 verzetten
              if (HUM[tel][0]-DestX<9&&HUM[tel][0]-DestX>-9) HUM[tel][4]++;
              break;
              }//einde case 5
           case 6:
              {
              //toilet openen
              int welke=HUM[tel][6];
              if (HUM[tel][2]<7) SAN[welke][2]+=6; //kind
              else
              if (HUM[tel][2]<13) SAN[welke][2]+=1; //man
              else
              SAN[welke][2]+=11;  //vrouw
              HUM[tel][2]=0;//verdwijnen
              HUM[tel][4]++;//naar case 7;
              break;
              }//einde case 6
           case 7:
              {
              //toilet inlopen
              int welke=HUM[tel][6];
              SAN[welke][2]++;       //03
              HUM[tel][4]++;//naar case 8;
              break;
              }
           case 8:
              {
              //toilet verder inlopen
              int welke=HUM[tel][6];
              SAN[welke][2]++;      //04
              //tevredenheid type plee
              if (SAN[welke][2]==WCC+4||SAN[welke][2]==WCC+9||SAN[welke][2]==WCC+14) HUM[tel][7]+=30;
              if (SAN[welke][2]==WCB+4||SAN[welke][2]==WCB+9||SAN[welke][2]==WCB+14) HUM[tel][7]+=15;
              HUM[tel][4]++;//naar case 9;
              break;
              }
           case 9:
              {
              //Pee or Poo?
              int Wat=1;    //1=pee, 0=poo
              if (random(3)==2) Wat=0; //meer kans pee

              int welke=HUM[tel][6];
              SAN[welke][2]+=Wat+1;   //datgene doen
              //schoonmaak [4] en tevredenheid
              HUM[tel][7]+=SAN[welke][4]-45; //tevredenheid schoonmaak
              SAN[welke][4]-=random(15)+5; //vies worden
              if (SAN[welke][4]<0) SAN[welke][4]=0;
              //papier [5] en tevredenheid
              if (SAN[welke][5]!=9999) SAN[welke][5]-=(random(10)+12-(Wat*12)); //op maken
              if (SAN[welke][5]<0)
                    {
                    HUM[tel][7]-50; //tevredenheid papier
                    SAN[welke][5]=0;
                    }
              HUM[tel][4]+=Wat+1;//naar case 10(Poo) of 11(Pee);
              break;
              }
            case 10:
              {
              int welke=HUM[tel][6];
              if (SAN[welke][2]<WCU+1)
              //Poo, duurt lang tenzij urinoir
              //Slaat hij op in San[welke][3]:
                 {
                 SAN[welke][3]=random(20)+20;
                 }
                 else SAN[welke][3]=random(10)+10;

                 HUM[tel][4]=12;

              break;
              }
           case 11:
              {
              //Pee, duurt korter...
              //Slaat hij op in San[welke][3]:
              int welke=HUM[tel][6];
              SAN[welke][3]+=random(10)+10;
              HUM[tel][4]=13;
              break;
              }
           case 12:
              {
              //Afmaken Poo
              int welke=HUM[tel][6];
              SAN[welke][3]--;
              if (SAN[welke][3]==0) //klaar?
                 {
                 HUM[tel][4]=14;
                 SAN[welke][2]-=1;
                 }
              break;
              }
           case 13:
              {
              //Afmaken Pee
              int welke=HUM[tel][6];
              SAN[welke][3]--;
              if (SAN[welke][3]==0) //klaar?
                 {
                 HUM[tel][4]=14;
                 SAN[welke][2]-=2;
                 }
              break;
              }
           case 14:
              {
              //geluid afspelen bij toiletten:
              HUM[tel][4]=16;
              int welke=HUM[tel][6];
              if (SAN[welke][2]>WCA&&SAN[welke][2]<WCB)
              PlaySound("flush_0.wav",NULL, SND_ASYNC | SND_FILENAME) ;
              else
              if (SAN[welke][2]>WCB&&SAN[welke][2]<WCC)
              PlaySound("flush_1.wav",NULL, SND_ASYNC | SND_FILENAME) ;
              else
              if (SAN[welke][2]>WCC&&SAN[welke][2]<WCU)
              PlaySound("flush_2.wav",NULL, SND_ASYNC | SND_FILENAME) ;
              break;
              }

           case 16:
              {
              //uitlopen
              int welke=HUM[tel][6];
              SAN[welke][2]--;       //03
              HUM[tel][4]++;
              break;
              }
           case 17:
              {
              //verder uitlopen
              int welke=HUM[tel][6];
              SAN[welke][2]--;       //02
              HUM[tel][4]++;
              break;
              }
           case 18:
              {
              int welke=HUM[tel][6];
              //mens zichtbaar maken
              if (SAN[welke][2]==WCA+2||SAN[welke][2]==WCB+2||SAN[welke][2]==WCC+2||SAN[welke][2]==WCU+2||SAN[welke][2]==WCUR+2)
                  {//man
                  HUM[tel][2]=11;
                  }
              else
              if (SAN[welke][2]==WCA+7||SAN[welke][2]==WCB+7||SAN[welke][2]==WCC+7||SAN[welke][2]==WCU+7||SAN[welke][2]==WCUR+7)
                  {//kind
                  HUM[tel][2]=5;
                  }
              else
              if (SAN[welke][2]==WCA+12||SAN[welke][2]==WCB+12||SAN[welke][2]==WCC+12)
                  {//vrouw
                  HUM[tel][2]=17;
                  }
              //toilet sluiten
              if (SAN[welke][2]<WCB) SAN[welke][2]=WCA+1; //WCA
              else
              if (SAN[welke][2]<WCC) SAN[welke][2]=WCB+1; //WCB
              else
              if (SAN[welke][2]<WCU) SAN[welke][2]=WCC+1; //WCC
              else
              if (SAN[welke][2]<WCUR) SAN[welke][2]=WCU+1; //WCU
              else
              SAN[welke][2]=WCUR+1; //WCUR

              SAN[welke][3]=-1;//vrijgeven toilet
              HUM[tel][4]++;
              break;
              }
           case 19:
              {
              //Altijd weg tenzij wasbak
              HUM[tel][4]=98;
              //kans op basis geslacht en leeftijd:
              //kinderen 1op4, vrouwen 2op3 en mannen 1op3
              switch (HUM[tel][2])
                  {
                  case 5: //kinderen
                      if (random(4)==0)
                        {
                        HUM[tel][4]=20;
                        }
                      break;
                  case 11: //mannen
                      if (random(3)==0)
                        {
                        HUM[tel][4]=20;
                        }
                      break;
                  case 17: //Vrouwen
                      if (random(3)<2)
                        {
                        HUM[tel][4]=20;
                        }
                      break;
                  }
              break;
              }
           case 20:
              {
              //wassen: kijken of wasbak beschikbaar
              for (int count=0; count<30; count++)
                   {
                   //als count 29 is, dan is er geen gevonden: naar huis
                   if (count>28) HUM[tel][4]=98;
                   int welke=random(8)+16+HUM[tel][3]*8; //wasbak kiezen, hum[tel][3] voor mannen
                   //kijken of het er een is
                   if (SAN[welke][2]==WCS+1)
                   //dat vrij is
                   if (SAN[welke][3]==-1)
                            {
                            //in HUM opslaan welk toilet(6)
                            HUM[tel][6]=welke;
                            //in SAN opslaan welke persoon
                            SAN[welke][3]=tel;
                            HUM[tel][4]++;  //index
                            //uit for-loop
                            count=30;
                            //draaien
                            HUM[tel][2]-=3;
                            }
                   }
              break;
              }
           case 21:
              {
              //Naar wasbak toelopen
              int welke=HUM[tel][6];
              //voor urinoirs/wasbakken:
              if (welke>15) welke-=16;
              int DestX=0;
              int DestY=0;
              if (welke<8)
                   {
                   DestX=306+welke*38;
                   DestY=220+welke*38;
                   }
              if (welke>7&&welke<16)
                   {
                   DestX=458+welke*38;
                   DestY=372+welke*38;
                   }
              walkto(HUM,tel,DestX,DestY);
              //als je er bent 1 verzetten
              if (HUM[tel][0]-DestX<15&&HUM[tel][0]-DestX>-15) HUM[tel][4]++;
              break;
              }
           case 22:
              {
              //naar wasbak toegaan
              int welke=HUM[tel][6];
              if (HUM[tel][2]<7) SAN[welke][2]+=6; //kind
              else
              if (HUM[tel][2]<13) SAN[welke][2]+=1; //man
              else
              SAN[welke][2]+=11;  //vrouw
              HUM[tel][2]=0;//verdwijnen
              HUM[tel][4]++;//naar case 23;
              break;
              }//einde case 22
           case 23:
              {
              //naar wasbak toegaan
              int welke=HUM[tel][6];
              SAN[welke][2]++;       //03
              HUM[tel][4]++;//naar case 24;
              break;
              }
           case 24:
              {
              //wassen1
              int welke=HUM[tel][6];
              SAN[welke][2]++;       //04
              HUM[tel][4]++;//naar case 25;
              break;
              }
           case 25:
              {
              HUM[tel][4]++;//naar case 26;
              //tevredenheid
              //voor wasbak
              int welke=HUM[tel][6];
              HUM[tel][7]+=25;
            //soap [4] en tevredenheid
              SAN[welke][4]-=(random(10)+5); //op maken
              if (SAN[welke][4]<0)
                    {
                    HUM[tel][7]-10; //tevredenheid towels
                    SAN[welke][4]=0;
                    }
            //towels [5] en tevredenheid
              SAN[welke][5]-=(random(10)+8); //op maken
              if (SAN[welke][5]<0)
                    {
                    HUM[tel][7]-10; //tevredenheid towels
                    SAN[welke][5]=0;
                    }
              }
              break;
           case 26:
              HUM[tel][4]++;//naar case 27;
              break;
           case 27:
              {
              //wassen2
              int welke=HUM[tel][6];
              SAN[welke][2]++;
              HUM[tel][4]++;
              break;
              }
           case 28:
              HUM[tel][4]++;//naar case 29;
              break;
           case 29:
              {
              //wassen3
              int welke=HUM[tel][6];
              SAN[welke][2]++;
              HUM[tel][4]++;
              break;
              }
           case 30:
              if (random(3)==0) HUM[tel][4]++;//naar case 31;
              break;
           case 31:
              if (random(3)==0) HUM[tel][4]++;//naar case 32;
              break;
           case 32:
              if (random(3)==0) HUM[tel][4]++;//naar case 33;
              break;
           case 33:
              {
              //weggaan
              int welke=HUM[tel][6];
              SAN[welke][2]-=3;       //03
              HUM[tel][4]++;//naar case 34;
              break;
              }
           case 34:
              {
              //weggaan
              int welke=HUM[tel][6];
              SAN[welke][2]--;       //02
              HUM[tel][4]=40;//naar case 40;
              break;
              }
           case 40:
              {
              int welke=HUM[tel][6];
              //mens zichtbaar maken
              if (SAN[welke][2]==WCS+2)
                  {//man
                  HUM[tel][2]=11;
                  }
              else
              if (SAN[welke][2]==WCS+7)
                  {//kind
                  HUM[tel][2]=5;
                  }
              else
              if (SAN[welke][2]==WCS+12)
                  {//vrouw
                  HUM[tel][2]=17;
                  }
              //wasbak normaal sluiten
              SAN[welke][2]=WCS+1;
              SAN[welke][3]=-1;//vrijgeven wasbak
              HUM[tel][4]=98;
              break;
              }
           case 98:
              {
              //betalen
              if (HUM[tel][7]>5)
                   {
                   if (HUM[tel][7]<101) money+=HUM[tel][7];
                   else money+=120;
                   }
              else money+=5;

              HUM[tel][4]++;
              klantteller++; //klantteller 1 opvoeren
              //happyness in grid, rating berekenen:
              if (klantteller<20)
                      {
                      Happy[klantteller]=HUM[tel][7];
                      }
              else
                 {
                 for (int tel=0; tel<19; tel++)
                      {
                      Happy[tel]=Happy[tel+1];
                      }
                 Happy[19]=HUM[tel][7];
                 }
              //rating berekenen
              Rating=0;
              for (int tel=0; tel<20; tel++)
                   {
                   Rating+=Happy[tel];
                   }
              Rating=Rating/20;
              RatingEdit->Text=Rating;
              KlantEdit->Text=klantteller;
              //upgrade bepalen
              if (Rating>60&&Upgrade==0) Upgrade=1;
              else
              if (Rating>70&&Upgrade==1) Upgrade=2;
              else
              if (Rating>80&&Upgrade==2) Upgrade=3;
              //Downgrade
              if (Rating<70&&Upgrade==3) Upgrade=2;
              else
              if (Rating<60&&Upgrade==2) Upgrade=1;
              else
              if (Rating<50&&Upgrade==1) Upgrade=0;
              }
              break;
           case 99:
              {
              //weggaan, eerst naar uitgang
              HUM[tel][2]-=3;
              HUM[tel][4]++;
              break;
              }
           case 100:
              {
              int DestX=648+HUM[tel][3]*38;
              int DestY=544+HUM[tel][3]*38;
              walkto(HUM,tel,DestX,DestY);
              //als bij uitgang
              if (DestY-HUM[tel][1]<15&&DestY-HUM[tel][1]>-15)
                  {
                  HUM[tel][2]--;//mens draaien
                  HUM[tel][4]++;
                  }
              break;
              }
           case 101:
              {
              //uit beeld
              int DestX=-36+HUM[tel][3]*38;
              int DestY=1228+HUM[tel][3]*38;
              walkto(HUM,tel,DestX,DestY);
              //cijfers op 0: [4]=0,[2]=0,[5]=0,[6]=-1
              if (DestY-HUM[tel][1]<14&&DestY-HUM[tel][1]>-14)
                  {
                  HUM[tel][2]=0;
                  HUM[tel][4]=0;
                  HUM[tel][5]=0;
                  HUM[tel][6]=-1;
                  }
              break;
              }
           }//einde walkloop
      }

//Happyconloop
   for (int tel=0; tel<maxmensen; tel++)
           {
           HUM[tel][8]=0;
           if (HUM[tel][7]<11) HUM[tel][8]=1;
           if (HUM[tel][7]>10) HUM[tel][8]=2;
           if (HUM[tel][7]>50) HUM[tel][8]=3;
           if (HUM[tel][7]>75) HUM[tel][8]=4;
           }
}
//---------------------------------------------------------------------------

void __fastcall TForm1::SpeedImageClick(TObject *Sender)
{
welkecel=-1;

//bepaald snelheid in speed, 0=pauze, 1=normaal, 2=fast
switch (speed)
    {
    case 0:
       {
       SpeedImage->Picture->LoadFromFile("IconSteady.bmp");
       PeoplemoveTimer->Interval=350;
       speed=1;
       break;
       }
    case 1:
       {
       SpeedImage->Picture->LoadFromFile("IconFast.bmp");
       PeoplemoveTimer->Interval=200;
       speed=2;
       break;
       }
    case 2:
       {
       SpeedImage->Picture->LoadFromFile("IconPause.bmp");
       PeoplemoveTimer->Interval=0;
       speed=0;
       break;
       }
    }
}
//---------------------------------------------------------------------------

void __fastcall TForm1::SaveImageClick(TObject *Sender)
{
welkecel=-1;

//Opslaan
OpslaMemo->Clear(); //opslaan, op volgorde van declaratie

OpslaMemo->Lines->Add(MX);  //voor bewegen van scherm
OpslaMemo->Lines->Add(MY);  //Tekent hij op CanvasImage, plaatjes kunnen op de canvaslayer...
//De sanitair-array:
for (int tel=0;tel<32;tel++)
     {
     for (int tel2=0;tel2<6;tel2++)
          {
          OpslaMemo->Lines->Add(SAN[tel][tel2]);
          }
     }
//De mensen-array:
for (int tel=0;tel<maxmensen;tel++)
     {
     for (int tel2=0;tel2<9;tel2++)
          {
          OpslaMemo->Lines->Add(HUM[tel][tel2]);
          }
     }
//OpslaMemo->Lines->Add(maxmensen); //omvang array
OpslaMemo->Lines->Add(rowman);
OpslaMemo->Lines->Add(rowwoman);
OpslaMemo->Lines->Add(welkgeslacht);  /*
//Array die bepaald wat hij neerzet:
OpslaMemo->Lines->Add(watbouwen);
OpslaMemo->Lines->Add(WCA);
OpslaMemo->Lines->Add(WCB);
OpslaMemo->Lines->Add(WCC);
OpslaMemo->Lines->Add(WCU);
OpslaMemo->Lines->Add(WCS);
OpslaMemo->Lines->Add(WCUR);
//Integer die bepaald welke cel aangeraakt wordt
OpslaMemo->Lines->Add(welkecel);
//en een die aangeeft wat aangeraakt wordt: plee, Sink, Urinoir...
OpslaMemo->Lines->Add(watte);  */
//Array met geld
//Array met voorraad
OpslaMemo->Lines->Add(money);
for (int tel=0;tel<6;tel++)
     {
     OpslaMemo->Lines->Add(VRD[tel]);
     }   //bevat voorraad, 2 posities per artikel, 2e voor hoeveelheid dozen
for (int tel=0;tel<12;tel++)
     {
     for (int tel2=0;tel2<2;tel2++)
          {
          OpslaMemo->Lines->Add(VRDP[tel][tel2]);
          }
     }   //bevat info voor voorraadplaatjes
for (int tel=0;tel<4;tel++)
     {
     for (int tel2=0;tel2<2;tel2++)
          {
          OpslaMemo->Lines->Add(VRDS[tel][tel2]);
          }
     }
for (int tel=0;tel<4;tel++)
     {
     for (int tel2=0;tel2<2;tel2++)
          {
          OpslaMemo->Lines->Add(VRDT[tel][tel2]);
          }
     }
//Array met happyness, alsmede teller voor eerste 20 klanten
OpslaMemo->Lines->Add(klantteller);
for (int tel=0;tel<20;tel++)
     {
     OpslaMemo->Lines->Add(Happy[tel]);
     }
OpslaMemo->Lines->Add(Rating); //rating
OpslaMemo->Lines->Add(Upgrade);
//clean: welk plaatje op statuspaneel, clean2 is om te voorkomen dat de boel onrustig wordt...
/*
OpslaMemo->Lines->Add(clean);
OpslaMemo->Lines->Add(clean2);  */
//speed
//OpslaMemo->Lines->Add(speed);

//OpslaMemo->Lines->Add(OpslaMemo->Lines->Count);

//Controleregel
OpslaMemo->Lines->Add("Game Loaded");
OpslaMemo->Lines->SaveToFile("SaveGame.sav");
ShowMessage("Game Saved");

}
//---------------------------------------------------------------------------

void __fastcall TForm1::LoadImageClick(TObject *Sender)
{
welkecel=-1;

//laden
OpslaMemo->Clear();
OpslaMemo->Lines->LoadFromFile("SaveGame.sav");
//niet alles opslaan dus niet alles laden:
clean=666;
clean2=0;
watbouwen=0;
BouwPanel->Hide();
StockPanel->Hide();
SpeedImage->Picture->LoadFromFile("IconSteady.bmp");
PeoplemoveTimer->Interval=350;
speed=1;
close=1;
CloseButton->Picture->LoadFromFile("IconGo.bmp");

//laden
int R=0;
MX=OpslaMemo->Lines->Strings[R].ToInt();
R++;
MY=OpslaMemo->Lines->Strings[R].ToInt();
R++;
for (int tel=0;tel<32;tel++)
     {
     for (int tel2=0;tel2<6;tel2++)
          {
          SAN[tel][tel2]=OpslaMemo->Lines->Strings[R].ToInt();
          R++;
          }
     }
//De mensen-array:
for (int tel=0;tel<maxmensen;tel++)
     {
     for (int tel2=0;tel2<9;tel2++)
          {
          HUM[tel][tel2]=OpslaMemo->Lines->Strings[R].ToInt();
          R++;
          }
     }
//maxmensen
rowman=OpslaMemo->Lines->Strings[R].ToInt();
R++;
rowwoman=OpslaMemo->Lines->Strings[R].ToInt();
R++;
welkgeslacht=OpslaMemo->Lines->Strings[R].ToInt();
R++;
 /*
//Array die bepaald wat hij neerzet:
OpslaMemo->Lines->Add(watbouwen);
OpslaMemo->Lines->Add(WCA);
OpslaMemo->Lines->Add(WCB);
OpslaMemo->Lines->Add(WCC);
OpslaMemo->Lines->Add(WCU);
OpslaMemo->Lines->Add(WCS);
OpslaMemo->Lines->Add(WCUR);
//Integer die bepaald welke cel aangeraakt wordt
OpslaMemo->Lines->Add(welkecel);
//en een die aangeeft wat aangeraakt wordt: plee, Sink, Urinoir...
OpslaMemo->Lines->Add(watte);  */
money=OpslaMemo->Lines->Strings[R].ToInt();
R++;
for (int tel=0;tel<6;tel++)
     {
     VRD[tel]=OpslaMemo->Lines->Strings[R].ToInt();
     R++;
     }
for (int tel=0;tel<12;tel++)
     {
     for (int tel2=0;tel2<2;tel2++)
          {
          VRDP[tel][tel2]=OpslaMemo->Lines->Strings[R].ToInt();
          R++;
          }
     }
for (int tel=0;tel<4;tel++)
     {
     for (int tel2=0;tel2<2;tel2++)
          {
          VRDS[tel][tel2]=OpslaMemo->Lines->Strings[R].ToInt();
          R++;
          }
     }
for (int tel=0;tel<4;tel++)
     {
     for (int tel2=0;tel2<2;tel2++)
          {
          VRDT[tel][tel2]=OpslaMemo->Lines->Strings[R].ToInt();
          R++;
          }
     }

klantteller=OpslaMemo->Lines->Strings[R].ToInt();
R++;
for (int tel=0;tel<20;tel++)
     {
     Happy[tel]=OpslaMemo->Lines->Strings[R].ToInt();
     R++;
     }
Rating=OpslaMemo->Lines->Strings[R].ToInt();
R++;
Upgrade=OpslaMemo->Lines->Strings[R].ToInt();
R++;
//controleregel laten zien
ShowMessage(OpslaMemo->Lines->Strings[R]);
}
//---------------------------------------------------------------------------

void __fastcall TForm1::PaperIconImageDblClick(TObject *Sender)
{
if (dev==1) dev=2;
}
//---------------------------------------------------------------------------

void __fastcall TForm1::SoapIconImageClick(TObject *Sender)
{
if (dev==2) dev=3;
}
//---------------------------------------------------------------------------

void __fastcall TForm1::TowelIconImageClick(TObject *Sender)
{
if (dev==3) dev=4;
}
//---------------------------------------------------------------------------

void __fastcall TForm1::PeopleIconImageClick(TObject *Sender)
{
dev=1;        
}
//---------------------------------------------------------------------------

void __fastcall TForm1::HappyIconImageClick(TObject *Sender)
{
if (dev==4) money+=10000;
}
//---------------------------------------------------------------------------

void __fastcall TForm1::CloseButtonClick(TObject *Sender)
{
welkecel=-1;

if (close==0)//open gooien
       {
       close=1;
       CloseButton->Picture->LoadFromFile("IconGo.bmp");
       }
else       
if (close==1)//dicht gooien
       {
       close=0;
       CloseButton->Picture->LoadFromFile("IconClose.bmp");
       }
}
//---------------------------------------------------------------------------








