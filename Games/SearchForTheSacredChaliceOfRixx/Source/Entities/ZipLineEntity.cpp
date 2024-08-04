#include "ZipLineEntity.h"

ZipLineEntity::ZipLineEntity()
{
}

/*virtual*/ ZipLineEntity::~ZipLineEntity()
{
}

void ZipLineEntity::SetZipLine(ZipLine* givenZipLine)
{
	this->zipLine = givenZipLine;
}

/*virtual*/ bool ZipLineEntity::Setup()
{
	return true;
}

/*virtual*/ bool ZipLineEntity::Shutdown()
{
	return true;
}

/*virtual*/ bool ZipLineEntity::Tick(Imzadi::TickPass tickPass, double deltaTime)
{
	return true;
}