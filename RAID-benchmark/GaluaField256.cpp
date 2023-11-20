#include "GaluaField256.h"

GaluaField256::GaluaField256()
{
	Polynom = 0;
}

GaluaField256::GaluaField256(const uint8_t Init)
{
	Polynom = Init;
}


unsigned char& GaluaField256::at(const size_t Index)
{
	if (Index >= polynomSz) throw std::out_of_range("Invalid index");
	return reinterpret_cast<unsigned char*>(&Polynom)[Index];
}

GaluaField256 GaluaField256::operator>>(const size_t operand) const
{
	return Polynom >> operand;
}

GaluaField256 GaluaField256::operator<<(const size_t operand) const
{
	return Polynom << operand;
}

bool GaluaField256::operator==(const GaluaField256 operand) const
{
	return (*this).Polynom == operand.Polynom;
}

bool GaluaField256::operator!=(const GaluaField256 operand) const
{
	return (*this).Polynom != operand.Polynom;
}

GaluaField256 GaluaField256::operator&(const uint8_t operand) const
{
	return GaluaField256((*this).Polynom & operand);
}

GaluaField256 GaluaField256::operator+(GaluaField256 operand) const
{
	return (*this).Polynom ^ operand.Polynom;
}

GaluaField256 GaluaField256::operator*(uint8_t operand) const
{
	return (*this).Polynom * operand;
}

GaluaField256 GaluaField256::operator*(GaluaField256 operand) const
{
	GaluaField256 oper1 = *this;
	GaluaField256 Itog;
	size_t kolvoSteps = 0;
	for (kolvoSteps = 0; kolvoSteps < polynomSz; ++kolvoSteps)
		if ((oper1 >> kolvoSteps) == 0) break;
	for (size_t i = 0; i < kolvoSteps; ++i)
	{
		Itog = Itog + (operand * (uint8_t)(oper1 & 1));
		operand = (operand << 1) + basePolynom * ((operand >> (polynomSz - 1)).Polynom);
		oper1 = oper1 >> 1;
	}
	return Itog;
}

std::ostream& operator<<(std::ostream& out, GaluaField256 GFOut)
{
	std::string strOut = "0+";
	for (size_t i = 0; i < 8; ++i)
	{
		if (GFOut == 0) break;
		if ((GFOut & 1) != GaluaField256(0)) strOut = strOut + "+x^" + std::to_string(i);
		GFOut = GFOut >> 1;
	}
	out << strOut;
	return out;
}