#include "fmcw.h"

#include "util.h"

typedef struct
{
	double mean, skew, width, kurtosis;
} doppler_moments_t;

void bucket(fmcw_cpi_t *frame, double *histogram, doppler_moments_t *moments)
{
	// memset(histogram, 0, frame->cpi_size * sizeof(double));

	for (size_t i = 0; i < frame->cpi_size; ++i)
	{
		for (size_t j = 0; j < frame->n_bins; ++j)
		{
			histogram[j] += ;
		}
	}
}

void doppler_extract(fmcw_cpi_t *frame)
{

}
