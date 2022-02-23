#include "daq.h"
#include "util.h"

//#define FAKE_DAQ
#ifndef FAKE_DAQ

#include <Windows.h>
#include "Wd-dask64.h"
#include "wddaskex.h"

#define SCAN_INTERVAL 1

struct daq_t
{
  uint16_t card_id;
};

daq_t *daq_init(uint16_t card_type, uint16_t card_num)
{
  daq_t *daq = malloc(sizeof(daq_t));
  if ((daq->card_id = WD_Register_Card(card_type, card_num)) < 0)
  {
    FATAL_ERROR("Device not found",
                "Failed to register card type=%d num=%d. Error code: %d",
                card_type, card_num, daq->card_id);
    free(daq);
    return NULL;
  }
  LOG_FMT(DEBUG, "WD_Register_Card(%x, %d) returned %d",
          card_type, card_num, daq->card_id);

  int16_t err;
  if ((err = WD_AD_Auto_Calibration_ALL(daq->card_id)))
  {
    FATAL_ERROR("WD card error",
                "WD_AD_Auto_Calibration_ALL returned %d (card id=%d)",
                err, daq->card_id);
    free(daq);
    return NULL;
  }

  if ((err = WD_AI_CH_Config(daq->card_id, All_Channels, AD_B_1_V)))
  {
    FATAL_ERROR("WD card error",
                "WD_AI_CH_Config returned %d (card id=%d)",
                err, daq->card_id);
    free(daq);
    return NULL;
  }

  if ((err = WD_AI_Config(daq->card_id,
                          WD_ExtTimeBase,
                          TRUE,
                          WD_AI_ADCONVSRC_TimePacer,
                          FALSE,
                          TRUE)))
  {
    FATAL_ERROR("WD card error",
                "WD_AI_Config returned %d (card id=%d)",
                err, daq->card_id);
    free(daq);
    return NULL;
  }
  
  return daq;
}

void daq_destroy(daq_t *daq)
{
  LOG_FMT(DEBUG, "Releasing DAQ card (id=%d)", daq->card_id);
  WD_AI_ContBufferReset(daq->card_id);
  WD_Release_Card(daq->card_id);
  free(daq);
}

int daq_acquire(daq_t *daq, uint16_t *buffer,
                uint32_t samples_per_trig, uint32_t trig_count,
                bool async)
{
  uint16_t buffer_id;
  int16_t err;
  uint32_t buffer_size = samples_per_trig * trig_count;

  LOG_FMT(TRACE,
          "buffer = %p, samples/trig = %d, trig count = %d, buffer size = %d",
          (void *)buffer, samples_per_trig, trig_count, buffer_size);

  if ((err = WD_AI_Trig_Config(daq->card_id,
                               WD_AI_TRGMOD_POST,
                               WD_AI_TRGSRC_ExtD,
                               WD_AI_TrgPositive,
                               0, 0., 0, 0, 0,
                               trig_count)))
  {
    FATAL_ERROR("WD card error",
                "WD_AI_Trig_Config returned %d (card id=%d)",
                err, daq->card_id);
    return -1;
  }

  if ((err = WD_AI_ContBufferSetup(daq->card_id,
                                   buffer,
                                   buffer_size,
                                   &buffer_id)))
  {
    FATAL_ERROR("WD card error",
                "WD_AI_ContBufferSetup returned %d (card id=%d)",
                err, daq->card_id);
    return -1;
  }

  if ((err = WD_AI_ContScanChannels(daq->card_id,
                                    0,
                                    buffer_id,
                                    buffer_size,
                                    SCAN_INTERVAL,
                                    SCAN_INTERVAL,
                                    async ? ASYNCH_OP : SYNCH_OP)))
  {
    FATAL_ERROR("WD card error",
                "WD_AI_ContScanChannels returned %d (card id=%d)",
                err, daq->card_id);
    return -1;
  }

  return buffer_size;
}

int daq_await(daq_t *daq)
{
  BOOLEAN ready;
  uint32_t count, start_pos;
  do { WD_AI_AsyncCheck(daq->card_id, &ready, &count); } while (!ready);
  WD_AI_AsyncClear(daq->card_id, &start_pos, &count);

  return count;
}

#else

#include <stdio.h>

struct daq_t
{
  FILE *file;
};

daq_t *daq_init(uint16_t card_type, uint16_t card_num)
{
  daq_t *daq = malloc(sizeof(daq_t));

  const char *path = "data/test.dat";

  daq->file = fopen(path, "rb");
  if (daq->file == NULL)
  {
    FATAL_ERROR("Failed to open file", "Unable to open '%s'", path);
    free(daq);
    return NULL;
  }
  
  return daq;
}

void daq_destroy(daq_t *daq)
{
  fclose(daq->file);
  free(daq);
}

int daq_acquire(daq_t *daq, uint16_t *buffer,
                uint32_t samples_per_trig,
                uint32_t trig_count,
                bool async)
{
  uint32_t buffer_size = samples_per_trig * trig_count;

  LOG_FMT(TRACE,
          "buffer = %p, samples/trig = %d, trig count = %d, buffer size = %d",
          (void *)buffer, samples_per_trig, trig_count, buffer_size);

  size_t read = fread(buffer, sizeof(uint16_t), buffer_size, daq->file);
  LOG_FMT(TRACE, "Read %zu sample(s)", read);

  return read;
}

int daq_await(daq_t *daq)
{
  return 0;
}

#endif // FAKE_DAQ
