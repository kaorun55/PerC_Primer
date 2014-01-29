// 04_DepthCamera.cpp : コンソール アプリケーションのエントリ ポイントを定義します。
//

#include "stdafx.h"

#include <iostream>

#include "util_pipeline.h"
#include <opencv2\opencv.hpp>

class Pipeline: public UtilPipeline {
public:

  Pipeline(void)
    : UtilPipeline()
    , colorFormat( PXCImage::COLOR_FORMAT_RGB32 )
  {
    // 必要なデータを有効にする
    EnableGesture();
    EnableImage( colorFormat, Width, Height);
  }

  // 新しいフレーム
  virtual bool OnNewFrame(void)
  {
    try {
      // フレームを取得する
      auto colorFrame = QueryImage( PXCImage::IMAGE_TYPE_COLOR );
      auto depthFrame = QueryImage( PXCImage::IMAGE_TYPE_DEPTH );

      // 取得したデータを表示する
      cv::Mat colorImage( Height, Width, CV_8UC4 );
      cv::Mat depthImage( DEPTH_HEIGHT, DEPTH_WIDTH, CV_8U );
      showColorFrame( colorImage, colorFrame );
      showDepthFrame( depthImage, depthFrame );
      
      // 画像を左右反転する
      //cv::flip( colorImage, colorImage, 1 );

      // 表示
      cv::imshow( "Color Camera", colorImage );
      cv::imshow( "Depth Camera", depthImage );
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

    // Colorデータを取得する
    memcpy( colorImage.data, data.planes[0], data.pitches[0] * Height );

    // Colorデータを解放する
    colorFrame->ReleaseAccess( &data );
  }

  // Depthデータを表示する
  void showDepthFrame( cv::Mat& depthImage, PXCImage* depthFrame )
  {
    // Depthデータを取得する
    PXCImage::ImageData data = { 0 };
    auto sts = depthFrame->AcquireAccess( PXCImage::ACCESS_READ, PXCImage::COLOR_FORMAT_DEPTH, &data );
    if ( sts < PXC_STATUS_NO_ERROR ) {
      return;
    }

    // Depthデータを可視化する
    ushort* srcDepth = (ushort*)data.planes[0];
    uchar* dstDepth = (uchar*)depthImage.data;
    for ( int i = 0; i < DEPTH_WIDTH * DEPTH_HEIGHT; ++i ) {
      if ( (150 <= srcDepth[i]) && (srcDepth[i] < 800) ) {
        dstDepth[i] = srcDepth[i] * 0xFF / 1000;
      }
      else {
        dstDepth[i] = 0xFF;
      }
    }

    // Depthデータを解放する
    depthFrame->ReleaseAccess( &data );
  }

protected:

  static const int Width = 640;
  static const int Height = 480;
  //static const int Width = 1280;
  //static const int Height = 720;

  static const int DEPTH_WIDTH = 320;
  static const int DEPTH_HEIGHT = 240;

  PXCImage::ColorFormat colorFormat;
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

