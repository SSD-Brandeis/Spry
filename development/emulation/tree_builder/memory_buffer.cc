#include "memory_buffer.h"

using namespace std;
using namespace tree_builder;
using namespace workload_exec;


/*Set up the singleton object with the experiment wide options*/
MemoryBuffer *MemoryBuffer::buffer_instance = 0;
long MemoryBuffer::max_buffer_size = 0;
float MemoryBuffer::buffer_flush_threshold = 0;
long MemoryBuffer::current_buffer_entry_count = 0;
long MemoryBuffer::current_buffer_point_entry_count = 0;
long MemoryBuffer::current_buffer_range_entry_count = 0;
long MemoryBuffer::current_buffer_size = 0;
float MemoryBuffer::current_buffer_saturation = 0;
int MemoryBuffer::buffer_flush_count = 0;

int MemoryBuffer::verbosity = 0;

EntryList* MemoryBuffer::buffer = new EntryList();




MemoryBuffer::MemoryBuffer(EmuEnv *_env)
{
  max_buffer_size = _env->buffer_size;
  buffer_flush_threshold = _env->buffer_flush_threshold;
  verbosity = _env->verbosity;
  int entries_per_file = _env->entries_per_page * _env->buffer_size_in_pages;

  for (int i = 1; i < 32; ++i)
  {
    DiskMetaFile::disk_run_flush_threshold[i] = _env->disk_run_flush_threshold;
    DiskMetaFile::level_max_size[i] = _env->buffer_size * pow(_env->size_ratio, i);
    DiskMetaFile::level_max_file_count[i] = DiskMetaFile::level_max_size[i]/(entries_per_file*_env->entry_size);
  }
cout<< "MonmorybufferTrack First level max file: "<< DiskMetaFile::level_max_file_count[1]<<endl;
}

MemoryBuffer *MemoryBuffer::getBufferInstance(EmuEnv *_env)
{
  if (buffer_instance == 0)
    buffer_instance = new MemoryBuffer(_env);

  return buffer_instance;
}

int MemoryBuffer::setCurrentBufferStatistics(int point_entry_count_change, int range_entry_count_change, int entry_size_change)
{
  current_buffer_entry_count += (point_entry_count_change + range_entry_count_change);
  current_buffer_point_entry_count += point_entry_count_change;
  current_buffer_range_entry_count += range_entry_count_change;
  current_buffer_size += entry_size_change;
  current_buffer_saturation = (float)current_buffer_size / max_buffer_size;

  return 1;
}

int MemoryBuffer::getCurrentBufferStatistics()
{
  std::cout << "********** PRINTING CURRENT BUFFER STATISTICS **********" << std::endl;
  std::cout << "Current buffer entry count = " << current_buffer_entry_count << std::endl;
  std::cout << "Current buffer point entry count = " << current_buffer_point_entry_count << std::endl;
  std::cout << "Current buffer range entry count = " << current_buffer_range_entry_count << std::endl;
  std::cout << "Current buffer size = " << current_buffer_size << std::endl;
  std::cout << "Current buffer saturation = " << current_buffer_saturation << std::endl;
  std::cout << "Total buffer flushes  = " << buffer_flush_count << std::endl;
  std::cout << "Total buffer insert (point) = " << WorkloadExecutor::total_insert_count << std::endl;
  std::cout << "insert count (point) (append) = " << WorkloadExecutor::buffer_insert_count << std::endl;
  std::cout << "Update count (point) (in-place) = " << WorkloadExecutor::buffer_update_count << std::endl;
  std::cout << "Total buffer range delete (range) = " << WorkloadExecutor::total_range_tombstone_count << std::endl;
  std::cout << "range delete (range) (append) = " << WorkloadExecutor::buffer_range_tombstone_count << std::endl;
  std::cout << "********************************************************" << std::endl;
  return 1;
}

int MemoryBuffer::initiateBufferFlush(int level_to_flush_in)
{
  if (MemoryBuffer::verbosity == 2){
    cout << "Calling sort and write from Buffer............................" << endl;
  }

  Utility::sortAndWrite_RDF(MemoryBuffer::getBuffer(), level_to_flush_in);
  buffer->clear();
  MemoryBuffer::buffer_flush_count++;
  return 1;
}

int MemoryBuffer::printBufferEntries()
{
  long size = 0;
  std::cout << "Printing sorted buffer (only keys): ";
  MemoryBuffer::buffer->sortPointEntryVectorOnSortKseyInAscendingOrder();
  for (int i = 0; i < MemoryBuffer::buffer->get_point_entry_vector_size(); ++i)
  {
    std::cout << "< " << MemoryBuffer::buffer->get_ith_point_entry(i).getSortKey() << ",  " 
              << MemoryBuffer::buffer->get_ith_point_entry(i).getValue() << " >"
              << "\t";
    size += (2 * sizeof(MemoryBuffer::buffer->get_ith_point_entry(i).getSortKey()) 
            + MemoryBuffer::buffer->get_ith_point_entry(i).getValue().size());
  }
  for (int i = 0; i < MemoryBuffer::buffer->get_range_entry_vector_size(); ++i)
  {
    std::cout << "< " << MemoryBuffer::buffer->get_ith_range_entry(i).getStartKey() << ",  " 
              << MemoryBuffer::buffer->get_ith_range_entry(i).getEndKey()  << " >"
              << "\t";
    size += (2 * sizeof(MemoryBuffer::buffer->get_ith_range_entry(i).getStartKey()));
  }
  
  std::cout << std::endl;


  std::cout << "Buffer size = " << size << std::endl;

  return 1;
}

EntryList& MemoryBuffer::getBuffer(){
  return *buffer;
}
