// 03_FaceDetection.cpp : コンソール アプリケーションのエントリ ポイントを定義します。
//

#include "stdafx.h"

#include <iostream>

#include "util_pipeline.h"
#include <opencv2\opencv.hpp>

class Pipeline: public UtilPipeline {
protected:

  //static const int Width = 640;
  //static const int Height = 480;
  static const int Width = 1280;
  static const int Height = 720;
  //static const int Width = 1920;
  //static const int Height = 1080;

  PXCImage::ColorFormat colorFormat;

public:

  Pipeline(void)
    : UtilPipeline()
    , colorFormat( PXCImage::COLOR_FORMAT_RGB32 )
  {
    // 必要なデータを有効にする
    EnableImage( colorFormat, Width, Height );

    // 顔の検出を有効にする
    EnableFaceLocation();
  }

  // 新しいフレーム
  virtual bool OnNewFrame(void)
  {
    try {
      // フレームを取得する
      auto colorFrame = QueryImage( PXCImage::IMAGE_TYPE_COLOR );
      auto faceFrame = QueryFace();

      // 取得したデータを表示する
      cv::Mat colorImage( Height, Width, CV_8UC4 );
      showColorFrame( colorImage, colorFrame );
      showFaceDetection( colorImage, faceFrame );
      
      // 画像を左右反転する
      //cv::flip( colorImage, colorImage, 1 );

      // 表示
      cv::imshow( "Color Camera", colorImage );
    }
    catch ( std::exception& ex ) {
      std::cout << ex.what() << std::endl;
    }

    auto key = cv::waitKey( 10 );
    return key != 'q';
  }

  // Colorデータを表示する
  void showColorFrame( cv::Mat& colorImage, PXCImage* colorFrame )
  {
    // Colorデータを取得する
    PXCImage::ImageData data = { 0 };
    auto sts = colorFrame->AcquireAccess( PXCImage::ACCESS_READ, colorFormat, &data );
    if ( sts < PXC_STATUS_NO_ERROR ) {
      return;
    }

    // Colorデータを可視化する
    memcpy( colorImage.data, data.planes[0], data.pitches[0] * Height );

    // Colorデータを解放する
    colorFrame->ReleaseAccess( &data );
  }

  // 検出した顔の位置を表示する
  void showFaceDetection( cv::Mat& colorImage, PXCFaceAnalysis* faceFrame )
  {
    auto detector = faceFrame->DynamicCast<PXCFaceAnalysis::Detection>();
    for ( int i = 0 ;; ++i ) {
      // ユーザーの顔IDを取得する
      pxcUID fid = 0; 
      pxcU64 timeStampe = 0;
      auto sts = faceFrame->QueryFace( i, &fid, &timeStampe );
      if ( sts < PXC_STATUS_NO_ERROR ) {
        break;
      }

      // 顔のデータを取得する
      PXCFaceAnalysis::Detection::Data data;
      detector->QueryData(fid,&data);

      // 顔の位置を描画する
      cv::rectangle( colorImage,
        cv::Rect( data.rectangle.x, data.rectangle.y, data.rectangle.w, data.rectangle.h ),
        cv::Scalar( 255, 0, 0 ) );
    }
  }
};

int _tmain(int argc, _TCHAR* argv[])
{
  try {
    Pipeline pipeline;
    pipeline.LoopFrames();
  }
  catch ( std::exception& ex ) {
    std::cout << ex.what() << std::endl;
  }

  return 0;
}

