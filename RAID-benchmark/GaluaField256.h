#pragma once
#include <iostream>
#include <vector>
#include <string>

class GaluaField256
{
private:
	uint8_t Polynom;
	const uint8_t basePolynom = 195;
	const unsigned char Module = 2;
	const unsigned char polynomSz = 8;

public:
	GaluaField256();
	GaluaField256(const uint8_t Init);
	unsigned char& at(const size_t Index);
	explicit operator uint8_t() const { return Polynom; }
	GaluaField256& operator=(const GaluaField256& operand)
	{
		Polynom = operand.Polynom;
		return *this;
	}
	GaluaField256 operator>>(const size_t operand) const;
	GaluaField256 operator<<(const size_t operand) const;
	bool operator==(const GaluaField256) const;
	bool operator!=(const GaluaField256) const;
	GaluaField256 operator&(const uint8_t operand) const;
	GaluaField256 operator+(GaluaField256 operand) const;
	//GaluaField256 operator*(unsigned char operand) const; //Умножение многочлена на константу
	GaluaField256 operator*(uint8_t operand) const; //Попарное умножение элементов многочленов
	GaluaField256 operator*(const GaluaField256 operand) const; //Умножение многочлена на многочлен
	GaluaField256 operator%(const GaluaField256 operand) const;
	GaluaField256 pow(const GaluaField256 baseDeg, const unsigned char indDeg);

};

std::ostream& operator<<(std::ostream& out, GaluaField256 GFOut);