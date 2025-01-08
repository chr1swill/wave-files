#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#define DEBUG 1

#define DEBUG_PRINT_STR(str)                                                   \
  do {                                                                         \
    printf(#str "=");                                                          \
    fflush(0);                                                                 \
    write(0, (str), sizeof((*str)));                                           \
    printf("\n");                                                              \
  } while (0);

#define CHUNK_IDS                                                              \
  X(RIFF, 0x46464952)                                                          \
  X(WAVE, 0x45564157)                                                          \
  X(FMT, 0x20746D66)                                                           \
  X(FACT, 0x74636166)                                                          \
  X(DATA, 0x61746164)                                                          \
  X(CUE, 0x20657563)                                                           \
  X(PLST, 0x74736C70)                                                          \
  X(LABL, 0x6C62616C)                                                          \
  X(NOTE, 0x65746F6E)                                                          \
  X(SMPL, 0x6C706D73)                                                          \
  X(BEXT, 0x74786562)                                                          \
  X(IXML, 0x4C4D5869)                                                          \
  X(DS64, 0x34367364)

typedef enum {
#define X(name, value) name = value,
  CHUNK_IDS
#undef X
      INVALID_CHUNK_ID = 0x00000000,
} Chunk_Id;

Chunk_Id peek_chunk_type(char *data_buffer) {
  unsigned int chunk_id = *(unsigned int *)data_buffer;
  switch (chunk_id) {
#define X(name, value)                                                         \
  case (name):                                                                 \
    return (name);
    CHUNK_IDS
#undef X
  default:
    return INVALID_CHUNK_ID;
  }
}

typedef struct {
  unsigned int chunk_id;
  unsigned int chunk_size;
  unsigned int wave_id;
  // wave_chunk value is implictly
  // stored through a subtraction of
  // 4(bytes) from the chunk_size
  unsigned int wave_chunks;
} Riff_Chunk;

void parse_riff_chunk(char *data_start_p, Riff_Chunk *rc) {
  memset(rc, 0, sizeof(*rc));

  memmove(&rc->chunk_id, data_start_p, sizeof(rc->chunk_id));

  memmove(&rc->chunk_size, &data_start_p[4], sizeof(rc->chunk_size));
  rc->wave_chunks = rc->chunk_size;
  rc->chunk_size += 4;

  memmove(&rc->wave_id, &data_start_p[8], sizeof(rc->wave_id));
}

#define CODECS                                                                 \
  X(UNKNOWN, 0x0000)                                                           \
  X(PCM, 0x0001)                                                               \
  X(IEEE_FLOAT, 0x0003)                                                        \
  X(ALAW, 0x0006)                                                              \
  X(MULAW, 0x0007)                                                             \
  X(EXTENSIBLE, 0xFFFE)

typedef enum {
#define X(name, value) WAVE_FORMAT_##name = (value),

  CODECS
#undef X
} Codec;

typedef struct {
  unsigned int chunk_id;
  unsigned int chunk_size;
  unsigned short format_tag;
  unsigned short n_channels;
  unsigned int n_sample_per_sec;
  unsigned int n_avg_bytes_per_sec;
  unsigned short n_block_align;
  unsigned short w_bit_per_sample;
} Fmt_Chunk;

#define FMT_CHUNK_START 12

void parse_fmt_chunk(char *data_start_p, Fmt_Chunk *fc) {
  memset(fc, 0, sizeof(*fc));

  char *data = &data_start_p[FMT_CHUNK_START];
  memmove(&fc->chunk_id, data, sizeof(fc->chunk_id));

  data += sizeof(fc->chunk_id);
  memmove(&fc->chunk_size, data, sizeof(fc->chunk_size));

  data += sizeof(fc->chunk_size);
  memmove(&fc->format_tag, data, sizeof(fc->format_tag));

  data += sizeof(fc->format_tag);
  memmove(&fc->n_channels, data, sizeof(fc->n_channels));

  data += sizeof(fc->n_channels);
  memmove(&fc->n_sample_per_sec, data, sizeof(fc->n_sample_per_sec));

  data += sizeof(fc->n_sample_per_sec);
  memmove(&fc->n_avg_bytes_per_sec, data, sizeof(fc->n_avg_bytes_per_sec));

  data += sizeof(fc->n_avg_bytes_per_sec);
  memmove(&fc->n_block_align, data, sizeof(fc->n_block_align));

  data += sizeof(fc->n_block_align);
  memmove(&fc->w_bit_per_sample, data, sizeof(fc->w_bit_per_sample));
}

typedef struct {
  unsigned int chunk_id;
  unsigned int chunk_size;
  unsigned int dw_sample_length;
} Fact_Chunk;

#define FACT_CHUNK_START 38 // FMT_CHUNK_START + sizeof(Fmt_chunk)

void parse_fact_chunk(char *data_start_p, Fact_Chunk *fac) {
  memset(fac, 0, sizeof(*fac));

  char *data = &data_start_p[FACT_CHUNK_START];
  memmove(&fac->chunk_id, data, sizeof(fac->chunk_id));

  data += sizeof(fac->chunk_id);
  memmove(&fac->chunk_size, data, sizeof(fac->chunk_size));

  data += sizeof(fac->chunk_size);
  memmove(&fac->dw_sample_length, data, sizeof(fac->dw_sample_length));
}

int main(void) {
  int file_fd;
  int bytes_read;
  struct stat st;

  if ((file_fd = open("samples/tts-audio.wav", O_RDONLY)) == -1) {
    fprintf(stderr, "Error opening file: %s\n", strerror(errno));
    return 1;
  }

  if (fstat(file_fd, &st) == -1) {
    fprintf(stderr, "Error failed to stat fd %d: %s\n", file_fd,
            strerror(errno));
    close(file_fd);
    return 1;
  }

  char *file_buf = malloc(sizeof(char) * st.st_size);
  if (file_buf == NULL) {
    fprintf(stderr, "Error allocating memory for file buf of size: %lu\n",
            st.st_size);
    return 1;
  }

  if ((bytes_read = read(file_fd, file_buf, st.st_size)) == -1) {
    fprintf(stderr, "Error reading file to buffer: %s\n", strerror(errno));
    close(file_fd);
    return 1;
  } else if (bytes_read < st.st_size) {
    while (bytes_read < st.st_size) {
      if ((bytes_read += read(file_fd, &file_buf[bytes_read],
                              st.st_size - bytes_read)) == -1) {
        fprintf(stderr,
                "Error reading file, could only ready %d of %zu bytes of file: "
                "%s\n",
                bytes_read, st.st_size, strerror(errno));
        close(file_fd);
        return 1;
      }
    }
  }

  Riff_Chunk rc;
  parse_riff_chunk(file_buf, &rc);
  if (DEBUG) {
    DEBUG_PRINT_STR(&rc.chunk_id);
    printf("rc.chunk_id=%d\n", rc.chunk_id);
    assert(RIFF == rc.chunk_id);
    printf("rc.chunk_size=%d\n", rc.chunk_size);
    DEBUG_PRINT_STR(&rc.wave_id);
    assert(WAVE == rc.wave_id);
    printf("rc.wave_chunks=%d\n", rc.wave_chunks);
  }

  Fmt_Chunk fc;
  parse_fmt_chunk(file_buf, &fc);
  if (DEBUG) {
    DEBUG_PRINT_STR(&fc.chunk_id);
    assert(FMT == fc.chunk_id);
    printf("fc.chunk_size=%d\n", fc.chunk_size);
    printf("fc.format_tag=%d\n", fc.format_tag);

#define X(name, value) fc.format_tag == WAVE_FORMAT_##name ||
    assert(CODECS fc.format_tag == WAVE_FORMAT_UNKNOWN);
#undef X

    switch (fc.format_tag) {
    case WAVE_FORMAT_PCM:
      printf("format_tag=WAVE_FORMAT_PCM\n");
      break;
    case WAVE_FORMAT_IEEE_FLOAT:
      printf("format_tag=WAVE_FORMAT_IEEE_FLOAT\n");
      break;
    case WAVE_FORMAT_ALAW:
      printf("format_tag=WAVE_FORMAT_ALAW\n");
      break;
    case WAVE_FORMAT_MULAW:
      printf("format_tag=WAVE_FORMAT_MULAW\n");
      break;
    case WAVE_FORMAT_EXTENSIBLE:
      printf("format_tag=WAVE_FORMAT_EXTENSIBLE\n");
      break;
    }

    printf("fc.n_channels=%d\n", fc.n_channels);
    printf("fc.n_sample_per_sec=%d\n", fc.n_sample_per_sec);
    printf("fc.n_avg_bytes_per_sec=%d\n", fc.n_avg_bytes_per_sec);
    printf("fc.n_block_align=%d\n", fc.n_block_align);
    printf("fc.w_bit_per_sample=%d\n", fc.w_bit_per_sample);
  }

  Fact_Chunk fac;
  parse_fact_chunk(file_buf, &fac);
  if (DEBUG) {
    DEBUG_PRINT_STR(&fac.chunk_id);
    assert(FACT == fac.chunk_id);
    printf("fac.chunk_size=%d\n", fac.chunk_size);
    printf("fac.dw_sample_length=%d\n", fac.dw_sample_length);
  }

  free(file_buf);
  close(file_fd);
  return 0;
}
