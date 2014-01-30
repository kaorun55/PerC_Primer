// 04_DepthCamera.cpp : コンソール アプリケーションのエントリ ポイントを定義します。
//

#include "stdafx.h"

#include <iostream>

// ヘッダーファイル
#include "util_pipeline.h"
#include <opencv2\opencv.hpp>

// UtilPipelineから継承したクラス
class Pipeline: public UtilPipeline {
protected:

  // 1. 定数、変数の宣言 
  //static const int Width = 640;
  //static const int Height = 480;
  static const int Width = 1280;
  static const int Height = 720;

  // Depthカメラの解像度
  static const int DEPTH_WIDTH = 320;   // 追加
  static const int DEPTH_HEIGHT = 240;  // 追加

  PXCImage::ColorFormat colorFormat;

public:

  // 2. コンストラクター
  Pipeline(void)
    : UtilPipeline()
    , colorFormat( PXCImage::COLOR_FORMAT_RGB32 )
  {
    // 必要なデータを有効にする
    EnableImage( colorFormat, Width, Height);
    EnableImage( PXCImage::COLOR_FORMAT_DEPTH, DEPTH_WIDTH, DEPTH_HEIGHT);
  }

  // 3. 新しいフレームの更新イベント
  virtual bool OnNewFrame(void)
  {
    try {
      // フレームを取得する
      auto colorFrame = QueryImage( PXCImage::IMAGE_TYPE_COLOR );
      auto depthFrame = QueryImage( PXCImage::IMAGE_TYPE_DEPTH );

      // フレームデータを取得する
      cv::Mat colorImage( Height, Width, CV_8UC4 );
      cv::Mat depthImage;
      getColorData( colorImage, colorFrame );
      getDepthData( depthImage, depthFrame );
      
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

  // Colorデータを取得する
  void getColorData( cv::Mat& colorImage, PXCImage* colorFrame )
  {
    // Colorデータを取得する
    PXCImage::ImageData data = { 0 };
    auto sts = colorFrame->AcquireAccess( PXCImage::ACCESS_READ,
                                          colorFormat, &data );
    if ( sts < PXC_STATUS_NO_ERROR ) {
      return;
    }

    // Colorデータを可視化する
    memcpy( colorImage.data, data.planes[0], data.pitches[0] * Height );

    // Colorデータを解放する
    colorFrame->ReleaseAccess( &data );
  }

  // 4. Depthカメラのデータを取得する
  void getDepthData( cv::Mat& depthImage, PXCImage* depthFrame )
  {
#if 0
    // Depthデータを取得する
    PXCImage::ImageData data = { 0 };
    auto sts = depthFrame->AcquireAccess( PXCImage::ACCESS_READ,
                                          PXCImage::COLOR_FORMAT_DEPTH, &data );
    if ( sts < PXC_STATUS_NO_ERROR ) {
      return;
    }

    // 8bitのGray画像を作成する
    depthImage = cv::Mat( DEPTH_HEIGHT, DEPTH_WIDTH, CV_8U );

    // Depthデータを可視化する
    ushort* srcDepth = (ushort*)data.planes[0];
    uchar* dstDepth = (uchar*)depthImage.data;
    for ( int i = 0; i < (DEPTH_WIDTH * DEPTH_HEIGHT); ++i ) {
      // 一定の距離のみ有効にする
      if ( (150 <= srcDepth[i]) && (srcDepth[i] < 800) ) {
        dstDepth[i] = srcDepth[i] * 0xFF / 1000;
      }
      else {
        dstDepth[i] = 0xFF;
      }
    }

    // Depthデータを解放する
    depthFrame->ReleaseAccess( &data );
#else
    // Depthデータを取得する
    PXCImage::ImageData data = { 0 };
    auto sts = depthFrame->AcquireAccess( PXCImage::ACCESS_READ,
                                          PXCImage::COLOR_FORMAT_RGB32, &data );
    if ( sts < PXC_STATUS_NO_ERROR ) {
      return;
    }

    // 32bitのRGB画像を作成する
    depthImage = cv::Mat( DEPTH_HEIGHT, DEPTH_WIDTH, CV_8UC4 );

    // Depthデータを可視化する
    memcpy( depthImage.data, data.planes[0], data.pitches[0] * DEPTH_HEIGHT );

    // Depthデータを解放する
    depthFrame->ReleaseAccess( &data );
#endif
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

