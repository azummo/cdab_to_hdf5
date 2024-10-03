/**
 * Eos HDF5 Data Structure
 *
 * Used to write cdab files to HDF5.
 * 
 * Event information is stored in buffers using the fill() method
 * until data is written to the HDF5 file using the write() method
 */

#ifndef __EOS_HDF5__
#define __EOS_HDF5__

#include <H5Cpp.h>
#include <vector>
#include <iostream>
#include "ds.h"

class eos_hdf5
{
private:
  std::vector<Event> event_buffer;
  H5::H5File *hdf5_file;
  H5::DataSpace *event_space;
  H5::DataSet *dataset;
  H5::CompType *event_type;
  int max_buffer = 1000;
  static const int rank=1;
  int length;
  hsize_t dim[rank];
  hsize_t maxdims[rank];
  hsize_t size[rank];
  hsize_t dimsext[rank];
  hsize_t chunk_dims[rank];
public:

/**
 * Create hdf5 file
 */
void create_hdf5(std::string hdf5_filename);

/**
 * Fill the event information from e into a buffer
 */
void fill(Event e);

/**
 * Write the buffered information into a new hdf5_file
 * with filename hdf5_filename
 */
void write();

};
#endif
