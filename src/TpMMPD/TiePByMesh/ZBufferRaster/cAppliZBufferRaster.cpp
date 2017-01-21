#include "ZBufferRaster.h"

cAppliZBufferRaster::cAppliZBufferRaster(
                                            cInterfChantierNameManipulateur * aICNM,
                                            const std::string & aDir,
                                            const std::string & anOri,
                                            vector<cTri3D> & aVTri,
                                            vector<string> & aVImg
                                        ):
    mICNM (aICNM),
    mDir  (aDir),
    mOri  (anOri),
    mVTri (aVTri),
    mVImg (aVImg),
    mNInt (0),
    mW    (0),
    mSzW  (Pt2di(500,500)),
    mReech(1.0),
    mDistMax (TT_DISTMAX_NOLIMIT),
    mIsTmpZBufExist (ELISE_fp::IsDirectory(aDir + "Tmp-ZBuffer/"))
{    
    if ( !mIsTmpZBufExist)
    {
        ELISE_fp::MkDir(mDir + "Tmp-ZBuffer/");
    }
}

void cAppliZBufferRaster::SetNameMesh(string & aNameMesh)
{
    mNameMesh = aNameMesh;
    if (!ELISE_fp::IsDirectory(mDir + "Tmp-ZBuffer/" + aNameMesh))
        ELISE_fp::MkDir(mDir + "Tmp-ZBuffer/" + aNameMesh);
}



void  cAppliZBufferRaster::DoAllIm(vector<vector<bool> > & aVTriValid)
{
    for (int aKIm=0; aKIm<int(mVImg.size()); aKIm++)
    {
        string path = mDir + "Tmp-ZBuffer/" + mNameMesh + "/" + mVImg[aKIm] + "/";
        string fileOutZBuf = path + mVImg[aKIm] + "_ZBuffer_DeZoom" + ToString(int(1.0/mReech)) + ".tif";
        string fileOutLbl = path + mVImg[aKIm] + "_TriLabel_DeZoom" +  ToString(int(1.0/mReech)) + ".tif";
        ElTimer aChrono;
        cImgZBuffer * aZBuf;
       if ( ELISE_fp::exist_file(fileOutLbl))
       {
           cout<<mVImg[aKIm]<<" existed in Tmp-ZBuffer ! . Skip"<<endl;
           aZBuf =  new cImgZBuffer(this, mVImg[aKIm]);
           aZBuf->ImportResult(fileOutLbl, fileOutLbl);
           mIsTmpZBufExist = true;
       }
       else
       {
           aZBuf =  new cImgZBuffer(this, mVImg[aKIm]);
           for (int aKTri=0; aKTri<int(mVTri.size()); aKTri++)
           {
               if (aKTri % 200 == 0 && mNInt != 0)
                   cout<<"["<<(aKTri*100.0/mVTri.size())<<" %]"<<endl;
               aZBuf->LoadTri(mVTri[aKTri]);
           }
           //save Image ZBuffer to disk
           ELISE_fp::MkDir(path);
           ELISE_COPY
                   (
                       aZBuf->ImZ().all_pts(),
                       aZBuf->ImZ().in_proj(),
                       Tiff_Im(
                           fileOutZBuf.c_str(),
                           aZBuf->ImZ().sz(),
                           GenIm::real8,
                           Tiff_Im::No_Compr,
                           aZBuf->Tif().phot_interp()
                           ).out()

                       );
           ELISE_COPY
                   (
                       aZBuf->ImInd().all_pts(),
                       aZBuf->ImInd().in_proj(),
                       Tiff_Im(
                           fileOutLbl.c_str(),
                           aZBuf->ImInd().sz(),
                           GenIm::real8,
                           Tiff_Im::No_Compr,
                           aZBuf->Tif().phot_interp()
                           ).out()

                       );
       }
       aVTriValid.push_back(aZBuf->TriValid());
       if (mNInt != 0)
       {
           cout<<"Im "<<mVImg[aKIm]<<endl;
           cout<<"Finish Img Cont.. - Nb Tri Traiter : "<<aZBuf->CntTriTraite()<<" -Time: "<<aChrono.uval()<<endl;

           //=======================================
           aZBuf->normalizeIm(aZBuf->ImZ(), 0.0, 255.0);
           aZBuf->normalizeIm(aZBuf->ImInd(), 0.0, 255.0);

           if (aZBuf->ImZ().sz().x >= aZBuf->ImZ().sz().y)
           {
               double scale =  double(aZBuf->ImZ().sz().x) / double(aZBuf->ImZ().sz().y) ;
               mSzW = Pt2di(mSzW.x , round_ni(mSzW.x/scale));
           }
           else
           {
               double scale = double(aZBuf->ImZ().sz().y) / double(aZBuf->ImZ().sz().x);
               mSzW = Pt2di(round_ni(mSzW.y/scale) ,mSzW.y);
           }
           Pt2dr aZ(double(mSzW.x)/double(aZBuf->ImZ().sz().x) , double(mSzW.y)/double(aZBuf->ImZ().sz().y) );

           if (mW ==0)
           {
               mW = Video_Win::PtrWStd(mSzW, true, aZ);
               mW->set_sop(Elise_Set_Of_Palette::TheFullPalette());
               if (mWithImgLabel)
               {
                   if (mWLbl ==0)
                   {
                       mWLbl = Video_Win::PtrWStd(mSzW, true, aZ);
                       mWLbl->set_sop(Elise_Set_Of_Palette::TheFullPalette());
                   }
               }
           }

           if (mW)
           {
               mW->set_title( (mVImg[aKIm] + "_ZBuf").c_str());
               ELISE_COPY(   aZBuf->ImZ().all_pts(),
                             aZBuf->ImZ().in(),
                             mW->ogray()
                             );
               mW->clik_in();
           }

           if (mWithImgLabel && mWLbl)
           {

               mWLbl->set_title( (mVImg[aKIm] + "_Label").c_str());
               ELISE_COPY(   aZBuf->ImInd().all_pts(),
                             aZBuf->ImInd().in(),
                             mWLbl->ogray()
                             );

           }
           getchar();
       }
       for (double i=0; i<aZBuf->TriValid().size(); i++)
       {
           if(aZBuf->TriValid()[i] == true)
               aZBuf->CntTriValab()++;
       }
       cout<<"Nb Tri In ZBuf : "<<aZBuf->CntTriValab()<<endl;
    }

}

void cAppliZBufferRaster::DoAllIm()
{
    DoAllIm(mTriValid);
}


