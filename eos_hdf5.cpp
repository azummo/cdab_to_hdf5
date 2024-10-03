#include "eos_hdf5.hh"

void eos_hdf5::fill(Event e)
{
  // Fill buffer
  event_buffer.push_back(e);
  // When buffer is full write out the events
  if( event_buffer.size() > max_buffer ) write();
}

void eos_hdf5::create_hdf5(std::string hdf5_filename)
{
  // Define data structure based on structs from builder

  // Array of CAEN Samples
  int rank_samples = 1;
  hsize_t dim_samples[1];
  dim_samples[0]=400;
  H5::ArrayType caen_sample_array(H5::PredType::NATIVE_UINT16, rank_samples, dim_samples);

  // CAENChannel Struct
  H5::CompType caen_channel_type(sizeof(CAENChannel));
  caen_channel_type.insertMember("chID", HOFFSET(CAENChannel, chID), H5::PredType::NATIVE_UINT32);
  caen_channel_type.insertMember("offset", HOFFSET(CAENChannel, offset), H5::PredType::NATIVE_UINT32);
  caen_channel_type.insertMember("threshold", HOFFSET(CAENChannel, threshold), H5::PredType::NATIVE_UINT32);
  caen_channel_type.insertMember("dynamic_range", HOFFSET(CAENChannel, dynamic_range), H5::PredType::NATIVE_FLOAT);
  caen_channel_type.insertMember("samples", HOFFSET(CAENChannel, samples), caen_sample_array);
  caen_channel_type.insertMember("pattern", HOFFSET(CAENChannel, pattern), H5::PredType::NATIVE_UINT16);

  // Array of CAENChannel Structs
  int rank_caen_channel = 1;
  hsize_t dim_caen_channel[1];
  dim_caen_channel[0]=16;
  H5::ArrayType caen_channel_array(caen_channel_type, rank_caen_channel, dim_caen_channel);

  // Array of characters for name FIXME
  int rank_name = 1;
  hsize_t dim_name[1];
  dim_name[0]=50;
  H5::ArrayType caen_name_array(H5::PredType::NATIVE_CHAR, rank_name, dim_name);

  // CAENEvent Struct
  H5::CompType caen_event_type(sizeof(CAENEvent));
  caen_event_type.insertMember("type", HOFFSET(CAENEvent, type), H5::PredType::NATIVE_UINT16);
  caen_event_type.insertMember("name", HOFFSET(CAENEvent, name), caen_name_array);
  caen_event_type.insertMember("bits", HOFFSET(CAENEvent, bits), H5::PredType::NATIVE_UINT16);
  caen_event_type.insertMember("samples", HOFFSET(CAENEvent, samples), H5::PredType::NATIVE_UINT16);
  caen_event_type.insertMember("ns_sample", HOFFSET(CAENEvent, ns_sample), H5::PredType::NATIVE_FLOAT);
  caen_event_type.insertMember("counter", HOFFSET(CAENEvent, counter), H5::PredType::NATIVE_UINT32);
  caen_event_type.insertMember("timetag", HOFFSET(CAENEvent, timetag), H5::PredType::NATIVE_UINT32);
  caen_event_type.insertMember("exttimetag", HOFFSET(CAENEvent, exttimetag), H5::PredType::NATIVE_UINT16);
  caen_event_type.insertMember("channels", HOFFSET(CAENEvent, channels), caen_channel_array);

  // Array of CAENEvent Structs
  int rank_caen_dig = 1;
  hsize_t dim_caen_dig[1];
  dim_caen_dig[0]=NDIGITIZERS;
  H5::ArrayType caen_dig_array(caen_event_type, rank_caen_dig, dim_caen_dig);

  // ptb_trigger_t Struct
  H5::CompType ptb_trigger_type(sizeof(ptb_trigger_t));
  ptb_trigger_type.insertMember("timestamp", HOFFSET(ptb_trigger_t, timestamp), H5::PredType::NATIVE_ULONG);

  // timespec Struct
  H5::CompType timespec_type(sizeof(timespec));
  timespec_type.insertMember("tv_sec", HOFFSET(timespec, tv_sec), H5::PredType::NATIVE_INT);
  timespec_type.insertMember("tv_nsec", HOFFSET(timespec, tv_nsec), H5::PredType::NATIVE_ULONG);

  // Event Struct
  event_type = new H5::CompType(sizeof(Event));
  event_type->insertMember("id", HOFFSET(Event, id), H5::PredType::NATIVE_ULONG);
  event_type->insertMember("type", HOFFSET(Event, type), H5::PredType::NATIVE_UINT);
  event_type->insertMember("caen", HOFFSET(Event, caen), caen_dig_array);
  event_type->insertMember("ptb", HOFFSET(Event, ptb), ptb_trigger_type);
  event_type->insertMember("caen_status", HOFFSET(Event, caen_status), H5::PredType::NATIVE_UINT32);
  event_type->insertMember("ptb_status", HOFFSET(Event, ptb_status), H5::PredType::NATIVE_UINT8);
  event_type->insertMember("creation_time", HOFFSET(Event, creation_time), timespec_type);

  // Length of data
  length = event_buffer.size();
  dim[0] = length;
  maxdims[0] = H5S_UNLIMITED; 

  // Enable chunking
  chunk_dims[0] = max_buffer;
  H5::DSetCreatPropList prop;
  prop.setChunk(rank, chunk_dims);

  // Open hdf5 file
  hdf5_file = new H5::H5File(hdf5_filename , H5F_ACC_TRUNC);

  // Create empty dataset
  event_space = new H5::DataSpace(rank,dim,maxdims);
  dataset = new H5::DataSet(hdf5_file->createDataSet("ds",*event_type,*event_space,prop));
}

void eos_hdf5::write()
{
  // Extend dataset by size of buffer
  dimsext[0] = event_buffer.size();
  size[0] = dim[0] + dimsext[0];
  dataset->extend(size);

  // Select newly created "hyperslab" in dataset
  H5::DataSpace *filespace = new H5::DataSpace(dataset->getSpace());
  filespace->selectHyperslab(H5S_SELECT_SET, dimsext, dim);

  // Define memory space
  H5::DataSpace *memspace = new H5::DataSpace(rank, dimsext, NULL);

  // Write to dataset
  dataset->write(&event_buffer[0], *event_type, *memspace, *filespace);

  // Redefine size of dataset
  dim[0] = size[0];

  // Clear events from the buffer
  event_buffer.clear();
}
