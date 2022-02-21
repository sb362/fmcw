#ifndef WINDOW_H
#define WINDOW_H

#include <fftw3.h>

typedef enum
{
  NO_WINDOW,
  FLAT_TOP_WINDOW,
  HAN_WINDOW,
  BLACKMAN_WINDOW,
  BLACKMAN_HARRIS_WINDOW
} win_type_t;

void        flat_top_window(double *s, size_t n, size_t stride);
void             han_window(double *s, size_t n, size_t stride);
void        blackman_window(double *s, size_t n, size_t stride);
void blackman_harris_window(double *s, size_t n, size_t stride);

void        flat_top_window_cx(fftw_complex *s, size_t n, size_t stride);
void             han_window_cx(fftw_complex *s, size_t n, size_t stride);
void        blackman_window_cx(fftw_complex *s, size_t n, size_t stride);
void blackman_harris_window_cx(fftw_complex *s, size_t n, size_t stride);

void scaled_window   (win_type_t t,       double *s, size_t n, size_t stride);
void scaled_window_cx(win_type_t t, fftw_complex *s, size_t n, size_t stride);

#endif // WINDOW_H
