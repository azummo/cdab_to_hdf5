#include <iostream>
#include <fstream>
#include <vector>
#include <H5Cpp.h>
#include "ds.h"
#include "eos_hdf5.hh"

int main(int argc, char** argv)
{

  CDABHeader chdr;
  RHDR rhdr;
  Event e;
  std::ifstream cdab;
  eos_hdf5 eos_hdf5_file;

  std::string cdab_filename = argv[1];
  std::string hdf5_filename = argv[2];

  std::cout << "Reading: " << cdab_filename << std::endl << "Writing to: " << hdf5_filename << std::endl;

  // Open cdab file
  cdab.open(cdab_filename, std::ifstream::binary);

  // Open hdf5 file
  eos_hdf5_file.create_hdf5(hdf5_filename);
  std::cout << "Created HDF5 File" << std::endl;
  int events = 0;
  while(cdab)
    {
      events++;
      cdab.read(reinterpret_cast<char*>(&chdr),sizeof(chdr));
/*
      printf("CDAB Header Type: %u \n", chdr.record_type);
      printf("CDAB Header Size: %u \n", chdr.size);
*/
      switch(chdr.record_type)
      {
        case EMPTY:
          std::cout << "Found empty event" << std::endl;
	  break;
        case DETECTOR_EVENT:
          // Attempt to read event according to CDAB Header
          cdab.read(reinterpret_cast<char*>(&e), sizeof(e));
          // Print some info about the event
/*
          printf("Event ID: %lu \n", e.id);
          printf("Event Type: %u \n", e.type);
          printf("Event CAEN Status: %u \n", e.caen_status);
          printf("Event PTB Status: %u \n", e.ptb_status);
*/
/*
          printf("CAEN samples: ");

	  for(int i=0;i<499;i++)
	  {
	    printf("%u, ", e.caen[0].channels[0].samples[i]);
          }
	  printf("%u]\n",e.caen[0].channels[0].samples[499]);
          if(e.caen_status==0) break;
*/
	  // Add event to hdf5 buffer
	  eos_hdf5_file.fill(e);
	  break;
	case RUN_HEADER:
          cdab.read(reinterpret_cast<char*>(&rhdr),sizeof(rhdr));
/*
          printf("Run Header Type: %u \n", rhdr.type);
          printf("Run Header Date: %u \n", rhdr.date);
          printf("Run Header Time: %u \n", rhdr.time);
          printf("Run Header DAQ: %u \n", rhdr.daq_ver);
          printf("Run Header Calib: %u \n", rhdr.calib_trial_id);
          printf("Run Header Src: %u \n", rhdr.srcmask);
          printf("Run Header Run: %u \n", rhdr.runmask);
          printf("Run Header Crate: %u \n", rhdr.cratemask);
          printf("Run Header First: %u \n", rhdr.first_event_id);
          printf("Run Header Valid: %u \n", rhdr.valid_event_id);
          printf("Run Header RunID: %u \n", rhdr.run_id);
*/
	  break;
        default:
	  printf("Found header with unknown type, size: %u, %u \nSomething has gone wrong", chdr.record_type, chdr.size);
          return 1;
      }
    }

  // Close cdab file
  cdab.close();

  // Write remaining events to disk
  eos_hdf5_file.write();
  return 0;
}
