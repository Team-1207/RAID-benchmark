#include "GaluaField256.h"
#include "MyRAID6.h"
#include <Windows.h>

int main()
{
	GaluaField256 A(255);
	GaluaField256 B(255);
	GaluaField256 C = A * B;
	std::cout << C << "\n";
	getchar();
	system("cls");

	MyRAID writeFile("C:\\RAID_MODEL\\writeCheck.txt", false);

	MyRAID readFile("C:\\RAID_MODEL\\readCheck.txt", true);
	std::cout << "Delo sdelano\n";
	getchar();
	return 0;
}