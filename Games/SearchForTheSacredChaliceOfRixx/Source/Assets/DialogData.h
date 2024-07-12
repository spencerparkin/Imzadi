#pragma once

#include "AssetCache.h"
#include <unordered_map>
#include <string>

class DialogSequence;
class DialogTurn;

class DialogData : public Imzadi::Asset
{
public:
	DialogData();
	virtual ~DialogData();

	virtual bool Load(const rapidjson::Document& jsonDoc, Imzadi::AssetCache* assetCache) override;
	virtual bool Unload() override;

	typedef std::unordered_map<std::string, Imzadi::Reference<DialogSequence>> SequenceMap;
	SequenceMap sequenceMap;
};

class DialogElement : public Imzadi::ReferenceCounted
{
public:
	DialogElement();
	virtual ~DialogElement();

	virtual bool Load(const rapidjson::Value& elementValue);

	std::string speaker;
};

class DialogBasicElement : public DialogElement
{
public:
	DialogBasicElement();
	virtual ~DialogBasicElement();

	virtual bool Load(const rapidjson::Value& elementValue) override;

	std::string text;
};

class DialogChoiceElement : public DialogElement
{
public:
	DialogChoiceElement();
	virtual ~DialogChoiceElement();

	virtual bool Load(const rapidjson::Value& elementValue) override;

	struct Choice
	{
		std::string text;
		std::string sequenceName;
	};

	std::vector<Choice> choiceArray;
};

class DialogAcquireElement : public DialogBasicElement
{
public:
	DialogAcquireElement();
	virtual ~DialogAcquireElement();

	virtual bool Load(const rapidjson::Value& elementValue) override;

	std::string acquiredItem;
};

class DialogSequence : public Imzadi::ReferenceCounted
{
public:
	DialogSequence();
	virtual ~DialogSequence();

	std::vector<Imzadi::Reference<DialogElement>> dialogElementArray;
};