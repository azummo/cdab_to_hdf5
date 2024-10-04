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
#include <map>
#include "ds.h"

class eos_hdf5
{
private:
  std::vector<Event> event_buffer;
  std::vector<uint32_t> counter_buffer;
  std::vector<uint32_t> timetag_buffer;
  std::vector<uint32_t> exttimetag_buffer;
  std::vector<uint16_t> samples_buffer;
  std::vector<uint16_t> pattern_buffer;
  std::vector<uint32_t> caen_status_buffer;
  std::vector<uint8_t> ptb_status_buffer;
  std::vector<uint64_t> timestamp_buffer;
  std::vector<uint64_t> trigger_word_buffer;
  std::vector<uint64_t> word_type_buffer;
  std::vector<uint64_t> id_buffer;
  std::vector<int> type_buffer;
  std::vector<int> tv_sec_buffer;
  std::vector<uint64_t> tv_nsec_buffer;
  H5::H5File *hdf5_file;
  H5::DataSpace *samples_space;
  H5::DataSpace *event_space;
  H5::DataSpace *scalar;
  H5::DataSet *dataset;
  H5::CompType *event_type;
  int max_buffer = 1000;
  int n_samples = 100;
  int n_boards = 17;
  int n_channels = 16;
  static const int rank=1;
  int length;
  hsize_t dim[rank];
  hsize_t maxdims[rank];
  hsize_t size[rank];
  hsize_t dimsext[rank];
  hsize_t chunk_dims[rank];
  static const int rank_samples=2;
  hsize_t dim_samples[rank_samples];
  hsize_t maxdims_samples[rank_samples];
  hsize_t size_samples[rank_samples];
  hsize_t dimsext_samples[rank_samples];
  hsize_t chunk_dims_samples[rank_samples];
  hsize_t offset_samples[rank_samples];
  bool first = true;
  std::map<int, int> boards { {0,19858}, {1,24190}, {2,24191}, {3,24192},
                            {4,24193}, {5,24194}, {6,24195}, {7,24196},
                            {8,24305}, {9,24306}, {10,24307},{11,24308},
                            {12,24309},{13,24310},{14,24311},{15,24312},
                            {16,19857} };

public:

/**
 * Create hdf5 file
 */
void create_hdf5(std::string hdf5_filename);

/**
 * Fill the event information from e into a buffer
 */
void fill(Event e);

void fill_caen_board_buffers(int board);
void fill_caen_chan_buffers(int board, int chan);
void fill_other_buffers();

/**
 * Write the meta info into the hdf5_file
 */
void write_meta(RunStart r);

/**
 * Write the attributes into the hdf5_file
 */
void write_attrs(Event e);


/**
 * Write the buffered information into a new hdf5_file
 * with filename hdf5_filename
 */
void write();

};
#endif
