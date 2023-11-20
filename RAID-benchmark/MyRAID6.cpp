#include "MyRAID6.h"

std::vector<uint8_t> operator^ (const std::vector<uint8_t> inData1, const std::vector<uint8_t> inData2)
{
	std::vector<uint8_t> Itog = inData1;
	for (size_t i = 0; i < SECTOR_SZ; ++i)
		Itog.at(i) = Itog.at(i) ^ inData2.at(i);
	return Itog;
}

MyRAID::MyRAID(std::string filePath, bool isRead)
{
	raidFile0.open(RAID_DISK_0);
	if (!raidFile0.is_open()) throw std::runtime_error("DISK 0 open error");
	raidFile1.open(RAID_DISK_1);
	if (!raidFile1.is_open()) throw std::runtime_error("DISK 1 open error");
	raidFile2.open(RAID_DISK_2);
	if (!raidFile2.is_open()) throw std::runtime_error("DISK 2 open error");
	raidFile3.open(RAID_DISK_3);
	if (!raidFile3.is_open()) throw std::runtime_error("DISK 3 open error");
	raidFile4.open(RAID_DISK_4);
	if (!raidFile4.is_open()) throw std::runtime_error("DISK 4 open error");

	userFile.open(filePath);
	if (!userFile.is_open()) throw std::runtime_error("UserFile open error");
	strcmp(metaData.fileName, filePath.c_str());

	//Заготовка под обработку комманд
	if (isRead) read_file();
	else write_file();
}

MyRAID::~MyRAID()
{
	raidFile0.close();
	raidFile1.close();
	raidFile2.close();
	raidFile3.close();
	raidFile4.close();

	userFile.close();
}

int MyRAID::calc_parity_bits(const std::vector<uint8_t> inData0, const std::vector<uint8_t> inData1, const std::vector<uint8_t> inData2, std::vector<uint8_t>& outP1, std::vector<uint8_t>& outP2)
{
	outP1 = inData0 ^ inData1 ^ inData2;
	outP2 = inData0 ^ inData1 ^ outP1;
	return 0;
}

int MyRAID::get_datablock(std::vector<uint8_t>& out)
{
	out = std::vector<uint8_t>(SECTOR_SZ, 0);
	std::string tmpReadStr = "";
	for (size_t i = 0; (i < SECTOR_SZ) && (!userFile.eof()); ++i)
	{
		char* tmpChar;
		tmpChar = new char;
		userFile.read(tmpChar, 1);
		tmpReadStr.push_back(*tmpChar);
		delete tmpChar;
	}
	size_t outSz = tmpReadStr.size();
	for (size_t i = 0; i < outSz; ++i) out.at(i) = tmpReadStr.at(i);
	if (outSz < SECTOR_SZ) out.at(outSz) = 1;
	else if (outSz > SECTOR_SZ) return 2;
	return 0;
}

int MyRAID::get_metadata(std::vector<uint8_t>& out)
{
	out = std::vector<uint8_t>(0, SECTOR_SZ);
	userFile.seekg(0, std::ios::end);
	std::streampos fileSz = userFile.tellg();
	userFile.seekg(0, std::ios::beg);
	return 0;
}

int MyRAID::get_free_sector()
{
	raidFile0.seekg(0, std::ios::beg);
	raidFile1.seekg(0, std::ios::beg);
	raidFile2.seekg(0, std::ios::beg);
	raidFile3.seekg(0, std::ios::beg);
	raidFile4.seekg(0, std::ios::beg);
	bool isFree = false;
	for (short int i = 0; !isFree; ++i)
	{
		char* tmpStr;
		tmpStr = new char;
		if ((i == 0) || (i == 3) || (i == 6)) raidFile0.read(tmpStr, sizeof(bool));
		else if ((i == 1) || (i == 4) || (i == 12)) raidFile1.read(tmpStr, sizeof(bool));
		else if ((i == 2) || (i == 9) || (i == 13)) raidFile2.read(tmpStr, sizeof(bool));
		else if ((i == 7) || (i == 10) || (i == 14)) raidFile3.read(tmpStr, sizeof(bool));
		else if ((i == 5) || (i == 8) || (i == 11)) raidFile4.read(tmpStr, sizeof(bool));
		if ((i == 2) || (i == 5) || (i == 8) || (i == 11))
		{
			raidFile0.seekg(SECTOR_SZ, std::ios_base::cur);
			raidFile1.seekg(SECTOR_SZ, std::ios_base::cur);
			raidFile2.seekg(SECTOR_SZ, std::ios_base::cur);
			raidFile3.seekg(SECTOR_SZ, std::ios_base::cur);
			raidFile4.seekg(SECTOR_SZ, std::ios_base::cur);
		}
		else if (i == 14) i = -1;
		isFree = (bool)tmpStr;
		if (!isFree) ++currentSector;
		if (currentSector > 16384) break;
		delete tmpStr;
	}
	if (!isFree) return 1;
	return 0;
}

