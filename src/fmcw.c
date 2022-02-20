#include "fmcw.h"
#include "util.h"

void fmcw_cpi_init(fmcw_cpi_t *cpi, size_t chirp_size, size_t cpi_size)
{
	LOG_FMT(TRACE, "Initialising CPI (%llu, %llu)", chirp_size, cpi_size);
	size_t buf_size      = chirp_size * cpi_size;
	cpi->cpi_size        = cpi_size;
	cpi->chirp_size      = chirp_size;
	cpi->buffer_size     = buf_size;

	// Input signal (volts) is entirely real, which means there is
	// symmetry in the output of the fast-time FFT. Only half of
	// the elements in the complex output need to be computed.
	cpi->volts            = aligned_malloc(sizeof(double)       * buf_size);
	cpi->freq_spectrum    = aligned_malloc(sizeof(fftw_complex) * buf_size / 2);
	cpi->freq_spectrum_db = aligned_malloc(sizeof(double)       * buf_size / 2);
	cpi->range_doppler    = aligned_malloc(sizeof(fftw_complex) * buf_size / 2);
	cpi->range_doppler_db = aligned_malloc(sizeof(double)       * buf_size / 2);
}

void fmcw_cpi_destroy(fmcw_cpi_t *cpi)
{
	aligned_free(cpi->volts);
	aligned_free(cpi->freq_spectrum);
	aligned_free(cpi->freq_spectrum_db);
	aligned_free(cpi->range_doppler);
	aligned_free(cpi->range_doppler_db);
}

void fmcw_context_init(fmcw_context_t *ctx, size_t chirp_size, size_t cpi_size)
{
	fmcw_cpi_init(&ctx->cpi, chirp_size, cpi_size);

	// Details on fftw_plan_many_dft() are available at:
	// https://www.fftw.org/fftw3_doc/Advanced-Complex-DFTs.html

	LOG(TRACE, "Initialising fast-time FFT plan...");
	int nfast[] = {chirp_size};
	ctx->fast_time = fftw_plan_many_dft_r2c(
		1,        // rank, we are performing only 1D transforms along each row
    nfast,    // length of 1D transforms = number of columns (range bins)
		cpi_size, // number of 1D transforms = number of rows (chirps)
		ctx->cpi.volts,         // input array
		NULL,                   // unused, for FFT'ing subarrays
		1,                      // distance between elements in each input row
		chirp_size,             // distance between each transform input (row)
		ctx->cpi.freq_spectrum, // output array
		NULL,                   // unused, for FFT'ing subarrays
		1,                      // distance between elements in each output row
		chirp_size,             // distance between each transform output (row)
		FFTW_MEASURE            // optimise plan by profiling several FFTs
	);

	LOG(TRACE, "Initialising slow-time FFT plan...");
	int nslow[] = {cpi_size}; 
	ctx->slow_time = fftw_plan_many_dft(
		1,          // rank, we are performing only 1D transforms along each col
    nslow,      // length of 1D transforms = number of rows (chirps)
		chirp_size, // number of 1D transforms = number of columns (range bins)
		ctx->cpi.freq_spectrum, // input array
		NULL,                   // unused, for FFT'ing subarrays
		chirp_size,             // distance between elements in each input column
		1,                      // distance between each transform input (column)
		ctx->cpi.range_doppler, // output array
		NULL,                   // unused, for FFT'ing subarrays
		chirp_size,             // distance between elements in each output column
		1,                      // distance between each transform output (column)
		FFTW_FORWARD,					  // forward transform
		FFTW_MEASURE            // optimise plan by profiling several FFTs
	);

	LOG(TRACE, "Done.");
}

void fmcw_context_destroy(fmcw_context_t *ctx)
{
	fftw_destroy_plan(ctx->fast_time);
	fftw_destroy_plan(ctx->slow_time);
	fmcw_cpi_destroy(&ctx->cpi);
}

void fmcw_process(fmcw_context_t *ctx)
{
	double dt = elapsed_milliseconds();

  fftw_execute(ctx->fast_time);

	for (size_t chirp = 0; chirp < ctx->cpi.cpi_size; ++chirp)
	{
		for (size_t range_bin = 0; range_bin < ctx->cpi.chirp_size / 2; ++range_bin)
		{
			size_t i = chirp * ctx->cpi.chirp_size + range_bin;
		}
	}

	fftw_execute(ctx->slow_time);

	dt = elapsed_milliseconds() - dt;
	LOG_FMT(TRACE, "Processing took %.2f ms", dt);
}
