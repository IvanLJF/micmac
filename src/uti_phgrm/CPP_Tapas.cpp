/*Header-MicMac-eLiSe-25/06/2007

    MicMac : Multi Image Correspondances par Methodes Automatiques de Correlation
    eLiSe  : ELements of an Image Software Environnement

    www.micmac.ign.fr


    Copyright : Institut Geographique National
    Author : Marc Pierrot Deseilligny
    Contributors : Gregoire Maillet, Didier Boldo.

[1] M. Pierrot-Deseilligny, N. Paparoditis.
    "A multiresolution and optimization-based image matching approach:
    An application to surface reconstruction from SPOT5-HRS stereo imagery."
    In IAPRS vol XXXVI-1/W41 in ISPRS Workshop On Topographic Mapping From Space
    (With Special Emphasis on Small Satellites), Ankara, Turquie, 02-2006.

[2] M. Pierrot-Deseilligny, "MicMac, un lociel de mise en correspondance
    d'images, adapte au contexte geograhique" to appears in
    Bulletin d'information de l'Institut Geographique National, 2007.

Francais :

   MicMac est un logiciel de mise en correspondance d'image adapte
   au contexte de recherche en information geographique. Il s'appuie sur
   la bibliotheque de manipulation d'image eLiSe. Il est distibue sous la
   licences Cecill-B.  Voir en bas de fichier et  http://www.cecill.info.


English :

    MicMac is an open source software specialized in image matching
    for research in geographic information. MicMac is built on the
    eLiSe image library. MicMac is governed by the  "Cecill-B licence".
    See below and http://www.cecill.info.

Header-MicMac-eLiSe-25/06/2007*/
#include "StdAfx.h"
#include <algorithm>



class cMemRes
{
   public :
        cMemRes()
        {
            mVAll.reserve(1000000);
        }

        void Init(int aSzMax,int aValInit)
        {
           double aMemTot = 0;
           for (int aExpSz=1 ; aExpSz<=aSzMax ; aExpSz++)
           {
               // int aNbAll = pow(1+aSzMax-aExpSz,4);
               int aNbAll = pow(1.6,aSzMax-aExpSz);
               int aSz = 1 << aExpSz;
               aMemTot += aSz * aNbAll;
               // std::cout << "Mem " << aMemTot << "\n";

               for (int aNb=0 ; aNb<aNbAll ; aNb++)
               {
                  char * aMem = (char *) malloc(aSz);
                  memset(aMem,aValInit,aSz);
                  mVAll.push_back(aMem);
               }
           }
        }
        void Free()
        {
           while (! mVAll.empty())
           {
              free(mVAll.back());
              mVAll.pop_back();
           }
       }
   private:
        std::vector<char *> mVAll;
};








/*
Parametre de Tapas :

   - calibration In : en base de donnees ou deja existantes.


*/

// bin/Tapioca MulScale "../micmac_data/ExempleDoc/Boudha/IMG_[0-9]{4}.tif" 300 -1 ExpTxt=1
// bin/Tapioca All  "../micmac_data/ExempleDoc/Boudha/IMG_[0-9]{4}.tif" -1  ExpTxt=1
// bin/Tapioca Line  "../micmac_data/ExempleDoc/Boudha/IMG_[0-9]{4}.tif" -1   3 ExpTxt=1
// bin/Tapioca File  "../micmac_data/ExempleDoc/Boudha/MesCouples.xml" -1  ExpTxt=1

#define DEF_OFSET -12349876



void Tapas_Banniere()
{
    std::cout <<  "\n";
    std::cout <<  " *********************************************\n";
    std::cout <<  " *     T-ool for                             *\n";
    std::cout <<  " *     A-erotriangulation                    *\n";
    std::cout <<  " *     P-hotogrammetric with                 *\n";
    std::cout <<  " *     A-pero                                *\n";
    std::cout <<  " *     S-implified (hopefully...)            *\n";
    std::cout <<  " *********************************************\n\n";
}


