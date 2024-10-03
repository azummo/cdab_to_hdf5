#include <iostream>
#include <fstream>
#include <vector>
#include <H5Cpp.h>
#include "ds.h"
#include "eos_hdf5.hh"

int main(int argc, char** argv)
{

  CDABHeader chdr;
  RunStart rhdr;
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

      switch(chdr.record_type)
      {
        case EMPTY:
          std::cout << "Found empty event" << std::endl;
	  break;
        case DETECTOR_EVENT:
          // Attempt to read event according to CDAB Header
          cdab.read(reinterpret_cast<char*>(&e), sizeof(e));
	  // Add event to hdf5 buffer
	  eos_hdf5_file.fill(e);
	  break;
	case RUN_START:
          // Just print header information for now
          cdab.read(reinterpret_cast<char*>(&rhdr),sizeof(rhdr));
          printf("Run Header Type: %u \n", rhdr.type);
          printf("Run Header Number: %u \n", rhdr.run_number);
          printf("Run Header Filename: %s \n", rhdr.outfile);
          printf("Run Header Run Type: %u \n", rhdr.run_type);
          printf("Run Header Source Type: %u \n", rhdr.source_type);
          printf("Run Header Source x: %f \n", rhdr.source_x);
          printf("Run Header Source y: %f \n", rhdr.source_y);
          printf("Run Header Source z: %f \n", rhdr.source_z);
          printf("Run Header Source theta: %f \n", rhdr.source_theta);
          printf("Run Header Source phi: %f \n", rhdr.source_phi);
          printf("Run Header Fiber number: %u \n", rhdr.fiber_number);
          printf("Run Header Laserball size: %f \n", rhdr.laserball_size);
          printf("Run Header Laser wavelength: %f \n", rhdr.laser_wavelength);
          printf("Run Header First Event ID: %lu \n", rhdr.first_event_id);
          // TODO fill meta information
          eos_hdf5_file.write_meta(rhdr);
	  break;
        case RUN_END:
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
