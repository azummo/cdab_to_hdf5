#include "eos_hdf5.hh"
using namespace H5;

void eos_hdf5::fill(Event e)
{
  if(first)
  {
    write_attrs(e);
    first = false;
  }
  // Fill buffer
  event_buffer.push_back(e);
  // When buffer is full write out the events
  if( event_buffer.size() >= max_buffer ) write();
}

void eos_hdf5::create_hdf5(std::string hdf5_filename)
{
  // Length of data
  length = event_buffer.size();
  dim[0] = length;
  maxdims[0] = H5S_UNLIMITED; 

  // Enable chunking
  chunk_dims[0] = max_buffer;
  DSetCreatPropList prop;
  prop.setChunk(rank, chunk_dims);

  // Same for 2D samples data
  dim_samples[0] = length;
  dim_samples[1] = n_samples;
  maxdims_samples[0] = H5S_UNLIMITED;
  maxdims_samples[1] = n_samples;

  chunk_dims_samples[0] = max_buffer;
  chunk_dims_samples[1] = n_samples;
  DSetCreatPropList prop_samples;
  prop_samples.setChunk(rank_samples, chunk_dims_samples);

  // Open hdf5 file
  hdf5_file = new H5File(hdf5_filename , H5F_ACC_TRUNC);

  // Create CAEN, PTB, INFO Groups
  Group event_group = hdf5_file->createGroup("event");
  Group meta_group = hdf5_file->createGroup("meta");
  Group caen_group = event_group.createGroup("caen");
  Group ptb_group = event_group.createGroup("ptb");

  // Create Samples DataSpace for 2D sample info (events x sample number)
  samples_space = new DataSpace(rank_samples,dim_samples,maxdims_samples);
  // Create Event DataSpace for per event information
  event_space = new DataSpace(rank,dim,maxdims);
  // Create Scalar DataSpace for attributes
  scalar = new DataSpace(0,NULL);

  // Create each CAEN board group
  for(int board = 0; board<n_boards;board++)
  {
    Group card_group = caen_group.createGroup(std::to_string(board));
    // Create Attributes
    Attribute type = card_group.createAttribute("type",PredType::NATIVE_UINT16,*scalar);
    Attribute name = card_group.createAttribute("name",PredType::NATIVE_CHAR,*scalar);
    Attribute bits = card_group.createAttribute("bits",PredType::NATIVE_UINT32,*scalar);
    Attribute ns_sample = card_group.createAttribute("ns_sample",PredType::NATIVE_UINT32,*scalar);
    Attribute samples = card_group.createAttribute("samples",PredType::NATIVE_UINT32,*scalar);
    // Create DataSets
    DataSet counter_set = card_group.createDataSet("counter", PredType::NATIVE_UINT32,*event_space,prop);
    DataSet timetag_set = card_group.createDataSet("timetag", PredType::NATIVE_UINT32,*event_space,prop);
    DataSet exttimetag_set = card_group.createDataSet("exttimetag", PredType::NATIVE_UINT32,*event_space,prop);
    // Create each CAEN channel group
    for(int chan = 0; chan<n_channels; chan++)
    {
      Group chan_group = card_group.createGroup(std::to_string(chan));
      Attribute channel_id_group = chan_group.createAttribute("chID",PredType::NATIVE_UINT32,*scalar);
      Attribute offset = chan_group.createAttribute("offset",PredType::NATIVE_UINT32,*scalar);
      Attribute threshold = chan_group.createAttribute("threshold",PredType::NATIVE_UINT32,*scalar);
      Attribute dynamic_range = chan_group.createAttribute("dynamic_range",PredType::NATIVE_DOUBLE,*scalar);
      // Create DataSpaces and DataSets for Samples
      DataSet samples_set = chan_group.createDataSet("samples", PredType::NATIVE_UINT16, *samples_space, prop_samples);
      DataSet patterns_set = chan_group.createDataSet("pattern", PredType::NATIVE_UINT16, *event_space, prop);
    }
  }

  DataSet caen_status_set = caen_group.createDataSet("caen_status",PredType::NATIVE_UINT32,*event_space,prop);

  // Create PTB DataSets
  DataSet ptb_status_set = ptb_group.createDataSet("ptb_status", PredType::NATIVE_UINT8,*event_space,prop);
  DataSet timestamp_set = ptb_group.createDataSet("timestamp", PredType::NATIVE_ULONG,*event_space,prop);
  DataSet trigger_word_set = ptb_group.createDataSet("trigger_word", PredType::NATIVE_ULONG,*event_space,prop);
  DataSet word_type_set = ptb_group.createDataSet("word_type", PredType::NATIVE_ULONG,*event_space,prop);

  // Create TimeSpec DataSets
  DataSet tv_sec_set = event_group.createDataSet("tv_sec",PredType::NATIVE_INT,*event_space,prop);
  DataSet tv_nsec_set = event_group.createDataSet("tv_nsec",PredType::NATIVE_ULONG,*event_space,prop);

  // Create ID and type DataSets
  DataSet id = event_group.createDataSet("id",PredType::NATIVE_ULONG,*event_space,prop);
  DataSet type = event_group.createDataSet("type",PredType::NATIVE_INT,*event_space,prop);

  // Create Meta DataSets
  Attribute meta_type = event_group.createAttribute("type",PredType::NATIVE_UINT32,*scalar);
  Attribute run_number = event_group.createAttribute("run_number",PredType::NATIVE_UINT32,*scalar);
  Attribute outfile_name = event_group.createAttribute("outfile",PredType::NATIVE_CHAR,*scalar);
  Attribute run_type = event_group.createAttribute("run_type",PredType::NATIVE_INT,*scalar);
  Attribute source_type = event_group.createAttribute("source_type",PredType::NATIVE_INT,*scalar);
  Attribute source_x = event_group.createAttribute("source_x",PredType::NATIVE_FLOAT,*scalar);
  Attribute source_y = event_group.createAttribute("source_y",PredType::NATIVE_FLOAT,*scalar);
  Attribute source_z = event_group.createAttribute("source_z",PredType::NATIVE_FLOAT,*scalar);
  Attribute source_theta = event_group.createAttribute("source_theta",PredType::NATIVE_FLOAT,*scalar);
  Attribute source_phi = event_group.createAttribute("source_phi",PredType::NATIVE_FLOAT,*scalar);
  Attribute fiber_number = event_group.createAttribute("fiber_number",PredType::NATIVE_INT,*scalar);
  Attribute laserball_size = event_group.createAttribute("laserball_size",PredType::NATIVE_FLOAT,*scalar);
  Attribute laser_wavelength = event_group.createAttribute("laser_wavelength",PredType::NATIVE_FLOAT,*scalar);
  Attribute first_event_id = event_group.createAttribute("first_event_id",PredType::NATIVE_UINT64,*scalar);
}

