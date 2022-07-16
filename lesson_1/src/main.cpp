//
// Created by hkx on 22-7-16.
//
#ifdef __cplusplus
extern "C" {
#endif

#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>

#ifdef __cplusplus
} // endof extern "C"
#endif

#include <cstdio>
#include <cstring>
#include <cinttypes>

int main(int argc, const char *argv[]){

  //检查输入参数
  if(argc < 2){
    printf("请输入解码文件路径");
    return -1;
  }

  //创建 AVFormatContext
  AVFormatContext *pFormatContext = avformat_alloc_context();
  if (!pFormatContext) {
    printf("ERROR could not allocate memory for Format Context");
    return -1;
  }

  //加载媒体文件到 AVFormatContext
  if (avformat_open_input(&pFormatContext, argv[1], NULL, NULL) != 0) {
    printf("ERROR could not open the file");
    return -1;
  }

  //输出文件信息
  printf("format %s, duration %ld us, bit_rate %ld",
          pFormatContext->iformat->name,
          pFormatContext->duration,
          pFormatContext->bit_rate);

  // 输出码流信息
  printf("AVStream->time_base before open coded %d/%d",
          pFormatContext->streams[0]->time_base.num,
          pFormatContext->streams[0]->time_base.den);
  printf("AVStream->r_frame_rate before open coded %d/%d",
          pFormatContext->streams[0]->r_frame_rate.num,
          pFormatContext->streams[0]->r_frame_rate.den);
  printf("AVStream->start_time %"
          PRId64, pFormatContext->streams[0]->start_time);
  printf("AVStream->duration %"
          PRId64, pFormatContext->streams[0]->duration);

  //读取流数据（假定媒体文件中只有一个视频流）codec信息到AVCodecParameters结构体
  AVCodecParameters *pCodecParameters = NULL;
  pCodecParameters = pFormatContext->streams[0]->codecpar;

  //查找编码流对应的解码器(软件解码器)
  const AVCodec *pCodec = avcodec_find_decoder(pCodecParameters->codec_id);

  //创建解码器context并分配内存
  AVCodecContext *pCodecContext = avcodec_alloc_context3(pCodec);
  if (!pCodecContext) {
    printf("failed to allocated memory for AVCodecContext");
    return -1;
  }

  //根据码流codec参数设置解码器
  if (avcodec_parameters_to_context(pCodecContext, pCodecParameters) < 0) {
  printf("failed to copy codec params to codec context");
  return -1;
  }

  //打开解码器
  if (avcodec_open2(pCodecContext, pCodec, NULL) < 0) {
    printf("failed to open codec through avcodec_open2");
    return -1;
  }

  //创建/分配 码流packet和解码帧frame的 引用/内存空间
  AVFrame *pFrame = av_frame_alloc();
  if (!pFrame) {
    printf("failed to allocate memory for AVFrame");
    return -1;
  }

  AVPacket *pPacket = av_packet_alloc();
  if (!pPacket) {
    printf("failed to allocate memory for AVPacket");
    return -1;
  }

  //循环读取码流至EOF
  while (av_read_frame(pFormatContext, pPacket) >= 0) {
    avcodec_send_packet(pCodecContext, pPacket);
    avcodec_receive_frame(pCodecContext, pFrame);
    printf(
        "Frame %d (type=%c, size=%d bytes, format=%d) pts %ld key_frame %d [DTS %d] \n",
        pCodecContext->frame_number,
        av_get_picture_type_char(pFrame->pict_type),
        pFrame->pkt_size,
        pFrame->format,
        pFrame->pts,
        pFrame->key_frame,
        pFrame->coded_picture_number
    );
    av_packet_unref(pPacket);
  }
  avformat_close_input(&pFormatContext);
  av_packet_free(&pPacket);
  av_frame_free(&pFrame);
  avcodec_free_context(&pCodecContext);
}