#define  NbModele 15

const char * Modele[NbModele] = {
                                   "RadialBasic",     // 0
                                   "RadialExtended",  // 1
                                   "Fraser",          // 2
                                   "FishEyeEqui",     // 3
                                   "AutoCal",         // 4
                                   "Figee",           // 5
                                   "HemiEqui",        // 6
                                   "RadialStd",       // 7
                                   "FraserBasic",     // 8
                                   "FishEyeBasic",    // 9
                                   "FE_EquiSolBasic",  // 10
                                   "Four7x2",          // 11
                                   "Four11x2",         // 12
                                   "Four15x2",         // 13
                                   "Four19x2"          // 14
                                };


std::string eModAutom;
int aDegRadMax = 100;
int aDegGenMax = 100;
std::string FileLibere;
double PropDiag = -1.0;


void ShowAuthorizedModel()
{
   std::cout << "\n";
   std::cout << "Authorized models : \n";
   for (int aKM=0 ; aKM<NbModele ; aKM++)
       std::cout << "   " << Modele[aKM] << "\n";
}

bool GlobLibAff = true;

bool GlobLibDec = true;
bool GlobLibPP  =true;
bool GlobLibCD=true;
bool GlobLibFoc=true;
int  GlobDRadMaxUSer = 100;
int  GlobDegGen = 100;


void InitVerifModele(const std::string & aMod,cInterfChantierNameManipulateur *)
{
    std::string  aModParam = aMod;


    int aKModele = -1;

    for (int aK=0 ; aK<NbModele ; aK++)
       if (aMod==Modele[aK])
         aKModele = aK;

    int  LocDegGen  = 100;
    bool LocLibDec = true;

    bool LocLibCD=true;
    int  LocDRadMaxUSer = 100;

    bool LocLibPP  =true;
    bool LocLibFoc=true;


    if (aMod==Modele[0])  // RadialBasic
    {
       eModAutom = "eCalibAutomRadialBasic";

       LocDegGen = 0;
       LocLibDec = false;
       LocDRadMaxUSer = 2;
       LocLibCD = false;
    }
    else if ((aMod==Modele[1]) ||  (aMod==Modele[7]))  // RadialExtended +  RadialStd
    {
       LocDegGen = 0;
       LocLibDec = false;
       LocDRadMaxUSer = (aMod==Modele[1]) ? 5 : 3;
       eModAutom = "eCalibAutomRadial";
    }
    else if (aMod==Modele[2])  //  Fraser
    {
        LocDegGen = 1;
        LocDRadMaxUSer = 3;
        eModAutom = "eCalibAutomPhgrStd";
    }
    else if ((aMod==Modele[3]) || (aMod==Modele[6]))   //   FishEyeEqui +  HemiEqui
    {
        LocDegGen = 2;
        LocDRadMaxUSer = 5;

        eModAutom = "eCalibAutomFishEyeLineaire";
        aModParam  = Modele[3];
        if (aMod==Modele[6])
        {
            if (PropDiag<0)
               PropDiag = 0.52;
        }
    }
    else if ((aMod==Modele[9]) || (aMod==Modele[10])) // "FishEyeBasic" +  "FE_EquiSolBasic"
    {
        LocDegGen = 1;
        LocDRadMaxUSer = 3;
        LocLibDec = false;

        eModAutom = "eCalibAutomFishEyeLineaire";
        if (aMod==Modele[10])
           eModAutom = "eCalibAutomFishEyeEquiSolid";
        aModParam  = Modele[9];
    }
    else if ((aMod==Modele[4]) || (aMod==Modele[5])) // AutoCal  +  Figee
    {
        if (aMod==Modele[5])
        {
           LocDegGen  = 0;
           LocLibDec = false;
           LocLibCD= false;
           LocDRadMaxUSer = 0;
           LocLibPP  =false;
           LocLibFoc=false;
        }
        eModAutom = "eCalibAutomNone";
    }
    else if (aMod==Modele[8])  //  FraserBasic
    {
        eModAutom = "eCalibAutomPhgrStdBasic";
        aModParam= Modele[2];
        LocDegGen = 1;
        LocLibDec = true;
        LocDRadMaxUSer = 3;
    }
    // Four7x2  => Four19x2
    else if (     (aMod==Modele[11])
              ||  (aMod==Modele[12])
              ||  (aMod==Modele[13])
              ||  (aMod==Modele[14])
            )
    {
        eModAutom = "eCalibAutom" + aMod;
        aModParam = "Four";

        LocDRadMaxUSer = 3 + (aKModele-11) * 2;
        LocDegGen = 1;
        LocLibDec = false;

        aDegRadMax =  3 + (aKModele-11) * 2;
        aDegGenMax = 2;
    }
    else
    {
        ShowAuthorizedModel();
        ELISE_ASSERT(false,"Value is not a correct model\n");
    }

    if (! EAMIsInit(&GlobLibDec))       GlobLibDec = LocLibDec;
    if (! EAMIsInit(&GlobLibPP ))       GlobLibPP = LocLibPP ;
    if (! EAMIsInit(&GlobLibCD ))       GlobLibCD = LocLibCD ;
    if (! EAMIsInit(&GlobLibFoc ))      GlobLibFoc = LocLibFoc ;
    if (! EAMIsInit(&GlobDRadMaxUSer )) GlobDRadMaxUSer = LocDRadMaxUSer ;
    if (! EAMIsInit(&GlobDegGen ))      GlobDegGen = LocDegGen ;

    FileLibere = "Param-"+aModParam+".xml";
}
/*
bool GlobLibFoc=true;
int  GlobDRadMaxUSer = 100;
int  GlobDegGen = 100;
*/