void eos_hdf5::fill_caen_board_buffers(int board)
{
  counter_buffer.clear();
  timetag_buffer.clear();
  exttimetag_buffer.clear();
  for(auto & event : event_buffer)
  {
    counter_buffer.push_back(event.caen[board].counter);
    timetag_buffer.push_back(event.caen[board].timetag);
    exttimetag_buffer.push_back(event.caen[board].exttimetag);
  }
}

void eos_hdf5::fill_caen_chan_buffers(int board, int chan)
{
  samples_buffer.clear();
  pattern_buffer.clear();
  for(auto & event : event_buffer)
  {
    pattern_buffer.push_back(event.caen[board].channels[chan].pattern);
    for(int i=0; i<n_samples;i++)
      {
        samples_buffer.push_back(event.caen[board].channels[chan].samples[i]);
      }
  }
}

void eos_hdf5::fill_other_buffers()
{
  id_buffer.clear();
  type_buffer.clear();
  caen_status_buffer.clear();
  ptb_status_buffer.clear();
  timestamp_buffer.clear();
  trigger_word_buffer.clear();
  word_type_buffer.clear();
  tv_sec_buffer.clear();
  tv_nsec_buffer.clear();
  for(auto & event : event_buffer)
  {
    id_buffer.push_back(event.id);
    type_buffer.push_back(event.type);
    caen_status_buffer.push_back(event.caen_status);
    ptb_status_buffer.push_back(event.ptb_status);
    timestamp_buffer.push_back(event.ptb.timestamp);
    trigger_word_buffer.push_back(event.ptb.trigger_word);
    word_type_buffer.push_back(event.ptb.word_type);
    tv_sec_buffer.push_back(event.creation_time.tv_sec);
    tv_nsec_buffer.push_back(event.creation_time.tv_nsec);
  }
}

