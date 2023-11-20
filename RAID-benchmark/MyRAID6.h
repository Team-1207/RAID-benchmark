#pragma once
#include <fstream>
#include <vector>
#include <cmath>
#include <string>
#include "GaluaField256.h"

const size_t SECTOR_SZ = 4096; //sector size in bytes
const size_t NAME_SZ = 256;
const size_t START_SECTOR_DATA = 1280; //Reserve 5Mb for metadata

class MyRAID
{
private:
	struct Block
	{
		std::vector<uint8_t> blockData0;
		std::vector<uint8_t> blockData1;
		std::vector<uint8_t> blockData2;
		std::vector<uint8_t> blockCheck0;
		std::vector<uint8_t> blockCheck1;
	};
	struct Inode
	{
		char fileName[NAME_SZ];
		uint16_t sectorOffset;
		bool isFree;
	};

private: //consts
	const std::string RAID_PATH = "C:\\RAID_MODEL\\";
	const std::string RAID_DISK_0 = "C:\\RAID_MODEL\\DISK0.myraid";
	const std::string RAID_DISK_1 = "C:\\RAID_MODEL\\DISK1.myraid";
	const std::string RAID_DISK_2 = "C:\\RAID_MODEL\\DISK2.myraid";
	const std::string RAID_DISK_3 = "C:\\RAID_MODEL\\DISK3.myraid";
	const std::string RAID_DISK_4 = "C:\\RAID_MODEL\\DISK4.myraid";
	const uint8_t NUM_DATA_DISKS = 3;
	const uint8_t NUM_ALL_DISKS = 5;

private: //fields
	std::string filePath = "";
	bool isRead;

	std::fstream raidFile0;
	std::fstream raidFile1;
	std::fstream raidFile2;
	std::fstream raidFile3;
	std::fstream raidFile4;
	std::fstream userFile;

	Inode metaData;
	uint16_t currentSector = START_SECTOR_DATA;

private: //methods
	int calc_parity_bits(const std::vector<uint8_t> inData0, const std::vector<uint8_t> inData1, const std::vector<uint8_t> inData2, std::vector<uint8_t>& outP1, std::vector<uint8_t>& outP2);
	int calc_reedsolomon(std::string& out);
	int get_datablock(std::vector<uint8_t>& out);
	int get_metadata(std::vector<uint8_t>& out);
	int get_free_sector();
	int write_file();
	int read_file();

public:
	MyRAID(std::string filePath, bool isRead);
	~MyRAID();
};

std::vector<uint8_t> operator^ (const std::vector<uint8_t> inData1, const std::vector<uint8_t> inData2);