int MyRAID::write_file() 
{						
	Block writeBlock;
	get_free_sector();
	while (!userFile.eof())
	{
		get_datablock(writeBlock.blockData0);
		get_datablock(writeBlock.blockData1);
		get_datablock(writeBlock.blockData2);
		calc_parity_bits(writeBlock.blockData0, writeBlock.blockData1, writeBlock.blockData2, writeBlock.blockCheck0, writeBlock.blockCheck1);
		size_t lastPos = (size_t)(currentSector);
		raidFile0.seekg(lastPos * SECTOR_SZ, std::ios::beg);
		raidFile1.seekg(lastPos * SECTOR_SZ, std::ios::beg);
		raidFile2.seekg(lastPos * SECTOR_SZ, std::ios::beg);
		raidFile3.seekg(lastPos * SECTOR_SZ, std::ios::beg);
		raidFile4.seekg(lastPos * SECTOR_SZ, std::ios::beg);
		bool tmpBool = true;
		if (lastPos % NUM_ALL_DISKS == 4)
		{

			raidFile0.write((char*)writeBlock.blockCheck1.data(), SECTOR_SZ);
			raidFile1.write((char*)writeBlock.blockData0.data(), SECTOR_SZ);
			raidFile2.write((char*)writeBlock.blockData1.data(), SECTOR_SZ);
			raidFile3.write((char*)writeBlock.blockData2.data(), SECTOR_SZ);
			raidFile4.write((char*)writeBlock.blockCheck0.data(), SECTOR_SZ);
			raidFile0.seekg((lastPos - START_SECTOR_DATA) * SECTOR_SZ, std::ios::beg);
			raidFile1.seekg((lastPos - START_SECTOR_DATA) * SECTOR_SZ, std::ios::beg);
			raidFile2.seekg((lastPos - START_SECTOR_DATA) * SECTOR_SZ, std::ios::beg);
			raidFile3.seekg((lastPos - START_SECTOR_DATA) * SECTOR_SZ, std::ios::beg);
			raidFile4.seekg((lastPos - START_SECTOR_DATA) * SECTOR_SZ, std::ios::beg);
			raidFile0.write((char*)&tmpBool, sizeof(bool));
			raidFile1.write((char*)&tmpBool, sizeof(bool));
			raidFile2.write((char*)&tmpBool, sizeof(bool));
			raidFile3.write((char*)&tmpBool, sizeof(bool));
			raidFile4.write((char*)&tmpBool, sizeof(bool));
		}
		else if (lastPos % NUM_ALL_DISKS == 3)
		{
			raidFile0.write((char*)writeBlock.blockCheck0.data(), SECTOR_SZ);
			raidFile1.write((char*)writeBlock.blockCheck1.data(), SECTOR_SZ);
			raidFile2.write((char*)writeBlock.blockData0.data(), SECTOR_SZ);
			raidFile3.write((char*)writeBlock.blockData1.data(), SECTOR_SZ);
			raidFile4.write((char*)writeBlock.blockData2.data(), SECTOR_SZ);
			raidFile0.seekg((lastPos - START_SECTOR_DATA) * SECTOR_SZ, std::ios::beg);
			raidFile1.seekg((lastPos - START_SECTOR_DATA) * SECTOR_SZ, std::ios::beg);
			raidFile2.seekg((lastPos - START_SECTOR_DATA) * SECTOR_SZ, std::ios::beg);
			raidFile3.seekg((lastPos - START_SECTOR_DATA) * SECTOR_SZ, std::ios::beg);
			raidFile4.seekg((lastPos - START_SECTOR_DATA) * SECTOR_SZ, std::ios::beg);
			raidFile0.write((char*)&tmpBool, sizeof(bool));
			raidFile1.write((char*)&tmpBool, sizeof(bool));
			raidFile2.write((char*)&tmpBool, sizeof(bool));
			raidFile3.write((char*)&tmpBool, sizeof(bool));
			raidFile4.write((char*)&tmpBool, sizeof(bool));
		}
		else if (lastPos % NUM_ALL_DISKS == 2)
		{
			raidFile0.write((char*)writeBlock.blockData0.data(), SECTOR_SZ);
			raidFile1.write((char*)writeBlock.blockCheck0.data(), SECTOR_SZ);
			raidFile2.write((char*)writeBlock.blockCheck1.data(), SECTOR_SZ);
			raidFile3.write((char*)writeBlock.blockData1.data(), SECTOR_SZ);
			raidFile4.write((char*)writeBlock.blockData2.data(), SECTOR_SZ);
			raidFile0.seekg((lastPos - START_SECTOR_DATA) * SECTOR_SZ, std::ios::beg);
			raidFile1.seekg((lastPos - START_SECTOR_DATA) * SECTOR_SZ, std::ios::beg);
			raidFile2.seekg((lastPos - START_SECTOR_DATA) * SECTOR_SZ, std::ios::beg);
			raidFile3.seekg((lastPos - START_SECTOR_DATA) * SECTOR_SZ, std::ios::beg);
			raidFile4.seekg((lastPos - START_SECTOR_DATA) * SECTOR_SZ, std::ios::beg);
			raidFile0.write((char*)&tmpBool, sizeof(bool));
			raidFile1.write((char*)&tmpBool, sizeof(bool));
			raidFile2.write((char*)&tmpBool, sizeof(bool));
			raidFile3.write((char*)&tmpBool, sizeof(bool));
			raidFile4.write((char*)&tmpBool, sizeof(bool));
		}

		else if ((lastPos) % NUM_ALL_DISKS == 1)
		{
			raidFile0.write((char*)writeBlock.blockData0.data(), SECTOR_SZ);
			raidFile1.write((char*)writeBlock.blockData1.data(), SECTOR_SZ);
			raidFile2.write((char*)writeBlock.blockCheck0.data(), SECTOR_SZ);
			raidFile3.write((char*)writeBlock.blockCheck1.data(), SECTOR_SZ);
			raidFile4.write((char*)writeBlock.blockData2.data(), SECTOR_SZ);
			raidFile0.seekg((lastPos - START_SECTOR_DATA) * SECTOR_SZ, std::ios::beg);
			raidFile1.seekg((lastPos - START_SECTOR_DATA) * SECTOR_SZ, std::ios::beg);
			raidFile2.seekg((lastPos - START_SECTOR_DATA) * SECTOR_SZ, std::ios::beg);
			raidFile3.seekg((lastPos - START_SECTOR_DATA) * SECTOR_SZ, std::ios::beg);
			raidFile4.seekg((lastPos - START_SECTOR_DATA) * SECTOR_SZ, std::ios::beg);
			raidFile0.write((char*)&tmpBool, sizeof(bool));
			raidFile1.write((char*)&tmpBool, sizeof(bool));
			raidFile2.write((char*)&tmpBool, sizeof(bool));
			raidFile3.write((char*)&tmpBool, sizeof(bool));
			raidFile4.write((char*)&tmpBool, sizeof(bool));
		}
		else if ((lastPos) % NUM_ALL_DISKS == 0)
		{
			raidFile0.write((char*)writeBlock.blockData0.data(), SECTOR_SZ);
			raidFile1.write((char*)writeBlock.blockData1.data(), SECTOR_SZ);
			raidFile2.write((char*)writeBlock.blockData2.data(), SECTOR_SZ);
			raidFile3.write((char*)writeBlock.blockCheck0.data(), SECTOR_SZ);
			raidFile4.write((char*)writeBlock.blockCheck1.data(), SECTOR_SZ);
			raidFile0.seekg((lastPos - START_SECTOR_DATA) * SECTOR_SZ, std::ios::beg);
			raidFile1.seekg((lastPos - START_SECTOR_DATA) * SECTOR_SZ, std::ios::beg);
			raidFile2.seekg((lastPos - START_SECTOR_DATA) * SECTOR_SZ, std::ios::beg);
			raidFile3.seekg((lastPos - START_SECTOR_DATA) * SECTOR_SZ, std::ios::beg);
			raidFile4.seekg((lastPos - START_SECTOR_DATA) * SECTOR_SZ, std::ios::beg);
			raidFile0.write((char*)&tmpBool, sizeof(bool));
			raidFile1.write((char*)&tmpBool, sizeof(bool));
			raidFile2.write((char*)&tmpBool, sizeof(bool));
			raidFile3.write((char*)&tmpBool, sizeof(bool));
			raidFile4.write((char*)&tmpBool, sizeof(bool));
		}
	}
	return 0;
}

int MyRAID::read_file()
{
	char* outStr;
	outStr = new char[SECTOR_SZ + 1];
	raidFile0.seekg(START_SECTOR_DATA * SECTOR_SZ, std::ios::beg);
	raidFile0.read(outStr, SECTOR_SZ);
	userFile.write(outStr, SECTOR_SZ);
	delete[] outStr;
	return 0;
}