void eos_hdf5::write_meta(RunStart r)
{
  Group meta_group = hdf5_file->openGroup("meta");
  Attribute meta_type = meta_group.openAttribute("type");
  Attribute run_number = meta_group.openAttribute("run_number");
  Attribute outfile_name = meta_group.openAttribute("outfile");
  Attribute run_type = meta_group.openAttribute("run_type");
  Attribute source_type = meta_group.openAttribute("source_type");
  Attribute source_x = meta_group.openAttribute("source_x");
  Attribute source_y = meta_group.openAttribute("source_y");
  Attribute source_z = meta_group.openAttribute("source_z");
  Attribute source_theta = meta_group.openAttribute("source_theta");
  Attribute source_phi = meta_group.openAttribute("source_phi");
  Attribute fiber_number = meta_group.openAttribute("fiber_number");
  Attribute laserball_size = meta_group.openAttribute("laserball_size");
  Attribute laser_wavelength = meta_group.openAttribute("laser_wavelength");
  Attribute first_event_id = meta_group.openAttribute("first_event_id");

  meta_type.write(PredType::NATIVE_UINT32, &r.type);
  run_number.write(PredType::NATIVE_UINT32, &r.run_number);
  outfile_name.write(PredType::NATIVE_CHAR, &r.outfile);
  run_type.write(PredType::NATIVE_INT, &r.run_type);
  source_type.write(PredType::NATIVE_INT, &r.source_type);
  source_x.write(PredType::NATIVE_FLOAT, &r.source_x);
  source_y.write(PredType::NATIVE_FLOAT, &r.source_y);
  source_z.write(PredType::NATIVE_FLOAT, &r.source_z);
  source_theta.write(PredType::NATIVE_FLOAT, &r.source_theta);
  source_phi.write(PredType::NATIVE_FLOAT, &r.source_phi);
  fiber_number.write(PredType::NATIVE_INT, &r.fiber_number);
  laserball_size.write(PredType::NATIVE_FLOAT, &r.laserball_size);
  laser_wavelength.write(PredType::NATIVE_FLOAT, &r.laser_wavelength);
  first_event_id.write(PredType::NATIVE_UINT64, &r.first_event_id);
}


void eos_hdf5::write_attrs(Event e)
{
  Group caen_group = hdf5_file->openGroup("event/caen");
  // Loop over each CAEN board
  for(int board = 0; board<n_boards;board++)
  {
    Group card_group = caen_group.openGroup(std::to_string(board));
    Attribute type = card_group.openAttribute("type");
    Attribute name = card_group.openAttribute("name");
    Attribute bits = card_group.openAttribute("bits");
    Attribute ns_sample = card_group.openAttribute("ns_sample");
    Attribute samples = card_group.openAttribute("samples");
    type.write(PredType::NATIVE_UINT16, &e.caen[board].type);
    name.write(PredType::NATIVE_CHAR, & e.caen[board].name);
    bits.write(PredType::NATIVE_UINT16, &e.caen[board].bits);
    ns_sample.write(PredType::NATIVE_FLOAT, &e.caen[board].ns_sample);
    samples.write(PredType::NATIVE_UINT16, &e.caen[board].samples);
    for(int chan = 0; chan<n_channels; chan++)
    {
      Group chan_group = card_group.openGroup(std::to_string(chan));
      Attribute channel_id = chan_group.openAttribute("chID");
      Attribute offset = chan_group.openAttribute("offset");
      Attribute threshold = chan_group.openAttribute("threshold");
      Attribute dynamic_range = chan_group.openAttribute("dynamic_range");
      channel_id.write(PredType::NATIVE_UINT32, &e.caen[board].channels[chan].chID);
      offset.write(PredType::NATIVE_UINT32, &e.caen[board].channels[chan].offset);
      threshold.write(PredType::NATIVE_UINT32, &e.caen[board].channels[chan].threshold);
      dynamic_range.write(PredType::NATIVE_FLOAT, &e.caen[board].channels[chan].dynamic_range);
    }
  }
}

