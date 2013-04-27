#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <linux/soundcard.h>
# include "mad.h"

#define BUFF_SIZE   8192
struct mp3_file {
    int aaaa;
    int fd;     //文件描述符
    int sound_fd;   //声卡描述符
    int fpos;   //当前位置
    int flen;   //文件长度
    unsigned char* buffer;  //数据存储区域
};

unsigned int prerate = 0;

struct mp3_file* mp3 = NULL;

void setup_dps()
{
    int format = AFMT_S16_LE;
    int channels = 2;

    mp3->sound_fd = open("/dev/dsp", O_WRONLY);
    ioctl(mp3->sound_fd, SNDCTL_DSP_SETFMT, &format);
    ioctl(mp3->sound_fd, SNDCTL_DSP_CHANNELS, &channels);
}


static enum mad_flow input(void *data, struct mad_stream *stream)
{
    struct mp3_file *buffer = data;
    int unproc_data_size;
    int copy_size;

    //printf("tirgger: %s\n", __func__);
    if (!buffer->fpos) {
        return MAD_FLOW_STOP;
    } else {
        unproc_data_size = stream->bufend - stream->next_frame;

        //将不完整侦的数据拷贝到buffer最开始
        memcpy(buffer->buffer, buffer->buffer + BUFF_SIZE - unproc_data_size,
                    unproc_data_size);

        copy_size = BUFF_SIZE - unproc_data_size;

        if (buffer->fpos + copy_size > buffer->flen) {
            copy_size = buffer->flen - buffer->fpos;
            buffer->fpos = 0;
        } else {
            buffer->fpos += copy_size;
        }
        read(buffer->fd, buffer->buffer + unproc_data_size, copy_size);

        mad_stream_buffer(stream, buffer->buffer, BUFF_SIZE);
    }
    return MAD_FLOW_CONTINUE;
}

static inline
signed int scale(mad_fixed_t sample)
{
  /* round */
  sample += (1L << (MAD_F_FRACBITS - 16));

  /* clip */
  if (sample >= MAD_F_ONE)
    sample = MAD_F_ONE - 1;
  else if (sample < -MAD_F_ONE)
    sample = -MAD_F_ONE;

  /* quantize */
  return sample >> (MAD_F_FRACBITS + 1 - 16);
}


int writedsp(int c)
{
    return write(mp3->sound_fd, (char *)&c, 1);
}

static
enum mad_flow output(void *data,
  	     struct mad_header const *header,
		     struct mad_pcm *pcm)
{
  unsigned int nchannels, nsamples;
  mad_fixed_t const *left_ch, *right_ch;
  unsigned int rate;

  /* pcm->samplerate contains the sampling frequency */

    //printf("tirgger: %s\n", __func__);
  nchannels = pcm->channels;
  nsamples  = pcm->length;
  left_ch   = pcm->samples[0];
  right_ch  = pcm->samples[1];


  /* update the sample rate of dsp*/
  if(rate != prerate)
  {
      ioctl(mp3->sound_fd, SNDCTL_DSP_SPEED, &rate);
      prerate = rate;
  }

  while (nsamples--) {
    signed int sample;

    /* output sample(s) in 16-bit signed little-endian PCM */

    sample = scale(*left_ch++);
    writedsp((sample >> 0) & 0xff);
    writedsp((sample >> 8) & 0xff);

    if (nchannels == 2) {
      sample = scale(*right_ch++);
      writedsp((sample >> 0) & 0xff);
      writedsp((sample >> 8) & 0xff);
    }
  }

  return MAD_FLOW_CONTINUE;
}


static
enum mad_flow error(void *data,
		    struct mad_stream *stream,
		    struct mad_frame *frame)
{
  struct mp3_file *buffer = data;

  fprintf(stderr, "decoding error 0x%04x (%s) at byte offset %u\n",
	  stream->error, mad_stream_errorstr(stream),
	  stream->this_frame - buffer->buffer);
	  //stream->this_frame - buffer->buffer);

  return MAD_FLOW_CONTINUE;
}

int decode(struct mp3_file* mp3)
{
    struct mad_decoder decoder;
    int result;

    /* configure input, output, and error functions */
    mad_decoder_init(&decoder, mp3,
             input, 0 /* header */, 0 /* filter */, output,
             error, 0 /* message */);

    /* start decoding */
    result = mad_decoder_run(&decoder, MAD_DECODER_MODE_SYNC);

    /* release the decoder */
    mad_decoder_finish(&decoder);

  return result;

}

int main(int argc, char** argv)
{
    struct stat ptr;

    if (argc < 2) {
        printf("Useage: ./mp3play xxx.mp3\n");
        return -1;
    }

    mp3 = malloc(sizeof(struct mp3_file));
    if (mp3 == NULL) {
        printf("alloc memory failed\n");
        return -1;
    }

    mp3->fd = open(argv[1], O_RDONLY);
    if (mp3->fd < 0) {
        printf("opne %s failed\n", argv[1]);
        return -1;
    }

    fstat(mp3->fd, &ptr);
    mp3->flen = ptr.st_size;
    printf("mp3->flen:%d\n", mp3->flen);

    mp3->buffer = malloc(sizeof(unsigned char) * BUFF_SIZE);
    //mp3->buffer = malloc(sizeof(char) * mp3->flen);
    memset(mp3->buffer, 0, BUFF_SIZE);
    read(mp3->fd, mp3->buffer, BUFF_SIZE);

    printf("read over first buffer\n");

    mp3->fpos = BUFF_SIZE;
    setup_dps();

    decode(mp3);

    free(mp3->buffer);
    mp3->buffer = NULL;
    close(mp3->fd);
    close(mp3->sound_fd);
    return 0;
}