int Tapas_main(int argc,char ** argv)
{
    NoInit = "#@LL?~~XXXXXXXXXX";

    MMD_InitArgcArgv(argc,argv);

    std::string  aModele,aDir,aPat,aFullDir;
    int ExpTxt=0;
    int DoC=1;
    std::string AeroOut = "";
    std::string AeroIn  = NoInit;
    std::string CalibIn = NoInit;
    std::string ImInit  = NoInit;
    int   aVitesseInit=2;
    int   aDecentre = -1;
    Pt2dr Focales(-1,-1);
    Pt2dr aPPDec(-1,-1);
    std::string SauvAutom="";
    std::string aSetHom="";
    double  TolLPPCD;

    if ((argc>=2)  && (std::string(argv[1])==std::string("-help")))
    {
        ShowAuthorizedModel();
    }

    int FEAutom= 0;
    double SeuilFEAutom = -1;

    int IsForCalib = -1;


    bool MOI = false;
    int DBF = 0;


    std::string  aRapTxt;
    std::string  aPoseFigee="";
    bool Debug = false;


    ElInitArgMain
    (
        argc,argv,
        LArgMain()  << EAMC(aModele,"Calibration model",eSAM_None,ListOfVal(eTT_NbVals,"eTT_"))
                    << EAMC(aFullDir,"Full Directory (Dir+Pattern)", eSAM_IsPatFile),
        LArgMain()  << EAM(ExpTxt,"ExpTxt",true,"Export in text format (Def=false)",eSAM_IsBool)
                    << EAM(AeroOut,"Out",true, "Directory of Output Orientation", eSAM_IsOutputDirOri)
                    << EAM(CalibIn,"InCal",true,"Directory of Input Internal Orientation (Calibration)",eSAM_IsExistDirOri)
                    << EAM(AeroIn,"InOri",true,"Directory of Input External Orientation",eSAM_IsExistDirOri)
                    << EAM(DoC,"DoC",true,"Do Compensation", eSAM_IsBool)
                    << EAM(IsForCalib,"ForCalib",true,"Is for calibration (Change def value of LMV and prop diag)?")
                    << EAM(Focales,"Focs",true, "Keep images with focal length inside range [A,B] (A,B in mm) (Def=keep all)")
                    << EAM(aVitesseInit,"VitesseInit",true)
                    << EAM(aPPDec,"PPRel",true, "Principal point shift")
                    << EAM(aDecentre,"Decentre",true, "Principal point is shifted (Def=false)")
                    << EAM(PropDiag,"PropDiag",true, "Hemi-spherik fisheye diameter to diagonal ratio")
                    << EAM(SauvAutom,"SauvAutom",true, "Save intermediary results to", eSAM_IsOutputFile)
                    << EAM(ImInit,"ImInit",true, "Force first image", eSAM_IsExistFile)
                    << EAM(MOI,"MOI",true,"MOI", eSAM_IsBool)
                    << EAM(DBF,"DBF",true,"Debug (internal use : DebugPbCondFaisceau=true) ",eSAM_InternalUse)
                    << EAM(Debug,"Debug",true,"Partial file for debug", eSAM_InternalUse)
                    << EAM(GlobDRadMaxUSer,"DegRadMax",true,"Max degree of radial, default dependant of model")
                    << EAM(GlobLibAff,"LibAff",true,"Free affine parameter, Def=true", eSAM_IsBool)
                    << EAM(GlobLibDec,"LibDec",true,"Free decentric parameter, Def=true", eSAM_IsBool)
                    << EAM(GlobLibPP  ,"LibPP",true,"Free principal point, Def=true", eSAM_IsBool)
                    << EAM(GlobLibCD,"LibCP",true,"Free distorsion center, Def=true", eSAM_IsBool)
                    << EAM(GlobLibFoc,"LibFoc",true,"Free focal, Def=true", eSAM_IsBool)
                    << EAM(aRapTxt,"RapTxt",true, "RapTxt", eSAM_NoInit)
                    << EAM(TolLPPCD,"LinkPPaPPs",true, "Link PPa and PPs (double)", eSAM_NoInit)
                    << EAM(aPoseFigee,"FrozenPoses",true,"List of frozen poses (pattern)", eSAM_IsPatFile)
                    << EAM(aSetHom,"SH",true,"Set of Hom, Def=\"\", give MasqFiltered for result of HomolFilterMasq")
    );

    if (!MMVisualMode)
    {

        if ((AeroIn!= NoInit)  && (CalibIn==NoInit))
            CalibIn = AeroIn;

        #if (ELISE_windows)
            replace( aFullDir.begin(), aFullDir.end(), '\\', '/' );
        #endif
        SplitDirAndFile(aDir,aPat,aFullDir);
        setInputDirectory( aDir );

        if (AeroIn!= NoInit)
           StdCorrecNameOrient(AeroIn,aDir);

        if (CalibIn!= NoInit)
           StdCorrecNameOrient(CalibIn,aDir);


        cTplValGesInit<std::string> aTplN;
        cInterfChantierNameManipulateur * aICNM = cInterfChantierNameManipulateur::StdAlloc(0,0,aDir,aTplN);

                MakeXmlXifInfo(aFullDir,aICNM);



        if (FEAutom && (SeuilFEAutom<0))
           SeuilFEAutom = 16.5;

        if (IsForCalib<0)
            IsForCalib=(CalibIn==NoInit); // A Changer avec cle de calib

        double TetaLVM = IsForCalib ? 0.01 : 0.15;
        double CentreLVM = IsForCalib ? 0.1 : 1.0;
        double RayFEInit = IsForCalib ? 0.85 : 0.95;

    // std::cout << "IFCCCCC " << IsForCalib << " " << CentreLVM << " " << RayFEInit << "\n"; getchar();

        InitVerifModele(aModele,aICNM);

        if (PropDiag<0) PropDiag = 1.0;

        if (AeroOut=="")
           AeroOut = "" +  aModele;



       std::string aNameFileApero = Debug  ? "Apero-Debug-Glob.xml" : "Apero-Glob.xml" ;




       std::string aCom =     MM3dBinFile_quotes( "Apero" )
                           + ToStrBlkCorr( MMDir()+"include"+ELISE_CAR_DIR+"XML_MicMac"+ELISE_CAR_DIR+ aNameFileApero ) + " "
                           + std::string(" DirectoryChantier=") +aDir +  std::string(" ")
                           + std::string(" ") + QUOTE(std::string("+PatternAllIm=") + aPat) + std::string(" ")
                           //+ std::string(" +PatternAllIm=") + aPat + std::string(" ")
                           + std::string(" +AeroOut=-") + AeroOut
                           + std::string(" +Ext=") + (ExpTxt?"txt":"dat")
                           + std::string(" +ModeleCam=") + eModAutom
                           + std::string(" +FileLibereParam=") + FileLibere
                           + std::string(" DoCompensation=") + ToString(DoC)
                           + std::string(" +SeuilFE=") + ToString(SeuilFEAutom)
                           + std::string(" +TetaLVM=") + ToString(TetaLVM)
                           + std::string(" +CentreLVM=") + ToString(CentreLVM)
                           + std::string(" +RayFEInit=") + ToString(RayFEInit)
                           + std::string(" +CalibIn=-") + CalibIn
                           + std::string(" +AeroIn=-") + AeroIn
                           + std::string(" +VitesseInit=") + ToString(2+aVitesseInit)
                           + std::string(" +PropDiagU=") + ToString(PropDiag)
                           + std::string(" +ValDec=") + (GlobLibDec ?"eLiberte_Phgr_Std_Dec" : "eFige_Phgr_Std_Dec")
                           + std::string(" +ValDecPP=") + (GlobLibDec ?"eLiberte_Dec1" : "eLiberte_Dec0")
                           + std::string(" +ValAffPP=") + (GlobLibDec ?"eLiberteParamDeg_1" : "eLiberteParamDeg_0")
                           + std::string(" +ValAff=") + (GlobLibAff ?"eLiberte_Phgr_Std_Aff" : "eFige_Phgr_Std_Aff")

                        ;

    if (EAMIsInit(&GlobLibPP  ))
       aCom = aCom + std::string(" +DoPP=") + ToString(GlobLibPP  ) + std::string(" ");
    if (EAMIsInit(&GlobLibCD))
       aCom = aCom + std::string(" +DoCD=") + ToString(GlobLibCD) + std::string(" ");
    if (EAMIsInit(&GlobLibFoc))
       aCom = aCom + std::string(" +DoFoc=") + ToString(GlobLibFoc) + std::string(" ");


        StdCorrecNameHomol(aSetHom,aDir);
        if (EAMIsInit(&aSetHom))
        {
            aCom = aCom + std::string(" +SetHom=") + aSetHom;
        }


        if (EAMIsInit(&GlobDRadMaxUSer))
           aDegRadMax = GlobDRadMaxUSer;

        if (aDegRadMax<100)
           aCom = aCom +  std::string(" +DegRadMax=") + ToString(aDegRadMax) + std::string(" ");


        if (EAMIsInit(&GlobLibAff) && (!GlobLibAff))
        {
              aCom = aCom + " +LiberteAff=false ";
        }

        if (EAMIsInit(&TolLPPCD))
           aCom = aCom + " +TolLinkPPCD=" + ToString(TolLPPCD);

       if (aRapTxt!="")
          aCom = aCom +  std::string(" +RapTxt=") + aRapTxt;

       if (DBF)
         aCom  = aCom + " DebugPbCondFaisceau=true";


       if (ImInit!=NoInit)
       {
             aCom =   aCom + " +SetImInit="+ImInit;
             aCom = aCom + " +FileCamInit=InitCamSpecified.xml";
             ELISE_ASSERT(AeroIn==NoInit,"Incoherence AeroIn/ImInit");
       }
       if (SauvAutom!="")
         aCom =   aCom + " +SauvAutom="+SauvAutom;

       if (AeroIn!= NoInit)
          aCom =   aCom
                 + " +FileCamInit=InitCamBDD.xml" ;

       if (Focales.x>=0)
          aCom =   aCom
                 + " +FocMin=" + ToString(Focales.x)
                 + " +FocMax=" + ToString(Focales.y);

       if (aPPDec.x>=0)
           aCom =   aCom + " +xPRelPP=" + ToString(aPPDec.x);
       else
          aPPDec.x =0.5;
       if (aPPDec.y>=0)
           aCom =   aCom + " +yPRelPP=" + ToString(aPPDec.y);
       else
          aPPDec.y =0.5;

       if (aDecentre==-1)
       {
            double aDist = euclid(aPPDec,Pt2dr(0.5,0.5));
            aDecentre= (aDist>=0.25);
       }

       if (aDecentre)
       {
            aCom  = aCom + " +ModeCDD=eCDD_OnRemontee";
       }

       if (MOI)
       {
            aCom  = aCom + " +MOI=true";
       }

       if (aPoseFigee!="")
       {
          aCom  = aCom + " +PoseFigee=" + QUOTE(aPoseFigee);
       }

       // std::cout << "Com = " << aCom << "\n";
       int aRes = 0;
       aRes = TopSystem(aCom);
    /*
       if (MajickTest)
       {
            std::string aNameFile = MMDir() + "DbgAp" + GetUnikId() + ".txt";

            // cMemRes aMR1;
            // cMemRes aMR2;

             aCom = aCom + " +FileDebug=" +  aNameFile;


            for (int aTest=0 ; aTest < 1000000 ; aTest++)
            {

               // int aValInit = (aTest % 17);
               // int aSzMax = 29;
               // aMR1.Init(aSzMax,aValInit);
               // aMR2.Init(aSzMax,aValInit);

               // aMR2.Free();
               aRes = ::System(aCom.c_str(),true,true);
               // aMR1.Free();



               sleep(1); // Pour faciliter l'arret
            }

       }
       else
       {
           aRes = ::System(aCom.c_str(),false,true,true);
       }
    */


       Tapas_Banniere();
       BanniereMM3D();


       return aRes;
   }
   else
       return EXIT_SUCCESS;
}





