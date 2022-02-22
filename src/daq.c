#include "daq.h"
#include "util.h"

#include <Windows.h>
#include "Wd-dask64.h"
#include "wddaskex.h"

#define SCAN_INTERVAL 1

struct daq_t
{
  uint16_t card_id;
};

int daq_init(daq_t *daq, uint16_t card_type, uint16_t card_num)
{
  if ((daq->card_id = WD_Register_Card(card_type, card_num)) < 0)
  {
    FATAL_ERROR("Device not found",
                "Failed to register card type=%d num=%d. Error code: %d",
                card_type, card_num, daq->card_id);
    return -1;
  }
  LOG_FMT(DEBUG, "WD_Register_Card(%x, %d) returned %d",
          card_type, card_num, daq->card_id);

  WD_AD_Auto_Calibration_ALL(daq->card_id);

  WD_AI_CH_Config(daq->card_id, All_Channels, AD_B_1_V);
  WD_AI_Config(daq->card_id,
               WD_ExtTimeBase,
               TRUE,
               WD_AI_ADCONVSRC_TimePacer,
               FALSE,
               TRUE);
}

void daq_destroy(daq_t *daq)
{
  WD_AI_ContBufferReset(daq->card_id);
  WD_Release_Card(daq->card_id);
}

int daq_acquire(daq_t *daq, uint16_t *buffer,
                uint32_t samples_per_trig, uint32_t trig_count)
{
  uint16_t buffer_id, status;
  uint32_t buffer_size = samples_per_trig * trig_count;

  status = WD_AI_Trig_Config(daq->card_id,
                             WD_AI_TRGMOD_POST,
                             WD_AI_TRGSRC_ExtD,
                             WD_AI_TrgPositive,
                             0, 0., 0, 0, 0,
                             trig_count);

  status = WD_AI_ContBufferSetup(daq->card_id, buffer, buffer_size, &buffer_id);

  status = WD_AI_ContScanChannels(daq->card_id,
                                  0,
                                  buffer_id,
                                  buffer_size,
                                  SCAN_INTERVAL,
                                  SCAN_INTERVAL,
                                  SYNCH_OP);
}
