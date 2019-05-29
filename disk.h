#ifndef DISK_H_
#define DISK_H_

#define SECTOR_SIZE 512
#define MAX_SECTOR_ID 163296 //80mb
extern void get_data(int secotr_id, int sector_num, char* info);
extern void save_data(int sector_id, int sector_num, char* info);

#endif // !DISK_H_