/*Footer-MicMac-eLiSe-25/06/2007

Ce logiciel est un programme informatique servant �  la mise en
correspondances d'images pour la reconstruction du relief.

Ce logiciel est régi par la licence CeCILL-B soumise au droit français et
respectant les principes de diffusion des logiciels libres. Vous pouvez
utiliser, modifier et/ou redistribuer ce programme sous les conditions
de la licence CeCILL-B telle que diffusée par le CEA, le CNRS et l'INRIA
sur le site "http://www.cecill.info".

En contrepartie de l'accessibilité au code source et des droits de copie,
de modification et de redistribution accordés par cette licence, il n'est
offert aux utilisateurs qu'une garantie limitée.  Pour les mêmes raisons,
seule une responsabilité restreinte pèse sur l'auteur du programme,  le
titulaire des droits patrimoniaux et les concédants successifs.

A cet égard  l'attention de l'utilisateur est attirée sur les risques
associés au chargement,  �  l'utilisation,  �  la modification et/ou au
développement et �  la reproduction du logiciel par l'utilisateur étant
donné sa spécificité de logiciel libre, qui peut le rendre complexe �
manipuler et qui le réserve donc �  des développeurs et des professionnels
avertis possédant  des  connaissances  informatiques approfondies.  Les
utilisateurs sont donc invités �  charger  et  tester  l'adéquation  du
logiciel �  leurs besoins dans des conditions permettant d'assurer la
sécurité de leurs systèmes et ou de leurs données et, plus généralement,
�  l'utiliser et l'exploiter dans les mêmes conditions de sécurité.

Le fait que vous puissiez accéder �  cet en-tête signifie que vous avez
pris connaissance de la licence CeCILL-B, et que vous en avez accepté les
termes.
Footer-MicMac-eLiSe-25/06/2007*/
