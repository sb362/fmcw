#include "export.h"

#include <netcdf.h>

int netcdf_create_file(const char *path)
{
	int ncid, range_doppler_varid;
	int dimids[3];

	nc_create(path, NC_CLOBBER, &ncid);

	nc_def_dim(ncid, "time",     abc,        dimids[0]);
	nc_def_dim(ncid, "velocity", frame_size, dimids[1]);
	nc_def_dim(ncid, "range",    n_bins,     dimids[2]);

	nc_def_var(ncid, "range_doppler", NC_DOUBLE,
						 3, dimids, &range_doppler_varid);

	nc_enddef(ncid);

	nc_put_var_double(ncid, range_doppler_varid, );

	nc_close(ncid);
}

void netcdf_close_file(int ncid)
{
	nc_close(ncid);
}