void eos_hdf5::write()
{
  // Extend dataset by size of buffer
  dimsext[0] = event_buffer.size();
  size[0] = dim[0] + dimsext[0];

  dimsext_samples[0] = event_buffer.size();
  dimsext_samples[1] = n_samples;
  offset_samples[0] = size_samples[0];
  offset_samples[1] = 0;
  size_samples[0] = dim_samples[0] + dimsext_samples[0];
  size_samples[1] = dim_samples[1];

  // Define memory space
  H5::DataSpace *memspace = new DataSpace(rank, dimsext, NULL);
  H5::DataSpace *memspace_samples = new DataSpace(rank_samples, dimsext_samples, NULL);

  Group caen_group = hdf5_file->openGroup("event/caen");

  // Loop over each CAEN board
  for(int board = 0; board<n_boards;board++)
  {
    // Open CAEN board Group
    Group card_group = caen_group.openGroup(std::to_string(board));
    // Open CAEN board DataSets
    DataSet counter_set = card_group.openDataSet("counter");
    DataSet timetag_set = card_group.openDataSet("timetag");
    DataSet exttimetag_set = card_group.openDataSet("exttimetag");
    // Extend DataSets
    counter_set.extend(size);
    timetag_set.extend(size);
    exttimetag_set.extend(size);
    // Select newly created "hyperslabs" in datasets
    DataSpace *counter_space = new DataSpace(counter_set.getSpace());
    counter_space->selectHyperslab(H5S_SELECT_SET, dimsext, dim);
    DataSpace *timetag_space = new DataSpace(timetag_set.getSpace());
    timetag_space->selectHyperslab(H5S_SELECT_SET, dimsext, dim);
    DataSpace *exttimetag_space = new DataSpace(exttimetag_set.getSpace());
    exttimetag_space->selectHyperslab(H5S_SELECT_SET, dimsext, dim);
    // Fill the buffers
    fill_caen_board_buffers(board);
    // Write to datasets
    counter_set.write(&counter_buffer[0], PredType::NATIVE_UINT32, *memspace, *counter_space);
    timetag_set.write(&timetag_buffer[0], PredType::NATIVE_UINT32, *memspace, *timetag_space);
    exttimetag_set.write(&exttimetag_buffer[0], PredType::NATIVE_UINT32, *memspace, *exttimetag_space);
    // Loop over each CAEN channel
    for(int chan = 0; chan<n_channels; chan++)
    {
      // Open CAEN channel Group
      Group channel_group = card_group.openGroup(std::to_string(chan));
      // Open CAEN channel DataSets
      DataSet samples_set = channel_group.openDataSet("samples");
      DataSet pattern_set = channel_group.openDataSet("pattern");
      // Extend DataSets
      samples_set.extend(size_samples);
      pattern_set.extend(size);
      // Select "hyperslabs"
      DataSpace *samples_space = new DataSpace(samples_set.getSpace());
      samples_space->selectHyperslab(H5S_SELECT_SET, dimsext_samples, offset_samples);
      DataSpace *pattern_space = new DataSpace(pattern_set.getSpace());
      pattern_space->selectHyperslab(H5S_SELECT_SET, dimsext, dim);
      // Fill the buffers
      fill_caen_chan_buffers(board, chan);
      // Write to datasets
      samples_set.write(&samples_buffer[0], PredType::NATIVE_UINT16, *memspace_samples, *samples_space);
      pattern_set.write(&pattern_buffer[0], PredType::NATIVE_UINT16, *memspace, *pattern_space);
    }
  }

  // Fill the buffers
  fill_other_buffers();
  DataSet caen_status_set = caen_group.openDataSet("caen_status");
  caen_status_set.extend(size);
  DataSpace *caen_status_space = new DataSpace(caen_status_set.getSpace());
  caen_status_space->selectHyperslab(H5S_SELECT_SET,dimsext,dim);
  caen_status_set.write(&caen_status_buffer[0], PredType::NATIVE_UINT32, *memspace, *caen_status_space);
  // Open PTB Group
  Group ptb_group = hdf5_file->openGroup("/event/ptb/");
  // Open PTB DataSets
  DataSet timestamp_set = ptb_group.openDataSet("timestamp");
  DataSet trigger_word_set = ptb_group.openDataSet("trigger_word");
  DataSet word_type_set = ptb_group.openDataSet("word_type");
  // Extend DataSets
  timestamp_set.extend(size);
  trigger_word_set.extend(size);
  word_type_set.extend(size);
  // Select "hyperslabs"
  DataSpace *timestamp_space = new DataSpace(timestamp_set.getSpace());
  timestamp_space->selectHyperslab(H5S_SELECT_SET, dimsext, dim);
  DataSpace *trigger_word_space = new DataSpace(trigger_word_set.getSpace());
  trigger_word_space->selectHyperslab(H5S_SELECT_SET, dimsext, dim);
  DataSpace *word_type_space = new DataSpace(word_type_set.getSpace());
  word_type_space->selectHyperslab(H5S_SELECT_SET, dimsext, dim);
  // Write to datasets
  timestamp_set.write(&timestamp_buffer[0], PredType::NATIVE_ULONG, *memspace, *timestamp_space);
  trigger_word_set.write(&trigger_word_buffer[0], PredType::NATIVE_ULONG, *memspace, *trigger_word_space);
  word_type_set.write(&word_type_buffer[0], PredType::NATIVE_ULONG, *memspace, *word_type_space);


  // Open Event group
  Group event_group = hdf5_file->openGroup("/event/");
  // Open TimeSpec and id/type DataSets
  DataSet id_set = event_group.openDataSet("id");
  DataSet type_set = event_group.openDataSet("type");
  DataSet tv_sec_set = event_group.openDataSet("tv_sec");
  DataSet tv_nsec_set = event_group.openDataSet("tv_nsec");
  // Extend DataSets
  id_set.extend(size);
  type_set.extend(size);
  tv_sec_set.extend(size);
  tv_nsec_set.extend(size);
  // Select "hyperslabs"
  DataSpace *id_space = new DataSpace(id_set.getSpace());
  id_space->selectHyperslab(H5S_SELECT_SET, dimsext, dim);
  DataSpace *type_space = new DataSpace(type_set.getSpace());
  type_space->selectHyperslab(H5S_SELECT_SET, dimsext, dim);
  DataSpace *tv_sec_space = new DataSpace(tv_sec_set.getSpace());
  tv_sec_space->selectHyperslab(H5S_SELECT_SET, dimsext, dim);
  DataSpace *tv_nsec_space = new DataSpace(tv_nsec_set.getSpace());
  tv_nsec_space->selectHyperslab(H5S_SELECT_SET, dimsext, dim);
  // Write to datasets
  id_set.write(&id_buffer[0], PredType::NATIVE_ULONG, *memspace, *id_space);
  type_set.write(&type_buffer[0], PredType::NATIVE_INT, *memspace, *type_space);
  tv_sec_set.write(&tv_sec_buffer[0], PredType::NATIVE_INT, *memspace, *tv_sec_space);
  tv_nsec_set.write(&tv_nsec_buffer[0], PredType::NATIVE_ULONG, *memspace, *tv_nsec_space);

  // Redefine size of datasets
  dim[0] = size[0];
  dim_samples[0] = size_samples[0];
  dim_samples[1] = size_samples[1];

  // Clear events from the buffer
  event_buffer.clear();
}

