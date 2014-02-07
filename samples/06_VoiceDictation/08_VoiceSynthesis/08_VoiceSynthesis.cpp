// 08_VoiceSynthesis.cpp : コンソール アプリケーションのエントリ ポイントを定義します。
//

#include "stdafx.h"

#include <iostream>

// 1.ヘッダーファイル
#include "util_pipeline.h"
#include "voice_out.h"
#pragma comment( lib, "winmm.lib" )

int _tmain(int argc, _TCHAR* argv[])
{
  // 2.wide-charactorを扱えるようにする
  std::locale::global(std::locale("japanese"));

  // 3. UtilPipelineクラスをそのまま利用する
  UtilPipeline pipeline;
  pipeline.Init();

  // 4. 音声合成の機能を利用可能にする
  PXCVoiceSynthesis* synthesis = nullptr;
  pipeline.QuerySession()->CreateImpl<PXCVoiceSynthesis>( &synthesis );

  // 5. 音声エンジンを設定する
  PXCVoiceSynthesis::ProfileInfo pinfo = { 0 };
  for ( int i = 0; ; ++i ) {
    auto ret = synthesis->QueryProfile( i, &pinfo );
    if ( ret != PXC_STATUS_NO_ERROR ) {
      break;
    }

    if ( pinfo.language == PXCVoiceRecognition::ProfileInfo::LANGUAGE_JP_JAPANESE ) {
      synthesis->SetProfile( &pinfo );
      std::cout << "日本語の音声エンジンを設定しました" << std::endl;
      break;
    }
  }

  // 6.テキストを音声化し、スピーカーから出力する
  VoiceOut voice( &pinfo );

  while ( 1 ) {
    std::cout << "入力してください：";
    std::wstring message;
    std::wcin >> message;

    // 音声合成のキューに入れる
    pxcUID id=0;
    synthesis->QueueSentence( (wchar_t*)message.c_str(), message.size(), &id );

    for (;;) {
      PXCSmartSP sp;
      PXCAudio *sample;

      // 音声合成を行う
      auto ret = synthesis->ProcessAudioAsync(id, &sample, &sp);
      if ( ret<PXC_STATUS_NO_ERROR ) {
        break;
      }

      ret = sp->Synchronize();
      if ( ret<PXC_STATUS_NO_ERROR ) {
        break;
      }

      // 音声データをスピーカーに出力する
      voice.RenderAudio(sample);
    }
  }

  return 0;
}

